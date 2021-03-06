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

/*
 * For a given PUT path, create all intermediate subdirectories
 * for given path. Return 0 if the path itself is a directory,
 * or -1 on error, 1 if OK.
 */
int
put_dir(const char *path)
{
	char		buf[FILENAME_MAX];
	const char	*s, *p;
	struct stat	st;
	size_t		len;

	for (s = p = path + 2; (p = strchr(s, '/')) != NULL; s = ++p) {
		len = p - path;
		assert(len < sizeof(buf));
		(void) memcpy(buf, path, len);
		buf[len] = '\0';

		/* Try to create intermediate directory */
		if (my_stat(buf, &st) == -1 && my_mkdir(buf, 0755) != 0)
			return (-1);

		/* Is path itself a directory ? */
		if (p[1] == '\0')
			return (0);
	}

	return (1);
}

static int
read_dir(struct stream *stream, void *buf, size_t len)
{
	struct dirent	*dp = NULL;
	char		file[FILENAME_MAX], line[FILENAME_MAX + 512],
				size[64], mod[64];
	struct stat	st;
	struct conn	*c = stream->conn;
	int		n, nwritten = 0;
	const char	*slash = "";
	char img_src[1024];
	DBG(("access read_dir\n"));
	assert(stream->chan.dir.dirp != NULL);
	//assert(stream->conn->uri[0] != '\0');
	do {
		if (len < sizeof(line))
		{
			printf("len : %u\nline len:%u\n",len,sizeof(line));
			break;
		}
		if ((dp = readdir(stream->chan.dir.dirp)) == NULL) {
			stream->flags |= FLAG_CLOSED;
			n = my_snprintf(line, sizeof(line),"<tr><th colspan=\"4\"><hr></th></tr></table>");
			(void) memcpy(buf, line, n);
			buf = (char *) buf + n;
			nwritten += n;
			len -= n;
			char path_buf[128];
			memset(path_buf, 0, sizeof(path_buf));
			sprintf(path_buf,"%s/%s", c->ctx->document_root, FOOTER_FILE);
			//printf("path_buf = %s\n",path_buf);
			FILE *fp = NULL;
		    fp = fopen(path_buf, "r");
			if(fp == NULL){
				printf("open fail\n");
				if (NULL != c->uri){
					free(c->uri);
				}
				send_server_error(c, 404, "Not Found");
				return nwritten;
			}
		    fseek( fp , 0 , SEEK_END );
		    int file_size;
		    file_size = ftell( fp );
		    DBG(( "file_size:%d\n" , file_size ));
		    fseek( fp , 0 , SEEK_SET);
		    fread( buf , file_size , sizeof(char) , fp);
			nwritten += file_size;
			len -= file_size;
			DBG(("end read_dir\n"));
			break;
		}
		DBG(("read_dir: %s", dp->d_name));

		/* Do not show current dir and passwords file */
		if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0 ||
		   strcmp(dp->d_name, HTPASSWD) == 0 || strcmp(dp->d_name, DM_DB_FILE) == 0 || 
		   strcmp(dp->d_name, DM_UUID_FILE) == 0)
			continue;

		(void) my_snprintf(file, sizeof(file),
		    "%s%s%s", stream->chan.dir.path, slash, dp->d_name);
		(void) my_stat(file, &st);
		if (S_ISDIR(st.st_mode)) {
			my_snprintf(size,sizeof(size),"%s","&lt;DIR&gt;");
		} else {
			if (st.st_size < 1024)
				(void) my_snprintf(size, sizeof(size),
				    "%lu", (unsigned long) st.st_size);
			else if (st.st_size < 1024 * 1024)
				(void) my_snprintf(size, sizeof(size), "%luk",
				    (unsigned long) (st.st_size >> 10)  + 1);
			else
				(void) my_snprintf(size, sizeof(size),
				    "%.1fM", (float) st.st_size / 1048576);
		}
		(void) strftime(mod, sizeof(mod), "%d-%b-%Y %H:%M",
			localtime(&st.st_mtime));
		memset(img_src,0,1024);
		if(S_ISDIR(st.st_mode))
		{
			sprintf(img_src,"/icons/48x48/folder.png");
		}else{
			int file_type = dm_get_mime_type(dp->d_name,strlen(dp->d_name));
			if(file_type == 0)//doc
			{
				sprintf(img_src,"/icons/48x48/doc.png");
			}else if(file_type == 1)//video
			{
				sprintf(img_src,"/icons/48x48/video.png");
			}else if(file_type == 2)//audio
			{
				sprintf(img_src,"/icons/48x48/audio.png");
			}else if(file_type == 3)//picture
			{
				sprintf(img_src,"/icons/48x48/image.png");
			}else//unknow
			{
				sprintf(img_src,"/icons/48x48/unknown.png");
			}
		}
		
	    n = my_snprintf(line, sizeof(line),
	    "<tr><td valign=\"top\"><a href=\"%s\"><img src=\"%s\" alt=\"[%s]\"></a></td>"
	    "<td><a href=\"%s\">%s</a></td><td align=\"right\">%s</td><td align=\"right\">%s</td></tr>\n",
	    dp->d_name,img_src,S_ISDIR(st.st_mode) ? "DIR" : " ",dp->d_name,dp->d_name, mod, size);
		(void) memcpy(buf, line, n);
		buf = (char *) buf + n;
		nwritten += n;
		len -= n;
	} while (dp != NULL);
	return (nwritten);
}


static void
close_dir(struct stream *stream)
{
	assert(stream->chan.dir.dirp != NULL);
	assert(stream->chan.dir.path != NULL);
	(void) closedir(stream->chan.dir.dirp);
	free(stream->chan.dir.path);
}
void
get_header_file(struct conn *c)
{
	char		date[64], lm[64], etag[64], range[64] = "";
	size_t		n, status = 200;
	unsigned long	r1, r2;
	const char	*fmt = "%a, %d %b %Y %H:%M:%S GMT", *msg = "OK";
	big_int_t	cl; /* Content-Length */
	struct stat	stp;
	char path_buf[64];
	memset(path_buf, 0, sizeof(path_buf));
	sprintf(path_buf,"%s/%s", c->ctx->document_root, HEADER_FILE);
	DBG(("path_buf = %s\n",path_buf));
	if (my_stat(path_buf, &stp) != 0){
		printf("no stat\n");
		return;
	}
	cl = (big_int_t) stp.st_size;
	c->status = status;
	c->loc.content_len = cl;
	c->loc.io_class = &io_file;
	c->loc.flags |= FLAG_R | FLAG_ALWAYS_READY;

	if (c->method == METHOD_HEAD)
		stop_stream(&c->loc);
	/*
	 * We do not do io_inc_head here, because it will increase 'total'
	 * member in io. We want 'total' to be equal to the content size,
	 * and exclude the headers length from it.
	 */
	c->loc.io.head = c->loc.headers_len = my_snprintf(c->loc.io.buf,
	    c->loc.io.size,
	     "HTTP/1.1 200 OK\r\n"
		    "Content-Type: text/html; charset=utf-8\r\n\r\n");

	if (c->method == METHOD_HEAD)
		stop_stream(&c->loc);
}

void
get_dir(struct conn *c)
{
	DBG(("c->loc.chan.dir.path = %s\n",c->loc.chan.dir.path));
	get_header_file(c);
	/*if ((c->loc.chan.dir.dirp = opendir(c->loc.chan.dir.path)) == NULL) {
		(void) free(c->loc.chan.dir.path);
		send_server_error(c, 500, "Cannot open directory");
	} else {
		c->loc.io.head = my_snprintf(c->loc.io.buf, c->loc.io.size,
		    "HTTP/1.1 200 OK\r\n"
		    "Content-Type: text/html; charset=utf-8\r\n\r\n"
		    "<html><head><title>Index of %s</title>"
		    "<style>th {text-align: left;}</style></head>"
		    "<body><h1>Index of %s</h1><pre><table cellpadding=\"0\">"
		    "<tr><th>Name</th><th>Modified</th><th>Size</th></tr>"
		    "<tr><td colspan=\"3\"><hr></td></tr>",
		    c->uri, c->uri);
		io_clear(&c->rem.io);
		c->status = 200;
		c->loc.io_class = &io_dir;
		c->loc.flags |= FLAG_R | FLAG_ALWAYS_READY;
	}*/
}

const struct io_class	io_dir =  {
	"dir",
	read_dir,
	NULL,
	close_dir
};
