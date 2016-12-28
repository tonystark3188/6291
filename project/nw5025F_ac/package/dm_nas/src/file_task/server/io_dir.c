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


static int
read_dir(struct stream *stream, void *buf, size_t len)
{
	struct dirent	*dp = NULL;
	char		file[DM_FILENAME_MAX], line[DM_FILENAME_MAX + 512];
	struct stat	st;
	struct conn	*c = stream->conn;
	int		n = 0, nwritten = 0;
	const char	*slash = "/";
	assert(stream->chan.dir.dirp != NULL);
	//assert(stream->conn->uri[0] != '\0');

	do {
		if (len < sizeof(line))
		{
			DMCLOG_D("out of stream");
			break;
		}

		if ((dp = readdir(stream->chan.dir.dirp)) == NULL) {
			stream->flags |= FLAG_CLOSED;
			stream->flags &= ~(FLAG_R | FLAG_W | FLAG_ALWAYS_READY);
			n = my_snprintf(line, sizeof(line)," ], \"count\": %u, \"totalCount\": %u },\"header\": { \"cmd\": %d, \"seq\": %d, \"error\": %d }}",
				c->nfiles,c->nfiles,c->cmd,c->seq,c->error);
			(void) memcpy(buf, line, n);
			buf = (char *) buf + n;
			nwritten += n;
			len -= n;
			DMCLOG_D("read finish");
			break;
		}
		
		if (dp->d_name[0] == '.') {
			continue;
		}
		(void) my_snprintf(file, sizeof(file),
		    "%s%s%s", stream->chan.dir.path, slash, dp->d_name);
		(void) my_stat(file, &st);
		if(c->nfiles == 0)
		{
			n = my_snprintf(line, sizeof(line),
		    "{ \"isFolder\": %d, \"size\": %lld, \"data\": %u, \"name\": \"%s\",\"attr\":%d}",
		    S_ISDIR(st.st_mode), st.st_size,st.st_mtime, dp->d_name,0);
		}else{
			n = my_snprintf(line, sizeof(line),
		    ",{ \"isFolder\": %d, \"size\": %lld, \"data\": %u, \"name\": \"%s\",\"attr\":%d}",
		    S_ISDIR(st.st_mode), st.st_size,st.st_mtime, dp->d_name,0);
		}
		
		(void) memcpy(buf, line, n);
		buf = (char *) buf + n;
		nwritten += n;
		len -= n;
		c->nfiles++;
	} while (dp != NULL);
	return (nwritten);
}

static void
close_dir(struct stream *stream)
{
	assert(stream->chan.dir.dirp != NULL);
	assert(stream->chan.dir.path != NULL);
	(void) closedir(stream->chan.dir.dirp);
	//free(stream->chan.dir.path);
}

void
get_dir(struct conn *c)
{
	if ((c->loc.chan.dir.dirp = opendir(c->loc.chan.dir.path)) == NULL) {
		c->loc.flags |= FLAG_CLOSED;
	} else {
		c->loc.io.head = my_snprintf(c->loc.io.buf, c->loc.io.size,
		    "{ \"data\": { \"filelist\": [");
		io_clear(&c->rem.io);
		c->loc.io_class = &io_dir;
		c->loc.flags |= FLAG_R | FLAG_ALWAYS_READY;
	}
}

const struct io_class	io_dir =  {
	"dir",
	read_dir,
	NULL,
	close_dir
};

