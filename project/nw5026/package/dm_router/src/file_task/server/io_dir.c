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

static void
all_close_dir(struct stream *stream)
{
	int i = 0;
	file_info_t *p_file_info = NULL;
	file_info_t *nn = NULL;
	struct conn	*c = stream->conn;
	dl_list_for_each_safe(p_file_info, nn, &(get_db_opr_obj(c->msg).data.file_list.head), file_info_t, next)
    {
        free_db_fd(&p_file_info);
    }
	free_message(&c->msg);
}


static int
all_read_dir(struct stream *stream, void *buf, size_t len)
{
	file_info_t *p_file_info = NULL;
	char		line[FILENAME_MAX + 512];
	int		n, nwritten = 0;
	int i = 0;
	struct conn	*c = stream->conn;
	dl_list_for_each(p_file_info,&(get_db_opr_obj(c->msg).data.file_list.head), file_info_t, next)
	{
		if (len < sizeof(line))
		{
			//DMCLOG_E("out of mem");
			break;
		}
		
		if(get_fuser_flag() == AIRDISK_ON_PC)
		{
			DMCLOG_D("airdisk on pc");
			break;
		}
		
		if(i == c->nfiles)
		{
			if(c->nfiles == 0)
			{
				n = my_snprintf(line, sizeof(line),
			    "{ \"isFolder\": %d, \"size\": %lld, \"data\": %u, \"name\": \"%s\",\"attr\":%d}",
			    p_file_info->isFolder, p_file_info->file_size,p_file_info->modify_time, p_file_info->name,p_file_info->attr);
			}else{
				n = my_snprintf(line, sizeof(line),
			    ",{ \"isFolder\": %d, \"size\": %lld, \"data\": %u, \"name\": \"%s\",\"attr\":%d}",
			    p_file_info->isFolder, p_file_info->file_size,p_file_info->modify_time, p_file_info->name,p_file_info->attr);
			}
			c->nfiles++;
			(void) memcpy(buf, line, n);
			buf = (char *) buf + n;
			nwritten += n;
			len -=n;
			if(c->nfiles == c->count)
				break;
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
		n = my_snprintf(line, sizeof(line)," ],\"startIndex\":%lld, \"count\": %u, \"totalCount\": %u, \"totalPage\": %d, \"pageSize\": %d},\"header\": { \"cmd\": %d, \"seq\": %d, \"error\": %d }}",
			c->offset,c->count,c->totalCount,1,c->count,c->cmd,c->seq,c->error);
		(void) memcpy(buf, line, n);
		buf = (char *) buf + n;
		nwritten += n;
		len -= n;
	}
	return (nwritten);
}



static int
read_dir(struct stream *stream, void *buf, size_t len)
{
	struct dirent	*dp = NULL;
	char		file[FILENAME_MAX], line[FILENAME_MAX + 512];
	struct stat	st;
	struct conn	*c = stream->conn;
	int		n = 0, nwritten = 0;
	const char	*slash = "/";
	
	assert(stream->chan.dir.dirp != NULL);
	do {
		if (len < sizeof(line))
		{
			//DMCLOG_D("out of stream");
			break;
		}

		if ((dp = readdir(stream->chan.dir.dirp)) == NULL) {
			stream->flags |= FLAG_CLOSED;
			stream->flags &= ~(FLAG_R | FLAG_W | FLAG_ALWAYS_READY);
			n = my_snprintf(line, sizeof(line)," ], \"startIndex\": %lld,\"count\": %u, \"totalCount\": %u, \"totalPage\": 1, \"pageSize\": %u },\"header\": { \"cmd\": %d, \"seq\": %d, \"error\": %d }}",
				c->offset,c->nfiles,c->totalCount,c->nfiles,c->cmd,c->seq,c->error);
			(void) memcpy(buf, line, n);
			buf = (char *) buf + n;
			nwritten += n;
			len -= n;
			DMCLOG_D("read finish,c->totalCount = %u,c->nfiles = %u",c->totalCount,c->nfiles);
			break;
		}
		
		if (dp->d_name[0] == '.') {
			continue;
		}
		/*(void) my_snprintf(file, sizeof(file),
		    "%s%s%s", stream->chan.dir.path, slash, dp->d_name);*/
		if(c->nfiles <= 1024)
		{
			sprintf(file,"%s%s%s", stream->chan.dir.path, slash, dp->d_name);
			(void) my_stat(file, &st);
		}
	    
		if(c->length <= 0)
		{
			if(c->nfiles == 0)
			{
				n = sprintf(line,"{ \"isFolder\": %d, \"size\": %lld, \"data\": %u, \"name\": \"%s\"}",
			    dp->d_type == 8?0:1, st.st_size,st.st_mtime, dp->d_name);
				/*n = my_snprintf(line, sizeof(line),
			    "{ \"isFolder\": %d, \"size\": %lld, \"data\": %u, \"name\": \"%s\"}",
			    S_ISDIR(st.st_mode), st.st_size,st.st_mtime, dp->d_name);*/
			}else{
				n = sprintf(line,
			    ",{ \"isFolder\": %d, \"size\": %lld, \"data\": %u, \"name\": \"%s\"}",
			    dp->d_type == 8?0:1, st.st_size,st.st_mtime, dp->d_name);
				/*n = my_snprintf(line, sizeof(line),
			    ",{ \"isFolder\": %d, \"size\": %lld, \"data\": %u, \"name\": \"%s\"}",
			    S_ISDIR(st.st_mode), st.st_size,st.st_mtime, dp->d_name);*/
			}
			
			(void) memcpy(buf, line, n);
			buf = (char *) buf + n;
			nwritten += n;
			len -= n;
			c->nfiles++;
		}else{
			if(c->totalCount >= c->offset&&c->totalCount < c->length)
			{
				if(c->totalCount == c->offset)
				{
					n = my_snprintf(line, sizeof(line),
				    "{ \"isFolder\": %d, \"size\": %lld, \"data\": %u, \"name\": \"%s\",\"attr\":%d}",
				    S_ISDIR(st.st_mode), st.st_size,st.st_mtime, dp->d_name,dm_get_attr_hide(file));
				}else{
					n = my_snprintf(line, sizeof(line),
				    ",{ \"isFolder\": %d, \"size\": %lld, \"data\": %u, \"name\": \"%s\",\"attr\":%d}",
				    S_ISDIR(st.st_mode), st.st_size,st.st_mtime, dp->d_name,dm_get_attr_hide(file));
				}
				(void) memcpy(buf, line, n);
				buf = (char *) buf + n;
				nwritten += n;
				len -= n;
				c->nfiles++;
			}
		}
		c->totalCount++;
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

void
all_get_dir(struct conn *c)
{
	ENTER_FUNC();
	int res = 0;
	int i = 0;
	struct file_list file_list_t;
	memset(&file_list_t,0,sizeof(struct file_list));

	io_clear(&c->rem.io);
	c->loc.io_class = &io_all_dir;
	c->loc.flags |= FLAG_R | FLAG_ALWAYS_READY;
	if((c->msg = new_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
	

	c->disk_info = get_disk_node(c->disk_uuid);
	if(c->disk_info == NULL)
	{
		DMCLOG_E("get the disk info failed");
		c->error = ERROR_GET_FILE_LIST;
        return EMESSAGE_NEW;
	}
	
	dl_list_init(&c->msg->msg_data.db_obj.data.file_list.head);
	res = _handle_get_all_type_file_list_cmd(c);
	if(res != RET_SUCCESS)
	{
		DMCLOG_D("_handle_get_all_type_file_list_cmd error");
		c->error = ERROR_GET_FILE_LIST;
		c->loc.flags |= FLAG_CLOSED;
		return;
	}
	c->loc.io.head = my_snprintf(c->loc.io.buf, c->loc.io.size,
	    "{ \"data\": { \"filelist\": [");
	EXIT_FUNC();
}


const struct io_class	io_dir =  {
	"dir",
	read_dir,
	NULL,
	close_dir
};

const struct io_class	io_all_dir =  {
	"dir",
	all_read_dir,
	NULL,
	all_close_dir
};


