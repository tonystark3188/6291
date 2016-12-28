/*
 * =============================================================================
 *
 *       Filename:  db_task.c
 *
 *    Description:  create database task
 *
 *        Version:  1.0
 *        Created:  2014/8/2 13:59:25
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ye Wenhao (), wenhaoye@126.com
 *   Organization:  
 *
 * =============================================================================
 */

#include "mo.h"
#include "db_task.h"

typedef enum 
{
	DB_NORMAL_RUNNING,
	DB_RESTARTING,
	DB_STOP
}db_state_t;

static int g_db_errcode = RET_SUCCESS;
static db_state_t g_db_state = DB_STOP;


int get_db_errcode(void)
{
	return g_db_errcode;
}

void recover_db(void)
{
	g_db_state = DB_NORMAL_RUNNING;
}


int db_query_task_msg_cb(void *self, void *msg)
{
	TaskObj *task = (TaskObj *)self;
	Message *p_msg = (Message *)msg;
	MsgObj *p_obj = &p_msg->msg_header;
	sqlite3 *database = *(sqlite3 **)task->arg;
	DB_OprObj *p_OprObj = &p_msg->msg_data.db_obj;
	int ret = 0;

	if(task == NULL || p_msg == NULL)
	{
		DMCLOG_E("database task: invalid parameters\n");
		return ENULL_POINT;
	}
	if(p_obj->m_type == MSG_DB_QUERY_USER)
	{
		 ret = g_user_table.ops.query(database,&p_OprObj->data.query_data.user_query);
	}else if(p_obj->m_type == MSG_DB_QUERY_AUTHORITY)
	{
		 ret = g_authority_table.ops.query(database,&p_OprObj->data.query_data.authority_query);
	}else if(p_obj->m_type == MSG_DB_QUERY_BUCKET)
	{
		 ret = g_bucket_table.ops.query(database,&p_OprObj->data.query_data.bucket_query);
	}else if(p_obj->m_type == MSG_DB_QUERY_VFILE)
	{
		 ret = g_v_file_table.ops.query(database,&p_OprObj->data.query_data.v_file_query);
	}else if(p_obj->m_type == MSG_DB_QUERY_DISK)
	{
		disk_query_t *disk_query = &p_OprObj->data.query_data.disk_query;
		ret = g_disk_table.ops.delete(database,disk_query);
	}
		
	if(p_obj->ret_flag == MSG_NO_RETURN)
	{
		return ret;
	}
	else if(p_obj->ret_flag == MSG_NORMAL_RETURN)
	{
		p_msg->msg_header.m_type += 1;
		send_message(p_msg);
	}  
	else if(p_obj->ret_flag == MSG_SYN_RETURN)
	{
		int r;
		r = sem_post(&p_obj->m_syn->sm_sem);
		if(r!=0)
		{
			DMCLOG_E("sem_post error in database handler");
			return ESEM_POST;
		}
	}
	else
	{
		DMCLOG_E("invalid return flag\n");
	}
	
	return ret;
}


int db_write_task_msg_cb(void *self, void *msg)
{
    TaskObj *task = (TaskObj *)self;
	Message *p_msg = (Message *)msg;
	MsgObj	*p_obj = &p_msg->msg_header;
	sqlite3 *database = *(sqlite3 **)task->arg;
	DB_OprObj *p_OprObj = &p_msg->msg_data.db_obj;
	//&msg->msg_data.db_obj.data.insert_data.user_insert.user_info;
	int ret = 0;
		
	if(task == NULL || p_msg == NULL)
	{
		DMCLOG_E("database task: invalid parameters\n");
		return ENULL_POINT;
	}
    

	if((ret = start_transaction(database)) != RET_SUCCESS)
	{
		DMCLOG_E("db start transaction error!");
		goto EXIT;
	}
	if(p_obj->m_type == MSG_DB_INSERT_USER)
	{
		user_insert_t *user_insert = &p_OprObj->data.insert_data.user_insert;
		ret = g_user_table.ops.insert(database,user_insert);
	}else if(p_obj->m_type == MSG_DB_UPDATE_USER)
	{
		 ret = g_user_table.ops.update(database,&p_OprObj->data.update_data.user_update);
	}else if(p_obj->m_type == MSG_DB_DELETE_USER)
	{
		 ret = g_user_table.ops.delete(database,&p_OprObj->data.delete_data.user_delete);
	}else if(p_obj->m_type == MSG_DB_INSERT_AUTHORITY)
	{
		 ret = g_authority_table.ops.insert(database,&p_OprObj->data.insert_data.authority_insert);
	}else if(p_obj->m_type == MSG_DB_UPDATE_AUTHORITY)
	{
		 ret = g_authority_table.ops.update(database,&p_OprObj->data.update_data.authority_update);
	}else if(p_obj->m_type == MSG_DB_DELETE_AUTHORITY)
	{
		 ret = g_authority_table.ops.delete(database,&p_OprObj->data.delete_data.authority_delete);
	}else if(p_obj->m_type == MSG_DB_INSERT_BUCKET)
	{
		 ret = g_bucket_table.ops.insert(database,&p_OprObj->data.insert_data.bucket_insert);
	}else if(p_obj->m_type == MSG_DB_UPDATE_BUCKET)
	{
		 ret = g_bucket_table.ops.update(database,&p_OprObj->data.update_data.bucket_update);
	}else if(p_obj->m_type == MSG_DB_DELETE_BUCKET)
	{
		 ret = g_bucket_table.ops.delete(database,&p_OprObj->data.delete_data.bucket_delete);
	}else if(p_obj->m_type == MSG_DB_INSERT_VFILE)
	{
		v_file_insert_t *v_file_insert = &p_OprObj->data.insert_data.v_file_insert;
		ret = g_v_file_table.ops.insert(database,v_file_insert);
	}else if(p_obj->m_type == MSG_DB_UPDATE_VFILE)
	{
		v_file_update_t *v_file_update = &p_OprObj->data.update_data.v_file_update;
		ret = g_v_file_table.ops.update(database,v_file_update);
	}else if(p_obj->m_type == MSG_DB_DELETE_VFILE)
	{
		v_file_delete_t *v_file_delete= &p_OprObj->data.delete_data.v_file_delete;
		ret = g_v_file_table.ops.delete(database,v_file_delete);
	}else if(p_obj->m_type == MSG_DB_INSERT_DISK)
	{
		disk_insert_t *disk_insert = &p_OprObj->data.insert_data.disk_insert;
		ret = g_disk_table.ops.insert(database,disk_insert);
	}else if(p_obj->m_type == MSG_DB_UPDATE_DISK)
	{
		disk_update_t *disk_update = &p_OprObj->data.update_data.disk_update;
		ret = g_disk_table.ops.update(database,disk_update);
	}else if(p_obj->m_type == MSG_DB_DELETE_DISK)
	{
		disk_delete_t *disk_delete= &p_OprObj->data.delete_data.disk_delete;
		ret = g_disk_table.ops.delete(database,disk_delete);
	}
	

	if(ret == RET_SUCCESS)
	{
		if((ret = commit_transaction(database)) != RET_SUCCESS)
		{
			DMCLOG_E("db commit transaction error!");
			goto EXIT;
		}
	}
	else
	{
		if(rollback_transaction(database) != RET_SUCCESS)
		{
			DMCLOG_E("db rollback error");
		}
	}

	
EXIT:	
	return ret;
}

void db_task_cleanup(void *self)
{
    TaskObj *task = (TaskObj *)self;
    sqlite3 *con = (*((sqlite3 **)task->arg));
    
    if(con != NULL)
    {
        sqlite3_close(con);
        *((sqlite3 **)task->arg) = NULL;
    }
}


static sqlite3 *write_connection = NULL;
static sqlite3 *query_connection = NULL;

int close_db_all_connection(void)
{
    int ret;
    
    if((ret = close_db_connection(write_connection)) != SQLITE_OK)
	{
		DMCLOG_E("cannot close db write handle,ret=0x%x",ret);
		return ret;	
	}
    write_connection = NULL;
    
	if((ret = close_db_connection(query_connection)) != SQLITE_OK)
	{
		DMCLOG_E("cannot close db read handle,ret=0x%x",ret);
		return ret;	
	}
    query_connection = NULL;

    return RET_SUCCESS;
}

int create_db_all_connection(void)
{
    int ret;
    
	if((ret = create_db_connection(&write_connection, 1)) != RET_SUCCESS)
	{
		return ret;
	}

	if((ret = create_db_connection(&query_connection, 0)) != RET_SUCCESS)
	{
		return ret;
	}

    return RET_SUCCESS;
}

int restart_db(void)
{
	int ret;
	
	if((ret = close_db_all_connection()) != RET_SUCCESS)
	    return ret;

    if((ret = create_db_all_connection()) != RET_SUCCESS)
        return ret;

	recover_db();
	return RET_SUCCESS;
}

int create_db_task(void)
{
    int ret = -1;
    
	MEMSET(g_db_query_task);
	MEMSET(g_db_write_task);

	if(create_db_connection(&query_connection, 0) != RET_SUCCESS)
	{
		return -1;
	}

    g_db_query_task.msg_cb = db_query_task_msg_cb;
    g_db_query_task.arg = &query_connection;
	g_db_query_task.exit_cb = db_task_cleanup;

	if(create_db_connection(&write_connection, 1) != RET_SUCCESS)
	{
        write_connection = NULL;
		return -1;
	}
	g_db_write_task.msg_cb = db_write_task_msg_cb;
    g_db_write_task.arg = &write_connection;
	g_db_write_task.exit_cb = db_task_cleanup;


    ret = create_task(&g_db_query_task, "db_query_task", MSG_TYPE_DB_QUERY);
    if(ret < 0)
    {
        printf("create_db_task failed!");
        return ret;
    }
	ret = create_task(&g_db_write_task, "db_write_task", MSG_TYPE_DB_WRITE);
	if(ret < 0)
	{
		printf("create_db_task failed!");
		return ret;
	}
	
	g_db_state = DB_NORMAL_RUNNING;
	
    return 0;
}

