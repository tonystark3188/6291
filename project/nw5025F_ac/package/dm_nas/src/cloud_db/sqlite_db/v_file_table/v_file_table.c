/*
 * =============================================================================
 *
 *       Filename: v_file_table.c
 *
 *    Description: v_file table related data structure definition.
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
db_table_t g_v_file_table;

const char *g_sort[] = {"ORDER BY MODIFY_TIME ASC",
				   		"ORDER BY MODIFY_TIME DESC",
				   		"ORDER BY SIZE ASC",
				   		"ORDER BY SIZE DESC",
				   		"ORDER BY GB_NAME ASC",
				  	    "ORDER BY GB_NAME DESC"};

 //sqlite exec callback to get file id
 static int get_v_file_id_callback(void *data, int nFields, char **FieldValue, char **FieldName)
 {
	 int *pid = (int *)data;
 
	 if(FieldValue[0] != NULL)
	 {
		 *pid = strtoul(FieldValue[0], NULL, 10);
	 }
	 return 0;
 }

  //sqlite exec callback to get file path
 static int get_v_file_path_callback(void *data, int nFields, char **FieldValue, char **FieldName)
 {
	 char *path = (char *)data;
	if(FieldValue[0] != NULL)
	{
		S_STRNCPY(path, FieldValue[0], MAX_FILE_PATH_LEN);
	 	DMCLOG_D("pfile->path = %s",path);
	}
	return 0;
 }
 
 //eg.	 if we have a file with path '/root/a/b/c', the level of c is 4.
 static int get_file_level( char *path)
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

//get  appointed level's name in a path
//eg.  the name of third level of '/root/aa/bb/cc'  is 'bb'
static error_t get_level_file_name( char *path, int level, char *name)
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

static char *db_path_escape(char *path)
{
	int i;
	error_t errcode = RET_SUCCESS;
	char name[MAX_FILE_NAME_LEN];
    char name_str_escape[MAX_FILE_NAME_LEN + 0x10];
	if(path == NULL)
	{
		return NULL;
	}
	char *path_escape = (char *)calloc(1,strlen(path) + 0x10);
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

error_t query_dir_file_id(sqlite3 *database,char *path,unsigned *g_id,char *bucket_name)
{
	ENTER_FUNC();
    char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;
    *g_id = INVALID_FILE_ID;
	if(database == NULL||path == NULL)
	{
		DMCLOG_E("the para is null");
		return EINVAL_ARG;
	}
	char *path_escape = db_path_escape(path);
	if(path_escape == NULL)
	{
		DMCLOG_E("the para is null");
		return EINVAL_ARG;
	}
	sprintf(sql, "SELECT * FROM %s where FILE_PATH = '%s'",bucket_name, path_escape);
	safe_free(path_escape);
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_v_file_id_callback, g_id))
			!= RET_SUCCESS)
	{
        DMCLOG_E("exit error");
		return errcode;
	}

    if(*g_id == INVALID_FILE_ID)
    {
        DMCLOG_E("EDB_RECORD_NOT_EXIST");
		return EDB_RECORD_NOT_EXIST;
	}

	DMCLOG_D("g_id = %d",*g_id);
	EXIT_FUNC();
    return errcode;	
}

static error_t load_v_file_insert_cmd(sqlite3 *database,v_file_info_t *pfile,char *bucket_name)
{
	ENTER_FUNC();
	int n;
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	if( pfile == NULL||!*bucket_name)
	{
		DMCLOG_E("para is null");
		return ENULL_POINT;
	}
	char *path_escape = db_path_escape(pfile->path);
	char *name = (char *)bb_basename(path_escape);
	char out[MAX_FILE_NAME_LEN] = {0};
	xxGetPinyin(name, pfile->gb_name, out);
	
	DMCLOG_D("name = %s,gb_name = %s,out = %s",name,pfile->gb_name,out);
	if(pfile->isDir == 1)
	{
		snprintf(sql, SQL_CMD_WRITE_BUF_SIZE, "INSERT INTO %s(FILE_NAME,FILE_PATH,REAL_FILE_ID,PARENT_ID,IS_DIR,TYPE,CREATE_TIME,MODIFY_TIME,ACCESS_TIME,SIZE,GB_NAME,UUID)"\
			"VALUES('%s','%s','%d','%d','%d','%d','%ld','%ld','%ld','%lld','%s','%s')",bucket_name,
			name, path_escape, pfile->real_file_id,pfile->parent_id,pfile->isDir,pfile->type,pfile->ctime,pfile->mtime,pfile->atime,(long long)pfile->size,pfile->gb_name,pfile->uuid);
	}else{
		snprintf(sql, SQL_CMD_WRITE_BUF_SIZE, "INSERT INTO %s(FILE_NAME,REAL_FILE_ID,PARENT_ID,IS_DIR,TYPE,CREATE_TIME,MODIFY_TIME,ACCESS_TIME,SIZE,GB_NAME,UUID)"\
			"VALUES('%s','%d','%d','%d','%d','%ld','%ld','%ld','%lld','%s','%s')",bucket_name,
			name, pfile->real_file_id,pfile->parent_id,pfile->isDir,pfile->type,pfile->ctime,pfile->mtime,pfile->atime,(long long)pfile->size,pfile->gb_name,pfile->uuid);
	}
	safe_free(path_escape);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}


static error_t get_parent_id_by_path(sqlite3 *database,char *path,int *parent_id,char *bucket_name)
{
	ENTER_FUNC();
	error_t errcode = RET_SUCCESS;
	char *tmp = strrchr(path,'/');
	if(tmp == NULL)
	{
		DMCLOG_E("the path is not valid");
		return EDB_RECORD_NOT_EXIST;
	}
	if(tmp == path)
	{
		*parent_id = 0;
	}else{
		*tmp = '\0';
		errcode = query_dir_file_id(database,path,parent_id,bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_D("query file doesn't exist,src_path=%s",path);
			*tmp = '/';
			return EDB_RECORD_NOT_EXIST;
		}
		*tmp = '/';
	}
	EXIT_FUNC();
	return errcode;
}

static error_t get_max_id(sqlite3 *database,uint32_t *max_id,char *bucket_name)
{
	error_t errcode;
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql,"SELECT MAX(FILE_ID) FROM %s",bucket_name);
	if((errcode = sqlite3_exec_busy_wait(database,sql,get_v_file_id_callback, max_id)) != SQLITE_OK)
	{
		DMCLOG_D("sqlite3_exec_busy_wait error");
        sqlite3_close(database);
        return errcode;
	}
	return errcode;
}


/*
*description:create a dir record when we insert a new path
*input param:pfi->dir information. 
*output param:dir_id->directory record id.
*return: RET_SUCCESS if ok
*/
static error_t create_dir_node(sqlite3 *database, v_file_info_t *pfi, uint32_t *dir_id,char *bucket_name)
{
	
	error_t errcode;
	pfi->isDir = 1;
	if((errcode = load_v_file_insert_cmd(database, pfi,bucket_name)) != RET_SUCCESS)
	{
		return errcode;
	}

	if((errcode = get_max_id(database,dir_id,bucket_name)) != RET_SUCCESS)
	{
		return errcode;
	}
	return errcode;
}
static error_t v_file_name_exist(sqlite3 *database,const char *file_name,int parent_id,int *g_id,char *bucket_name)
{
    char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;
    char name_str_escape[MAX_FILE_NAME_LEN + 0x10];
   	*g_id = INVALID_V_FILE_ID;
	if(database == NULL)
	{
		return EINVAL_ARG;
	}
	sqlite3_str_escape(file_name, name_str_escape, sizeof(name_str_escape));
	sprintf(sql, "SELECT * FROM %s where FILE_NAME = '%s' AND PARENT_ID = '%d'",
		bucket_name, name_str_escape,parent_id);
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_v_file_id_callback, g_id))
			!= RET_SUCCESS)
	{
        DMCLOG_E("exit error");
		return errcode;
	}
    if(*g_id == INVALID_V_FILE_ID)
    {
        DMCLOG_D("EDB_RECORD_NOT_EXIST");
		return EDB_RECORD_NOT_EXIST;
	}
    return errcode;	
}


/*
*description: create directories not exists in a path, save the last directory's id in last_parent_id.
*input param:pfi->buffer storing file information.
*output param:las_parent_id->buffer storing the parent id.
*return: RET_SUCCESS if OK
*/
static error_t create_full_path(sqlite3 *database, v_file_info_t *pfi, int *last_parent_id,char *bucket_name)
{
    char dir_name[MAX_FILE_NAME_LEN];
	int level = 0;
	int i;
    uint32_t parent_id;
	uint32_t id;
	error_t errcode = RET_SUCCESS;

	if(database == NULL || pfi == NULL || last_parent_id == NULL)
	{
		return EINVAL_ARG;
	}

	char *path = pfi->path;
	DMCLOG_D("path = %s",pfi->path);
	if((level = get_file_level(path)) <= ROOT_LEVEL)
	{
		DMCLOG_E("level = %d",level);
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
	
		errcode = v_file_name_exist(database,dir_name,parent_id,&id,bucket_name);
		if(errcode != EDB_RECORD_NOT_EXIST)//dir exist
		{
			parent_id = id;
			continue;
		}

		pfi->parent_id = parent_id;
		end = strstr(tail,dir_name);
		if(end != NULL&&*(end + strlen(dir_name)) == '/')
		{
			*(end + strlen(dir_name)) = 0;
			tail = end + 1;
		}
		char *name = (char *)bb_basename((char *)path);
		strcpy(pfi->name,name);
		DMCLOG_D("path = %s,name = %s",path,pfi->name);
		if((errcode = create_dir_node(database, pfi, &parent_id,bucket_name)) != RET_SUCCESS)
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


//get vfile information from sqlite query result
static void load_v_file_info(char **FieldName, char **FieldValue, int nFields, v_file_info_t *pfile)
{
	if(FieldValue[0] != NULL)
    	pfile->id = strtoul(FieldValue[0], NULL, 10);
	
	if(FieldValue[1] != NULL)
	{
		S_STRNCPY(pfile->name, FieldValue[1], MAX_FILE_NAME_LEN);
		DMCLOG_D("name = %s",pfile->name);
	}
	#if 0
	if(FieldValue[2] != NULL)
		S_STRNCPY(pfile->path, FieldValue[2], MAX_FILE_PATH_LEN);
	DMCLOG_D("pfile->path = %s",pfile->path);
	#endif
	if(FieldValue[3] != NULL)
    	pfile->real_file_id = strtoul(FieldValue[3], NULL, 10);

	if(FieldValue[4] != NULL)
    	pfile->parent_id = strtoul(FieldValue[4], NULL, 10);

	if(FieldValue[5] != NULL)
    	pfile->isDir = atoi(FieldValue[5]);

	if(FieldValue[6] != NULL)
    	pfile->type = atoi(FieldValue[6]);
	
	if(FieldValue[7] != NULL)
    	pfile->ctime= strtoul(FieldValue[7], NULL, 10);
	if(FieldValue[8] != NULL)
    	pfile->mtime= strtoul(FieldValue[8], NULL, 10);
	if(FieldValue[9] != NULL)
    	pfile->atime= strtoul(FieldValue[9], NULL, 10);

	if(FieldValue[10] != NULL)
    	pfile->size = strtoul(FieldValue[10], NULL, 10);

	if(FieldValue[11] != NULL)
	{
		S_STRNCPY(pfile->gb_name, FieldValue[11], MAX_FILE_NAME_LEN);
		DMCLOG_D("gb_name = %s",pfile->gb_name);
	}
	
	if(FieldValue[12] != NULL)
    	S_STRNCPY(pfile->uuid, FieldValue[12], MAX_FILE_UUID_LEN);
	
}

//get vfile information from sqlite query all result
static void load_all_v_file_info(char **FieldName, char **FieldValue, int nFields, v_file_info_t *pfile)
{
	if(FieldValue[0] != NULL)
    	pfile->parent_id= strtoul(FieldValue[0], NULL, 10);
	
	if(FieldValue[1] != NULL)
    	pfile->isDir = atoi(FieldValue[1]);

	if(FieldValue[2] != NULL)
	{
		pfile->type = atoi(FieldValue[2]);
	}
    	
	if(FieldValue[3] != NULL)
	{
		S_STRNCPY(pfile->name, FieldValue[3], MAX_FILE_NAME_LEN);
	}

	if(FieldValue[4] != NULL)
    	pfile->size = strtoul(FieldValue[4], NULL, 10);

	if(FieldValue[5] != NULL)
    	pfile->mtime = strtoul(FieldValue[5], NULL, 10);
	if(FieldValue[6] != NULL)
	{
		S_STRNCPY(pfile->uuid, FieldValue[6], MAX_FILE_UUID_LEN);
	}
}



static int get_v_file_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	v_file_info_t *pfile = (v_file_info_t *)data;

	load_v_file_info(FieldName, FieldValue, nFields,pfile);	

	return 0;
}

static error_t load_v_file_path_cmd(sqlite3 *database, int id, char *path,char *bucket_name)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;

	sprintf(sql,"SELECT FILE_PATH FROM '%s' WHERE FILE_ID='%d'",bucket_name, id);

	if((errcode = sqlite3_exec_busy_wait(database, sql, get_v_file_path_callback, path))
			!= RET_SUCCESS)
	{
		return errcode;
	}

	if(!*path)
	{
		return EDB_RECORD_NOT_EXIST;
	}
	return errcode;
}


static int get_v_file_list_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	v_file_list_t *plist = (v_file_list_t *)data;
	v_file_info_t *pfi = NULL;
	error_t errcode = RET_SUCCESS;
	pfi = (v_file_info_t *)calloc(1,sizeof(v_file_info_t));
	
	assert(pfi != NULL);
	load_all_v_file_info(FieldName, FieldValue, nFields, pfi);
	
	if(plist->parent_id != pfi->parent_id)
	{
		plist->parent_id = pfi->parent_id;
		memset(plist->path,0,MAX_FILE_PATH_LEN);
		if((errcode = load_v_file_path_cmd(plist->database, plist->parent_id, plist->path,plist->bucket_name)) == RET_SUCCESS)
		{
			if(*plist->path)
			{
				if(!*(plist->path + strlen(plist->bucket_name) + 2))
				{
					sprintf(pfi->path,"/%s",pfi->name);
				}else{
					sprintf(pfi->path,"%s/%s",plist->path + strlen(plist->bucket_name) + 2,pfi->name);
				}
				
			}
		}
	}else{
		if(!*(plist->path + strlen(plist->bucket_name) + 2))
		{
			sprintf(pfi->path,"/%s",pfi->name);
		}else{
			sprintf(pfi->path,"%s/%s",plist->path + strlen(plist->bucket_name) + 2,pfi->name);
		}
	}
	dl_list_add_tail(&plist->head, &pfi->next);
	plist->total++;
	return 0;
}

static int get_v_file_list_by_path_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	v_file_list_t *plist = (v_file_list_t *)data;
	error_t errcode = RET_SUCCESS;
	v_file_info_t *pfi = (v_file_info_t *)calloc(1,sizeof(v_file_info_t));
	assert(pfi != NULL);
	load_v_file_info(FieldName, FieldValue, nFields, pfi);
	dl_list_add_tail(&plist->head, &pfi->next);
	plist->total++;
	return 0;
}


static error_t generate_get_list_sql_cmd(sqlite3 *database, char *sql, 
									 v_file_list_t *file_list,char *bucket_name)
{
	char file_type_buf[64] = {0};
	char list_type_buf[64] = {0};
	char parent_limit_buf[1024] = {0};
	char tail[64] = {0};
	
	switch(file_list->file_type)
	{
		case ALL: 
		{
			break;
		}
		case IMAGE:
		case AUDIO:
		case VIDEO:
		case DOCU:
		{
			sprintf(file_type_buf, "%s.TYPE=%d ", bucket_name,file_list->file_type);
			break;
		}
		default:            break;
	}	

	if(*file_list->path)
	{
		int parent_id;
		error_t errcode = query_dir_file_id(database,file_list->path,&parent_id,bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_D("query file doesn't exist,src_path=%s",file_list->path);
			return EDB_RECORD_NOT_EXIST;
		}
		DMCLOG_D("parent_id = %d",parent_id);
		if(file_list->file_type != 0)
			sprintf(parent_limit_buf, "AND %s.PARENT_ID='%d' ",bucket_name,parent_id);
		else
			sprintf(parent_limit_buf, "%s.PARENT_ID='%d' ",bucket_name,parent_id);
	}
	
	if(file_list->len > 0)
	{
		sprintf(tail,"LIMIT %u OFFSET %u", file_list->len, file_list->startIndex);
	}
	sprintf(sql, "SELECT \
		PARENT_ID,\
		IS_DIR,\
		TYPE,\
		FILE_NAME,\
		SIZE,\
		MODIFY_TIME,\
		UUID\
	FROM %s WHERE %s %s %s %s",\
	bucket_name,file_type_buf, parent_limit_buf,g_sort[file_list->sortType],tail);
	return RET_SUCCESS;
}

static error_t load_v_file_update_real_id_cmd(sqlite3 *database,v_file_info_t *pfile,char *bucket_name)
{
	int n;
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	char name_str_escape[MAX_FILE_NAME_LEN + 0x10];
	if(sql == NULL || pfile == NULL)
	{
		return ENULL_POINT;
	}
	
	n = snprintf(sql, SQL_CMD_WRITE_BUF_SIZE, "UPDATE %s SET REAL_FILE_ID='%d' WHERE FILE_ID='%d'",bucket_name,
		pfile->real_file_id,pfile->id);

	if(n >= SQL_CMD_WRITE_BUF_SIZE)
	{
		return EOUTBOUND;
	}
	
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

static error_t load_v_file_delete_by_id_cmd(
	sqlite3 *database,
	int isDir,
	int id,
	int real_id,
	real_remove remove,
	char *bucket_name)
{
	error_t errcode = RET_SUCCESS;
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "DELETE FROM %s WHERE FILE_ID ='%d'", bucket_name, id);
	if(isDir == 1)
	{
		return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
	}
	file_info_t pfile;
	memset(&pfile,0,sizeof(file_info_t));
	pfile.real_file_id = real_id;
	errcode = load_file_info_cmd(database, &pfile);
	if(errcode != RET_SUCCESS)
	{
		DMCLOG_E("load file info error");
		return errcode;
	}
	if(pfile.link > 1)
	{
		errcode = load_file_asc_link_cmd(database,real_id,pfile.link);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("load file update link error");
			return errcode;
		}
	}else{
		errcode = load_file_delete_cmd(database,real_id);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("load file delete link error");
			return errcode;
		}
		errcode = remove(pfile.real_path);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("delete %s error[%d]",pfile.real_path,errno);
			return errcode;
		}
	}
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}
static error_t load_v_file_list_cmd(sqlite3 *database, v_file_list_t *plist,char *bucket_name)
{
	error_t errcode = RET_SUCCESS;
	char sql[SQL_CMD_QUERY_BUF_SIZE] = {0};
	errcode = generate_get_list_sql_cmd(database, sql, plist,bucket_name);
    if(errcode == EDB_EMPTY_LIST)
    {
        DMCLOG_E("get empty list");
        return RET_SUCCESS;
    }
	plist->database = database;
	return sqlite3_exec_busy_wait(database, sql, get_v_file_list_callback, plist);
}
static error_t get_file_list_by_parent_id(sqlite3 *database, v_file_list_t * v_file_list,char *bucket_name)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE] = {0};
	error_t errcode;
	sprintf(sql, "SELECT * FROM '%s' WHERE PARENT_ID='%d'", bucket_name,v_file_list->parent_id);
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_v_file_list_by_path_callback, v_file_list))
			!= RET_SUCCESS)
	{	
		return errcode;
	}
	return errcode;
}

static error_t load_v_file_search_list_cmd(sqlite3 *database,int parent_id, char *search_str,struct dl_list *head,char *bucket_name)
{
	error_t errcode = RET_SUCCESS;
	v_file_list_t file_list;
	v_file_info_t *item,*n;
	memset(&file_list,0,sizeof(v_file_list_t));
	file_list.parent_id = parent_id;
	dl_list_init(&file_list.head);
	if((errcode = get_file_list_by_parent_id(database, &file_list,bucket_name)) != RET_SUCCESS)
	{
		DMCLOG_E("get file list by parent_id error");
		dl_list_for_each_safe(item, n, &file_list.head, v_file_info_t, next)
		{
		  	safe_free(item);
		}   
		return errcode;
	}
	
	DMCLOG_D("result_cnt(%d)", file_list.total);
    if(file_list.total > 0)
    {
		dl_list_for_each(item, &(file_list.head), v_file_info_t, next)
		{
			if(item->isDir == 1)
			{
				//the file is folder
				errcode = load_v_file_search_list_cmd(database,item->id,search_str,head,bucket_name);
				if(errcode != RET_SUCCESS)
				{
					DMCLOG_E("delete normal file error");
					break;
				}
			}
			if(strstr(item->name,search_str))
			{
				DMCLOG_D("%s is match",item->name);
				v_file_info_t *pfi = (v_file_info_t *)calloc(1,sizeof(v_file_info_t));
				memcpy(pfi,item,sizeof(v_file_info_t));
				dl_list_add_tail(head, &pfi->next);
			}
		}
     }
     dl_list_for_each_safe(item, n, &(file_list.head), v_file_info_t, next)
     {
         safe_free(item);
     }
	return errcode;
}


static error_t query_file_info_by_parentid_and_name(sqlite3 *database,v_file_info_t *pfi,char *bucket_name)
{
    char sql[SQL_CMD_QUERY_BUF_SIZE];
	pfi->id = INVALID_FILE_ID;
	error_t errcode = RET_SUCCESS;
    char name_str_escape[MAX_FILE_NAME_LEN + 0x10];
	if(database == NULL)
	{
		return EINVAL_ARG;
	}
	DMCLOG_D("pfi->path = %s",pfi->path);
	char *name = (char *)bb_basename(pfi->path);
	DMCLOG_D("name = %s",name);
	sqlite3_str_escape(name, name_str_escape, sizeof(name_str_escape));
	sprintf(sql, "SELECT * FROM %s where FILE_NAME = '%s' AND PARENT_ID = %d",
		bucket_name, name_str_escape,pfi->parent_id);
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_v_file_callback, pfi))
			!= RET_SUCCESS)
	{
        DMCLOG_E("exit error");
		return errcode;
	}
    if(pfi->id == INVALID_FILE_ID)
    {
        DMCLOG_E("EDB_RECORD_NOT_EXIST");
		return EDB_RECORD_NOT_EXIST;
	}
    return errcode;	
}

static error_t query_file_info_by_id(sqlite3 *database,v_file_info_t *pfi,char *bucket_name)
{
    char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;
	if(database == NULL)
	{
		return EINVAL_ARG;
	}
	sprintf(sql, "SELECT * FROM %s where FILE_ID = %d",bucket_name,pfi->id);
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_v_file_callback, pfi))
			!= RET_SUCCESS)
	{
        DMCLOG_E("exit error");
		return errcode;
	}
    return errcode;	
}

error_t load_v_file_cmd(sqlite3 *database, v_file_info_t *v_file_info,char *bucket_name)
{
	error_t errcode = RET_SUCCESS;
	
	if(database == NULL)
	{
		return EINVAL_ARG;
	}
	
	errcode = get_parent_id_by_path(database,v_file_info->path,&v_file_info->parent_id,bucket_name);
	if(errcode == EDB_RECORD_NOT_EXIST)//file not exist, go ahead
	{
		DMCLOG_E("%s is not exist",v_file_info->path);
		return errcode;
	}

	return query_file_info_by_parentid_and_name(database,v_file_info,bucket_name);
}

static error_t load_v_uuid_exist_cmd(sqlite3 *database,char *uuid,char *bucket_name)
{
    char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode = RET_SUCCESS;
   	int g_id = INVALID_FILE_ID;
	if(database == NULL)
	{
		return EINVAL_ARG;
	}
	sprintf(sql, "SELECT * FROM %s where UUID = '%s'",
		bucket_name, uuid);
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_v_file_id_callback, &g_id))
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



/**
 * inset new file
 */
error_t v_file_table_insert(sqlite3 *database,void *data)
{
	error_t errcode = RET_SUCCESS;
	char *name;
	if(database == NULL || data == NULL)
	{
		return ENULL_POINT;
	}
	v_file_insert_t *pfile_insert = (v_file_insert_t *)data;

	if(pfile_insert->cmd == V_FILE_TABLE_INSERT_INFO)
	{
		v_file_info_t *v_file_info = &pfile_insert->v_file_info;

		DMCLOG_D("v_file_info->path = %s",v_file_info->path);
		if(v_file_info->isDir != 1)
		{
			name = (char *)bb_basename(v_file_info->real_path);
			strcpy(v_file_info->real_name,name);
			// 2 :insert to file table whether or not real id is exist
			if((errcode = load_file_insert_cmd(database,v_file_info)) != RET_SUCCESS)
			{
				DMCLOG_E("load file insert error");
				return errcode;
			}

			// 3:get real_file_id
			if((errcode = load_max_real_id(database,&v_file_info->real_file_id)) != RET_SUCCESS)
			{
				DMCLOG_E("get max file id error");
				return errcode;
			}

			DMCLOG_D("v_file_info->real_file_id = %d",v_file_info->real_file_id);
		}
		
		errcode = get_parent_id_by_path(database,v_file_info->path,&v_file_info->parent_id,pfile_insert->bucket_name);
		if(errcode == EDB_RECORD_NOT_EXIST)//file not exist, go ahead
		{
			DMCLOG_E("%s is not exist",v_file_info->path);
			//parent  doesn't exist
			v_file_info_t fi_tmp;
			memset(&fi_tmp,0,sizeof(file_info_t));
			char *tmp = strrchr(v_file_info->path,SEPERATOR);
			*tmp = '\0';
			strcpy(fi_tmp.path,v_file_info->path);
			//create the parent and ancestor directories.
			if((errcode = create_full_path(database, &fi_tmp, &v_file_info->parent_id,pfile_insert->bucket_name))!= RET_SUCCESS)
			{
				DMCLOG_E("create full path error:%s\n", v_file_info->path);
				*tmp = SEPERATOR;
				return errcode;
			}
			*tmp = SEPERATOR;
			DMCLOG_D("create_full_path");
		}
		DMCLOG_D("path = %s",v_file_info->path);
		name = (char *)bb_basename(v_file_info->path);
		errcode = v_file_name_exist(database,name ,v_file_info->parent_id,&v_file_info->id,pfile_insert->bucket_name);
		if(errcode == EDB_RECORD_NOT_EXIST)
		{	
			//if the file is not exist in v file table,insert to v file table
			errcode = load_v_file_insert_cmd(database,v_file_info,pfile_insert->bucket_name);
		}else{
			// if the file is exist in v file table,you need update real id
			if(v_file_info->isDir == 1)
			{
				return RET_SUCCESS;
			}
			errcode = load_v_file_update_real_id_cmd(database,v_file_info,pfile_insert->bucket_name);
		}
	}
	return errcode;
}




int load_v_file_delete_cmd(sqlite3 *database,int parent_id,real_remove remove,char *bucket_name)
{
	error_t errcode = RET_SUCCESS;
	v_file_list_t file_list;
	v_file_info_t *item,*n;
	memset(&file_list,0,sizeof(v_file_list_t));
	file_list.parent_id = parent_id;
	dl_list_init(&file_list.head);
	if((errcode = get_file_list_by_parent_id(database, &file_list,bucket_name)) != RET_SUCCESS)
	{
		DMCLOG_E("get file list by parent_id error");
		dl_list_for_each_safe(item, n, &file_list.head, v_file_info_t, next)
		{
		  	safe_free(item);
		}   
		return errcode;
	}
	
	DMCLOG_D("result_cnt(%d)", file_list.total);
    if(file_list.total > 0)
    {
		dl_list_for_each(item, &(file_list.head), v_file_info_t, next)
		{
		 // init item member
			if(item->isDir == 1)
			{
				//the file is folder
				errcode = load_v_file_delete_cmd(database,item->id,remove,bucket_name);
				if(errcode != RET_SUCCESS)
				{
					DMCLOG_E("delete normal file error");
					break;
				}
			}else{//the file is normal file
				DMCLOG_E("item->name = %s,item->id = %d,real_id = %d",item->name,item->id,item->real_file_id);
				if((errcode = load_v_file_delete_by_id_cmd(database,item->isDir,item->id,item->real_file_id,remove,bucket_name)) != 0)
				{
					DMCLOG_E("delete normal file error");
					break;
				}
			}
		}
     }
     dl_list_for_each_safe(item, n, &(file_list.head), v_file_info_t, next)
     {
         safe_free(item);
     }
	if((errcode = load_v_file_delete_by_id_cmd(database, 1,parent_id,0,remove,bucket_name)) != 0)
	{
		DMCLOG_E("delete normal file error");
		return errcode;
	}
    return errcode;
}

int load_v_file_delete_type_by_path_cmd(sqlite3 *database,int file_type,int parent_id,real_remove remove,char *bucket_name)
{
	error_t errcode = RET_SUCCESS;
	v_file_list_t file_list;
	v_file_info_t *item,*n;
	memset(&file_list,0,sizeof(v_file_list_t));
	file_list.parent_id = parent_id;
	S_STRNCPY(file_list.bucket_name,bucket_name,MAX_BUCKET_NAME_LEN);
	dl_list_init(&file_list.head);
	if((errcode = get_file_list_by_parent_id(database, &file_list,bucket_name)) != RET_SUCCESS)
	{
		DMCLOG_E("get file list by parent_id error");
		dl_list_for_each_safe(item, n, &file_list.head, v_file_info_t, next)
		{
		  	safe_free(item);
		}   
		return errcode;
	}
	
	DMCLOG_D("result_cnt(%d)", file_list.total);
    if(file_list.total > 0)
    {
		dl_list_for_each(item, &(file_list.head), v_file_info_t, next)
		{
		 	// init item member
			if(item->isDir != 1&&item->type == file_type)
			{
				DMCLOG_E("item->name = %s,item->id = %d,real_id = %d",item->name,item->id,item->real_file_id);
				if((errcode = load_v_file_delete_by_id_cmd(database,item->isDir,item->id,item->real_file_id,remove,bucket_name)) != 0)
				{
					DMCLOG_E("delete normal file error");
					break;
				}
			}
		}
     }
     dl_list_for_each_safe(item, n, &(file_list.head), v_file_info_t, next)
     {
         safe_free(item);
     }
    return errcode;
}


/**
 * delete file
 */
error_t v_file_table_delete(sqlite3 *database,void *data)
{
	error_t errcode = RET_SUCCESS;
	if(database == NULL || data == NULL)
	{
		return EINVAL_ARG;
	}
	v_file_delete_t *pfile_delete = (v_file_delete_t *)data;
	v_file_info_t *pfile = &pfile_delete->v_file_info;
	if(pfile_delete->cmd == V_FILE_TABLE_DELETE_INFO)
	{
		errcode = load_v_file_cmd(database, pfile,pfile_delete->bucket_name);
		if(errcode != EDB_RECORD_NOT_EXIST)//file exist, go ahead
		{
			if(pfile->isDir == 1)
				return load_v_file_delete_cmd(database,pfile->id,pfile_delete->remove,pfile_delete->bucket_name);
			else{
				return load_v_file_delete_by_id_cmd(database,pfile->isDir,pfile->id,pfile->real_file_id,pfile_delete->remove,pfile_delete->bucket_name);
			}
		}
	}else if(pfile_delete->cmd == V_FILE_TABLE_DELETE_TYPE_BY_PATH)
	{
		int file_type = pfile->type;
		DMCLOG_D("file_type = %d",file_type);
		errcode = load_v_file_cmd(database, pfile,pfile_delete->bucket_name);
		if(errcode != EDB_RECORD_NOT_EXIST)//file exist, go ahead
		{
			if(pfile->isDir != 1)
				return EDB_FILE_IS_GENENAL;
			
			return load_v_file_delete_type_by_path_cmd(database,file_type,pfile->id,pfile_delete->remove,pfile_delete->bucket_name);
		}
	}
	return errcode;
}

static error_t v_file_table_update_parent_id_and_path(sqlite3 *database, int id,int parent_id,int isfolder,
									 char *des_path,char *bucket_name)
{
	ENTER_FUNC();
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	error_t errcode = RET_SUCCESS;
	if(des_path == NULL||bucket_name == NULL)
	{
		DMCLOG_E("para is null");
		return EINVAL_ARG;
	}

	

	char *path_escape = db_path_escape(des_path);
	assert(path_escape != NULL);
	
	char *name = (char *)bb_basename(path_escape);
	char out[MAX_FILE_NAME_LEN] = {0};
	char gb_name[MAX_FILE_NAME_LEN] = {0};
	xxGetPinyin(name, gb_name, out);
	
	DMCLOG_D("name = %s,gb_name = %s,out = %s",name,gb_name,out);
	if(isfolder == 1)
	{
		sprintf(sql,"UPDATE %s SET FILE_NAME='%s',GB_NAME='%s',FILE_PATH='%s',PARENT_ID='%d' WHERE FILE_ID='%d'",bucket_name, name,gb_name,path_escape,parent_id, id);
	}	
	else
	{
		int type = db_get_mime_type(des_path,strlen(des_path));
		sprintf(sql,"UPDATE %s SET FILE_NAME='%s',GB_NAME='%s',PARENT_ID='%d',TYPE='%d' WHERE FILE_ID='%d'", bucket_name,name,gb_name,parent_id,type,id);
	}

	if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL))
			!= RET_SUCCESS)
	{
		DMCLOG_E("update rename error");
		safe_free(path_escape);
		return errcode;	
	}
	safe_free(path_escape);
	EXIT_FUNC();
	return errcode;
}

static error_t v_file_table_update_path(sqlite3 *database, int id, char *des_path,char *bucket_name)
{
	ENTER_FUNC();
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	error_t errcode = RET_SUCCESS;
	if(des_path == NULL||bucket_name == NULL)
	{
		DMCLOG_E("para is null");
		return EINVAL_ARG;
	}

	char *path_escape = db_path_escape(des_path);
	assert(path_escape != NULL);
	char *name = (char *)bb_basename(path_escape);

	sprintf(sql,"UPDATE %s SET FILE_PATH='%s',FILE_NAME='%s' WHERE FILE_ID=%d",\
		bucket_name,path_escape,name,id);
		
	if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL))
			!= RET_SUCCESS)
	{
		DMCLOG_E("update rename error");
		safe_free(path_escape);
		return errcode;	
	}
	safe_free(path_escape);
	EXIT_FUNC();
	return errcode;
}


static int v_file_table_recursion_update_file(sqlite3 *database,int parent_id,char *parent_path,char *bucket_name)
{
	error_t errcode = RET_SUCCESS;
	v_file_list_t v_file_list;
	v_file_info_t *item,*n;
	memset(&v_file_list,0,sizeof(v_file_list_t));
	v_file_list.parent_id = parent_id;
	dl_list_init(&v_file_list.head);
	if((errcode = get_file_list_by_parent_id(database, &v_file_list,bucket_name)) != RET_SUCCESS)
	{
		DMCLOG_E("get file list by parent_id error");
		dl_list_for_each_safe(item, n, &v_file_list.head, v_file_info_t, next)
		{
		  	safe_free(item);
		}   
		return errcode;
	}
	
	DMCLOG_D("result_cnt(%u)", v_file_list.total);
    if(v_file_list.total > 0)
    {
		dl_list_for_each(item, &(v_file_list.head), v_file_info_t, next)
		{
		 // init item member
			if(item->isDir == 1)
			{
				//the file is folder
				if(parent_path[strlen(parent_path) - 1] != '/')
					sprintf(item->path,"%s/%s",parent_path,item->name);
				else{
					sprintf(item->path,"%s%s",parent_path,item->name);
				}

				errcode = v_file_table_update_path(database,item->id,item->path,bucket_name);
				if(errcode != RET_SUCCESS)
				{
					DMCLOG_E("update dir path error");
					break;
				}
				errcode = v_file_table_recursion_update_file(database,item->id,item->path,bucket_name);
				if(errcode != RET_SUCCESS)
				{
					DMCLOG_E("update dir path error");
					break;
				}
			}
		}
     }
	dl_list_for_each_safe(item, n, &(v_file_list.head), v_file_info_t, next)
	{
		safe_free(item);
	}
    return errcode;
}

static error_t v_file_table_recursion_insert_file(sqlite3 *database,int src_id,int parent_id,char *parent_path,char *src_bucket_name,char *des_bucket_name)
{
	error_t errcode = RET_SUCCESS;
	v_file_list_t v_file_list;
	v_file_info_t *item,*n;
	memset(&v_file_list,0,sizeof(v_file_list_t));

	v_file_list.parent_id = src_id;
	dl_list_init(&v_file_list.head);
	
	if((errcode = get_file_list_by_parent_id(database, &v_file_list,src_bucket_name)) != RET_SUCCESS)
	{
		DMCLOG_E("get file list by parent_id error");
		dl_list_for_each_safe(item, n, &v_file_list.head, v_file_info_t, next)
		{
		  	safe_free(item);
		}   
		return errcode;
	}

    if(v_file_list.total > 0)
    {
		dl_list_for_each(item, &(v_file_list.head), v_file_info_t, next)
		{
		 // init item member
		 	item->parent_id = parent_id;
		 	if(item->isDir == 1)
		 	{
				if(parent_path[strlen(parent_path) - 1] != '/')
					sprintf(item->path,"%s/%s",parent_path,item->name);
				else{
					sprintf(item->path,"%s%s",parent_path,item->name);
				}
				DMCLOG_D("path = %s",item->path);
			}
		 	int src_id = item->id;
			errcode = load_v_file_insert_cmd(database,item,des_bucket_name);
			if(errcode != RET_SUCCESS)
			{
				DMCLOG_E("insert file info error");
				return errcode;
			}
			load_file_des_link_cmd(database,item->real_file_id);
			
			if(item->isDir == 1)
			{
				if((errcode = get_max_id(database,&item->id,des_bucket_name)) != RET_SUCCESS)
				{
					DMCLOG_E("get max id error");
					return errcode;
				}
				DMCLOG_D("item->path = %s",item->path);
				//the file is folder
				errcode = v_file_table_recursion_insert_file(database,src_id,item->id,item->path,src_bucket_name,des_bucket_name);
				if(errcode != RET_SUCCESS)
				{
					DMCLOG_E("insert files error");
					return errcode;
				}
			}
		}
     }
     dl_list_for_each_safe(item, n, &(v_file_list.head), v_file_info_t, next)
     {
         safe_free(item);
     }
    return errcode;
}


static error_t v_file_table_rename(sqlite3 *database, v_file_info_t *v_file_info,real_remove remove,char *bucket_name)
{
	ENTER_FUNC();
	error_t errcode = RET_SUCCESS;
	
	if(database == NULL || v_file_info == NULL ||bucket_name == NULL)
	{
		return EINVAL_ARG;
	}
	// if the des file is exist,get the file parent id and delete it by id
	v_file_info_t v_des_info;
	memset(&v_des_info,0,sizeof(v_file_info_t));
	strcpy(v_des_info.path,v_file_info->des_path);
	errcode = load_v_file_cmd(database,&v_des_info,bucket_name);
	if(errcode == RET_SUCCESS)
	{
		if(v_des_info.isDir == 1)
		{
			errcode = load_v_file_delete_cmd(database,v_des_info.id,remove,bucket_name);
		}else{
			errcode = load_v_file_delete_by_id_cmd(database,v_des_info.isDir,v_des_info.id,v_des_info.real_file_id,remove,bucket_name);
		}
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("delete file error");
			return errcode;
		}
	}

	if(v_file_info->des_path[strlen(v_file_info->des_path) - 1] != '/')
		sprintf(v_file_info->path,"%s",v_file_info->des_path);
	else{
		v_file_info->des_path[strlen(v_file_info->des_path) - 1] = '0';
		sprintf(v_file_info->path,"%s",v_file_info->des_path);
	}

	// update parent id and path by id
	errcode = v_file_table_update_parent_id_and_path(database, v_file_info->id,v_des_info.parent_id,v_file_info->isDir,
		v_file_info->path,bucket_name);
	if(errcode != RET_SUCCESS)
	{
		DMCLOG_E("update parent_id and name error");
		return errcode;
	}

	query_file_info_by_id(database,v_file_info,bucket_name);
	
	
	// 2 recursion update chile item's path if the file is dir
	if(v_file_info->isDir == 1)
	{
		errcode = v_file_table_recursion_update_file(database,v_file_info->id,v_file_info->path,bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("update dir path error");
			return errcode;
		}
	}
	return errcode;
}

static error_t _v_file_table_rename(sqlite3 *database, v_file_info_t *v_src_info, v_file_info_t *v_des_info,real_remove remove)
{
	ENTER_FUNC();
	error_t errcode = RET_SUCCESS;
	
	if(database == NULL || v_src_info == NULL ||v_des_info == NULL)
	{
		return EINVAL_ARG;
	}
	// if the des file is exist,get the file parent id and delete it by id
	errcode = load_v_file_cmd(database,v_des_info,v_des_info->bucket_name);
	if(errcode == RET_SUCCESS)
	{
		if(v_des_info->isDir == 1)
		{
			errcode = load_v_file_delete_cmd(database,v_des_info->id,remove,v_des_info->bucket_name);
		}else{
			errcode = load_v_file_delete_by_id_cmd(database,v_des_info->isDir,v_des_info->id,v_des_info->real_file_id,remove,v_des_info->bucket_name);
		}
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("delete file error");
			return errcode;
		}
	}

	if(v_des_info->path[strlen(v_des_info->path) - 1] != '/')
		sprintf(v_src_info->path,"%s",v_des_info->path);
	else{
		v_des_info->path[strlen(v_des_info->path) - 1] = '0';
		sprintf(v_src_info->path,"%s",v_des_info->path);
	}

	// update parent id and path by id
	errcode = v_file_table_update_parent_id_and_path(database, v_src_info->id,v_des_info->parent_id,v_src_info->isDir,
		v_src_info->path,v_src_info->bucket_name);
	if(errcode != RET_SUCCESS)
	{
		DMCLOG_E("update parent_id and name error");
		return errcode;
	}

	query_file_info_by_id(database,v_src_info,v_src_info->bucket_name);
	
	
	// 2 recursion update chile item's path if the file is dir
	if(v_src_info->isDir == 1)
	{
		errcode = v_file_table_recursion_update_file(database,v_src_info->id,v_src_info->path,v_src_info->bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("update dir path error");
			return errcode;
		}
	}
	return errcode;
}


static error_t v_file_table_move(sqlite3 *database, v_file_info_t *v_file_info,real_remove remove, char *bucket_name)
{
	ENTER_FUNC();
	error_t errcode = RET_SUCCESS;
	
	if(database == NULL || v_file_info == NULL ||bucket_name == NULL)
	{
		return EINVAL_ARG;
	}
	// 1 get des path info
	v_file_info_t v_des_info;
	memset(&v_des_info,0,sizeof(v_file_info_t));
	strcpy(v_des_info.path,v_file_info->des_path);
	errcode = load_v_file_cmd(database,&v_des_info,bucket_name);
	if(errcode != RET_SUCCESS)
	{
		DMCLOG_E("get v file info error",v_des_info.path);
		return errcode;
	}

	// 2 if the des file is not dir ,go back
	if(v_des_info.isDir != 1)
	{
		DMCLOG_E("the des path is not dir");
		return EDIR_EXIST;
	}
	DMCLOG_D("des path's id = %d",v_des_info.id);

	// 3 if the des_path+src_name is exist,delete it
	int id;
	char *name = (char *)bb_basename(v_file_info->path);
	errcode = v_file_name_exist(database,name,v_des_info.id,&id,bucket_name);
	if(errcode == RET_SUCCESS)
	{	
		//if the file is exist in v file table,delete it
		errcode = load_v_file_delete_cmd(database,id,remove,bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("delete error");
			return errcode;
		}
	}
	
	// 4 update the src_path item to new parent id = des path's id , new path and new name;
	errcode = load_v_file_cmd(database,v_file_info,bucket_name);
	if(errcode != RET_SUCCESS)
	{
		DMCLOG_E("get v file info error",v_file_info->path);
		return errcode;
	}
	
	v_file_info->parent_id = v_des_info.id;
	if(v_file_info->des_path[strlen(v_file_info->des_path) - 1] != '/')
		sprintf(v_file_info->path,"%s/%s",v_file_info->des_path,v_file_info->name);
	else{
		sprintf(v_file_info->path,"%s%s",v_file_info->des_path,v_file_info->name);
	}
	DMCLOG_D("v_file_info->path = %s",v_file_info->path);
	errcode = v_file_table_update_parent_id_and_path(database, v_file_info->id,v_file_info->parent_id,v_file_info->isDir,
		v_file_info->path,bucket_name);
	if(errcode != RET_SUCCESS)
	{
		DMCLOG_E("update parent_id and name error");
		return errcode;
	}
	
	// 5 recursion update chile item's path that isDir == true
	errcode = v_file_table_recursion_update_file(database,v_file_info->id,v_file_info->path,bucket_name);
	if(errcode != RET_SUCCESS)
	{
		DMCLOG_E("update dir path error");
		return errcode;
	}
	return errcode;
}

static error_t _v_file_table_move(sqlite3 *database, v_file_info_t *v_src_info, v_file_info_t *v_des_info,real_remove remove)
{
	ENTER_FUNC();
	error_t errcode = RET_SUCCESS;
	
	if(database == NULL || v_src_info == NULL ||v_des_info == NULL)
	{
		return EINVAL_ARG;
	}
	// 1 get des path info
	errcode = load_v_file_cmd(database,v_des_info,v_des_info->bucket_name);
	if(errcode != RET_SUCCESS)
	{
		DMCLOG_E("get v file info error",v_des_info->path);
		return errcode;
	}

	// 2 if the des file is not dir ,go back
	if(v_des_info->isDir != 1)
	{
		DMCLOG_E("the des path is not dir");
		return EDIR_EXIST;
	}
	DMCLOG_D("des path's id = %d",v_des_info->id);

	// 3 if the des_path+src_name is exist,delete it
	int id;
	char *name = (char *)bb_basename(v_src_info->path);
	errcode = v_file_name_exist(database,name,v_des_info->id,&id,v_des_info->bucket_name);
	if(errcode == RET_SUCCESS)
	{	
		//if the file is exist in v file table,delete it
		errcode = load_v_file_delete_cmd(database,id,remove,v_des_info->bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("delete error");
			return errcode;
		}
	}
	
	// 4 update the src_path item to new parent id = des path's id , new path and new name;
	errcode = load_v_file_cmd(database,v_src_info,v_src_info->bucket_name);
	if(errcode != RET_SUCCESS)
	{
		DMCLOG_E("get v file info error",v_src_info->path);
		return errcode;
	}
	v_src_info->parent_id = v_des_info->id;
			
	if(v_des_info->path[strlen(v_des_info->path) - 1] != '/')
		sprintf(v_src_info->path,"%s/%s",v_des_info->path,v_src_info->name);
	else{
		sprintf(v_src_info->path,"%s%s",v_des_info->path,v_src_info->name);
	}
	
	if(!strcmp(v_src_info->bucket_name,v_des_info->bucket_name))
	{
		DMCLOG_D("v_file_info->path = %s",v_src_info->path);
		errcode = v_file_table_update_parent_id_and_path(database, v_src_info->id,v_src_info->parent_id,v_src_info->isDir,
		v_src_info->path,v_src_info->bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("update parent_id and name error");
			return errcode;
		}
		
		// 5 recursion update chile item's path that isDir == true
		errcode = v_file_table_recursion_update_file(database,v_src_info->id,v_src_info->path,v_src_info->bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("update dir path error");
			return errcode;
		}
	}else{
		int src_id = v_src_info->id;
		if(v_src_info->isDir != 1)
		{
			errcode = load_v_file_insert_cmd(database,v_src_info,v_des_info->bucket_name);
			if(errcode != RET_SUCCESS)
			{
				DMCLOG_E("insert file info error");
				return errcode;
			}
			load_file_des_link_cmd(database,v_src_info->real_file_id);
			return errcode;
		}

		
		DMCLOG_D("move des path:%s",v_src_info->path);
		errcode = load_v_file_insert_cmd(database,v_src_info,v_des_info->bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("insert file info error");
			return errcode;
		}

		if((errcode = get_max_id(database,&v_src_info->id,v_des_info->bucket_name)) != RET_SUCCESS)
		{
			DMCLOG_E("get max id error");
			return errcode;
		}

		errcode = v_file_table_recursion_insert_file(database,src_id,v_src_info->id,v_src_info->path,v_src_info->bucket_name,v_des_info->bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("recursion insert file error");
			return errcode;
		}

		//copy finished,delete src file
		errcode = load_v_file_delete_cmd(database,src_id,remove,v_src_info->bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("delete error");
			return errcode;
		}
	}
	
	return errcode;
}



static error_t v_file_table_copy(sqlite3 *database, v_file_info_t *v_file_info,real_remove remove, char *bucket_name)
{
	ENTER_FUNC();
	error_t errcode = RET_SUCCESS;
	
	if(database == NULL || v_file_info == NULL ||bucket_name == NULL)
	{
		return EINVAL_ARG;
	}
	// 1 get des path info
	v_file_info_t v_des_info;
	memset(&v_des_info,0,sizeof(v_file_info_t));
	strcpy(v_des_info.path,v_file_info->des_path);
	errcode = load_v_file_cmd(database,&v_des_info,bucket_name);
	if(errcode != RET_SUCCESS)
	{
		DMCLOG_E("get v file info error",v_des_info.path);
		return errcode;
	}

	// 2 if the des file is not dir ,go back
	if(v_des_info.isDir != 1)
	{
		DMCLOG_E("the des path is not dir");
		return EDIR_EXIST;
	}
	DMCLOG_D("des path's id = %d",v_des_info.id);

	// 3 if the des_path+src_name is exist,delete it
	int id;
	char *name = (char *)bb_basename(v_file_info->path);
	errcode = v_file_name_exist(database,name,v_des_info.id,&id,bucket_name);
	if(errcode == RET_SUCCESS)
	{	
		//if the file is exist in v file table,delete it
		errcode = load_v_file_delete_cmd(database,id,remove,bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("delete error");
			return errcode;
		}
	}
	
	// 4 insert src file to des path recursion;
	errcode = load_v_file_cmd(database,v_file_info,bucket_name);
	if(errcode != RET_SUCCESS)
	{
		DMCLOG_E("get v file info error",v_file_info->path);
		return errcode;
	}

	int src_id = v_file_info->id;
	
	v_file_info->parent_id = v_des_info.id;
	if(v_file_info->isDir != 1)
	{
		errcode = load_v_file_insert_cmd(database,v_file_info,bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("insert file info error");
			return errcode;
		}
		load_file_des_link_cmd(database,v_file_info->real_file_id);
		return errcode;
	}
	
	if(v_des_info.path[strlen(v_des_info.path) - 1] != '/')
		sprintf(v_file_info->path,"%s/%s",v_des_info.path,v_file_info->name);
	else{
		sprintf(v_file_info->path,"%s%s",v_des_info.path,v_file_info->name);
	}
	
	DMCLOG_D("copy des path:%s",v_file_info->path);
	errcode = load_v_file_insert_cmd(database,v_file_info,bucket_name);
	if(errcode != RET_SUCCESS)
	{
		DMCLOG_E("insert file info error");
		return errcode;
	}

	if((errcode = get_max_id(database,&v_file_info->id,bucket_name)) != RET_SUCCESS)
	{
		DMCLOG_E("get max id error");
		return errcode;
	}

	errcode = v_file_table_recursion_insert_file(database,src_id,v_file_info->id,v_file_info->path,bucket_name,bucket_name);
	if(errcode != RET_SUCCESS)
	{
		DMCLOG_E("recursion insert file error");
		return errcode;
	}
	return errcode;
}

static error_t _v_file_table_copy(sqlite3 *database, v_file_info_t *v_src_info,v_file_info_t *v_des_info,real_remove remove)
{
	ENTER_FUNC();
	error_t errcode = RET_SUCCESS;
	
	if(database == NULL || v_src_info == NULL ||v_des_info == NULL)
	{
		return EINVAL_ARG;
	}
	// 1 get des path info
	errcode = load_v_file_cmd(database,v_des_info,v_des_info->bucket_name);
	if(errcode != RET_SUCCESS)
	{
		DMCLOG_E("get v file info error",v_des_info->path);
		return errcode;
	}

	// 2 if the des file is not dir ,go back
	if(v_des_info->isDir != 1)
	{
		DMCLOG_E("the des path is not dir");
		return EDIR_EXIST;
	}
	DMCLOG_D("des path's id = %d",v_des_info->id);

	// 3 if the des_path+src_name is exist,delete it
	int id;
	char *name = (char *)bb_basename(v_src_info->path);
	errcode = v_file_name_exist(database,name,v_des_info->id,&id,v_des_info->bucket_name);
	if(errcode == RET_SUCCESS)
	{	
		//if the file is exist in v file table,delete it
		errcode = load_v_file_delete_cmd(database,id,remove,v_des_info->bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("delete error");
			return errcode;
		}
	}
	
	// 4 insert src file to des path recursion;
	errcode = load_v_file_cmd(database,v_src_info,v_src_info->bucket_name);
	if(errcode != RET_SUCCESS)
	{
		DMCLOG_E("get v file info error",v_src_info->path);
		return errcode;
	}

	int src_id = v_src_info->id;
	
	v_src_info->parent_id = v_des_info->id;
	if(v_des_info->path[strlen(v_des_info->path) - 1] != '/')
		sprintf(v_src_info->path,"%s/%s",v_des_info->path,v_src_info->name);
	else{
		sprintf(v_src_info->path,"%s%s",v_des_info->path,v_src_info->name);
	}
	
	
	if(v_src_info->isDir != 1)
	{
		errcode = load_v_file_insert_cmd(database,v_src_info,v_des_info->bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("insert file info error");
			return errcode;
		}
		load_file_des_link_cmd(database,v_src_info->real_file_id);
		return errcode;
	}
	
	DMCLOG_D("copy des path:%s",v_src_info->path);
	errcode = load_v_file_insert_cmd(database,v_src_info,v_des_info->bucket_name);
	if(errcode != RET_SUCCESS)
	{
		DMCLOG_E("insert file info error");
		return errcode;
	}

	if((errcode = get_max_id(database,&v_src_info->id,v_des_info->bucket_name)) != RET_SUCCESS)
	{
		DMCLOG_E("get max id error");
		return errcode;
	}

	errcode = v_file_table_recursion_insert_file(database,src_id,v_src_info->id,v_src_info->path,v_src_info->bucket_name,v_des_info->bucket_name);
	if(errcode != RET_SUCCESS)
	{
		DMCLOG_E("recursion insert file error");
		return errcode;
	}
	return errcode;
}


static error_t load_v_file_update_cmd(sqlite3 *database,file_info_t *pfile,char *bucket_name)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "UPDATE %s SET TYPE='%d',SIZE='%lld',CREATE_TIME ='%ld',MODIFY_TIME = '%ld',ACCESS_TIME='%ld',UUID='%s' WHERE FILE_ID='%d'", \
		bucket_name,pfile->type,(long long)pfile->size,pfile->ctime,pfile->mtime,pfile->atime,pfile->uuid,pfile->id);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}


/**
 * update file table
 */
error_t v_file_table_update(sqlite3 *database,void *data)
{
	error_t errcode = RET_SUCCESS;
	if(database == NULL || data == NULL)
	{
		return EINVAL_ARG;
	}
	v_file_update_t *pfile_update = (v_file_update_t *)data;

	v_file_info_t *pfile = &pfile_update->v_file_info;
	v_file_info_t *dfile = &pfile_update->v_des_info;
	
	if(pfile_update->cmd == V_FILE_TABLE_UPDATE_FILE_INFO)
	{
		errcode = get_parent_id_by_path(database,pfile->path,&pfile->parent_id,pfile_update->bucket_name);
		if(errcode != RET_SUCCESS)//file not exist, go ahead
		{
			DMCLOG_E("%s is not exist",pfile->path);
			return errcode;
		}
		DMCLOG_D("path = %s",pfile->path);
		char *name = (char *)bb_basename(pfile->path);
		errcode = v_file_name_exist(database,name ,pfile->parent_id,&pfile->id,pfile_update->bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("the %s is not exist",pfile->path);
			return errcode;
		}
		return load_v_file_update_cmd(database,pfile,pfile_update->bucket_name);
	}else if(pfile_update->cmd == V_FILE_TABLE_UPDATE_FILE_LIST)
	{
		//TODO
	}else if(pfile_update->cmd == V_FILE_TABLE_UPDATE_FILE_RENAME)
	{
		errcode = load_v_file_cmd(database,pfile,pfile->bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("get %s v file info error",pfile->path);
			return errcode;
		}
		return _v_file_table_rename(database,pfile,dfile,pfile_update->remove);
	}else if(pfile_update->cmd == V_FILE_TABLE_UPDATE_FILE_MOVE)
	{
		return _v_file_table_move(database,pfile,dfile,pfile_update->remove);
	}else if(pfile_update->cmd == V_FILE_TABLE_UPDATE_FILE_COPY)
	{
		return _v_file_table_copy(database,pfile,dfile,pfile_update->remove);
	}else if(pfile_update->cmd == V_FILE_TABLE_UPDATE_IMAGE_INFO)
	{
		load_file_info_by_path_cmd(database, pfile->real_path,&pfile->real_file_id);
		if(pfile->real_file_id == 0)
		{
			DMCLOG_E("the %s is not exist",pfile->real_path);
			return EDB_RECORD_NOT_EXIST;
		}
		//pfile->media_info.image_info.index = pfile->real_file_id;
		errcode = load_image_insert_cmd(database,&pfile->media_info.image_info);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("update %s v file info error",pfile->real_path);
			return errcode;
		}
		get_media_max_id(database,&pfile->media_info.image_info.index,IMAGE_TABLE_NAME);
		DMCLOG_D("pfile->media_info.image_info.index = %d",pfile->media_info.image_info.index);
		load_file_update_media_index_cmd(database,pfile->real_file_id,pfile->media_info.image_info.index);
		return errcode;
	}
	else if(pfile_update->cmd == V_FILE_TABLE_UPDATE_VIDEO_INFO)
	{
		load_file_info_by_path_cmd(database, pfile->real_path,&pfile->real_file_id);
		if(pfile->real_file_id == 0)
		{
			DMCLOG_E("the %s is not exist");
			return EDB_RECORD_NOT_EXIST;
		}
		//pfile->media_info.video_info.index = pfile->real_file_id;
		errcode = load_video_insert_cmd(database,&pfile->media_info.video_info);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("update %s v file info error",pfile->real_path);
			return errcode;
		}
		get_media_max_id(database,&pfile->media_info.video_info.index,VIDEO_TABLE_NAME);
		DMCLOG_D("pfile->media_info.video_info.index = %d",pfile->media_info.video_info.index);
		load_file_update_media_index_cmd(database,pfile->real_file_id,pfile->media_info.video_info.index);
		return errcode;
	}
	else if(pfile_update->cmd == V_FILE_TABLE_UPDATE_AUDIO_INFO)
	{
		load_file_info_by_path_cmd(database, pfile->real_path,&pfile->real_file_id);
		if(pfile->real_file_id == 0)
		{
			DMCLOG_E("the %s is not exist",pfile->real_path);
			return EDB_RECORD_NOT_EXIST;
		}
		//pfile->media_info.audio_info.index = pfile->real_file_id;
		errcode = load_audio_insert_cmd(database,&pfile->media_info.audio_info);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("update %s v file info error",pfile->real_path);
			return errcode;
		}
		get_media_max_id(database,&pfile->media_info.audio_info.index,AUDIO_TABLE_NAME);
		DMCLOG_D("pfile->media_info.audio_info.index = %d",pfile->media_info.audio_info.index);
		load_file_update_media_index_cmd(database,pfile->real_file_id,pfile->media_info.audio_info.index);
		return errcode;
	}else if(pfile_update->cmd == V_FILE_TABLE_UPDATE_THUM_INFO)
	{
		load_file_info_by_path_cmd(database, pfile->real_path,&pfile->real_file_id);
		if(pfile->real_file_id == 0)
		{
			DMCLOG_E("the %s is not exist",pfile->real_path);
			return EDB_RECORD_NOT_EXIST;
		}

		errcode = load_thum_insert_cmd(database,&pfile->media_info.thum_info);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("update %s thum info error",pfile->real_path);
			return errcode;
		}
		get_media_max_id(database,&pfile->media_info.thum_info.index,THUM_TABLE_NAME);
		DMCLOG_D("pfile->media_info.thum_info.index = %d",pfile->media_info.thum_info.index);
		load_file_update_thum_index_cmd(database,pfile->real_file_id,pfile->media_info.thum_info.index);
		return errcode;
	}
	return errcode;
}



/**
 *get file list
 */
error_t v_file_table_query(sqlite3 *database,void *data)
{
	error_t errcode = RET_SUCCESS;
	if(database == NULL || data == NULL)
	{
		return EINVAL_ARG;
	}
	v_file_query_t *pfile_query = (v_file_query_t *)data;
	v_file_list_t  *v_file_list = &pfile_query->v_file_list;
	v_file_info_t *v_file_info = &pfile_query->v_file_info;
	if(pfile_query->cmd == V_FILE_TABLE_QUERY_LIST)
	{
		return load_v_file_list_cmd(database, &pfile_query->v_file_list,pfile_query->bucket_name);
	}else if(pfile_query->cmd == V_FILE_TABLE_QUERY_INFO)
	{
		DMCLOG_D("v_file_info->path = %s",v_file_info->path);
		errcode = load_v_file_cmd(database,v_file_info,pfile_query->bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("get file info error",v_file_info->path);
			return errcode;
		}
		errcode = load_file_info_cmd(database, v_file_info);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("get file info error",v_file_info->path);
			return errcode;
		}
		DMCLOG_D("real_path = %s",v_file_info->real_path);
		return errcode;
	}else if(pfile_query->cmd == V_FILE_TABLE_QUERY_INFO_BY_UUID)
	{
		 return load_v_uuid_exist_cmd(database,v_file_info->uuid,pfile_query->bucket_name);
	}else if(pfile_query->cmd == V_FILE_TABLE_QUERY_LIST_BY_UUID)
	{
		v_file_info_t *p_file_info;
		dl_list_for_each(p_file_info, &(pfile_query->v_file_list.head), v_file_info_t, next)
		{
			DMCLOG_D("p_file_info->file_uuid = %s",p_file_info->uuid);	
			
			if((errcode = load_v_uuid_exist_cmd(database,p_file_info->uuid,pfile_query->bucket_name)) != RET_SUCCESS)
		    {
		        DMCLOG_E("query fileuuid failed");
				p_file_info->backupFlag = 1;
		    }
		}
    	return errcode;	
	}else if(pfile_query->cmd == V_FILE_TABLE_SEARCH_QUERY_LIST)
	{
		int parent_id;
		error_t errcode = query_dir_file_id(database,v_file_list->path,&parent_id,pfile_query->bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_D("query file doesn't exist,src_path=%s",v_file_list->path);
			return EDB_RECORD_NOT_EXIST;
		}
		return load_v_file_search_list_cmd(database,parent_id, v_file_list->search_str,&v_file_list->head,pfile_query->bucket_name);
	}else if(pfile_query->cmd == V_FILE_TABLE_QUERY_AUDIO_INFO)
	{
		DMCLOG_D("v_file_info->path = %s",v_file_info->path);
		errcode = load_v_file_cmd(database,v_file_info,pfile_query->bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("get file info error",v_file_info->path);
			return errcode;
		}
		errcode = load_file_info_cmd(database, v_file_info);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("get file info error",v_file_info->path);
			return errcode;
		}
		DMCLOG_D("real_path = %s,meida_index = %d",v_file_info->real_path,v_file_info->media_index);
		audio_info_t *paudio = &v_file_info->media_info.audio_info;
		paudio->index = v_file_info->media_index;
		errcode = load_audio_query_cmd(database,paudio);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("load image error",v_file_info->media_index);
			return errcode;
		}
		return errcode;
	}else if(pfile_query->cmd == V_FILE_TABLE_QUERY_IMAGE_INFO)
	{
		DMCLOG_D("v_file_info->path = %s",v_file_info->path);
		errcode = load_v_file_cmd(database,v_file_info,pfile_query->bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("get file info error",v_file_info->path);
			return errcode;
		}
		errcode = load_file_info_cmd(database, v_file_info);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("get file info error",v_file_info->path);
			return errcode;
		}
		DMCLOG_D("real_path = %s,meida_index = %d",v_file_info->real_path,v_file_info->media_index);
		image_info_t *pimage = &v_file_info->media_info.image_info;
		pimage->index = v_file_info->media_index;
		errcode = load_image_query_cmd(database,pimage);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("load image error",v_file_info->media_index);
			return errcode;
		}
		return errcode;
	}else if(pfile_query->cmd == V_FILE_TABLE_QUERY_VIDEO_INFO)
	{
		DMCLOG_D("v_file_info->path = %s",v_file_info->path);
		errcode = load_v_file_cmd(database,v_file_info,pfile_query->bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("get file info error",v_file_info->path);
			return errcode;
		}
		errcode = load_file_info_cmd(database, v_file_info);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("get file info error",v_file_info->path);
			return errcode;
		}
		DMCLOG_D("real_path = %s",v_file_info->real_path);
		video_info_t *pvideo = &v_file_info->media_info.video_info;
		pvideo->index = v_file_info->media_index;
		errcode = load_video_query_cmd(database,pvideo);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("load video error",v_file_info->media_index);
			return errcode;
		}
		return errcode;
	}else if(pfile_query->cmd == V_FILE_TABLE_QUERY_THUM_INFO)
	{
		DMCLOG_D("v_file_info->path = %s",v_file_info->path);
		errcode = load_v_file_cmd(database,v_file_info,pfile_query->bucket_name);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("get file info error",v_file_info->path);
			return errcode;
		}
		errcode = load_file_info_cmd(database, v_file_info);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("get file info error",v_file_info->path);
			return errcode;
		}
		DMCLOG_D("real_path = %s,meida_index = %d",v_file_info->real_path,v_file_info->media_index);
		thum_info_t *pthum = &v_file_info->media_info.thum_info;
		pthum->index = v_file_info->thum_index;
		errcode = load_thum_query_cmd(database,pthum);
		if(errcode != RET_SUCCESS)
		{
			DMCLOG_E("load thum error",v_file_info->thum_index);
			return errcode;
		}
		return errcode;
	}
	return ESHELL_CMD;
}

void register_v_file_table_ops(void)
{
	memset(&g_v_file_table, 0, sizeof(g_v_file_table));
	
	g_v_file_table.ops.insert          	= v_file_table_insert;
	g_v_file_table.ops.update   		= v_file_table_update;
	g_v_file_table.ops.delete     	 	= v_file_table_delete;
	g_v_file_table.ops.query           	= v_file_table_query;
}


