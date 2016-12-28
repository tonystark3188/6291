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

#include "task_base.h"
//#include "signal_task.h"
#include "mo/mo.h"
#include "db/nas_db.h"
//#include "db/db_base.h"
#include "hidisk_errno.h"
#include "config.h"
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


static error_t insert_handler(MSG_TYPE type, sqlite3 *database, DB_OprObj *obj)
{	
	error_t errcode = RET_SUCCESS;
	
    if(type == MSG_DB_FILE_SINGLE_ADD)
    {
        if((errcode = db_insert_file(database, &obj->data.insert_data))
				!= RET_SUCCESS)
        {
            log_warning("failed to insert item %s into database\n",obj->data.insert_data.file_info.path);
			return errcode;
		}
	}
	else
	{
      /*  if(!db_insert_batch_files(database, &obj->head))
        {
            log_warning("failed to insert batch items into database\n");
			return 0;
		}*/
	}
	
	return errcode;
}


int db_query_task_msg_cb(void *self, void *msg)
{
	TaskObj *task = (TaskObj *)self;
	Message *p_msg = (Message *)msg;
	MsgObj *p_obj = &p_msg->msg_header;
	sqlite3 *database = *(sqlite3 **)task->arg;
	DB_OprObj *p_OprObj = &p_msg->msg_data.db_obj;
	int ret = AIRNAS_ERRNO;

	if(task == NULL || p_msg == NULL)
	{
		log_warning("database task: invalid parameters\n");
		return ENULL_POINT;
	}
#if 0
	if(g_db_state != DB_NORMAL_RUNNING)
	{
		log_warning("db cannot work now,state=%d", g_db_state);
		
		if(g_db_state == DB_RESTARTING)
		{
			ret = EDB_RESTARTING;
		}
		else if(g_db_state == DB_STOP)
		{
			ret = EDB_STOP_WORKING;
		}

        if(p_obj->m_type != MSG_DB_QUERY_EXIT)
        {
		    goto EXIT;
        }
        else
        {
            log_notice("db_query recv exit msg");
        }
	}
#endif

	switch(p_obj->m_type)
	{
		case MSG_DB_QUERY_USER_INFO_BY_ID:
			ret = db_query(database, QUERY_USER_INFO_BY_SESSION, &p_OprObj->data.user_data);
			break;
		case MSG_DB_QUERY_DEVICE_BY_USER:
			ret = db_query(database, QUERY_DEVICE_BY_UUID, &p_OprObj->data.device_data);
			break;
		case MSG_DB_QUERY_PATH:
			ret = db_query(database, QUERY_FILE_PATH, &p_OprObj->data.qry_path_data);
			break;
		case MSG_DB_QUERY_BATCH_PATH:
			ret = db_query(database, QUERY_BATCH_FILE_PATH, &p_OprObj->data.abs_path_list);
			break;
		case MSG_DB_QUERY_FILE_INFO:
			ret = db_query(database, QUERY_FILE_INFO, &p_OprObj->data.file_data);
            break;
		case MSG_DB_QUERY_FILE_BY_NAME:
			ret = db_query(database, QUERY_FILE_BY_NAME, &p_OprObj->data.file_data);
            break;
		case MSG_DB_QUERY_FILE_LIST_CNT:
			ret = db_query(database, QUERY_FILE_LIST_COUNT, &p_OprObj->data.file_list);
            break;
		case MSG_DB_QUERY_FILE_LIST:
			ret = db_query(database, QUERY_FILE_LIST, &p_OprObj->data.file_list);
			break;
		case MSG_DB_QUERY_HDISK_INFO:
            ret = db_query(database, QUERY_HDISK_INFO, &p_OprObj->data.hdisk_data);
            break;
		case MSG_DB_QUERY_BACKUP:
            ret = db_query(database, QUERY_BACKUP_FILE, &p_OprObj->data.backup_file_data);
            break;
		case MSG_DB_QUERY_BACKUP_LIST:
            ret = db_query(database, QUERY_BACKUP_FILE_LIST, &p_OprObj->data.backup_list);
            break;
		case MSG_DB_QUERY_BACKUP_DEVICE_SIZE:
            ret = db_query(database, QUERY_BACKUP_DEVICE_INFO, &p_OprObj->data.backup_list);
            break;
	    case MSG_DB_QUERY_EXIT:
		    task->exit_flag = 1;
            // Fix bug: after format hdisk, re-register mq failed. 
            unregister_mq(task->msg_type);
	        release_mq(&task->task_mq);
			ret = RET_SUCCESS;
	        break;
		default:break;
	}

	if(SQLITE_FATAL_ERROR(ret))
	{	
        if(g_db_state != DB_RESTARTING)
        {
    		g_db_state = DB_RESTARTING;
    		g_db_errcode = ret;
    	#ifdef EVENT_SUPPORT
            brocast_db_error();
        #else
    		//pthread_kill(get_signal_task_id(), SIGUSR2);	
        #endif
        }
	}
	
EXIT:
	p_OprObj->ret = ret;
		
	if(p_obj->ret_flag == MSG_NO_RETURN)
	{
		free_message(&p_msg);
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
			log_warning("sem_post error in database handler");
			return ESEM_POST;
		}
	}
	else
	{
		log_warning("invalid return flag\n");
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
	int ret = AIRNAS_ERRNO;
		
	if(task == NULL || p_msg == NULL)
	{
		log_warning("database task: invalid parameters\n");
		return ENULL_POINT;
	}
#if 0
	if(g_db_state != DB_NORMAL_RUNNING)
	{
		log_warning("db cannot work now,state=%d", g_db_state);
		
		if(g_db_state == DB_RESTARTING)
		{
			ret = EDB_RESTARTING;
		}
		else if(g_db_state == DB_STOP)
		{
			ret = EDB_STOP_WORKING;
		}

        if(p_obj->m_type != MSG_DB_WRITE_EXIT)
        {
		    goto EXIT;
        }
        else
        {
            log_notice("db_write recv exit msg");
        }
	}
#endif
	if(p_obj->m_type != MSG_DB_WRITE_EXIT)
	{
		if((ret = start_transaction(database)) != RET_SUCCESS)
		{
			log_warning("db start transaction error!");
			goto DB_ERROR_HANDLE;
		}
	}
	
	 //add new items into database
	if(p_obj->m_type >= MSG_DB_FILE_SINGLE_ADD && p_obj->m_type <= MSG_DB_FILE_BATCH_ADD)
	{
		//ret = insert_handler(p_obj->m_type, database, p_OprObj);
		ret = dm_db_insert_file(database, &p_OprObj->data.insert_data);
	}
	else if(p_obj->m_type == MSG_DB_FILE_COPY)
	{//copy item
		ret = db_copy_file(database, &p_OprObj->data.copy_data);
	}
	else if(p_obj->m_type == MSG_DB_FILE_UPDATE)
	{
		ret = db_update_file(database, &p_OprObj->data.file_update_data);
	}
	else if(p_obj->m_type == MSG_DB_FILE_MOVE)
	{//move item
		ret = db_move_file(database, &p_OprObj->data.move_data);
	}
	else if(p_obj->m_type == MSG_DB_FILE_DELETE)
	{//delete item
		ret = db_delete_file(database, &p_OprObj->data.delete_data);
	}
	else if(p_obj->m_type == MSG_DB_FILE_TRASH)
	{
		ret = db_trash_file(database, &p_OprObj->data.trash_data);
	}
	else if(p_obj->m_type == MSG_DB_FILE_RECYCLE)
	{
		ret = db_recycle_file(database, &p_OprObj->data.recycle_data);
	}
	else if(p_obj->m_type == MSG_DB_FILE_RENAME)
	{
		ret = db_rename_file(database, &p_OprObj->data.rename_data);
	}
	else if(p_obj->m_type == MSG_DB_BACKUP_FILE_ADD)
	{
		ret = db_insert_backup_file(database, &p_OprObj->data.backup_file_data);
	}
	else if(p_obj->m_type == MSG_DB_BACKUP_FILE_DEL)
	{
		ret = db_delete_backup_file(database, &p_OprObj->data.delete_data);
	}
	else if(p_obj->m_type == MSG_DB_BACKUP_FILE_UPDATE)
	{
		ret = db_update_backup_file(database, &p_OprObj->data.backup_file_update_data);
	}
	else if(p_obj->m_type == MSG_DB_USER_INFO_ADD)
	{
		ret = db_insert_user(database, &p_OprObj->data.user_data);
	}
	else if(p_obj->m_type == MSG_DB_USER_INFO_DEL)
	{
		ret = db_delete_user(database, &p_OprObj->data.user_del_data);
	}
	else if(p_obj->m_type == MSG_DB_USER_INFO_UPDATE)
	{
		//ret = db_update_user(database, &p_OprObj->data.user_update_data);
	}
	else if(p_obj->m_type == MSG_DB_DEVICE_INFO_ADD)
	{
		ret = db_insert_device(database, &p_OprObj->data.device_data);
	}
	else if(p_obj->m_type == MSG_DB_SHARE_FILE)
	{
		ret = db_share_file(database, &p_OprObj->data.share_data);
	}
	else if(p_obj->m_type == MSG_DB_UPDATE_FILE)
	{
		ret = db_update_file(database, &p_OprObj->data.file_update_data);
	}
	else if(p_obj->m_type == MSG_DB_HDISK_INFO_ADD)
	{
		ret = db_insert_hdisk(database, &p_OprObj->data.hdisk_data);
	}
	else if(p_obj->m_type == MSG_DB_HDISK_INFO_UPDATE)
	{
		ret = db_update_hdisk(database, &p_OprObj->data.hdisk_data);
	}
    else if(p_obj->m_type == MSG_DB_COMMIT)
    {
        ret = db_commit(database);
    }
	else if(p_obj->m_type == MSG_DB_CLEAN_UP_FILE)
	{
		ret = db_clean_up_invalid_file(database, &p_OprObj->data.cln_data);
	}
	else if(p_obj->m_type == MSG_DB_REINDEX)
	{
		ret = db_reindex_table(database);
	}
	else if(p_obj->m_type == MSG_DB_WRITE_EXIT)
	{
		task->exit_flag = 1;
        // Fix bug: after format hdisk, re-register mq failed.
        unregister_mq(task->msg_type);
	    release_mq(&task->task_mq);
		ret = RET_SUCCESS;
	}else if(p_obj->m_type == MSG_DB_SCANNING_DISK)
	{
		DMCLOG_D("access db_scan_disk_file");
		ret = db_scan_disk_file(database, &p_OprObj->data.scan_data);
	}

	if(p_obj->m_type != MSG_DB_WRITE_EXIT)
	{
		if(ret == RET_SUCCESS)
		{
			if((ret = commit_transaction(database)) != RET_SUCCESS)
			{
				log_warning("db commit transaction error!");
				goto DB_ERROR_HANDLE;
			}
		}
		else
		{
			if(rollback_transaction(database) != RET_SUCCESS)
			{
				log_warning("db rollback error");
			}
		}
	}

DB_ERROR_HANDLE:
	if(SQLITE_FATAL_ERROR(ret))
	{
        if(g_db_state != DB_RESTARTING)
        {
    		g_db_state = DB_RESTARTING;
    		g_db_errcode = ret;
        #ifdef EVENT_SUPPORT
            brocast_db_error();
        #else
    		//pthread_kill(get_signal_task_id(), SIGUSR2);	
        #endif
        }
	}
	
EXIT:	
	p_OprObj->ret = ret;


	if(p_obj->ret_flag == MSG_NO_RETURN)
	{
		free_message(&p_msg);
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
			log_warning("sem_post error in database handler");
			return 0;
		}
	}
	else
	{
		log_warning("invalid return flag\n");
	}
	
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
		log_warning("cannot close db write handle,ret=0x%x",ret);
		return ret;	
	}
    write_connection = NULL;
    
//#if 0
	if((ret = close_db_connection(query_connection)) != SQLITE_OK)
	{
		log_warning("cannot close db read handle,ret=0x%x",ret);
		return ret;	
	}
    query_connection = NULL;
//#endif

    return RET_SUCCESS;
}

int create_db_all_connection(void)
{
    int ret;
    
	if((ret = create_db_connection(&write_connection, 1)) != RET_SUCCESS)
	{
		return ret;
	}
//#if 0
	if((ret = create_db_connection(&query_connection, 0)) != RET_SUCCESS)
	{
		return ret;
	}
//#endif

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
	DMCLOG_D("g_db_query_task.msg_cb is not NULL ");
    g_db_query_task.arg = &query_connection;
	g_db_query_task.exit_cb = db_task_cleanup;

	if(create_db_connection(&write_connection, 1) != RET_SUCCESS)
	{
        write_connection = NULL;
		return -1;
	}
	g_db_write_task.msg_cb = db_write_task_msg_cb;
	DMCLOG_D("g_db_query_task.msg_cb is not NULL ");
	DMCLOG_D("g_db_write_task.msg_cb = %x",g_db_write_task.msg_cb);
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

int create_db_disk_task(struct disk_node *disk_info)
{
    int ret = -1;
    
	MEMSET(disk_info->g_db_query_task);
	MEMSET(disk_info->g_db_write_task);

	if(create_db_disk_connection(&disk_info->query_connection,disk_info->g_database,0) != RET_SUCCESS)
	{
		return -1;
	}

    disk_info->g_db_query_task.msg_cb = db_query_task_msg_cb;
    disk_info->g_db_query_task.arg = &disk_info->query_connection;
	disk_info->g_db_query_task.exit_cb = db_task_cleanup;

	if(create_db_disk_connection(&disk_info->write_connection,disk_info->g_database, 1) != RET_SUCCESS)
	{
        disk_info->write_connection = NULL;
		return -1;
	}
	disk_info->g_db_write_task.msg_cb = db_write_task_msg_cb;
    disk_info->g_db_write_task.arg = &disk_info->write_connection;
	disk_info->g_db_write_task.exit_cb = db_task_cleanup;


    /*ret = create_task(&disk_info->g_db_query_task, "db_query_task", MSG_TYPE_DB_QUERY);
    if(ret < 0)
    {
        printf("create_db_task failed!");
        return ret;
    }
	ret = create_task(&disk_info->g_db_write_task, "db_write_task", MSG_TYPE_DB_WRITE);
	if(ret < 0)
	{
		printf("create_db_task failed!");
		return ret;
	}*/
	
	//disk_info->g_db_state = DB_NORMAL_RUNNING;
	
    return 0;
}


#ifdef EVENT_SUPPORT
extern int stop_db_task(void);
static int _handle_event_server_ctrl_db_error_msg(void *self)
{
    EventServerSubHandle *handler = (EventServerSubHandle *)(self);
    int db_error_code = (*(int *)(handler->ctrl_msg_handler_arg));

    static int last_error = RET_SUCCESS;
	static int times = 0;
    int ret;
    
    log_warning("handle DbError(0x%x)!", db_error_code);
    if(db_error_code != RET_SUCCESS)
    {
        set_sys_status(DB_RECOVERING);
    }

    if(last_error == RET_SUCCESS)
	{
		last_error = db_error_code;
	}
	else
	{
		if(last_error == db_error_code)//we meet the same error!
		{
			times++;
			if(times > DB_ERROR_TOLERANCE)//it means we have continuously met a same error
			{
				log_error("db error(0x%x) cannot be solved by restart db, now going to shutdown the process!",
						last_error);
				sleep(2);
				//exit(-1);
				stop_airnas_process();
                return EDB;
			}
		}
		else//a new error
		{
			times = 0;
		}
	}

    last_error = db_error_code;//update last error to the current db error
    if((ret = close_db_all_connection()) != RET_SUCCESS)
    {
        log_error("close_db_all_connection");
        stop_airnas_process();
        return ret;
    }

    if(db_error_code == SQLITE_IOERR)//if io error.
    {
        log_warning("handle sqlite_ioerr");
        log_switch_dir(PRE_LOG_PATH);
        partition_umount("/dev/sda1");

        uint8_t try_times = 0;
        while(try_times < 3)
        {
            update_system_storage_info(get_sys_storage_ptr(), \
                                        UPDATE_DISK | UPDATE_PARTION);
            ret = get_sdisk_status(get_sys_storage_ptr());
            if(ret == RET_SUCCESS)
            {
                log_notice("sdisk is ok!");
                break;
            }
            else if(ret == EDISK_NOT_EXIST)
            {
                log_error("disk not found, we will reboot after 3s");
                stop_db_task();
    			sleep(3);
    			system("reboot -f");
            }
            else if(ret == EPARTION_NOT_EXIST)
            {
                log_error("partion not found, we will reboot after 3s");
                stop_db_task();
    			sleep(3);
    			system("reboot -f");
            }
            else if(ret == EPARTION_NOT_MOUNT)
            {
                log_warning("re-mount(%dth) partion sda1!", try_times);
                // it has error!!! by wenhao
                char dev_node[32];
                S_SNPRINTF(dev_node, sizeof(dev_node), "/dev/%s", DATA_PARTITION_NAME);
                partition_mount(dev_node, get_sys_nas_data_path());
            }
            else
            {
                log_error("disk error, reboot it");
                stop_db_task();
    			sleep(3);
    			system("reboot -f");
            }

            ++try_times;
            sleep(1);
        }

        if(try_times >= 3)
        {
            log_error("re-mount partion sda1 failed, reboot after 1s");
            //stop_airnas_process();
            stop_db_task();
			sleep(1);
			system("reboot -f");
            return AIRNAS_ERRNO;
        }
        else
        {
            log_notice("re-mount partion sda1 success!");
            //log_switch_dir(get_sys_nas_log_name());
        }
	}

    if(db_error_code == SQLITE_CORRUPT)
    {
        log_warning("handle sqlite_corrupt");
        system("/usr/sbin/handle_sqlite_corrupt.sh");
        sleep(1);
    }

    //for db error, we try to restart db connection
	if((ret = create_db_all_connection()) != RET_SUCCESS)
	{
		log_error("create_db_all_connection failed, ret(0x%x)\nWe will exit airnas process", ret);
		stop_airnas_process();
        return EDB;
	}

    recover_db();
    set_sys_status(NORMAL);
    return RET_SUCCESS;
}

int add_event_server_ctrl_db_error_handler(void)
{
    EventServerSubHandle handler;
    memset(&handler, 0, sizeof(handler));

    handler.ctrl_msg_id = EVENT_SERVER_DB_ERROR_MSG_ID;
    S_STRNCPY(handler.ctrl_msg_name, "DbError", sizeof(handler.ctrl_msg_name));
    handler.ctrl_msg_handler = _handle_event_server_ctrl_db_error_msg;
    handler.ctrl_msg_handler_arg = &g_db_errcode;
    
    return add_event_server_ctrl_sub_handler(&handler);
}

int brocast_db_error(void)
{
    uint16_t msg = EVENT_SERVER_DB_ERROR_MSG_ID;
    log_notice("brocast");
    return send_msg_to_event_server((char *)(&msg), sizeof(msg));
}
#endif


