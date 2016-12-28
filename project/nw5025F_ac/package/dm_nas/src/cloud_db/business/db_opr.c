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
#include "mo.h"
#include "mq.h"
#include "db_opr.h"
#include "db_task.h"


/************************************************register**************************************************************/
/**
 *	insert item to user table when register
 */
ERROR_CODE_VFS handle_user_table_insert(_In_ char *username,_In_ char *password,_Out_ int *user_id)
{
	ENTER_FUNC();
    // 2. query db.
    if(username == NULL||password == NULL)
   	{
		return FAIL;
	}
    Message * msg = create_sync_message();
	assert(msg != NULL);
	msg->msg_data.db_obj.data.insert_data.user_insert.cmd = USER_TABLE_INSERT_INFO;
	user_info_t *pui = &msg->msg_data.db_obj.data.insert_data.user_insert.user_info;
    msg->msg_header.m_type = MSG_DB_INSERT_USER; 
    S_STRNCPY(pui->user_name,username,MAX_USER_NAME_LEN);
	S_STRNCPY(pui->password,password,MAX_PASSWORD_LEN);
   	get_db_opr_obj(msg).ret = g_db_write_task.msg_cb(&g_db_write_task, msg);

    // check result if ok
    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("insert disk info failed");
        free_sync_message(&msg);
        return get_db_opr_obj(msg).ret;
		
    }
	*user_id = pui->user_id;
	DMCLOG_D("pui->user_id: %d",pui->user_id);
	free_sync_message(&msg);
    
    // 3. Response client
	return ACTION_SUCCESS;
}

/*
*	insert item to bucket table when register
*/
ERROR_CODE_VFS handle_bucket_table_insert(_In_ char *bucket_name,_In_ int creat_user_id,_Out_ int *bucket_id)
{
	ENTER_FUNC();
    // 2. query db.
    if(bucket_name == NULL)
   	{
		return FAIL;
	}
    Message *msg = create_sync_message();
	assert(msg != NULL);
	msg->msg_data.db_obj.data.insert_data.bucket_insert.cmd = BUCKET_TABLE_INSERT_INFO;
	bucket_info_t *pui = &msg->msg_data.db_obj.data.insert_data.bucket_insert.bucket_info;
    msg->msg_header.m_type = MSG_DB_INSERT_BUCKET; 
    S_STRNCPY(pui->bucket_table_name,bucket_name,MAX_BUCKET_NAME_LEN);
	pui->create_user_id = creat_user_id;
   	get_db_opr_obj(msg).ret = g_db_write_task.msg_cb(&g_db_write_task, msg);

    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("insert disk info failed");
        free_sync_message(&msg);
        return get_db_opr_obj(msg).ret;
		
    }
	
	*bucket_id = pui->bucket_id;
	DMCLOG_D("bucket id : %d",pui->bucket_id);
	free_sync_message(&msg);
    EXIT_FUNC();
    // 3. Response client
	return ACTION_SUCCESS;
}


/*
*	insert item to authority table when register
*/
ERROR_CODE_VFS handle_authority_table_insert(_In_ int bucket_id,_In_ int user_id,_In_ int autority,_Out_ int *autority_id)
{
	ENTER_FUNC();
    // 2. query db.
    Message * msg = create_sync_message();
	assert(msg != NULL);
	msg->msg_data.db_obj.data.insert_data.authority_insert.cmd = AUTHORITY_TABLE_INSERT_INFO;
	authority_info_t *pui = &msg->msg_data.db_obj.data.insert_data.authority_insert.authority_info;
    msg->msg_header.m_type = MSG_DB_INSERT_AUTHORITY; 
	pui->bucket_id = bucket_id;
	pui->user_id = user_id;
	pui->authority = autority;
   	get_db_opr_obj(msg).ret = g_db_write_task.msg_cb(&g_db_write_task, msg);

    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("insert disk info failed");
        free_sync_message(&msg);
        return get_db_opr_obj(msg).ret;
		
    }
	
	*autority_id = pui->authority_id;
	DMCLOG_D("pui->authority_id: %d",pui->authority_id);
	free_sync_message(&msg);
    EXIT_FUNC();
    // 3. Response client
	return ACTION_SUCCESS;
}
/************************************************register**************************************************************/

/************************************************login**************************************************************/
/*
*	query item from user table when login
*/
ERROR_CODE_VFS handle_user_table_query(_In_ char *username,_Out_ char *password,_Out_ int *user_id)
{
	ENTER_FUNC();
    // 2. query db.
    if(username == NULL||password == NULL)
   	{
		return FAIL;
	}
    Message * msg = create_sync_message();
	assert(msg != NULL);
	
	user_info_t *pui = &msg->msg_data.db_obj.data.query_data.user_query.user_info;
	if(*user_id == 0)
		msg->msg_data.db_obj.data.query_data.user_query.cmd = USER_TABLE_QUERY_INFO;
	else{
		pui->user_id = *user_id;
		msg->msg_data.db_obj.data.query_data.user_query.cmd = USER_TABLE_QUERY_INFO_BY_ID;
	}
    msg->msg_header.m_type = MSG_DB_QUERY_USER; 
	if(*username)
    	S_STRNCPY(pui->user_name,username,MAX_USER_NAME_LEN);
	
   	get_db_opr_obj(msg).ret = g_db_query_task.msg_cb(&g_db_query_task, msg);

    // check result if ok
    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("query user info failed");
        free_sync_message(&msg);
        return get_db_opr_obj(msg).ret;
		
    }
	if(!*username)
	{
		S_STRNCPY(username,pui->user_name,MAX_USER_NAME_LEN);
		DMCLOG_D("username = %s",username);
	}
	*user_id = pui->user_id;
	S_STRNCPY(password,pui->password,MAX_PASSWORD_LEN);
	
	free_sync_message(&msg);
    EXIT_FUNC();
    // 3. Response client
	return ACTION_SUCCESS;
}

/*
*	query item from bucket table when login
*/
ERROR_CODE_VFS handle_bucket_table_query(_Out_ char *bucket_name,_In_ int creat_user_id,_Out_ int *bucket_id)
{
	ENTER_FUNC();
    // 2. query db.
    if(bucket_name == NULL)
   	{
		return FAIL;
	}
    Message * msg = create_sync_message();
	assert(msg != NULL);
	msg->msg_data.db_obj.data.query_data.bucket_query.cmd = BUCKET_TABLE_QUERY_INFO;
	bucket_info_t *pui = &msg->msg_data.db_obj.data.query_data.bucket_query.bucket_info;
    msg->msg_header.m_type = MSG_DB_QUERY_BUCKET; 
	pui->create_user_id = creat_user_id;
   	get_db_opr_obj(msg).ret = g_db_query_task.msg_cb(&g_db_query_task, msg);

    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("query bucket info failed");
        free_sync_message(&msg);
        return get_db_opr_obj(msg).ret;
		
    }
	S_STRNCPY(bucket_name,pui->bucket_table_name,MAX_BUCKET_NAME_LEN);
	*bucket_id = pui->bucket_id;
	DMCLOG_D("bucket_name = %s",bucket_name);
	free_sync_message(&msg);
    EXIT_FUNC();
    // 3. Response client
	return ACTION_SUCCESS;
}

/*
*	query item from authority table when login
*/
ERROR_CODE_VFS handle_authority_table_query(_In_ int bucket_id,_In_ int user_id,_Out_ int *autority,_Out_ int *autority_id)
{
	ENTER_FUNC();
    // 2. query db.
    Message * msg = create_sync_message();
	assert(msg != NULL);
	msg->msg_data.db_obj.data.query_data.authority_query.cmd = AUTHORITY_TABLE_QUERY_INFO;
	authority_info_t *pui = &msg->msg_data.db_obj.data.query_data.authority_query.authority_info;
    msg->msg_header.m_type = MSG_DB_QUERY_AUTHORITY; 
	pui->bucket_id = bucket_id;
	pui->user_id = user_id;
	
   	get_db_opr_obj(msg).ret = g_db_query_task.msg_cb(&g_db_query_task, msg);

    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("insert disk info failed");
        free_sync_message(&msg);
        return get_db_opr_obj(msg).ret;
		
    }
	*autority = pui->authority;
	*autority_id = pui->authority_id;
	free_sync_message(&msg);
    EXIT_FUNC();
    // 3. Response client
	return ACTION_SUCCESS;
}

/************************************************login end**************************************************************/

/*
*	query list from bucket table when login
*/
char **handle_bucket_table_list_query()
{
	ENTER_FUNC();
    Message * msg = create_sync_message();
	assert(msg != NULL);
	msg->msg_data.db_obj.data.query_data.bucket_query.cmd = BUCKET_TABLE_QUERY_LIST;
	bucket_list_t *plist = &msg->msg_data.db_obj.data.query_data.bucket_query.bucket_list;

	dl_list_init(&plist->head);
    msg->msg_header.m_type = MSG_DB_QUERY_BUCKET; 
   	get_db_opr_obj(msg).ret = g_db_query_task.msg_cb(&g_db_query_task, msg);

    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("query bucket info failed");
        free_sync_message(&msg);
        return NULL;
		
    }
	if(plist->len == 0)
	{
		free_sync_message(&msg);
    	EXIT_FUNC();
		return NULL;
	}
	char **bucket_list = (char **)calloc(plist->len + 1,sizeof(char *));
	int i = 0;
	bucket_info_t *p_bucket_info = NULL;
	bucket_info_t *n = NULL;
	dl_list_for_each(p_bucket_info, &(plist->head), bucket_info_t, next)
	{
		bucket_list[i] = (char *)calloc(1,strlen(p_bucket_info->bucket_table_name) + 1);
		strcpy(bucket_list[i],p_bucket_info->bucket_table_name);
		DMCLOG_D("p_bucket_info->bucket_table_name = %s",p_bucket_info->bucket_table_name);
		i++;
	}

	dl_list_for_each_safe(p_bucket_info,n,&plist->head,bucket_info_t,next)
	{
		dl_list_del(&p_bucket_info->next);
		safe_free(p_bucket_info);
	}
	free_sync_message(&msg);
    EXIT_FUNC();
	return bucket_list;
}


/************************************************upload start**************************************************************/
/*
*	query item from v file table when upload
*/
ERROR_CODE_VFS handle_v_file_table_query(_In_ char *bucket_name,v_file_info_t *file_info)
{
	ENTER_FUNC();
    // 2. query db.
    if(!*bucket_name||file_info == NULL)
   	{
   		DMCLOG_E("para is null");
		return FAIL;
	}
    Message * msg = create_sync_message();
	assert(msg != NULL);
	msg->msg_data.db_obj.data.query_data.v_file_query.cmd = V_FILE_TABLE_QUERY_INFO;
	v_file_info_t *pui = &msg->msg_data.db_obj.data.query_data.v_file_query.v_file_info;

	v_file_query_t *pue = &msg->msg_data.db_obj.data.query_data.v_file_query;

	S_STRNCPY(pue->bucket_name,bucket_name,MAX_BUCKET_NAME_LEN);
	
    msg->msg_header.m_type = MSG_DB_QUERY_VFILE; 
    S_STRNCPY(pui->path,file_info->path,MAX_FILE_PATH_LEN);
	

   	get_db_opr_obj(msg).ret = g_db_query_task.msg_cb(&g_db_query_task, msg);

    // check result if ok
    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("query v file info failed");
        free_sync_message(&msg);
        return get_db_opr_obj(msg).ret;
		
    }
	file_info->id = pui->id;
	file_info->type = pui->type;
	strcpy(file_info->real_path,pui->real_path);
	free_sync_message(&msg);
    EXIT_FUNC();
    // 3. Response client
	return ACTION_SUCCESS;
}

/*
*	query item from v file table
*/
ERROR_CODE_VFS _handle_v_file_table_query(v_file_query_t *v_file_query)
{
	ENTER_FUNC();
    // 2. query db.
    if(v_file_query == NULL)
   	{
   		DMCLOG_E("para is null");
		return FAIL;
	}
    Message * msg = create_sync_message();
	assert(msg != NULL);
    msg->msg_header.m_type = MSG_DB_QUERY_VFILE; 
	v_file_query_t *pue = &msg->msg_data.db_obj.data.query_data.v_file_query;
	memcpy(pue,v_file_query,sizeof(v_file_query_t));
   	get_db_opr_obj(msg).ret = g_db_query_task.msg_cb(&g_db_query_task, msg);
	
    // check result if ok
    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("query v file info failed");
        free_sync_message(&msg);
        return get_db_opr_obj(msg).ret;
		
    }
	v_file_query->v_file_list.total = pue->v_file_list.total;
	memcpy(v_file_query,pue,sizeof(v_file_query_t));
	free_sync_message(&msg);
    EXIT_FUNC();
    // 3. Response client
	return ACTION_SUCCESS;
}


/*
*	query item is or not exist in the file table by uuid
*	return 0:the uuid is exist;
*		   !0:the uuid is not exist
*/
ERROR_CODE_VFS handle_file_uuid_exist_query(_In_ char *bucket_name,_In_ char *uuid)
{
	ENTER_FUNC();
    // 2. query db.
    if(!*bucket_name||!*uuid)
   	{
   		DMCLOG_E("para is null");
		return FAIL;
	}
    Message * msg = create_sync_message();
	assert(msg != NULL);
	msg->msg_data.db_obj.data.query_data.v_file_query.cmd = V_FILE_TABLE_QUERY_INFO_BY_UUID;
	v_file_info_t *pui = &msg->msg_data.db_obj.data.query_data.v_file_query.v_file_info;

	v_file_query_t *pue = &msg->msg_data.db_obj.data.query_data.v_file_query;

	S_STRNCPY(pue->bucket_name,bucket_name,MAX_BUCKET_NAME_LEN);
	
    msg->msg_header.m_type = MSG_DB_QUERY_VFILE; 
    S_STRNCPY(pui->uuid,uuid,MAX_FILE_UUID_LEN);
	

   	get_db_opr_obj(msg).ret = g_db_query_task.msg_cb(&g_db_query_task, msg);

    // check result if ok
    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("query v file info by uuid failed");
        free_sync_message(&msg);
        return get_db_opr_obj(msg).ret;
		
    }
	free_sync_message(&msg);
    EXIT_FUNC();
    // 3. Response client
	return ACTION_SUCCESS;
}

/*
*	query item list are or not exist in the file table by uuid
*	return 0:success
*		   !0:error
*/
ERROR_CODE_VFS handle_uuid_list_exist_query(_In_ char *bucket_name,struct dl_list *head )
{
	ENTER_FUNC();
    // 2. query db.
    if(!*bucket_name||head == NULL)
   	{
   		DMCLOG_E("para is null");
		return FAIL;
	}
    Message * msg = create_sync_message();
	assert(msg != NULL);
	msg->msg_data.db_obj.data.query_data.v_file_query.cmd = V_FILE_TABLE_QUERY_LIST_BY_UUID;
	v_file_list_t *pui = &msg->msg_data.db_obj.data.query_data.v_file_query.v_file_list;
	memcpy(&pui->head,head,sizeof(struct dl_list));
	v_file_query_t *pue = &msg->msg_data.db_obj.data.query_data.v_file_query;

	S_STRNCPY(pue->bucket_name,bucket_name,MAX_BUCKET_NAME_LEN);
	
    msg->msg_header.m_type = MSG_DB_QUERY_VFILE; 
	

   	get_db_opr_obj(msg).ret = g_db_query_task.msg_cb(&g_db_query_task, msg);

    // check result if ok
    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("query v file info by uuid failed");
        free_sync_message(&msg);
        return get_db_opr_obj(msg).ret;
		
    }
	free_sync_message(&msg);
    EXIT_FUNC();
    // 3. Response client
	return ACTION_SUCCESS;
}



/*
*	insert item to v file table
*/
ERROR_CODE_VFS handle_v_file_table_insert(_In_ char *bucket_name,v_file_info_t *v_file_info)
{
	ENTER_FUNC();
    // 2. query db.
    if(!*bucket_name)
   	{
		return FAIL;
	}
    Message * msg = create_sync_message();
	assert(msg != NULL);
	msg->msg_data.db_obj.data.insert_data.v_file_insert.cmd = V_FILE_TABLE_INSERT_INFO;

	v_file_insert_t *pue = &msg->msg_data.db_obj.data.insert_data.v_file_insert;

	S_STRNCPY(pue->bucket_name,bucket_name,MAX_BUCKET_NAME_LEN);

	DMCLOG_D("bucket_name = %s",pue->bucket_name);
	v_file_info_t *pui = &msg->msg_data.db_obj.data.insert_data.v_file_insert.v_file_info;
    msg->msg_header.m_type = MSG_DB_INSERT_VFILE; 
	memcpy(pui,v_file_info,sizeof(v_file_info_t));
   	get_db_opr_obj(msg).ret = g_db_write_task.msg_cb(&g_db_write_task, msg);

    // check result if ok
    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("insert file table failed");
        free_sync_message(&msg);
        return get_db_opr_obj(msg).ret;
		
    }
	free_sync_message(&msg);
    EXIT_FUNC();
    // 3. Response client
	return ACTION_SUCCESS;
}

/*
*	insert item to v file table
*/
ERROR_CODE_VFS _handle_v_file_table_insert(v_file_insert_t *v_file_insert)
{
	ENTER_FUNC();
    // 2. query db.
    if(v_file_insert == NULL)
   	{
		return FAIL;
	}
    Message * msg = create_sync_message();
	assert(msg != NULL);
	v_file_insert_t *pue = &msg->msg_data.db_obj.data.insert_data.v_file_insert;
    msg->msg_header.m_type = MSG_DB_INSERT_VFILE; 
	memcpy(pue,v_file_insert,sizeof(v_file_insert_t));
   	get_db_opr_obj(msg).ret = g_db_write_task.msg_cb(&g_db_write_task, msg);

    // check result if ok
    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("insert file table failed");
        free_sync_message(&msg);
        return get_db_opr_obj(msg).ret;
		
    }
	free_sync_message(&msg);
    EXIT_FUNC();
    // 3. Response client
	return ACTION_SUCCESS;
}


/************************************************upload end**************************************************************/

/*
*	update item to v file table when upload
*/
ERROR_CODE_VFS handle_v_file_table_update(_In_ char *bucket_name,int cmd,real_remove remove,v_file_info_t *v_file_info)
{
	ENTER_FUNC();
    // 2. query db.
    if(!*bucket_name)
   	{
   		DMCLOG_E("para is null");
		return FAIL;
	}
    Message *msg = create_sync_message();
	assert(msg != NULL);
	msg->msg_data.db_obj.data.update_data.v_file_update.cmd = cmd;
	msg->msg_data.db_obj.data.update_data.v_file_update.remove = remove;
	v_file_info_t *pui = &msg->msg_data.db_obj.data.update_data.v_file_update.v_file_info;
    msg->msg_header.m_type = MSG_DB_UPDATE_VFILE; 
	S_STRNCPY(msg->msg_data.db_obj.data.update_data.v_file_update.bucket_name,bucket_name,MAX_BUCKET_NAME_LEN);

	memcpy(pui,v_file_info,sizeof(v_file_info_t));
   	get_db_opr_obj(msg).ret = g_db_write_task.msg_cb(&g_db_write_task, msg);

    // check result if ok
    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("update file info failed");
        free_sync_message(&msg);
        return get_db_opr_obj(msg).ret;
		
    }
	free_sync_message(&msg);
    EXIT_FUNC();
    // 3. Response client
	return ACTION_SUCCESS;
}

/*
*	update item to another bucket
*/
ERROR_CODE_VFS _handle_v_file_table_update(v_file_update_t *v_file_update)
{
	ENTER_FUNC();
    // 2. query db.
    Message *msg = create_sync_message();
	assert(msg != NULL);
	v_file_update_t *pui = &msg->msg_data.db_obj.data.update_data.v_file_update;
    msg->msg_header.m_type = MSG_DB_UPDATE_VFILE; 
	memcpy(pui,v_file_update,sizeof(v_file_update_t));
   	get_db_opr_obj(msg).ret = g_db_write_task.msg_cb(&g_db_write_task, msg);

    // check result if ok
    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("update file info failed");
        free_sync_message(&msg);
        return get_db_opr_obj(msg).ret;
		
    }
	free_sync_message(&msg);
    EXIT_FUNC();
    // 3. Response client
	return ACTION_SUCCESS;
}


/*
*	delete item to v file table 
*/
ERROR_CODE_VFS handle_v_file_table_delete(_In_ char *bucket_name,int cmd,real_remove remove,v_file_info_t *v_file_info)
{
	ENTER_FUNC();
    // 2. query db.
    if(!*bucket_name||v_file_info == NULL)
   	{
   		DMCLOG_E("para is null");
		return FAIL;
	}
    Message *msg = create_sync_message();
	assert(msg != NULL);
	msg->msg_data.db_obj.data.delete_data.v_file_delete.cmd = cmd;
	msg->msg_data.db_obj.data.delete_data.v_file_delete.remove = remove;
	v_file_info_t *pui = &msg->msg_data.db_obj.data.delete_data.v_file_delete.v_file_info;
    msg->msg_header.m_type = MSG_DB_DELETE_VFILE; 
	S_STRNCPY(msg->msg_data.db_obj.data.delete_data.v_file_delete.bucket_name,bucket_name,MAX_BUCKET_NAME_LEN);
	memcpy(pui,v_file_info,sizeof(v_file_info_t));
   	get_db_opr_obj(msg).ret = g_db_write_task.msg_cb(&g_db_write_task, msg);

    // check result if ok
    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("query user info failed");
        free_sync_message(&msg);
        return get_db_opr_obj(msg).ret;
		
    }
	free_sync_message(&msg);
    EXIT_FUNC();
    // 3. Response client
	return ACTION_SUCCESS;
}

/*
*	delete item to v file table 
*/
ERROR_CODE_VFS _handle_v_file_table_delete(v_file_delete_t *v_file_delete)
{
	ENTER_FUNC();
    // 2. query db.
    if(v_file_delete == NULL)
   	{
   		DMCLOG_E("para is null");
		return FAIL;
	}
    Message *msg = create_sync_message();
	assert(msg != NULL);
	v_file_delete_t *pui = &msg->msg_data.db_obj.data.delete_data.v_file_delete;
    msg->msg_header.m_type = MSG_DB_DELETE_VFILE; 
	memcpy(pui,v_file_delete,sizeof(v_file_delete_t));
   	get_db_opr_obj(msg).ret = g_db_write_task.msg_cb(&g_db_write_task, msg);

    // check result if ok
    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("query user info failed");
        free_sync_message(&msg);
        return get_db_opr_obj(msg).ret;
		
    }
	free_sync_message(&msg);
    EXIT_FUNC();
    // 3. Response client
	return ACTION_SUCCESS;
}



/*
*	query list from v file table
*	return total result
*/
int handle_v_file_table_list_query(_In_ char *bucket_name,v_file_list_t *vlist)
{
	ENTER_FUNC();
	if(!*bucket_name)
	{
		DMCLOG_E("para is null");
		return FAIL;
	}
    Message * msg = create_sync_message();
	assert(msg != NULL);
	msg->msg_data.db_obj.data.query_data.v_file_query.cmd = vlist->cmd;
	v_file_list_t *plist = &msg->msg_data.db_obj.data.query_data.v_file_query.v_file_list;
    msg->msg_header.m_type = MSG_DB_QUERY_VFILE; 
	v_file_query_t *pue = &msg->msg_data.db_obj.data.query_data.v_file_query;
	S_STRNCPY(pue->bucket_name,bucket_name,MAX_BUCKET_NAME_LEN);
	memcpy(plist,vlist,sizeof(v_file_list_t)); 
	strcpy(plist->bucket_name,bucket_name);
   	get_db_opr_obj(msg).ret = g_db_query_task.msg_cb(&g_db_query_task, msg);

    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("query v file info failed");
        free_sync_message(&msg);
        return get_db_opr_obj(msg).ret ;
		
    }
	vlist->total = plist->total;
	DMCLOG_D("result total:%d",vlist->total);
	free_sync_message(&msg);
    EXIT_FUNC();
	return ACTION_SUCCESS;
}

/**
 *	insert item to disk table
 */
ERROR_CODE_VFS handle_disk_table_insert(unsigned long long capacity,unsigned long long free_capacity,char *uuid)
{
	ENTER_FUNC();
    Message * msg = create_sync_message();
	assert(msg != NULL);
	msg->msg_data.db_obj.data.insert_data.disk_insert.cmd = DISK_TABLE_INSERT_INFO;
	disk_info_t *pui = &msg->msg_data.db_obj.data.insert_data.disk_insert.disk_info;
    msg->msg_header.m_type = MSG_DB_INSERT_DISK; 
   	get_db_opr_obj(msg).ret = g_db_write_task.msg_cb(&g_db_write_task, msg);
	S_STRNCPY(pui->uuid,uuid,MAX_UUID_LEN);
	pui->free_size = free_capacity;
	pui->total_size = capacity;
    // check result if ok
    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("insert disk info failed");
        free_sync_message(&msg);
        return get_db_opr_obj(msg).ret;
		
    }
	DMCLOG_D("pui->disk: %d",pui->disk_id);
	free_sync_message(&msg);
    
    // 3. Response client
	return ACTION_SUCCESS;
}

/**
 *	query item from disk table
 */
ERROR_CODE_VFS handle_disk_table_query(unsigned long long *capacity,unsigned long long *free_capacity,char *uuid)
{
	ENTER_FUNC();
    Message * msg = create_sync_message();
	assert(msg != NULL);
	msg->msg_data.db_obj.data.insert_data.disk_insert.cmd = DISK_TABLE_QUERY_INFO;
	disk_info_t *pui = &msg->msg_data.db_obj.data.query_data.disk_query.disk_info;
    msg->msg_header.m_type = MSG_DB_QUERY_DISK; 
   	get_db_opr_obj(msg).ret = g_db_query_task.msg_cb(&g_db_query_task, msg);
	S_STRNCPY(pui->uuid,uuid,MAX_UUID_LEN);
	
    // check result if ok
    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("query disk info failed");
        free_sync_message(&msg);
        return get_db_opr_obj(msg).ret;
		
    }
	*free_capacity = pui->free_size;
	*capacity = pui->total_size;
	DMCLOG_D("pui->disk: %d",pui->disk_id);
	free_sync_message(&msg);
    
    // 3. Response client
	return ACTION_SUCCESS;
}

ERROR_CODE_VFS handle_media_update(int cmd,v_file_info_t *v_file_info)
{
	ENTER_FUNC();
    // 2. query db.
    Message *msg = create_sync_message();
	assert(msg != NULL);
	msg->msg_data.db_obj.data.update_data.v_file_update.cmd = cmd;
	v_file_info_t *pui = &msg->msg_data.db_obj.data.update_data.v_file_update.v_file_info;
    msg->msg_header.m_type = MSG_DB_UPDATE_VFILE; 

	memcpy(pui,v_file_info,sizeof(v_file_info_t));
   	get_db_opr_obj(msg).ret = g_db_write_task.msg_cb(&g_db_write_task, msg);

    // check result if ok
    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("update file info failed");
        free_sync_message(&msg);
        return get_db_opr_obj(msg).ret;
		
    }
	free_sync_message(&msg);
    EXIT_FUNC();
    // 3. Response client
	return ACTION_SUCCESS;
}


ERROR_CODE_VFS handle_media_query(int cmd,v_file_info_t *v_file_info)
{
	ENTER_FUNC();
    // 2. query db.
    Message *msg = create_sync_message();
	assert(msg != NULL);
	msg->msg_data.db_obj.data.query_data.v_file_query.cmd = cmd;
	v_file_info_t *pui = &msg->msg_data.db_obj.data.query_data.v_file_query.v_file_info;
    msg->msg_header.m_type = MSG_DB_QUERY_VFILE; 
   	get_db_opr_obj(msg).ret = g_db_query_task.msg_cb(&g_db_query_task, msg);

    // check result if ok
    if(get_db_opr_obj(msg).ret != ACTION_SUCCESS)
    {
        DMCLOG_E("query file info failed");
        free_sync_message(&msg);
        return get_db_opr_obj(msg).ret;
		
    }
	free_sync_message(&msg);
    EXIT_FUNC();
    // 3. Response client
	return ACTION_SUCCESS;
}


