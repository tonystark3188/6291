/*
 * =============================================================================
 *
 *       Filename: version_table.c
 *
 *    Description: version table related data structure definition.
 *
 *        Version:  1.0
 *        Created:  2016/12/10
 *       Revision:  none
 *       Compiler:  gcc
 *
 *          Author:  Oliver <henry.jin@dmsys.com>
 *   Organization:  longsys
 *
 * =============================================================================
 */
 #include "version_table.h"


static error_t load_version_insert_cmd(char *sql, version_info_t *pvi)
{
	if(sql == NULL || pvi == NULL)
	{
		return ENULL_POINT;
	}

	sprintf(sql, "INSERT INTO %s(VERSION,STATUS) VALUES('%s',%d)", VERSION_TABLE_NAME,
			pvi->version, pvi->state);

	return RET_SUCCESS;
}

error_t version_table_insert(sqlite3 *database, version_info_t *pvi)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	error_t errcode = RET_SUCCESS;

	
	if(database == NULL || pvi == NULL)
	{
		return ENULL_POINT;
	}

	if((errcode = load_version_insert_cmd(sql, pvi)) != RET_SUCCESS)
	{	
		DMCLOG_E("load version insert error,version = %s",pvi->version);
		return errcode;
	}

	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}


error_t version_table_update(sqlite3 *database, update_version_t *puv)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	
	if(database == NULL || puv == NULL)
	{
		return ENULL_POINT;
	}

	if(puv->update_state == FALSE && puv->update_version == FALSE)
	{
		return RET_SUCCESS;
	}
    else if(puv->update_state == TRUE && puv->update_version == FALSE)
    {
		sprintf(sql,"UPDATE %s SET STATUS=%d",VERSION_TABLE_NAME, puv->state);
	}
	else if(puv->update_state == FALSE && puv->update_version == TRUE)
	{
		if(puv->version == NULL)
		{
			return ENULL_POINT;
		}
		sprintf(sql, "UPDATE %s SET VERSION='%s'", VERSION_TABLE_NAME, puv->version);
	}
	else
	{
		if(puv->version == NULL)
		{
			return ENULL_POINT;
		}
		sprintf(sql, "UPDATE %s SET VERSION='%s' , STATUS=%d", VERSION_TABLE_NAME,
					puv->version, puv->state);
	}

	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}



static int get_version_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	version_info_t *pvi = (version_info_t *)data;

	if(FieldValue[0] != NULL)
		S_STRNCPY(pvi->version, FieldValue[0], MAX_VERSION_SIZE);
	else
		pvi->version[0] = 0;

	if(FieldValue[1] != NULL)
		pvi->state = atoi(FieldValue[1]);

	return 0;
}

error_t version_table_query(sqlite3 *database, version_info_t *pvi)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE] = {0};
	error_t errcode = RET_SUCCESS;
	if(database == NULL || pvi == NULL)
	{
		return ENULL_POINT;
	}

	sprintf(sql, "SELECT * FROM %s", VERSION_TABLE_NAME);
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_version_callback, pvi))
			!= RET_SUCCESS)
	{
		return errcode;
	}

	if(!*pvi->version)
	{
		return EDB_RECORD_NOT_EXIST;
	}
	return errcode;
}

