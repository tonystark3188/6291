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

#define MAX_FILE_NUM_LEN    1024*5

static void
close_type(struct stream *stream)
{
	struct conn	*c = stream->conn;
//    file_info_t *p_file_info = NULL;
//    file_info_t *nn = NULL;
//	dl_list_for_each_safe(p_file_info, nn, &(get_db_opr_obj(c->msg).data.file_list.head), file_info_t, next)
//    {
//        free_db_fd(&p_file_info);
//    }
	free_message(&c->msg);
}

static int
all_read_type(struct stream *stream, void *buf, size_t len)
{
	file_info_t *p_file_info = NULL;
	char		line[FILENAME_MAX + 512];
	int		n, nwritten = 0;
	int i = 0;
	struct conn	*c = stream->conn;
	dl_list_for_each(p_file_info,&(get_db_opr_obj(c->msg).data.file_list.head), file_info_t, next)
	{
        
        if(get_fuser_flag() == AIRDISK_ON_PC)
        {
            DMCLOG_D("airdisk on pc");
            break;
        }
        
		if (len < sizeof(line))
		{
			//DMCLOG_E("out of mem");
			break;
		}
		
		if(i == c->nfiles)
		{
			if(c->totalCount == 0)
			{
				n = my_snprintf(line, sizeof(line),
			    "{ \"parent_id\": %u,\"size\": %lld, \"data\": %u, \"isFolder\": %d,\"name\": \"%s\", \"path\": \"%s\", \"count\": %u , \"file_uuid\": \"%s\"}",
			    p_file_info->parent_id,p_file_info->file_size, p_file_info->modify_time,0,p_file_info->name,\
			    p_file_info->path != NULL?p_file_info->path + 1:p_file_info->path,0,p_file_info->file_uuid);
			}else{
				n = my_snprintf(line, sizeof(line),
			    ",{ \"parent_id\": %u,\"size\": %lld, \"data\": %u, \"isFolder\": %d,\"name\": \"%s\", \"path\": \"%s\", \"count\": %u , \"file_uuid\": \"%s\"}",
			    p_file_info->parent_id,p_file_info->file_size, p_file_info->modify_time,0,p_file_info->name,\
			    p_file_info->path != NULL?p_file_info->path + 1:p_file_info->path,0,p_file_info->file_uuid);
			}
			c->nfiles++;
            c->totalCount++;
			(void) memcpy(buf, line, n);
			buf = (char *) buf + n;
			nwritten += n;
			len -=n;
		}
		i++;
	}
    
//    DMCLOG_D("c->nfiles = %d,total_count = %d",c->nfiles,get_db_opr_obj(c->msg).data.file_list.result_cnt);
	if(c->nfiles == get_db_opr_obj(c->msg).data.file_list.result_cnt)
	{
		if (len < sizeof(line))
		{
			DMCLOG_E("out of mem");
			return nwritten;
		}
        
        DMCLOG_D("c->cmd = %d,c->nfiles = %d,total = %d",c->cmd,c->nfiles,get_db_opr_obj(c->msg).data.file_list.result_cnt);
        
        file_info_t *p_file_info = NULL;
        file_info_t *nn = NULL;
        struct conn	*c = stream->conn;
        dl_list_for_each_safe(p_file_info, nn, &(get_db_opr_obj(c->msg).data.file_list.head), file_info_t, next)
        {
            dl_list_del(&p_file_info->next);
            free_db_fd(&p_file_info);
        }
        
        if(c->nfiles < MAX_FILE_NUM_LEN)//finished
        {
            DMCLOG_D("totalCount = %d",c->totalCount);
            stream->flags |= FLAG_CLOSED;
            stream->flags &= ~(FLAG_R | FLAG_W | FLAG_ALWAYS_READY);
            n = my_snprintf(line, sizeof(line)," ],\"startIndex\":0, \"count\": %u, \"totalCount\": %u, \"totalPage\": %d, \"pageSize\": %d, \"totalSize\": %lu},\"header\": { \"cmd\": %d, \"seq\": %d, \"error\": %d }}",
                            c->totalCount,c->totalCount,1,c->totalCount,c->totalSize,c->cmd,c->seq,c->error);
            (void) memcpy(buf, line, n);
            buf = (char *) buf + n;
            nwritten += n;
            len -= n;
            
        }else{
            c->off += c->nfiles;
            int count = 0;
            if(c->len <= 0)
            {
                count = MAX_FILE_NUM_LEN;
            }else{
                c->len = c->len - c->nfiles;
                if(c->len < MAX_FILE_NUM_LEN)
                {
                    count = c->len;
                }else{
                    count = MAX_FILE_NUM_LEN;
                }
            }
            DMCLOG_D("c->off = %d,count = %d",c->off,count);
            get_db_opr_obj(c->msg).data.file_list.result_cnt = 0;
            int res = handle_get_all_type_file_list_cmd(c->src_path,c->fileType,c->sortType,c->off,count,c);
            if(res != RET_SUCCESS)
            {
                stream->flags |= FLAG_CLOSED;
                stream->flags &= ~(FLAG_R | FLAG_W | FLAG_ALWAYS_READY);
                n = my_snprintf(line, sizeof(line)," ],\"startIndex\":0, \"count\": %u, \"totalCount\": %u, \"totalPage\": %d, \"pageSize\": %d, \"totalSize\": %lu},\"header\": { \"cmd\": %d, \"seq\": %d, \"error\": %d }}",
                                c->totalCount,c->totalCount,1,c->totalCount,c->totalSize,c->cmd,c->seq,c->error);
                (void) memcpy(buf, line, n);
                buf = (char *) buf + n;
                nwritten += n;
                len -= n;
            }
            c->nfiles = 0;
        }
	}
	return (nwritten);
}

int get_parent_path(char *path,char *parentPath)
{
	char *tmp = strrchr(path,'/');
	if(tmp == NULL)
	{
		DMCLOG_E("the path is not valid");
		return -1;
	}
	if(tmp == path)
	{
		DMCLOG_E("the path is not valid");
		return -1;
	}else{
		*tmp = '\0';
		strcpy(parentPath,path);
		*tmp = '/';
	}
	return 0;
}

static int read_dir_type(struct stream *stream, void *buf, size_t len)
{
	file_info_t *p_file_info = NULL;
	char		line[FILENAME_MAX + 512];
	char		curParentPath[FILENAME_MAX];
	int		n, nwritten = 0;
	int i = 0;
	struct conn	*c = stream->conn;
	struct file_list *file_list_t = &get_db_opr_obj(c->msg).data.file_list;
	dl_list_for_each(p_file_info,&(get_db_opr_obj(c->msg).data.file_list.head), file_info_t, next)
	{
		if (len < sizeof(line))
		{
			break;
		}
		
		if(get_fuser_flag() == AIRDISK_ON_PC)
		{
			DMCLOG_D("airdisk on pc");
			break;
		}
		if(i == c->nfiles)
		{
			if(c->curParentId != p_file_info->parent_id)
			{
				c->curParentId = p_file_info->parent_id;
				c->totalCount++;
				if(c->length < 0)
				{
					get_parent_path(p_file_info->path + 1,curParentPath);
					DMCLOG_D("curParentPath = %s",curParentPath);
					if(c->fileNum == 0)
					{
						n = my_snprintf(line, sizeof(line),
					    "{ \"path\":%s,\"name\":%s,\"filelist\": [ {\"size\": %lld, \"data\": %u, \"isFolder\": %d, \"type\": %d, \"name\": \"%s\", \"path\": \"%s\"}",
					    curParentPath,bb_basename(curParentPath),p_file_info->file_size, p_file_info->modify_time,p_file_info->isFolder, p_file_info->file_type,p_file_info->name,\
					    p_file_info->path != NULL?p_file_info->path + 1:p_file_info->path);
						file_list_t->totalCount++;
						c->fileNum++;
					}else{
						n = my_snprintf(line, sizeof(line),
					    "],\"count\":%u},{ \"path\":%s,\"name\":%s,\"filelist\": [ {\"size\": %lld, \"data\": %u, \"isFolder\": %d, \"type\": %d, \"name\": \"%s\", \"path\": \"%s\"}",
					    file_list_t->totalCount,curParentPath,bb_basename(curParentPath),p_file_info->file_size, p_file_info->modify_time,p_file_info->isFolder, p_file_info->file_type,p_file_info->name,\
					    p_file_info->path != NULL?p_file_info->path + 1:p_file_info->path);
						file_list_t->totalCount++;
					}
					(void) memcpy(buf, line, n);
					buf = (char *) buf + n;
					nwritten += n;
					len -=n;
					file_list_t->totalCount = 0;
				}else{
					DMCLOG_D("c->totalCount = %u",c->totalCount);
					DMCLOG_D("c->length = %lld",c->length);
					if(c->totalCount >= c->offset&&c->totalCount <= c->offset + c->length)
					{
						DMCLOG_D("c->totalCount = %d",c->totalCount);
						get_parent_path(p_file_info->path + 1,curParentPath);
						DMCLOG_D("curParentPath = %s",curParentPath);
						if(c->fileNum == 0)
						{
							n = my_snprintf(line, sizeof(line),
						    "{ \"path\":\"%s\",\"name\":\"%s\",\"filelist\": [ {\"size\": %lld, \"data\": %u, \"isFolder\": %d, \"type\": %d, \"name\": \"%s\", \"path\": \"%s\"}",
						    curParentPath,bb_basename(curParentPath),p_file_info->file_size, p_file_info->modify_time,p_file_info->isFolder, p_file_info->file_type,p_file_info->name,\
						    p_file_info->path != NULL?p_file_info->path + 1:p_file_info->path);
							file_list_t->totalCount++;
							c->fileNum++;
						}else{
							n = my_snprintf(line, sizeof(line),
						    "],\"count\":%u},{ \"path\":\"%s\",\"name\":\"%s\",\"filelist\": [ {\"size\": %lld, \"data\": %u, \"isFolder\": %d, \"type\": %d, \"name\": \"%s\", \"path\": \"%s\"}",
						    file_list_t->totalCount,curParentPath,bb_basename(curParentPath),p_file_info->file_size, p_file_info->modify_time,p_file_info->isFolder, p_file_info->file_type,p_file_info->name,\
						    p_file_info->path != NULL?p_file_info->path + 1:p_file_info->path);
							file_list_t->totalCount++;
						}
						(void) memcpy(buf, line, n);
						buf = (char *) buf + n;
						nwritten += n;
						len -=n;
						file_list_t->totalCount = 0;
					}
				}
				
			}else{
				if(c->length < 0)
				{
					n = my_snprintf(line, sizeof(line),
				    ",{\"size\": %lld, \"data\": %u, \"isFolder\": %d, \"type\": %d, \"name\": \"%s\", \"path\": \"%s\"}",
				    p_file_info->file_size, p_file_info->modify_time,p_file_info->isFolder, p_file_info->file_type,p_file_info->name,\
				    p_file_info->path != NULL?p_file_info->path + 1:p_file_info->path);
					file_list_t->totalCount++;
					(void) memcpy(buf, line, n);
					buf = (char *) buf + n;
					nwritten += n;
					len -=n;
				}else{
					if(c->totalCount >= c->offset&&c->totalCount <= c->offset + c->length)
					{
						n = my_snprintf(line, sizeof(line),
					    ",{\"size\": %lld, \"data\": %u, \"isFolder\": %d, \"type\": %d, \"name\": \"%s\", \"path\": \"%s\"}",
					    p_file_info->file_size, p_file_info->modify_time,p_file_info->isFolder, p_file_info->file_type,p_file_info->name,\
					    p_file_info->path != NULL?p_file_info->path + 1:p_file_info->path);
						file_list_t->totalCount++;
						(void) memcpy(buf, line, n);
						buf = (char *) buf + n;
						nwritten += n;
						len -=n;
					}
				}
			}
			c->nfiles++;
		}
		i++;
	}
	if(c->nfiles == c->count||get_fuser_flag() == AIRDISK_ON_PC)
	{
		if (len < sizeof(line))
		{
			DMCLOG_E("out of mem");
			return nwritten;
		}
		stream->flags |= FLAG_CLOSED;
		stream->flags &= ~(FLAG_R | FLAG_W | FLAG_ALWAYS_READY);
		unsigned totalCount;
		if(c->length < 0)
		{
			totalCount = c->totalCount;
		}else{
			totalCount = c->length > c->totalCount?c->totalCount:c->length;
		}
		n = my_snprintf(line, sizeof(line),"],\"count\":%u}], \"startIndex\":%lld,\"count\": %u, \"totalCount\": %u},\"header\": { \"cmd\": %d, \"seq\": %d, \"error\": %d }}",
			file_list_t->totalCount,c->offset,totalCount,c->totalCount,c->cmd,c->seq,c->error);
		(void) memcpy(buf, line, n);
		buf = (char *) buf + n;
		nwritten += n;
		len -= n;
	}
	return (nwritten);
}



void
all_get_type(struct conn *c)
{
	int res = 0;
	int i = 0;
	struct file_list file_list_t;
	memset(&file_list_t,0,sizeof(struct file_list));
	all_disk_t mAll_disk_t;
	memset(&mAll_disk_t,0,sizeof(all_disk_t));
	res = dm_get_storage(&mAll_disk_t);
	if(res != 0)
	{
		c->error = ERROR_GET_DISK_INFO;
		c->loc.flags |= FLAG_CLOSED;
		return 0;
	}
//	DMCLOG_D("drive_count = %d",mAll_disk_t.count);
	if(mAll_disk_t.count == 0)
	{
		c->error = ERROR_GET_DISK_INFO;
		c->loc.flags |= FLAG_CLOSED;
		return 0;
	}

	io_clear(&c->rem.io);
	c->loc.io_class = &io_type;
	c->loc.flags |= FLAG_R | FLAG_ALWAYS_READY;
	if((c->msg = new_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }

	dl_list_init(&c->msg->msg_data.db_obj.data.file_list.head);
	
	for(i = 0;i < mAll_disk_t.count;i++)
	{
		DMCLOG_D("mAll_disk_t.disk[i].name: %s", mAll_disk_t.disk[i].path);
		EnterCriticalSection(&c->ctx->mutex);
		res = read_mark_file(mAll_disk_t.disk[i].path,c->uuid);
		LeaveCriticalSection(&c->ctx->mutex);
		if(res < 0)
		{
			DMCLOG_D("get disk uuid error");
			c->error = ERROR_GET_DISK_INFO;
			c->loc.flags |= FLAG_CLOSED;
			return 0;
		}

		c->disk_info = get_disk_node(c->uuid);
		if(c->disk_info == NULL)
		{
			DMCLOG_E("get the disk info failed");
			c->error = ERROR_GET_FILE_LIST;
	        return EMESSAGE_NEW;
		}
        
        int len = 0;
        DMCLOG_D("length = %d",c->len);
        if(c->len <= 0)//user needs to get all files
        {
            len = MAX_FILE_NUM_LEN;
        }else{//user just need to get limit files and count equql to length
            if(c->len > MAX_FILE_NUM_LEN)
            {
                len = MAX_FILE_NUM_LEN;//we need sometimes to get files from db
            }else{
                len = c->len;//we just one time to get files from db,
            }
        }
        DMCLOG_D("c->cmd = %d,fileType = %d,c->offset = %d,len = %d",c->cmd,c->fileType,c->off,len);
		res = handle_get_all_type_file_list_cmd(c->src_path,c->fileType,c->sortType,c->off,len,c);
		if(res != RET_SUCCESS)
		{
			DMCLOG_D("_handle_get_all_type_file_list_cmd error");
			c->error = ERROR_GET_FILE_LIST;
			c->loc.flags |= FLAG_CLOSED;
			return;
		}
		
		/*query the file list by type*/
	}
	
	c->loc.io.head = my_snprintf(c->loc.io.buf, c->loc.io.size,
	    "{ \"data\": { \"filelist\": [");
}

void get_dir_type(struct conn *c)
{
	ENTER_FUNC();
	int res = 0;
	int i = 0;
	struct file_list file_list_t;
	memset(&file_list_t,0,sizeof(struct file_list));
	all_disk_t mAll_disk_t;
	memset(&mAll_disk_t,0,sizeof(all_disk_t));
	res = dm_get_storage(&mAll_disk_t);
	if(res != 0)
	{
		c->error = ERROR_GET_DISK_INFO;
		c->loc.flags |= FLAG_CLOSED;
		return;
	}
	DMCLOG_D("drive_count = %d",mAll_disk_t.count);
	if(mAll_disk_t.count == 0)
	{
		c->error = ERROR_GET_DISK_INFO;
		c->loc.flags |= FLAG_CLOSED;
		return;
	}

	io_clear(&c->rem.io);
	c->loc.io_class = &io_dir_type;
	c->loc.flags |= FLAG_R | FLAG_ALWAYS_READY;
	if((c->msg = new_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return;
    }

	dl_list_init(&c->msg->msg_data.db_obj.data.file_list.head);
	
	for(i = 0;i < mAll_disk_t.count;i++)
	{
		DMCLOG_D("mAll_disk_t.disk[i].name: %s", mAll_disk_t.disk[i].path);
		EnterCriticalSection(&c->ctx->mutex);
		res = read_mark_file(mAll_disk_t.disk[i].path,c->uuid);
		LeaveCriticalSection(&c->ctx->mutex);
		if(res < 0)
		{
			DMCLOG_D("get disk uuid error");
			c->error = ERROR_GET_DISK_INFO;
			c->loc.flags |= FLAG_CLOSED;
			return;
		}

		c->disk_info = get_disk_node(c->uuid);
		if(c->disk_info == NULL)
		{
			DMCLOG_E("get the disk info failed");
			c->error = ERROR_GET_FILE_LIST;
	        return;
		}
		
		res = _handle_get_all_type_dir_list_cmd(c);
		if(res != RET_SUCCESS)
		{
			DMCLOG_D("_handle_get_all_type_dir_list_cmd error");
			c->error = ERROR_GET_FILE_LIST;
			c->loc.flags |= FLAG_CLOSED;
			return;
		}
		
		/*query the file list by type*/
	}
	
	c->loc.io.head = my_snprintf(c->loc.io.buf, c->loc.io.size,
	    "{ \"data\": { \"dirlist\": [");
	EXIT_FUNC();
}



const struct io_class 	io_type =  {
	"type",
	all_read_type,
	NULL,
	close_type
};

const struct io_class 	io_dir_type =  {
	"dir_type",
	read_dir_type,
	NULL,
	close_type
};

