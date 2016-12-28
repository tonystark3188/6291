/*
 * =============================================================================
 *
 *       Filename: authority_table.c
 *
 *    Description: authority table related data structure definition.
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

db_table_t g_authority_table;

//get authority information from sqlite query result
static void load_authority_info(char **FieldName, char **FieldValue, int nFields, authority_info_t *pauthority)
{
	if(FieldValue[0] != NULL)
    	pauthority->authority_id = strtoul(FieldValue[0], NULL, 10);
	
	if(FieldValue[1] != NULL)
    	pauthority->user_id= strtoul(FieldValue[1], NULL, 10);

	if(FieldValue[2] != NULL)
    	pauthority->bucket_id = strtoul(FieldValue[2], NULL, 10);
	
	if(FieldValue[3] != NULL)
    	pauthority->authority = strtoul(FieldValue[3], NULL, 10);
}


static int get_authority_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	authority_info_t *pauthority = (authority_info_t *)data;

	load_authority_info(FieldName, FieldValue, nFields, pauthority);	

	return 0;
}

//sqlite exec callback to get pauthority id
static int get_authority_id_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	int *pid = (int *)data;

	if(FieldValue[0] != NULL)
	{
		*pid = strtoul(FieldValue[0], NULL, 10);
	}
	return 0;
}

static error_t get_max_authority_id(sqlite3 *database,uint32_t *max_id)
{
	error_t errcode;
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql,"SELECT MAX(ID) FROM %s",AUTHORITY_TABLE_NAME);
	if((errcode = sqlite3_exec_busy_wait(database,sql,get_authority_id_callback, max_id)) != SQLITE_OK)
	{
		DMCLOG_D("sqlite3_exec_busy_wait error");
        sqlite3_close(database);
        return errcode;
	}
	return errcode;
}


error_t authority_exist(sqlite3 *database,int user_id,int bucket_id,int *g_id)
{
    char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;
   	*g_id = INVALID_AUTHORITY_ID;
	if(database == NULL)
	{
		return EINVAL_ARG;
	}
	sprintf(sql, "SELECT * FROM %s where USER_ID = '%d' AND BUCKET_ID = '%d'",
		AUTHORITY_TABLE_NAME, user_id,bucket_id);
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_authority_id_callback, g_id))
			!= RET_SUCCESS)
	{
        DMCLOG_E("exit error");
		return errcode;
	}
    if(*g_id == INVALID_AUTHORITY_ID)
    {
        DMCLOG_D("EDB_RECORD_NOT_EXIST");
		return EDB_RECORD_NOT_EXIST;
	}
    return errcode;	
}

static int get_authority_list_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	authority_list_t *plist = (authority_list_t *)data;
	authority_info_t *pfi = NULL;

	pfi = (authority_info_t *)calloc(1,sizeof(authority_info_t));
	assert(pfi != NULL);
	load_authority_info(FieldName, FieldValue, nFields, pfi);
	dl_list_add_tail(&plist->head, &pfi->next);
	plist->len++;
	return 0;
}

static error_t load_authority_insert_cmd(sqlite3 *database,authority_info_t *pauthority)
{
	ENTER_FUNC();
	int n;
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	if(sql == NULL || pauthority == NULL)
	{
		return ENULL_POINT;
	}

	n = snprintf(sql, SQL_CMD_WRITE_BUF_SIZE, "INSERT INTO %s(USER_ID,BUCKET_ID,AUTHORITY)"\
		"VALUES('%d','%d','%d')",AUTHORITY_TABLE_NAME,
		pauthority->user_id,pauthority->bucket_id,pauthority->authority);

	if(n >= SQL_CMD_WRITE_BUF_SIZE)
	{
		return EOUTBOUND;
	}
	EXIT_FUNC();
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

static error_t load_authority_delete_cmd(sqlite3 *database,authority_info_t *pauthority)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "DELETE FROM %s WHERE ID ='%d'", AUTHORITY_TABLE_NAME, pauthority->authority_id);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

static error_t load_authority_update_cmd(sqlite3 *database,authority_info_t *pauthority)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "UPDATE %s SET AUTHORITY ='%s' WHERE ID='%d'", AUTHORITY_TABLE_NAME, pauthority->authority_id,pauthority->authority);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

static error_t load_authority_list_cmd(sqlite3 *database, authority_list_t *plist)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	sprintf(sql, "SELECT * FROM %s", AUTHORITY_TABLE_NAME);
	return sqlite3_exec_busy_wait(database, sql, get_authority_list_callback, plist);
}

error_t load_authority_cmd(sqlite3 *database, authority_info_t *pauthority)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;
	
	sprintf(sql,"SELECT * FROM %s WHERE ID=%d",AUTHORITY_TABLE_NAME, pauthority->authority_id);

	if((errcode = sqlite3_exec_busy_wait(database, sql, get_authority_callback, pauthority))
			!= RET_SUCCESS)
	{
		return errcode;
	}
	return errcode;
}


/**
 * 增加一条权限
 */
error_t authority_table_insert(sqlite3 *database,void *data)
{
	authority_insert_t *authority_insert = (authority_insert_t *)data;
	error_t errcode = RET_SUCCESS;
	int g_id = INVALID_AUTHORITY_ID;
	if(database == NULL || data == NULL)
	{
		return ENULL_POINT;
	}

	if(authority_insert->cmd == AUTHORITY_TABLE_INSERT_INFO)
	{
		authority_info_t *pauthority = &authority_insert->authority_info;
		errcode = authority_exist(database, pauthority->user_id,pauthority->bucket_id,&g_id);
		if(errcode == EDB_RECORD_NOT_EXIST)//authority not exist, go ahead
		{
			errcode = load_authority_insert_cmd(database,pauthority);
			if(errcode != RET_SUCCESS)
			{
				DMCLOG_E("insert authority id error");
				return errcode;
			}

			errcode = get_max_authority_id(database,&pauthority->authority_id);
			if(errcode != RET_SUCCESS)
			{
				DMCLOG_E("get max authority id error");
				return errcode;
			}
		}
	}
	
	return errcode;
}

/**
 * 删除一条权限
 */
error_t authority_table_delete(sqlite3 *database,void *data)
{
	int g_id = INVALID_AUTHORITY_ID;
	error_t errcode = RET_SUCCESS;
	if(database == NULL || data == NULL)
	{
		return EINVAL_ARG;
	}
	authority_info_t *pauthority = (authority_info_t *)data;
	errcode = authority_exist(database, pauthority->user_id,pauthority->bucket_id,&pauthority->authority_id);
	if(errcode == EDB_RECORD_NOT_EXIST)//authority not exist, go ahead
	{
		return load_authority_delete_cmd(database,pauthority);
	}
	return errcode;
}

/**
 * 修改一条权限
 */
error_t authority_table_update(sqlite3 *database,void *data)
{
	error_t errcode = RET_SUCCESS;
	if(database == NULL || data == NULL)
	{
		return EINVAL_ARG;
	}
	authority_update_t *pauthority_update = (authority_update_t *)data;
	if(pauthority_update->cmd == AUTHORITY_TABLE_UPDATE_AUTH)
	{
		return load_authority_update_cmd(database,&pauthority_update->authority_info);
	}
	return errcode;
}

/**
 * 查询一条权限或者权限列表
 */
error_t authority_table_query(sqlite3 *database,void *data)
{
	error_t errcode = SQLITE_OK;
	if(database == NULL || data == NULL)
	{
		return EINVAL_ARG;
	}
	authority_query_t *pauthority_query = (authority_query_t *)data;
	if(pauthority_query->cmd == AUTHORITY_TABLE_QUERY_LIST)
	{
		return load_authority_list_cmd(database, &pauthority_query->authority_list);
	}else if(pauthority_query->cmd == AUTHORITY_TABLE_QUERY_INFO)
	{
		authority_info_t *pauthority = &pauthority_query->authority_info;
		errcode = authority_exist(database, pauthority->user_id,pauthority->bucket_id,&pauthority->authority_id);
		if(errcode == EDB_RECORD_NOT_EXIST)//authority not exist, go ahead
		{
			DMCLOG_E("authority is not exist");
			return errcode;
		}
		return load_authority_cmd(database,pauthority);
	}
	return ESHELL_CMD;
}

void register_authority_table_ops(void)
{
	memset(&g_authority_table, 0, sizeof(g_authority_table));
	
	g_authority_table.ops.insert          	= authority_table_insert;
	g_authority_table.ops.update   		 	= authority_table_update;
	g_authority_table.ops.delete     	 	= authority_table_delete;
	g_authority_table.ops.query           	= authority_table_query;
}

