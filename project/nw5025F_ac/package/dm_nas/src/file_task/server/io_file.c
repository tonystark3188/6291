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
write_file(struct stream *stream, const void *buf, size_t len)
{	
	stream->conn->ctx->watch_dog_time = 3000000;
	assert(stream->chan.vf != NULL);
	return bfavfs_fwrite(buf,1, len, stream->chan.vf);
}

static int
read_file(struct stream *stream, void *buf, size_t len)
{
	int n = 0;
	assert(stream->chan.vf != NULL);
	n = bfavfs_fread(buf,1, len,stream->chan.vf);
	return n;
}

static void
close_file(struct stream *stream)
{
	assert(stream->chan.fd != -1);
	
	struct conn	*c = stream->conn;
	(void) _bfavfs_fclose(stream->chan.vf,c->token);
}

void
get_file(struct conn *c, struct stat *stp)
{
	ENTER_FUNC();
	big_int_t	cl; /* Content-Length */
	cl = (big_int_t) stp->st_size;
	DMCLOG_D("cl = %lld,stp->st_size = %lld,c->offset = %lld,c->length=%lld,stp->st_mtime = %lu",cl,stp->st_size,c->offset,c->length,stp->st_mtime);
	/* If Range: header specified, act accordingly */
	(void) bfavfs_fseek(c->loc.chan.vf, c->offset, SEEK_SET);
	if(c->length <= 1)
	{
		cl = cl - c->offset;
	}else{
		cl = c->length - c->offset;
	}
	c->modifyTime = stp->st_mtime;
	c->loc.content_len = cl;
	DMCLOG_D("sizeof(big_int_t) = %d, cl = %lld, c->loc.content_len = %lld", sizeof(big_int_t), cl, c->loc.content_len);
	c->loc.io_class = &io_file;
	c->loc.flags |= FLAG_R | FLAG_ALWAYS_READY;
	EXIT_FUNC();
}

const struct io_class	io_file =  {
	"file",
	read_file,
	write_file,
	close_file
};
