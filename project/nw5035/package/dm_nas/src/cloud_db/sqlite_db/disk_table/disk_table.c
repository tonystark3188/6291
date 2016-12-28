/*
 * =============================================================================
 *
 *       Filename: disk_table.c
 *
 *    Description: disk table related data structure definition.
 *
 *        Version:  1.0
 *        Created:  2016/10/20
 *       Revision:  none
 *       Compiler:  gcc
 *
 *          Author:  Oliver <henry.jin@dmsys.com>
 *   Organization:  longsys
 *
 * =============================================================================
 */
 #include "db_table.h"
 db_table_t g_disk_table;

//get disk information from sqlite query result
static void load_disk_info(char **FieldName, char **FieldValue, int nFields,disk_info_t *pdisk)
{
	if(FieldValue[0] != NULL)
    	pdisk->disk_id = strtoul(FieldValue[0], NULL, 10);
	if(FieldValue[1] != NULL)
		S_STRNCPY(pdisk->uuid, FieldValue[1], MAX_UUID_LEN);
	if(FieldValue[2] != NULL)
		S_STRNCPY(pdisk->name, FieldValue[2], MAX_DISK_NAME_LEN);
	if(FieldValue[3] != NULL)
		pdisk->total_size = strtoul(FieldValue[3], NULL, 10);
	if(FieldValue[4] != NULL)
		pdisk->free_size= strtoul(FieldValue[4], NULL, 10);

}


static int get_disk_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	disk_info_t *pdisk = (disk_info_t *)data;

	load_disk_info(FieldName, FieldValue, nFields, pdisk);	

	return 0;
}

//sqlite exec callback to get disk id
static int get_disk_id_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	int *pid = (int *)data;

	if(FieldValue[0] != NULL)
	{
		*pid = strtoul(FieldValue[0], NULL, 10);
	}
	return 0;
}

error_t disk_exist(sqlite3 *database,char *uuid,int *g_id)
{
    char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;
   	*g_id = INVALID_DISK_ID;
	if(database == NULL)
	{
		return EINVAL_ARG;
	}

	sprintf(sql, "SELECT * FROM %s where UUID = '%s'",
		DISK_TABLE_NAME, uuid);
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_disk_id_callback, g_id))
			!= RET_SUCCESS)
	{
        DMCLOG_E("exit error");
		return errcode;
	}
    if(*g_id == INVALID_DISK_ID)
    {
        DMCLOG_D("EDB_RECORD_NOT_EXIST");
		return EDB_RECORD_NOT_EXIST;
	}
    return errcode;	
}

static int get_disk_list_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	disk_list_t *plist = (disk_list_t *)data;
	disk_info_t *pfi = NULL;

	pfi = (disk_info_t *)calloc(1,sizeof(disk_info_t));
	assert(pfi != NULL);
	load_disk_info(FieldName, FieldValue, nFields, pfi);
	dl_list_add_tail(&plist->head, &pfi->next);
	plist->len++;
	return 0;
}

static error_t load_disk_insert_cmd(sqlite3 *database,disk_info_t *pdisk)
{
	int n;
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	if(sql == NULL || pdisk == NULL || !*pdisk->name)
	{
		return ENULL_POINT;
	}
	
	n = snprintf(sql, SQL_CMD_WRITE_BUF_SIZE, "INSERT INTO %s(NAME,UUID,CAPACITY,FREE_CAPACITY)"\
		"VALUES('%d','%s','%llu','%llu')",DISK_TABLE_NAME,
		 pdisk->name, pdisk->uuid, pdisk->total_size,pdisk->free_size);

	if(n >= SQL_CMD_WRITE_BUF_SIZE)
	{
		return EOUTBOUND;
	}
	
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

static error_t load_disk_delete_cmd(sqlite3 *database,disk_info_t *pdisk)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "DELETE FROM %s WHERE DISK_ID ='%d'", DISK_TABLE_NAME, pdisk->disk_id);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

static error_t load_disk_update_capacity_cmd(sqlite3 *database,disk_info_t *pdisk)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "UPDATE %s SET CAPACITY ='%lld' and FREE_CAPACITY='%lld' WHERE DISK_ID='%d'", DISK_TABLE_NAME, pdisk->total_size,pdisk->free_size,pdisk->disk_id);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

static error_t load_disk_list_cmd(sqlite3 *database, disk_list_t *plist)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	sprintf(sql, "SELECT * FROM %s", DISK_TABLE_NAME);
	return sqlite3_exec_busy_wait(database, sql, get_disk_list_callback, plist);
}

error_t load_disk_cmd(sqlite3 *database, disk_info_t *pdisk)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;
	
	pdisk->disk_id = INVALID_DISK_ID;

	sprintf(sql,"SELECT * FROM %s WHERE ID=%d",DISK_TABLE_NAME, pdisk->disk_id);

	if((errcode = sqlite3_exec_busy_wait(database, sql, get_disk_callback, pdisk))
			!= RET_SUCCESS)
	{
		return errcode;
	}

	if(pdisk->disk_id == INVALID_DISK_ID)
	{
		return EDB_RECORD_NOT_EXIST;
	}
	return errcode;
}


/**
 * add disk item
 */
error_t disk_table_insert(sqlite3 *database,void *data)
{
	disk_info_t *puser = (disk_info_t *)data;
	error_t errcode = RET_SUCCESS;
	int g_id = INVALID_DISK_ID;
	if(database == NULL || puser == NULL)
	{
		return ENULL_POINT;
	}
	
	errcode = disk_exist(database, puser->uuid,&g_id);
	if(errcode == EDB_RECORD_NOT_EXIST)//user not exist, go ahead
	{
		return load_disk_insert_cmd(database,puser);
	}
	return errcode;
}

/**
 * delete disk item
 */
error_t disk_table_delete(sqlite3 *database,void *data)
{
	int g_id = INVALID_DISK_ID;
	error_t errcode = RET_SUCCESS;
	if(database == NULL || data == NULL)
	{
		return EINVAL_ARG;
	}
	disk_info_t *pdisk = (disk_info_t *)data;
	errcode = disk_exist(database, pdisk->uuid,&pdisk->disk_id);
	if(errcode == EDB_RECORD_NOT_EXIST)//user not exist, go ahead
	{
		return load_disk_delete_cmd(database,pdisk);
	}
	return errcode;
}

/**
 * modify capacity
 */
error_t disk_table_update(sqlite3 *database,void *data)
{
	error_t errcode = RET_SUCCESS;
	if(database == NULL || data == NULL)
	{
		return EINVAL_ARG;
	}
	disk_update_t *pdisk_update = (disk_update_t *)data;
	if(pdisk_update->cmd == DISK_TABLE_UPDATE_CAPACITY)
	{
		return load_disk_update_capacity_cmd(database,&pdisk_update->disk_info);
	}
	return errcode;
}

/**
 * get disk list or get disk info
 */
error_t disk_table_query(sqlite3 *database,void *data)
{
	error_t errcode = SQLITE_OK;
	if(database == NULL || data == NULL)
	{
		return EINVAL_ARG;
	}
	disk_query_t *pdisk_query = (disk_query_t *)data;
	if(pdisk_query->cmd == DISK_TABLE_QUERY_LIST)
	{
		return load_disk_list_cmd(database, &pdisk_query->disk_list);
	}else if(pdisk_query->cmd == DISK_TABLE_QUERY_INFO)
	{
		disk_info_t *pdisk = &pdisk_query->disk_info;
		errcode = disk_exist(database, pdisk->uuid,&pdisk->disk_id);
		if(errcode == EDB_RECORD_NOT_EXIST)//authority not exist, go ahead
		{
			DMCLOG_E("disk is not exist");
			return errcode;
		}
		return load_disk_cmd(database,pdisk);
	}
	return ESHELL_CMD;
}

void register_disk_table_ops(void)
{
	memset(&g_disk_table, 0, sizeof(g_disk_table));
	
	g_disk_table.ops.insert          = disk_table_insert;
	g_disk_table.ops.update   		 = disk_table_update;
	g_disk_table.ops.delete     	 = disk_table_delete;
	g_disk_table.ops.query           = disk_table_query;
}

