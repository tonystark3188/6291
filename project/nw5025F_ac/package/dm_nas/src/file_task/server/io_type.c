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
#include "file_table.h"
#include "file_json.h"
/*#include <iconv.h>
#include <locale.h>


#define GBK_UNICODE "GBK"


int utf8togb2312(char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen)
{
    iconv_t cd;
    if( (cd = iconv_open(GBK_UNICODE,"UTF-8")) == -1 )
    {
        DMCLOG_E("iconv_open error,errno = %d",errno);
        return -1;
    }
    memset(destbuf,0,destlen);
    char **source = &sourcebuf;
    char **dest = &destbuf;
    DMCLOG_E("cd = %d,sourcebuf = %s,sourcelen = %d",cd,sourcebuf,sourcelen);
    if(-1 == iconv(cd,source,&sourcelen,dest,&destlen))
    {
        DMCLOG_E("iconv error,errno = %d",errno);
        return -1;
    }
    iconv_close(cd);
    return 0;
}*/

static void
close_type(struct stream *stream)
{
	ENTER_FUNC();
	file_info_t *p_file_info = NULL;
	file_info_t *nn = NULL;
	struct conn	*c = stream->conn;
	dl_list_for_each_safe(p_file_info, nn, &c->vlist.head, file_info_t, next)
    {
        safe_free(p_file_info);
    }
	EXIT_FUNC();
}

static int
read_type(struct stream *stream, void *buf, size_t len)
{
	file_info_t *p_file_info = NULL;
	char		line[DM_FILENAME_MAX] = {0};
	char		path[DM_FILENAME_MAX] = {0};
	int		n, nwritten = 0;
	unsigned total_count = 0;
	int i = 0;
	struct conn	*c = stream->conn;
	dl_list_for_each(p_file_info,&c->vlist.head, file_info_t, next)
	{
		if (len < sizeof(line))
		{
			DMCLOG_E("out of mem");
			break;
		}
	
		if(i == c->offset)
		{
			token_dnode_t *token_dnode = (token_dnode_t *)c->token;
			if(token_dnode->isPublicUser == false&&token_dnode->isPublicPath == true)
			{
				sprintf(path,"%s/%s",PUBLIC_PATH,*p_file_info->path == '/'?p_file_info->path + 1:p_file_info->path);
			}else{
				strcpy(path,*p_file_info->path == '/'?p_file_info->path + 1:p_file_info->path);
			}
			
			if(c->offset == 0)
			{
				n = my_snprintf(line, sizeof(line),
			    "{ \"parent_id\":%ld,\"size\": %lld, \"data\": %ld, \"isFolder\": %d, \"type\": %d, \"name\": \"%s\", \"path\": \"%s\", \"file_uuid\": \"%s\"}",
			    p_file_info->parent_id,(long long)p_file_info->size, p_file_info->mtime,p_file_info->isDir, p_file_info->type,p_file_info->name,\
			    path,p_file_info->uuid);
			}else{
				n = my_snprintf(line, sizeof(line),
			    ",{ \"parent_id\":%ld,\"size\": %lld, \"data\": %ld, \"isFolder\": %d, \"type\": %d, \"name\": \"%s\", \"path\": \"%s\", \"file_uuid\": \"%s\"}",
			    p_file_info->parent_id,(long long)p_file_info->size, p_file_info->mtime,p_file_info->isDir, p_file_info->type,p_file_info->name,\
			    path,p_file_info->uuid);
			}
			c->offset++;
			(void) memcpy(buf, line, n);
			buf = (char *) buf + n;
			nwritten += n;
			len -=n;
		}
		i++;
	}
	if(c->offset == c->totalCount)
	{
		if (len < sizeof(line))
		{
			DMCLOG_E("out of mem");
			return nwritten;
		}
		stream->flags |= FLAG_CLOSED;
		stream->flags &= ~(FLAG_R | FLAG_W | FLAG_ALWAYS_READY);
		
		n = my_snprintf(line, sizeof(line)," ], \"count\": %u, \"totalCount\": %u,\"totalSize\": %lu},\"header\": { \"cmd\": %d, \"seq\": %d, \"error\": %d }}",
			c->totalCount,c->totalCount,c->totalSize,c->cmd,c->seq,c->error);
		(void) memcpy(buf, line, n);
		buf = (char *) buf + n;
		nwritten += n;
		len -= n;
	}
	return (nwritten);
}


void
get_type(struct conn *c)
{
	ENTER_FUNC();
	io_clear(&c->rem.io);
	c->loc.io_class = &io_type;
	c->loc.flags |= FLAG_R | FLAG_ALWAYS_READY;
	dl_list_init(&c->vlist.head);
	c->vlist.startIndex = c->offset;
	c->vlist.len = c->length;
	c->vlist.file_type = c->fileType;
	c->vlist.sortType = c->sortType;
	BucketObject *sObject = NULL;
	if(c->cmd == FN_FILE_SEARCH)
	{
		c->vlist.cmd = V_FILE_TABLE_SEARCH_QUERY_LIST;
	}else{
		c->vlist.cmd = V_FILE_TABLE_QUERY_LIST;
	}
	if(c->src_path != NULL)
	{
		sObject = build_bucket_object(c->src_path,c->token);
		if(sObject == NULL)
		{
			DMCLOG_E("buile bucket object error");
			return ;
		}
	}else{
		sObject = (BucketObject *)calloc(1,sizeof(BucketObject));
		assert(sObject != NULL);
		sObject->file_type = c->fileType;
	}
	
	int res = _bfavfs_get_file_list(sObject,&c->vlist,c->token);
	if(res != RET_SUCCESS)
	{
		DMCLOG_D("_handle_get_all_type_file_list_cmd error");
		c->error = ERROR_GET_FILE_LIST;
		c->loc.flags |= FLAG_CLOSED;
		safe_free(sObject);
		return;
	}
	c->totalCount = c->vlist.total;
	c->loc.io.head = my_snprintf(c->loc.io.buf, c->loc.io.size,
	    "{ \"data\": { \"filelist\": [");
	safe_free(sObject);
	EXIT_FUNC();
	return;
}


const struct io_class 	io_type =  {
	"type",
	read_type,
	NULL,
	close_type
};
