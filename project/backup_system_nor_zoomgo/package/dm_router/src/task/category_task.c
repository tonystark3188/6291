/*
 * =============================================================================
 *
 *       Filename:  scan_task.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2015/9/24 16:45:38
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), wenhaoye@126.com
 *   Organization:  
 *
 * =============================================================================
 */
#include "category_task.h"
#include "router_inotify.h"



//标记磁盘为未挂载状态
int mark_disk_unmount(disk_info_t *disk_info)
{
	disk_info->mount_status = 0;
	return 0;
}
//标记磁盘为挂载状态
int mark_disk_mounted(disk_info_t *disk_info)
{
	disk_info->mount_status = 1;
	return 0;
}



//根据uuid与数据库的磁盘信息进行匹配
int match_disk_info(disk_info_t *disk_info)
{
	disk_info_t pfi_disk_info;
	if(handle_get_disk_info_cmd(disk_info->uuid,&pfi_disk_info) < 0)
	{
		DMCLOG_D("get disk info error");
		return -1;
	}
	DMCLOG_D("disk_info->free_size = %llu",disk_info->free_size);
	DMCLOG_D("pfi_disk_info.free_size = %llu",pfi_disk_info.free_size);
	if(disk_info->total_size == pfi_disk_info.total_size&&disk_info->free_size == pfi_disk_info.free_size)
	{
		DMCLOG_D("match_disk_info succ");
		return 0;
	}
	return -1;
}

//获取当前磁盘信息(磁盘标记文件)
int get_disk_mark_info(char *path)
{
	int res = 0;
	char uuid_path[256];
	memset(uuid_path,0,256);
	sprintf(uuid_path,"%s/%s",path,get_sys_disk_uuid_name());
	res = is_file_exist(uuid_path);
	if(res != 0)
	{
		DMCLOG_D("the uuid file is not exist");
		return -1;
	}
	return 0;
}
error_t drop_table_file(sqlite3 *database)
{
	error_t errcode;
	char sqlite_cmd[SQL_CMD_WRITE_BUF_SIZE];
	#if 1
	memset(sqlite_cmd,0,SQL_CMD_WRITE_BUF_SIZE);
	snprintf(sqlite_cmd, SQL_CMD_WRITE_BUF_SIZE, "DROP TABLE " "%s", FILE_TABLE_NAME);
	if((errcode = sqlite3_exec_busy_wait(database, sqlite_cmd, 
			NULL, NULL)) != SQLITE_OK)
	{
		DMCLOG_D("DROP TABLE  error");
		sqlite3_close(database);
		return errcode;
	}
	#endif
	memset(sqlite_cmd,0,SQL_CMD_WRITE_BUF_SIZE);
	snprintf(sqlite_cmd, SQL_CMD_WRITE_BUF_SIZE, "CREATE TABLE IF NOT EXISTS " "%s""(ID INT PRIMARY KEY NOT NULL,"\
	"NAME CHAR NOT NULL,"\
	"PATH CHAR NOT NULL,"\
	"PARENT_ID INT NOT NULL,"\
	"TYPE INT NOT NULL,"\
	"SIZE INT NOT NULL,"\
	"CREATE_TIME INT NOT NULL,"\
	"MEDIA_INFO_INDEX INT NOT NULL,"\
	"MIME_TYPE CHAR);", FILE_TABLE_NAME);
	DMCLOG_D("sqlite_cmd = %s",sqlite_cmd);
	if((errcode = sqlite3_exec_busy_wait(database, sqlite_cmd, NULL, NULL)) != SQLITE_OK)
	{
		DMCLOG_D("create file_table error");
		return errcode;
	}
	memset(sqlite_cmd,0,SQL_CMD_WRITE_BUF_SIZE);
	snprintf(sqlite_cmd, SQL_CMD_WRITE_BUF_SIZE, "CREATE INDEX IF NOT EXISTS TYPE_INDEX ON %s(TYPE)", FILE_TABLE_NAME);
	if((errcode = sqlite3_exec_busy_wait(database, sqlite_cmd, 
			NULL, NULL)) != SQLITE_OK)
	{
		DMCLOG_D("create type_index error");
		sqlite3_close(database);
		return errcode;
	}
	return errcode;
}

error_t create_table_file(sqlite3 *database)
{
	error_t errcode;
	char sqlite_cmd[SQL_CMD_WRITE_BUF_SIZE];
	memset(sqlite_cmd,0,SQL_CMD_WRITE_BUF_SIZE);
	snprintf(sqlite_cmd, SQL_CMD_WRITE_BUF_SIZE, "CREATE TABLE IF NOT EXISTS " "%s""(ID INT PRIMARY KEY NOT NULL,"\
	"NAME CHAR NOT NULL,"\
	"PATH CHAR NOT NULL,"\
	"PARENT_ID INT NOT NULL,"\
	"TYPE INT NOT NULL,"\
	"SIZE INT NOT NULL,"\
	"CREATE_TIME INT NOT NULL,"\
	"MEDIA_INFO_INDEX INT NOT NULL,"\
	"MIME_TYPE CHAR);", FILE_TABLE_NAME);
	DMCLOG_D("sqlite_cmd = %s",sqlite_cmd);
	if((errcode = sqlite3_exec_busy_wait(database, sqlite_cmd, NULL, NULL)) != SQLITE_OK)
	{
		DMCLOG_D("create file_table error");
		return errcode;
	}
	memset(sqlite_cmd,0,SQL_CMD_WRITE_BUF_SIZE);
	snprintf(sqlite_cmd, SQL_CMD_WRITE_BUF_SIZE, "CREATE INDEX IF NOT EXISTS TYPE_INDEX ON %s(TYPE)", FILE_TABLE_NAME);
	if((errcode = sqlite3_exec_busy_wait(database, sqlite_cmd, 
			NULL, NULL)) != SQLITE_OK)
	{
		DMCLOG_D("create type_index error");
		sqlite3_close(database);
		return errcode;
	}
	#if 0
	memset(sqlite_cmd,0,SQL_CMD_WRITE_BUF_SIZE);
	//snprintf(sqlite_cmd, SQL_CMD_WRITE_BUF_SIZE, "DROP TABLE " "%s", file_table_name);
	if((errcode = sqlite3_exec_busy_wait(database, sqlite_cmd, 
			NULL, NULL)) != SQLITE_OK)
	{
		DMCLOG_D("DROP TABLE  error");
		sqlite3_close(database);
		return errcode;
	}
	#endif
	return errcode;
}

error_t dm_create_db_file(char *name,all_disk_t* mAll_disk_t)
{
	sqlite3 *db;
	error_t errcode;
	int i;
	if(name == NULL || mAll_disk_t == NULL)
	{
		return -1;
	}
	errcode = sqlite3_open_v2(name, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	if(errcode != SQLITE_OK)
	{
		DMCLOG_D("sqlite open db file error,errcode=0x%X",errcode);
		return errcode;
	}
	for(i = 0;i < mAll_disk_t->count;i++)
	{
		errcode = get_disk_mark_info(mAll_disk_t->disk[i].path);
		if(errcode == 0)
		{
			//有标记文件
			DMCLOG_D("uuid file is exist");
			errcode = read_mark_file(mAll_disk_t->disk[i].path,mAll_disk_t->disk[i].uuid);
			if(errcode < 0)
				break;
			//sprintf(mAll_disk_t->disk[i].file_table_name,"airdisk_%s",mAll_disk_t->disk[i].uuid);
			if((errcode = match_disk_info(&mAll_disk_t->disk[i]))!= RET_SUCCESS)//匹配失败
			{
				mAll_disk_t->disk[i].is_scanning = 1;
				#if 0
				if((errcode = handle_update_disk_table(&mAll_disk_t->disk[i])) != RET_SUCCESS);
				{
					DMCLOG_D("handle_update_disk_table error errcode = %d,RET_SUCCESS = %d",errcode,RET_SUCCESS);
			        sqlite3_close(db);
					return -1;
				}
			    #endif
				handle_update_disk_table(&mAll_disk_t->disk[i]);
				DMCLOG_D("handle_update_disk_table succ");
				#if 1
				if((errcode = drop_table_file(db)) != SQLITE_OK)
				{
					DMCLOG_D("drop_table_file error");
			        sqlite3_close(db);
					return errcode;
				}
				#endif
				
			}
		}else{
			//没有标记文件
			disk_info_t disk_pfi;
			DMCLOG_D("uuid file is not exist");
			_dm_gen_uuid(mAll_disk_t->disk[i].uuid,mAll_disk_t->disk[i].pid,mAll_disk_t->disk[i].vid, mAll_disk_t->disk[i].total_size, mAll_disk_t->disk[i].free_size);		
		    //sprintf(mAll_disk_t->disk[i].file_table_name,"airdisk_%s",mAll_disk_t->disk[i].uuid);
			errcode = create_mark_file(mAll_disk_t->disk[i].path,mAll_disk_t->disk[i].uuid);//在磁盘创建uuid文件，标记磁盘的唯一性
			if(errcode < 0)
				break;
			mAll_disk_t->disk[i].is_scanning = 1;
			if((errcode = handle_get_disk_info_cmd(mAll_disk_t->disk[i].uuid,&disk_pfi)) != SQLITE_OK)
			{
				if((errcode = handle_insert_disk_table(&mAll_disk_t->disk[i])) != SQLITE_OK)
				{
					DMCLOG_D("insert_table_driv error");
			        sqlite3_close(db);
					return errcode;
				}
			}
			if((errcode = create_table_file(db)) != SQLITE_OK)
			{
				DMCLOG_D("create_table_file error");
		        sqlite3_close(db);
				return errcode;
			}
		}
		mark_disk_mounted(&mAll_disk_t->disk[i]);//标记盘符为挂载状态
	}
	sqlite3_close(db);
	return errcode;
}
