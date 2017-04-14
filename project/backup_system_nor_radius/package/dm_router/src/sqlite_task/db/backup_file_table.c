/*
 * =============================================================================
 *
 *       Filename: backup_file_table.c
 *
 *    Description: backup file table operation definition.
 *
 *        Version:  1.0
 *        Created:  2014/10/10 
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Tao Sheng (), vanbreaker90@gmail.com
 *   Organization:  
 *
 * =============================================================================
 */

#include "db/nas_db.h"
#include "db/device_table.h"
#include "db/db_table.h"
#include "db/backup_file_table.h"
#include "db/user_table.h"
#include "tools/str_opr.h"

#if 0

#define BACKUP_FILE_TABLE_NAME "backup_file_table"

db_table_t g_backup_file_table;
uint32_t g_backup_file_id;

const char *g_backup_sort[] = { "ORDER BY NAME ASC",
        				  	    "ORDER BY NAME DESC",
        				   		"ORDER BY TIME ASC",
        				   		"ORDER BY TIME DESC",
        				   		"ORDER BY SIZE ASC",
        				   		"ORDER BY SIZE DESC"};


//allocate new user id.
static uint32_t alloc_backup_file_id(void)
{
    uint32_t id;

	id = g_backup_file_id;
	g_backup_file_id++;
	if(g_backup_file_id < id)
	{
        log_warning("file id overflow!\n");
	}
	
	return g_backup_file_id;
}

extern char *db_path_escape(char *path);


static error_t load_backup_file_insert_cmd(char *sql, backup_file_info_t *pbackup_file)
{
	int n;
	
	if(sql == NULL || pbackup_file == NULL)
	{
		return ENULL_POINT;
	}
	char *path_escape = db_path_escape(pbackup_file->path);
	strcpy(pbackup_file->name,bb_basename(path_escape));
	
	n = snprintf(sql, SQL_CMD_WRITE_BUF_SIZE, "INSERT INTO %s(ID,FILE_UUID,NAME,SIZE,TYPE,TIME,"\
		 "DEVICE_ID,FILE_STATUS,PATH) VALUES(%u,'%s','%s',%lld,%d,%u,'%s',%d,'%s')",
		 BACKUP_FILE_TABLE_NAME, pbackup_file->id, pbackup_file->file_uuid, pbackup_file->name, pbackup_file->size, pbackup_file->file_type,
		 pbackup_file->backup_time,pbackup_file->device_uuid,pbackup_file->file_status,path_escape);
	if(path_escape != NULL)
		free(path_escape);
	if(n >= SQL_CMD_WRITE_BUF_SIZE)
	{
		log_warning("sql buffer size is not enough,the cmd is %s",sql);
		return EOUTBOUND;
	}

	return RET_SUCCESS;
}

static void load_backup_file_info(char **FieldValue, backup_file_info_t *pbackup_file)
{
	ENTER_FUNC();
	if(FieldValue[0] != NULL)
		pbackup_file->id = strtoul(FieldValue[0], NULL, 10);
	if(FieldValue[1] != NULL)
		S_STRNCPY(pbackup_file->file_uuid, FieldValue[1], 64);
	if(FieldValue[2] != NULL)
		S_STRNCPY(pbackup_file->name, FieldValue[2], 512);

	if(FieldValue[3] != NULL)
		pbackup_file->size = strtoull(FieldValue[3], NULL, 10);

	if(FieldValue[4] != NULL)
		pbackup_file->file_type= atoi(FieldValue[4]);

	if(FieldValue[5] != NULL)
		pbackup_file->backup_time = strtoull(FieldValue[5], NULL, 10);
	if(FieldValue[6] != NULL)
		S_STRNCPY(pbackup_file->device_uuid, FieldValue[6], 64);
	if(FieldValue[7] != NULL)
		pbackup_file->file_status= atoi(FieldValue[7]);
	if(FieldValue[8] != NULL)
		S_STRNCPY(pbackup_file->path, FieldValue[8], 2048);
	DMCLOG_D("pbackup_file->path = %s,file_uuid = %s",pbackup_file->path,pbackup_file->file_uuid);
	EXIT_FUNC();
}

static void load_backup_file_uuid(char **FieldValue, unsigned *id)
{
	if(FieldValue[0] != NULL)
		*id = strtoul(FieldValue[0], NULL, 10);
	DMCLOG_D("id = %u",*id);
}


static void load_backup_list_info(char **FieldValue, unsigned long long file_size)
{
	if(FieldValue[3] != NULL)
		file_size = strtoull(FieldValue[3], NULL, 10);
	DMCLOG_D("file_size = %llu",file_size);
}




static int get_backup_file_id_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	uint32_t *pid = (uint32_t *)data;

	if(FieldValue[0] != NULL)
	{
		*pid = strtoul(FieldValue[0], NULL, 10);
	}
	
	return 0;
}





static int get_backup_file_info_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	backup_file_info_t *pbackup_file = (backup_file_info_t *)data;

	load_backup_file_info(FieldValue, pbackup_file);

	return 0;
}


//sqlite exec callback to get user list
static int get_backup_file_list_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	backup_file_list_t *plist = (backup_file_list_t *)data;
	backup_file_info_t *pbackup_file = NULL;

	pbackup_file = (backup_file_info_t *)malloc(sizeof(backup_file_info_t));
	if(pbackup_file == NULL)
	{
		return 1;
	}

	load_backup_file_info(FieldValue, pbackup_file);
	dl_list_add_tail(&plist->head, &pbackup_file->node);
	plist->result_cnt++;
	
	return 0;
}
static int get_backup_list_info_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	backup_file_list_t *plist = (backup_file_list_t *)data;
	uint64_t file_size;
	load_backup_list_info(FieldValue, &file_size);
	plist->result_cnt++;
	plist->result_device_size += file_size;
	return 0;
}

static int get_backup_file_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	backup_file_info_t *pbackup_file = (backup_file_info_t *)data;

	load_backup_file_info(FieldValue, pbackup_file);
	
	return 0;
}
static int get_file_uuid_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	unsigned *pbackup_file = (unsigned *)data;

	load_backup_file_uuid(FieldValue, pbackup_file);
	
	return 0;
}


static void generate_get_bak_list_sql_cmd(char *sql, const backup_file_list_t *plist)
{
	uint8_t is_super_user = 0;
	char user_id_buf[64] = {0};
	char type_buf[100] = {0};
	char tail[50]= {0};

	sprintf(user_id_buf, "DEVICE_ID='%s' AND", plist->device_uuid);

	switch(plist->type)
	{
		case BACKUP_TYPE_ALL:
		{
			int len = strlen(user_id_buf);
			user_id_buf[len - strlen(" AND")] = 0;
			break;
		}
		case BACKUP_TYPE_MESSAGE:
		case BACKUP_TYPE_ADDRESS_LIST:
		case BACKUP_TYPE_CALL_RECORD:
			sprintf(type_buf, "TYPE=%d ", plist->type);
		    break;
		default: break;
	}

	sprintf(tail, "LIMIT %u OFFSET %u", plist->len, plist->start_index);

	sprintf(sql, "SELECT * FROM %s WHERE %s %s %s %s", BACKUP_FILE_TABLE_NAME, 
		user_id_buf, type_buf, g_backup_sort[plist->sort_mode], tail);
}

static void generate_get_bak_list_info_sql_cmd(char *sql, const backup_file_list_t *plist)
{
	uint8_t is_super_user = 0;
	char user_id_buf[64] = {0};
	char type_buf[100] = {0};

	sprintf(user_id_buf, "DEVICE_ID='%s' AND", plist->device_uuid);

	switch(plist->type)
	{
		case BACKUP_TYPE_ALL:
		{
			int len = strlen(user_id_buf);
			user_id_buf[len - strlen(" AND")] = 0;
			break;
		}
		case BACKUP_TYPE_MESSAGE:
		case BACKUP_TYPE_ADDRESS_LIST:
		case BACKUP_TYPE_CALL_RECORD:
			sprintf(type_buf, "TYPE=%d ", plist->type);
		    break;
		default: break;
	}

	sprintf(sql, "SELECT * FROM %s WHERE %s", BACKUP_FILE_TABLE_NAME, 
		user_id_buf);
}



static error_t query_backup_file_list(sqlite3 *database, backup_file_list_t *plist)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];

	if(database == NULL || plist == NULL)
	{
		return ENULL_POINT;
	}

	dl_list_init(&plist->head);
	plist->result_cnt = 0;

	generate_get_bak_list_sql_cmd(sql, plist);

	return sqlite3_exec_busy_wait(database, sql, get_backup_file_list_callback, plist);
}

static error_t query_backup_list_info(sqlite3 *database, backup_file_list_t *plist)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];

	if(database == NULL || plist == NULL)
	{
		return ENULL_POINT;
	}

	dl_list_init(&plist->head);
	plist->result_cnt = 0;
	plist->result_device_size = 0;
	generate_get_bak_list_info_sql_cmd(sql, plist);

	return sqlite3_exec_busy_wait(database, sql, get_backup_list_info_callback, plist);
}


static error_t query_backup_uuid_info(sqlite3 *database, backup_file_list_t *plist)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];

	if(database == NULL || plist == NULL)
	{
		return ENULL_POINT;
	}

	dl_list_init(&plist->head);
	plist->result_cnt = 0;
	plist->result_device_size = 0;
	generate_get_bak_list_info_sql_cmd(sql, plist);

	return sqlite3_exec_busy_wait(database, sql, get_backup_list_info_callback, plist);
}




error_t query_backup_file(sqlite3 *database, backup_file_info_t *backup_file_info)
{
	ENTER_FUNC();
	error_t errcode;
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	if(database == NULL || backup_file_info == NULL)
	{
		return ENULL_POINT;
	}
	//sprintf(sql, "SELECT * FROM %s WHERE FILE_UUID='%s' AND DEVICE_ID='%s'", BACKUP_FILE_TABLE_NAME, backup_file_info->file_uuid,backup_file_info->device_uuid);
	sprintf(sql, "SELECT * FROM %s WHERE FILE_UUID='%s' AND DEVICE_ID='%s'", BACKUP_FILE_TABLE_NAME, backup_file_info->file_uuid,backup_file_info->device_uuid);
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_backup_file_callback, backup_file_info))
			!= RET_SUCCESS)
	{
		EXIT_FUNC();
		return errcode;
	}

	DMCLOG_D("file_id->id = %u",backup_file_info->id);
	if(backup_file_info->id == INVALID_USER_ID)
	{
		EXIT_FUNC();
		return EDB_RECORD_NOT_EXIST;
	}
	EXIT_FUNC();
	return errcode;
}


error_t query_backup_file_by_uuid(sqlite3 *database, char *file_uuid)
{
	error_t errcode = RET_SUCCESS;
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	unsigned file_id = INVALID_USER_ID;
	if(database == NULL || file_uuid == NULL)
	{
		return ENULL_POINT;
	}
	sprintf(sql, "SELECT * FROM %s WHERE FILE_UUID='%s'", BACKUP_FILE_TABLE_NAME, file_uuid);
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_file_uuid_callback, &file_id))
			!= RET_SUCCESS)
	{
		EXIT_FUNC();
		return errcode;
	}
	
	if(file_id == INVALID_USER_ID)
	{
		return EDB_RECORD_NOT_EXIST;
	}

	DMCLOG_D("file_id->id = %u",file_id);
	return errcode;
}


error_t dm_do_delete_by_uuid(sqlite3 *database, char *file_uuid)
{
	if(database == NULL || file_uuid == NULL)
		return ENULL_POINT;
	char *sql = get_db_write_task_sql_buf();
	error_t errcode = RET_SUCCESS;
    if((errcode = query_backup_file_by_uuid(database, file_uuid)) == RET_SUCCESS)
	{
		sprintf(sql, "DELETE FROM %s WHERE FILE_UUID='%s'", BACKUP_FILE_TABLE_NAME, file_uuid);
		return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
	}else{
		if(errcode == EDB_RECORD_NOT_EXIST)
		{
			errcode = RET_SUCCESS;
		}
	}
	return errcode;
}



static error_t backup_file_table_delete(sqlite3 *database, void *target)
{
	backup_file_info_t *del_backup_file = (backup_file_info_t *)target;
	error_t errcode = RET_SUCCESS;
	char *sql = get_db_write_task_sql_buf();
	
	if(database == NULL || target == NULL)
	{
		return ENULL_POINT;
	}
	if((errcode = query_backup_file_by_uuid(database, del_backup_file->file_uuid)) != RET_SUCCESS)
	{
		if(errcode == EDB_RECORD_NOT_EXIST)
			return RET_SUCCESS;
		return errcode;
	}
	sprintf(sql, "DELETE FROM %s WHERE FILE_UUID='%s'", BACKUP_FILE_TABLE_NAME, del_backup_file->file_uuid);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);	
}

static error_t backup_file_table_special_delete(sqlite3 *database, void *target, delete_callback callback)
{
	backup_file_info_t *del_backup_file = (backup_file_info_t *)target;
	error_t errcode = RET_SUCCESS;
	char *del_sql = get_db_write_task_sql_buf();
	file_uuid_t *p_file_uuid = NULL;
	if(database == NULL || target == NULL)
	{
		return ENULL_POINT;
	}
	dl_list_for_each(p_file_uuid, &(del_backup_file->flist->head), file_uuid_t, next)
	{
		if((errcode = query_backup_file_by_uuid(database, p_file_uuid->file_uuid)) != RET_SUCCESS)
		{
			if(errcode == EDB_RECORD_NOT_EXIST)
			{
				DMCLOG_E("the file uuid is not exist");
				continue;
			}else
			{
				DMCLOG_E("sql error");
				break;
			}
		}
		sprintf(del_sql, "DELETE FROM %s WHERE FILE_UUID='%s'", BACKUP_FILE_TABLE_NAME, p_file_uuid->file_uuid);
		errcode = sqlite3_exec_busy_wait(database, del_sql, NULL, NULL);
	}
	
	return errcode;	
}




static error_t backup_file_table_update(sqlite3 *database, void *update_info)
{
	backup_file_update_t *p = (backup_file_update_t *)update_info;
	char *sql = get_db_write_task_sql_buf();

	if(p->cmd & BACKUP_FILE_TABLE_UPDATE_STATE)
	{
		sprintf(sql, "UPDATE %s SET FILE_STATUS=%d WHERE FILE_UUID='%s' AND DEVICE_ID='%s'", BACKUP_FILE_TABLE_NAME, p->backup_info.file_status,
					p->backup_info.file_uuid,p->backup_info.device_uuid);
	}

	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}



extern error_t user_table_query_by_id(sqlite3 *database, uint32_t id, user_info_t *puser);

static int clean_up_bkp_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	#if 0
	clean_up_data_t *p = (clean_up_data_t *)data;
	backup_file_info_t bkp_file;
	char name[BACKUP_NAME_SIZE];
	user_info_t ui;
	error_t errcode;

	load_backup_file_info(FieldValue, &bkp_file);
	
	if((errcode = user_table_query_by_id(p->database, bkp_file.owner, &ui)) != RET_SUCCESS)
	{
		log_warning("get user info error");
		//return 1;
		//skip it by wenhao
		return 0;
	}

	S_STRNCPY(name, bkp_file.name, BACKUP_NAME_SIZE);
	snprintf(bkp_file.name, BACKUP_PATH_SIZE, "%s/%s/backup/%s", ROOT_DIR, ui.user_name, name);
	
	if(p->cb_obj.del_cb_func != NULL)
	{
		p->cb_obj.del_cb_func(&bkp_file, p->cb_obj.del_cb_arg);
	}
#endif
	return 0;
}




static error_t backup_file_table_clean_up(sqlite3 *database, clean_up_data_t *cln_up_data)
{
	char *sql = get_db_write_task_sql_buf();
	error_t errcode;
	
	if(database == NULL || cln_up_data == NULL)
	{
		return ENULL_POINT;
	}

	sprintf(sql, "SELECT * FROM %s WHERE FILE_STATUS=%u", BACKUP_FILE_TABLE_NAME, STATE_NOT_READY);
	if((errcode = sqlite3_exec_busy_wait(database, sql, clean_up_bkp_callback,
			cln_up_data)) != RET_SUCCESS)
	{
		return errcode;
	}

	sprintf(sql, "DELETE FROM %s WHERE FILE_STATUS=%u", BACKUP_FILE_TABLE_NAME, STATE_NOT_READY);
	if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL)) != RET_SUCCESS)
	{
		log_warning("delete invalid records from backup file table error");
	}

	return errcode;
}

static error_t backup_file_table_query(sqlite3 *database, QueryApproach app, void *data)
{
	error_t errcode = RET_SUCCESS;

	if(database == NULL || data == NULL)
	{
		return ENULL_POINT;
	}

	if(app == QUERY_BACKUP_FILE_LIST)
	{
		backup_file_list_t *plist = (backup_file_list_t *)data;
		errcode = query_backup_file_list(database, plist);
	}else if(app == QUERY_BACKUP_FILE)
	{
		backup_file_info_t *backup_file_info = (backup_file_info_t *)data;
		errcode = query_backup_file(database, backup_file_info);
	}else if(app == QUERY_BACKUP_DEVICE_INFO)
	{
		backup_file_list_t *plist = (backup_file_list_t *)data;
		errcode = query_backup_list_info(database, plist);
	}
    return errcode;
    
}



static error_t backup_file_table_insert(sqlite3 *database, void *data)
{
	backup_file_info_t *pbackup_file = (backup_file_info_t *)data;
	char *sql = get_db_write_task_sql_buf();
	error_t errcode = RET_SUCCESS;

	if(database == NULL || data == NULL)
	{
		return ENULL_POINT;
	}
	DMCLOG_D("pbackup_file->file_uuid = %s",pbackup_file->file_uuid);
	if((errcode = query_backup_file_by_uuid(database, pbackup_file->file_uuid)) != RET_SUCCESS)
	{
		if(errcode == EDB_RECORD_NOT_EXIST)
		{
			pbackup_file->id = alloc_backup_file_id();
			if(pbackup_file->id == 0)
			{
				log_warning("backup file id overflow");
				return EDB_ID_OVERFLOW;
			}
			if((errcode = load_backup_file_insert_cmd(sql, pbackup_file)) != RET_SUCCESS)
			{
				return errcode;
			}

			return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
		}
	}
	return errcode;
}



void register_backup_file_table_ops(void)
{
	memset(&g_backup_file_table, 0, sizeof(g_backup_file_table));
	g_backup_file_table.ops.insert = backup_file_table_insert;
	g_backup_file_table.ops.special_delete = backup_file_table_special_delete;
	g_backup_file_table.ops.generic_delete = backup_file_table_delete;
	g_backup_file_table.ops.query = backup_file_table_query;
	g_backup_file_table.ops.active_update = backup_file_table_update;
	g_backup_file_table.ops.clean_up = backup_file_table_clean_up;
}


error_t backup_file_table_init(char *g_database)
{
	sqlite3 *database;
	error_t errcode = RET_SUCCESS;

	register_backup_file_table_ops();

	errcode = sqlite3_open(g_database, &database);
	if(errcode != SQLITE_OK)
	{
		log_warning("cannot open database in backup_file_table_init\n");
		return errcode;
	}

	//get max used user id
	if((errcode = sqlite3_exec_busy_wait(database,"SELECT MAX(ID) FROM backup_file_table",
			 get_backup_file_id_callback, &g_backup_file_id)) != SQLITE_OK)
	{
        sqlite3_close(database);
		return errcode;
	}

	sqlite3_close(database);
	
	return errcode;
}
#endif
