/*
 * Copyright (c) 2004-2005 Sergey Lyubka <valenok@gmail.com>
 * All rights reserved
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Sergey Lyubka wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "camera.h"
#include "defs.h"
#include "base.h"
#include <stdlib.h>
static int
write_file(struct stream *stream, const void *buf, size_t len)
{
	return 0;
}

static int
read_file(struct stream *stream, void *buf, size_t len)
{
	int n = 0;
	//assert(stream->chan.fd != -1);
	if (stream->thumb_data != NULL) {//get thumb
		memcpy(buf, stream->thumb_data+stream->offset, len);
		stream->offset+=len;
		return len;
	}
	n = cam_read_file(stream->path, (unsigned long long)stream->offset, (unsigned long long)len, buf);
	if (n > 0)
		stream->offset+=n;
//	n = read(stream->chan.fd, buf, len);
	return n;
}


static void
close_file(struct stream *stream)
{
	safe_free(stream->path);
	safe_free(stream->thumb_data);
	return ;
	assert(stream->chan.fd != -1);
	(void) close(stream->chan.fd);
}

int
cam_get_file(struct conn *c, char* path)
{
	ENTER_FUNC();
	big_int_t	cl = 0; /* Content-Length */
	if (c->loc.io_class == NULL) {//first time in
		asprintf(&c->loc.path, "%s", path);
		int ret = cl = cam_get_file_len(path);
		if (ret < 0) {
			return -1;
		}
	}
	/* If Range: header specified, act accordingly */
	c->loc.offset = c->offset;
	if(c->length <= 1)
	{
		cl = cl - c->offset;
	}else{
		cl = c->length - c->offset;
	}
	c->loc.content_len = cl;
	c->loc.io_class = &io_cam_file;
	c->loc.flags |= FLAG_R | FLAG_ALWAYS_READY;
	return 0;
	EXIT_FUNC();
}

int
io_cam_get_thumb(struct conn *c, char* path)
{
	ENTER_FUNC();
	big_int_t	cl = 0; /* Content-Length */
	if (c->loc.io_class == NULL) {//first time in
		asprintf(&c->loc.path, "%s", path);
		int ret = cl = cam_get_thumb_file(path, &c->loc.thumb_data);
		if (ret < 0) {
			return -1;
		}
	}
	/* If Range: header specified, act accordingly */
	c->loc.offset = c->offset;
	if(c->length <= 1)
	{
		cl = cl - c->offset;
	}else{
		cl = c->length - c->offset;
	}
	c->loc.content_len = cl;
	c->loc.io_class = &io_cam_file;
	c->loc.flags |= FLAG_R | FLAG_ALWAYS_READY;
	return 0;
	EXIT_FUNC();
}

const struct io_class	io_cam_file =  {
	"cam_file",
	read_file,
	write_file,
	close_file
};


