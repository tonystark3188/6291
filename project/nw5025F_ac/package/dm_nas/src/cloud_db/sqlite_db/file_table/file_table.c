/*
 * =============================================================================
 *
 *       Filename: file_table.c
 *
 *    Description: file table related data structure definition.
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
#include "file_table.h"
//get file information from sqlite query result
static void load_file_info(char **FieldName, char **FieldValue, int nFields, file_info_t *pfile)
{
	if(FieldValue[0] != NULL)
    	pfile->real_file_id= strtoul(FieldValue[0], NULL, 10);
	
	if(FieldValue[1] != NULL)
	{
		S_STRNCPY(pfile->real_name, FieldValue[1], MAX_REAL_NAME_LEN);
		DMCLOG_D("pfile->real_name = %s",pfile->real_name);
	}

	if(FieldValue[2] != NULL)
	{
		S_STRNCPY(pfile->real_path, FieldValue[2], MAX_REAL_PATH_LEN);
		DMCLOG_D("pfile->real_path = %s",pfile->real_path);
	}
	#if 0
	//if(FieldValue[3] != NULL)
    //	pfile->type = atoi(FieldValue[3]);
	#endif
	if(FieldValue[4] != NULL)
    	pfile->size = strtoul(FieldValue[4], NULL, 10);

	if(FieldValue[5] != NULL)
    	pfile->ctime = strtoul(FieldValue[5], NULL, 10);

	if(FieldValue[6] != NULL)
    	pfile->mtime = strtoul(FieldValue[6], NULL, 10);

	if(FieldValue[7] != NULL)
    	pfile->atime = strtoul(FieldValue[7], NULL, 10);

	if(FieldValue[8] != NULL)
	{
		S_STRNCPY(pfile->uuid, FieldValue[8], MAX_FILE_UUID_LEN);
		DMCLOG_D("pfile->uuid = %s",pfile->uuid);
	}

	if(FieldValue[9] != NULL)
	{
		pfile->link = atoi(FieldValue[9]);
		DMCLOG_D("link = %d",pfile->link);
	}
	if(FieldValue[10] != NULL)
	{
		pfile->media_index= atoi(FieldValue[10]);
		DMCLOG_D("media_index = %d",pfile->media_index);
	}
	if(FieldValue[11] != NULL)
	{
		pfile->thum_index = atoi(FieldValue[11]);
		DMCLOG_D("thum_index = %d",pfile->thum_index);
	}
    	
}


static int get_file_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	file_info_t *pfile = (file_info_t *)data;

	load_file_info(FieldName, FieldValue, nFields, pfile);	

	return 0;
}

//sqlite exec callback to get file id
static int get_file_id_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	int *pid = (int *)data;

	if(FieldValue[0] != NULL)
	{
		*pid = atoi(FieldValue[0]);
	}
	return 0;
}

//sqlite exec callback to get file link
static int get_file_link_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	int *pid = (int *)data;

	if(FieldValue[0] != NULL)
	{
		*pid = atoi(FieldValue[0]);
		DMCLOG_D("link = %d",*pid);
	}
	return 0;
}


error_t load_uuid_exist_cmd(sqlite3 *database,char *uuid)
{
    char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;
   	int g_id = INVALID_FILE_ID;
	if(database == NULL)
	{
		return EINVAL_ARG;
	}
	sprintf(sql, "SELECT * FROM %s where UUID = '%s'",
		FILE_TABLE_NAME, uuid);
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_file_id_callback, &g_id))
			!= RET_SUCCESS)
	{
        DMCLOG_E("exit error");
		return errcode;
	}
    if(g_id == INVALID_FILE_ID)
    {
        DMCLOG_D("EDB_RECORD_NOT_EXIST");
		return EDB_RECORD_NOT_EXIST;
	}
    return errcode;	
}

error_t load_file_insert_cmd(sqlite3 *database,file_info_t *pfile)
{
	ENTER_FUNC();
	int n;
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	if(sql == NULL || pfile == NULL)
	{
		DMCLOG_E("para is null");
		return ENULL_POINT;
	}
	n = snprintf(sql, SQL_CMD_WRITE_BUF_SIZE, "INSERT INTO %s(NAME,PATH,TYPE,SIZE,CREATE_TIME,MODIFY_TIME,ACCESS_TIME,UUID,LINK,MEDIA_INDEX,THUM_INDEX)"\
		"VALUES('%s','%s','%d','%lld','%ld','%ld','%ld','%s','%d','%d','%d')",FILE_TABLE_NAME,
		pfile->real_name,pfile->real_path, pfile->type, (long long)pfile->size,pfile->ctime,pfile->mtime,pfile->atime,pfile->uuid,1,pfile->media_index,pfile->thum_index);
	DMCLOG_D("name = %s,path = %s",pfile->real_name,pfile->real_path);
	if(n >= SQL_CMD_WRITE_BUF_SIZE)
	{
		return EOUTBOUND;
	}
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

error_t load_file_delete_cmd(sqlite3 *database,int id)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "DELETE FROM %s WHERE FILE_ID ='%d'", FILE_TABLE_NAME, id);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

static error_t load_file_update_time_cmd(sqlite3 *database,file_info_t *pfile)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "UPDATE %s SET CREATE_TIME ='%ld',MODIFY_TIME = '%ld' AND ACCESS_TIME='%ld' WHERE FILE_ID='%d'", FILE_TABLE_NAME, pfile->ctime,pfile->mtime,pfile->atime,pfile->real_file_id);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

error_t load_file_update_cmd(sqlite3 *database,file_info_t *pfile)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "UPDATE %s SET TYPE='%d',SIZE='%lld',CREATE_TIME ='%ld',MODIFY_TIME = '%ld',ACCESS_TIME='%ld',UUID='%s' WHERE FILE_ID='%d'", \
		FILE_TABLE_NAME, pfile->type,(long long)pfile->size,pfile->ctime,pfile->mtime,pfile->atime,pfile->uuid,pfile->real_file_id);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

error_t load_file_update_media_index_cmd(sqlite3 *database,int real_file_id,int media_index)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "UPDATE %s SET MEDIA_INDEX='%d' WHERE FILE_ID='%d'", FILE_TABLE_NAME, media_index,real_file_id);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

error_t load_file_update_thum_index_cmd(sqlite3 *database,int real_file_id,int thum_index)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "UPDATE %s SET THUM_INDEX='%d' WHERE FILE_ID='%d'", FILE_TABLE_NAME, thum_index,real_file_id);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}



error_t load_file_info_by_path_cmd(sqlite3 *database, char *real_path,int *real_id)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	sprintf(sql, "SELECT * FROM '%s' WHERE PATH='%s'",FILE_TABLE_NAME,real_path);
	return sqlite3_exec_busy_wait(database, sql, get_file_id_callback, real_id);
}

error_t load_file_info_cmd(sqlite3 *database, file_info_t *pfile)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	sprintf(sql, "SELECT * FROM %s WHERE FILE_ID='%d'", FILE_TABLE_NAME,pfile->real_file_id);
	return sqlite3_exec_busy_wait(database, sql, get_file_callback, pfile);
}

error_t load_max_real_id(sqlite3 *database,uint32_t *max_id)
{
	error_t errcode;
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql,"SELECT MAX(FILE_ID) FROM %s",FILE_TABLE_NAME);
	if((errcode = sqlite3_exec_busy_wait(database,sql,get_file_id_callback, max_id)) != SQLITE_OK)
	{
		DMCLOG_D("sqlite3_exec_busy_wait error");
        sqlite3_close(database);
        return errcode;
	}
	return errcode;
}

error_t load_file_link_cmd(sqlite3 *database,int id,int *link)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	sprintf(sql, "SELECT LINK FROM %s WHERE FILE_ID='%d'", FILE_TABLE_NAME,id);
	return sqlite3_exec_busy_wait(database, sql, get_file_link_callback, link);
}

error_t load_file_des_link_cmd(sqlite3 *database,int id)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	int link;
	load_file_link_cmd(database,id,&link);
	link++;
	sprintf(sql, "UPDATE %s SET LINK='%d' WHERE FILE_ID='%d'", \
		FILE_TABLE_NAME, link,id);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

error_t load_file_asc_link_cmd(sqlite3 *database,int id,int link)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	link--;
	sprintf(sql, "UPDATE %s SET LINK='%d' WHERE FILE_ID='%d'", \
		FILE_TABLE_NAME, link,id);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}






