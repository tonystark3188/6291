/*
 * =============================================================================
 *
 *       Filename:  sqlite_db.c
 *
 *    Description:  create database file and create database table
 *
 *        Version:  1.0
 *        Created:  2016/10/19
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver <henry.jin@dmsys.com>
 *   Organization:  longsys
 *
 * =============================================================================
 */
#include "sqlite_db.h"

static error_t create_audio_table(sqlite3 *database)
{
	return sqlite3_exec_busy_wait(database, SQLITE_CREATE_AUDIO_TABLE, NULL, NULL);
}

static error_t create_authority_table(sqlite3 *database)
{
	return sqlite3_exec_busy_wait(database, SQLITE_CREATE_AUTHORITY_TABLE, NULL, NULL);
}

static error_t create_bucket_table(sqlite3 *database)
{
	return sqlite3_exec_busy_wait(database, SQLITE_CREATE_BUCKET_TABLE, NULL, NULL);
}

static error_t create_disk_table(sqlite3 *database)
{
	return sqlite3_exec_busy_wait(database, SQLITE_CREATE_DISK_TABLE, NULL, NULL);
}

static error_t create_file_table(sqlite3 *database)
{
	
	return sqlite3_exec_busy_wait(database, SQLITE_CREATE_FILE_TABLE, NULL, NULL);
}

static error_t create_thumbnail_table(sqlite3 *database)
{
	return sqlite3_exec_busy_wait(database, SQLITE_CREATE_THUMBNAIL_TABLE, NULL, NULL);
}


static error_t create_image_table(sqlite3 *database)
{
	return sqlite3_exec_busy_wait(database, SQLITE_CREATE_IMAGE_TABLE, NULL, NULL);
}

static error_t create_user_table(sqlite3 *database)
{
	return sqlite3_exec_busy_wait(database, SQLITE_CREATE_USER_TABLE, NULL, NULL);
}

static error_t create_video_table(sqlite3 *database)
{
	return sqlite3_exec_busy_wait(database, SQLITE_CREATE_VIDEO_TABLE, NULL, NULL);
}

static error_t create_version_table(sqlite3 *database)
{
	return sqlite3_exec_busy_wait(database, SQLITE_CREATE_VERSION_TABLE, NULL, NULL);
}



static error_t create_all_tables(sqlite3 *database)
{
	error_t errcode = SQLITE_OK;
	if(create_audio_table(database) != SQLITE_OK
		||create_authority_table(database) != SQLITE_OK
		||create_bucket_table(database) != SQLITE_OK
		||create_disk_table(database) != SQLITE_OK
		||create_file_table(database) != SQLITE_OK
		||create_thumbnail_table(database) != SQLITE_OK
		||create_image_table(database) != SQLITE_OK
		||create_user_table(database) != SQLITE_OK
		||create_video_table(database) != SQLITE_OK
		||create_version_table(database) != SQLITE_OK)
	{
		DMCLOG_E("create tables error");
		return errcode;
	}
	return errcode;
}

error_t create_db_file(char *name)
{
	sqlite3 *db;
	error_t errcode;
	
	errcode = sqlite3_open_v2(name, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	if(errcode != SQLITE_OK)
	{
		DMCLOG_E("sqlite open db file error,errcode=%d",errcode);
		return errcode;
	}
	
	if((errcode = create_all_tables(db)) != SQLITE_OK)
	{
		DMCLOG_E("sqlite create db table error,errcode=%d",errcode);
        sqlite3_close(db);
		return errcode;
	}
	sqlite3_close(db);
	return errcode;
}

static error_t create_v_file_table(sqlite3 *database,char *bucket_name)
{
	char sql[1024] = {0};
	sprintf(sql,"CREATE TABLE IF NOT EXISTS %s ( \
    FILE_ID      INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,\
    FILE_NAME    CHAR NOT NULL,\
    FILE_PATH	 CHAR,\
    REAL_FILE_ID INT  NOT NULL,\
    PARENT_ID    INT  NOT NULL,\
    IS_DIR       INT  NOT NULL,\
    TYPE       	INT  NOT NULL,\
    CREATE_TIME INT  NOT NULL,\
    MODIFY_TIME INT  NOT NULL,\
    ACCESS_TIME INT  NOT NULL,\
    SIZE 		INT  NOT NULL,\
    GB_NAME    	CHAR NOT NULL,\
    UUID    	CHAR NOT NULL);",bucket_name);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}


error_t create_db_v_file(char *name,char *bucket_name)
{
	sqlite3 *db;
	error_t errcode;
	
	errcode = sqlite3_open_v2(name, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	if(errcode != SQLITE_OK)
	{
		DMCLOG_E("sqlite open db file error,errcode=%d",errcode);
		return errcode;
	}
	
	if((errcode = create_v_file_table(db,bucket_name)) != SQLITE_OK)
	{
		DMCLOG_E("sqlite create db table error,errcode=%d",errcode);
        sqlite3_close(db);
		return errcode;
	}
	sqlite3_close(db);
	return errcode;
}


static error_t db_init()
{
	error_t errcode = RET_SUCCESS;

	if((errcode = create_db_file(DATABASE)) != RET_SUCCESS)
	{
		DMCLOG_E("create db file error");
		return errcode;
	}
	if((errcode = create_db_file(BACKUP_DB)) != RET_SUCCESS)
	{
		DMCLOG_E("create db backup file error");
		return errcode;
	}
	return errcode;
}

error_t v_file_table_create(char *buncket_name)
{
	error_t errcode = RET_SUCCESS;

	if((errcode = create_db_v_file(DATABASE,buncket_name)) != RET_SUCCESS)
	{
		DMCLOG_E("create db v file error");
		return errcode;
	}
	return errcode;
}

error_t db_module_init()
{
	error_t errcode = RET_SUCCESS;
	
	if((errcode = db_init()) != RET_SUCCESS)
	{
		DMCLOG_D("db_init error");
		return AIRNAS_ERRNO;
	}
	
	if(create_db_task() != RET_SUCCESS)
	{
		DMCLOG_D("db task create error");
		return AIRNAS_ERRNO;
	}
	
	char **bucket_list = (char **)handle_bucket_table_list_query();
	if(bucket_list == NULL)
	{
		errcode = v_file_table_create(PUBLIC_BUCKET_NAME);
		if(errcode != 0)
		{
			DMCLOG_E("public bucket table create error");
			return AIRNAS_ERRNO;
		}
		
		v_file_insert_t v_file_insert;
		memset(&v_file_insert,0,sizeof(v_file_insert_t));
		v_file_insert.cmd = V_FILE_TABLE_INSERT_INFO;
		v_file_insert.v_file_info.isDir = 1;
		S_STRNCPY(v_file_insert.bucket_name,PUBLIC_BUCKET_NAME,MAX_BUCKET_NAME_LEN);
		sprintf(v_file_insert.v_file_info.path,"/%s",PUBLIC_BUCKET_NAME);
		errcode = _handle_v_file_table_insert(&v_file_insert);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("mkdir root file table error");
			return AIRNAS_ERRNO;
		}
		return RET_SUCCESS;
	}
		
	int i = 0;
	for(i = 0;bucket_list[i];i++)
	{
		errcode = v_file_table_create(bucket_list[i]);
	}

	errcode = v_file_table_create(PUBLIC_BUCKET_NAME);
	if(errcode != 0)
	{
		DMCLOG_E("public bucket table create error");
		return AIRNAS_ERRNO;
	}

	for(i = 0;bucket_list[i];i++)
	{
		safe_free(bucket_list[i]);
	}
	safe_free(bucket_list);
	
	return errcode;
}

void register_db_table_ops(void)
{
	register_user_table_ops();
	register_authority_table_ops();
	register_bucket_table_ops();
	register_v_file_table_ops();
	register_disk_table_ops();
    snprintf(DATABASE, 128, "%s/%s", get_sys_dm_db_path(),get_sys_db_name());
	snprintf(BACKUP_DB, 128, "%s/nas_backup.db", get_sys_dm_db_path());
	DMCLOG_D("db backup file path:%s", BACKUP_DB);
	DMCLOG_D("db file path:%s", DATABASE);
}




