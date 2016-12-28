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

/*	fileNum:用户需要的文件个数
	totalCount:指定路径下的文件总数
	length:每次取得文件的个数
*/
/*static int
read_type(struct stream *stream, void *buf, size_t len)
{
	ENTER_FUNC();
	int sort_mode = 3;
	char		line[FILENAME_MAX + 512];
	int		n, nwritten = 0;
	const char	*slash = "/";
	struct conn	*c = stream->conn;
	c->offset = c->pageNum*PAGESIZE;
	c->pageNum++;
	if(c->cmd == 124&&c->fileNum != 0)
	{
		if(c->fileNum > c->totalCount)
		{
			c->fileNum = c->totalCount;
		}
		c->length = c->fileNum;
	}else
	{
		c->fileNum = c->totalCount;
		c->length = PAGESIZE;
	}
	DMCLOG_D("c->offset = %lu,c->length = %lu",c->offset,c->length);
	n = _handle_get_file_list_cmd(sort_mode,&buf,&len,c);
	if(n < 0)
	{
		c->error = ERROR_GET_FILE_LIST;
		stream->flags |= FLAG_CLOSED;
	}
	nwritten += n;
	//len -= n;
	DMCLOG_D("nfiles = %u,totalCount = %u,fileNum = %u",c->nfiles,c->totalCount,c->fileNum);
	if(c->nfiles == c->fileNum)
	{
		stream->flags |= FLAG_CLOSED;
		n = my_snprintf(line, sizeof(line)," ], \"count\": %u, \"totalCount\": %u, \"totalPage\": %d, \"pageSize\": %d, \"totalSize\": %lu},\"header\": { \"cmd\": %d, \"seq\": %d, \"error\": %d }}",
			c->totalCount,c->totalCount,1,c->totalCount,c->totalSize,c->cmd,c->seq,c->error);
		(void) memcpy(buf, line, n);
		buf = (char *) buf + n;
		nwritten += n;
		len -= n;
	}
	EXIT_FUNC();
	return (nwritten);
}*/

static int
read_type(struct stream *stream, void *buf, size_t len)
{
	ENTER_FUNC();
	int sort_mode = 3;
	char		line[FILENAME_MAX + 512];
	int		n, nwritten = 0;
	const char	*slash = "/";
	struct conn	*c = stream->conn;
	c->length = c->totalCount - c->offset;
	DMCLOG_D("c->offset = %lu,c->length = %lu",c->offset,c->length);
	/*query the file list by type*/
	n = _handle_get_file_list_cmd(sort_mode,&buf,&len,c);
	if(n < 0)
	{
		c->error = ERROR_GET_FILE_LIST;
		stream->flags |= FLAG_CLOSED;
	}
	nwritten += n;
	DMCLOG_D("offset = %lu,totalCount = %u,length = %lu",c->offset,c->totalCount,c->length);
	if(c->offset == c->totalCount)
	{
		stream->flags |= FLAG_CLOSED;
		n = my_snprintf(line, sizeof(line)," ], \"count\": %u, \"totalCount\": %u, \"totalPage\": %d, \"pageSize\": %d, \"totalSize\": %lu},\"header\": { \"cmd\": %d, \"seq\": %d, \"error\": %d }}",
			c->totalCount,c->totalCount,1,c->totalCount,c->totalSize,c->cmd,c->seq,c->error);
		(void) memcpy(buf, line, n);
		buf = (char *) buf + n;
		nwritten += n;
		len -= n;
	}
	EXIT_FUNC();
	return (nwritten);
}


static void
close_type(struct stream *stream)
{
	
}

void
get_type(struct conn *c)
{
	ENTER_FUNC();
	int res = 0;
	int i = 0;
	int count = 0;
	struct file_list file_list_t;
	memset(&file_list_t,0,sizeof(struct file_list));
	all_disk_t mAll_disk_t;
	memset(&mAll_disk_t,0,sizeof(all_disk_t));
	res = dm_get_storage(&mAll_disk_t);
	if(res != 0)
	{
		c->error = ERROR_GET_DISK_INFO;
		c->loc.flags |= FLAG_CLOSED;
	}
	DMCLOG_D("drive_count = %d",mAll_disk_t.count);
	file_list_t.file_type = c->fileType;
	
	for(i = 0;i < mAll_disk_t.count;i++)
	{
		/*firstly,get the disk uuid according to .dmdiskuuid;
		secondly,query the file_table_name from file_table;
		thirdly,query the count by type from the file_table_name*/
		res = read_mark_file(mAll_disk_t.disk[i].path,c->uuid);
		if(res < 0)
		{
			DMCLOG_D("get disk uuid error");
			c->error = ERROR_GET_DISK_INFO;
			c->loc.flags |= FLAG_CLOSED;
			break;
		}
		if(c->cmd == 124)
		{
			res = dm_getTypeInfoByPath(c->fileType,c->src_path + strlen(DOCUMENT_ROOT), &file_list_t,c->uuid);
			c->totalSize = file_list_t.totalSize;
		}else
			res = dm_getTypeInfo(c->cmd,c->fileType,&file_list_t,c->uuid);
		if(res != 0)
		{
			DMCLOG_D("get file list count from db error");
			c->loc.flags |= FLAG_CLOSED;
		}
		c->totalCount = file_list_t.totalCount;
		c->totalPage = file_list_t.totalPage;
		/*query the file list by type*/
	}
	
	c->loc.io.head = my_snprintf(c->loc.io.buf, c->loc.io.size,
	    "{ \"data\": { \"filelist\": [");
	io_clear(&c->rem.io);
	c->loc.io_class = &io_type;
	c->loc.flags |= FLAG_R | FLAG_ALWAYS_READY;
	DMCLOG_D("c->loc.io.buf = %s",c->loc.io.buf);

	
}

const struct io_class 	io_type =  {
	"type",
	read_type,
	NULL,
	close_type
};
