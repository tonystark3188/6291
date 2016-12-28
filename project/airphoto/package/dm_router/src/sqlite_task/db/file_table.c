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


#include "db/file_table.h"
#include "db/user_table.h"
#include "db/db_mm_pool.h"
#include "db/db_table.h"
#include "db/db_config.h"
#include "file_opr.h"
#include "db/nas_db.h"
#include "defs.h"


#define  KB  1024LL
#define  MB (1024*KB)
#define  GB (1024*MB)

// modify by wenhao
#define  TRASH_FILE_PREFIX  "$%"
#define  TRASH_STR_LENGTH   2

#define  MARK_TRASH    1
#define  UNMARK_TRASH  0


#define TRAVERSE_BUFF_SIZE  1024



struct map
{
	uint32_t dir_id[TRAVERSE_BUFF_SIZE];
	char *path[TRAVERSE_BUFF_SIZE];
	int path_buf_len;
	int dir_cnt;
};

struct del_msg
{
    sqlite3 *db;
	update_info_t *update_info;
	delete_callback callback_fn;
	DBCallBackObj *ext_cb_obj;
//	uint32_t dir_id_buf[TRAVERSE_BUFF_SIZE];
	struct map map[2];
	int index;
	//int dir_cnt;
};

struct basic_msg
{
	sqlite3 *db;
	CallBackFunc callback;
	void *arg;
	struct map map[2];
	int index;
};

struct mark_trash_msg
{
   sqlite3 *database;
   file_state_t state;
   struct map map[2];
   int index;
   DBCallBackObj *ext_cb_obj;
};


struct copy_map
{
    uint32_t original_id[TRAVERSE_BUFF_SIZE];
	uint32_t copied_id[TRAVERSE_BUFF_SIZE];
	int cnt;
};

struct copy_msg
{
    sqlite3 *database;
	update_info_t *pui;
	struct copy_map copy_map[2];
	int index;
};



typedef error_t (*traverse_handler)(sqlite3 *database, char **FieldValue);


db_table_t g_file_table;
static uint32_t  g_file_id = 0;



static error_t get_parent_id(sqlite3 *database, const char *path, uint32_t *id);
static error_t delete_directory(sqlite3 *database, delete_data_t *pdel_data, file_info_t *pfi,
				update_info_t *update_info,delete_callback clean_up_fn);


static int get_id_callback(void *data, int nFields, char **FieldValue, char **FieldName);
static error_t file_table_trash(sqlite3 *database, trash_data_t *ptrash_data, update_info_t *pui);
static error_t do_trash_mark(sqlite3 *database, trash_data_t *ptrash_data, file_info_t *pfi, int mark_trash);
static bool avoid_repeat_name(sqlite3 *database, uint32_t parent_id, char *name_buf);
static error_t change_parent_dir(sqlite3 *database, uint32_t id, uint32_t new_parent_id);


extern error_t get_file_path(sqlite3 *database, uint32_t id, file_info_t *pfi);
extern int get_time_str_for_db(char *time_s, size_t size);


static void set_file_info_empty(file_info_t *pfi)
{
	if(pfi != NULL)
	{
		memset(pfi, 0, sizeof(file_info_t));
		S_STRNCPY(pfi->mime_type, "NULL", MAX_MIME_TYPE_SIZE);
	}
}


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


//get file name from path
static const char *extract_file_name(const char *path)
{
    int i,len;
  
    len = strlen(path);
    if(len <= 0)
    {
        return NULL;
    }
    
    i = len;
    while(path[i-1] != SEPERATOR)
    {
        i--;
    }

    return (const char*)(path + i);
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

static void subfix_file_name_with_time(char *name, size_t size)
{    
    char tmp_time[32] = {0};
    char dot_name[32] = {0};
    uint16_t i = 0;
    uint16_t flag = 0;
    char tmp_str[64] = {0};
    int len;

    tmp_time[0] = '(';
    len = get_time_str_for_db(tmp_time+1, sizeof(tmp_time)-1);
    tmp_time[1+len] = ')';
    len = strlen(name);

    for(i = len-1; i > 0; --i)
    {
        if(name[i] == '/')
            break;
        
        if(name[i] == '.')
        {
        	//for too long subfix or  file name like .xxx
        	if(len - i >= sizeof(dot_name) || name[i-1]=='/')
        	{
				break;
			}
			
            S_STRNCPY(dot_name, name+i, sizeof(dot_name));
            name[i] = '\0';
            flag = 1;
            break;
        }
    }
    
    if(flag)
    {
        snprintf(tmp_str, sizeof(tmp_str), "%s%s", tmp_time, dot_name);
        strncat(name, tmp_str, (size - i - 1));
        name[size - 1] = '\0';
    }
    else
    {
        S_STRNCPY(tmp_str, tmp_time, sizeof(tmp_str));
        strncat(name, tmp_str, (size - len - 1));
        name[size - 1] = '\0';
    }
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
	
	if(database == NULL || name == NULL || id == NULL)
	{
		return ENULL_POINT;
	}
	
	*id = INVALID_FILE_ID;
	
	sprintf(sql, "SELECT ID FROM %s where PARENT_ID = %u AND NAME='%s'",
		FILE_TABLE_NAME, parent_id, name);

	return sqlite3_exec_busy_wait(database, sql, get_id_callback, id);
}


/*
*description: query file's  detail information
*input param:parent_id-->the target file's parent dir id  name-->the target file's name
*output param:pfi-->pointer to buffer to store the file information.
*return: RET_SUCCESS if ok.
*/
error_t query_file_info(sqlite3 *database, const char *path, const char *name, file_info_t *pfi)
{
    char sql[SQL_CMD_QUERY_BUF_SIZE];
	char escape_name[NAME_SIZE+0x10];
	pfi->index = INVALID_FILE_ID;
	error_t errcode = RET_SUCCESS;

    log_debug("start");
    
	if(database == NULL || name == NULL)
	{
		return EINVAL_ARG;
	}

	sqlite3_str_escape(name, escape_name, sizeof(escape_name));
	sprintf(sql, "SELECT * FROM %s where PATH = '%s' AND NAME='%s'",
		FILE_TABLE_NAME, path, escape_name);

	if((errcode = sqlite3_exec_busy_wait(database, sql, file_query_callback, pfi))
			!= RET_SUCCESS)
	{
        log_debug("exit error");
		return errcode;
	}

    if(pfi->index == INVALID_FILE_ID)
    {
        log_debug("exit error");
		return EDB_RECORD_NOT_EXIST;
	}

    log_debug("exit success");
	
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

static bool file_item_exist_by_path(sqlite3 *database, char *path,unsigned *id_temp)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];

	if(database == NULL)
	{
		return FALSE;
	}

	sprintf(sql, "SELECT * FROM %s WHERE PATH='%s'", FILE_TABLE_NAME, path);

	if(sqlite3_exec_busy_wait(database, sql, get_id_callback, id_temp) != SQLITE_OK)
	{
		return FALSE;
	}

	if(*id_temp == INVALID_FILE_ID)
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


/*
static error_t query_file_by_path(sqlite3 *database, const char *path, file_info_t *pfi)
{
	const char *file_name = NULL;
	uint32_t parent_id;
	error_t errcode = RET_SUCCESS;

	pfi->index = INVALID_FILE_ID;

	if((errcode = get_parent_id(database, path, &parent_id)) != RET_SUCCESS)
	{
		return errcode;	
	}

	if(parent_id == INVALID_FILE_ID)
	{
		return EDB_RECORD_NOT_EXIST;
	}

	file_name = extract_file_name(path);
	return query_file_info(database, parent_id, file_name, pfi);
}
*/
	


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
    char name_str_escape[NAME_SIZE + 0x10];
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
        sqlite3_str_escape(name, name_str_escape, sizeof(name_str_escape));
        if((errcode = query_file_id(database, parent_id, name_str_escape, id)) != RET_SUCCESS)
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
*description: get parent path. eg. /root/aa/bb/cc ---->/root/aa/bb
*input param:path-->full path   
*output param:parent_dir-->parent path
*return: RET_SUCCESS if ok.
*/
error_t get_parent_dir(const char *path, char *parent_dir)
{
    int len,level,ParentPathLen;

    if(path == NULL || parent_dir == NULL)
    {
        return EINVAL_ARG;
    }
 
    len = strlen(path);
    level = get_file_level(path);

    if(level == ROOT_LEVEL || len <= 0)
    {
        return EINVAL_ARG;
    }

    ParentPathLen = len - strlen(extract_file_name(path));
    memcpy(parent_dir,path,ParentPathLen);
    parent_dir[ParentPathLen-1] = 0;//delete the ending seperator
	
    return RET_SUCCESS;    
}


/*
*description: get parent id of file 
*input param: path-->file path
*output param:buffer to store id.
*/
static error_t get_parent_id(sqlite3 *database, const char *path, uint32_t *id)
{
    int level;
	
    if(database == NULL || path == NULL || id == NULL)
    {
		return INVALID_FILE_ID;
	}
	
	level = get_file_level(path);
	if(level <= ROOT_LEVEL)
	{
		return INVALID_FILE_ID;
	}
	DMCLOG_D("level = %d,ROOT_LEVEL = %d",level,ROOT_LEVEL);
	return get_level_file_id(database, path, level-1, id);

}
/*
*description: get parent id of file 
*input param: path-->file path
*output param:buffer to store id.
*/
error_t dm_get_parent_id(const char *path, uint32_t *id)
{
    int level;
	
    if(path == NULL || id == NULL)
    {
		return INVALID_FILE_ID;
	}

	level = get_file_level(path);
	if(level <= ROOT_LEVEL)
	{
		return INVALID_FILE_ID;
	}
	*id = level - ROOT_LEVEL;
	return 0;

}


/*
*description :update parent and ancestor directory records when we add or delete files.
*input params:pfi-->buffer storing the deleting or adding file's information. 
			 bAddUpdate--> is adding or deleting
 return: RET_SUCCESS if ok.
*/
static error_t UpdateHighLevelItems(sqlite3 *db, file_info_t *pfi, bool bAddUpdate)
{
    char *sql = get_db_write_task_sql_buf();
    uint64_t llUpdateSize;
	uint64_t file_size = pfi->file_size;
	uint32_t parent_id;
	file_info_t file_info;
	error_t errcode = RET_SUCCESS;

    if(db == NULL || pfi == NULL)
    {
        return EINVAL_ARG;
    }

    if(pfi->index == ROOT_DIR_ID)
    {
		return RET_SUCCESS;
	}

    memcpy(&file_info, pfi, sizeof(file_info_t));
	//update until we reach root dir record
	do
	{
		parent_id = file_info.parent_id;
		if((errcode = query_file_by_id(db, parent_id, &file_info))
				!= RET_SUCCESS)
		{
			return errcode;
		}

		if(bAddUpdate)
		{
            llUpdateSize = file_info.file_size + file_size;
		}
		else
		{
			llUpdateSize = file_info.file_size - file_size;
		}

		sprintf(sql, "UPDATE %s SET SIZE=%llu WHERE ID=%u", FILE_TABLE_NAME,
			llUpdateSize, parent_id);
		if((errcode = sqlite3_exec_busy_wait(db, sql, NULL, NULL))
				!= RET_SUCCESS)
		{
			return errcode;
		}
	}while(parent_id != ROOT_DIR_ID);

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

	if(sql == NULL || pfi == NULL)
	{
		return ENULL_POINT;
	}

	n = snprintf(sql, SQL_CMD_WRITE_BUF_SIZE, "INSERT INTO %s(ID,NAME,PATH,PARENT_ID,TYPE,SIZE,CREATE_TIME,"\
	    "MEDIA_INFO_INDEX,DIR,FILE_UUID,MIME_TYPE) "\
        "VALUES(%u,'%s','%s',%u,%d,%llu,%llu,%d,%d,'%s','%s');", FILE_TABLE_NAME,
         pfi->index, pfi->name_escape,pfi->path,pfi->parent_id, pfi->file_type, pfi->file_size,
         pfi->create_time,pfi->media_info_index,pfi->dir_type,pfi->file_uuid,pfi->mime_type);
	if(n >= SQL_CMD_WRITE_BUF_SIZE)
	{
        log_warning("sql buffer size is not enough,the cmd is %s",sql);
		return EOUTBOUND;
	}
	return RET_SUCCESS;
}





//load item information,a helper function in sqlite exec callback 
void load_file_item(char **FieldName, char **FieldValue, int nFields, file_info_t *pfi)
{	
    //log_debug("load file info..start");
	if(FieldValue[0] != NULL)
    	pfi->index = strtoul(FieldValue[0], NULL, 10);
	if(FieldValue[1] != NULL)
	{
		pfi->name = (char *)malloc(strlen(FieldValue[1]) + 1);
		strcpy(pfi->name, FieldValue[1]);
		//DMCLOG_D("pfi->name = %s",pfi->name);
	}
	if(FieldValue[2] != NULL)
	{
		pfi->path = (char *)malloc(strlen(FieldValue[2]) + 1);
		strcpy(pfi->path, FieldValue[2]);
		//DMCLOG_D("pfi->path = %s",pfi->path);
	}
	if(FieldValue[3] != NULL)
		pfi->parent_id = strtoul(FieldValue[3], NULL, 10);
	if(FieldValue[4] != NULL)
		pfi->file_type = atoi(FieldValue[4]);
	if(FieldValue[5] != NULL)
		pfi->file_size = strtoull(FieldValue[5], NULL, 10);
	//DMCLOG_D("pfi->file_size = %llu",pfi->file_size);
    if(FieldValue[6] != NULL)
        pfi->create_time = strtoull(FieldValue[6], NULL, 10);
	if(FieldValue[8] != NULL)
        pfi->dir_type= atoi(FieldValue[8]);
	DMCLOG_D("pfi->dir_type = %d",pfi->dir_type);
	if(FieldValue[9] != NULL)
	{
		strcpy(pfi->file_uuid, FieldValue[9]);
		//DMCLOG_D("pfi->file_uuid = %s",pfi->file_uuid);
	}
    //log_debug("load file info..end");
}



int file_query_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
    file_info_t *pFileDscrpt = (file_info_t *)data;
    load_file_item(FieldName, FieldValue, nFields, pFileDscrpt);
    //log_debug("\n");
    return 0;
}




/*
*description:delete single file record.
*input param:id-->target file id
		       pfi-->deleting file's information buffer
		       	update_info-->update information data buffer
		       	clean_up_fn-->clean up callback.
*return: RET_SUCCESS if ok.
*/
static error_t do_delete(sqlite3 *database, uint32_t id, file_info_t *pfi, 
						 update_info_t *update_info, delete_callback clean_up_fn, DBCallBackObj *ext_db_obj)
{
    char *sql = get_db_write_task_sql_buf();
	error_t errcode = RET_SUCCESS;

    //delete record in file table.
 	sprintf(sql, "DELETE FROM %s WHERE ID=%u", FILE_TABLE_NAME, id);
	if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL))
			!= SQLITE_OK)
	{
		return errcode;
	}

    //this is a regular file , we may should do some clean work for audio,video,image file..
	if(pfi != NULL && clean_up_fn)
	{
		if((errcode = clean_up_fn(database, pfi, update_info))
				!= RET_SUCCESS)
		{
			return errcode;
		}
	}

	//we may need to do some work for file system...
	if(ext_db_obj != NULL && ext_db_obj->del_cb_func != NULL)
	{
		ext_db_obj->del_cb_func(pfi, ext_db_obj->del_cb_arg);
	}

	return errcode;
}
static error_t dm_do_delete(sqlite3 *database, delete_data_t *delete_data)
{
    char *sql = get_db_write_task_sql_buf();
	error_t errcode = RET_SUCCESS;
    //delete record in file table.
 	sprintf(sql, "DELETE FROM %s WHERE PATH='%s' or PATH like '%s%c'", FILE_TABLE_NAME, delete_data->path, delete_data->path,'%');
	if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL))
			!= SQLITE_OK)
	{
		return errcode;
	}
	return errcode;
}



static char *find_parent_path(struct map *search_map, uint32_t parent_id)
{
	int i;

	for(i=0; i<search_map->dir_cnt; i++)
	{
		if(search_map->dir_id[i] == parent_id)
		{
			return search_map->path[i];
		}
	}

    return NULL;
}


static error_t record_dir_path_handler(file_info_t *pfi, const char *name, struct map *search_map, struct map *storage_map)
{
	char *parent_path;

	parent_path = find_parent_path(search_map, pfi->parent_id);	
	
	if(pfi->file_type == TYPE_DIR)//this record is a directory record.
	{
	    if(storage_map->dir_cnt > TRAVERSE_BUFF_SIZE)
	    {
			log_warning("too many dir in same level. traverse buffer is used out\n");
			return EOUTBOUND;
		}
		storage_map->dir_id[storage_map->dir_cnt] = pfi->index;//record this dir id for traverse
		storage_map->dir_cnt++;

		if(storage_map->dir_cnt > storage_map->path_buf_len)//no free path buffer, malloc it
		{
			char *path;
			path = (char *)malloc(PATH_LENGTH * sizeof(char));
			if(path == NULL)
			{
				log_warning("malloc failed");
				return 1;
			}
			//save this dir's path
			if(snprintf(path, PATH_LENGTH, "%s%c%s", parent_path, SEPERATOR, pfi->path) >= PATH_LENGTH)
			{
				log_warning("path (%s/%s) too long",parent_path, pfi->path);
				return EOUTBOUND;
			}
			storage_map->path[storage_map->path_buf_len] = path;
			storage_map->path_buf_len++;
			S_STRNCPY(pfi->path, storage_map->path[storage_map->path_buf_len - 1], PATH_LENGTH);
		}
		else //we have free path space to use..
		{
			if(snprintf(storage_map->path[storage_map->dir_cnt - 1], PATH_LENGTH, "%s%c%s", 
					parent_path, SEPERATOR, pfi->path) >= PATH_LENGTH)
			{
				log_warning("path (%s/%s) too long",parent_path, pfi->path);
				return EOUTBOUND;
			}
			S_STRNCPY(pfi->path, storage_map->path[storage_map->dir_cnt - 1], PATH_LENGTH);
		}
	}
	else//for regular file, we save the full path
	{
		if(snprintf(pfi->path, PATH_LENGTH, "%s%c%s", parent_path, SEPERATOR, name) >= PATH_LENGTH)
		{
			log_warning("path (%s/%s) too long",parent_path, pfi->path);
			return EOUTBOUND;
		}
	}
	
	return RET_SUCCESS;
}


static int delete_child_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	struct del_msg *pmsg = (struct del_msg *)data;
	struct map *search_map = &pmsg->map[pmsg->index];
	struct map *storage_map = &pmsg->map[!pmsg->index];
	file_info_t file_info;

	load_file_item(FieldName, FieldValue, nFields, &file_info);

	if(file_info.file_state == STATE_VISIBLE_IN_TRASH)//this file(or dir) has been already deleted before, keep it in recycle bin
	{
		return 0;
	}

	if(record_dir_path_handler(&file_info, FieldValue[1], search_map, storage_map)!=RET_SUCCESS)
	{
		return 1;
	}

	if(file_info.file_type == TYPE_DIR)//this record is a directory record.
	{
        if(do_delete(pmsg->db, file_info.index, &file_info, NULL, NULL, pmsg->ext_cb_obj) 
				!= RET_SUCCESS)//delete this record in file table
        {
			return 1;
		}	
	}
	else//regular file record
	{
		if(pmsg->callback_fn)
		{	
			if(do_delete(pmsg->db, file_info.index, &file_info, pmsg->update_info, 
					pmsg->callback_fn, pmsg->ext_cb_obj) != RET_SUCCESS )
			{
				return 1;
			}
		}
		else
		{
			if(do_delete(pmsg->db, file_info.index, &file_info, NULL, NULL,
					pmsg->ext_cb_obj) != RET_SUCCESS)
			{
			    return 1;
			}
		}	
	}

	return 0;
}



//similar to function delete_child_callback, but we find a directory ,we just directly delete it.
static int generic_delete_child_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	struct del_msg *pmsg = (struct del_msg *)data;
	file_info_t fi;
			
    load_file_item(FieldName, FieldValue, nFields, &fi);

	if(fi.file_type == TYPE_DIR)
	{
        if(do_delete(pmsg->db, fi.index, &fi, NULL, NULL, pmsg->ext_cb_obj) != RET_SUCCESS)//delete this record in file table
        {
			return 1;
		}
	}
	else
	{
		if(pmsg->callback_fn)
		{
		   
			if(do_delete(pmsg->db, fi.index, &fi, pmsg->update_info, pmsg->callback_fn, pmsg->ext_cb_obj)
					!= RET_SUCCESS)
			{
				return 1;
			}
		}
		else
		{
			if(do_delete(pmsg->db, fi.index, &fi, NULL, NULL, pmsg->ext_cb_obj) != RET_SUCCESS)
			{
			    return 1;
			}
		}	
	}

	if(pmsg->ext_cb_obj && pmsg->ext_cb_obj->del_cb_func)
	{
		pmsg->ext_cb_obj->del_cb_func(&fi, pmsg->ext_cb_obj->del_cb_arg);
	}

	return 0;
}



/*
*description:load traverse cmd to sql cmd buffer.
*input param:	max_len-->buffer size
			id_buf-->buffer storing the ids of all the directory to go through
			cnt-->counts of directories to go through
*output param:sql--> sql buffer
*return : RET_SUCCESS if ok
*/
static error_t load_traverse_cmd(char *sql, int max_len, uint32_t *id_buf, int cnt)
{
    int len;
	int i;
	char id_str[20];

	if(cnt == 0)
	{
		return EINVAL_ARG;
	}

	if(cnt == 1)
	{
		sprintf(sql, "SELECT * FROM %s WHERE PARENT_ID=%u", FILE_TABLE_NAME, id_buf[0]);
		return RET_SUCCESS;
	}
	
    sprintf(sql,"SELECT * FROM %s WHERE PARENT_ID IN (", FILE_TABLE_NAME);
	len = strlen(sql);

    sprintf(id_str, "%u", id_buf[0]);
	strcat(sql,id_str);
	len += strlen(id_str);
	for(i=1;i < cnt;i++)
	{
		sprintf(id_str,",%u",id_buf[i]);
		len += strlen(id_str);
		if(len > max_len)
		{
			return EOUTBOUND;
		}
		strcat(sql,id_str);
	}

	
	len += 1;
	if(len > max_len-1)
	{
		return EOUTBOUND;
	}
	strcat(sql, ")");
	
    return RET_SUCCESS;	
}


static int generic_traverse_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	struct basic_msg *pmsg = (struct basic_msg *)data;
	struct map *search_map = &pmsg->map[pmsg->index];
	struct map *storage_map = &pmsg->map[!pmsg->index];
	file_info_t file_info;

	load_file_item(FieldName, FieldValue, nFields, &file_info);

	if(record_dir_path_handler(&file_info, FieldValue[1], search_map, storage_map)!=RET_SUCCESS)
	{
		return 1;
	}

	if(pmsg->callback && pmsg->arg)
	{
		pmsg->callback(&file_info, pmsg->arg);
	}

	return 0;
}



static error_t traverse_dir_for_ext_cb(sqlite3 *database, file_info_t *pfi, CallBackFunc ext_cb, void *arg)
{
	struct basic_msg msg;	
	char sql[TRAVERSE_BUFF_SIZE*8 + 1];
	error_t errcode = RET_SUCCESS;
	int i,j;

	memset(&msg, 0, sizeof(msg));
	msg.db = database;
	msg.callback = ext_cb;
	msg.arg = arg;
	msg.index = 0;
	msg.map[0].dir_id[0] = pfi->index;
	msg.map[0].path[0] = (char *)malloc(PATH_LENGTH * sizeof(char));
	if(msg.map[0].path[0] == NULL)
	{
		return EMALLOC;
	}
	S_STRNCPY(msg.map[0].path[0], pfi->path, PATH_LENGTH);
	msg.map[0].path_buf_len = 1;
	msg.map[0].dir_cnt = 1;

    while(msg.map[msg.index].dir_cnt != 0)//if dir_id buf is not empty, it means we have directories to traverse
    {
		if((errcode = load_traverse_cmd(sql, TRAVERSE_BUFF_SIZE*8,
				msg.map[msg.index].dir_id, msg.map[msg.index].dir_cnt)) != RET_SUCCESS)
		{
			log_warning("too many directory in same level\n");
			break;
		}
		if((errcode = sqlite3_exec_busy_wait(database,sql,generic_traverse_callback,&msg))
				!= RET_SUCCESS)
		{
			break;
		}
		msg.map[msg.index].dir_cnt = 0;
		msg.index = !msg.index;
	}

	//free malloc memory.
	for(i=0;i<2;i++)
	{
		for(j=0;j<msg.map[i].path_buf_len;j++)
		{
			if(msg.map[i].path[j] != NULL)
			{
				free(msg.map[i].path[j]);
			}
		}
	}

	return errcode;	
	
}


/*
*description: delete directory record in file table
*input param: id-->directory record id.
			 update_info-->update information data buffer
			 clean_up_fn-->clean up callback
return: RET_SUCCESS if ok
*/
static error_t delete_directory(sqlite3 *database, delete_data_t *pdel_data, file_info_t *pfi,
				update_info_t *update_info,delete_callback clean_up_fn)
{
	struct del_msg msg;
	char sql[TRAVERSE_BUFF_SIZE*8 + 1];
	error_t errcode = RET_SUCCESS;
	int i,j;

	//delete the root directory record.
	if((errcode = do_delete(database, pdel_data->target_id, pfi, NULL, NULL, msg.ext_cb_obj))
			!= RET_SUCCESS)
	{
		return errcode;
	}
	
	memset(&msg, 0, sizeof(msg));

	msg.db = database;
	msg.callback_fn = clean_up_fn;
	msg.update_info = update_info;
	msg.ext_cb_obj  = &pdel_data->cb_obj;
	msg.index = 0;
	msg.map[0].dir_id[0] = pdel_data->target_id;
	msg.map[0].path[0] = (char *)malloc(PATH_LENGTH * sizeof(char));
	if(msg.map[0].path[0] == NULL)
	{
		return EMALLOC;
	}
	S_STRNCPY(msg.map[0].path[0], pfi->path, PATH_LENGTH);
	msg.map[0].path_buf_len = 1;
	msg.map[0].dir_cnt = 1;
	
    while(msg.map[msg.index].dir_cnt != 0)//if dir_id buf is not empty, it means we have directories to traverse
    {
		if((errcode = load_traverse_cmd(sql, TRAVERSE_BUFF_SIZE*8,
				msg.map[msg.index].dir_id, msg.map[msg.index].dir_cnt)) != RET_SUCCESS)
		{
			log_warning("too many directory in same level\n");
			break;
		}
		if((errcode = sqlite3_exec_busy_wait(database,sql,delete_child_callback,&msg))
				!= RET_SUCCESS)
		{
			break;
		}
		msg.map[msg.index].dir_cnt = 0;
		msg.index = !msg.index;
	}

	//free malloc memory.
	for(i=0;i<2;i++)
	{
		for(j=0;j<msg.map[i].path_buf_len;j++)
		{
			if(msg.map[i].path[j] != NULL)
			{
				free(msg.map[i].path[j]);
			}
		}
	}

	return errcode;	
}

static error_t dm_delete_directory(sqlite3 *database, delete_data_t *pdel_data)
{
	error_t errcode = RET_SUCCESS;

	//delete the root directory record.
	if((errcode = dm_do_delete(database,pdel_data))
			!= RET_SUCCESS)
	{
		return errcode;
	}
	return errcode;	
}



/*
*description: delete file record in file table
*input param: target-->target id buf.
			 update_info-->update information data buffer
			 clean_up_fn-->clean up callback
return: RET_SUCCESS if ok
*/

static error_t file_table_special_delete(sqlite3 *database, void *target,
										   update_info_t *update_info,
	                                       delete_callback clean_up_fn)
{
	file_info_t fi;
	file_type_t type;
	delete_data_t *pdel_data = (delete_data_t *)target;
	error_t errcode = RET_SUCCESS;

	if(database == NULL || target == NULL)
	{
		return ENULL_POINT;
	}

	if((errcode = query_file_by_id(database, pdel_data->target_id, &fi)) 
			!= RET_SUCCESS)
	{
		return errcode;
	}

	if(pdel_data->user_id != fi.user_info_index)
	{
		return EDB_INVALID_OPERATION;
	}
	
	update_info->user_id = fi.user_info_index;
	update_info->hdisk_id = fi.storage_pos;
#if 0
    // put file into trash
	if(fi.file_state == STATE_NORMAL)
	{
		trash_data_t trash_data;
		
		trash_data.target_id = pdel_data->target_id;
		trash_data.cb_obj    = pdel_data->cb_obj;
		update_info->action = UPDATE_TRASH;
		return file_table_trash(database, &trash_data, update_info);
	}
	else if(fi.file_state == STATE_UNVISIBLE_IN_TRASH)
	{
		return EDB_INVALID_OPERATION;
	}

    // delete file from fs.
	if((errcode = get_file_path(database, fi.index, fi.path)) != RET_SUCCESS)
	{
		return errcode;
	}
#endif
	
 //   update_info->action = UPDATE_DEL_TRASH;
	type = get_file_type(&fi);
	if(type == TYPE_DIR)//if target is directory record
	{
		if((errcode = delete_directory(database, pdel_data, &fi, update_info, clean_up_fn))
				!= RET_SUCCESS)
		{
			return errcode;
		}
	}
	else//target is regular file record
	{
        if((errcode = do_delete(database, pdel_data->target_id, &fi, update_info, clean_up_fn,
				&pdel_data->cb_obj)) != RET_SUCCESS)
        {
			return errcode;
		}
	}

	return UpdateHighLevelItems(database, &fi, 0);//update parent and ancestors
}

static error_t file_table_delete(sqlite3 *database, void *target)
{
	ENTER_FUNC();
	//file_type_t type;
	delete_data_t *pdel_data = (delete_data_t *)target;
	error_t errcode = RET_SUCCESS;

	if(database == NULL || target == NULL)
	{
		return ENULL_POINT;
	}
	if((errcode = dm_do_delete(database, pdel_data))
			!= RET_SUCCESS)
	{
		return errcode;
	}
	EXIT_FUNC();
	return RET_SUCCESS;//update parent and ancestors
}	                           




//generic delete, here we used to delete all the files belong to a certain user when the user is deleted.
static error_t file_table_generic_delete(sqlite3 *database, uint8_t delete_cmd,
									void *target, delete_callback clean_up_fn)
{
	//uint32_t id = *(uint32_t *)target;
	char *sql = get_db_write_task_sql_buf();
	//file_info_t fi;
	struct del_msg msg;
	error_t errcode;

    //Fix bug: delete user's file failed when drop user.

	
	
	return errcode;
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
    pfi->file_state = STATE_NORMAL;

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
    char dir_name_escape[NAME_SIZE+0x10]; // add by wenhao at 2014-11-21 for SQL escape bug.
	char path[PATH_LENGTH];
	int level = 0;
	int i;
    uint32_t parent_id;
	uint32_t id;
	error_t errcode = RET_SUCCESS;

	if(database == NULL || pfi == NULL || last_parent_id == NULL)
	{
		return EINVAL_ARG;
	}

	S_STRNCPY(path, pfi->dir, PATH_LENGTH);
	DMCLOG_D("path = %s",path);
	if((level = get_file_level(path)) <= ROOT_LEVEL)
	{
		return EINVAL_ARG;
	}
	DMCLOG_D("ROOT_LEVEL = %d,level = %d",ROOT_LEVEL,level);
	for(i=ROOT_LEVEL+1,parent_id=ROOT_DIR_ID;i<=level;i++)
	{
		if((errcode = get_level_file_name(path, i, dir_name)) != RET_SUCCESS)
		{
			return errcode;
		}
		DMCLOG_D("dir_name = %s",dir_name);
        sqlite3_str_escape(dir_name, dir_name_escape, sizeof(dir_name_escape));
		
        if((errcode = query_file_id(database, parent_id, dir_name_escape, &id)) != RET_SUCCESS)
		{
			return errcode;
		}

		if(id != INVALID_FILE_ID)//dir exist
		{
			parent_id = id;
			continue;
		}

        //not exist, create it
		S_STRNCPY(pfi->dir, dir_name_escape, PATH_LENGTH);
		pfi->parent_id = parent_id;
		if((errcode = create_dir_node(database, pfi, &parent_id)) != RET_SUCCESS)
		{
			return errcode;
		}
        // modify end.
	}

	*last_parent_id = parent_id;

 	return errcode;
}
#if 0
/*
*description:insert new file record to file table
*input param:item_info-->file information buffer
*return : RET_SUCCESS if ok.
*/
static error_t file_table_insert(sqlite3 *database, void *item_info)
{
    file_info_t *pfi = (file_info_t *)item_info;
    char *sql = get_db_write_task_sql_buf();
	char path_buf[PATH_LENGTH];
    char name[NAME_SIZE];
    char name_escape[NAME_SIZE + 0x10];
	uint32_t parent_id;
	bool parent_exist = TRUE;
	error_t errcode;

    if(database == NULL || pfi == NULL)
    {
        log_warning("file table insert error:invalid parameters\n");
        return EINVAL_ARG;
    }

	//make sure whether parent directory exists
	errcode = get_parent_id(database, pfi->path, &parent_id);
	if(errcode == EDB_RECORD_NOT_EXIST)
	{
	    //parent  doesn't exist
		file_info_t fi_tmp;

		set_file_info_empty(&fi_tmp);
		
		parent_exist = FALSE;
		
		if((errcode = get_parent_dir(pfi->path, fi_tmp.dir)) != RET_SUCCESS)
		{
			return errcode;
		}
		
		set_file_type(&fi_tmp, TYPE_DIR);
		fi_tmp.file_size = 0;
		fi_tmp.create_time = pfi->create_time;

		//create the parent and ancestor directories.
		if((errcode = create_full_path(database, &fi_tmp, &parent_id))
				!= RET_SUCCESS)
		{
			log_warning("create full path error:%s\n", pfi->path);
			return errcode;
		}
	}

    //save path
    if(parent_exist && pfi->file_type != TYPE_DIR)
    {	    	
        const char *p = extract_file_name(pfi->path);
        //log_debug("name len:%d, (%s)", strlen(p), p);
		S_STRNCPY(name, p, NAME_SIZE);
        // modify by wenhao at 2014-11-21 for SQL escape bug.
        sqlite3_str_escape(name, name_escape, sizeof(name_escape));
		if(file_item_exist(database, parent_id, name_escape))//same name item existed!
		{
            // because pfi->path is full path, NAME_SIZE is too small space.
            subfix_file_name_with_time(pfi->path, PATH_LENGTH);
            S_STRNCPY(path_buf, pfi->path, sizeof(path_buf));
            sqlite3_str_escape(extract_file_name(pfi->path), name_escape, sizeof(name_escape));
            S_STRNCPY(pfi->path, name_escape, PATH_LENGTH);
            //pfi->path[PATH_LENGTH - 1] = '\0';
		}
		else
		{
			S_STRNCPY(path_buf, pfi->path, PATH_LENGTH);
			S_STRNCPY(pfi->path, name_escape, PATH_LENGTH); //just save file name for insert
		}

        // modify end.
	}
	else
	{
        // modify by wenhao at 2014-11-21 for SQL escape bug.
        //char name_escape[NAME_SIZE + 0x10];
        S_STRNCPY(path_buf, pfi->path, PATH_LENGTH);
        sqlite3_str_escape(extract_file_name(path_buf), name_escape, sizeof(name_escape));
		S_STRNCPY(pfi->path, name_escape, PATH_LENGTH);
        // modify end.
	}

	//insert
    pfi->parent_id = parent_id;
	pfi->index = alloc_file_id();
	if(pfi->index == 0)
	{
        log_error("db: alloc file id error when insert %s\n",pfi->path);
		return EDB_ID_OVERFLOW;
	}
	
	if((errcode = load_file_insert_cmd(sql, pfi)) != RET_SUCCESS)
	{
		return errcode;
	}

	if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL))
			!= RET_SUCCESS)
	{
	    log_warning("db:insert %s error\n", pfi->path);
        return errcode;
	}

	S_STRNCPY(pfi->path, path_buf, PATH_LENGTH);

	//update parent and ancestors record
	return UpdateHighLevelItems(database, pfi, 1);
}
#endif
/*
*description:insert new file record to file table
*input param:item_info-->file information buffer
*return : RET_SUCCESS if ok.
*/
static error_t dm_file_insert(sqlite3 *database, void *item_info)
{
    file_info_t *pfi = (file_info_t *)item_info;
    char *sql = get_db_write_task_sql_buf();
    char name[NAME_SIZE];
    char name_escape[NAME_SIZE + 0x10];
	uint32_t parent_id;
	bool parent_exist = TRUE;
	error_t errcode;
	
    if(database == NULL || pfi == NULL)
    {
        log_warning("file table insert error:invalid parameters\n");
        return EINVAL_ARG;
    }
	DMCLOG_D("pfi->path = %s",pfi->path);
	//make sure whether parent directory exists
	errcode = get_parent_id(database, pfi->path, &parent_id);
	if(errcode == EDB_RECORD_NOT_EXIST)
	{
	    //parent  doesn't exist
		file_info_t fi_tmp;
		set_file_info_empty(&fi_tmp);
		parent_exist = FALSE;
		if((errcode = get_parent_dir(pfi->path, fi_tmp.dir)) != RET_SUCCESS)
		{
			DMCLOG_D("get_parent_dir error");
			return errcode;
		}
		DMCLOG_D("access get_parent_dir");
		set_file_type(&fi_tmp, TYPE_DIR);
		fi_tmp.file_size = 0;
		fi_tmp.create_time = pfi->create_time;

		//create the parent and ancestor directories.
		if((errcode = create_full_path(database, &fi_tmp, &parent_id))!= RET_SUCCESS)
		{
			log_warning("create full path error:%s\n", pfi->path);
			return errcode;
		}
		DMCLOG_D("create_full_path");
	}

    //save path
    sqlite3_str_escape(pfi->name, pfi->name_escape, sizeof(name_escape));
    if(parent_exist && pfi->file_type != TYPE_DIR)
    {	    	
		if(file_item_exist(database, parent_id, pfi->name_escape))//same name item existed!
		{
            subfix_file_name_with_time(pfi->path, PATH_LENGTH);
		}
        // modify end.
	}
	DMCLOG_D("insert");
	//insert
    pfi->parent_id = parent_id;
	pfi->index = alloc_file_id();
	if(pfi->index == 0)
	{
        log_error("db: alloc file id error when insert %s\n",pfi->path);
		return EDB_ID_OVERFLOW;
	}
	
	if((errcode = load_file_insert_cmd(sql, pfi)) != RET_SUCCESS)
	{
		return errcode;
	}

	if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL))
			!= RET_SUCCESS)
	{
	    log_warning("db:insert %s error\n", pfi->path);
        return errcode;
	}

	//update parent and ancestors record
	return UpdateHighLevelItems(database, pfi, 1);
}
static error_t update_dir_type_by_id(sqlite3 *database, uint32_t id,int file_type)
{
	char *sql = get_db_write_task_sql_buf();
	error_t errcode = RET_SUCCESS;

	sprintf(sql,"UPDATE %s SET TYPE=%d WHERE ID=%u", FILE_TABLE_NAME, file_type, id);

	if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL))
			!= RET_SUCCESS)
	{
		return errcode;	
	}

	return errcode;
}

static error_t dm_scan_file_insert(sqlite3 *database, void *item_info)
{
	ENTER_FUNC();
    file_info_t *pfi = (file_info_t *)item_info;
    char *sql = get_db_write_task_sql_buf();
    char name[NAME_SIZE];
    char name_escape[NAME_SIZE + 0x10];
	error_t errcode = RET_SUCCESS;
	int type = 0;
    if(database == NULL || pfi == NULL)
    {
        log_warning("file table insert error:invalid parameters\n");
        return EINVAL_ARG;
    }
	if(pfi->index != 0)
	{
		if((errcode = get_parent_id(database,pfi->path, &pfi->parent_id)) != RET_SUCCESS)
		{
			log_warning("get parent id error:invalid parameters\n");
			return errcode;
		}
		file_info_t file_info;
		memset(&file_info,0,sizeof(file_info_t));
		//query destination information
	    if((errcode = query_file_by_id(database,pfi->parent_id,&file_info)) != RET_SUCCESS)
	    {
	    	 DMCLOG_D("query file by id error");
			 return EINVAL_ARG;
		}
		DMCLOG_D("pfi->parent_id = %s",pfi->parent_id);//video|audio|picture|document
		if(pfi->file_type == 1)
		{
			//video
			if(!(file_info.file_type>>3&1))
			{
				type = file_info.file_type|1<<3;
			}
		}else if(pfi->file_type == 2)
		{
			//audio
			if(!(file_info.file_type>>2&1))
			{
				type = file_info.file_type|1<<2;
			}
		}else if(pfi->file_type == 3)
		{
			//picture
			if(!(file_info.file_type>>1&1))
			{
				type = file_info.file_type|1<<1;
			}
		}else if(pfi->file_type == 4){
			//document
			if(!(file_info.file_type&1))
			{
				type = file_info.file_type|1;
			}
		}
		update_dir_type_by_id(database, pfi->parent_id,type);
	}
	
    sqlite3_str_escape(pfi->name, pfi->name_escape, sizeof(name_escape));
	pfi->index = alloc_file_id();
	if(pfi->index == 0)
	{
        log_error("db: alloc file id error when insert %s\n",pfi->path);
		return EDB_ID_OVERFLOW;
	}
	
	if((errcode = load_file_insert_cmd(sql, pfi)) != RET_SUCCESS)
	{
		return errcode;
	}

	if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL))!= RET_SUCCESS)
	{
	    log_warning("db:insert %s error\n", pfi->path);
        return errcode;
	}
	EXIT_FUNC();
	//update parent and ancestors record
	return errcode;
}


static struct file_dnode *dm_insert(sqlite3 *database,const char *fullname, const char *name)
{
	int error_t = 0;
	struct stat statbuf;
	struct file_dnode *cur;
	cur = xzalloc(sizeof(*cur));
	if (lstat(fullname, &statbuf)) {
		printf(fullname);
		safe_free(cur);
		return NULL;
	}
	cur->fullname = fullname;
	cur->name = name;
	cur->dn_mode = statbuf.st_mode;
	file_info_t file_info;
	memset(&file_info,0,sizeof(file_info_t));
	file_info.file_size = statbuf.st_size;
	file_info.path = fullname + strlen(DOCUMENT_ROOT);
	file_info.create_time = statbuf.st_ctime ;
	file_info.modify_time = statbuf.st_mtime;
	file_info.name = name;
	strcpy(file_info.file_uuid,"1234567890");
	
	if (S_ISDIR(cur->dn_mode))
	{
		file_info.dir_type = 1;// 1:文件夹,0:普通文件
	}else{
		file_info.dir_type = 0;// 1:文件夹,0:普通文件
		file_info.file_type = db_get_mime_type(fullname,strlen(fullname));//TODO Oliver
	}
	DMCLOG_D("name = %s,dir_type = %d",file_info.name,file_info.dir_type);
	error_t = dm_scan_file_insert(database, &file_info);
	if(error_t < 0)
	{
		DMCLOG_D("file insert failed");
		safe_free(cur);
		return NULL;
	}
	return cur;
}


/* Returns NULL-terminated malloced vector of pointers (or NULL) */
static struct file_dnode **scan_one_dir(sqlite3 *database,const char *path, unsigned *nfiles_p)
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
	while ((entry = readdir(dir)) != NULL) {
		//DMCLOG_D("start:nfiles = %d",nfiles);
		char *fullname;
		/* are we going to list the file- it may be . or .. or a hidden file */
		if (entry->d_name[0] == '.') {
			if ((!entry->d_name[1] || (entry->d_name[1] == '.' && !entry->d_name[2]))) 
			{
				continue;
			}
		}
		fullname = concat_path_file(path, entry->d_name);
		cur = dm_insert(database,fullname, bb_basename(fullname));
		if (!cur) {
			free(fullname);
			continue;
		}
		cur->fname_allocated = 1;
		cur->dn_next = dn;
		dn = cur;
		nfiles++;
		DMCLOG_D("end:nfiles = %d",nfiles);
	}
	DMCLOG_D("end:nfiles = %d",nfiles);
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
}


static void scan_and_display_dirs_recur(sqlite3 *database,struct file_dnode **dn)
{
	unsigned nfiles;
	struct file_dnode **subdnp;
	for (; *dn; dn++) {
		DMCLOG_D("start:%s:", (*dn)->fullname);
		subdnp = scan_one_dir(database,(*dn)->fullname, &nfiles);
		if (nfiles > 0) {
			DMCLOG_D("start....");
			struct file_dnode **dnd;
			unsigned dndirs;
			/* recursive - list the sub-dirs */
			dnd = splitdnarray(subdnp, SPLIT_SUBDIR);
			dndirs = count_dirs(subdnp, SPLIT_SUBDIR);
			DMCLOG_D("dndirs = %d",dndirs);
			if (dndirs > 0) {
				scan_and_display_dirs_recur(database,dnd);
				/* free the array of dnode pointers to the dirs */
				free(dnd);
			}
			/* free the dnodes and the fullname mem */
			dfree(subdnp);
		}
	}
}

static error_t file_table_scan(sqlite3 *database, void *path_info)
{
	ENTER_FUNC();
	error_t errcode = RET_SUCCESS;
    scan_data_t *pfi = (scan_data_t *)path_info;
	DMCLOG_D("pfi->disk_path = %s",pfi->disk_path);
	struct file_dnode **dnp;
    if(database == NULL || pfi == NULL)
    {
        log_warning("file table scanning error:invalid parameters\n");
        return EINVAL_ARG;
    }
	
	struct file_dnode dn;
	memset(&dn,0,sizeof(struct file_dnode));
	dn.fullname = pfi->disk_path;
	dnp = dnalloc(1);
	dnp[0] = &dn;
#if 1
	struct file_dnode *cur;

	file_info_t file_info;
	memset(&file_info,0,sizeof(file_info_t));
	file_info.path = pfi->disk_path;
	file_info.name = bb_basename(file_info.path);
	file_info.file_size = 0;
	file_info.dir_type = 1;
	file_info.parent_id = 0;
	//DMCLOG_D("pfi->file_table_name = %s",pfi->file_table_name);
	errcode = dm_scan_file_insert(database, &file_info);
	if(errcode < 0)
	{
		DMCLOG_D("root file insert failed");
		return -1;;
	}
#endif
	scan_and_display_dirs_recur(database,dnp);
	EXIT_FUNC();
	return 0;
}



/*
*description: insert new file record without update parent and ancestors.
  input param:pfi->buffer storing file information
  output param: pui->buffer storing update information
  return: RET_SUCCESS if OK.
*/
static error_t file_table_direct_insert(sqlite3 *database, file_info_t *pfi,
									update_info_t *pui)
{
    char *sql = get_db_write_task_sql_buf();
	file_type_t type;
	error_t errcode = RET_SUCCESS;
	
    if(database == NULL || pfi == NULL)
    {
        log_warning("Invalid Parameters\n");
        return ENULL_POINT;
    }

	type = get_file_type(pfi);
	#if 0
    if(type == TYPE_VIDEO)//is video file, insert video information record to video table
    {
    	extern db_table_t g_video_table;
        if(g_video_table.ops.insert)
        {
			if((errcode = g_video_table.ops.insert(database, &pfi->media.video_info))
					!= RET_SUCCESS)
			{
			    return errcode;
			}
			pfi->media_info_index = pfi->media.video_info.index;
			pui->video_size += pfi->file_size;
			pui->total_update_size += pfi->file_size;
		}
	}
	else if(type == TYPE_AUDIO)//is audio file, insert audio information record to audio table
	{
	    extern db_table_t g_audio_table;
	    if(g_audio_table.ops.insert)
	    {
		    if((errcode = g_audio_table.ops.insert(database, &pfi->media.audio_info))
					!= RET_SUCCESS)
		    {
			    return errcode;
			}
			pfi->media_info_index = pfi->media.audio_info.index;
			pui->audio_size += pfi->file_size;
			pui->total_update_size += pfi->file_size;
		}
	}
	else if(type == TYPE_IMAGE)//is image file. insert image information record to image table
	{
		extern db_table_t g_image_table;
	    if(g_image_table.ops.insert)
	    {
			if((errcode = g_image_table.ops.insert(database, &pfi->media.image_info))
					!= RET_SUCCESS)
			{
			    return errcode;
			}
			pfi->media_info_index = pfi->media.image_info.index;
			pui->photo_size += pfi->file_size;
			pui->total_update_size += pfi->file_size;
		}  
	}
	#endif
    //insert file base information record to file table
	pfi->index = alloc_file_id();
	if(pfi->index == 0)
	{
        log_error("db: alloc file id error when insert %s\n",pfi->path);
		return EDB_ID_OVERFLOW;
	}
	if((errcode = load_file_insert_cmd(sql, pfi)) != RET_SUCCESS)
	{
	    return errcode;
	}

	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}




/*
*description:rename file record given by  id
* input param:id-->target file id
                     new_name-->buffer storing new file name
* return:RET_SUCCESS if ok.
*/
static error_t file_table_rename_by_id(sqlite3 *database, uint32_t id,
									const char *new_name)
{
	char *sql = get_db_write_task_sql_buf();
	error_t errcode = RET_SUCCESS;

	sprintf(sql,"UPDATE %s SET NAME='%s' WHERE ID=%u", FILE_TABLE_NAME, new_name, id);

	if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL))
			!= RET_SUCCESS)
	{
		return errcode;	
	}

	return errcode;
}


static error_t file_table_rename_by_path(sqlite3 *database, unsigned id,
									const char *des_path)
{
	ENTER_FUNC();
	char *sql = get_db_write_task_sql_buf();
	error_t errcode = RET_SUCCESS;

	sprintf(sql,"UPDATE %s SET NAME='%s',PATH='%s' WHERE ID=%d", FILE_TABLE_NAME, bb_basename(des_path),des_path,id);
	DMCLOG_D("sql = %s",sql);
	if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL))
			!= RET_SUCCESS)
	{
		EXIT_FUNC();
		return errcode;	
	}
	EXIT_FUNC();
	return errcode;
}





//mark a file given by id as trash or normal
static error_t do_mark(sqlite3 *database, file_info_t *pfi, file_state_t state, DBCallBackObj *cb_obj)
{
    char *sql = get_db_write_task_sql_buf();
	error_t errcode;
	
	if(database == NULL || pfi == NULL)
	{
		return EINVAL_ARG;
	}

	sprintf(sql,"UPDATE %s SET STATUS=%d WHERE ID=%u", FILE_TABLE_NAME,
		state, pfi->index);
	
	if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL)) != RET_SUCCESS)
	{
		return errcode;
	}

	//call trash callback 
	if(state != STATE_NORMAL && cb_obj != NULL && cb_obj->trash_cb_func != NULL)
	{
		return cb_obj->trash_cb_func(pfi, cb_obj->trash_cb_arg);
	}

	// call recycle callback
	if(state == STATE_NORMAL && cb_obj != NULL && cb_obj->recover_cb_func != NULL)
	{
		return cb_obj->recover_cb_func(pfi, cb_obj->recover_cb_arg);
	}

	return RET_SUCCESS;
}



//sqlite exec callback for mark file as trash
static int mark_trash_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	struct mark_trash_msg *pmsg = (struct mark_trash_msg*)data;
	struct map *search_map = &pmsg->map[pmsg->index];
	struct map *storage_map = &pmsg->map[!pmsg->index];
	file_info_t file_info;
	
    load_file_item(FieldName, FieldValue, nFields, &file_info);

	if(file_info.file_state== STATE_VISIBLE_IN_TRASH)//this file(or dir) has been already deleted before, keep it in recycle bin
	{
		return 0;
	}

	if(record_dir_path_handler(&file_info, FieldValue[1], search_map, storage_map)!=RET_SUCCESS)
	{
		return 1;
	}
	
	if(do_mark(pmsg->database, &file_info, pmsg->state, pmsg->ext_cb_obj) != RET_SUCCESS)
	{
		return 1;
	}
	
	return 0;
}



/*
*description:mark file as trash or normal
* input param:id-->target file id
  		       type-->file type, we just concern whether it is a directory
  		       mark_trash-->mark trash or normal
* return: RET_SUCCESS if ok.
  		       	
*/
static error_t do_trash_mark(sqlite3 *database, trash_data_t *ptrash_data, file_info_t *pfi, int mark_trash)
{
    file_state_t state = mark_trash?STATE_VISIBLE_IN_TRASH:STATE_NORMAL;
	error_t errcode = RET_SUCCESS;

	//top level item handle
	if((errcode = do_mark(database, pfi, state, &ptrash_data->cb_obj))
			!= RET_SUCCESS)
	{
		return errcode;
	}
	
	if(pfi->file_type == TYPE_DIR)//if dir, go traverse and do mark
	{
	    struct mark_trash_msg msg;
		int i,j;
		char sql[TRAVERSE_BUFF_SIZE*8 + 1];

		memset(&msg, 0, sizeof(msg));

		msg.database = database;
	    msg.state = mark_trash?STATE_UNVISIBLE_IN_TRASH:STATE_NORMAL;
		msg.ext_cb_obj = &ptrash_data->cb_obj;
		msg.map[0].dir_id[0] = ptrash_data->target_id;
		msg.map[0].path[0] = (char *)malloc(PATH_LENGTH * sizeof(char));
		if(msg.map[0].path[0] == NULL)
		{
			return EMALLOC;
		}
		S_STRNCPY(msg.map[0].path[0], pfi->path, PATH_LENGTH);
		msg.map[0].path_buf_len = 1;
		msg.map[0].dir_cnt = 1;

		while(msg.map[msg.index].dir_cnt!= 0)
		{
			if((errcode = load_traverse_cmd(sql, TRAVERSE_BUFF_SIZE*8,
					msg.map[msg.index].dir_id, msg.map[msg.index].dir_cnt)) != RET_SUCCESS)
			{
				log_warning("too many directory in same level\n");
				break;
			}
	        
			if((errcode = sqlite3_exec_busy_wait(database, sql,
					mark_trash_callback, &msg)) != RET_SUCCESS)
			{
				break;
			}
			msg.map[msg.index].dir_cnt = 0;
			msg.index = !msg.index;
		}

		for(i=0;i<2;i++)
		{
			for(j=0;j<msg.map[i].path_buf_len;j++)
			{
				free(msg.map[i].path[j]);
			}
		}
	}

	return errcode;
}
//subfix file name to avoid repeated file name
// eg. test.rmvb-->test-1.rmvb-->test-2.rmvb
static void subfix_file_name(char *name, int n)
{
	char subfix[20]={0};
    char fmt[10]={0};
    int i;

	for(i=0;name[i]!=0;i++)
	{
	    if(name[i]=='.')
	    {
			break;
		}
	}

	if(name[i] == '.')
	{
		S_STRNCPY(fmt, &name[i], sizeof(fmt));
		name[i] = 0;
		sprintf(subfix, "-%d%s",n,fmt);
	}
	else
	{
		sprintf(subfix,"-%d",n);
	}
	
    strcat(name,subfix);
}



/*
*description:change parent directory
  input param:id-->target file id.
                     new_parent_id-->destinated id
  return:RET_SUCCESS if ok.
*/
static error_t change_parent_dir(sqlite3 *database, uint32_t id, uint32_t new_parent_id)
{
	char *sql = get_db_write_task_sql_buf();

	if(database == NULL)
	{
		return EINVAL_ARG;
	}

	sprintf(sql, "UPDATE %s SET PARENT_ID=%u WHERE ID=%u", FILE_TABLE_NAME,
		new_parent_id, id);
	
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}



/*
*description:avoid repeat file name under a directory
  input param:parent_id-->directory id
                     name_buf-->buffer storing name string to check
  output param:name_buf-->if repeated, storing the new name 
  return :TRUE if name repeated, otherwise FALSE
*/
static bool avoid_repeat_name(sqlite3 *database, uint32_t parent_id, char *name_buf)
{
	char name[NAME_SIZE];
	char *pname;
	int n=0;
	bool repeat=FALSE;
	
	pname = name_buf;
	while(file_item_exist(database, parent_id, pname))//if file exist
	{
	   repeat = TRUE;
       n++;
	   S_STRNCPY(name, name_buf, NAME_SIZE);
	   subfix_file_name(name,n);//subfix file name to create a new name.
	   pname = name;
	}

	if(repeat)
	{
		S_STRNCPY(name_buf, name, NAME_SIZE);
	}

	return repeat;
}



static error_t create_recover_dir(sqlite3 *database, uint32_t user_id, uint32_t user_dir_id,
											uint32_t *recover_dir_id)
{
	error_t errcode;
	char *sql = get_db_write_task_sql_buf();
	file_info_t file_info;

	memset(&file_info, 0, sizeof(file_info));

	S_STRNCPY(file_info.path, RECOVER_DIR_NAME, sizeof(file_info.path));
	file_info.parent_id = user_dir_id;
	file_info.user_info_index = user_id;
	set_file_type(&file_info, TYPE_DIR);
	file_info.file_state = STATE_NORMAL;
	file_info.index = alloc_file_id();
	if(file_info.index == 0)
	{
        log_error("db: alloc file id error when insert %s\n", file_info.path);
		return EDB_ID_OVERFLOW;
	}
	
	if((errcode = load_file_insert_cmd(sql, &file_info)) != RET_SUCCESS)
	{
		return errcode;
	}

	if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL))
			!= RET_SUCCESS)
	{
	    log_warning("db:insert %s error\n", file_info.path);
        return errcode;
	}

	*recover_dir_id = file_info.index;
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
	unsigned id = 0;
	if(database == NULL || src_path == NULL || des_path == NULL)
	{
		return EINVAL_ARG;
	}

	if(!file_item_exist_by_path(database, src_path,&id))
	{
		DMCLOG_D("rename file doesn't exist,src_path=%s",src_path);
		return EDB_RECORD_NOT_EXIST;
	}

	return file_table_rename_by_path(database, id, des_path);
}


static uint32_t find_copied_parent(uint32_t parent_id, struct copy_map *map)
{
	int i;

	for(i=0;i<map->cnt;i++)
	{
		if(map->original_id[i] == parent_id)
		{
			return map->copied_id[i];
		}
	}

	return INVALID_FILE_ID;
}


//sqlite exec callback for copy record
static int copy_child_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	struct copy_msg *pmsg = (struct copy_msg *)data;
	struct copy_map *search_map  = &pmsg->copy_map[pmsg->index];
	struct copy_map *storage_map = &pmsg->copy_map[!pmsg->index];
    file_info_t file_info;
	uint32_t copied_parent_id;
	uint32_t id;
	error_t errcode = RET_SUCCESS;

	load_file_item(FieldName, FieldValue, nFields, &file_info);
	id = file_info.index; 
  
    copied_parent_id = find_copied_parent(file_info.parent_id, search_map);
	file_info.parent_id = copied_parent_id;
	if(file_info.parent_id == INVALID_FILE_ID)
	{
		return 1;
	}
	
	if((errcode = file_table_direct_insert(pmsg->database,
			&file_info, pmsg->pui)) != RET_SUCCESS)
	{
		return 1;
	}

	if(file_info.file_type== TYPE_DIR)
	{
	    if(storage_map->cnt > TRAVERSE_BUFF_SIZE)
	    {
			log_warning("too many dir in same level. traverse buffer is used out\n");
			return 1;
		}
        storage_map->original_id[storage_map->cnt] = id;
		storage_map->copied_id[storage_map->cnt++] = file_info.index;      
	}

	return 0;
}


static error_t copy_directory(sqlite3 *database, uint32_t id, file_info_t *pfi, 
								update_info_t *pui)
{
	struct copy_msg msg;
	char sql[TRAVERSE_BUFF_SIZE*8 + 1];
	uint32_t root_copied_dir_id;
	error_t errcode = RET_SUCCESS;

	if((errcode = file_table_direct_insert(database, pfi, msg.pui))
			!= RET_SUCCESS)//firstly, insert the root copy dir.
	{
		return errcode;
	}
	root_copied_dir_id = pfi->index;

	msg.database = database;
	msg.index    = 0;
    msg.copy_map[0].original_id[0] = id;
	msg.copy_map[0].copied_id[0]   = root_copied_dir_id;
	msg.copy_map[0].cnt = 1;
	msg.copy_map[1].cnt = 0;
	msg.pui = pui;
	//traverse and copy child items.
	while(msg.copy_map[msg.index].cnt != 0)
	{
		if((errcode = load_traverse_cmd(sql, TRAVERSE_BUFF_SIZE*8, 
				msg.copy_map[msg.index].original_id, msg.copy_map[msg.index].cnt))
					!= RET_SUCCESS)
		{
			log_warning("copy: too many dir in same level\n");
			return errcode;
		}
		if((errcode = sqlite3_exec_busy_wait(database, sql, 
				copy_child_callback, &msg)) != SQLITE_OK)
		{
			return errcode;
		}
		msg.copy_map[msg.index].cnt = 0;
		msg.index = !msg.index;
	}

	return UpdateHighLevelItems(database, pfi, 1);
}



/*
*description:copy file record
  input param: id-> target file id
  			dest_id->destinated directory id
  output param:pui-->buffer storing update information
  return RET_SUCCESS if ok
*/
static error_t file_table_copy(sqlite3 *database, uint32_t id, uint32_t dest_id,
									update_info_t *pui)
{
	file_info_t file_info;
	error_t errcode;
	
	if(database == NULL || pui == NULL)
	{
		return EINVAL_ARG;
	}

	//query destination information
    if((errcode = query_file_by_id(database,dest_id,&file_info)) != RET_SUCCESS)
    {
		return FALSE;
	}
	pui->user_id    = file_info.user_info_index;
	pui->hdisk_id   = file_info.storage_pos;
	pui->to_user_id = file_info.user_info_index;//record whom the  destination belongs to

    if(get_file_type(&file_info) != TYPE_DIR)
    {
		return EDB_INVALID_OPERATION;
	}

	//query file information	
	if((errcode = query_file_by_id(database, id, &file_info))
			!= RET_SUCCESS)
	{
		return errcode;
	}
	pui->user_id = file_info.user_info_index;//user whom the source file belongs to

    pui->action = UPDATE_ADD;

	avoid_repeat_name(database, dest_id, file_info.path);

	if(file_info.file_type == TYPE_DIR)
	{
		file_info.parent_id = dest_id;
		return copy_directory(database, id, &file_info, pui);
	}
	else
	{
	    file_info.parent_id = dest_id;
	    if((errcode = file_table_direct_insert(database, &file_info, pui)) != RET_SUCCESS)
	    {
			return errcode;
		}
		return UpdateHighLevelItems(database, &file_info, 1);
	} 
}


/*
* description:mark share or not share
* input param:id->target file id 
	               share->share or not share
* output param: share_name->if set share, save the share name in it
* return:RET_SUCCESS if ok
*/
static error_t set_share(sqlite3 *database, uint32_t id, bool share, char *share_name)
{
	char *sql = get_db_write_task_sql_buf();

	if(share)	
	{
		char escape_share_name[SHARE_NAME_SIZE+0x10];

		sqlite3_str_escape(share_name, escape_share_name, sizeof(escape_share_name));
		sprintf(sql, "UPDATE %s SET SHARE=%d,SHARE_NAME='%s' WHERE ID=%u",
			FILE_TABLE_NAME, share, escape_share_name, id);
	}
	else
	{
		sprintf(sql, "UPDATE %s SET SHARE=%d WHERE ID=%u", FILE_TABLE_NAME, share, id);
	}

	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

// add by wenhao
static int _count_share_name_cb(void *context, int nArg, char **azArg, char **azCol)
{
    int *count = (int *)(context);
    *count = strtol(azArg[0], NULL, 10);
    return 0;
}

/*
*description:set file's share state
* input param:id->target file id 
	               share->share or not share
* output param: share_name->if set share, save the share name in it
* return:RET_SUCCESS if ok
*/


static error_t file_table_set_share(sqlite3 *database, set_share_t *pshare_data)
{
	file_info_t file_info;
	error_t errcode = RET_SUCCESS;

	if(database == NULL || pshare_data == NULL)
	{
		return ENULL_POINT;
	}

	memset(&file_info, 0, sizeof(file_info));

	if((errcode = query_file_by_id(database, pshare_data->id, &file_info)) != RET_SUCCESS)
	{
		return errcode;
	}

    // modify by wenaho
	if(file_info.user_info_index != pshare_data->user_id)
	{
    #if 0
		return EDB_INVALID_OPERATION;
    #else
        return EDB_USER_INVALID;
    #endif
	}

	if(file_info.file_state != STATE_NORMAL)
	{
		return EDB_INVALID_OPERATION;
	}

    // the logic is error!!! Need to modify!!! by wenhao!!!
	if(pshare_data->share)
	{
        int count = 0;
        {
            char sql[SQL_CMD_QUERY_BUF_SIZE];
			char share_name[SHARE_NAME_SIZE+0x10];

			sqlite3_str_escape(file_info.path, share_name, sizeof(share_name));      
            snprintf(sql, SQL_CMD_QUERY_BUF_SIZE, "SELECT COUNT(*) FROM %s WHERE SHARE_NAME='%s'",
                    FILE_TABLE_NAME, share_name);
            if((errcode = sqlite3_exec_busy_wait(database, sql, _count_share_name_cb,
                        &count)) != RET_SUCCESS)
                return errcode;
        }

        if(count)
        {
            snprintf(pshare_data->share_name, SHARE_NAME_SIZE, "%s", file_info.path);
		    subfix_file_name_with_time(pshare_data->share_name, SHARE_NAME_SIZE);
        }
        else
        {
            snprintf(pshare_data->share_name, sizeof(pshare_data->share_name),"%s",
                    file_info.path);
        }
		
		if((errcode = get_file_path(database, pshare_data->id, &file_info)) != RET_SUCCESS)
		{
			return errcode;
		}

        snprintf(pshare_data->path, PATH_LENGTH, "%s", file_info.path);
        
	}
	return set_share(database, pshare_data->id, pshare_data->share, pshare_data->share_name);
}



static error_t file_table_update(sqlite3 *database, void *update_info)
{
	file_update_t *file_update = (file_update_t *)update_info;
	file_info_t file_info;
	char *sql = get_db_write_task_sql_buf();
	char tmp_buf[128] = {0};
	//bool first=TRUE;
	error_t errcode = RET_SUCCESS;
	
	if(database == NULL || update_info == NULL)
	{
		return ENULL_POINT;
	}

	if(file_update->file_info.index == INVALID_FILE_ID)
	{
		return EDB_INVALID_ID;
	}

	if(file_update->cmd == 0)
	{
		return RET_SUCCESS;
	}

	if((errcode = query_file_by_id(database, file_update->file_info.index, &file_info))
		    != RET_SUCCESS)
	{
		if(errcode == EDB_RECORD_NOT_EXIST)
		{
			file_update->file_info.index = INVALID_FILE_ID;
		}
	}

	sprintf(sql,"UPDATE %s SET ", FILE_TABLE_NAME);
	if(file_update->cmd & FILE_TABLE_UPDATE_MODIFY_TIME)
	{
		sprintf(tmp_buf, "MODIFY_TIME=%llu, ", file_update->file_info.modify_time);
		strcat(sql, tmp_buf);
	}
	if(file_update->cmd & FILE_TABLE_UPDATE_SIZE)
	{
		sprintf(tmp_buf, "SIZE=%lld, ", file_update->file_info.file_size);
		strcat(sql, tmp_buf);
	}

	sql[strlen(sql)-strlen(", ")] = 0;
	sprintf(tmp_buf, " WHERE ID=%u", file_update->file_info.index);
	strcat(sql,tmp_buf);

	if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL))
			!= RET_SUCCESS)
	{
		return errcode;
	}

	return errcode;
}

extern error_t get_file_path(sqlite3 *database, uint32_t id, file_info_t *pfi);
static int clean_up_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	clean_up_data_t *p = (clean_up_data_t *)data;
	file_info_t file_info;
	uint32_t id;
	error_t errcode;

	id = strtoul(FieldValue[0], NULL, 10);
	if((errcode = get_file_path(p->database, id, &file_info)) != RET_SUCCESS)
	{
		log_warning("get file path in clean up callback error.errcode=0x%X", errcode);
		//return 1;
		//skip it by wenhao
		return 0;
	}
	
	if(p->cb_obj.del_cb_func != NULL)
	{
		p->cb_obj.del_cb_func(&file_info, p->cb_obj.del_cb_arg);
	}

	return 0;
}



static error_t file_table_clean_up(sqlite3 *database, clean_up_data_t *cln_up_data)
{
	char *sql = get_db_write_task_sql_buf();
	error_t errcode;

	if(database == NULL || cln_up_data == NULL)
	{
		return ENULL_POINT;
	}
	
	cln_up_data->database = database;

	sprintf(sql, "SELECT * FROM %s WHERE STATUS=%u", FILE_TABLE_NAME, STATE_NOT_READY);
	if((errcode = sqlite3_exec_busy_wait(database, sql, clean_up_callback, cln_up_data))
			!= RET_SUCCESS)
	{
		return errcode;
	}

	sprintf(sql, "DELETE FROM %s WHERE STATUS=%u", FILE_TABLE_NAME, STATE_NOT_READY);
	if((errcode = sqlite3_exec_busy_wait(database, sql, NULL, NULL)) != RET_SUCCESS)
	{
		log_warning("delete file table record error,errcode=0x%X",errcode);
	}

	return errcode;
}



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
	g_file_table.ops.scan			 = file_table_scan;
    g_file_table.ops.insert          = dm_scan_file_insert;
	g_file_table.ops.generic_delete  = file_table_generic_delete;
	g_file_table.ops.special_delete  = file_table_special_delete;
	g_file_table.ops.dm_delete  		 = file_table_delete;
	//g_file_table.ops.trash           = file_table_trash;
	//g_file_table.ops.recycle         = file_table_recycle;
	//g_file_table.ops.move            = file_table_move;
	g_file_table.ops.rename          = file_table_rename;
	g_file_table.ops.copy            = file_table_copy;
	g_file_table.ops.query           = file_table_query;
	g_file_table.ops.set_share       = file_table_set_share;
	g_file_table.ops.active_update   = file_table_update;  
	g_file_table.ops.clean_up        = file_table_clean_up;
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
