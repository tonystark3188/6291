/*
 * =============================================================================
 *
 *       Filename: user_table.c
 *
 *    Description: user table related data structure definition.
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
db_table_t g_user_table;

//get user information from sqlite query result
static void load_user_info(char **FieldName, char **FieldValue, int nFields,user_info_t *puser)
{
	if(FieldValue[0] != NULL)
    	puser->user_id = strtoul(FieldValue[0], NULL, 10);
	
	if(FieldValue[1] != NULL)
		S_STRNCPY(puser->user_name, FieldValue[1], MAX_USER_NAME_LEN);
	DMCLOG_D("puser->user_name = %s",puser->user_name);

	if(FieldValue[2] != NULL)
		S_STRNCPY(puser->password, FieldValue[2], MAX_PASSWORD_LEN);
	DMCLOG_D("puser->password = %s",puser->password);

	if(FieldValue[3] != NULL)
		S_STRNCPY(puser->nick_name, FieldValue[3], MAX_USER_NAME_LEN);
	DMCLOG_D("puser->nick_name = %s",puser->nick_name);

}


static int get_user_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	user_info_t *puser = (user_info_t *)data;

	load_user_info(FieldName, FieldValue, nFields,puser);	

	return 0;
}

//sqlite exec callback to get user id
static int get_user_id_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	int *pid = (int *)data;

	if(FieldValue[0] != NULL)
	{
		*pid = strtoul(FieldValue[0], NULL, 10);
	}
	return 0;
}

static error_t get_max_user_id(sqlite3 *database,uint32_t *max_id)
{
	error_t errcode;
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql,"SELECT MAX(USER_ID) FROM %s",USER_TABLE_NAME);
	if((errcode = sqlite3_exec_busy_wait(database,sql,get_user_id_callback, max_id)) != SQLITE_OK)
	{
		DMCLOG_D("sqlite3_exec_busy_wait error");
        sqlite3_close(database);
        return errcode;
	}
	return errcode;
}


error_t user_name_exist(sqlite3 *database,char *user_name,int *g_id)
{
	ENTER_FUNC();
    char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;
    char name_str_escape[MAX_USER_NAME_LEN+ 0x10];
   	*g_id = INVALID_USER_ID;
	if(database == NULL)
	{
		return EINVAL_ARG;
	}
	sqlite3_str_escape(user_name, name_str_escape, sizeof(name_str_escape));
	sprintf(sql, "SELECT * FROM %s where USER_NAME = '%s'",
		USER_TABLE_NAME, name_str_escape);
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_user_id_callback, g_id))
			!= RET_SUCCESS)
	{
        DMCLOG_E("exit error");
		return errcode;
	}
    if(*g_id == INVALID_USER_ID)
    {
        DMCLOG_D("EDB_RECORD_NOT_EXIST");
		return EDB_RECORD_NOT_EXIST;
	}
	EXIT_FUNC();
    return errcode;	
}

static int get_user_list_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	user_list_t *plist = (user_list_t *)data;
	user_info_t *pfi = NULL;

	pfi = (user_info_t *)calloc(1,sizeof(user_info_t));
	assert(pfi != NULL);
	load_user_info(FieldName, FieldValue, nFields, pfi);
	dl_list_add_tail(&plist->head, &pfi->next);
	plist->len++;
	return 0;
}

static error_t load_user_insert_cmd(sqlite3 *database,user_info_t *puser)
{
	ENTER_FUNC();
	int n;
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	char name_str_escape[MAX_USER_NAME_LEN+ 0x10];
	char nickname_str_escape[MAX_USER_NAME_LEN+ 0x10];
	if(sql == NULL || puser == NULL || !*puser->user_name)
	{
		return ENULL_POINT;
	}
	sqlite3_str_escape(puser->user_name, name_str_escape, sizeof(name_str_escape));
	if(*puser->nick_name)
	{
		sqlite3_str_escape(puser->nick_name, nickname_str_escape, sizeof(nickname_str_escape));
	}

	n = snprintf(sql, SQL_CMD_WRITE_BUF_SIZE, "INSERT INTO %s(USER_NAME,PASSWORD,NICK_NAME)"\
		"VALUES('%s','%s','%s')",USER_TABLE_NAME,
		name_str_escape, puser->password, nickname_str_escape);

	if(n >= SQL_CMD_WRITE_BUF_SIZE)
	{
		return EOUTBOUND;
	}
	EXIT_FUNC();
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

static error_t load_user_delete_cmd(sqlite3 *database,user_info_t *puser)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "DELETE FROM %s WHERE USER_ID ='%d'", USER_TABLE_NAME, puser->user_id);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}
//UPDATE %s SET HD_USED_SIZE=%llu
static error_t load_user_update_password_cmd(sqlite3 *database,user_info_t *puser)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "UPDATE %s SET PASSWORD ='%s' WHERE USER_ID='%d'", USER_TABLE_NAME, puser->password,puser->user_id);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

static error_t load_user_update_nick_name_cmd(sqlite3 *database,user_info_t *puser)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "UPDATE %s SET NICK_NAME ='%s' WHERE USER_ID='%d'", USER_TABLE_NAME, puser->password,puser->user_id);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

static error_t load_user_list_cmd(sqlite3 *database, user_list_t *plist)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	sprintf(sql, "SELECT * FROM %s", USER_TABLE_NAME);
	return sqlite3_exec_busy_wait(database, sql, get_user_list_callback, plist);
}

error_t load_user_info_cmd(sqlite3 *database, user_info_t *user_info)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;
	
	sprintf(sql,"SELECT * FROM %s WHERE USER_NAME='%s'",USER_TABLE_NAME, user_info->user_name);

	if((errcode = sqlite3_exec_busy_wait(database, sql, get_user_callback, user_info))
			!= RET_SUCCESS)
	{
		return errcode;
	}
	if(user_info->user_id == 0)
	{
		DMCLOG_E("the %s is not exist",user_info->user_name);
		return EDB_RECORD_NOT_EXIST;
	}
	return errcode;
}

error_t load_user_info_by_id_cmd(sqlite3 *database, user_info_t *user_info)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;
	
	sprintf(sql,"SELECT * FROM %s WHERE USER_ID='%d'",USER_TABLE_NAME, user_info->user_id);

	if((errcode = sqlite3_exec_busy_wait(database, sql, get_user_callback, user_info))
			!= RET_SUCCESS)
	{
		return errcode;
	}
	return errcode;
}



/**
 * 新增一个用户信息
 */
error_t user_table_insert(sqlite3 *database,void *data)
{
	ENTER_FUNC();
	user_insert_t *user_insert = (user_insert_t *)data;
	error_t errcode = RET_SUCCESS;
	int g_id = INVALID_USER_ID;
	if(database == NULL || data == NULL)
	{
		DMCLOG_E("para is null");
		return ENULL_POINT;
	}
	if(user_insert->cmd == USER_TABLE_INSERT_INFO)
	{
		user_info_t *puser = &user_insert->user_info;
		errcode = user_name_exist(database, puser->user_name,&g_id);
		if(errcode == EDB_RECORD_NOT_EXIST)//user not exist, go ahead
		{
			errcode = load_user_insert_cmd(database,puser);
			if(errcode != RET_SUCCESS)
			{
				DMCLOG_E("user insert error");
				return errcode;
			}
			errcode = get_max_user_id(database,&puser->user_id);
			if(errcode != RET_SUCCESS)
			{
				DMCLOG_E("get max user id error");
				return errcode;
			}
		}
	}else if(user_insert->cmd == USER_TABLE_INSERT_LIST)
	{
		//TODO
	}
	EXIT_FUNC();
	return errcode;
}


/**
 * 删除一个用户
 */
error_t user_table_delete(sqlite3 *database,void *data)
{
	int g_id = INVALID_USER_ID;
	error_t errcode = RET_SUCCESS;
	if(database == NULL || data == NULL)
	{
		return EINVAL_ARG;
	}
	user_delete_t *user_delete = (user_delete_t *)data;
	if(user_delete->cmd == USER_TABLE_DELETE_INFO)
	{
		user_info_t *puser = (user_info_t *)data;
		errcode = user_name_exist(database, puser->user_name,&puser->user_id);
		if(errcode == EDB_RECORD_NOT_EXIST)//user not exist, go ahead
		{
			return load_user_delete_cmd(database,puser);
		}
	}else if(user_delete->cmd == USER_TABLE_DELETE_LIST)
	{
		//TODO
	}
	
	return errcode;
}

/**
 * modify password or set user nick name
 */
error_t user_table_update(sqlite3 *database,void *data)
{
	error_t errcode = RET_SUCCESS;
	if(database == NULL || data == NULL)
	{
		return EINVAL_ARG;
	}
	user_update_t *puser_update = (user_update_t *)data;
	if(puser_update->cmd == USER_TABLE_UPDATE_PASSWORD)
	{
		return load_user_update_password_cmd(database,&puser_update->user_info);
	}else if(puser_update->cmd == USER_TABLE_UPDATE_NICK_NAME)
	{
		return load_user_update_nick_name_cmd(database,&puser_update->user_info);
	}
	return errcode;
}

/**
 * 获取用户列表
 */
error_t user_table_query(sqlite3 *database,void *data)
{
	if(database == NULL || data == NULL)
	{
		return EINVAL_ARG;
	}
	user_query_t *puser_query = (user_query_t *)data;
	if(puser_query->cmd == USER_TABLE_QUERY_LIST)
	{
		return load_user_list_cmd(database, &puser_query->user_list);
	}else if(puser_query->cmd == USER_TABLE_QUERY_INFO)
	{
		return load_user_info_cmd(database, &puser_query->user_info);
	}else if(puser_query->cmd == USER_TABLE_QUERY_INFO_BY_ID)
	{
		return load_user_info_by_id_cmd(database, &puser_query->user_info);
	}
	return ESHELL_CMD;
}

void register_user_table_ops(void)
{
	memset(&g_user_table, 0, sizeof(g_user_table));
	
	g_user_table.ops.insert          = user_table_insert;
	g_user_table.ops.update   		 = user_table_update;
	g_user_table.ops.delete     	 = user_table_delete;
	g_user_table.ops.query           = user_table_query;
}

