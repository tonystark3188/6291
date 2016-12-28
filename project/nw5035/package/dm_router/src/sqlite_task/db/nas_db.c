/*
 * =============================================================================
 *
 *       Filename: nas_db.c
 *
 *       Description: interface of db requests handle task and the  db module.
 *
 *        Version:  1.0
 *        Created:  2014/09/12 
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Tao Sheng (), vanbreaker90@gmail.com
 *   Organization:  
 *
 * =============================================================================
 */


#include "db/nas_db.h"
#include "db/file_table.h"
#include "db/user_table.h"
#include "db/hdisk_table.h"
#include "db/db_check.h"
#include "db/db_table.h"
#include "config.h"
#include "list.h"
#include "disk_manage.h"


extern db_table_t g_file_table;
extern db_table_t g_video_table;
extern db_table_t g_audio_table;
extern db_table_t g_image_table;
extern db_table_t g_user_table;
extern db_table_t g_hdisk_table;
extern db_table_t g_device_table;
extern db_table_t g_version_table;
extern db_table_t g_backup_file_table;

static error_t dm_do_insert_file(sqlite3 *database, file_info_t *pfi)
{
    file_type_t type;
	error_t errcode = RET_SUCCESS;

	type = get_file_type(pfi);
    
    if(type == TYPE_DIR)
    {
	    pfi->file_size = 0;
	}

	//finally, insert the record to file table.
	if(g_file_table.ops.insert)
	{
	    if((errcode = g_file_table.ops.insert(database, pfi)) != RET_SUCCESS)
	    {
	    	log_error("insert to file_table error...errcode=0x%X", errcode);
			return errcode;
		}
	}
	return errcode;
}

error_t dm_db_insert_file(sqlite3 *database, insert_data_t *pdata)
{
	file_info_t *pfi;
	error_t errcode = RET_SUCCESS;
	
	pfi = &pdata->file_info;

	if((errcode = dm_do_insert_file(database, pfi))
			!= RET_SUCCESS)
	{
		log_warning("error when insert %s to file_table\n",pfi->path);
		return errcode;
	}
	return errcode;
}

error_t db_scan_disk_file(sqlite3 *database, scan_data_t *data)
{
	error_t errcode;

	log_debug("enter");
	if(g_file_table.ops.scan == NULL)
	{
		DMCLOG_D("scan is NULL");
		return errcode;
	}
	if((errcode = g_file_table.ops.scan(database, data))) 
	{
		log_warning("insert hdisk table error,errcode=0x%X",errcode);
		return errcode;
	}
	
	log_debug("exit OK......");
	return errcode; 
}

static error_t file_table_delete_callback(sqlite3 *database, void *private_data, 
								update_info_t *pui)
{
	file_info_t *pfi = (file_info_t *)private_data;
    file_type_t  type;
	
	if(pfi == NULL)
	{
		return EINVAL_ARG;
	}

    if(pui != NULL)
    {
		pui->total_update_size += pfi->file_size;
    }
    //other type ..should not do anything more, directly return true.
    return RET_SUCCESS;
}



/*
*description: delete  file record and update according tables.
*input param:pdata-->delete target  info

*return: RET_SUCCESS if ok
*/
error_t db_delete_file(sqlite3 *database, delete_data_t *pdata)
{
	error_t errcode;
	// delete in file_table
	if((errcode = g_file_table.ops.dm_delete(database, pdata)) != RET_SUCCESS)
	{
		if(errcode == EDB_RECORD_NOT_EXIST)
		{
			return RET_SUCCESS;
		}
		log_warning("error when deleting from file_table,file path=%s\n", pdata->path);
		return errcode;
	}
	return errcode;
}

error_t db_generic_delete_file(sqlite3 *database, delete_data_t *pdata)
{
	error_t errcode;

	log_debug("enter......");

	// delete in file_table
	if((errcode = g_file_table.ops.generic_delete(database, pdata)) != RET_SUCCESS)
	{
		if(errcode == EDB_RECORD_NOT_EXIST)
		{
			return RET_SUCCESS;
		}
		log_warning("error when deleting from file_table,file path=%s\n", pdata->path);
		return errcode;
	}
	log_debug("exit ok......");

	return errcode;
}


error_t db_move_file(sqlite3 *pdatabase, move_data_t *pdata)
{
	error_t errcode = RET_SUCCESS;
	update_info_t ui;

	memset(&ui, 0, sizeof(ui));

	if((errcode = g_file_table.ops.move(pdatabase, pdata))
		 	!= RET_SUCCESS)
	{
		log_warning("error when moving file(id=%u) to file (id=%u)\n",
				pdata->target_id, pdata->new_parent);
	}
	return errcode;
}


error_t db_rename_file(sqlite3 *pdatabase, rename_data_t *pdata)
{
	error_t errcode = RET_SUCCESS;

	if((errcode = g_file_table.ops.rename(pdatabase, pdata->src_path, pdata->des_path))
			!= RET_SUCCESS)
	{
		log_warning("error when renaming file(id=%u) to new name %s\n", 
			pdata->target_id, pdata->new_name);
	}
	return errcode;
}

error_t db_copy_file(sqlite3 *database, copy_data_t *pdata)
{
	update_info_t ui;
	error_t errcode = RET_SUCCESS;

	memset(&ui, 0, sizeof(ui));
	//ui.user_id = pdata->user_id;
	//ui.hdisk_id = pdata->hdisk_id;
	
	if((errcode = g_file_table.ops.copy(database, pdata->target_id, pdata->dest_id, &ui))
			!= RET_SUCCESS)
	{
		log_warning("error when copying file(id=%u) to dir(id=%u)\n",
			pdata->target_id, pdata->dest_id);
		return errcode;
	}
	return errcode;
}



/*
*description: update file record information.
*input param:pdata-->update target  info

*return: RET_SUCCESS if ok
*/
error_t db_update_file(sqlite3 *database, file_update_t *pdata)
{
	error_t errcode;
	if((errcode = g_file_table.ops.active_update(database, pdata))
			!= RET_SUCCESS)
	{
		DMCLOG_E("update file error");
		return errcode;
	}
	return errcode;
}

/*
*description: insert user record.
*input param:pdata-->delete target  info

*return: RET_SUCCESS if ok
*/
error_t db_insert_user(sqlite3 *database, user_info_t *pdata)
{
	error_t errcode = RET_SUCCESS ;
	log_debug("enter......");

	log_debug("start insert user table");
	if((errcode = g_user_table.ops.insert(database, pdata)) != RET_SUCCESS)
	{
		log_warning("user table insert error.errcode=0x%X",errcode);
		return errcode;
	}
	log_debug("insert user table ok");

	log_debug("exit ok......");

	

	return errcode;
}

/*
*description: delete  user
*input param:pdata-->delete target  user info

*return: RET_SUCCESS if ok
*/
error_t db_delete_user(sqlite3 *database, user_del_data_t *pdata)
{
	ENTER_FUNC();
	user_info_t user;
	error_t errcode = RET_SUCCESS;
	memset(&user, 0, sizeof(user_info_t));
	DMCLOG_D("pdata->ip = %s",pdata->ip);
	if(pdata->ip != NULL&&*pdata->ip)
	{
		strcpy(user.ip,pdata->ip);
		if((errcode = g_user_table.ops.query(database, QUERY_USER_INFO_BY_IP, &user))
				!= RET_SUCCESS)
		{
			if(errcode == EDB_RECORD_NOT_EXIST)//user record not exist
			{
				DMCLOG_D("user to delete not exist(ip=%s)\n", user.ip);
				return RET_SUCCESS;
			}
			DMCLOG_D("error when querying user info(ip=%s)\n",user.ip);
			return errcode;
		}
	}else{
		DMCLOG_D("pdata->session = %s",pdata->session);
		strcpy(user.session,pdata->session);
	}
	DMCLOG_D("user.session = %s",user.session);
	//query user information.
	if((errcode = g_user_table.ops.query(database, QUERY_USER_INFO_BY_SESSION, &user))
				!= RET_SUCCESS)
	{
		if(errcode == EDB_RECORD_NOT_EXIST)//user record not exist
		{
			log_debug("user to delete not exist(session=%s)", user.session);
			return RET_SUCCESS;
		}
		log_warning("error when querying user info(session=%s)\n",user.session);
		return errcode;
	}
	//delete user record in user_table
	if((errcode = g_user_table.ops.lazy_delete(database, user.session))
			!= RET_SUCCESS)
	{
		if(errcode == EDB_RECORD_NOT_EXIST)//user record not exist
		{
			log_debug("user to delete not exist(session=%s)", user.session);
			return RET_SUCCESS;
		}
		log_warning("error when delete user(session=%s) in user table\n", user.session);
		return errcode;
	}
	EXIT_FUNC();
	return errcode;
}

/*
*description: insert device record.
*input param:pdata-->delete target  info

*return: RET_SUCCESS if ok
*/
error_t db_insert_device(sqlite3 *database, device_info_t *pdi)
{
	error_t errcode;

	log_debug("enter......");
	if((errcode =  g_device_table.ops.insert(database, pdi)) != RET_SUCCESS)
	{
		log_warning("insert device_table error.errcode = 0x%X", errcode);
		return errcode;
	}
	log_debug("exit ok......");
	return errcode;
}


error_t db_insert_hdisk(sqlite3 *database, void *data)
{
	ENTER_FUNC();
	error_t errcode;
	if((errcode = g_hdisk_table.ops.insert(database, data))) 
	{
		log_warning("insert hdisk table error,errcode=0x%X",errcode);
		return errcode;
	}
	
	EXIT_FUNC();
	return errcode; 
}

error_t db_update_hdisk(sqlite3 *database, void *data)
{
	error_t errcode;

	log_debug("enter");
	if((errcode = g_hdisk_table.ops.update(database, data))) 
	{
		log_warning("insert hdisk table error,errcode=0x%X",errcode);
		return errcode;
	}
	
	log_debug("exit OK......");
	return errcode; 
}


/*
*description: query database.
*input param:approach-->what to query
*output param:data-->target and result buffer 
*return: RET_SUCCESS if ok
*/
error_t db_query(sqlite3 *database, QueryApproach approach, void *data)
{
	error_t errcode = RET_SUCCESS;

	switch(approach)
	{
		case QUERY_FILE_PATH://query file's full path
		{
			//file_info_t *pfi = &((full_path_t *)data)->file_info;

			log_debug("start querying file path");
			errcode = g_file_table.ops.query(database, approach, data);
			if(errcode == EDB_RECORD_NOT_EXIST)
			{
				log_trace("file not exist");
				errcode = RET_SUCCESS;
			}
			log_debug("end querying file path");
			break;
		}
		case QUERY_BATCH_FILE_PATH:
		{
			log_debug("start querying batche path");
			errcode = g_file_table.ops.query(database, approach, data);
			log_debug("end querying batch path");
			break;
		}
		case QUERY_GROUP_LIST:
		{
			log_debug("start querying group list");
			errcode = g_file_table.ops.query(database, approach, data);
			log_debug("end querying group list");
			break;
		}
		case QUERY_FILE_LIST_COUNT:
		{
			file_list_t *plist = (file_list_t *)data;

			//log_debug("end querying file list count");
			break;
		}
		case QUERY_FILE_DIR_LIST_COUNT:
		{
			file_list_t *plist = (file_list_t *)data;
			break;
		}
		case QUERY_FILE_LIST_COUNT_BY_PATH:
		{
			file_list_t *plist = (file_list_t *)data;
			//log_debug("end querying file list count");
			break;
		}
		case QUERY_FILE_LIST_BY_PATH:
		{
			file_list_t *plist = (file_list_t *)data;
			//log_debug("end querying file list count");
			break;
		}
		case QUERY_FILE_LIST:
		{
			//file_info_t *pfi = NULL;
			file_list_t *plist = (file_list_t *)data;
			errcode = g_file_table.ops.query(database, approach, data);

			break;
		}
		case QUERY_DIR_LIST:
		{
			//file_info_t *pfi = NULL;
			file_list_t *plist = (file_list_t *)data;
			
			errcode = g_file_table.ops.query(database, approach, data);
			break;
		}
		case QUERY_HASH_CODE_FILE_LIST:
		{
			errcode = g_file_table.ops.query(database, approach, data);
			break;
		}
		case QUERY_FILE_INFO:
		case QUERY_FILE_BY_UUID:
		case QUERY_FILE_BY_NAME:
		{
			file_info_t *pfi = (file_info_t *)data;
	
			if((errcode = g_file_table.ops.query(database, approach, data)) != RET_SUCCESS)
			{
				log_warning("file table querying error,errcode=0x%X", errcode);
				return errcode;
			}

			break;
		}
		case QUERY_USER_LIST://query user list.
		{
			log_debug("start querying user list");
			errcode = g_user_table.ops.query(database, approach, data);
			log_debug("end querying user list");
			break;
		}
		case QUERY_USER_INFO_BY_ID:	
		case QUERY_USER_INFO_BY_NAME:
		case QUERY_USER_INFO_BY_SESSION:	
		{
			log_debug("start querying user info");
			errcode = g_user_table.ops.query(database, approach, data);
			if(errcode == EDB_RECORD_NOT_EXIST)
			{
				//errcode = RET_SUCCESS;
				log_trace("querying user not exist");
			}
			log_debug("end querying user info");
			break;
		}
		case QUERY_DEVICE_BY_UUID:
		case QUERY_DEVICE_BY_USER:
		case QUERY_DEVICE_BY_USER_MAC:
		case QUERY_DEVICE_BY_USER_MAC_NAME:
		{
			log_debug("start querying user's device");
			errcode = g_device_table.ops.query(database, approach, data);
			log_debug("end querying user's device");
			break;
		}
		case QUERY_HDISK_INFO:
		{
			log_debug("start querying hdisk info");
			errcode = g_hdisk_table.ops.query(database,approach,data);
            if(errcode == EDB_RECORD_NOT_EXIST)
			{
				errcode = RET_SUCCESS;
				log_trace("querying hdisk not exist");
			}
			log_debug("end querying hdisk info");
			break;
		}
		case QUERY_BACKUP_FILE_LIST:
		{
			//log_debug("start querying backup file");
			//errcode = g_backup_file_table.ops.query(database, approach, data);
			//log_debug("end querying backup file");
			break;
		}
		case QUERY_BACKUP_DEVICE_INFO:
		{
			//log_debug("start querying backup device size");
			//errcode = g_backup_file_table.ops.query(database, approach, data);
			//log_debug("end querying backup device size");
			break;
		}
		case QUERY_BACKUP_FILE:
		{
			//log_debug("start querying backup file");
			//errcode = g_backup_file_table.ops.query(database, approach, data);
			//log_debug("end querying backup file list");
			break;
		}
		case QUERY_MAX_THUMB_NO:
		{
			log_debug("start querying max thumbnail num");
			errcode = g_file_table.ops.query(database, approach, data);
			log_debug("end querying max thumbnail num");
			break;
		}
		case QUERY_NO_THM_PHOTO_LIST:
		{
			log_debug("start querying no thumbnail photo list");
			errcode = g_file_table.ops.query(database, approach, data);
			log_debug("end querying no thumbnail photo list");
			break;
		}

		default: break;
	}

	if(errcode != RET_SUCCESS)
	{
		log_warning("query db error, errcode = 0x%X",errcode);
	}

	return errcode;
}

static error_t config_db_file(sqlite3 *database)
{
	error_t errcode;
    char tmp_buf[SQL_CMD_QUERY_BUF_SIZE];
	version_info_t version;

	if((errcode = sqlite3_exec_busy_wait(database, SQLITE_CREATE_DEVICE_TABLE, NULL, NULL)) != SQLITE_OK)
	{
		log_warning("create device_table error");
		return errcode;
	}

    /*if((errcode = sqlite3_exec_busy_wait(database, SQLITE_CREATE_BACKUP_FILE_TABLE, NULL, NULL)) != SQLITE_OK)
	{
		log_warning("create backup_file_table error");
		return errcode;
	}*/
	if((errcode = sqlite3_exec_busy_wait(database, SQLITE_CREATE_USER_TABLE, NULL, NULL)) != SQLITE_OK)
	{
		log_warning("create user_table error");
		return errcode;
	}
	if((errcode = sqlite3_exec_busy_wait(database, SQLITE_CREATE_HDISK_TABLE, NULL, NULL)) != SQLITE_OK)
	{
		log_warning("create hdisk_table error");
		return errcode;
	}

	if((errcode = sqlite3_exec_busy_wait(database, SQLITE_CREATE_VERSION_TABLE, NULL, NULL)) != SQLITE_OK)
	{
		log_warning("create version_table error");
		return errcode;
	}

	#if 0
	S_STRNCPY(version.version, get_sys_db_version(), MAX_VERSION_SIZE);
	version.state = 0;
	if((errcode = version_table_insert(database, &version)) != RET_SUCCESS)
	{
		log_warning("insert version info error.");
		return errcode;
	}	
	#endif
	return errcode;
}
static error_t config_db_disk_file(sqlite3 *database)
{
	error_t errcode;
	version_info_t version_info;
	if((errcode = sqlite3_exec_busy_wait(database, SQLITE_CREATE_FILE_TABLE, NULL, NULL)) != SQLITE_OK)
	{
		DMCLOG_E("create device_table error");
		return errcode;
	}
    /*if((errcode = sqlite3_exec_busy_wait(database, SQLITE_CREATE_BACKUP_FILE_TABLE, NULL, NULL)) != SQLITE_OK)
	{
		DMCLOG_E("create backup_file_table error");
		return errcode;
	}*/
	if((errcode = sqlite3_exec_busy_wait(database, "CREATE INDEX IF NOT EXISTS TYPE_INDEX ON file_table(TYPE)", 
			NULL, NULL)) != SQLITE_OK)
	{
		DMCLOG_E("create type_index error");
		sqlite3_close(database);
		return errcode;
	}
	if((errcode = sqlite3_exec_busy_wait(database, "CREATE  VIEW IF NOT EXISTS image_view AS\
		SELECT * FROM file_table WHERE TYPE=3 ORDER BY CHANGE_TIME ASC", NULL, NULL)) != SQLITE_OK)
	{
		DMCLOG_E("create type_index error");
		sqlite3_close(database);
		return errcode;
	}
	if((errcode = sqlite3_exec_busy_wait(database, "CREATE  VIEW IF NOT EXISTS audio_view AS\
		SELECT * FROM file_table WHERE TYPE=2 ORDER BY CHANGE_TIME ASC", NULL, NULL)) != SQLITE_OK)
	{
		DMCLOG_E("create type_index error");
		sqlite3_close(database);
		return errcode;
	}
	if((errcode = sqlite3_exec_busy_wait(database, "CREATE  VIEW IF NOT EXISTS video_view AS\
		SELECT * FROM file_table WHERE TYPE=1 ORDER BY CHANGE_TIME ASC", NULL, NULL)) != SQLITE_OK)
	{
		DMCLOG_E("create type_index error");
		sqlite3_close(database);
		return errcode;
	}
	if((errcode = sqlite3_exec_busy_wait(database, "CREATE  VIEW IF NOT EXISTS docum_view AS\
		SELECT * FROM file_table WHERE TYPE=4 ORDER BY CHANGE_TIME ASC", NULL, NULL)) != SQLITE_OK)
	{
		DMCLOG_E("create type_index error");
		sqlite3_close(database);
		return errcode;
	}

	if((errcode = sqlite3_exec_busy_wait(database, "CREATE  VIEW IF NOT EXISTS backup_file_view AS\
		SELECT * FROM file_table WHERE FILE_UUID<>'1234567890'", NULL, NULL)) != SQLITE_OK)
	{
		DMCLOG_E("create type_index error");
		sqlite3_close(database);
		return errcode;
	}
				
	if((errcode = sqlite3_exec_busy_wait(database, SQLITE_CREATE_VERSION_TABLE, NULL, NULL)) != SQLITE_OK)
	{
		log_warning("create version_table error");
		return errcode;
	}
	memset(&version_info,0,sizeof(version_info_t));
	if((errcode = version_table_query(database, &version_info)) != RET_SUCCESS)
	{
		DMCLOG_E("query version info error");
		return errcode;
	}
	
	/*if(*version_info.version)
	{
		DMCLOG_D("db version:%s,sys db version:%s",version_info.version,get_sys_db_version());
		if(strcmp(version_info.version,get_sys_db_version()) != 0)
		{
			if((errcode = sqlite3_exec_busy_wait(database, "DELETE FROM version_table", NULL, NULL)) != SQLITE_OK)
			{
				DMCLOG_E("delete version_table error");
				sqlite3_close(database);
				return errcode;
			}
			if((errcode = sqlite3_exec_busy_wait(database, "DELETE FROM file_table", NULL, NULL)) != SQLITE_OK)
			{
				DMCLOG_E("delete file_table error");
				sqlite3_close(database);
				return errcode;
			}	
			if((errcode = sqlite3_exec_busy_wait(database, "DELETE FROM backup_file_table", NULL, NULL)) != SQLITE_OK)
			{
				DMCLOG_E("delete backup_file_table error");
				sqlite3_close(database);
				return errcode;
			}
			S_STRNCPY(version_info.version, get_sys_db_version(), MAX_VERSION_SIZE);
			version_info.state = 0;// 1:test version,0:formal version
			if((errcode = version_table_insert(database, &version_info)) != RET_SUCCESS)
			{
				DMCLOG_E("insert version info error.");
				return errcode;
			}
		}
	}else{
		DMCLOG_D("version table id null");
		if((errcode = sqlite3_exec_busy_wait(database, "DELETE FROM file_table", NULL, NULL)) != SQLITE_OK)
		{
			DMCLOG_E("delete file_table error");
			sqlite3_close(database);
			return errcode;
		}	
		if((errcode = sqlite3_exec_busy_wait(database, "DELETE FROM backup_file_table", NULL, NULL)) != SQLITE_OK)
		{
			DMCLOG_E("delete backup_file_table error");
			sqlite3_close(database);
			return errcode;
		}
		S_STRNCPY(version_info.version, get_sys_db_version(), MAX_VERSION_SIZE);
		version_info.state = 0;// 1:test version,0:formal version
		if((errcode = version_table_insert(database, &version_info)) != RET_SUCCESS)
		{
			DMCLOG_E("insert version info error.");
			return errcode;
		}
	}*/
	return errcode;
}

error_t db_reindex_table(sqlite3 *database)
{
	error_t errcode;

	if((errcode = g_file_table.ops.reindex_table(database)) != RET_SUCCESS)
	{
		log_warning("file table reindex error(0x%X)", errcode);
		return errcode;
	}

	return errcode;

}


static error_t create_db_file(char *name)
{
	sqlite3 *db;
	error_t errcode;
	
	errcode = sqlite3_open_v2(name, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	if(errcode != SQLITE_OK)
	{
		DMCLOG_D("sqlite open db file error,errcode=0x%X",errcode);
		return errcode;
	}
	
	if((errcode = config_db_file(db)) != SQLITE_OK)
	{
        sqlite3_close(db);
		return errcode;
	}
	
	sqlite3_close(db);
	return errcode;
}

static error_t create_db_disk_file(char *name)
{
	ENTER_FUNC();
	sqlite3 *db;
	error_t errcode;
	
	errcode = sqlite3_open_v2(name, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	if(errcode != SQLITE_OK)
	{
		DMCLOG_E("sqlite open db file error,errcode=0x%X",errcode);
		
		return errcode;
	}
	
	if((errcode = config_db_disk_file(db)) != SQLITE_OK)
	{
        sqlite3_close(db);
		EXIT_FUNC();
		return errcode;
	}
	sqlite3_close(db);
	EXIT_FUNC();
	return errcode;
}




static error_t db_init(bool hdisk_formated)
{
	error_t errcode = RET_SUCCESS;

	if(hdisk_formated)
	{
		if((errcode = create_db_file(DATABASE)) != RET_SUCCESS)
		{
			DMCLOG_D("create db file error");
			return errcode;
		}
	}
	if((errcode = user_table_init()) != RET_SUCCESS)
	{
		log_warning("user table init error,error code=0x%x", errcode);
		return errcode;
	}

	if((errcode = hdisk_table_init()) != RET_SUCCESS)
	{
		log_warning("hdisk table init error,error code=0x%x", errcode);
		return errcode;
	}


	if((errcode = device_table_init() != RET_SUCCESS))
	{
		log_warning("device table init error,error code=0x%x", errcode);
		return errcode;
	}
	return errcode;
}

int ftruncate_database(char *path)
{
	
	/* 打开一个文件 */
	int fd = open(path,O_RDWR);
	if(fd < 0)
	{
		DMCLOG_D("open %s failed",path);
		return -1;
	}
	else
	{
		DMCLOG_D("open %s successful",path);
	
		/* 清空文件 */
		ftruncate(fd,0);
	
		/* 重新设置文件偏移量 */
		lseek(fd,0,SEEK_SET);
	
		close(fd);
	}
	return 0;
}
static error_t db_disk_init(char *name)
{
	error_t errcode = RET_SUCCESS;
	if((errcode = create_db_disk_file(name)) != RET_SUCCESS)
	{
		DMCLOG_E("create db file error");
		if(ftruncate_database(name) == 0)
			return create_db_disk_file(name);
		else
			return errcode;
	}
	if((errcode = file_table_init(name)) != RET_SUCCESS)
	{
		DMCLOG_E("file table init error,error code=0x%x", errcode);
		return errcode;
	}
	/*if((errcode = backup_file_table_init(name)) != RET_SUCCESS)
	{
		DMCLOG_E("file table init error,error code=0x%x", errcode);
		return errcode;
	}*/
	return errcode;
}


error_t db_commit(sqlite3 *database)
{
   // commit_transaction(database); //commit this transaction
   // start_transaction(database);//start next transaction.

   return RET_SUCCESS;
}

extern int create_db_task(void);

error_t db_module_init(bool hdisk_formated)
{
	error_t errcode;
	
	if((errcode = db_init(hdisk_formated)) != RET_SUCCESS)
	{
		DMCLOG_D("db_init error");
		return errcode;
	}
	
	if(create_db_task() != RET_SUCCESS)
	{
		DMCLOG_D("db task create error");
		return AIRNAS_ERRNO;
	}

	return errcode;
}
error_t db_disk_module_init(struct disk_node *disk_info)
{
	error_t errcode;
	if((errcode = db_disk_init(disk_info->g_database)) != RET_SUCCESS)
	{
		DMCLOG_D("db_init error");
		return errcode;
	}
	if(create_db_disk_task(disk_info) != RET_SUCCESS)
	{
		DMCLOG_D("db task create error");
		return AIRNAS_ERRNO;
	}
	return errcode;
}

void register_db_table_ops(void)
{
	register_user_table_ops();
	register_device_table_ops();
	register_hdisk_table_ops();
    snprintf(DATABASE, 128, "%s/%s", get_sys_dm_db_path(),get_sys_db_name());
	DMCLOG_D("db file path:%s", DATABASE);
}
void register_disk_table_ops(struct disk_node *disk_info)
{
	register_file_table_ops();
	//register_backup_file_table_ops();
	snprintf(disk_info->g_database, 128, "%s/%s", disk_info->path,get_sys_db_name());
	DMCLOG_D("db file path:%s", disk_info->g_database);
}


