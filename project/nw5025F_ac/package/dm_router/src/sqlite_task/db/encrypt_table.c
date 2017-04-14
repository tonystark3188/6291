/*
 * =============================================================================
 *
 *       Filename: encrypt_table.c
 *
 *    Description: encrypt table operation definition.
 *
 *        Version:  1.0
 *        Created:  2017/03/13
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 515296288jf@163.com
 *   Organization:  longsys
 *
 * =============================================================================
 */
#include "db/encrypt_table.h"
#include "db/db_base.h"
#include "db/nas_db.h"



int encrypt_get_parent_id_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
   
	int id = (int *)data;
	id = atol(FieldValue[3]);
    return 0;
}

int encrypt_get_parent_path_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
    ENTER_FUNC();
    char *path = (char *)data;
    if(FieldValue[2] != NULL)
 		if(path == NULL)
		{
			path = (char *)calloc(1,strlen(FieldValue[2]) + 1);
			strcpy(path , FieldValue[2]);
		}
	
    return 0;
}
error_t encrypt_query_parent_path_by_id(sqlite3 *database, uint32_t id, char *path)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;

	if(id == INVALID_FILE_ID)
	{
		return EDB_INVALID_ID;
	}

	sprintf(sql,"SELECT * FROM %s WHERE ID=%u",ENCRYPT_TABLE_NAME, id);

	if((errcode = sqlite3_exec_busy_wait(database, sql, encrypt_get_parent_path_callback, path))
			!= RET_SUCCESS)
	{
		return errcode;
	}

	if(*path == NULL)
	{
		return EDB_RECORD_NOT_EXIST;
	}
	return errcode;
}

//similar to function query_file_info, but the target file is given by id
error_t encrypt_query_parent_id_by_id(sqlite3 *database, uint32_t id, uint32_t parent_id)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;

	if(id == INVALID_FILE_ID)
	{
		return EDB_INVALID_ID;
	}

	sprintf(sql,"SELECT * FROM %s WHERE ID=%u",ENCRYPT_TABLE_NAME, id);

	if((errcode = sqlite3_exec_busy_wait(database, sql, encrypt_get_parent_id_callback, &parent_id))
			!= RET_SUCCESS)
	{
		return errcode;
	}

	if(parent_id == 0)
	{
		return EDB_RECORD_NOT_EXIST;
	}
	return errcode;
}

//sqlite exec callback to get encrypt id
static int get_encrypt_id_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
    int *pid = (int *)data;
    
    if(FieldValue[0] != NULL)
    {
        *pid = strtoul(FieldValue[0], NULL, 10);
    }
    return 0;
}
int get_encrypt_encrypt_id_callback(void *data, int nFields, char **FieldValue, char **FieldName){
    int *pid = (int *)data;
    
    if(FieldValue[0] != NULL)
    {
        *pid = strtoul(FieldValue[0], NULL, 10);
    }
    return 0;
}
int encrypt_get_encrypt_id_by_name(sqlite3 *database,const char*name,int *id)
{
    char sql[SQL_CMD_QUERY_BUF_SIZE];
    
    if(database == NULL || name == NULL)
    {
        return ENULL_POINT;
    }
    
    sprintf(sql, "SELECT * FROM %s WHERE NAME = '%s'", ENCRYPT_TABLE_NAME,name);
    
    return sqlite3_exec_busy_wait(database, sql, get_encrypt_encrypt_id_callback,id);
}

error_t get_max_encrypt_id(sqlite3 *database,int *max_id)
{
    error_t errcode;
    char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
    sprintf(sql,"SELECT MAX(ID) FROM %s",ENCRYPT_TABLE_NAME);
    if((errcode = sqlite3_exec_busy_wait(database,sql,get_encrypt_id_callback, max_id)) != SQLITE_OK)
    {
        DMCLOG_D("sqlite3_exec_busy_wait error");
        sqlite3_close(database);
        return errcode;
    }
    return errcode;
}

static int get_encrypt_file_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{

    encrypt_info_t *pvi = (encrypt_info_t *)data;
    
    if(FieldValue[0] != NULL)
        pvi->id = atoi(FieldValue[0]);

    if(FieldValue[1] != NULL)
        //if(pvi->name == NULL)
		{
		    //DMCLOG_D("FieldValue[1]=%s",FieldValue[1]);
			pvi->name = (char *)calloc(1,strlen(FieldValue[1]) + 1);
			strcpy(pvi->name , FieldValue[1]);
		}

    if(FieldValue[2] != NULL)

 		//if(pvi->path == NULL)
		{
			//DMCLOG_D("FieldValue[2]=%s",FieldValue[2]);
			pvi->path = (char *)calloc(1,strlen(FieldValue[2]) + 1);
			memcpy(pvi->path , FieldValue[2],strlen(FieldValue[2]));
			//DMCLOG_D("pvi->path=%s,len = %d",pvi->path,strlen(FieldValue[2]));
		}
	
    if(FieldValue[3] != NULL)
        pvi->date = atol(FieldValue[3]);
        
    if(FieldValue[4] != NULL)
        pvi->fileSize = atol(FieldValue[4]);
   
    return 0;
}

error_t encrypt_table_query(sqlite3 *database, encrypt_info_t *pInfo)
{
    char sql[SQL_CMD_QUERY_BUF_SIZE];
    
    if(database == NULL || pInfo == NULL)
    {
        return ENULL_POINT;
    }
    
    sprintf(sql, "SELECT * FROM %s WHERE ID=%d", ENCRYPT_TABLE_NAME,pInfo->id);
    
    return sqlite3_exec_busy_wait(database, sql, get_encrypt_file_callback, pInfo);
}

error_t encrypt_table_update(sqlite3 *database, encrypt_info_t *pInfo)
{
    return 0;
}

error_t encrypt_table_insert(sqlite3 *database, encrypt_info_t *pInfo)
{
    char *sql = get_db_write_task_sql_buf();
    error_t errcode = RET_SUCCESS;
    
    
    if(database == NULL || pInfo == NULL)
    {
        return ENULL_POINT;
    }
    
    if(pInfo->isDir == 1)
    {
        sprintf(sql, "INSERT INTO %s(NAME,PATH,DATE,SIZE) VALUES('%s','%s',%ld,%lld)", ENCRYPT_TABLE_NAME,
                pInfo->name, pInfo->path,pInfo->date,pInfo->fileSize);
    }else{
        sprintf(sql, "INSERT INTO %s(NAME,DATE,SIZE) VALUES('%s',%ld,%lld)", ENCRYPT_TABLE_NAME,
                pInfo->name,pInfo->date,pInfo->fileSize);
    }
    return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

error_t encrypt_table_delete(sqlite3 *database, encrypt_info_t *pInfo)
{
    char *sql = get_db_write_task_sql_buf();
    error_t errcode = RET_SUCCESS;
    if(database == NULL || pInfo == NULL)
    {
        return ENULL_POINT;
    }
    sprintf(sql, "DELETE FROM %s WHERE ID=%d", ENCRYPT_TABLE_NAME,
            pInfo->id);
    return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

