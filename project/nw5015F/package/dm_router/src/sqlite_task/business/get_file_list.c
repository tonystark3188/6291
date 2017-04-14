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
//#include "network/net_util.h"
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
    if((msg = new_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
	msg->msg_header.ret_flag = MSG_NO_RETURN;
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
        free_message(&msg);
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

    free_message(&msg);
    log_trace("Exit: success");
    return RET_SUCCESS;

_HANDLE_CMD_FAILED_:
    
    dl_list_for_each_safe(p_file_info, n, &(get_db_opr_obj(msg).data.file_list.head), file_info_t, next)
    {
        free_db_fd(&p_file_info);
    }
    
    free_message(&msg);
    log_trace("Exit: failed!");
    return EJSON_NEW;
}

int	_handle_get_all_type_file_list_cmd(struct conn *c)
{ 
	c->msg->msg_header.ret_flag = MSG_NO_RETURN;
	if(c->cmd == 120||c->cmd == 126)
    	c->msg->msg_header.m_type = MSG_DB_QUERY_FILE_LIST;
	else if(c->cmd == 123)
		c->msg->msg_header.m_type = MSG_DB_QUERY_DIR_LIST;
	else if(c->cmd == 124)
	{
		DMCLOG_D("c->src_path: %s, c->disk_info.path: %s", c->src_path, c->disk_info->path);
		if(strstr(c->src_path, c->disk_info->path) == NULL){
			return RET_SUCCESS;
		}
		c->msg->msg_header.m_type = MSG_DB_QUERY_FILE_LIST_BY_PATH;
		get_db_opr_obj(c->msg).data.file_list.path = c->src_path != NULL?c->src_path + strlen(DOCUMENT_ROOT):NULL;
	}
	get_db_opr_obj(c->msg).data.file_list.list_type = TYPE_ALL;
    get_db_opr_obj(c->msg).data.file_list.file_type = c->fileType;
	get_db_opr_obj(c->msg).data.file_list.start_index = c->offset;
	get_db_opr_obj(c->msg).data.file_list.len = c->length;
	get_db_opr_obj(c->msg).data.file_list.sort_mode = c->sortType;
	pthread_mutex_lock(&c->disk_info->mutex);
	if(get_fuser_flag() == AIRDISK_ON_PC)
	{
		DMCLOG_D("airdisk on pc");
		c->rem.flags |= FLAG_CLOSED;
		c->loc.flags |= FLAG_CLOSED;
		c->loc.flags &= ~(FLAG_R | FLAG_W | FLAG_ALWAYS_READY);
		pthread_mutex_unlock(&c->disk_info->mutex);
		return EJSON_NEW;
	}
   	get_db_opr_obj(c->msg).ret = c->disk_info->g_db_query_task.msg_cb(&c->disk_info->g_db_query_task,c->msg);
	pthread_mutex_unlock(&c->disk_info->mutex);
	if(get_db_opr_obj(c->msg).ret != RET_SUCCESS)
    {
        DMCLOG_E("query file list failed");
        return EJSON_NEW;
    }
    c->count += get_db_opr_obj(c->msg).data.file_list.result_cnt;
	c->totalCount += get_db_opr_obj(c->msg).data.file_list.total_cnt;
	DMCLOG_D("count = %u,c->totalCount = %u",c->count,c->totalCount);
    return RET_SUCCESS;
}

int	_handle_get_all_type_dir_list_cmd(struct conn *c)
{ 
	ENTER_FUNC();
	c->msg->msg_header.ret_flag = MSG_NO_RETURN;
	c->msg->msg_header.m_type = MSG_DB_QUERY_DIR_LIST;
	get_db_opr_obj(c->msg).data.file_list.list_type = TYPE_ALL;
    get_db_opr_obj(c->msg).data.file_list.file_type = c->fileType;
	get_db_opr_obj(c->msg).data.file_list.sort_mode = c->sortType;
	pthread_mutex_lock(&c->disk_info->mutex);
	if(get_fuser_flag() == AIRDISK_ON_PC)
	{
		DMCLOG_D("airdisk on pc");
		c->rem.flags |= FLAG_CLOSED;
		c->loc.flags |= FLAG_CLOSED;
		c->loc.flags &= ~(FLAG_R | FLAG_W | FLAG_ALWAYS_READY);
		pthread_mutex_unlock(&c->disk_info->mutex);
		return EJSON_NEW;
	}
   	get_db_opr_obj(c->msg).ret = c->disk_info->g_db_query_task.msg_cb(&c->disk_info->g_db_query_task,c->msg);
	pthread_mutex_unlock(&c->disk_info->mutex);
	if(get_db_opr_obj(c->msg).ret != RET_SUCCESS)
    {
        DMCLOG_E("query dir list failed");
        return EJSON_NEW;
    }
    c->count += get_db_opr_obj(c->msg).data.file_list.result_cnt;
	DMCLOG_D("count = %u",c->count);
	EXIT_FUNC();
    return RET_SUCCESS;
}


int handle_get_file_list_count_by_path_cmd(int  file_type,unsigned int *type_count,unsigned long *type_size,char *path,char *uuid)
{   
    uint32_t counts;
    Message *msg;
    
    //ENTER_FUNC();
	
    // 1. get request params.
    //DMCLOG_D("file_type(0x%x),path = %s", file_type,path);
	struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    // 2. query db.
    if((msg = new_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    // add end.
	msg->msg_header.ret_flag = MSG_NO_RETURN;
    msg->msg_header.m_type = MSG_DB_QUERY_FILE_LIST_CNT_BY_PATH; 
	
    get_db_opr_obj(msg).data.file_list.file_type = file_type;
	get_db_opr_obj(msg).data.file_list.path = path;
	get_db_opr_obj(msg).ret = disk_info->g_db_query_task.msg_cb(&disk_info->g_db_query_task,msg);
   
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file full path failed");
        free_message(&msg);
        return ECLIENT_PARAM;
		
    }

	*type_count = get_db_opr_obj(msg).data.file_list.result_cnt;
	//*type_size = get_db_opr_obj(msg).data.file_list.result_size;
    //DMCLOG_D("counts = %d,size = %lu", *type_count,*type_size);
	free_message(&msg);
    
    // 3. Response client
    
	//log_trace("Exit: success");
	return RET_SUCCESS;
	
}

int handle_get_file_list_count_by_path(int  file_type,unsigned int *type_count,off_t *type_size,char *path,struct disk_node *disk_info )
{   
    uint32_t counts;
    Message *msg;
    
    //ENTER_FUNC();
	
    // 1. get request params.
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    // 2. query db.
    if((msg = new_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    // add end.
	msg->msg_header.ret_flag = MSG_NO_RETURN;
    msg->msg_header.m_type = MSG_DB_QUERY_FILE_LIST_CNT_BY_PATH; 
	
    get_db_opr_obj(msg).data.file_list.file_type = file_type;
	get_db_opr_obj(msg).data.file_list.path = path;
	get_db_opr_obj(msg).ret = disk_info->g_db_query_task.msg_cb(&disk_info->g_db_query_task,msg);
   
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file full path failed");
        free_message(&msg);
        return ECLIENT_PARAM;
		
    }

	*type_count = get_db_opr_obj(msg).data.file_list.result_cnt;
	//*type_size = get_db_opr_obj(msg).data.file_list.result_size;
    //DMCLOG_D("counts = %d,size = %lu", *type_count,*type_size);
	free_message(&msg);
    
    // 3. Response client
    
	//log_trace("Exit: success");
	return RET_SUCCESS;
	
}


int	_handle_get_file_list_for_album_cmd(int sort_mode,char **buf,struct conn *c,char *path,struct disk_node *disk_info)
{ 
    Message *msg;
	int ret = RET_SUCCESS;
	file_info_t *p_file_info = NULL;
	file_info_t *nn = NULL;
	int			len ,n, nwritten = 0;
	char		line[1024];
	int i = 0;

    // 2. query db.
    if((msg = new_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
	msg->msg_header.ret_flag = MSG_NO_RETURN;
	get_db_opr_obj(msg).data.file_list.list_type = TYPE_UNKNOWN;
	msg->msg_header.m_type = MSG_DB_QUERY_FILE_LIST_BY_PATH;
	get_db_opr_obj(msg).data.file_list.path = path;
		
    get_db_opr_obj(msg).data.file_list.file_type = c->fileType;
	get_db_opr_obj(msg).data.file_list.sort_mode = sort_mode;
	get_db_opr_obj(msg).data.file_list.start_index = 0;
	get_db_opr_obj(msg).data.file_list.len = 4;
    get_db_opr_obj(msg).data.file_list.result_cnt = 0;
	


   	get_db_opr_obj(msg).ret = disk_info->g_db_query_task.msg_cb(&disk_info->g_db_query_task,msg);
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file list failed");
        goto _HANDLE_CMD_FAILED_;
    }
    len = get_db_opr_obj(msg).data.file_list.result_cnt;
	//DMCLOG_D("len = %d",len);
    if(len > 0)
    {
        // handle query result.
    	dl_list_for_each(p_file_info, &(get_db_opr_obj(msg).data.file_list.head), file_info_t, next)
    	{
			if(i == 0)
			{
				n = my_snprintf(line, sizeof(line),
			    "[{\"name\": \"%s\"}",p_file_info->name);
			}else{
				n = my_snprintf(line, sizeof(line),
			    ",{\"name\": \"%s\"}",p_file_info->name);
			}
			i++;
			(void) memcpy(*buf, line, n);
			*buf = (char *) *buf + n;
			nwritten += n;
    	}
		n = my_snprintf(line, sizeof(line),"]}");
		(void) memcpy(*buf, line, n);
		*buf = (char *) *buf + n;
		nwritten += n;
    }
    dl_list_for_each_safe(p_file_info, nn, &(get_db_opr_obj(msg).data.file_list.head), file_info_t, next)
    {
        free_db_fd(&p_file_info);
    }
    
    free_message(&msg);
    return nwritten;

_HANDLE_CMD_FAILED_:
    
    dl_list_for_each_safe(p_file_info, nn, &(get_db_opr_obj(msg).data.file_list.head), file_info_t, next)
    {
        free_db_fd(&p_file_info);
    }
    
    free_message(&msg);
    EXIT_FUNC();
    return EJSON_NEW;
}

int handle_del_file_list_by_path_cmd(int file_type,char *path,char *uuid)
{   
    Message *msg;
    ENTER_FUNC();
	struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    DMCLOG_D("file_type(0x%x))",  file_type);

    // 2. query db.
    if((msg = new_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
	delete_data_t *pdel_data = &get_db_opr_obj(msg).data.delete_data;
	msg->msg_header.ret_flag = MSG_NO_RETURN;
    msg->msg_header.m_type = MSG_DB_FILE_DEL_LIST_BY_PATH; 
	get_db_opr_obj(msg).data.delete_data.path = path + strlen(DOCUMENT_ROOT);
	get_db_opr_obj(msg).data.delete_data.file_type = file_type;
	get_db_opr_obj(msg).data.delete_data.cmd = FILE_TABLE_DELETE_TYPE_BY_PATH;
	DMCLOG_D("path = %s",get_db_opr_obj(msg).data.delete_data.path);
    pthread_mutex_lock(&disk_info->mutex);
	disk_info->g_db_write_task.msg_cb(&disk_info->g_db_write_task,msg);
    pthread_mutex_unlock(&disk_info->mutex);
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file list failed");
        free_message(&msg);
        return EDB;
    }
    free_message(&msg);
    return 0;
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
    if((msg = new_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    // add end.
    msg->msg_header.ret_flag = MSG_NO_RETURN;
	if(cmd == 120)
    	msg->msg_header.m_type = MSG_DB_QUERY_FILE_LIST_CNT;
	else if(cmd == 123 || cmd == 126)
		msg->msg_header.m_type = MSG_DB_QUERY_FILE_DIR_LIST_CNT;
	
    get_db_opr_obj(msg).data.file_list.file_type = file_type;
	//strcpy(get_db_opr_obj(msg).data.file_list.file_table_name,file_table_name);
	get_db_opr_obj(msg).ret = disk_info->g_db_query_task.msg_cb(&disk_info->g_db_query_task,msg);
   
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file full path failed");
        free_message(&msg);
        return ECLIENT_PARAM;
		
    }

	*type_count = get_db_opr_obj(msg).data.file_list.result_cnt;
    DMCLOG_D("counts = %d", *type_count);
	free_message(&msg);
    
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
    if((msg = new_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    // add end.
	msg->msg_header.ret_flag = MSG_NO_RETURN;
    msg->msg_header.m_type = MSG_DB_QUERY_FILE_DIR_LIST_CNT; 
	
    get_db_opr_obj(msg).data.file_list.file_type = file_type;
	//strcpy(get_db_opr_obj(msg).data.file_list.file_table_name,file_table_name);
	get_db_opr_obj(msg).ret = disk_info->g_db_query_task.msg_cb(&disk_info->g_db_query_task,msg);
   
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file full path failed");
        free_message(&msg);
        return ECLIENT_PARAM;
		
    }

	*type_count = get_db_opr_obj(msg).data.file_list.result_cnt;
    DMCLOG_D("counts = %d", *type_count);
	free_message(&msg);
    
    // 3. Response client
    
	log_trace("Exit: success");
	return RET_SUCCESS;
	
}

int handle_file_rename_cmd(char *src_path,char *des_path,char *uuid)
{   
    Message *msg;
    struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    if((msg = new_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
	
	msg->msg_header.ret_flag = MSG_NO_RETURN;
    msg->msg_header.m_type = MSG_DB_FILE_RENAME; 

    get_db_opr_obj(msg).data.rename_data.des_path = des_path + strlen(DOCUMENT_ROOT);
	get_db_opr_obj(msg).data.rename_data.src_path = src_path + strlen(DOCUMENT_ROOT);
	
	get_db_opr_obj(msg).data.rename_data.new_name = bb_basename(des_path);
	DMCLOG_D("new_name = %s",get_db_opr_obj(msg).data.rename_data.new_name);
    pthread_mutex_lock(&disk_info->mutex);
	disk_info->g_db_write_task.msg_cb(&disk_info->g_db_write_task,msg);
    pthread_mutex_unlock(&disk_info->mutex);
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file full path failed");
        free_message(&msg);
        return ECLIENT_PARAM;
		
    }
	free_message(&msg);
    
    // 3. Response client
	return RET_SUCCESS;
}

int handle_file_hide_cmd(char *src_path,bool attr,char *uuid,bool album)
{   
    Message *msg;
    struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    if((msg = new_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
	
	msg->msg_header.ret_flag = MSG_NO_RETURN;
    msg->msg_header.m_type = MSG_DB_FILE_UPDATE; 

	get_db_opr_obj(msg).data.file_update_data.file_info.path = src_path + strlen(DOCUMENT_ROOT);
	get_db_opr_obj(msg).data.file_update_data.file_info.attr = attr;
	if(album == true)
	{
		get_db_opr_obj(msg).data.file_update_data.cmd |= FILE_TABLE_UPDATE_ALBUM_HIDE;
	}else{
		get_db_opr_obj(msg).data.file_update_data.cmd |= FILE_TABLE_UPDATE_HIDE;
	}
	
	DMCLOG_D("path = %s,attr = %d",src_path,attr);
    pthread_mutex_lock(&disk_info->mutex);
	disk_info->g_db_write_task.msg_cb(&disk_info->g_db_write_task,msg);
    pthread_mutex_unlock(&disk_info->mutex);
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        log_error("query file full path failed");
        free_message(&msg);
        return ECLIENT_PARAM;
		
    }
	free_message(&msg);
    
    // 3. Response client
	return RET_SUCCESS;
}


int handle_file_insert_cmd(char *file_uuid,char *src_path,char bIsRegularFile,char *uuid)
{   
	ENTER_FUNC();
    Message *msg;	
	if(file_uuid == NULL||src_path == NULL||uuid == NULL)
	{
		DMCLOG_E("para is NULL");
        return EMESSAGE_NEW;
	}
    
    DMCLOG_D("uuid = %s",uuid);
	struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		DMCLOG_E("para is NULL");
        return EMESSAGE_NEW;
	}
    if((msg = new_message()) == NULL)
    {
        DMCLOG_E("create_sync_message failed");
        return EMESSAGE_NEW;
    }
	struct stat statbuf;
	lstat(src_path, &statbuf);
	memset(&msg->msg_data, 0, sizeof(msg->msg_data));
	msg->msg_header.ret_flag = MSG_NO_RETURN;
    msg->msg_header.m_type = MSG_DB_FILE_SINGLE_ADD; 
    strcpy(get_db_opr_obj(msg).data.insert_data.file_info.mime_type,"NULL");
	if(file_uuid != NULL&&*file_uuid)
	{
		S_STRNCPY(get_db_opr_obj(msg).data.insert_data.file_info.file_uuid,file_uuid,FILE_UUID_LEN);
	}else
	{
		strcpy(get_db_opr_obj(msg).data.insert_data.file_info.file_uuid,GENERAL_FILE_UUID);
	}
	DMCLOG_D("bIsRegularFile = %d",bIsRegularFile);
    if(bIsRegularFile)
    {
    	DMCLOG_D("path = %s,file_size = %lld",src_path,statbuf.st_size);
    	//��ͨ�ļ�
    	if(bIsRegularFile == 2)//��ʼ���ݣ����ļ���������Ϊ-1
    	{
			get_db_opr_obj(msg).data.insert_data.file_info.file_type = -1;
		}else{
			get_db_opr_obj(msg).data.insert_data.file_info.file_type = db_get_mime_type(src_path,strlen(src_path));
		}
        get_db_opr_obj(msg).data.insert_data.file_info.file_size= statbuf.st_size;
        get_db_opr_obj(msg).data.insert_data.file_info.isFolder = 0;
    }
    else
    {
    	//�ļ���
        get_db_opr_obj(msg).data.insert_data.file_info.file_size = 0;
        get_db_opr_obj(msg).data.insert_data.file_info.isFolder = 1;
    }
	get_db_opr_obj(msg).data.insert_data.file_info.create_time = statbuf.st_ctime;
	get_db_opr_obj(msg).data.insert_data.file_info.modify_time = statbuf.st_mtime;
	get_db_opr_obj(msg).data.insert_data.file_info.access_time = statbuf.st_atime;
	get_db_opr_obj(msg).data.insert_data.file_info.path = src_path + strlen(DOCUMENT_ROOT);
	get_db_opr_obj(msg).data.insert_data.file_info.name = bb_basename(src_path);
	DMCLOG_D("path = %s,file_size = %lld",src_path,statbuf.st_size);
    pthread_mutex_lock(&disk_info->mutex);
	disk_info->g_db_write_task.msg_cb(&disk_info->g_db_write_task,msg);
    pthread_mutex_unlock(&disk_info->mutex);
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        DMCLOG_E("handle_file_insert_cmd failed");
        free_message(&msg);
        return ECLIENT_PARAM;
		
    }
	free_message(&msg);
    // 3. Response client
    
	EXIT_FUNC();
	return RET_SUCCESS;
	
}

int handle_file_delete_cmd(char *src_path,char *uuid)
{   
    Message *msg;
	struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		DMCLOG_E("get the disk info failed");
        return EMESSAGE_NEW;
	}
    // 2. query db.
    if((msg = new_message()) == NULL)
    {
        DMCLOG_E("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    // add end.
    msg->msg_header.ret_flag = MSG_NO_RETURN;
    msg->msg_header.m_type = MSG_DB_FILE_DELETE; 
	get_db_opr_obj(msg).data.delete_data.path = src_path + strlen(DOCUMENT_ROOT);
	get_db_opr_obj(msg).data.delete_data.cmd = FILE_TABLE_DELETE_INFO;
	DMCLOG_D("path = %s",get_db_opr_obj(msg).data.delete_data.path);
    pthread_mutex_lock(&disk_info->mutex);
	disk_info->g_db_write_task.msg_cb(&disk_info->g_db_write_task,msg);
    pthread_mutex_unlock(&disk_info->mutex);
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        DMCLOG_E("query file full path failed");
        free_message(&msg);
        return ECLIENT_PARAM;
		
    }
	free_message(&msg);
	return RET_SUCCESS;
	
}

int handle_file_list_delete_cmd(char **file_list,char *uuid)
{   
    Message *msg;
	struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		DMCLOG_E("get the disk info failed");
        return EMESSAGE_NEW;
	}
    // 2. query db.
    if((msg = new_message()) == NULL)
    {
        DMCLOG_E("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    // add end.
    msg->msg_header.ret_flag = MSG_NO_RETURN;
    msg->msg_header.m_type = MSG_DB_FILE_DELETE; 
	get_db_opr_obj(msg).data.delete_data.cmd = FILE_TABLE_DELETE_LIST;
	get_db_opr_obj(msg).data.delete_data.file_list = file_list;
    pthread_mutex_lock(&disk_info->mutex);
	disk_info->g_db_write_task.msg_cb(&disk_info->g_db_write_task,msg);
    pthread_mutex_unlock(&disk_info->mutex);
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        DMCLOG_E("query file full path failed");
        free_message(&msg);
        return ECLIENT_PARAM;
		
    }
	free_message(&msg);
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
	if(g_db_write_task.msg_cb != NULL)
   		get_db_opr_obj(msg).ret = g_db_write_task.msg_cb(&g_db_write_task, msg);
	else
		DMCLOG_D("msg_cb is null");
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
	if(g_db_query_task.msg_cb != NULL)
   		get_db_opr_obj(msg).ret = g_db_query_task.msg_cb(&g_db_query_task, msg);
    else
   		DMCLOG_D("g_db_query_task.msg_cb is null");	
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
	if(g_db_query_task.msg_cb != NULL)
   		get_db_opr_obj(msg).ret = g_db_query_task.msg_cb(&g_db_query_task, msg);
	if(get_db_opr_obj(msg).ret != RET_SUCCESS)
	{
	    log_error("query user info failed!");
	    free_sync_message(&msg);
	    return EDB;
	}
	S_STRNCPY(device_uuid,get_db_opr_obj(msg).data.user_data.device_uuid,FILE_UUID_LEN);
	S_STRNCPY(device_name,get_db_opr_obj(msg).data.user_data.device_name,DEVICE_NAME_LEN);
	DMCLOG_D("device_uuid = %s",device_uuid);
	DMCLOG_D("device_name = %s",device_name);
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
	S_STRNCPY(get_db_opr_obj(msg).data.device_data.device_name, device_name, MAX_USER_DEV_NAME_LEN - 1);
    S_STRNCPY(get_db_opr_obj(msg).data.device_data.device_uuid, device_uuid, 64);
	S_STRNCPY(get_db_opr_obj(msg).data.device_data.disk_uuid, disk_uuid, 16);
    DMCLOG_D("device_uuid = %s",get_db_opr_obj(msg).data.device_data.device_uuid);
	 if(g_db_write_task.msg_cb != NULL)
   		get_db_opr_obj(msg).ret = g_db_write_task.msg_cb(&g_db_write_task, msg);
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

int get_bind_disk_uuid_from_db(_In_ const char *device_uuid,_Out_ char *disk_uuid)
{
    ENTER_FUNC();
    device_list_t *pdevlist;
    int flag;
    device_info_t *pdi;
	Message *p_msg = NULL;
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

	 if(g_db_query_task.msg_cb != NULL)
   		get_db_opr_obj(p_msg).ret = g_db_query_task.msg_cb(&g_db_query_task, p_msg);

    // check db result if ok
    if(get_db_opr_obj(p_msg).ret != RET_SUCCESS)
    {
        log_error("get user device list from db failed");
		free_sync_message(&p_msg);
        return EDB;
    }

	S_STRNCPY(disk_uuid,get_db_opr_obj(p_msg).data.device_data.disk_uuid,DISK_UUID_LEN);
	DMCLOG_D("disk_uuid = %s",disk_uuid);
	free_sync_message(&p_msg);
    EXIT_FUNC();
    return RET_SUCCESS;
}

int handle_query_file_path_by_uuid(char *file_uuid,char *device_uuid,char *uuid,char **file_path)
{   
    Message *msg;
	int ret = 0;
    ENTER_FUNC();
    // 2. query db.
    struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
    if((msg = new_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    // add end.
    msg->msg_header.ret_flag = MSG_NO_RETURN;
    msg->msg_header.m_type = MSG_DB_QUERY_FILE_BY_UUID; 
    strcpy(get_db_opr_obj(msg).data.file_data.file_uuid,file_uuid);
	pthread_mutex_lock(&disk_info->mutex);
	ret = disk_info->g_db_query_task.msg_cb(&disk_info->g_db_query_task,msg);
	pthread_mutex_unlock(&disk_info->mutex);
    // check result if ok
    if(ret != RET_SUCCESS)
    {
        log_error("query fileuuid failed");
        free_message(&msg);
        return ECLIENT_PARAM;
    }
	DMCLOG_D("path = %s",get_db_opr_obj(msg).data.file_data.path);
	if(get_db_opr_obj(msg).data.file_data.path != NULL)
	{
		*file_path = get_db_opr_obj(msg).data.file_data.path;
		DMCLOG_D("path = %s",*file_path);
		
	}
	
	//DMCLOG_D("Exit, m_ptr=0x%x", (unsigned int)(msg));
	free_message(&msg);
	return RET_SUCCESS;
	
}

int handle_file_uuid_exist_cmd(char *file_uuid,char *device_uuid,char *uuid,int *file_type)
{   
    Message *msg;
	int ret = 0;
    ENTER_FUNC();
    // 2. query db.
    struct disk_node *disk_info = get_disk_node(uuid);
	if(disk_info == NULL)
	{
		log_error("get the disk info failed");
        return EMESSAGE_NEW;
	}
	
    if((msg = new_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    // add end.
    msg->msg_header.ret_flag = MSG_NO_RETURN;
    msg->msg_header.m_type = MSG_DB_QUERY_FILE_BY_UUID; 
    strcpy(get_db_opr_obj(msg).data.file_data.file_uuid,file_uuid);
	pthread_mutex_lock(&disk_info->mutex);
	ret = disk_info->g_db_query_task.msg_cb(&disk_info->g_db_query_task,msg);
	pthread_mutex_unlock(&disk_info->mutex);
    // check result if ok
    if(ret != RET_SUCCESS)
    {
        log_error("query fileuuid failed");
        free_message(&msg);
        return ECLIENT_PARAM;
    }
	DMCLOG_D("file_type = %d",get_db_opr_obj(msg).data.file_data.file_type);
	*file_type = get_db_opr_obj(msg).data.file_data.file_type;
	free_message(&msg);
    ENTER_FUNC();
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
    if((msg = new_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    // add end.
    memset(&msg->msg_data, 0, sizeof(msg->msg_data));
	msg->msg_header.ret_flag = MSG_NO_RETURN;
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
	free_message(&msg);
    
    // 3. Response client
    //backup the file finished
	log_trace("Exit: success");
	return RET_SUCCESS;
	
}
