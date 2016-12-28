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
	struct stat	st;
	struct stream	*rem = &stream->conn->rem;
	int		n, fd = stream->chan.fd;

	assert(fd != -1);
	n = write(fd, buf, len);

	//DMCLOG_D("put_file(%p, %d): %d bytes", (void *) stream, len, n);

	if (n <= 0) {
		stop_stream(stream);
	}

	return (n);
}

static int
read_file(struct stream *stream, void *buf, size_t len)
{
	int n = 0;
	assert(stream->chan.fd != -1);
	n = read(stream->chan.fd, buf, len);
	return n;
}

static void
close_file(struct stream *stream)
{
	assert(stream->chan.fd != -1);
	(void) close(stream->chan.fd);
}

void
get_file(struct conn *c, struct stat *stp)
{
	ENTER_FUNC();
	big_int_t	cl; /* Content-Length */
	cl = (big_int_t) stp->st_size;
	DMCLOG_D("cl = %lu,stp->st_size = %lu,c->offset = %d",cl,stp->st_size,c->offset);
	/* If Range: header specified, act accordingly */
	(void) lseek(c->loc.chan.fd, c->offset, SEEK_SET);
	if(c->length <= 1)
	{
		cl = cl - c->offset;
	}else{
		cl = c->length - c->offset;
	}
	c->loc.content_len = cl;
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
