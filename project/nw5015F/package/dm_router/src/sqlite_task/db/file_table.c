/*
 * =============================================================================
 *
 *       Filename: file_table.c
 *
 *    Description: file table operation definition.
 *
 *        Version:  1.0
 *        Created:  2014/09/10 
 *       Revision:  none
 *       Compiler:  gcc
 *
 *        Version:1.1
 *        Time:2014/10/27
 *        Desc: modify the strategy of trashing file and recycling file.
 *
 *         Author:  Tao Sheng (), vanbreaker90@gmail.com
 *   Organization:  
 *
 * =============================================================================
 */

#include <sys/sysinfo.h>

#include "db/file_table.h"
#include "db/user_table.h"
#include "db/db_mm_pool.h"
#include "db/db_table.h"
#include "db/db_config.h"
#include "file_opr.h"
#include "db/nas_db.h"
#include "defs.h"

#define COMMIT_COUNT 32
#define  KB  1024LL
#define  MB (1024*KB)
#define  GB (1024*MB)



//typedef error_t (*traverse_handler)(sqlite3 *database, char **FieldValue);
extern error_t get_file_list_by_parent_id(sqlite3 *database, file_list_t *file_list);
extern error_t get_file_album_by_parent_id(sqlite3 *database, file_list_t *file_list);




db_table_t g_file_table;
static uint32_t  g_file_id = 0;

static int get_id_callback(void *data, int nFields, char **FieldValue, char **FieldName);


extern error_t get_file_path(sqlite3 *database, uint32_t id, file_info_t *pfi);
extern int get_time_str_for_db(char *time_s, size_t size);


//eg.   if we have a file with path '/root/a/b/c', the level of c is 4.
static int get_file_level(const char *path)
{
    int i,len,level;

    len = strlen(path);
    if(len <= 0)
    {
        return 0;
    }

    for(i=0,level=0;i<len;i++)
    {
        if(path[i] == SEPERATOR)
        {
	        level++;
        }
    }
    return level;
}


int file_get_parent_id(sqlite3 *database,char *path,int *parent_id)
{
	if(database == NULL || path == NULL)
	{
		return ENULL_POINT;
	}
	error_t errcode = RET_SUCCESS;
	char *tmp = strrchr(path,'/');
	*tmp = '\0';
	errcode = query_dir_file_id(database,path,parent_id);
	if(errcode == EDB_RECORD_NOT_EXIST)
	{
		DMCLOG_D("rename file doesn't exist,src_path=%s",path);
		*tmp = '/';
		return EDB_RECORD_NOT_EXIST;
	}
	*tmp = '/';
	return errcode;
}



//get  appointed level's name in a path
//eg.  the name of third level of '/root/aa/bb/cc'  is 'bb'
static error_t get_level_file_name(const char *path, int level, char *name)
{
    int i,j=0,l=0;

	if(path == NULL || level <= ROOT_LEVEL)
	{
		return EINVAL_ARG;
	}

	for(i=0; path[i] != 0; i++)
	{
		if(path[i] == SEPERATOR)
		{
		    l++;
			if(l == level)
			{
				break;
			}
		}
	}

	if(l != level)
	{
		return EINVAL_ARG;
	}

	i++;
	while((path[i] != SEPERATOR) && (path[i] != 0))
	{
		name[j++] = path[i++];
	}
	name[j] = 0;

	return RET_SUCCESS;
}
char *db_path_escape(char *path)
{
	char *path_escape;
	int i;
	error_t errcode = RET_SUCCESS;
	char name[NAME_SIZE];
    char name_str_escape[NAME_SIZE + 0x10];
	if(path == NULL)
	{
		return NULL;
	}
	path_escape = (char *)calloc(1,strlen(path) + 0x10);
	assert(path_escape != NULL);
	int level = get_file_level(path);
	for(i=ROOT_LEVEL+1;i<=level;i++)
	{
		if((errcode = get_level_file_name(path, i, name)) != RET_SUCCESS)
		{
			return NULL;
		}
        // modify by wenhao at 2014-11-21 for SQL escape bug.
        sqlite3_str_escape(name, name_str_escape, sizeof(name_str_escape));
		if(i == ROOT_LEVEL+1)
		{
			sprintf(path_escape,"%c",'/');
		}else{
			strcat(path_escape,"/");
		}
        strcat(path_escape,name_str_escape);
	}
	return path_escape;
}


/*
 *description: query file's id
 *input param:parent_id--> id of parent directory    name-->file's name
 *output param:id-->file id we are querying
 *return: RET_SUCCESS if ok, otherwise 
*/
static error_t query_file_id(sqlite3 *database, uint32_t parent_id, char *name, uint32_t *id)
{
    char sql[SQL_CMD_QUERY_BUF_SIZE];
	char name_str_escape[NAME_SIZE + 0x10];
	if(database == NULL || name == NULL || id == NULL)
	{
		return ENULL_POINT;
	}
	sqlite3_str_escape(name, name_str_escape, sizeof(name_str_escape));
	*id = INVALID_FILE_ID;
	
	sprintf(sql, "SELECT ID FROM %s where PARENT_ID = %u AND NAME='%s'",
		FILE_TABLE_NAME, parent_id, name_str_escape);

	return sqlite3_exec_busy_wait(database, sql, get_id_callback, id);
}


/*
*description: query file's  detail information
*input param:parent_id-->the target file's parent dir id  name-->the target file's name
*output param:pfi-->pointer to buffer to store the file information.
*return: RET_SUCCESS if ok.
*/
error_t query_file_info(sqlite3 *database,char *path,file_info_t *pfi)
{
    char sql[SQL_CMD_QUERY_BUF_SIZE];
	pfi->index = INVALID_FILE_ID;
	error_t errcode = RET_SUCCESS;
    
	if(database == NULL||path == NULL)
	{
		return EINVAL_ARG;
	}
	char *path_escape = db_path_escape(path);
	if(path_escape == NULL)
	{
		DMCLOG_D("rename file error,src_path=%s",path);
		return EDB_RECORD_NOT_EXIST;
	}
	DMCLOG_D("path_escape = %s",path_escape);
	sprintf(sql, "SELECT * FROM %s where PATH = '%s'",FILE_TABLE_NAME, path_escape);
	safe_free(path_escape);

	if((errcode = sqlite3_exec_busy_wait(database, sql, file_query_callback, pfi))
			!= RET_SUCCESS)
	{
        log_debug("exit error");
		return errcode;
	}

    if(pfi->index == INVALID_FILE_ID)
    {
        log_debug("EDB_RECORD_NOT_EXIST");
		return EDB_RECORD_NOT_EXIST;
	}
	
    return errcode;	
}

int file_id_query_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
    unsigned *g_id = (unsigned *)data;
    load_file_id(FieldName, FieldValue, nFields, g_id);
    return 0;
}

error_t query_dir_file_id(sqlite3 *database,char *path,unsigned *g_id)
{
	ENTER_FUNC();
    char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;
    
	if(database == NULL||path == NULL)
	{
		DMCLOG_E("the para is null");
		return EINVAL_ARG;
	}
	char *path_escape = db_path_escape(path);
	if(path_escape == NULL)
	{
		DMCLOG_D("rename file error,src_path=%s",path);
		return EDB_RECORD_NOT_EXIST;
	}
	sprintf(sql, "SELECT * FROM %s where PATH = '%s'",
		FILE_TABLE_NAME, path_escape);
	safe_free(path_escape);
	if((errcode = sqlite3_exec_busy_wait(database, sql, file_id_query_callback, g_id))
			!= RET_SUCCESS)
	{
        log_debug("exit error");
		return errcode;
	}

    if(*g_id == INVALID_FILE_ID)
    {
        log_debug("EDB_RECORD_NOT_EXIST");
		return EDB_RECORD_NOT_EXIST;
	}
	EXIT_FUNC();
    return errcode;	
}

error_t query_file_info_by_parentid_and_name(sqlite3 *database,unsigned parent_id,char *name,file_info_t *pfi)
{
    char sql[SQL_CMD_QUERY_BUF_SIZE];
	pfi->index = INVALID_FILE_ID;
	error_t errcode = RET_SUCCESS;
    char name_str_escape[NAME_SIZE + 0x10] = {0};
	if(database == NULL)
	{
		return EINVAL_ARG;
	}
	sqlite3_str_escape(name, name_str_escape, sizeof(name_str_escape));
	sprintf(sql, "SELECT * FROM %s where NAME = '%s' AND PARENT_ID = %u",
		FILE_TABLE_NAME, name_str_escape,parent_id);
	if((errcode = sqlite3_exec_busy_wait(database, sql, file_query_callback, pfi))
			!= RET_SUCCESS)
	{
        log_debug("exit error");
		return errcode;
	}
	//DMCLOG_D("name = %s,parent_id = %u",name,parent_id);
    if(pfi->index == INVALID_FILE_ID)
    {
        log_debug("EDB_RECORD_NOT_EXIST");
		return EDB_RECORD_NOT_EXIST;
	}

//    log_debug("exit success");
	
    return errcode;	
}

error_t query_file_id_by_parentid_and_name(sqlite3 *database,unsigned parent_id,char *name,unsigned *g_id)
{
    char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;
    char name_str_escape[NAME_SIZE + 0x10];
    *g_id = INVALID_FILE_ID;
	if(database == NULL)
	{
		return EINVAL_ARG;
	}
	sqlite3_str_escape(name, name_str_escape, sizeof(name_str_escape));
	sprintf(sql, "SELECT * FROM %s where NAME = '%s' AND PARENT_ID = %u",
		FILE_TABLE_NAME, name_str_escape,parent_id);
	if((errcode = sqlite3_exec_busy_wait(database, sql, file_id_query_callback, g_id))
			!= RET_SUCCESS)
	{
        log_debug("exit error");
		return errcode;
	}
    if(*g_id == INVALID_FILE_ID)
    {
        log_debug("EDB_RECORD_NOT_EXIST");
		return EDB_RECORD_NOT_EXIST;
	}
    return errcode;	
}



/*
*description: query whether target file exists
input param:parent_id-->querying file's parent dir id.  name-->querying file's name
return: TRUE if file exists, FALSE if not exist.
*/
static bool file_item_exist(sqlite3 *database, uint32_t parent_id, const char *name)
{
    uint32_t id = INVALID_FILE_ID;
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode;

	if(database == NULL || name == NULL)
	{
		return FALSE;
	}

	sprintf(sql, "SELECT * FROM %s where PARENT_ID = %u AND NAME='%s'",
		FILE_TABLE_NAME, parent_id, name);

	if((errcode = sqlite3_exec_busy_wait(database, sql, get_id_callback, &id))
			!= SQLITE_OK)
	{
		return FALSE;
	}

    if(id == INVALID_FILE_ID)
    {
		return FALSE;
	}
	
    return TRUE;
}


//similar to function file_item_exist,but the file is given by id.
static bool file_item_exist_by_id(sqlite3 *database, uint32_t id)
{
	uint32_t id_temp = INVALID_FILE_ID;
	char sql[SQL_CMD_QUERY_BUF_SIZE];

	if(database == NULL)
	{
		return FALSE;
	}

	sprintf(sql, "SELECT * FROM %s WHERE ID=%u", FILE_TABLE_NAME, id);
	if(sqlite3_exec_busy_wait(database, sql, get_id_callback, &id_temp) != SQLITE_OK)
	{
		return FALSE;
	}

	if(id_temp == INVALID_FILE_ID)
	{
		return FALSE;
	}

	return TRUE;
}





//similar to function query_file_info, but the target file is given by id
error_t query_file_by_id(sqlite3 *database, uint32_t id, file_info_t *pfi)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;

	if(id == INVALID_FILE_ID)
	{
		return EDB_INVALID_ID;
	}
	
	pfi->index = INVALID_FILE_ID;

	sprintf(sql,"SELECT * FROM %s WHERE ID=%u",FILE_TABLE_NAME, id);

	if((errcode = sqlite3_exec_busy_wait(database, sql, file_query_callback, pfi))
			!= RET_SUCCESS)
	{
		return errcode;
	}

	if(pfi->index == INVALID_FILE_ID)
	{
		return EDB_RECORD_NOT_EXIST;
	}
	return errcode;
}


//similar to function query_file_info, but the target file is given by id
error_t query_path_by_id(sqlite3 *database, uint32_t id, char **path)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;

	if(id == INVALID_FILE_ID)
	{
		return EDB_INVALID_ID;
	}

	sprintf(sql,"SELECT * FROM %s WHERE ID=%u",FILE_TABLE_NAME, id);

	if((errcode = sqlite3_exec_busy_wait(database, sql, file_path_query_callback, path))
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

error_t query_attr_by_id(sqlite3 *database, uint32_t id, int *attr)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;

	if(id == INVALID_FILE_ID)
	{
		return EDB_INVALID_ID;
	}

	sprintf(sql,"SELECT * FROM %s WHERE ID=%u",FILE_TABLE_NAME, id);

	if((errcode = sqlite3_exec_busy_wait(database, sql, file_attr_query_callback, attr))
			!= RET_SUCCESS)
	{
		return errcode;
	}
	return errcode;
}



error_t query_file_by_uuid(sqlite3 *database, char *uuid, file_info_t *pfi)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;
	char *parentPath = NULL;
	
	pfi->index = INVALID_FILE_ID;

	sprintf(sql,"SELECT * FROM %s WHERE FILE_UUID='%s'",FILE_BACKUP_VIEW, uuid);

	if((errcode = sqlite3_exec_busy_wait(database, sql, file_query_callback, pfi))
			!= RET_SUCCESS)
	{
		return errcode;
	}

	if(pfi->index == INVALID_FILE_ID)
	{
		DMCLOG_D("file is not exist");
		return EDB_RECORD_NOT_EXIST;
	}
	if((errcode = query_path_by_id(database, pfi->parent_id, &parentPath)) == RET_SUCCESS)
	{
		if(parentPath != NULL)
		{
			DMCLOG_D("parentPath = %s,pfi->name = %s",parentPath,pfi->name);
			pfi->path = (char *)calloc(1,strlen(parentPath) + strlen(pfi->name) + 2);
			sprintf(pfi->path,"%s/%s",parentPath,pfi->name);
			safe_free(parentPath);
		}
	}
	return errcode;
}



/*
*description:  get the id of certain level item in the path
 *input param:  path-->full path  level-->which level's id do we want to get
 *output param: id-->buffer to store id.
 *return: RET_SUCCESS if ok.
*/
static error_t get_level_file_id(sqlite3 *database, const char *path, int level, uint32_t *id)
{
    int i;
	uint32_t parent_id;
    char name[NAME_SIZE];
    
	error_t errcode = RET_SUCCESS;
	
    if(path == NULL || id == NULL ||level < ROOT_LEVEL)
    {
		return EINVAL_ARG;
	}

	if(level == ROOT_LEVEL)
	{
		*id = ROOT_DIR_ID;
		return RET_SUCCESS;
	}

	//go through from root dir until we reach destinated level.
	for(i=ROOT_LEVEL+1,parent_id=ROOT_DIR_ID;i<=level;i++)
	{
		if((errcode = get_level_file_name(path, i, name)) != RET_SUCCESS)
		{
			return errcode;
		}

        // modify by wenhao at 2014-11-21 for SQL escape bug.
        if((errcode = query_file_id(database, parent_id, name, id)) != RET_SUCCESS)
		{
			return errcode;
		}
        
        // modify end.

		if(*id == INVALID_FILE_ID)
		{
			DMCLOG_D("INVALID_FILE_ID");
			return EDB_RECORD_NOT_EXIST;
		}

		parent_id = *id;
	}

	return errcode;
}


/*
*description: transfer the file information to sqlite cmd for insert.
  input param: sql-->sql cmd string buffer.
  		        pfi-->pointer to buffer storing file information
  return : RET_SUCCESS if ok.
*/
static error_t load_file_insert_cmd(char *sql,file_info_t *pfi)
{
    int n;
	char name_str_escape[NAME_SIZE + 0x10] = {0};	
	if(sql == NULL || pfi == NULL)
	{
		return ENULL_POINT;
	}
	
	sqlite3_str_escape(pfi->name, name_str_escape,sizeof(name_str_escape));
	
	if(pfi->isFolder)
	{
		char *path_escape = db_path_escape(pfi->path);
		DMCLOG_D("path_escape = %s,parent_id = %u,index = %u",path_escape,pfi->parent_id,pfi->index);
		n = snprintf(sql, SQL_CMD_WRITE_BUF_SIZE, "INSERT INTO %s(ID,NAME,PATH,PARENT_ID,TYPE,SIZE,CREATE_TIME,"\
	    "MEDIA_INFO_INDEX,DIR,FILE_UUID,MIME_TYPE,CHANGE_TIME,ACCESS_TIME) "\
        "VALUES(%u,'%s','%s',%u,%d,%lld,%u,%d,%d,'%s','%s',%u,%u);", FILE_TABLE_NAME,
         pfi->index, name_str_escape,path_escape,pfi->parent_id, pfi->file_type, pfi->file_size,
         pfi->create_time,pfi->attr,pfi->isFolder,pfi->file_uuid,pfi->mime_type,pfi->modify_time,pfi->access_time);
		safe_free(path_escape);
	}else{
		n = snprintf(sql, SQL_CMD_WRITE_BUF_SIZE, "INSERT INTO %s(ID,NAME,PATH,PARENT_ID,TYPE,SIZE,CREATE_TIME,"\
	    "MEDIA_INFO_INDEX,DIR,FILE_UUID,MIME_TYPE,CHANGE_TIME,ACCESS_TIME) "\
        "VALUES(%u,'%s','%s',%u,%d,%lld,%u,%d,%d,'%s','%s',%u,%u);", FILE_TABLE_NAME,
         pfi->index, name_str_escape,"",pfi->parent_id, pfi->file_type, pfi->file_size,
         pfi->create_time,pfi->attr,pfi->isFolder,pfi->file_uuid,pfi->mime_type,pfi->modify_time,pfi->access_time);
	}
	if(n >= SQL_CMD_WRITE_BUF_SIZE)
	{
        log_warning("sql buffer size is not enough,the cmd is %s",sql);
		return EOUTBOUND;
	}
	return RET_SUCCESS;
}

static error_t update_file_info_by_id(sqlite3 *database, uint32_t id,file_info_t *pfi)
{
	char *sql = get_db_write_task_sql_buf();
	error_t errcode = RET_SUCCESS;

	sprintf(sql,"UPDATE %s SET FILE_UUID='%s', CHANGE_TIME=%u,TYPE=%d,SIZE=%lld,MEDIA_INFO_INDEX=%d WHERE ID=%u", FILE_TABLE_NAME, pfi->file_uuid, pfi->modify_time,pfi->file_type,pfi->file_size,pfi->attr, id);

	if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL))
			!= RET_SUCCESS)
	{
		DMCLOG_E("update file info error");
		return errcode;	
	}

	return errcode;
}




//load item information,a helper function in sqlite exec callback 
void load_file_item(char **FieldName, char **FieldValue, int nFields, file_info_t *pfi)
{
	if(FieldValue[8] != NULL)
        pfi->isFolder= atoi(FieldValue[8]);
	if(FieldValue[0] != NULL)
    	pfi->index = strtoul(FieldValue[0], NULL, 10);
	if(FieldValue[1] != NULL)
	{
		if(pfi->name == NULL)
		{
			pfi->name = (char *)calloc(1,strlen(FieldValue[1]) + 1);
			strcpy(pfi->name , FieldValue[1]);
		}
	}
	if(pfi->isFolder)
	{
		if(FieldValue[2] != NULL)
		{
			if(pfi->path == NULL)
			{
				pfi->path = (char *)calloc(1,strlen(FieldValue[2]) + 1);
				strcpy(pfi->path, FieldValue[2]);
			}
		}
	}else{
		pfi->path == NULL;
	}
	
	if(FieldValue[3] != NULL)
		pfi->parent_id = strtoul(FieldValue[3], NULL, 10);
	if(FieldValue[4] != NULL)
		pfi->file_type = atoi(FieldValue[4]);
	if(FieldValue[5] != NULL)
		pfi->file_size = strtoull(FieldValue[5], NULL, 10);

	if(FieldValue[7] != NULL)
	{
		pfi->attr = atoi(FieldValue[7]);
	}
	if(FieldValue[9] != NULL)
	{
		strcpy(pfi->file_uuid, FieldValue[9]);
	}
	if(FieldValue[11] != NULL)
        pfi->modify_time= strtoull(FieldValue[11], NULL, 10);
}

void load_file_uuid_and_id(char **FieldName, char **FieldValue, int nFields, file_uuid_t *pfi)
{
	
	if(FieldValue[1] != NULL)
	{
		pfi->name = (char *)calloc(1,strlen(FieldValue[1]) + 1);
		strcpy(pfi->name , FieldValue[1]);
	}
	
	if(FieldValue[9] != NULL)
	{
		strcpy(pfi->file_uuid, FieldValue[9]);
	}
	DMCLOG_D("pfi->name = %s",pfi->name);
}


void load_file_path(char **FieldName, char **FieldValue, int nFields, char  **path)
{
	if(FieldValue[2] != NULL)
	{
		*path = (char *)calloc(1,strlen(FieldValue[2]) + 1);
		strcpy(*path, FieldValue[2]);
	}
}

void load_file_attr(char **FieldName, char **FieldValue, int nFields, int  *attr)
{
	if(FieldValue[7] != NULL)
	{
		*attr = atoi(FieldValue[7]);
	}
}



//load item information,a helper function in sqlite exec callback 
void load_file_id(char **FieldName, char **FieldValue, int nFields, unsigned *id)
{
	if(FieldValue[0] != NULL)
    	*id = strtoul(FieldValue[0], NULL, 10);
}




int file_query_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
    file_info_t *pFileDscrpt = (file_info_t *)data;
    load_file_item(FieldName, FieldValue, nFields, pFileDscrpt);
    return 0;
}


int file_path_query_callback(void **data, int nFields, char **FieldValue, char **FieldName)
{
    char **pFileDscrpt = (char **)data;
    load_file_path(FieldName, FieldValue, nFields, pFileDscrpt);
    return 0;
}

int file_attr_query_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
    int *pFileDscrpt = (int *)data;
    load_file_attr(FieldName, FieldValue, nFields, pFileDscrpt);
    return 0;
}


static error_t dm_do_delete_by_id(sqlite3 *database, unsigned id)
{
    char *sql = get_db_write_task_sql_buf();
	error_t errcode = RET_SUCCESS;
    //delete record in file table.
    if(file_item_exist_by_id(database, id) == TRUE)
	{
		sprintf(sql, "DELETE FROM %s WHERE ID=%u", FILE_TABLE_NAME, id);
		if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL))
				!= SQLITE_OK)
		{
			return errcode;
		}
	}
	return errcode;
}

static error_t dm_do_hide_by_id(sqlite3 *database, unsigned id,bool attr)
{
    char *sql = get_db_write_task_sql_buf();
	error_t errcode = RET_SUCCESS;
    if(file_item_exist_by_id(database, id) == TRUE)
	{
		sprintf(sql, "UPDATE %s SET MEDIA_INFO_INDEX=%d WHERE ID=%u", FILE_TABLE_NAME,attr,id);
		if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL))
				!= SQLITE_OK)
		{
			return errcode;
		}
	}
	return errcode;
}

int dm_do_hide(sqlite3 *database,unsigned parent_id,bool attr)
{
	error_t errcode = RET_SUCCESS;
	file_list_t file_list;
	file_info_t *item,*n;
	memset(&file_list,0,sizeof(file_list_t));
	file_list.parent_id = parent_id;
	dl_list_init(&file_list.head);
	if((errcode = get_file_list_by_parent_id(database, &file_list)) != RET_SUCCESS)
	{
		DMCLOG_E("get file list by parent_id error");
		dl_list_for_each_safe(item, n, &file_list.head, file_info_t, next)
		{
		  free_db_fd(&item);
		}   
		return errcode;
	}
	
	DMCLOG_D("result_cnt(%u)", file_list.result_cnt);
    if(file_list.result_cnt > 0)
    {
		dl_list_for_each(item, &(file_list.head), file_info_t, next)
		{
		 // init item member
			if(item->isFolder == 1)
			{
				//the file is folder
				dm_do_hide(database,item->index,attr);
			}
			//the file is normal file
			if((errcode = dm_do_hide_by_id(database, item->index,attr)) != 0)
			{
				DMCLOG_E("update attr normal file error");
				break;
			}
		}
     }
     dl_list_for_each_safe(item, n, &(file_list.head), file_info_t, next)
     {
         free_db_fd(&item);
     }
	if((errcode = dm_do_hide_by_id(database, parent_id,attr)) != 0)
	{
		DMCLOG_E("hide normal file error");
		return errcode;
	}
    return errcode;
}

int dm_do_album_hide(sqlite3 *database,unsigned parent_id,bool attr)
{
	error_t errcode = RET_SUCCESS;
	file_list_t file_list;
	file_info_t *item,*n;
	memset(&file_list,0,sizeof(file_list_t));
	file_list.parent_id = parent_id;
	file_list.file_type = 3;
	dl_list_init(&file_list.head);
	if((errcode = get_file_album_by_parent_id(database, &file_list)) != RET_SUCCESS)
	{
		DMCLOG_E("get file list by parent_id error");
		dl_list_for_each_safe(item, n, &file_list.head, file_info_t, next)
		{
		  free_db_fd(&item);
		}   
		return errcode;
	}
	
	DMCLOG_D("result_cnt(%u)", file_list.result_cnt);
    if(file_list.result_cnt > 0)
    {
		dl_list_for_each(item, &(file_list.head), file_info_t, next)
		{
			//the file is normal file
			if((errcode = dm_do_hide_by_id(database, item->index,attr)) != 0)
			{
				DMCLOG_E("update attr normal file error");
				break;
			}
		}
     }
     dl_list_for_each_safe(item, n, &(file_list.head), file_info_t, next)
     {
         free_db_fd(&item);
     }
    return errcode;
}




int dm_do_delete(sqlite3 *database,unsigned parent_id)
{
	error_t errcode = RET_SUCCESS;
	file_list_t file_list;
	file_info_t *item,*n;
	memset(&file_list,0,sizeof(file_list_t));
	file_list.parent_id = parent_id;
	dl_list_init(&file_list.head);
	if((errcode = get_file_list_by_parent_id(database, &file_list)) != RET_SUCCESS)
	{
		DMCLOG_E("get file list by parent_id error");
		dl_list_for_each_safe(item, n, &file_list.head, file_info_t, next)
		{
		  free_db_fd(&item);
		}   
		return errcode;
	}
	
	DMCLOG_D("result_cnt(%u)", file_list.result_cnt);
    if(file_list.result_cnt > 0)
    {
		dl_list_for_each(item, &(file_list.head), file_info_t, next)
		{
		 // init item member
			if(item->isFolder == 1)
			{
				//the file is folder
				dm_do_delete(database,item->index);
			}
			//the file is normal file
			if((errcode = dm_do_delete_by_id(database, item->index)) != 0)
			{
				DMCLOG_E("delete normal file error");
				break;
			}
		}
     }
     dl_list_for_each_safe(item, n, &(file_list.head), file_info_t, next)
     {
         free_db_fd(&item);
     }
	if((errcode = dm_do_delete_by_id(database, parent_id)) != 0)
	{
		DMCLOG_E("delete normal file error");
		return errcode;
	}
    return errcode;
}


static error_t dm_do_delete_by_path(sqlite3 *database, uint32_t parent_id,int file_type)
{
    char *sql = get_db_write_task_sql_buf();
	error_t errcode = RET_SUCCESS;
    //delete record in file table.
 	sprintf(sql, "DELETE FROM %s WHERE PARENT_ID=%u AND TYPE = %d", FILE_TABLE_NAME, parent_id,file_type);
	if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL))
			!= SQLITE_OK)
	{
		return errcode;
	}
	return errcode;
}


static error_t file_table_delete_normal(sqlite3 *database,char *path)
{
	unsigned parent_id;
	int id = 0;
	error_t errcode = RET_SUCCESS;
	if(database == NULL || path == NULL)
	{
		return ENULL_POINT;
	}
	errcode = file_get_parent_id(database,path,&parent_id);
	if(errcode != RET_SUCCESS)
	{
		DMCLOG_D("rename file doesn't exist,src_path=%s",path);
		return EDB_RECORD_NOT_EXIST;
	}

	DMCLOG_D("parent_id = %d",parent_id);
	
	char *name = bb_basename(path);
	DMCLOG_D("name = %s",name);
	errcode = query_file_id_by_parentid_and_name(database,parent_id,name,&id);
	if(errcode == EDB_RECORD_NOT_EXIST)
	{
		DMCLOG_D("file doesn't exist,src_path=%s",path);
		return EDB_RECORD_NOT_EXIST;
	}
	errcode = dm_do_delete(database,id);
	if(errcode!= RET_SUCCESS)
	{
		return errcode;
	}
	return errcode;
}

static error_t file_table_delete_type_by_path(sqlite3 *database,char *path,int file_type)
{
	unsigned parent_id;
	int id = 0;
	error_t errcode = RET_SUCCESS;
	if(database == NULL || path == NULL)
	{
		return ENULL_POINT;
	}
	errcode = query_dir_file_id(database,path,&parent_id);
	if(errcode == EDB_RECORD_NOT_EXIST)
	{
		DMCLOG_D("file doesn't exist,src_path=%s",path);
		return EDB_RECORD_NOT_EXIST;
	}

	DMCLOG_D("parent_id = %u,pdel_data->file_type = %d",parent_id,file_type);
	errcode = dm_do_delete_by_path(database,parent_id,file_type);
	if(errcode!= RET_SUCCESS)
	{
		return errcode;
	}
	return errcode;
}


static error_t file_table_delete(sqlite3 *database, void *target)
{
	ENTER_FUNC();
	error_t errcode = RET_SUCCESS;
	if(database == NULL || target == NULL)
	{
		return ENULL_POINT;
	}
	
	delete_data_t *pdel_data = (delete_data_t *)target;
	if(pdel_data->cmd == FILE_TABLE_DELETE_INFO)
	{
		return file_table_delete_normal(database,pdel_data->path);
	}else if(pdel_data->cmd == FILE_TABLE_DELETE_TYPE_BY_PATH)
	{
		return file_table_delete_type_by_path(database,pdel_data->path,pdel_data->file_type);
	}else if(pdel_data->cmd == FILE_TABLE_DELETE_LIST)
	{
		int i = 0;
		for(i = 0;;i++)
		{
			if(pdel_data->file_list[i] == NULL)
				break;
			errcode = file_table_delete_normal(database,pdel_data->file_list[i]);
			if(errcode != RET_SUCCESS)
			{
				DMCLOG_E("delete normal file error [%s]",pdel_data->file_list[i]);
				break;
			}
		}
	}
	EXIT_FUNC();
	return errcode;
}	

static int get_file_type_list_by_path_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	file_uuid_list_t *plist = (file_uuid_list_t *)data;
	file_uuid_t *pfi = (file_uuid_t *)calloc(1,sizeof(file_uuid_t));
	assert(pfi != NULL);
	load_file_uuid_and_id(FieldName, FieldValue, nFields, pfi);
	dl_list_add_tail(&plist->head, &pfi->next);
	plist->result_cnt++;
	return 0;
}

//get a new file id 
static uint32_t alloc_file_id(void)
{
    uint32_t id;

	id = g_file_id;
	g_file_id++;
	if(g_file_id < id)
	{
        log_warning("file id overflow!\n");
	}
	
	return g_file_id;
}


/*
*description:create a dir record when we insert a new path
*input param:pfi->dir information. 
*output param:dir_id->directory record id.
*return: RET_SUCCESS if ok
*/
static error_t create_dir_node(sqlite3 *database, file_info_t *pfi, uint32_t *dir_id)
{
	char *sql = get_db_write_task_sql_buf();
	error_t errcode;
	
	pfi->index = alloc_file_id();
	if(pfi->index == 0)
	{
		log_warning("file id overflow\n");
		return EDB_ID_OVERFLOW;
	}

	*dir_id = pfi->index;

	if((errcode = load_file_insert_cmd(sql, pfi)) != RET_SUCCESS)
	{
		return errcode;
	}

	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}



/*
*description: create directories not exists in a path, save the last directory's id in last_parent_id.
*input param:pfi->buffer storing file information.
*output param:las_parent_id->buffer storing the parent id.
*return: RET_SUCCESS if OK
*/
static error_t create_full_path(sqlite3 *database, file_info_t *pfi, uint32_t *last_parent_id)
{
    char dir_name[NAME_SIZE];
	//char path[PATH_LENGTH];
	char *path = NULL;
	int level = 0;
	int i;
    uint32_t parent_id;
	uint32_t id;
	error_t errcode = RET_SUCCESS;

	if(database == NULL || pfi == NULL || last_parent_id == NULL)
	{
		return EINVAL_ARG;
	}

	//S_STRNCPY(path, pfi->dir, PATH_LENGTH);
	path = pfi->dir;
	DMCLOG_D("path = %s",path);
	if((level = get_file_level(path)) <= ROOT_LEVEL)
	{
		return EINVAL_ARG;
	}
	DMCLOG_D("ROOT_LEVEL = %d,level = %d",ROOT_LEVEL,level);
	char *end = NULL;
	char *tail = path;
	for(i=ROOT_LEVEL+1,parent_id=ROOT_DIR_ID;i<=level;i++)
	{
		if((errcode = get_level_file_name(path, i, dir_name)) != RET_SUCCESS)
		{
			return errcode;
		}
		DMCLOG_D("dir_name = %s",dir_name);
        //sqlite3_str_escape(dir_name, pfi->name_escape, sizeof(pfi->name_escape));
		
        if((errcode = query_file_id(database, parent_id, dir_name, &id)) != RET_SUCCESS)
		{
			return errcode;
		}

		if(id != INVALID_FILE_ID)//dir exist
		{
			parent_id = id;
			continue;
		}
        //not exist, create it
		pfi->parent_id = parent_id;
		end = strstr(tail,dir_name);
		if(end != NULL&&*(end + strlen(dir_name)) == '/')
		{
			*(end + strlen(dir_name)) = 0;
			pfi->isFolder = 1;
			pfi->file_type = 0;
			tail = end + 1;
		}
		pfi->path = path;
		pfi->name = bb_basename(path);
		DMCLOG_D("path = %s,name = %s",path,pfi->name);
		if((errcode = create_dir_node(database, pfi, &parent_id)) != RET_SUCCESS)
		{
			return errcode;
		}
		if(*(end + strlen(dir_name)) == 0&&end + strlen(dir_name) + 1 != NULL)
		{
			*(end + strlen(dir_name)) = '/';
		}
        // modify end.
	}

	*last_parent_id = parent_id;

 	return errcode;
}
/*
*description:insert new file record to file table
*input param:item_info-->file information buffer
*return : RET_SUCCESS if ok.
*/
static error_t dm_file_insert(sqlite3 *database, void *item_info)
{
    file_info_t *pfi = (file_info_t *)item_info;
	
    char *sql = get_db_write_task_sql_buf();
	uint32_t parent_id;
	error_t errcode;
	
    if(database == NULL || pfi == NULL)
    {
        log_warning("file table insert error:invalid parameters\n");
        return EINVAL_ARG;
    }
	//make sure whether parent directory exists
	DMCLOG_D("pfi->path = %s",pfi->path);
	errcode = file_get_parent_id(database, pfi->path, &parent_id);
	if(errcode == EDB_RECORD_NOT_EXIST)
	{
	    //parent  doesn't exist
		file_info_t fi_tmp;
		memset(&fi_tmp, 0, sizeof(file_info_t));
		fi_tmp.isFolder = 1;// the file is folder
		fi_tmp.file_size = 0;
		fi_tmp.modify_time = pfi->modify_time;
		char *tmp = strrchr(pfi->path,SEPERATOR);
		*tmp = '\0';
		fi_tmp.dir = pfi->path;
		//create the parent and ancestor directories.
		if((errcode = create_full_path(database, &fi_tmp, &parent_id))!= RET_SUCCESS)
		{
			log_warning("create full path error:%s\n", pfi->path);
			return errcode;
		}
		*tmp = SEPERATOR;
		DMCLOG_D("create_full_path");
	}
	//insert
	unsigned id;
	DMCLOG_D("parent_id = %u",parent_id);
	char *fullname = (char *)calloc(1,strlen(DOCUMENT_ROOT) + strlen(pfi->path) + 1);
	sprintf(fullname,"%s%s",DOCUMENT_ROOT,pfi->path);
	pfi->attr = dm_get_attr_hide(fullname);
	safe_free(fullname);
	errcode = query_file_id_by_parentid_and_name(database,parent_id,bb_basename(pfi->path),&id);
    if(errcode == EDB_RECORD_NOT_EXIST)
   	{
   		DMCLOG_D("the file is not exist");
		file_info_t file_tmp;
		memset(&file_tmp,0,sizeof(file_info_t));
		memcpy(&file_tmp,pfi,sizeof(file_info_t));
	    file_tmp.parent_id = parent_id;
		file_tmp.index = alloc_file_id();
		file_tmp.path = pfi->path;
		file_tmp.name = bb_basename(pfi->path);
		file_tmp.attr = pfi->attr;
		if((errcode = load_file_insert_cmd(sql,&file_tmp)) != RET_SUCCESS)
		{
			return errcode;
		}

		if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL))
				!= RET_SUCCESS)
		{
		    log_warning("db:insert %s error\n", pfi->path);
	        return errcode;
		}
	}else{
		DMCLOG_D("file uuid :%s",pfi->file_uuid);
		
		update_file_info_by_id(database,id,pfi);
	}
	return errcode;
}

static error_t _dm_scan_file_insert(sqlite3 *database,void *item_info)
{
    file_info_t *pfi = (file_info_t *)item_info;
    char *sql = get_db_write_task_sql_buf();
	error_t errcode = RET_SUCCESS;
    if(database == NULL || pfi == NULL)
    {
        log_warning("file table insert error:invalid parameters\n");
		errcode = EINVAL_ARG;
        goto EXIT;
    }
	
	if((errcode = load_file_insert_cmd(sql, pfi)) != RET_SUCCESS)
	{
		goto EXIT;
	}
	if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL))!= RET_SUCCESS)
	{
	    DMCLOG_E("db:insert %s error", pfi->path);
        goto EXIT;
	}
EXIT:
	return errcode;
}

int db_scan_surplus_files(sqlite3 *database,const char *path,unsigned parent_id,file_list_t *plist)
{
	error_t errcode = RET_SUCCESS;
	file_list_t file_list;
	file_info_t *item,*n;

	if(get_fuser_flag() == AIRDISK_ON_PC)
		return -1;
	memset(&file_list,0,sizeof(file_list_t));
	file_list.parent_id = parent_id;
	dl_list_init(&file_list.head);
	
	if((errcode = get_file_list_by_parent_id(database, &file_list)) != RET_SUCCESS)
	{
		DMCLOG_E("get file list by parent_id error");
		dl_list_for_each_safe(item, n, &file_list.head, file_info_t, next)
		{
		  free_db_fd(&item);
		}   
		return errcode;
	}

	char *full_path = NULL;
    if(file_list.result_cnt > 0)
    {
		dl_list_for_each(item, &(file_list.head), file_info_t, next)
		{
		 // init item member
		 	if(get_fuser_flag() == AIRDISK_ON_PC)
		 	{
				errcode = -1;
				break;
			}
			if(item->isFolder == 1&&item->path != NULL)
			{
				DMCLOG_D("item->path = %s",item->path);
				//the file is folder
				db_scan_surplus_files(database,item->path,item->index,plist);
			}
			full_path = (char *)calloc(1,strlen(DOCUMENT_ROOT) + strlen(path) + strlen(item->name) + 2);
			sprintf(full_path,"%s%s/%s",DOCUMENT_ROOT,path,item->name);
			if(access(full_path,F_OK) != 0)/*  如果不存在将相关信息加入链表*/
			{
				//the file is normal file
				DMCLOG_D("item->name = %s",item->name);
				file_id_t *file_id = (file_id_t *)calloc(1,sizeof(file_id_t));
				file_id->index = item->index;
				strcpy(file_id->file_uuid,item->file_uuid);
				dl_list_add_tail(&plist->sur_head, &file_id->next);
			}
			free(full_path);
			
		}
     }
     dl_list_for_each_safe(item, n, &(file_list.head), file_info_t, next)
     {
         free_db_fd(&item);
     }
    return errcode;
}

static struct file_dnode *_dm_insert(sqlite3 *database,char *fullname,unsigned parent_id,file_list_t *plist)
{
	error_t errcode = RET_SUCCESS;
	struct stat statbuf;
	struct file_dnode *cur;
	cur = xzalloc(sizeof(*cur));
	if (lstat(fullname, &statbuf)) {
		printf(fullname);
		safe_free(cur);
		return NULL;
	}
	
	cur->fullname = fullname;
	cur->dn_mode = statbuf.st_mode;
	cur->name = bb_basename(fullname);
	cur->index = 0;
    errcode = query_file_id_by_parentid_and_name(database,parent_id,cur->name,&cur->index);
	
    if(errcode == EDB_RECORD_NOT_EXIST)
    {
    	file_info_t *file_info = (file_info_t *)calloc(1,sizeof(file_info_t));
		
    	if (S_ISDIR(cur->dn_mode))
		{
			file_info->isFolder = 1;// the file is folder
			file_info->path = strdup(cur->fullname + strlen(DOCUMENT_ROOT));
		}else{
			file_info->isFolder = 0;// the file is normal
			file_info->file_type = db_get_mime_type(cur->fullname,strlen(cur->fullname));//TODO Oliver
		}
		
		cur->index = alloc_file_id();
		
		file_info->name = strdup(cur->name);
    	file_info->create_time = statbuf.st_ctime;
		file_info->modify_time = statbuf.st_mtime;
		file_info->access_time = statbuf.st_atime;
		file_info->parent_id = parent_id;
		file_info->index = cur->index;
		file_info->attr = dm_get_attr_hide(cur->fullname);
		strcpy(file_info->file_uuid,"1234567890");
		file_info->file_size = statbuf.st_size;
		
		dl_list_add_tail(&plist->head, &file_info->next);
		
    }
	return cur;
}

static struct file_dnode **_scan_one_dir(sqlite3 *database,const char *path, unsigned parent_id,unsigned *nfiles_p,file_list_t *plist)
{
	struct file_dnode *dn, *cur, **dnp;
	struct dirent *entry;
	DIR *dir;
	unsigned i, nfiles;
	*nfiles_p = 0;
	dir = warn_opendir(path);
	if (dir == NULL) {
		return NULL;	/* could not open the dir */
	}
	dn = NULL;
	nfiles = 0;
	while ((entry = readdir(dir)) != NULL&&get_fuser_flag() == AIRDISK_OFF_PC) {
		char *fullname;
		/* are we going to list the file- it may be . or .. or a hidden file */
		if (entry->d_name[0] == '.') {
			/*if ((!entry->d_name[1] || (entry->d_name[1] == '.' && !entry->d_name[2]))) 
			{
				continue;
			}*/
			continue;
		}
		fullname = concat_path_file(path, entry->d_name);
		cur = _dm_insert(database,fullname,parent_id,plist);
		if (!cur) {
			free(fullname);
			goto EXIT;
		}
		cur->fname_allocated = 1;
		cur->dn_next = dn;
		dn = cur;
		nfiles++;
	}
	closedir(dir);
	if (dn == NULL)
		return NULL;

	/* now that we know how many files there are
	 * allocate memory for an array to hold dnode pointers
	 */
	*nfiles_p = nfiles;
	dnp = dnalloc(nfiles);
	for (i = 0; /* i < nfiles - detected via !dn below */; i++) {
		dnp[i] = dn;	/* save pointer to node in array */
		dn = dn->dn_next;
		if (!dn)
			break;
	}
    return dnp;
EXIT:
    closedir(dir);
    if (dn == NULL)
        return NULL;
    dnp = dnalloc(nfiles);
    for (i = 0; /* i < nfiles - detected via !dn below */; i++) {
        dnp[i] = dn;	/* save pointer to node in array */
        dn = dn->dn_next;
        if (!dn)
            break;
    }
    dfree(dnp);
    return NULL;
}

static int _dm_scan_and_display_dirs_recur(sqlite3 *database,struct file_dnode **dn,file_list_t *plist)
{
	unsigned nfiles;
	int ret = 0;
	struct file_dnode **subdnp;
	if(get_fuser_flag() == AIRDISK_ON_PC)
		return -1;
	for (; *dn; dn++) {
		subdnp = _scan_one_dir(database,(*dn)->fullname, (*dn)->index,&nfiles,plist);
		if (nfiles > 0) {
			struct file_dnode **dnd;
			unsigned dndirs;
			/* recursive - list the sub-dirs */
			dnd = splitdnarray(subdnp, SPLIT_SUBDIR);
			dndirs = count_dirs(subdnp, SPLIT_SUBDIR);
			if (dndirs > 0) {
				ret = _dm_scan_and_display_dirs_recur(database,dnd,plist);
				/* free the array of dnode pointers to the dirs */
				free(dnd);
			}
			/* free the dnodes and the fullname mem */
			dfree(subdnp);
		}
	}
	return ret;
}

int dm_db_clean_surplus_files(sqlite3 *database,file_list_t *plist,pthread_mutex_t *mutex)
{
	error_t errcode = RET_SUCCESS;
	file_id_t *item;
	/*int now_time = 0;
	int record_time = 0;
	struct  sysinfo info; */
	if(get_fuser_flag() == AIRDISK_ON_PC )
	{
		DMCLOG_E("airdisk on pc");
		return -1;
	}
	pthread_mutex_lock(mutex);
    sqlite3_exec_busy_wait(database, "begin", NULL, NULL);
	
	dl_list_for_each(item, &(plist->sur_head), file_id_t, next)
	{
		/*sysinfo(&info); 
		now_time = info.uptime;
		if(now_time - record_time > 0)
		{
			record_time = now_time + 1;
			sqlite3_exec_busy_wait(database, "commit", NULL, NULL);
			pthread_mutex_unlock(mutex);
			if(get_fuser_flag() == AIRDISK_ON_PC)
			{
				DMCLOG_E("airdisk on pc");
				return 0;
			}
			pthread_mutex_lock(mutex);
    			sqlite3_exec_busy_wait(database, "begin", NULL, NULL);
		}*/
		DMCLOG_D("index = %u",item->index);
		if(get_fuser_flag() == AIRDISK_OFF_PC && (errcode = dm_do_delete_by_id(database,item->index)) != RET_SUCCESS)
        {
            DMCLOG_D("file insert failed");
			errcode = -1;
           	break;
        }
		/*DMCLOG_D("file uuid = %s",item->file_uuid);
		if(get_fuser_flag() == AIRDISK_OFF_PC && (errcode = dm_do_delete_by_uuid(database,item->file_uuid)) != RET_SUCCESS)
        {
            DMCLOG_D("file insert failed");
			errcode = -1;
           	break;
        }*/
		
	}
	sqlite3_exec_busy_wait(database, "commit", NULL, NULL);
	pthread_mutex_unlock(mutex);
	return errcode;
}

int dm_db_insert_new_files(sqlite3 *database,file_list_t *plist,pthread_mutex_t *mutex)
{
	error_t errcode = RET_SUCCESS;
	file_info_t *item;

	int now_time = 0;
	int record_time = 0;
	struct  sysinfo info; 
	if(get_fuser_flag() == AIRDISK_ON_PC )
		return -1;
	pthread_mutex_lock(mutex);
    sqlite3_exec_busy_wait(database, "begin", NULL, NULL);
	
	dl_list_for_each(item, &(plist->head), file_info_t, next)
	{
		sysinfo(&info); 
		now_time = info.uptime;
		if(now_time - record_time > 0)
		{
			record_time = now_time + 1;
			sqlite3_exec_busy_wait(database, "commit", NULL, NULL);
			pthread_mutex_unlock(mutex);
			if(get_fuser_flag() == AIRDISK_ON_PC )
				return -1;
			pthread_mutex_lock(mutex);
    		sqlite3_exec_busy_wait(database, "begin", NULL, NULL);
		}
		if(get_fuser_flag() == AIRDISK_OFF_PC && (errcode = _dm_scan_file_insert(database,item)) != RET_SUCCESS)
        {
            DMCLOG_D("file insert failed");
			errcode = -1;
           	break;
        }
	}
	sqlite3_exec_busy_wait(database, "commit", NULL, NULL);
	pthread_mutex_unlock(mutex);
	return errcode;
}

static error_t dm_file_table_scan(sqlite3 *database, void *path_info)
{
	error_t errcode = RET_SUCCESS;
    scan_data_t *pfi = (scan_data_t *)path_info;
	
    if(database == NULL || pfi == NULL)
    {
        log_warning("file table scanning error:invalid parameters\n");
        return EINVAL_ARG;
    }

	struct stat statbuf;
	if (lstat(pfi->disk_path, &statbuf)) {
		printf("pfi->disk_path = %s\n",pfi->disk_path);
		return EINVAL_ARG;
	}

	struct file_dnode **dnp;
	struct file_dnode dn;
	memset(&dn,0,sizeof(struct file_dnode));
	dn.fullname = pfi->disk_path;
	dnp = dnalloc(1);
	dnp[0] = &dn;
	
	file_list_t *plist;
	file_info_t *item,*n;
	file_id_t *s_item,*s_n;
	plist = (file_list_t *)calloc(1,sizeof(file_list_t));
	dl_list_init(&plist->head);
	dl_list_init(&plist->sur_head);

	unsigned g_id = INVALID_FILE_ID;
	char *disk_path = pfi->disk_path + strlen(DOCUMENT_ROOT);
    errcode = query_dir_file_id(database,disk_path,&g_id);
    if(errcode == EDB_RECORD_NOT_EXIST)
    {
    	file_info_t *file_info = (file_info_t *)calloc(1,sizeof(file_info_t));
    	file_info->path = strdup(disk_path);
		file_info->name = strdup(bb_basename(disk_path));
		file_info->file_size = statbuf.st_size;
		file_info->create_time = statbuf.st_ctime ;
		file_info->modify_time = statbuf.st_mtime;
		file_info->access_time = statbuf.st_atime;
		file_info->isFolder = 1;
        file_info->parent_id = 0;
		file_info->attr = dm_get_attr_hide(pfi->disk_path);
		file_info->index = alloc_file_id();
     	dl_list_add_tail(&plist->head, &file_info->next);

		dn.index = file_info->index;
		if((errcode = _dm_scan_and_display_dirs_recur(database,dnp,plist)) < 0)/*扫描未插入数据库的文件*/
		{
			DMCLOG_D("quit files scanning");
			goto EXIT;
		}
    }else{
    	dn.index = g_id;
		if((errcode = _dm_scan_and_display_dirs_recur(database,dnp,plist)) < 0)/*扫描未插入数据库的文件*/
		{
			DMCLOG_D("quit files scanning");
			goto EXIT;
		}
		
	   	DMCLOG_D("scan drive files finished");

		if((errcode = db_scan_surplus_files(database,disk_path,g_id,plist)) < 0)/*扫描数据库中不存在的文件*/
		{
			DMCLOG_D("quit database files scanning");
			goto EXIT;
		}

		DMCLOG_D("scan database files finished");

		if((errcode = dm_db_clean_surplus_files(database,plist,pfi->mutex)) < 0)/*删除数据库中多余的数据*/
		{
			DMCLOG_D("quit files cleanning");
			goto EXIT;
		}

		DMCLOG_D("clean database files finished");
	}
	if((errcode = dm_db_insert_new_files(database,plist,pfi->mutex)) < 0)/*将磁盘的文件插入数据库*/
	{
		DMCLOG_D("quit files inserting");
		goto EXIT;
	}
EXIT:
	dl_list_for_each_safe(item, n, &(plist->head), file_info_t, next)
	{
		free_db_fd(&item);
	}
	dl_list_for_each_safe(s_item, s_n, &(plist->sur_head), file_id_t, next)
	{
		safe_free(s_item);
	}
	safe_free(plist);
	DMCLOG_D("file process finished");
	return 0;
}

static error_t file_table_rename_by_id(sqlite3 *database, unsigned id,int isfolder,
									const char *des_path)
{
	ENTER_FUNC();
	char *sql = get_db_write_task_sql_buf();
	error_t errcode = RET_SUCCESS;
	char *path_escape = db_path_escape(des_path);
	if(path_escape == NULL)
	{
		return EINVAL_ARG;
	}
	char *fullname = (char *)calloc(1,strlen(DOCUMENT_ROOT) + strlen(des_path) + 1);
	sprintf(fullname,"%s%s",DOCUMENT_ROOT,des_path);
	if(isfolder == 1)
		sprintf(sql,"UPDATE %s SET NAME='%s',PATH='%s' MEDIA_INFO_INDEX='%d' WHERE ID=%d", FILE_TABLE_NAME, bb_basename(path_escape),path_escape,dm_get_attr_hide(fullname),id);
	else
		sprintf(sql,"UPDATE %s SET NAME='%s' ,MEDIA_INFO_INDEX='%d' WHERE ID=%d", FILE_TABLE_NAME, bb_basename(path_escape),dm_get_attr_hide(fullname),id);
	safe_free(fullname);
	safe_free(path_escape);
	if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL))!= RET_SUCCESS)
	{
		EXIT_FUNC();
		return errcode;	
	}
	EXIT_FUNC();
	return errcode;
}

int update_dir_path(sqlite3 *database,unsigned parent_id,char *new_path)
{
	error_t errcode = RET_SUCCESS;
	file_list_t file_list;
	file_info_t *item,*n;
	char *child_path = NULL;
	memset(&file_list,0,sizeof(file_list_t));
	file_list.parent_id = parent_id;
	if(new_path == NULL)
	{
		DMCLOG_E("para is error");
		return ENULL_POINT;
	}
	DMCLOG_D("new_path = %s",new_path);
	dl_list_init(&file_list.head);
	if((errcode = get_file_list_by_parent_id(database, &file_list)) != RET_SUCCESS)
	{
		DMCLOG_E("get file list by parent_id error");
		dl_list_for_each_safe(item, n, &file_list.head, file_info_t, next)
		{
		  free_db_fd(&item);
		}   
		return errcode;
	}
	
	DMCLOG_D("result_cnt(%u)", file_list.result_cnt);
    if(file_list.result_cnt > 0)
    {
		dl_list_for_each(item, &(file_list.head), file_info_t, next)
		{
		 // init item member
			if(item->isFolder == 1)
			{
				//the file is folder
				child_path = (char *)calloc(1,strlen(new_path) + strlen(item->name) + 2);
				sprintf(child_path,"%s/%s",new_path,item->name);
				DMCLOG_D("child_path = %s",child_path);
				update_dir_path(database,item->index,child_path);
				file_table_rename_by_id(database, item->index,item->isFolder,child_path);
				safe_free(child_path);
			}
		}
     }
	dl_list_for_each_safe(item, n, &(file_list.head), file_info_t, next)
	{
		free_db_fd(&item);
	}
    return errcode;
}

//sqlite exec callback to get file id
static int get_id_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
    uint32_t *id = (uint32_t *)data;

	if(FieldValue[0] != NULL)
	{
		*id = atol(FieldValue[0]);
	}
	
	return 0;
}


//rename file record in file table
static error_t file_table_rename(sqlite3 *database, char *src_path,char *des_path)
{
	ENTER_FUNC();
	error_t errcode = RET_SUCCESS;
	int parent_id;
	if(database == NULL || src_path == NULL || des_path == NULL)
	{
		return EINVAL_ARG;
	}

	errcode = file_get_parent_id(database,src_path,&parent_id);
	if(errcode != RET_SUCCESS)
	{
		DMCLOG_D("rename file doesn't exist,src_path=%s",src_path);
		return EDB_RECORD_NOT_EXIST;
	}

	file_info_t file_info;
	memset(&file_info,0,sizeof(file_info_t));
	char *name = bb_basename(src_path);
    errcode = query_file_info_by_parentid_and_name(database,parent_id,name,&file_info);
	if(errcode == EDB_RECORD_NOT_EXIST)
	{
		DMCLOG_D("rename file doesn't exist,src_path=%s",src_path);
		return EDB_RECORD_NOT_EXIST;
	}
	errcode = file_table_rename_by_id(database, file_info.index,file_info.isFolder,des_path);
	if(errcode != RET_SUCCESS)
	{
		return errcode;
	}
	errcode = update_dir_path(database,file_info.index,des_path);
	if(errcode != RET_SUCCESS)
	{
		return errcode;
	}
	return errcode;
}

static error_t file_table_update(sqlite3 *database, void *update_info)
{
	ENTER_FUNC();
	file_update_t *file_update = (file_update_t *)update_info;
	unsigned parent_id = 0;
	error_t errcode = RET_SUCCESS;
	
	if(database == NULL || update_info == NULL)
	{
		DMCLOG_E("para is null");
		return ENULL_POINT;
	}
	
	if(file_update->cmd == 0)
	{
		return RET_SUCCESS;
	}
	
	char *tmp = strrchr(file_update->file_info.path,'/');
	if(tmp == NULL)
	{
		DMCLOG_E("the path is not valid");
		return EDB_RECORD_NOT_EXIST;
	}
	if(tmp == file_update->file_info.path)
	{
		parent_id = 0;
	}else{
		errcode = file_get_parent_id(database,file_update->file_info.path,&parent_id);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_D("rename file doesn't exist,src_path=%s",file_update->file_info.path);
			return EDB_RECORD_NOT_EXIST;
		}
	}
	char *name = bb_basename(file_update->file_info.path);
	DMCLOG_D("parent_id = %u",parent_id);
	errcode = query_file_id_by_parentid_and_name(database,parent_id,name,&file_update->file_info.index);
	if(errcode == EDB_RECORD_NOT_EXIST)
	{
		DMCLOG_D("query file doesn't exist,src_path=%s",file_update->file_info.path);
		return EDB_RECORD_NOT_EXIST;
	}
	
	DMCLOG_D("index = %u,attr = %d",file_update->file_info.index,file_update->file_info.attr);
	
	if(file_update->cmd & FILE_TABLE_UPDATE_MODIFY_TIME)
	{
		//TODO
	}

	if(file_update->cmd & FILE_TABLE_UPDATE_HIDE)
	{
		errcode = dm_do_hide(database,file_update->file_info.index,file_update->file_info.attr);
		if(errcode!= RET_SUCCESS)
		{
			return errcode;
		}
	}

	if(file_update->cmd & FILE_TABLE_UPDATE_ALBUM_HIDE)
	{
		errcode = dm_do_album_hide(database,file_update->file_info.index,file_update->file_info.attr);
		if(errcode!= RET_SUCCESS)
		{
			return errcode;
		}
	}
	
	if(file_update->cmd & FILE_TABLE_UPDATE_SIZE)
	{
		//TODO
	}
	EXIT_FUNC();
	return errcode;
}

extern error_t get_file_path(sqlite3 *database, uint32_t id, file_info_t *pfi);

static error_t file_table_reindex(sqlite3 *database)
{
	char *sql = get_db_write_task_sql_buf();

	if(database == NULL)
	{
		return ENULL_POINT;
	}

	S_SNPRINTF(sql, SQL_CMD_WRITE_BUF_SIZE, "REINDEX %s", FILE_TABLE_NAME);

	return  sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}


extern error_t file_table_query(sqlite3 *database, QueryApproach approach, void *buf);

//register database file table's operation functions, these functions are the entries to file table.
void register_file_table_ops(void)
{
    memset(&g_file_table, 0, sizeof(g_file_table));
	g_file_table.ops.scan			 = dm_file_table_scan;
    g_file_table.ops.insert          = dm_file_insert;
	g_file_table.ops.dm_delete  	 = file_table_delete;
	g_file_table.ops.rename          = file_table_rename;
	g_file_table.ops.query           = file_table_query;
	g_file_table.ops.active_update   = file_table_update;  
	g_file_table.ops.reindex_table   = file_table_reindex;
}


//file table init.
error_t  file_table_init(char *g_database)
{   
    sqlite3 *database;
	error_t errcode = RET_SUCCESS;

	register_file_table_ops();
	DMCLOG_D("DATABASE = %s",g_database);
	errcode = sqlite3_open(g_database, &database);
	if(errcode != SQLITE_OK)
	{
		DMCLOG_D("cannot open database in file_table_init");
		return errcode;
	}

	//find the max file id in file table.
	if((errcode = sqlite3_exec_busy_wait(database,"SELECT MAX(ID) FROM file_table",
		     get_id_callback, &g_file_id)) != SQLITE_OK)
	{
		DMCLOG_D("sqlite3_exec_busy_wait error");
        sqlite3_close(database);
        return errcode;
	}
    sqlite3_close(database);	
	return errcode;
}
