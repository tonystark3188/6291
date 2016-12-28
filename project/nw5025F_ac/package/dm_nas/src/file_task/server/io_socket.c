/*
 * Copyright (c) 2004-2005 Sergey Lyubka <valenok@gmail.com>
 * All rights reserved
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Sergey Lyubka wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */

#include "defs.h"
#include "base.h"

static int
read_socket(struct stream *stream, void *buf, size_t len)
{
	int n = 0;
	assert(stream->chan.sock != -1);
	n = recv(stream->chan.sock, buf, len, 0);
	return n;
}
int
msg_read_socket(struct stream *stream, void *buf, size_t len)
{
	int n = 0;
	char *pBuf = NULL;
	int pnTimeOut = 3000;
	assert(stream->chan.sock != -1);
	n = DM_MsgReceive(stream->chan.sock, &pBuf, &pnTimeOut);
	//DMCLOG_D("pBuf count = %d,buf count = %d",strlen(pBuf),sizeof(buf));
	
	if(n > 0)
	{
		strcpy(buf,pBuf);
		safe_free(pBuf);
		//DMCLOG_D("n = %d,buf = %s,len = %d",n,(char *)buf,strlen(buf));
	}
	return n;
}


static int
write_socket(struct stream *stream, const void *buf, size_t len)
{

	int n = 0;
	assert(stream->chan.sock != -1);
	n = send(stream->chan.sock, buf, len, 0);
	//DMCLOG_D("n = %d,buf %X,%X,%X", n,((unsigned char *)buf)[0],((unsigned char *)buf)[1],((unsigned char *)buf)[2]);
	return n;
}
int
msg_write_socket(struct stream *stream, const void *buf, size_t len)
{
	int n = 0;
	assert(stream->chan.sock != -1);
	n = DM_MsgSend(stream->chan.sock, buf, len);
	DMCLOG_D("buf = %s,n = %d,error = %d",buf,n,errno);
	return n;
}

static void
close_socket(struct stream *stream)
{
	assert(stream->chan.sock != -1);
	(void) closesocket(stream->chan.sock);
}

const struct io_class	io_socket =  {
	"socket",
	read_socket,
	write_socket,
	close_socket
};
