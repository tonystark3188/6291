/*
 * Copyright (c) 2004-2005 Sergey Lyubka <valenok@gmail.com>
 * All rights reserved
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Sergey Lyubka wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */

/*
 * Small and portable HTTP server, http://shttpd.sourceforge.net
 * $Id: shttpd.c,v 1.10 2007/06/01 17:59:32 drozd Exp $
 */
#include <utime.h>  
#include <sys/sysinfo.h>
#include "defs.h"
#include "base.h"
#include "file_json.h"
//#include "media_process.h"


static int dm_msg_combin(char *pBuf,int nLen,char **output_buf,int *nTotalLen)
{
	MsgRet enRet = MSGRET_SUCCESS;
	MsgHeader *pstMsg;
	
	if (NULL == pBuf)
	{
		DMCLOG_E( "Send buffer is null!");
		return -1;
	}
	
	pstMsg = (MsgHeader *)malloc(sizeof(MsgHeader));
	if (pstMsg == NULL)
	{
		DMCLOG_E("malloc of msg header failed");
		return -1;
	}
	memset(pstMsg, 0, sizeof(MsgHeader));
	pstMsg->dataLength = nLen;
	
	pstMsg = (MsgHeader *)realloc(pstMsg, sizeof(MsgHeader) + nLen);
	if (pstMsg == NULL)
	{
		DMCLOG_D("realloc to %ld bytes failed", sizeof(MsgHeader) + nLen);
		free(pstMsg);
		return -1;
	}
	memcpy((SINT8 *)(pstMsg+1), pBuf, nLen);
	
	*nTotalLen = sizeof(MsgHeader) + nLen;
	*output_buf = (SINT8 *)pstMsg;
	return 0;
}

/*
 * Send error message back to a client.
 */
void
send_server_error(struct conn *c,const char *reason)
{
	c->loc.io.head = c->loc.headers_len = my_snprintf(c->loc.io.buf,c->loc.io.size,"%s", reason);
	if(c->loc.io.head == 0)
	{
		return ;
	}	
	DMCLOG_D("c->loc.io.buf = %s",c->loc.io.buf);
}

void
send_server_comb(struct conn *c,const char *reason)
{
	char *send_buf = NULL;
	int nTotalLen = 0;
	DMCLOG_D("send:%s",reason);
	if(dm_msg_combin(reason,strlen(reason),&send_buf,&nTotalLen) < 0)
	{
		EXIT_FUNC();
		return;
	}
	memcpy(c->loc.io.buf, send_buf, nTotalLen);
	c->loc.io.head = c->loc.headers_len = nTotalLen;	
}

void
read_stream_comb(struct stream *stream,const char *reason)
{
	char *send_buf = NULL;
	int nTotalLen = 0;
	int len = 0;
	DMCLOG_D("send:%s",reason);
	if(dm_msg_combin(reason,strlen(reason),&send_buf,&nTotalLen) < 0)
	{
		EXIT_FUNC();
		return;
	}

	len = io_space_len(&stream->io);
	assert(len > 0);
	if(len < nTotalLen)
	{
		DMCLOG_E("no enough space");
		return ;
	}
	memcpy(io_space(&stream->io), send_buf, nTotalLen);
	io_inc_head(&stream->io, nTotalLen);
	return ;
}



int file_json_to_string(struct conn *c,JObj* response_json)
{
	ENTER_FUNC();
	JObj* header_json = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(header_json, "token", JSON_NEW_OBJECT(c->token,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	char *send_buf = NULL;
	int nTotalLen = 0;
	char *response_str = JSON_TO_STRING(response_json);
	if(response_str == NULL)
	{
		EXIT_FUNC();
		return -1;
	}
	DMCLOG_D("send:%s",response_str);
	if(dm_msg_combin(response_str,strlen(response_str),&send_buf,&nTotalLen) < 0)
	{
		EXIT_FUNC();
		return -1;
	}
	memcpy(c->loc.io.buf, send_buf, nTotalLen);
	c->loc.io.head = c->loc.headers_len = nTotalLen;
	if(c->loc.io.head == 0)
	{
		safe_free(send_buf);
		return -1;
	}	
	safe_free(send_buf);
	EXIT_FUNC();
	return 0;
}

static LL_HEAD(listeners);	/* List of listening sockets	*/


struct listener {
	struct llhead	link;
	struct shttpd_ctx *ctx;		/* Context that socket belongs	*/
	int		sock;		/* Listening socket		*/
};


void
stop_stream(struct stream *stream)
{
	//if close rem fd now ,we can not send upload success msg to client
	if(!(stream->conn->cmd == FN_FILE_UPLOAD|| stream->conn->cmd == FN_FILE_GET_BACKUP_FILE || stream->conn->cmd == FN_ROUTER_SET_UPLOAD_FIRMWARE))
	{
		if (stream->io_class != NULL && stream->io_class->close != NULL)
			stream->io_class->close(stream);
		stream->io_class = NULL;
	}
	
	stream->flags |= FLAG_CLOSED;
	stream->flags &= ~(FLAG_R | FLAG_W | FLAG_ALWAYS_READY);

	DMCLOG_D("%d %s stopped. %lu of content data, %d now in a buffer",
	    stream->conn->rem.chan.sock, 
	    stream->io_class ? stream->io_class->name : "(null)",
	    (unsigned long) stream->io.total, io_data_len(&stream->io));
}

/*
 * Setup listening socket on given port, return socket
 */
static int
open_listening_port(int port)
{
	int		sock, on = 1;
	struct usa	sa;

	sa.len				= sizeof(sa.u.sin);
	sa.u.sin.sin_family		= AF_INET;
	sa.u.sin.sin_port		= htons((uint16_t) port);
	sa.u.sin.sin_addr.s_addr	= htonl(INADDR_ANY);

	if ((sock = socket(PF_INET, SOCK_STREAM, 6)) == -1)
		goto fail;
	if (set_non_blocking_mode(sock) != 0)
		goto fail;
	if (setsockopt(sock, SOL_SOCKET,
	    SO_REUSEADDR,(char *) &on, sizeof(on)) != 0)
		goto fail;
	if (bind(sock, &sa.u.sa, sa.len) < 0)
		goto fail;
	if (listen(sock, 128) != 0)
		goto fail;

	return (sock);
fail:
	if (sock != -1)
		(void) closesocket(sock);
	//elog(E_LOG, NULL, "open_listening_port(%d): %s", port, strerror(errno));
	return (-1);
}
#if 0
static void
msg_write_stream(struct stream *from, struct stream *to)
{
	int	n, len;
	len = io_data_len(&from->io);
	assert(len > 0);
	/* TODO: should be assert on CAN_WRITE flag */
	n = msg_write_socket(to, io_data(&from->io), len);
	DMCLOG_D("write_stream (%d %s): written %d/%d bytes (errno %d)",
	    to->conn->rem.chan.sock,
	    to->io_class ? to->io_class->name : "(null)", n, len, ERRNO);
	if (n > 0)
	{
		io_clear(&from->io);
		io_clear(&to->io);
		//io_inc_tail(&from->io, n);
	}
	else if (n == -1 && (ERRNO == EINTR || ERRNO == EWOULDBLOCK))
		n = n;	/* Ignore EINTR and EAGAIN */
	else if (!(to->flags & FLAG_DONT_CLOSE))
		stop_stream(to);
}
#endif
static void
write_stream(struct stream *from, struct stream *to)
{
	int	n, len;
	len = io_data_len(&from->io);
	assert(len > 0);
	
	/* TODO: should be assert on CAN_WRITE flag */
	n = to->io_class->write(to, io_data(&from->io), len);
	if (n > 0)
	{
		io_inc_tail(&from->io, n);
		if(!(from->conn->cmd == FN_ROUTER_SET_UPLOAD_FIRMWARE||from->conn->cmd == FN_FILE_UPLOAD||from->conn->cmd == FN_FILE_GET_BACKUP_FILE))
			to->io.total += n;
	}
	else if (n == -1 && (ERRNO == EINTR || ERRNO == EWOULDBLOCK || ERRNO == EAGAIN))
	{
		n = n;	/* Ignore EINTR and EAGAIN */
	}
	else if (!(to->flags & FLAG_DONT_CLOSE))
		stop_stream(to);
	/*DMCLOG_D("write_stream (%d %s): written %d/%d/%lu bytes (errno %d)\n",
	    to->conn->rem.chan.sock,
	    to->io_class ? to->io_class->name : "(null)", n, len,(unsigned long) to->io.total, ERRNO);*/
}


static void
parse_http_request(struct conn *c)
{
	char	*s = c->rem.io.buf;
	char	*buf = NULL;
	file_parse_process(c);
	io_clear(&c->rem.io);
	io_clear(&c->loc.io);
	c->rem.flags |= FLAG_HEADERS_PARSED;
	if(c->error != 0){
		goto EXIT1;
	}
	file_func_process(c);
	if(c->error != 0){
		if(c->cmd == FN_FILE_UPLOAD||c->cmd == FN_FILE_GET_BACKUP_FILE||c->cmd == FN_FILE_GET_LIST||c->cmd == FN_FILE_GET_LSIT_BY_TYPE)
			goto EXIT1;
		goto EXIT2;
	}
	if(!(c->cmd == FN_FILE_DOWNLOAD ||c->cmd == FN_FILE_UPLOAD|| c->cmd == FN_FILE_GET_BACKUP_FILE \
		||c->cmd == FN_ROUTER_SET_UPLOAD_FIRMWARE || c->cmd == FN_FILE_SEARCH\
		||c->cmd == FN_FILE_GET_LIST||c->cmd == FN_FILE_GET_LSIT_BY_TYPE\
		||c->cmd == FN_FILE_GET_ALBUM_LIST\
		||c->cmd == FN_FILE_GET_DIR_BY_TYPE\
		||c->cmd == FN_FILE_GET_LIST_BY_PATH))
	{
		stop_stream(&c->loc);
	}	
	return;
	
EXIT1:
	DMCLOG_D("cmd = %d,error = %d",c->cmd,c->error);
	
	buf = DM_FileInotify(c->cmd ,c->error);
	if(c->cmd == FN_FILE_GET_LIST||c->cmd == FN_FILE_GET_LSIT_BY_TYPE)
	{
		send_server_error(c,buf);
	}else{
		send_server_comb(c,buf);
	}
	safe_free(buf);
EXIT2:
	stop_stream(&c->loc);
	return;
}

/* Set TCP keep alive option to detect dead peers. The interval option
 * is only used for Linux as we are using Linux-specific APIs to set
 * the probe send time, interval, and count. */
int anetKeepAlive(int fd, int interval)
{
    int val = 1;
    //开启keepalive机制
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&val, sizeof(val)) == -1)
    {
//        anetSetError(err, "setsockopt SO_KEEPALIVE: %s", strerror(errno));
        return -1;
    }
    
#ifdef __linux__
    /* Default settings are more or less garbage, with the keepalive time
     * set to 7200 by default on Linux. Modify settings to make the feature
     * actually useful. */
    
    /* Send first probe after interval. */
    val = interval;
    if (setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void *)&val, sizeof(val)) < 0) {
//        anetSetError(err, "setsockopt TCP_KEEPIDLE: %s\n", strerror(errno));
        return -1;
    }
    
    /* Send next probes after the specified interval. Note that we set the
     * delay as interval / 3, as we send three probes before detecting
     * an error (see the next setsockopt call). */
    val = interval/3;
    if (val == 0) val = 1;
    if (setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void *)&val, sizeof(val)) < 0) {
//        anetSetError(err, "setsockopt TCP_KEEPINTVL: %s\n", strerror(errno));
        return -1;
    }
    
    /* Consider the socket in error state after three we send three ACK
     * probes without getting a reply. */
    val = 2;
    if (setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void *)&val, sizeof(val)) < 0) {
//        anetSetError(err, "setsockopt TCP_KEEPCNT: %s\n", strerror(errno));
        return -1;
    }
#endif
    return 0;
}

void
shttpd_add_socket(struct shttpd_ctx *ctx, int sock)
{
	struct conn	*c;
	struct usa	sa;
	if(anetKeepAlive(sock, 9) < 0)
	{
		DMCLOG_E("net keep alive errno = %d",errno);
		return ;
	}
	sa.len = sizeof(sa.u.sin);
	(void) set_non_blocking_mode(sock);//设置为非阻塞

	if (getpeername(sock, &sa.u.sa, &sa.len)) {
		DMCLOG_D("add_socket: %s", strerror(errno));
	} else if ((c = calloc(1, sizeof(*c) + 2 * ctx->io_buf_size)) == NULL) {
		(void) closesocket(sock);
		DMCLOG_D("add_socket: calloc: %s", strerror(ERRNO));
	} else {
		DMCLOG_D("add socket success");
		c->rem.conn = c->loc.conn = c;
		c->ctx		= ctx;
		c->sa		= sa;
		strcpy(c->client_ip, inet_ntoa(sa.u.sin.sin_addr));
		DMCLOG_D("ctx->io_buf_size = %d,c->client_ip = %s",ctx->io_buf_size,c->client_ip);
		(void) getsockname(sock, &sa.u.sin, &sa.len);
		c->loc_port = sa.u.sin.sin_port;

		set_close_on_exec(sock);

		c->loc.io_class	= NULL;
	
		c->rem.io_class	= &io_socket;
		c->rem.chan.sock = sock;

		/* Set IO buffers */
		c->loc.io.buf	= (char *) (c + 1);
		c->rem.io.buf	= c->loc.io.buf + ctx->io_buf_size;
		c->loc.io.size	= c->rem.io.size = ctx->io_buf_size;

		c->record_time = 0;
		int len = io_space_len(&c->rem.io);
		assert(len > 0);
		EnterCriticalSection(&ctx->mutex);
		LL_TAIL(&ctx->connections, &c->link);
		ctx->nactive++;
		LeaveCriticalSection(&ctx->mutex);
		
		DMCLOG_D("%s:%hu connected (socket %d)",
		    inet_ntoa(* (struct in_addr *) &sa.u.sin.sin_addr.s_addr),
		    ntohs(sa.u.sin.sin_port), sock);
	}
}


/*
 * Setup a listening socket on given port. Return opened socket or -1
 */
int
shttpd_listen(struct shttpd_ctx *ctx, int port)
{
	struct listener	*l;
	int		sock;

	if ((sock = open_listening_port(port)) == -1) {
		DMCLOG_D("cannot open port %d", port);
	} else if ((l = calloc(1, sizeof(*l))) == NULL) {
		(void) closesocket(sock);
		DMCLOG_D("cannot allocate listener");
	} else {
		l->sock	= sock;
		l->ctx	= ctx;
		LL_TAIL(&listeners, &l->link);
		DMCLOG_D("shttpd_listen: added socket %d", sock);
	}

	return (sock);
}

int
shttpd_accept(int lsn_sock, int milliseconds)
{
	struct timeval	tv;
	struct usa	sa;
	fd_set		read_set;
	int		sock = -1;
	
	tv.tv_sec	= milliseconds / 1000;
	tv.tv_usec	= milliseconds % 1000;
	sa.len		= sizeof(sa.u.sin);
	FD_ZERO(&read_set);
	FD_SET(lsn_sock, &read_set);
	
	if (select(lsn_sock + 1, &read_set, NULL, NULL, &tv) == 1)
		sock = accept(lsn_sock, &sa.u.sa, &sa.len);

	return (sock);
}

static void
read_stream(struct stream *stream)
{
	int	n, len;
	len = io_space_len(&stream->io);
	assert(len > 0);
	/* Do not read more that needed */
	if (stream->content_len > 0 &&
	    stream->io.total + len > stream->content_len)
		len = stream->content_len - stream->io.total;
	/* Read from underlying channel */
	n = stream->nread_last = stream->io_class->read(stream,
	    io_space(&stream->io), len);
	if (n > 0)
	{
		io_inc_head(&stream->io, n);
		if(stream->content_len == stream->io.total&&(stream->conn->cmd == FN_ROUTER_SET_UPLOAD_FIRMWARE||stream->conn->cmd == FN_FILE_GET_BACKUP_FILE))
		{
			if(stream->conn->cmd == FN_ROUTER_SET_UPLOAD_FIRMWARE)
			{
				len = io_data_len(&stream->conn->rem.io);
				n = stream->conn->loc.io_class->write(&stream->conn->loc, io_data(&stream->conn->rem.io), len);
				if (n > 0)
				{
					io_inc_tail(&stream->conn->rem.io, n);
					stream->conn->loc.io.total += n;
					fsync(stream->conn->loc.chan.fd);
				}
				stream->conn->error = verify_firmware();
			}
			char *buf = DM_FileInotify(stream->conn->cmd,stream->conn->error);
			send_server_comb(stream->conn,buf);
			safe_free(buf);
			//stop_stream(stream);//if close rem fd now ,we can not send upload success msg to client
		}
	}
	else if (n == -1 && (ERRNO == EINTR || ERRNO == EWOULDBLOCK ))
		n = n;	/* Ignore EINTR and EAGAIN */
	else if (!(stream->flags & FLAG_DONT_CLOSE))
	{
		if((stream->conn->cmd == FN_FILE_GET_DIR_BY_TYPE||stream->conn->cmd == FN_FILE_GET_LSIT_BY_TYPE||stream->conn->cmd == FN_FILE_GET_ALBUM_LIST||stream->conn->cmd == FN_FILE_GET_LIST_BY_PATH||stream->conn->cmd == FN_FILE_GET_LIST)&&n == 0)
		{
			n = n;
		}else{
			stop_stream(stream);
		}
		
	}
	/*DMCLOG_D("read_stream (%d %s): read %d/%d/%lu/%lu bytes (errno %d)",
	    stream->conn->rem.chan.sock,
	    stream->io_class ? stream->io_class->name : "(null)",
	    n, len, (unsigned long) stream->io.total, (unsigned long)stream->content_len, ERRNO);*/

	/*
	 * Close the local stream if everything was read
	 * XXX We do not close the remote stream though! It may be
	 * a POST data completed transfer, we do not want the socket
	 * to be closed.
	 */
	if (stream->content_len > 0 && stream == &stream->conn->loc) {
		assert(stream->io.total <= stream->content_len);
		if (stream->io.total == stream->content_len)
		{
			stop_stream(stream);
		}
	}
}

static void
msg_read_stream(struct stream *stream)
{
	int	n, len;
	len = io_space_len(&stream->io);
	assert(len > 0);
	/* Do not read more that needed */
	if (stream->content_len > 0 &&
	    stream->io.total + len > stream->content_len)
		len = stream->content_len - stream->io.total;
	/* Read from underlying channel */
	n = stream->nread_last = msg_read_socket(stream,
	    io_space(&stream->io), len);
	if (n > 0)
	{
		DMCLOG_D("n = %d,buf = %s",n,io_space(&stream->io));
		io_inc_head(&stream->io, n);
		io_inc_tail(&stream->io, n);
	}
	else if (n <= 0 && (ERRNO == EINTR || ERRNO == EWOULDBLOCK))
	{
		DMCLOG_D("stop stream");
		stop_stream(stream);
	}
	DMCLOG_D("msg read_stream (%d %s): read %d/%d/%lu bytes (errno %d)",
	    stream->conn->rem.chan.sock,
	    stream->io_class ? stream->io_class->name : "(null)",
	    n, len, (unsigned long) stream->io.total, ERRNO);
	
	if (stream->content_len > 0 && stream == &stream->conn->loc) {
		assert(stream->io.total <= stream->content_len);
		if (stream->io.total == stream->content_len)
		{
			DMCLOG_D("stop stream");
			stop_stream(stream);
		}	
	}
}






static void
disconnect(struct conn *c)
{
	DMCLOG_D("Disconnecting %d", c->rem.chan.sock);
	
	if (c->loc.io_class != NULL && c->loc.io_class->close != NULL)
		c->loc.io_class->close(&c->loc);
	if(c->error == 0&&(c->cmd == FN_FILE_UPLOAD|| c->cmd == FN_FILE_GET_BACKUP_FILE || c->cmd == FN_ROUTER_SET_UPLOAD_FIRMWARE))
	{
		//if(c->rem.io.total == c->rem.content_len)
		{
			_bfavfs_fclose(c->record_fd,c->token);
			del_record_from_list_for_index(&c->dn,c->length);
			if(c->dn == NULL)
			{
				//文件upload完毕,删除配置文件并将临时文件名更改为目标文件名
				DMCLOG_D("tmp_path = %s,des path = %s",c->tmp_path,c->des_path);
				BucketObject *sObject = build_bucket_object(c->tmp_path,c->token);
				assert(sObject != NULL);

				BucketObject *dObject = build_bucket_object(c->des_path,c->token);
				assert(dObject != NULL);
				
				_bfavfs_frename(sObject,dObject,c->token);
				safe_free(sObject);
				safe_free(dObject);
				BucketObject *cObject = build_bucket_object(c->cfg_path,c->token);
				assert(cObject != NULL);

				_bfavfs_remove(cObject,c->token);
				safe_free(cObject);
				//bfavfs_fsetattr(c->des_path,c->token);
				sync();
			}
		}
	}
		
	if(c->error == 0)
	{
		if(c->cmd == FN_FILE_RENAME\
			||c->cmd == FN_FILE_UPLOAD\
			|| c->cmd == FN_FILE_DELETE\
			|| c->cmd == FN_FILE_COPY\
			|| c->cmd == FN_FILE_MOVE\
			|| c->cmd == FN_FILE_GET_BACKUP_FILE\
			|| c->cmd == FN_FILE_DEL_LIST_BY_PATH)
		{
			_check_ip_to_list(c->client_ip);
			uint16_t sign = get_database_sign();
			sign++;
			set_database_sign(sign);
			dm_cycle_change_inotify(data_base_changed);
		}
	}
	if (c->rem.io_class != NULL)
		c->rem.io_class->close(&c->rem);
	EnterCriticalSection(&c->ctx->mutex);
	LL_DEL(&c->link);
	c->ctx->nactive--;
	assert(c->ctx->nactive >= 0);
	LeaveCriticalSection(&c->ctx->mutex);
	
	safe_free(c->cfg_path);
	safe_free(c->tmp_path);
	safe_free(c->src_path);
	safe_free(c->des_path);
	safe_free(c->search_string);
	safe_free(c);
}

static void
add_to_set(int fd, fd_set *set, int *max_fd)
{
	FD_SET(fd, set);
	if (fd > *max_fd)
		*max_fd = fd;
}

/*
 * One iteration of server loop. This is the core of the data exchange.
 */
void
shttpd_poll(struct shttpd_ctx *ctx, int milliseconds)
{
	struct llhead	*lp, *tmp;
	struct listener	*l;
	struct conn	*c;
	struct timeval	tv;			/* Timeout for select() */
	fd_set		read_set, write_set;
	int		sock, max_fd = -1, msec = milliseconds;
	struct usa	sa;
	//time_t now_time = 0;	
	int now_time = 0;
	struct  sysinfo info;  

	FD_ZERO(&read_set);
	FD_ZERO(&write_set);

	/* Add listening sockets to the read set */
	LL_FOREACH(&listeners, lp) {
		l = LL_ENTRY(lp, struct listener, link);
		FD_SET(l->sock, &read_set);
		if (l->sock > max_fd)
			max_fd = l->sock;
	}
	/* Multiplex streams */
	LL_FOREACH(&ctx->connections, lp) {
		c = LL_ENTRY(lp, struct conn, link);
		
		/* If there is a space in remote IO, check remote socket */
		if (io_space_len(&c->rem.io))
			add_to_set(c->rem.chan.fd, &read_set, &max_fd);

		/*
		 * If there is some data read from local endpoint, check the
		 * remote socket for write availability
		 */
		if (io_data_len(&c->loc.io))
			add_to_set(c->rem.chan.fd, &write_set, &max_fd);

		if (io_space_len(&c->loc.io) && (c->loc.flags & FLAG_R) &&
		    (c->loc.flags & FLAG_ALWAYS_READY))
			msec = 0;
		
		if (io_data_len(&c->rem.io) && (c->loc.flags & FLAG_W) &&
		    (c->loc.flags & FLAG_ALWAYS_READY))
			msec = 0;
	}
	tv.tv_sec = msec / 1000;
	tv.tv_usec = msec % 1000;

	/* Check IO readiness */
	if (select(max_fd + 1, &read_set, &write_set, NULL, &tv) < 0) {
		DMCLOG_D("select: %d", ERRNO);
		return;
	}

	/* Check for incoming connections on listener sockets */
	LL_FOREACH(&listeners, lp) {
		l = LL_ENTRY(lp, struct listener, link);
		if (!FD_ISSET(l->sock, &read_set))
			continue;
		do {

			sa.len = sizeof(sa.u.sin);
			if ((sock = accept(l->sock, &sa.u.sa, &sa.len)) != -1) {
#if defined(_WIN32)
				shttpd_add_socket(ctx, sock);
#else
				if (sock < (int) FD_SETSIZE) {
					shttpd_add_socket(ctx, sock);
				} else {
					DMCLOG_D("shttpd_poll: ctx %p: disarding ""socket %d, too busy", ctx, sock);
					(void) closesocket(sock);
				}
#endif /* _WIN32 */
			}	
		} while (sock != -1);
	}
	
	/* Process all connections */
	LL_FOREACH_SAFE(&ctx->connections, lp, tmp) {
		c = LL_ENTRY(lp, struct conn, link);

		/* Read from remote end if it is ready */
		if (FD_ISSET(c->rem.chan.fd, &read_set) &&
		    io_space_len(&c->rem.io))
		{
			/* If the request is not parsed yet, do so */
			if (!(c->rem.flags & FLAG_HEADERS_PARSED))
			{
				msg_read_stream(&c->rem);
				parse_http_request(c);
			}else{
				read_stream(&c->rem);
			}
		}
		/* Read from the local end if it is ready */
		if (io_space_len(&c->loc.io) &&
		    ((c->loc.flags & FLAG_ALWAYS_READY)
		    ))
			read_stream(&c->loc);
		
		if (io_data_len(&c->rem.io) > 0 && (c->loc.flags & FLAG_W) &&
		    c->loc.io_class != NULL && c->loc.io_class->write != NULL)
		{				
			write_stream(&c->rem, &c->loc);	
			#if 0
			sysinfo(&info); 
			now_time = info.uptime;
			if(now_time - c->record_time > 0)
			{
				c->record_time = now_time + EXPIRE_PER_SEC;
				off_t cur_position = c->offset + c->rem.io.total;
				update_record_for_index(&c->dn,c->length,cur_position);
				write_list_to_stream(c->record_fd,c->dn);//写入数据
			}
			#endif
		}	
		if (io_data_len(&c->loc.io) > 0 && c->rem.io_class != NULL)
			write_stream(&c->loc, &c->rem); 
		/* Check whether we should close this connection */
		if ((c->rem.flags & FLAG_CLOSED) ||
		    ((c->loc.flags & FLAG_CLOSED) && !io_data_len(&c->loc.io)))
			disconnect(c);
	}
}

/*
 * Deallocate shttpd object, free up the resources
 */
void
shttpd_fini(struct shttpd_ctx *ctx)
{
	struct llhead		*lp, *tmp;
	struct conn		*c;
	struct listener		*l;
	/* Free all connections */
	LL_FOREACH_SAFE(&ctx->connections, lp, tmp) {
		c = LL_ENTRY(lp, struct conn, link);
		disconnect(c);
	}
	
	/* Free listener sockets for this context */
	LL_FOREACH_SAFE(&listeners, lp, tmp) {
		l = LL_ENTRY(lp, struct listener, link);
		(void) closesocket(l->sock);
		LL_DEL(&l->link);
		free(l);
	}
	DestroyCriticalSection(&ctx->mutex);
	_destory_dev_list();
#ifdef TOKEN_MANAGE
	dm_token_destroy(ctx);
#endif
	xxDestoryPinyinDB();
	media_list_destroy();
	media_prc_thpool_destroy();
	rfsvfs_destroy();
	/* TODO: free SSL context */
	free(ctx);
}

void
open_listening_ports(struct shttpd_ctx *ctx)
{
	if (shttpd_listen(ctx,get_sys_init_port()) == -1)
		DMCLOG_D("Cannot open socket on port %d",get_sys_init_port());
}



