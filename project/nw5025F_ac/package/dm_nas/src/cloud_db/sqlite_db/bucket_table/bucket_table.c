/*
 * =============================================================================
 *
 *       Filename: bucket_table.c
 *
 *    Description: bucket table related data structure definition.
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
 db_table_t g_bucket_table;

//get bucket information from sqlite query result
static void load_bucket_info(char **FieldName, char **FieldValue, int nFields,bucket_info_t *pbucket)
{
	if(FieldValue[0] != NULL)
    	pbucket->bucket_id = strtoul(FieldValue[0], NULL, 10);
	
	if(FieldValue[1] != NULL)
		S_STRNCPY(pbucket->bucket_table_name, FieldValue[1], MAX_BUCKET_NAME_LEN);
	DMCLOG_D("pbucket->bucket_table_name = %s",pbucket->bucket_table_name);

	if(FieldValue[2] != NULL)
    	pbucket->create_user_id= strtoul(FieldValue[0], NULL, 10);
}


static int get_bucket_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	bucket_info_t *pbucket = (bucket_info_t *)data;

	load_bucket_info(FieldName, FieldValue, nFields, pbucket);	

	return 0;
}

//sqlite exec callback to get user id
static int get_bucket_id_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	int *pid = (int *)data;

	if(FieldValue[0] != NULL)
	{
		*pid = strtoul(FieldValue[0], NULL, 10);
	}
	return 0;
}

static error_t get_max_bucket_id(sqlite3 *database,uint32_t *max_id)
{
	error_t errcode;
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql,"SELECT MAX(BUCKET_ID) FROM %s",BUCKET_TABLE_NAME);
	if((errcode = sqlite3_exec_busy_wait(database,sql,get_bucket_id_callback, max_id)) != SQLITE_OK)
	{
		DMCLOG_D("sqlite3_exec_busy_wait error");
        sqlite3_close(database);
        return errcode;
	}
	return errcode;
}


error_t bucket_name_exist(sqlite3 *database,char *bucket_name,int *g_id)
{
    char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;
    char name_str_escape[64];
   	*g_id = INVALID_BUCKET_ID;
	if(database == NULL)
	{
		return EINVAL_ARG;
	}
	sqlite3_str_escape(bucket_name, name_str_escape, sizeof(name_str_escape));
	sprintf(sql, "SELECT * FROM %s where BUCKET_TABLE_NAME = '%s'",
		BUCKET_TABLE_NAME, name_str_escape);
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_bucket_id_callback, g_id))
			!= RET_SUCCESS)
	{
        DMCLOG_E("exit error");
		return errcode;
	}
    if(*g_id == INVALID_BUCKET_ID)
    {
        DMCLOG_D("EDB_RECORD_NOT_EXIST");
		return EDB_RECORD_NOT_EXIST;
	}
    return errcode;	
}

static int get_bucket_list_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	bucket_list_t *plist = (bucket_list_t *)data;
	bucket_info_t *pfi = NULL;

	pfi = (bucket_info_t *)calloc(1,sizeof(bucket_info_t));
	assert(pfi != NULL);
	load_bucket_info(FieldName, FieldValue, nFields, pfi);
	dl_list_add_tail(&plist->head, &pfi->next);
	plist->len++;
	return 0;
}

static error_t load_bucket_insert_cmd(sqlite3 *database,bucket_info_t *pbucket)
{
	ENTER_FUNC();
	int n;
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	char name_str_escape[64];
	if(sql == NULL || pbucket == NULL || !*pbucket->bucket_table_name)
	{
		return ENULL_POINT;
	}
	sqlite3_str_escape(pbucket->bucket_table_name, name_str_escape, sizeof(name_str_escape));

	n = snprintf(sql, SQL_CMD_WRITE_BUF_SIZE, "INSERT INTO %s(BUCKET_TABLE_NAME,CREATE_USER_ID)"\
		"VALUES('%s','%d')",BUCKET_TABLE_NAME,
		name_str_escape, pbucket->create_user_id);

	if(n >= SQL_CMD_WRITE_BUF_SIZE)
	{
		return EOUTBOUND;
	}
	EXIT_FUNC();
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

static error_t load_bucket_delete_cmd(sqlite3 *database,bucket_info_t *pbucket)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "DELETE FROM %s WHERE BUCKET_ID ='%d'", BUCKET_TABLE_NAME, pbucket->bucket_id);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

static error_t load_bucket_update_bucket_name_cmd(sqlite3 *database,bucket_info_t *pbucket)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "UPDATE %s SET BUCKET_TABLE_NAME ='%s' WHERE BUCKET_ID='%d'", BUCKET_TABLE_NAME, pbucket->bucket_table_name,pbucket->bucket_id);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

static error_t load_bucket_update_user_id_cmd(sqlite3 *database,bucket_info_t *pbucket)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "UPDATE %s SET CREATE_USER_ID ='%d' WHERE BUCKET_ID='%d'", BUCKET_TABLE_NAME, pbucket->create_user_id,pbucket->bucket_id);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

static error_t load_bucket_list_cmd(sqlite3 *database, bucket_list_t *plist)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	sprintf(sql, "SELECT * FROM %s", BUCKET_TABLE_NAME);
	return sqlite3_exec_busy_wait(database, sql, get_bucket_list_callback, plist);
}

static error_t load_bucket_info_cmd(sqlite3 *database, bucket_info_t *pinfo)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	sprintf(sql, "SELECT * FROM %s WHERE CREATE_USER_ID=%d", BUCKET_TABLE_NAME,pinfo->create_user_id);
	return sqlite3_exec_busy_wait(database, sql, get_bucket_callback, pinfo);
}


/**
 * 新增一个bucket信息
 */
error_t bucket_table_insert(sqlite3 *database,void *data)
{
	bucket_insert_t *bucket_insert = (bucket_insert_t *)data;
	
	error_t errcode = RET_SUCCESS;
	int g_id = INVALID_BUCKET_ID;
	if(database == NULL || data == NULL)
	{
		return ENULL_POINT;
	}

	if(bucket_insert->cmd == BUCKET_TABLE_INSERT_INFO)
	{
		bucket_info_t *pbucket = &bucket_insert->bucket_info;
		errcode = bucket_name_exist(database, pbucket->bucket_table_name,&g_id);
		if(errcode == EDB_RECORD_NOT_EXIST)//bucket is not exist, go ahead
		{
			errcode = load_bucket_insert_cmd(database,pbucket);
			if(errcode != RET_SUCCESS)
			{
				DMCLOG_E("bucket insert error");
				return errcode;
			}

			errcode = get_max_bucket_id(database,&pbucket->bucket_id);
			if(errcode != RET_SUCCESS)
			{
				DMCLOG_E("get max bucket id error");
				return errcode;
			}
		}
	}
	return errcode;
}

/**
 * 删除一个bucket
 */
error_t bucket_table_delete(sqlite3 *database,void *data)
{
	int g_id = INVALID_BUCKET_ID;
	error_t errcode = RET_SUCCESS;
	if(database == NULL || data == NULL)
	{
		return EINVAL_ARG;
	}
	bucket_info_t *pbucket = (bucket_info_t *)data;
	errcode = bucket_name_exist(database, pbucket->bucket_table_name,&pbucket->bucket_id);
	if(errcode == EDB_RECORD_NOT_EXIST)//user not exist, go ahead
	{
		return load_bucket_delete_cmd(database,pbucket);
	}
	return errcode;
}


/**
 * update bucket table name or user id
 */

error_t bucket_table_update(sqlite3 *database,void *data)
{
	error_t errcode = RET_SUCCESS;
	if(database == NULL || data == NULL)
	{
		return EINVAL_ARG;
	}
	bucket_update_t *pbucket_update = (bucket_update_t *)data;
	if(pbucket_update->cmd == BUCKET_TABLE_UPDATE_BUCKET_NAME)
	{
		return load_bucket_update_bucket_name_cmd(database,&pbucket_update->bucket_info);
	}else if(pbucket_update->cmd == BUCKET_TABLE_UPDATE_CREATE_USER_ID)
	{
		return load_bucket_update_user_id_cmd(database,&pbucket_update->bucket_info);
	}
	return errcode;
}

/**
 * 获取bucket列表或者指定bucket信息
 */
error_t bucket_table_query(sqlite3 *database,void *data)
{
	if(database == NULL || data == NULL)
	{
		return EINVAL_ARG;
	}
	bucket_query_t *pbucket_query = (bucket_query_t *)data;
	if(pbucket_query->cmd == BUCKET_TABLE_QUERY_LIST)
	{
		return load_bucket_list_cmd(database, &pbucket_query->bucket_list);
	}else if(pbucket_query->cmd == BUCKET_TABLE_QUERY_INFO)
	{
		return load_bucket_info_cmd(database, &pbucket_query->bucket_info);
	}
	return ESHELL_CMD;
}

void register_bucket_table_ops(void)
{
	memset(&g_bucket_table, 0, sizeof(g_bucket_table));
	
	g_bucket_table.ops.insert          		= bucket_table_insert;
	g_bucket_table.ops.update   		 	= bucket_table_update;
	g_bucket_table.ops.delete     	 		= bucket_table_delete;
	g_bucket_table.ops.query           		= bucket_table_query;
}


