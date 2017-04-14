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

#include "defs.h"
#include "file_json.h"
#include "base.h"

/*
 * Parse date-time string, and return the corresponding time_t value
 */
 #if 0
static time_t
date_to_epoch(const char *s)
{
	struct tm	tm, *tmp;
	char		mon[32];
	int		sec, min, hour, mday, month, year;

	(void) memset(&tm, 0, sizeof(tm));
	sec = min = hour = mday = month = year = 0;

	if (((sscanf(s, "%d/%3s/%d %d:%d:%d",
	    &mday, mon, &year, &hour, &min, &sec) == 6) ||
	    (sscanf(s, "%d %3s %d %d:%d:%d",
	    &mday, mon, &year, &hour, &min, &sec) == 6) ||
	    (sscanf(s, "%*3s, %d %3s %d %d:%d:%d",
	    &mday, mon, &year, &hour, &min, &sec) == 6) ||
	    (sscanf(s, "%d-%3s-%d %d:%d:%d",
	    &mday, mon, &year, &hour, &min, &sec) == 6)) &&
	    (month = montoi(mon)) != -1) {
		tm.tm_mday	= mday;
		tm.tm_mon	= month;
		tm.tm_year	= year;
		tm.tm_hour	= hour;
		tm.tm_min	= min;
		tm.tm_sec	= sec;
	}

	if (tm.tm_year > 1900)
		tm.tm_year -= 1900;
	else if (tm.tm_year < 70)
		tm.tm_year += 100;

	/* Set Daylight Saving Time field */
	tmp = localtime(&current_time);
	tm.tm_isdst = tmp->tm_isdst;

	return (mktime(&tm));
}
#endif
/*
 * This structure tells how HTTP headers must be parsed.
 * Used by parse_headers() function.
 */
#define	OFFSET(x)	offsetof(struct headers, x)
static const struct http_header http_headers[] = {
	{16, HDR_INT,	 OFFSET(cl),		"Content-Length: "	},
	{14, HDR_STRING, OFFSET(ct),		"Content-Type: "	},
	{12, HDR_STRING, OFFSET(useragent),	"User-Agent: "		},
	{19, HDR_DATE,	 OFFSET(ims),		"If-Modified-Since: "	},
	{15, HDR_STRING, OFFSET(auth),		"Authorization: "	},
	{9,  HDR_STRING, OFFSET(referer),	"Referer: "		},
	{8,  HDR_STRING, OFFSET(cookie),	"Cookie: "		},
	{10, HDR_STRING, OFFSET(location),	"Location: "		},
	{8,  HDR_INT,	 OFFSET(status),	"Status: "		},
	{7,  HDR_STRING, OFFSET(range),		"Range: "		},
	{12, HDR_STRING, OFFSET(connection),	"Connection: "		},
	{19, HDR_STRING, OFFSET(transenc),	"Transfer-Encoding: "	},
	{0,  HDR_INT,	 0,			NULL			}
};

void
parse_headers(const char *s, int len, struct headers *parsed)
{
	const struct http_header	*h;
	union variant			*v;
	const char			*p, *e = s + len;

	//p_debug("parsing headers (len %d): [%.*s]", len, len, s);

	/* Loop through all headers in the request */
	while (s < e) {

		/* Find where this header ends */
		for (p = s; p < e && *p != '\n'; ) p++;

		/* Is this header known to us ? */
		for (h = http_headers; h->len != 0; h++)
			if (e - s > h->len &&
			    !my_strncasecmp(s, h->name, h->len))
				break;

		/* If the header is known to us, store its value */
		if (h->len != 0) {

			/* Shift to where value starts */
			s += h->len;

			/* Find place to store the value */
			v = (union variant *) ((char *) parsed + h->offset);

			/* Fetch header value into the connection structure */
			if (h->type == HDR_STRING) {
				v->v_vec.ptr = s;
				v->v_vec.len = p - s;
				if (p[-1] == '\r' && v->v_vec.len > 0)
					v->v_vec.len--;
			} else if (h->type == HDR_INT) {
				v->v_big_int = strtoul(s, NULL, 10);
			} else if (h->type == HDR_DATE) {
				//v->v_time = date_to_epoch(s);
			}
		}

		s = p + 1;	/* Shift to the next header */
	}
}

int file_json_to_string(struct conn *c,JObj* response_json)
{
	char *response_str = JSON_TO_STRING(response_json);
	if(response_str == NULL)
	{
		return -1;
	}
	c->loc.io.head = c->loc.headers_len = my_snprintf(c->loc.io.buf,c->loc.io.size,"%s", response_str);
	if(c->loc.io.head == 0)
	{
		return -1;
	}	
	p_debug("c->loc.io.buf = %s,c->loc.io.size = %d",c->loc.io.buf,c->loc.io.size);
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
	if (stream->io_class != NULL && stream->io_class->close != NULL)
		stream->io_class->close(stream);

	stream->io_class= NULL;
	stream->flags |= FLAG_CLOSED;
	stream->flags &= ~(FLAG_R | FLAG_W | FLAG_ALWAYS_READY);

	p_debug("%d %s stopped. %lu of content data, %d now in a buffer",
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


/*
 * Send error message back to a client.
 */
void 
dm_send_file_server(struct conn *c)
{
	int ret = 0;
	io_clear(&c->loc.io);
	file_process(c);

}


static void
parse_http_request(struct conn *c)
{
	int res = 0;
	//char	*s;
	//s = c->rem.io.buf;

	//p_debug("Conn %d: parsing request: %s", c->rem.chan.sock, c->rem.io.buf);
	p_debug("access parse_http_request");

	res=file_parse_process(c);
	if(res<0)
		{
		return res;
	}
	//p_debug("c->loc.io.buf = %s",c->loc.io.buf);	
	
	c->rem.flags |= FLAG_HEADERS_PARSED;
	dm_send_file_server(c);
}

void
shttpd_add_socket(struct shttpd_ctx *ctx, int sock)
{
	struct conn	*c;
	struct usa	sa;

	sa.len = sizeof(sa.u.sin);
	(void) set_non_blocking_mode(sock);

	if (getpeername(sock, &sa.u.sa, &sa.len)) {
		p_debug("add_socket: %s", strerror(errno));
	} else if ((c = calloc(1, sizeof(*c) + 2 * ctx->io_buf_size)) == NULL) {
		(void) closesocket(sock);
		p_debug("add_socket: calloc: %s", strerror(ERRNO));
	} else {
		p_debug("add socket success");
		c->rem.conn = c->loc.conn = c;
		c->ctx		= ctx;
		c->sa		= sa;
		strcpy(c->client_ip, inet_ntoa(sa.u.sin.sin_addr));
		p_debug("ctx->io_buf_size = %d,c->client_ip = %s",ctx->io_buf_size,c->client_ip);
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
		int len = io_space_len(&c->rem.io);
		assert(len > 0);
		EnterCriticalSection(&ctx->mutex);
		LL_TAIL(&ctx->connections, &c->link);
		ctx->nactive++;
		LeaveCriticalSection(&ctx->mutex);
		
		p_debug("%s:%hu connected (socket %d)",
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
		p_debug("cannot open port %d", port);
	} else if ((l = calloc(1, sizeof(*l))) == NULL) {
		(void) closesocket(sock);
		p_debug("cannot allocate listener");
	} else {
		l->sock	= sock;
		l->ctx	= ctx;
		LL_TAIL(&listeners, &l->link);
		p_debug("shttpd_listen: added socket %d", sock);
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
		io_inc_head(&stream->io, n);
	else if (n == -1 && (ERRNO == EINTR || ERRNO == EWOULDBLOCK))
		n = n;	/* Ignore EINTR and EAGAIN */
	else if (!(stream->flags & FLAG_DONT_CLOSE))
		stop_stream(stream);

	/*p_debug("read_stream (%d %s): read %d/%d/%lu bytes (errno %d)",
	    stream->conn->rem.chan.sock,
	    stream->io_class ? stream->io_class->name : "(null)",
	    n, len, (unsigned long) stream->io.total, ERRNO);*/

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
		io_inc_head(&stream->io, n);
		io_inc_tail(&stream->io, n);
	}
	else if (n <= 0 && (ERRNO == EINTR || ERRNO == EWOULDBLOCK))
	{
		p_debug("stop stream");
		stop_stream(stream);
	}
		

	p_debug("msg read_stream (%d %s): read %d/%d/%lu bytes (errno %d)",
	    stream->conn->rem.chan.sock,
	    stream->io_class ? stream->io_class->name : "(null)",
	    n, len, (unsigned long) stream->io.total, ERRNO);
	
	if (stream->content_len > 0 && stream == &stream->conn->loc) {
		assert(stream->io.total <= stream->content_len);
		if (stream->io.total == stream->content_len)
		{
			p_debug("stop stream");
			stop_stream(stream);
		}	
	}
}


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
		to->io.total += n;
	}
	else if (n == -1 && (ERRNO == EINTR || ERRNO == EWOULDBLOCK))
	{
		n = n;	/* Ignore EINTR and EAGAIN */
		stop_stream(to);
	}
	else if (!(to->flags & FLAG_DONT_CLOSE))
		stop_stream(to);
	p_debug("write_stream (%d %s): written %d/%d/%lu bytes (errno %d)\n",
	    to->conn->rem.chan.sock,
	    to->io_class ? to->io_class->name : "(null)", n, len,(unsigned long) to->io.total, ERRNO);
}

static void
msg_write_stream(struct stream *from, struct stream *to)
{
	//p_debug("from->io.buf = %s", from->io.buf);
	int	n, len;
	len = io_data_len(&from->io);
	assert(len > 0);
	/* TODO: should be assert on CAN_WRITE flag */
	n = msg_write_socket(to, io_data(&from->io), len);
	p_debug("write_stream (%d %s): written %d/%d bytes (errno %d)",
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



static void
disconnect(struct conn *c)
{
	ENTER_FUNC();
	static const struct vec	ka = {"keep-alive", 10};
	int			dont_close;
	int rc;
	char		path[URI_MAX] = {0};

	p_debug("Disconnecting %d (%.*s)", c->rem.chan.sock,
	    c->ch.connection.v_vec.len, c->ch.connection.v_vec.ptr);


	if (c->loc.io_class != NULL && c->loc.io_class->close != NULL)
		c->loc.io_class->close(&c->loc);

	/*
	 * Check the "Connection: " header before we free c->request
	 * If it its 'keep-alive', then do not close the connection
	 */
	dont_close =  c->ch.connection.v_vec.len >= ka.len &&
	    !my_strncasecmp(ka.ptr, c->ch.connection.v_vec.ptr, ka.len);

	if (c->request)
		safe_free(c->request);

	/* Handle Keep-Alive */
	dont_close = 0;
	if (dont_close) {
		c->loc.io_class = NULL;
		c->loc.flags = c->rem.flags = 0;
		(void) memset(&c->ch, 0, sizeof(c->ch));
		io_clear(&c->rem.io);
		io_clear(&c->loc.io);
		c->rem.io.total = c->loc.io.total = 0;
	} else {
		if (c->rem.io_class != NULL)
			c->rem.io_class->close(&c->rem);

		EnterCriticalSection(&c->ctx->mutex);
		LL_DEL(&c->link);
		c->ctx->nactive--;
		assert(c->ctx->nactive >= 0);
		LeaveCriticalSection(&c->ctx->mutex);
		safe_free(c);
	}
	EXIT_FUNC();
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
	//ENTER_FUNC();
	struct llhead	*lp, *tmp;
	struct listener	*l;
	struct conn	*c;
	struct timeval	tv;			/* Timeout for select() */
	fd_set		read_set, write_set;
	int		sock, max_fd = -1, msec = milliseconds;
	struct usa	sa;

	FD_ZERO(&read_set);
	FD_ZERO(&write_set);

	/* Add listening sockets to the read set */
	LL_FOREACH(&listeners, lp) {
		l = LL_ENTRY(lp, struct listener, link);
		FD_SET(l->sock, &read_set);
		if (l->sock > max_fd)
			max_fd = l->sock;
		//p_debug("FD_SET(%d) (listening)", l->sock);
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
		p_debug("select: %d", ERRNO);
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
				p_debug("sock = %d",sock);
#if defined(_WIN32)
				shttpd_add_socket(ctx, sock);
#else
				if (sock < (int) FD_SETSIZE) {
					shttpd_add_socket(ctx, sock);
				} else {
					p_debug("shttpd_poll: ctx %p: disarding ""socket %d, too busy", ctx, sock);
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
				//p_debug("c->loc.io.buf = %s", c->loc.io.buf);
				if(!(c->rem.flags & FLAG_CLOSED))
				{
					msg_write_stream(&c->loc, &c->rem);
				}
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
		}	
		if (io_data_len(&c->loc.io) > 0 && c->rem.io_class != NULL)
			write_stream(&c->loc, &c->rem); 
		if (c->rem.nread_last > 0)
			c->ctx->in += c->rem.nread_last;
		if (c->loc.nread_last > 0)
			c->ctx->out += c->loc.nread_last;
		/* Check whether we should close this connection */
		if ((c->rem.flags & FLAG_CLOSED) ||
		    ((c->loc.flags & FLAG_CLOSED) && !io_data_len(&c->loc.io)))
			disconnect(c);
		#if 0
		struct timeval tnow;
		gettimeofday(&tnow, NULL);
		unsigned int t_sec = (unsigned int)tnow.tv_sec - (unsigned int)c->tstart.tv_sec;
		int t_usec = (unsigned int)tnow.tv_usec/1000;
		if(t_sec/EXPIRE_PER_SEC > 0&&t_sec%EXPIRE_PER_SEC == 0&&t_usec%10 == 0)
		{
			p_debug("access2 t = %lu",t_sec);
			int ret = 0;
			EnterCriticalSection(&c->ctx->mutex);
			//extern struct task_dnode *task_dn;
			//write_list_to_file(CFG_PATH,task_dn);//保存任务列表
			p_debug("access3 t = %lu",t_sec);

			sync();
						p_debug("access4 t = %lu",t_sec);
			LeaveCriticalSection(&c->ctx->mutex);
						p_debug("access5 t = %lu",t_sec);
		}
					p_debug("access6 t = %lu",t_sec);
		#endif
	}
	//EXIT_FUNC();
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
	extern struct task_dnode *task_dn;
	destory_task_list(task_dn);
	/* TODO: free SSL context */
	free(ctx);
}

void
open_listening_ports(struct shttpd_ctx *ctx)
{
	if (shttpd_listen(ctx, INIT_PORT) == -1)
		p_debug("Cannot open socket on port %d",INIT_PORT);
}
