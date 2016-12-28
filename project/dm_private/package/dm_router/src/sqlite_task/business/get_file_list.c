/*
 * =============================================================================
 *
 *       Filename:  get_file_list.c
 *
 *    Description:  handle get file list cmd.
 *
 *        Version:  1.0
 *        Created:  2014/10/11 17:55:00
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver(),
 *   Organization:  
 *
 * =============================================================================
 */
#include "tools/base.h"
#include "tools/config.h"
#include "network/net_util.h"
#include "db/file_table.h"
#include "mo/mo.h"
#include "mq/mq.h"
#include "db/db_mm_pool.h"
#include "disk_manage.h"
#include "task/db_task.h"
#include "defs.h"


int handle_get_file_size_cmd(int file_type,unsigned long offset,unsigned long length,int sort_mode,JObj *response_para_json,char *uuid)
{   
    uint32_t index = 0;
    Message *msg;
	file_info_t *p_file_info = NULL;
    file_info_t *n = NULL;
	int len;
	uint64_t result_size = 0;
    ENTER_FUNC();
	
    DMCLOG_D("file_type(0x%x), index(0x%x), sort_mode(0x%x), start(0x%x), len(0x%x)",  file_type, index, sort_mode, offset, length);
	struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    // 2. query db.
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }

    msg->msg_header.m_type = MSG_DB_QUERY_FILE_LIST;
    get_db_opr_obj(msg).data.file_list.file_type = file_type;
	get_db_opr_obj(msg).data.file_list.sort_mode = sort_mode;
	get_db_opr_obj(msg).data.file_list.start_index = offset;
	get_db_opr_obj(msg).data.file_list.len = length;
	get_db_opr_obj(msg).data.file_list.result_size = 0;
	get_db_opr_obj(msg).data.file_list.result_cnt = 0;
    disk_info->g_db_query_task.msg_cb(&disk_info->g_db_query_task,msg);
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file list failed");
        free_sync_message(&msg);
        return EDB;
    }
    len = get_db_opr_obj(msg).data.file_list.result_cnt;
	result_size = get_db_opr_obj(msg).data.file_list.result_size;
    DMCLOG_D("result_cnt(%d)", len);
	DMCLOG_D("result_size = %llu",result_size);
    if(len > 0)
    {
		JSON_ADD_OBJECT(response_para_json, "totalCount", JSON_NEW_OBJECT(len ,int));
		JSON_ADD_OBJECT(response_para_json, "totalSize", JSON_NEW_OBJECT(result_size,int64));
    }
    dl_list_for_each_safe(p_file_info, n, &(get_db_opr_obj(msg).data.file_list.head), file_info_t, next)
    {
        free_db_fd(&p_file_info);
    }

    free_sync_message(&msg);
    log_trace("Exit: success");
    return RET_SUCCESS;

_HANDLE_CMD_FAILED_:
    
    dl_list_for_each_safe(p_file_info, n, &(get_db_opr_obj(msg).data.file_list.head), file_info_t, next)
    {
        free_db_fd(&p_file_info);
    }
    
    free_sync_message(&msg);
    log_trace("Exit: failed!");
    return EJSON_NEW;
}

/*int handle_get_file_list_cmd(int file_type,unsigned long offset,unsigned long length,int sort_mode,JObj *response_para_json,char *uuid)
{   
    uint32_t index = 0;
    Message *msg;
	int ret = RET_SUCCESS;
	file_info_t *p_file_info = NULL;
    file_info_t *n = NULL;
	int len;
    ENTER_FUNC();
	struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    DMCLOG_D("file_type(0x%x), index(0x%x), sort_mode(0x%x), start(0x%x), len(0x%x)",  file_type, index, sort_mode, offset, length);

    // 2. query db.
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }

    msg->msg_header.m_type = MSG_DB_QUERY_FILE_LIST;
    get_db_opr_obj(msg).data.file_list.file_type = file_type;
	get_db_opr_obj(msg).data.file_list.sort_mode = sort_mode;
	get_db_opr_obj(msg).data.file_list.start_index = offset;
	get_db_opr_obj(msg).data.file_list.len = length;
    get_db_opr_obj(msg).data.file_list.result_cnt = 0;
	get_db_opr_obj(msg).data.file_list.result_size = 0;

   	get_db_opr_obj(msg).ret = disk_info->g_db_query_task.msg_cb(&disk_info->g_db_query_task,msg);
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file list failed");
        free_sync_message(&msg);
        return EDB;
    }
    len = get_db_opr_obj(msg).data.file_list.result_cnt;
    log_debug("result_cnt(%d)", len);
	JObj *file_info_array = JSON_NEW_ARRAY();
    if(len > 0)
    {
        // handle query result.
    	dl_list_for_each(p_file_info, &(get_db_opr_obj(msg).data.file_list.head), file_info_t, next)
    	{
            // init item member
            //DMCLOG_D("p_file_info->path = %s,p_file_info->name = %s,utime = %lu",p_file_info->path,p_file_info->name,p_file_info->create_time);
			JObj *file_info = JSON_NEW_EMPTY_OBJECT();
			JSON_ADD_OBJECT(file_info, "size",JSON_NEW_OBJECT(p_file_info->file_size,int64));
			JSON_ADD_OBJECT(file_info, "data",JSON_NEW_OBJECT(p_file_info->create_time,int64));
            JSON_ADD_OBJECT(file_info, "isFolder",JSON_NEW_OBJECT(p_file_info->isFolder,int));
            JSON_ADD_OBJECT(file_info, "type",JSON_NEW_OBJECT(p_file_info->file_type,int));
			JSON_ADD_OBJECT(file_info, "name",JSON_NEW_OBJECT(p_file_info->name,string));
			JSON_ADD_OBJECT(file_info, "path",JSON_NEW_OBJECT(p_file_info->path + 1,string));
			//handle_get_file_list_count_by_path_cmd(file_type,&p_file_info->file_count,p_file_info->path,uuid);
			JSON_ADD_OBJECT(file_info, "count",JSON_NEW_OBJECT(p_file_info->file_count,int));
			JSON_ARRAY_ADD_OBJECT (file_info_array,file_info);
    	}
		JSON_ADD_OBJECT(response_para_json, "filelist", file_info_array);
    }
    dl_list_for_each_safe(p_file_info, n, &(get_db_opr_obj(msg).data.file_list.head), file_info_t, next)
    {
        free_db_fd(&p_file_info);
    }
    
    free_sync_message(&msg);
    
    log_trace("Exit: success");
    return RET_SUCCESS;

_HANDLE_CMD_FAILED_:
    
    dl_list_for_each_safe(p_file_info, n, &(get_db_opr_obj(msg).data.file_list.head), file_info_t, next)
    {
        free_db_fd(&p_file_info);
    }
    
    free_sync_message(&msg);
    log_trace("Exit: failed!");
    return EJSON_NEW;
}*/
int	_handle_get_file_list_cmd(int sort_mode,char **buf,size_t *io_len,struct conn *c)
{ 
	ENTER_FUNC();
    Message *msg;
	int ret = RET_SUCCESS;
	file_info_t *p_file_info = NULL;
	file_info_t *nn = NULL;
	int			len ,n, nwritten = 0;
	char		line[FILENAME_MAX + 512];
	struct disk_node *disk_info = get_disk_node(c->uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    DMCLOG_D("file_type(0x%x),sort_mode(0x%x), start(0x%x), len(0x%x)" ,c->fileType,sort_mode, c->offset, c->length);

    // 2. query db.
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
	if(c->cmd == 120)
    	msg->msg_header.m_type = MSG_DB_QUERY_FILE_LIST;
	else if(c->cmd == 123)
		msg->msg_header.m_type = MSG_DB_QUERY_DIR_LIST;
	else if(c->cmd == 124)
	{
		msg->msg_header.m_type = MSG_DB_QUERY_FILE_LIST_BY_PATH;
		get_db_opr_obj(msg).data.file_list.path = c->src_path != NULL?c->src_path + strlen(DOCUMENT_ROOT):NULL;
	}
		
    get_db_opr_obj(msg).data.file_list.file_type = c->fileType;
	get_db_opr_obj(msg).data.file_list.sort_mode = sort_mode;
	get_db_opr_obj(msg).data.file_list.start_index = c->offset;
	get_db_opr_obj(msg).data.file_list.len = c->length;
    get_db_opr_obj(msg).data.file_list.result_cnt = 0;
	get_db_opr_obj(msg).data.file_list.result_size = 0;
	


   	get_db_opr_obj(msg).ret = disk_info->g_db_query_task.msg_cb(&disk_info->g_db_query_task,msg);
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file list failed");
        goto _HANDLE_CMD_FAILED_;
    }
    len = get_db_opr_obj(msg).data.file_list.result_cnt;
    DMCLOG_D("result_cnt(%d)", len);
    if(len > 0)
    {
        // handle query result.
    	dl_list_for_each(p_file_info, &(get_db_opr_obj(msg).data.file_list.head), file_info_t, next)
    	{
    		//DMCLOG_D("nfiels = %u",*nfiles);
    		if (*io_len < sizeof(line))
			{
				DMCLOG_E("out of mem");
				break;
			}
			if(c->offset == 0)
			{
				n = my_snprintf(line, sizeof(line),
			    "{ \"size\": %lu, \"data\": %u, \"isFolder\": %d, \"type\": %d, \"name\": \"%s\", \"path\": \"%s\", \"count\": %u }",
			    p_file_info->file_size, p_file_info->create_time,p_file_info->isFolder, p_file_info->file_type,p_file_info->name,p_file_info->path + 1,p_file_info->file_count);
				
			}else{
				n = my_snprintf(line, sizeof(line),
			    ",{ \"size\": %lu, \"data\": %u, \"isFolder\": %d, \"type\": %d, \"name\": \"%s\", \"path\": \"%s\", \"count\": %u }",
			    p_file_info->file_size, p_file_info->create_time,p_file_info->isFolder, p_file_info->file_type,p_file_info->name,p_file_info->path + 1,p_file_info->file_count);
			}
			c->offset++;
			(void) memcpy(*buf, line, n);
			*buf = (char *) *buf + n;
			nwritten += n;
			*io_len -=n;
			if(c->offset == c->totalCount)
				break;
    	}
    }
    dl_list_for_each_safe(p_file_info, nn, &(get_db_opr_obj(msg).data.file_list.head), file_info_t, next)
    {
        free_db_fd(&p_file_info);
    }
    
    free_sync_message(&msg);
    log_trace("Exit: success");
    return nwritten;

_HANDLE_CMD_FAILED_:
    
    dl_list_for_each_safe(p_file_info, nn, &(get_db_opr_obj(msg).data.file_list.head), file_info_t, next)
    {
        free_db_fd(&p_file_info);
    }
    
    free_sync_message(&msg);
    EXIT_FUNC();
    return EJSON_NEW;
}

#if 0
int handle_get_dir_list_cmd(int file_type,unsigned long offset,unsigned long length,int sort_mode,JObj *response_para_json,char *uuid)
{   
    uint32_t index = 0;
    Message *msg;
	int ret = RET_SUCCESS;
	file_info_t *p_file_info = NULL;
    file_info_t *n = NULL;
	int len;
    ENTER_FUNC();
	struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    DMCLOG_D("file_type(0x%x), index(0x%x), sort_mode(0x%x), start(0x%x), len(0x%x)",  file_type, index, sort_mode, offset, length);

    // 2. query db.
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }

    msg->msg_header.m_type = MSG_DB_QUERY_DIR_LIST;
    get_db_opr_obj(msg).data.file_list.file_type = file_type;
	get_db_opr_obj(msg).data.file_list.sort_mode = sort_mode;
	get_db_opr_obj(msg).data.file_list.start_index = offset;
	get_db_opr_obj(msg).data.file_list.len = length;
    get_db_opr_obj(msg).data.file_list.result_cnt = 0;
	get_db_opr_obj(msg).data.file_list.result_size = 0;
	

   	get_db_opr_obj(msg).ret = disk_info->g_db_query_task.msg_cb(&disk_info->g_db_query_task,msg);
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file list failed");
        free_sync_message(&msg);
        return EDB;
    }
    len = get_db_opr_obj(msg).data.file_list.result_cnt;
    log_debug("result_cnt(%d)", len);
	JObj *file_info_array = JSON_NEW_ARRAY();
    if(len > 0)
    {
        // handle query result.
    	dl_list_for_each(p_file_info, &(get_db_opr_obj(msg).data.file_list.head), file_info_t, next)
    	{
            // init item member
            DMCLOG_D("p_file_info->path = %s,p_file_info->name = %s,utime = %lu",p_file_info->path,p_file_info->name,p_file_info->create_time);
			JObj *file_info = JSON_NEW_EMPTY_OBJECT();
			JSON_ADD_OBJECT(file_info, "size",JSON_NEW_OBJECT(p_file_info->file_size,int64));
			JSON_ADD_OBJECT(file_info, "data",JSON_NEW_OBJECT(p_file_info->create_time,int64));
            JSON_ADD_OBJECT(file_info, "isFolder",JSON_NEW_OBJECT(p_file_info->isFolder,int));
            JSON_ADD_OBJECT(file_info, "type",JSON_NEW_OBJECT(p_file_info->file_type,int));
			JSON_ADD_OBJECT(file_info, "name",JSON_NEW_OBJECT(p_file_info->name,string));
			JSON_ADD_OBJECT(file_info, "path",JSON_NEW_OBJECT(p_file_info->path + 1,string));
			//handle_get_file_list_count_by_path_cmd(file_type,&p_file_info->file_count,p_file_info->path,uuid);
			JSON_ADD_OBJECT(file_info, "count",JSON_NEW_OBJECT(p_file_info->file_count,int));
			JSON_ARRAY_ADD_OBJECT (file_info_array,file_info);
    	}
		JSON_ADD_OBJECT(response_para_json, "filelist", file_info_array);
    }
    dl_list_for_each_safe(p_file_info, n, &(get_db_opr_obj(msg).data.file_list.head), file_info_t, next)
    {
        free_db_fd(&p_file_info);
    }
    
    free_sync_message(&msg);
    
    log_trace("Exit: success");
    return RET_SUCCESS;

_HANDLE_CMD_FAILED_:
    
    dl_list_for_each_safe(p_file_info, n, &(get_db_opr_obj(msg).data.file_list.head), file_info_t, next)
    {
        free_db_fd(&p_file_info);
    }
    
    free_sync_message(&msg);
    log_trace("Exit: failed!");
    return EJSON_NEW;
}


int handle_get_file_list_by_path_cmd(int file_type,unsigned long offset,unsigned long length,int sort_mode,char *path,JObj *response_para_json,char *uuid)
{   
    uint32_t index = 0;
    Message *msg;
	int ret = RET_SUCCESS;
	file_info_t *p_file_info = NULL;
    file_info_t *n = NULL;
	int len;
    ENTER_FUNC();
	struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    DMCLOG_D("file_type(0x%x), index(0x%x), sort_mode(0x%x), start(0x%x), len(0x%x)",  file_type, index, sort_mode, offset, length);

    // 2. query db.
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }

    msg->msg_header.m_type = MSG_DB_QUERY_FILE_LIST_BY_PATH;
    get_db_opr_obj(msg).data.file_list.file_type = file_type;
	get_db_opr_obj(msg).data.file_list.sort_mode = sort_mode;
	get_db_opr_obj(msg).data.file_list.start_index = offset;
	get_db_opr_obj(msg).data.file_list.len = length;
	get_db_opr_obj(msg).data.file_list.path = path;
    get_db_opr_obj(msg).data.file_list.result_cnt = 0;
	get_db_opr_obj(msg).data.file_list.result_size = 0;

   	get_db_opr_obj(msg).ret = disk_info->g_db_query_task.msg_cb(&disk_info->g_db_query_task,msg);
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file list failed");
        free_sync_message(&msg);
        return EDB;
    }
    len = get_db_opr_obj(msg).data.file_list.result_cnt;
    log_debug("result_cnt(%d)", len);
	JObj *file_info_array = JSON_NEW_ARRAY();
    if(len > 0)
    {
        // handle query result.
    	dl_list_for_each(p_file_info, &(get_db_opr_obj(msg).data.file_list.head), file_info_t, next)
    	{
            // init item member
            //DMCLOG_D("p_file_info->path = %s,p_file_info->name = %s,utime = %lu",p_file_info->path,p_file_info->name,p_file_info->create_time);
			JObj *file_info = JSON_NEW_EMPTY_OBJECT();
			JSON_ADD_OBJECT(file_info, "size",JSON_NEW_OBJECT(p_file_info->file_size,int64));
			JSON_ADD_OBJECT(file_info, "data",JSON_NEW_OBJECT(p_file_info->create_time,int64));
            JSON_ADD_OBJECT(file_info, "isFolder",JSON_NEW_OBJECT(p_file_info->isFolder,int));
            JSON_ADD_OBJECT(file_info, "type",JSON_NEW_OBJECT(p_file_info->file_type,int));
			JSON_ADD_OBJECT(file_info, "name",JSON_NEW_OBJECT(p_file_info->name,string));
			JSON_ADD_OBJECT(file_info, "path",JSON_NEW_OBJECT(p_file_info->path + 1,string));// /USB-disk-1/ ->USB-disk-1/
			JSON_ARRAY_ADD_OBJECT (file_info_array,file_info);
    	}
		JSON_ADD_OBJECT(response_para_json, "filelist", file_info_array);
    }
    dl_list_for_each_safe(p_file_info, n, &(get_db_opr_obj(msg).data.file_list.head), file_info_t, next)
    {
        free_db_fd(&p_file_info);
    }
    
    free_sync_message(&msg);
    
    log_trace("Exit: success");
    return RET_SUCCESS;

_HANDLE_CMD_FAILED_:
    
    dl_list_for_each_safe(p_file_info, n, &(get_db_opr_obj(msg).data.file_list.head), file_info_t, next)
    {
        free_db_fd(&p_file_info);
    }
    
    free_sync_message(&msg);
    log_trace("Exit: failed!");
    return EJSON_NEW;
}

#endif

int handle_del_file_list_by_path_cmd(int file_type,unsigned long offset,unsigned long length,int sort_mode,char *path,char *uuid)
{   
    uint32_t index = 0;
    Message *msg;
	int ret = RET_SUCCESS;
	file_info_t *p_file_info = NULL;
    file_info_t *n = NULL;
	int len;
	char *fullname = NULL;
    ENTER_FUNC();
	struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    DMCLOG_D("file_type(0x%x), index(0x%x), sort_mode(0x%x), start(0x%x), len(0x%x)",  file_type, index, sort_mode, offset, length);

    // 2. query db.
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }

    msg->msg_header.m_type = MSG_DB_QUERY_FILE_LIST_BY_PATH;
    get_db_opr_obj(msg).data.file_list.file_type = file_type;
	get_db_opr_obj(msg).data.file_list.sort_mode = sort_mode;
	get_db_opr_obj(msg).data.file_list.start_index = offset;
	get_db_opr_obj(msg).data.file_list.len = length;
	get_db_opr_obj(msg).data.file_list.path = path;
    get_db_opr_obj(msg).data.file_list.result_cnt = 0;
	get_db_opr_obj(msg).data.file_list.result_size = 0;

   	get_db_opr_obj(msg).ret = disk_info->g_db_query_task.msg_cb(&disk_info->g_db_query_task,msg);
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file list failed");
        free_sync_message(&msg);
        return EDB;
    }
    len = get_db_opr_obj(msg).data.file_list.result_cnt;
    log_debug("result_cnt(%d)", len);
    if(len > 0)
    {
        // handle query result. /USB-disk-1/ ->USB-disk-1/
    	dl_list_for_each(p_file_info, &(get_db_opr_obj(msg).data.file_list.head), file_info_t, next)
    	{
    		fullname = (char *)calloc(1,strlen(DOCUMENT_ROOT) + strlen(p_file_info->path) + 1);
			sprintf(fullname,"%s%s",DOCUMENT_ROOT,p_file_info->path);
    		if((ret = db_backup_file_rm(fullname,uuid)) != RET_SUCCESS)
		    {
		        DMCLOG_D("delete_(%s) failed from backup db, res(%d)", fullname, ret);
				if(fullname != NULL)
					free(fullname);
				goto _HANDLE_CMD_FAILED_;
		    }

			ret = handle_file_delete_cmd(fullname,uuid);
			if(ret < 0)
			{
		        DMCLOG_D("db rm(%s) failed, ret(0x%x)", fullname, ret);
				if(fullname != NULL)
					free(fullname);
				goto _HANDLE_CMD_FAILED_;
		    }
			if((ret = rm(fullname)) != RET_SUCCESS)
		    {
		        DMCLOG_D("delete_(%s) failed, res(%d)",fullname, ret);
				if(fullname != NULL)
					free(fullname);
				goto _HANDLE_CMD_FAILED_;
		    }
			if(fullname != NULL)
				free(fullname);
    	}
    }
    dl_list_for_each_safe(p_file_info, n, &(get_db_opr_obj(msg).data.file_list.head), file_info_t, next)
    {
        free_db_fd(&p_file_info);
    }
    
    free_sync_message(&msg);
    
    log_trace("Exit: success");
    return RET_SUCCESS;

_HANDLE_CMD_FAILED_:
    
    dl_list_for_each_safe(p_file_info, n, &(get_db_opr_obj(msg).data.file_list.head), file_info_t, next)
    {
        free_db_fd(&p_file_info);
    }
    
    free_sync_message(&msg);
    log_trace("Exit: failed!");
    return EJSON_NEW;
}


int handle_get_file_list_count_cmd(int cmd,int file_type,unsigned int *type_count,char *uuid)
{   
    uint32_t counts;
    Message *msg;
    
    ENTER_FUNC();
	
    // 1. get request params.
    DMCLOG_D("file_type(0x%x)", file_type);
	struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    // 2. query db.
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    // add end.
	if(cmd == 120)
    	msg->msg_header.m_type = MSG_DB_QUERY_FILE_LIST_CNT;
	else if(cmd == 123)
		msg->msg_header.m_type = MSG_DB_QUERY_FILE_DIR_LIST_CNT;
	
    get_db_opr_obj(msg).data.file_list.file_type = file_type;
	//strcpy(get_db_opr_obj(msg).data.file_list.file_table_name,file_table_name);
	get_db_opr_obj(msg).ret = disk_info->g_db_query_task.msg_cb(&disk_info->g_db_query_task,msg);
   
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file full path failed");
        free_sync_message(&msg);
        return ECLIENT_PARAM;
		
    }

	*type_count = get_db_opr_obj(msg).data.file_list.result_cnt;
    DMCLOG_D("counts = %d", *type_count);
	free_sync_message(&msg);
    
    // 3. Response client
    
	log_trace("Exit: success");
	return RET_SUCCESS;
	
}

int handle_get_file_dir_list_count_cmd(int  file_type,unsigned int *type_count,char *uuid)
{   
    uint32_t counts;
    Message *msg;
    
    ENTER_FUNC();
	
    // 1. get request params.
    DMCLOG_D("file_type(0x%x)", file_type);
	struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    // 2. query db.
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    // add end.

    msg->msg_header.m_type = MSG_DB_QUERY_FILE_DIR_LIST_CNT; 
	
    get_db_opr_obj(msg).data.file_list.file_type = file_type;
	//strcpy(get_db_opr_obj(msg).data.file_list.file_table_name,file_table_name);
	get_db_opr_obj(msg).ret = disk_info->g_db_query_task.msg_cb(&disk_info->g_db_query_task,msg);
   
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file full path failed");
        free_sync_message(&msg);
        return ECLIENT_PARAM;
		
    }

	*type_count = get_db_opr_obj(msg).data.file_list.result_cnt;
    DMCLOG_D("counts = %d", *type_count);
	free_sync_message(&msg);
    
    // 3. Response client
    
	log_trace("Exit: success");
	return RET_SUCCESS;
	
}

int handle_get_file_list_count_by_path_cmd(int  file_type,unsigned int *type_count,unsigned long *type_size,char *path,char *uuid)
{   
    uint32_t counts;
    Message *msg;
    
    ENTER_FUNC();
	
    // 1. get request params.
    DMCLOG_D("file_type(0x%x),path = %s", file_type,path);
	struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    // 2. query db.
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    // add end.

    msg->msg_header.m_type = MSG_DB_QUERY_FILE_LIST_CNT_BY_PATH; 
	
    get_db_opr_obj(msg).data.file_list.file_type = file_type;
	get_db_opr_obj(msg).data.file_list.path = path;
	//strcpy(get_db_opr_obj(msg).data.file_list.file_table_name,file_table_name);
	get_db_opr_obj(msg).ret = disk_info->g_db_query_task.msg_cb(&disk_info->g_db_query_task,msg);
   
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file full path failed");
        free_sync_message(&msg);
        return ECLIENT_PARAM;
		
    }

	*type_count = get_db_opr_obj(msg).data.file_list.result_cnt;
	*type_size = get_db_opr_obj(msg).data.file_list.result_size;
    DMCLOG_D("counts = %d,size = %lu", *type_count,*type_size);
	free_sync_message(&msg);
    
    // 3. Response client
    
	log_trace("Exit: success");
	return RET_SUCCESS;
	
}

int handle_query_file_cmd(char *src_path,char *disk_uuid)
{   
    uint32_t counts;
    Message *msg;
    uint32_t parent_id;
	int ret = 0;
    ENTER_FUNC();
	if(src_path == NULL)
	{
		return EMESSAGE_NEW;
	}
	struct disk_node *disk_info = get_disk_node(disk_uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    // 2. query db.
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    msg->msg_header.m_type = MSG_DB_QUERY_FILE_BY_NAME; 
	DMCLOG_D("src_path = %s",src_path);
	get_db_opr_obj(msg).data.file_data.path = src_path + strlen(DOCUMENT_ROOT);
	get_db_opr_obj(msg).data.file_data.name = bb_basename(src_path);
	
    ret = disk_info->g_db_query_task.msg_cb(&disk_info->g_db_query_task,msg);
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file full path failed");
        free_sync_message(&msg);
        return ECLIENT_PARAM;
		
    }
	free_sync_message(&msg);
    
    // 3. Response client
    
	log_trace("Exit: success");
	return RET_SUCCESS;
	
}


int handle_get_file_uuid_cmd(char *src_path,char *file_uuid,char *disk_uuid)
{   
    uint32_t counts;
    Message *msg;
	int ret = 0;
    ENTER_FUNC();
	if(src_path == NULL||file_uuid == NULL)
	{
		return EMESSAGE_NEW;
	}
	struct disk_node *disk_info = get_disk_node(disk_uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    // 2. query db.
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    msg->msg_header.m_type = MSG_DB_QUERY_FILE_BY_NAME; 
	DMCLOG_D("src_path = %s",src_path);
	get_db_opr_obj(msg).data.file_data.path = src_path + strlen(DOCUMENT_ROOT);
	get_db_opr_obj(msg).data.file_data.name = bb_basename(src_path);
	DMCLOG_D("name = %s",get_db_opr_obj(msg).data.file_data.name);
	
    ret = disk_info->g_db_query_task.msg_cb(&disk_info->g_db_query_task,msg);
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file full path failed");
        free_sync_message(&msg);
        return ECLIENT_PARAM;
		
    }
	char bIsRegularFile = get_db_opr_obj(msg).data.file_data.isFolder;
	if(bIsRegularFile != 2)
	{
		log_error("query back file full path failed");
		free_sync_message(&msg);
		return ECLIENT_PARAM;
	}
	strcpy(file_uuid,get_db_opr_obj(msg).data.file_data.file_uuid);
	free_sync_message(&msg);
    
    // 3. Response client
    
	log_trace("Exit: success");
	return RET_SUCCESS;
	
}

int handle_file_rename_cmd(char *src_path,char *des_path,char *uuid)
{   
    uint32_t counts;
    Message *msg;
	int ret = 0;
	uint32_t target_id = 0;
    ENTER_FUNC();
	//handle_get_file_id_cmd(src_path,&target_id) ;
    // 2. query db.
    struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    // add end.
	//file_info_t file_info;
    msg->msg_header.m_type = MSG_DB_FILE_RENAME; 

    //get_db_opr_obj(msg).data.rename_data.target_id = target_id;
    get_db_opr_obj(msg).data.rename_data.des_path = des_path + strlen(DOCUMENT_ROOT);
	get_db_opr_obj(msg).data.rename_data.src_path = src_path + strlen(DOCUMENT_ROOT);
	
	get_db_opr_obj(msg).data.rename_data.new_name = bb_basename(des_path);
	DMCLOG_D("new_name = %s",get_db_opr_obj(msg).data.rename_data.new_name);
	//strcpy(get_db_opr_obj(msg).data.rename_data.file_table_name,file_table_name);
    pthread_mutex_lock(&disk_info->mutex);
	disk_info->g_db_write_task.msg_cb(&disk_info->g_db_write_task,msg);
    pthread_mutex_unlock(&disk_info->mutex);
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file full path failed");
        free_sync_message(&msg);
        return ECLIENT_PARAM;
		
    }
	free_sync_message(&msg);
    
    // 3. Response client
    
	log_trace("Exit: success");
	return RET_SUCCESS;
	
}

int handle_file_insert_cmd(char *file_uuid,char *src_path,char bIsRegularFile,char *uuid)
{   
    Message *msg;
	int ret = 0;
    ENTER_FUNC();
    // 2. query db.
    if(uuid == NULL)
   	{
		log_error("para is NULL");
        return EMESSAGE_NEW;
	}
	
    DMCLOG_D("uuid = %s",uuid);
	struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		log_error("para is NULL");
        return EMESSAGE_NEW;
	}
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
	struct stat statbuf;
	if(lstat(src_path, &statbuf)) {
		//goto EXIT;
	}
    // add end.
	memset(&msg->msg_data, 0, sizeof(msg->msg_data));
    msg->msg_header.m_type = MSG_DB_FILE_SINGLE_ADD; 
    strcpy(get_db_opr_obj(msg).data.insert_data.file_info.mime_type,"NULL");
	if(file_uuid != NULL)
	{
		strcpy(get_db_opr_obj(msg).data.insert_data.file_info.file_uuid,file_uuid);
	}else
	{
		strcpy(get_db_opr_obj(msg).data.insert_data.file_info.file_uuid,"1234567890");
	}
    if(bIsRegularFile)
    {
        get_db_opr_obj(msg).data.insert_data.file_info.file_size= statbuf.st_size;
        get_db_opr_obj(msg).data.insert_data.file_info.isFolder = 0;
    }
    else
    {
        get_db_opr_obj(msg).data.insert_data.file_info.file_size = 0;
        get_db_opr_obj(msg).data.insert_data.file_info.isFolder = 1;
    }
	get_db_opr_obj(msg).data.insert_data.file_info.create_time = statbuf.st_ctime;
	get_db_opr_obj(msg).data.insert_data.file_info.modify_time = statbuf.st_mtime;
	get_db_opr_obj(msg).data.insert_data.file_info.access_time = statbuf.st_atime;
	get_db_opr_obj(msg).data.insert_data.file_info.path = src_path + strlen(DOCUMENT_ROOT);
	get_db_opr_obj(msg).data.insert_data.file_info.name = bb_basename(src_path);
	get_db_opr_obj(msg).data.insert_data.file_info.file_type = db_get_mime_type(src_path,strlen(src_path));
	
    pthread_mutex_lock(&disk_info->mutex);
	disk_info->g_db_write_task.msg_cb(&disk_info->g_db_write_task,msg);
    pthread_mutex_unlock(&disk_info->mutex);
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("handle_file_insert_cmd failed");
        free_sync_message(&msg);
        return ECLIENT_PARAM;
		
    }
	free_sync_message(&msg);
    // 3. Response client
    
	EXIT_FUNC();
	return RET_SUCCESS;
	
}

int handle_file_delete_cmd(char *src_path,char *uuid)
{   
    Message *msg;
    ENTER_FUNC();
	struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    // 2. query db.
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    // add end.
    msg->msg_header.m_type = MSG_DB_FILE_DELETE; 
	//get_db_opr_obj(msg).data.delete_data.target_id = target_id;
	get_db_opr_obj(msg).data.delete_data.path = src_path + strlen(DOCUMENT_ROOT);
	//strcpy(get_db_opr_obj(msg).data.delete_data.file_table_name,file_table_name);
	DMCLOG_D("path = %s",get_db_opr_obj(msg).data.delete_data.path);
    DMCLOG_D("del test lock in");
    pthread_mutex_lock(&disk_info->mutex);
	disk_info->g_db_write_task.msg_cb(&disk_info->g_db_write_task,msg);
    pthread_mutex_unlock(&disk_info->mutex);
    DMCLOG_D("del test lock out");
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file full path failed");
        free_sync_message(&msg);
        return ECLIENT_PARAM;
		
    }
	free_sync_message(&msg);
    
    // 3. Response client
    
	log_trace("Exit: success");
	return RET_SUCCESS;
	
}
#if 0
int handle_file_generic_delete_cmd(char *src_path,char *uuid)
{   
    Message *msg;
    ENTER_FUNC();
	struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    // 2. query db.
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    // add end.
    msg->msg_header.m_type = MSG_DB_FILE_DELETE; 
	//get_db_opr_obj(msg).data.delete_data.target_id = target_id;
	get_db_opr_obj(msg).data.delete_data.path = src_path + strlen(DOCUMENT_ROOT);
	//strcpy(get_db_opr_obj(msg).data.delete_data.file_table_name,file_table_name);
	DMCLOG_D("path = %s",get_db_opr_obj(msg).data.delete_data.path);
    DMCLOG_D("del test lock in");
    pthread_mutex_lock(&disk_info->mutex);
	disk_info->g_db_write_task.msg_cb(&disk_info->g_db_write_task,msg);
    pthread_mutex_unlock(&disk_info->mutex);
    DMCLOG_D("del test lock out");
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file full path failed");
        free_sync_message(&msg);
        return ECLIENT_PARAM;
		
    }
	free_sync_message(&msg);
    
    // 3. Response client
    
	log_trace("Exit: success");
	return RET_SUCCESS;
	
}
#endif
int handle_get_disk_info_cmd(char *uuid,disk_info_t *disk_info)
{   
    Message *msg;
    ENTER_FUNC();
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    msg->msg_header.m_type = MSG_DB_QUERY_HDISK_INFO; 
	strcpy(get_db_opr_obj(msg).data.hdisk_data.uuid,uuid);
	get_db_opr_obj(msg).data.hdisk_data.total_capacity = 0;
    if(send_message_sync(msg) != RET_SUCCESS)
    {
        log_error("send_message_sync failed");
        free_sync_message(&msg);
        return EMESSAGE_SEND;
    }
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query disk info failed");
        free_sync_message(&msg);
        return ECLIENT_PARAM;
		
    }
	if(get_db_opr_obj(msg).data.hdisk_data.total_capacity == 0)
	{
		log_error("query disk info is not exist");
        free_sync_message(&msg);
        return ECLIENT_PARAM;
	}
	DMCLOG_D("get_db_opr_obj(msg).data.hdisk_data.total_capacity = %llu",get_db_opr_obj(msg).data.hdisk_data.total_capacity);
	disk_info->total_size = get_db_opr_obj(msg).data.hdisk_data.total_capacity;
	disk_info->free_size = get_db_opr_obj(msg).data.hdisk_data.free_capacity;
	DMCLOG_D("disk_info->total_size = %llu",disk_info->total_size);
	DMCLOG_D("disk_info->free_size = %llu",disk_info->free_size);
	free_sync_message(&msg);
    // 3. Response client
    
	log_trace("Exit: success");
	return RET_SUCCESS;
	
}

int  handle_insert_disk_table(disk_info_t *disk_info)
{
	Message *msg;
    ENTER_FUNC();
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    msg->msg_header.m_type = MSG_DB_HDISK_INFO_ADD; 
	strcpy(get_db_opr_obj(msg).data.hdisk_data.uuid,disk_info->uuid);
	strcpy(get_db_opr_obj(msg).data.hdisk_data.pid,disk_info->pid);
	strcpy(get_db_opr_obj(msg).data.hdisk_data.vid,disk_info->vid);
	get_db_opr_obj(msg).data.hdisk_data.total_capacity = disk_info->total_size;
	get_db_opr_obj(msg).data.hdisk_data.free_capacity = disk_info->free_size;
	get_db_opr_obj(msg).data.hdisk_data.recycle_size = disk_info->total_size - disk_info->free_size;
	get_db_opr_obj(msg).data.hdisk_data.video_size = 0;
	get_db_opr_obj(msg).data.hdisk_data.audio_size = 0;
	get_db_opr_obj(msg).data.hdisk_data.photo_size = 0;
	get_db_opr_obj(msg).data.hdisk_data.mount_status = 1;
    if(send_message_sync(msg) != RET_SUCCESS)
    {
        log_error("send_message_sync failed");
        free_sync_message(&msg);
        return EMESSAGE_SEND;
    }
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("insert disk info failed");
        free_sync_message(&msg);
        return ECLIENT_PARAM;
		
    }
	free_sync_message(&msg);
    
    // 3. Response client
    
	log_trace("Exit: success");
	return RET_SUCCESS;
}


int handle_update_disk_table(disk_info_t *disk_info)
{
	Message *msg;
    ENTER_FUNC();
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    msg->msg_header.m_type = MSG_DB_HDISK_INFO_UPDATE; 
	strcpy(get_db_opr_obj(msg).data.hdisk_data.uuid,disk_info->uuid);
	get_db_opr_obj(msg).data.hdisk_data.total_capacity = disk_info->total_size;
	get_db_opr_obj(msg).data.hdisk_data.free_capacity = disk_info->free_size;
	get_db_opr_obj(msg).data.hdisk_data.recycle_size = disk_info->total_size - disk_info->free_size;
    if(send_message_sync(msg) != RET_SUCCESS)
    {
        log_error("send_message_sync failed");
        free_sync_message(&msg);
        return EMESSAGE_SEND;
    }
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("insert disk info failed");
        free_sync_message(&msg);
        return ECLIENT_PARAM;
		
    }
	free_sync_message(&msg);
    
    // 3. Response client
    
	log_trace("Exit: success");
	return RET_SUCCESS;
}

int dm_getTypeInfo(int cmd,int file_type,struct file_list *file_list_t,char *uuid,char *path)
{
	unsigned nfiles = 0;
	int res = 0;
	res = handle_get_file_list_count_cmd(cmd,file_list_t->file_type,&nfiles,uuid);
	if(res != 0)
	{
		DMCLOG_D("get file list count from db error");
		return -1;
	}
	DMCLOG_D("end:nfiles = %d",nfiles);
	file_list_t->totalCount = nfiles;
	file_list_t->pageSize = PAGESIZE;
	if(file_list_t->totalCount%PAGESIZE == 0)
		file_list_t->totalPage = file_list_t->totalCount/PAGESIZE;
	else
		file_list_t->totalPage = file_list_t->totalCount/PAGESIZE + 1;

	DMCLOG_D("file_list_t->totalCount = %d,file_list_t->pageSize = %d",file_list_t->totalCount,file_list_t->pageSize);
	DMCLOG_D("file_list_t->totalPage = %d",file_list_t->totalPage);
	return 0;
}

int dm_getTypeDirInfo(int file_type,struct file_list *file_list_t,char *uuid)
{
	unsigned nfiles = 0;
	int res = 0;
	res = handle_get_file_dir_list_count_cmd(file_list_t->file_type,&nfiles,uuid);
	if(res != 0)
	{
		DMCLOG_D("get file list count from db error");
		return -1;
	}
	DMCLOG_D("end:nfiles = %d",nfiles);
	file_list_t->totalCount = nfiles;
	file_list_t->pageSize = PAGESIZE;
	if(file_list_t->totalCount%PAGESIZE == 0)
		file_list_t->totalPage = file_list_t->totalCount/PAGESIZE;
	else
		file_list_t->totalPage = file_list_t->totalCount/PAGESIZE + 1;

	DMCLOG_D("file_list_t->totalCount = %d,file_list_t->pageSize = %d",file_list_t->totalCount,file_list_t->pageSize);
	DMCLOG_D("file_list_t->totalPage = %d",file_list_t->totalPage);
	return 0;
}


int dm_getTypeInfoByPath(int file_type,char *path,struct file_list *file_list_t,char *uuid)
{
	unsigned nfiles = 0;
	unsigned long nSizes = 0;
	int res = 0;
	DMCLOG_D("path = %s",path);
	res = handle_get_file_list_count_by_path_cmd(file_list_t->file_type,&nfiles,&nSizes,path,uuid);
	if(res != 0)
	{
		DMCLOG_D("get file list count from db error");
		return -1;
	}
	DMCLOG_D("end:nfiles = %d",nfiles);
	file_list_t->totalCount = nfiles;
	file_list_t->totalSize = nSizes;
	file_list_t->pageSize = PAGESIZE;
	if(file_list_t->totalCount%PAGESIZE == 0)
		file_list_t->totalPage = file_list_t->totalCount/PAGESIZE;
	else
		file_list_t->totalPage = file_list_t->totalCount/PAGESIZE + 1;

	DMCLOG_D("file_list_t->totalCount = %d,file_list_t->pageSize = %d",file_list_t->totalCount,file_list_t->pageSize);
	DMCLOG_D("file_list_t->totalPage = %d",file_list_t->totalPage);
	return 0;
}

/*!
 @method
 @abstractΩ´”√ªß–≈œ¢¥Ê»Î ˝æ›ø‚
 @param char session[32];
		char ip[32];
		char username[128];
		char password[128];
 @result INT(∑µªÿ÷µ¥Û”⁄µ»”⁄0±Ì æ≥…π¶£¨∑Ò‘Ú±Ì æ ß∞‹)
*/
int handle_db_login(char *session,char *deviceUuid,char *deviceName,char *ip,char *username,char *password)
{
	Message *msg;
	user_info_t *pui;
    ENTER_FUNC();
    // 2. query db.
    DMCLOG_D("g_db_write_task.msg_cb = %x",g_db_write_task.msg_cb);
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
	pui = &msg->msg_data.db_obj.data.user_data;
    // add end.
    msg->msg_header.m_type = MSG_DB_USER_INFO_ADD; 
	pui->user_name = username;
	pui->password = password;
	strcpy(pui->device_name,deviceName);
	strcpy(pui->device_uuid,deviceUuid);
	strcpy(pui->session,session);
	strcpy(pui->ip,ip);
	#if 0
   if(send_message_sync(msg) != RET_SUCCESS)
    {
        log_error("send_message_sync failed");
        free_sync_message(&msg);
        return EMESSAGE_SEND;
    }
   #else
	if(g_db_write_task.msg_cb != NULL)
   		get_db_opr_obj(msg).ret = g_db_write_task.msg_cb(&g_db_write_task, msg);
	else
		DMCLOG_D("msg_cb is null");
   #endif
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("insert disk info failed");
        free_sync_message(&msg);
        return ECLIENT_PARAM;
		
    }
	free_sync_message(&msg);
    
    // 3. Response client
    
	log_trace("Exit: success");
	return RET_SUCCESS;
}

int handle_db_logout(char *session)
{
	Message *msg;
	user_del_data_t *p_del_data;
    ENTER_FUNC();
    // 2. query db.
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
	p_del_data = &msg->msg_data.db_obj.data.user_del_data;
    // add end.
    msg->msg_header.m_type = MSG_DB_USER_INFO_DEL; 
	strcpy(p_del_data->session,session);
	memset(p_del_data->ip,0,sizeof(p_del_data->ip));

   if(g_db_write_task.msg_cb != NULL)
   		get_db_opr_obj(msg).ret = g_db_write_task.msg_cb(&g_db_write_task, msg);
   else
   		DMCLOG_D("g_db_write_task.msg_cb is null");
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("hidisk logout failed");
        free_sync_message(&msg);
        return ECLIENT_PARAM;
		
    }
	free_sync_message(&msg);
    
    // 3. Response client
    
	log_trace("Exit: success");
	return RET_SUCCESS;
}

int handle_db_del_usr_for_ip(const char *client_ip)
{
	Message *msg;
	user_del_data_t *p_del_data;
    ENTER_FUNC();
    // 2. query db.
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
	memset(&msg->msg_data, 0, sizeof(msg->msg_data));
	p_del_data = &msg->msg_data.db_obj.data.user_del_data;
    // add end.
    msg->msg_header.m_type = MSG_DB_USER_INFO_DEL; 
	strcpy(p_del_data->ip,client_ip);
	
    if(g_db_write_task.msg_cb != NULL)
   		get_db_opr_obj(msg).ret = g_db_write_task.msg_cb(&g_db_write_task, msg);
   
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("handle_db_del_usr_for_ip failed");
        free_sync_message(&msg);
        return ECLIENT_PARAM;
		
    }
	free_sync_message(&msg);
    
    // 3. Response client
    
	log_trace("Exit: success");
}

int get_ip_from_usr_table(_In_ const char *session,_Out_ char *ip)
{
	ENTER_FUNC();
    Message *msg = NULL;
    
    log_trace("Enter:");
    if(session == NULL)
    {
        DMCLOG_D("NULL point! We will exit client thread!");
        return ENULL_POINT;
    }

    // 1. query db
    if((msg = create_sync_message()) == NULL)
    {
        DMCLOG_D("create_sync_message failed");
        return EMESSAGE_NEW;
    }

    msg->msg_header.m_type = MSG_DB_QUERY_USER_INFO_BY_ID;
	DMCLOG_D("session = %s",session);
	strcpy(get_db_opr_obj(msg).data.user_data.session,session);
	#if 0
    if(send_message_sync(msg) != RET_SUCCESS)
    {
        DMCLOG_D("send_message_sync failed");
        free_sync_message(&msg);
        return EMESSAGE_SEND;
    }
	#else
	if(g_db_query_task.msg_cb != NULL)
   		get_db_opr_obj(msg).ret = g_db_query_task.msg_cb(&g_db_query_task, msg);
    else
   		DMCLOG_D("g_db_query_task.msg_cb is null");	
	#endif
    // check db handle if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        DMCLOG_D("query user info failed!");
        free_sync_message(&msg);
        return EDB;
    }
	strcpy(ip,get_db_opr_obj(msg).data.user_data.ip);
	DMCLOG_D("ip = %s",ip);
	free_sync_message(&msg);
    
    // 3. Response client
    
	log_trace("Exit: success");
	return RET_SUCCESS;
}

int get_devuuid_from_usr_table(_In_ const char *session,_Out_ char *device_uuid,_Out_ char *device_name)
{
    Message *msg = NULL;
    
    log_trace("Enter:");
    if(session == NULL || device_uuid == NULL||device_name == NULL)
    {
        log_error("NULL point! We will exit client thread!");
        return ENULL_POINT;
    }

    // 1. query db
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }

    msg->msg_header.m_type = MSG_DB_QUERY_USER_INFO_BY_ID;
	strcpy(get_db_opr_obj(msg).data.user_data.session,session);
	#if 0
    if(send_message_sync(msg) != RET_SUCCESS)
    {
        log_error("send_message_sync failed");
        free_sync_message(&msg);
        return EMESSAGE_SEND;
    }
	#else
	if(g_db_query_task.msg_cb != NULL)
   		get_db_opr_obj(msg).ret = g_db_query_task.msg_cb(&g_db_query_task, msg);
	#endif
    // check db handle if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query user info failed!");
        free_sync_message(&msg);
        return EDB;
    }

	strcpy(device_uuid,get_db_opr_obj(msg).data.user_data.device_uuid);
	strcpy(device_name,get_db_opr_obj(msg).data.user_data.device_name);
	
	DMCLOG_D("device_uuid = %s",device_uuid);
	free_sync_message(&msg);
    
    // 3. Response client
    
	log_trace("Exit: success");
	return RET_SUCCESS;
}


int bind_devuuid_to_diskuuid(_In_ const char *device_uuid,_In_ const char *device_name,_In_ const char *disk_uuid)
{
    Message *msg = NULL;
    
    ENTER_FUNC();
    if(!*device_uuid||!*device_name||!*disk_uuid)
    {
        log_error("NULL point! We will exit client thread!");
        return ENULL_POINT;
    }

    // 1. query db
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }

    msg->msg_header.m_type = MSG_DB_DEVICE_INFO_ADD;
	S_STRNCPY(get_db_opr_obj(msg).data.device_data.device_name, device_name, 64);
    S_STRNCPY(get_db_opr_obj(msg).data.device_data.device_uuid, device_uuid, 32);
	S_STRNCPY(get_db_opr_obj(msg).data.device_data.disk_uuid, disk_uuid, 16);
    DMCLOG_D("device_uuid = %s",get_db_opr_obj(msg).data.device_data.device_uuid);
	#if 0
    if(send_message_sync(msg) != RET_SUCCESS)
    {
        log_error("send_message_sync failed");
        free_sync_message(&msg);
        return EMESSAGE_SEND;
    }
	#else
	 if(g_db_write_task.msg_cb != NULL)
   		get_db_opr_obj(msg).ret = g_db_write_task.msg_cb(&g_db_write_task, msg);
	#endif
    // check db handle if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query user info failed!");
        free_sync_message(&msg);
        return EDB;
    }
	free_sync_message(&msg);
    
    // 3. Response client
    
	EXIT_FUNC();
	return RET_SUCCESS;
}

int get_bind_disk_uuid_from_db(_In_ const char *device_uuid,_Out_ const char *disk_uuid)
{
    ENTER_FUNC();
    device_list_t *pdevlist;
    int flag;
    device_info_t *pdi;
	Message *p_msg = NULL;
    
    log_trace("Enter:");
    if(!*device_uuid||disk_uuid == NULL)
    {
        log_error("NULL point! We will exit client thread!");
        return ENULL_POINT;
    }

    // 1. query db
    if((p_msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    memset(&p_msg->msg_data, 0, sizeof(p_msg->msg_data));
    // 1. get user devcie list from db
    p_msg->msg_header.m_type = MSG_DB_QUERY_DEVICE_BY_USER;
	strcpy(get_db_opr_obj(p_msg).data.device_data.device_uuid,device_uuid);
	DMCLOG_D("device_uuid = %s",device_uuid);
    //dl_list_init(&(get_db_opr_obj(p_msg).data.device_list.head));

   #if 0
    if(send_message_sync(msg) != RET_SUCCESS)
    {
        log_error("send_message_sync failed");
        free_sync_message(&msg);
        return EMESSAGE_SEND;
    }
	#else
	 if(g_db_query_task.msg_cb != NULL)
   		get_db_opr_obj(p_msg).ret = g_db_query_task.msg_cb(&g_db_query_task, p_msg);
	#endif

    // check db result if ok
    if(get_db_opr_obj(p_msg).ret != RET_SUCCESS)
    {
        log_error("get user device list from db failed");
		 free_sync_message(&p_msg);
        return EDB;
    }

	strcpy(disk_uuid,get_db_opr_obj(p_msg).data.device_data.disk_uuid);
	DMCLOG_D("disk_uuid = %s",disk_uuid);
	free_sync_message(&p_msg);
    EXIT_FUNC();
    return RET_SUCCESS;
}
int handle_file_uuid_exist_cmd(char *file_uuid,char *device_uuid,char *uuid,char **file_path)
{   
    uint32_t counts;
    Message *msg;
	int ret = 0;
	uint32_t target_id = 0;
    ENTER_FUNC();
	//handle_get_file_id_cmd(src_path,&target_id) ;
    // 2. query db.
    struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    // add end.
    msg->msg_header.m_type = MSG_DB_QUERY_BACKUP; 
    strcpy(get_db_opr_obj(msg).data.backup_file_data.file_uuid,file_uuid);
	strcpy(get_db_opr_obj(msg).data.backup_file_data.device_uuid,device_uuid);
    pthread_mutex_lock(&disk_info->mutex);
	ret = disk_info->g_db_query_task.msg_cb(&disk_info->g_db_query_task,msg);
    pthread_mutex_unlock(&disk_info->mutex);
    // check result if ok
    if(ret != RET_SUCCESS)
    {
        log_error("query fileuuid failed");
        free_sync_message(&msg);
        return ECLIENT_PARAM;
		
    }
	DMCLOG_D("path = %s",get_db_opr_obj(msg).data.backup_file_data.path);
	if(get_db_opr_obj(msg).data.backup_file_data.path != NULL&&*get_db_opr_obj(msg).data.backup_file_data.path)
	{
		*file_path = (char *)malloc(strlen(get_db_opr_obj(msg).data.backup_file_data.path) + 1);
		strcpy(*file_path,get_db_opr_obj(msg).data.backup_file_data.path);
	}

	if(get_db_opr_obj(msg).data.backup_file_data.file_status == 1)//Œƒº˛ªπ‘⁄±∏∑›÷–
	{
		free_sync_message(&msg);
		return 1;
	}
	free_sync_message(&msg);
    
    // 3. Response client
    //backup the file finished
	log_trace("Exit: success");
	return RET_SUCCESS;
	
}

int handle_file_list_uuid_exist_cmd(file_uuid_list_t *flist,JObj*response_para_json,char *disk_uuid)
{   
    uint32_t counts;
    Message *msg;
	int ret = 0;
    ENTER_FUNC();
    // 2. query db.
    struct disk_node *disk_info = get_disk_node(disk_uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    // add end.
    memset(&msg->msg_data, 0, sizeof(msg->msg_data));
	msg->msg_header.m_type = MSG_DB_QUERY_BACKUP; 
	file_uuid_t *p_file_uuid = NULL;
	file_uuid_t *n;
	if(flist->result_cnt > 0)
    {
        // handle query result.
    	dl_list_for_each(p_file_uuid, &(flist->head), file_uuid_t, next)
    	{
			DMCLOG_D("p_file_uuid->file_uuid = %s",p_file_uuid->file_uuid);	
			strcpy(get_db_opr_obj(msg).data.backup_file_data.file_uuid,p_file_uuid->file_uuid);
			strcpy(get_db_opr_obj(msg).data.backup_file_data.device_uuid,flist->device_uuid);
			ret = disk_info->g_db_query_task.msg_cb(&disk_info->g_db_query_task,msg);
			if(ret != RET_SUCCESS)
		    {
		        log_error("query fileuuid failed");
				p_file_uuid->backupFlag = 1;
		    }
    	}
    }
    // check result if ok
   
	if(flist->result_cnt > 0)
    {
        // handle query result.
        JObj *file_info_array = JSON_NEW_ARRAY();
    	dl_list_for_each(p_file_uuid, &(flist->head), file_uuid_t, next)
    	{
    		if(p_file_uuid->backupFlag == 1)
    		{
				JObj *file_info = JSON_NEW_EMPTY_OBJECT();
				JSON_ADD_OBJECT(file_info, "file_uuid",JSON_NEW_OBJECT(p_file_uuid->file_uuid,string));
				DMCLOG_D("file_uuid = %s",p_file_uuid->file_uuid);
				JSON_ARRAY_ADD_OBJECT (file_info_array,file_info);
			}
			
    	}
		JSON_ADD_OBJECT(response_para_json, "backupFileListInfo", file_info_array);
    }
    dl_list_for_each_safe(p_file_uuid, n, &flist->head, file_uuid_t, next)
    {
        dl_list_del(&p_file_uuid->next);
		safe_free(p_file_uuid);
        
    }
    safe_free(flist);
	free_sync_message(&msg);
    
    // 3. Response client
    //backup the file finished
	log_trace("Exit: success");
	return RET_SUCCESS;
	
}


#define get_msg_bk_info(msg) get_db_opr_obj(msg).data.backup_file_data

int handle_backfile_update_cmd(char *file_uuid,char *device_uuid,char file_status,char *uuid)
{   
    Message *msg;
	int ret = 0;
    ENTER_FUNC();
    // 2. query db.
	struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL||uuid == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    // add end.
	memset(&msg->msg_data, 0, sizeof(msg->msg_data));
   	msg->msg_header.m_type = MSG_DB_BACKUP_FILE_UPDATE;
    get_db_opr_obj(msg).data.backup_file_update_data.cmd = BACKUP_FILE_TABLE_UPDATE_STATE;
	snprintf(get_db_opr_obj(msg).data.backup_file_update_data.backup_info.file_uuid, MAX_USER_DEV_NAME_LEN, "%s", file_uuid);
	snprintf(get_db_opr_obj(msg).data.backup_file_update_data.backup_info.device_uuid, MAX_USER_DEV_NAME_LEN, "%s", device_uuid);
    get_db_opr_obj(msg).data.backup_file_update_data.backup_info.file_status = file_status;
    pthread_mutex_lock(&disk_info->mutex);
	disk_info->g_db_write_task.msg_cb(&disk_info->g_db_write_task,msg);
    pthread_mutex_unlock(&disk_info->mutex);
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("handle_file_insert_cmd failed");
        free_sync_message(&msg);
        return ECLIENT_PARAM;
		
    }
	free_sync_message(&msg);
    // 3. Response client
	EXIT_FUNC();
	return RET_SUCCESS;
	
}

int handle_backfile_delete_cmd(char *file_uuid,char *uuid)
{   
    Message *msg;
	int ret = 0;
    ENTER_FUNC();
    // 2. query db.
	struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL||uuid == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    // add end.
	memset(&msg->msg_data, 0, sizeof(msg->msg_data));
   	msg->msg_header.m_type = MSG_DB_BACKUP_FILE_DEL;
	snprintf(get_db_opr_obj(msg).data.backup_file_data.file_uuid, MAX_USER_DEV_NAME_LEN, "%s", file_uuid);
    pthread_mutex_lock(&disk_info->mutex);
	disk_info->g_db_write_task.msg_cb(&disk_info->g_db_write_task,msg);
    pthread_mutex_unlock(&disk_info->mutex);
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("handle_file_insert_cmd failed");
        free_sync_message(&msg);
        return ECLIENT_PARAM;
		
    }
	free_sync_message(&msg);
    // 3. Response client
	EXIT_FUNC();
	return RET_SUCCESS;
	
}

int handle_backfile_insert_cmd(struct conn *c,char file_status)
{   
    Message *msg;
	int ret = 0;
    ENTER_FUNC();
	if(c->des_path == NULL || c->file_uuid == NULL || c->deviceUuid == NULL)
	{
		log_error("para is null");
        return EMESSAGE_NEW;
	}
    // 2. query db.
	struct disk_node *disk_info = get_disk_node(c->disk_uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    // add end.
	memset(&msg->msg_data, 0, sizeof(msg->msg_data));
    msg->msg_header.m_type = MSG_DB_BACKUP_FILE_ADD; 
	snprintf(get_msg_bk_info(msg).file_uuid, MAX_USER_DEV_NAME_LEN, "%s", c->file_uuid);
	snprintf(get_msg_bk_info(msg).name, BACKUP_PATH_SIZE, "%s", bb_basename(c->des_path));
	snprintf(get_msg_bk_info(msg).path, 1024, "%s", c->des_path + strlen(DOCUMENT_ROOT));
    get_msg_bk_info(msg).size = c->fileSize;
    //snprintf(get_msg_bk_info(msg).device_name, MAX_USER_DEV_NAME_LEN, "%s", device_name);
	snprintf(get_msg_bk_info(msg).device_uuid, MAX_USER_DEV_NAME_LEN, "%s", c->deviceUuid);
    get_msg_bk_info(msg).backup_time = c->modifyTime;
	get_msg_bk_info(msg).file_type = db_get_mime_type(c->des_path,strlen(c->des_path));
	DMCLOG_D("file_type = %d",get_msg_bk_info(msg).file_type);
	get_msg_bk_info(msg).file_status = file_status;
	disk_info->g_db_write_task.msg_cb(&disk_info->g_db_write_task,msg);
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("handle_file_insert_cmd failed");
        free_sync_message(&msg);
        return ECLIENT_PARAM;
		
    }
	free_sync_message(&msg);
    // 3. Response client
    
	EXIT_FUNC();
	return RET_SUCCESS;
	
}

int handle_backfile_query_by_device_id(char *device_uuid,unsigned long long *deviceSize,unsigned long long *entireSize,const char *uuid)
{   
    Message *msg;
	int ret = 0;
    ENTER_FUNC();
	if(device_uuid == NULL||uuid == NULL)
		return EMESSAGE_NEW;
    // 2. query db.
	struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    if((msg = create_sync_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    //query
	memset(&msg->msg_data, 0, sizeof(msg->msg_data));
    msg->msg_header.m_type = MSG_DB_QUERY_BACKUP_DEVICE_SIZE;
	strcpy(get_db_opr_obj(msg).data.backup_list.device_uuid,device_uuid);
    get_db_opr_obj(msg).data.backup_list.type = BACKUP_TYPE_ALL;
	disk_info->g_db_write_task.msg_cb(&disk_info->g_db_write_task,msg);
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("handle_file_insert_cmd failed");
        free_sync_message(&msg);
        return ECLIENT_PARAM;
		
    }
	*deviceSize = get_db_opr_obj(msg).data.backup_list.result_device_size;
	*entireSize = get_db_opr_obj(msg).data.backup_list.result_entire_device_size;
	free_sync_message(&msg);
    // 3. Response client
    
	EXIT_FUNC();
	return RET_SUCCESS;
	
}

int db_backup_file_rm(const char *path,const char *disk_uuid)
{
    struct stat path_stat;

    if(stat(path,&path_stat) < 0)
    {
        return -1;
    }

    //directory
    if(S_ISDIR(path_stat.st_mode))
    {
        DIR *dp;
        struct dirent *d;
        //int status = 0;

        dp = opendir(path);
        if (dp == NULL)
        {
    	    return -1;
        }

        while((d = readdir(dp)) != NULL)
        {
            char *new_path;
            if(strcmp(d->d_name, ".") == 0
              || strcmp(d->d_name, "..") == 0)
            {
                continue;
            }
 
            new_path = concat_path_file(path, d->d_name);
            if(db_backup_file_rm(new_path,disk_uuid) < 0)
            {
                printf("remove file %s failed,errno = %d\n", new_path,errno);
                free(new_path);
                closedir(dp);
                return -1;
            }
            free(new_path);
        } 
        closedir(dp);
        return 0; 
    }
    //regular file
    char file_uuid[32];
	memset(file_uuid,0,32);
    int ret = handle_get_file_uuid_cmd(path,file_uuid,disk_uuid);
	if(ret == 0)
	{
		if((ret = handle_backfile_delete_cmd(file_uuid,disk_uuid)) != RET_SUCCESS)
		{
			DMCLOG_D("del back file error");
			return -1;
		}
	}
    return 0;
}


