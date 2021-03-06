#include "db/nas_db.h"
#include "db/file_table.h"
#include "db/db_mm_pool.h"
#include "disk_manage.h"
#include "defs.h"
// "ORDER BY NAME COLLATE PinYin"
const char *g_sort[] = {"ORDER BY NAME ASC",
				  	    "ORDER BY NAME DESC",
				  	    "ORDER BY PINYIN ASC",
				  	    "ORDER BY PINYIN DESC",
				  	    "ORDER BY SIZE ASC",
				   		"ORDER BY SIZE DESC",
				   		"ORDER BY CHANGE_TIME ASC",
				   		"ORDER BY CHANGE_TIME DESC",
				   		"ORDER BY TYPE ASC",
				   		"ORDER BY TYPE DES"
				   		};




extern error_t query_file_by_id(sqlite3 *database, uint32_t id, file_info_t *pfi);
extern error_t query_file_by_uuid(sqlite3 *database, char * uuid, file_info_t *pfi);

extern void load_file_item(char **FieldName, char **FieldValue, int nFields, file_info_t *pfi);

extern char * db_path_escape(char * path);

extern int file_get_parent_id(sqlite3 *database,char *path,int *parent_id);

static int get_file_list_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	char *parent_path = NULL;
	file_list_t *plist = (file_list_t *)data;
	error_t errcode;
	if(get_fuser_flag() == AIRDISK_ON_PC)
	{
		DMCLOG_D("airdisk is on pc");
		return 1;
	}
	file_info_t *pfi = new_db_fd();
	assert(pfi != NULL);
	
	load_view_item(FieldName, FieldValue, nFields, pfi);
	
	if(plist->curParentId != pfi->parent_id)
	{
		plist->curParentId = pfi->parent_id;
		if((errcode = query_path_by_id(plist->database, plist->curParentId, &parent_path)) == RET_SUCCESS)
		{
			if(parent_path != NULL)
			{
				pfi->path = (char *)calloc(1,strlen(parent_path) + strlen(pfi->name) + 2);
				assert(pfi->path != NULL);
				strcpy(plist->curPath,parent_path);
				sprintf(pfi->path,"%s/%s",parent_path,pfi->name);
				safe_free(parent_path);
			}
		}
	}else{
		pfi->path = (char *)calloc(1,strlen(plist->curPath) + strlen(pfi->name) + 2);
		assert(pfi->path != NULL);
		sprintf(pfi->path,"%s/%s",plist->curPath,pfi->name);
	}
	//DMCLOG_D("pfi->path = %s",pfi->path);
	dl_list_add_tail(&plist->head, &pfi->next);
	plist->result_cnt++;
	return 0;
}

static int get_dir_list_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	file_list_t *plist = (file_list_t *)data;
	char *parent_path = NULL;
	error_t errcode;
	if(get_fuser_flag() == AIRDISK_ON_PC)
	{
		DMCLOG_D("airdisk is on pc");
		return 1;
	}
	file_info_t *pfi = new_db_fd();
	assert(pfi != NULL);
	load_file_item(FieldName, FieldValue, nFields, pfi);
	
	DMCLOG_D("pfi->parent_id = %u",pfi->parent_id);
	if(plist->curParentId != pfi->parent_id)
	{
		plist->curParentId = pfi->parent_id;
		if((errcode = query_path_by_id(plist->database, plist->curParentId, &parent_path)) == RET_SUCCESS)
		{
			if(parent_path != NULL)
			{
				DMCLOG_D("parent_path = %s",parent_path);
				pfi->path = (char *)calloc(1,strlen(parent_path) + strlen(pfi->name) + 2);
				assert(pfi->path != NULL);
				strcpy(plist->curPath,parent_path);
				sprintf(pfi->path,"%s/%s",parent_path,pfi->name);
				safe_free(parent_path);
			}
		}
	}else{
		pfi->path = (char *)calloc(1,strlen(plist->curPath) + strlen(pfi->name) + 2);
		assert(pfi->path != NULL);
		sprintf(pfi->path,"%s/%s",plist->curPath,pfi->name);
	}
	dl_list_add_tail(&plist->head, &pfi->next);
	plist->result_cnt++;
	return 0;
}


static int get_file_list_by_path_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	file_list_t *plist = (file_list_t *)data;
	file_info_t *pfi = NULL;
	error_t errcode;
	pfi = new_db_fd();
	if(pfi == NULL)
	{
		log_warning("new_db_fd failed\n");
		return 1;
	}
	load_file_item(FieldName, FieldValue, nFields, pfi);
	if(plist->file_type != 0)
	{
		pfi->path = (char *)calloc(1,strlen(plist->path) + strlen(pfi->name) + 2);
		assert(pfi->path != NULL);
		sprintf(pfi->path,"%s/%s",plist->path,pfi->name);
	}
	dl_list_add_tail(&plist->head, &pfi->next);
	plist->result_cnt++;
	return 0;
}


static int get_file_list_by_parent_id_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	file_list_t *plist = (file_list_t *)data;
	file_info_t *pfi = NULL;

	pfi = new_db_fd();
	if(pfi == NULL)
	{
		log_warning("new_db_fd failed\n");
		return 1;
	}
	load_file_item(FieldName, FieldValue, nFields, pfi);
	dl_list_add_tail(&plist->head, &pfi->next);
	plist->result_cnt++;
	return 0;
}



static int get_list_count_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	//ENTER_FUNC();
	uint32_t *p =(uint32_t *)data;

	if(FieldValue[0] != NULL)
	{
		*p = strtoul(FieldValue[0], NULL, 10);
	}
	//EXIT_FUNC();
	return 0;
}


error_t get_file_path(sqlite3 *database, uint32_t id, file_info_t *pfi)
{
	//char name_buf[NAME_SIZE]={0};
	error_t errcode = RET_SUCCESS;
	
	if(database == NULL)
	{
		return ENULL_POINT;
	}

	if(id == INVALID_FILE_ID)
	{
		return EDB_INVALID_ID;
	}

	if(id == ROOT_DIR_ID)
	{		
		return query_file_by_id(database, ROOT_DIR_ID, pfi);
	}
	
	if((errcode = query_file_by_id(database, id, pfi)) != RET_SUCCESS)
	{
		return errcode;
	}

	return errcode;//get_file_path_in_fs(pfi);
}



error_t get_file_path_only(sqlite3 *database, uint32_t id, char *path, size_t size)
{
	file_info_t fi;
	error_t errcode = RET_SUCCESS;
	
	if(database == NULL || path == NULL)
	{
		return ENULL_POINT;
	}

    path[0] = '\0';

	if(id == INVALID_FILE_ID)
	{
		return EDB_INVALID_ID;
	}

	if(id == ROOT_DIR_ID)
	{
		S_STRNCPY(path, ROOT_DIR, PATH_LENGTH);
		return RET_SUCCESS;
	}

	memset(&fi,0,sizeof(fi));

	if((errcode = query_file_by_id(database, id, &fi)) != RET_SUCCESS)
	{
		return errcode;
	}

	S_STRNCPY(path, fi.path, size);
	
	return RET_SUCCESS;
}
static error_t generate_get_list_by_parent_id_sql_cmd(sqlite3 *database, char *sql, 
									const file_list_t *file_list)
{
	//combine all of the condition..
	sprintf(sql, "SELECT * FROM %s WHERE PARENT_ID=%u", FILE_TABLE_NAME,file_list->parent_id);
	//DMCLOG_D("sql = %s",sql);
	return RET_SUCCESS;
}

static error_t generate_get_album_by_parent_id_sql_cmd(sqlite3 *database, char *sql, 
									const file_list_t *file_list)
{
	//combine all of the condition..
	sprintf(sql, "SELECT * FROM %s WHERE PARENT_ID=%u AND TYPE=%d", FILE_TABLE_NAME,file_list->parent_id,file_list->file_type);
	//DMCLOG_D("sql = %s",sql);
	return RET_SUCCESS;
}


static error_t generate_get_list_sql_cmd(sqlite3 *database, char *sql, 
									const file_list_t *file_list)
{
	char file_type_buf[128] = {0};
	char encrypt_buf[64] = {0};
	char tail[64];
	error_t errcode;

	if(file_list->encryptType == 1)
	{
		sprintf(encrypt_buf,"%s","AND ENCRYPT_ID = 0");
	}

	switch(file_list->list_type)
	{
		case TYPE_ALL: sprintf(file_type_buf, "TYPE=%d ", file_list->file_type);
						break;
		case TYPE_AUDIO:
		case TYPE_VIDEO:
		case TYPE_IMAGE:
		case TYPE_DOCUMENT:
		case TYPE_ARCHIVE:
        case TYPE_DIR:
        case TYPE_UNKNOWN:  sprintf(file_type_buf, "TYPE=%d ", file_list->file_type);
							sprintf(tail,"LIMIT %u OFFSET %u", file_list->len, file_list->start_index);
							break;
		default:            break;
	}	

	//combine all of the condition..
	sprintf(sql, "SELECT * FROM %s WHERE %s %s %s %s ", FILE_TABLE_NAME,
		file_type_buf, encrypt_buf,g_sort[file_list->sort_mode], tail);
	//SELECT * FROM file_table WHERE TYPE=3 ORDER BY MODIFY_TIME ASC
	return RET_SUCCESS;
}

static error_t generate_get_list_cnt_by_view_sql_cmd(sqlite3 *database, char *sql, 
									const file_list_t *file_list)
{
	char tail[64] = {0};
	char file_type_buf[64] = {0};
	switch(file_list->file_type)
	{
		case TYPE_ALL: 
		case TYPE_AUDIO:
			sprintf(file_type_buf, "%s ", FILE_AUDIO_VIEW);
			break;
		case TYPE_VIDEO:
			sprintf(file_type_buf, "%s ", FILE_VIDEO_VIEW);
			break;
		case TYPE_IMAGE:
			sprintf(file_type_buf, "%s ", FILE_IMAGE_VIEW);
			break;
		case TYPE_DOCUMENT:
			sprintf(file_type_buf, "%s ", FILE_DOCUM_VIEW);
			break;
		case TYPE_ARCHIVE:
        case TYPE_DIR:
        case TYPE_UNKNOWN: 
		default:            break;
		
	}	
	sprintf(sql,"SELECT COUNT(*) FROM %s",file_type_buf);
	return RET_SUCCESS;
}




static error_t generate_get_list_by_view_sql_cmd(sqlite3 *database, char *sql, 
									const file_list_t *file_list)
{
	char tail[64] = {0};
	char file_type_buf[64] = {0};
	char encrypt_buf[64] = {0};
	DMCLOG_D("file_list->len = %d",file_list->len);
	if(file_list->len > 0)
	{
		sprintf(tail,"LIMIT %u OFFSET %u", file_list->len, file_list->start_index);
	}
    
	
	if(file_list->encryptType == 1)
	{
		sprintf(encrypt_buf,"%s"," WHERE ENCRYPT_ID = 0");
	}
	
	switch(file_list->file_type)
	{
		case TYPE_ALL: 
		case TYPE_AUDIO:
			sprintf(file_type_buf, "%s ", FILE_AUDIO_VIEW);
			break;
		case TYPE_VIDEO:
			sprintf(file_type_buf, "%s ", FILE_VIDEO_VIEW);
			break;
		case TYPE_IMAGE:
			sprintf(file_type_buf, "%s ", FILE_IMAGE_VIEW);
			break;
		case TYPE_DOCUMENT:
			sprintf(file_type_buf, "%s ", FILE_DOCUM_VIEW);
			break;
		case TYPE_ARCHIVE:
        case TYPE_DIR:
        case TYPE_UNKNOWN: 
		default:            break;
		
	}	
	sprintf(sql,"SELECT * FROM %s %s %s %s",file_type_buf,encrypt_buf, g_sort[file_list->sort_mode],tail);
	return RET_SUCCESS;
}



static error_t generate_get_dir_list_sql_cmd(sqlite3 *database, char *sql, 
									const file_list_t *file_list)
{
	char file_type_buf[64] = {0};
	switch(file_list->file_type)
	{
		case TYPE_ALL: 
		case TYPE_AUDIO:
			sprintf(file_type_buf, "%s ", FILE_AUDIO_VIEW);
			break;
		case TYPE_VIDEO:
			sprintf(file_type_buf, "%s ", FILE_VIDEO_VIEW);
			break;
		case TYPE_IMAGE:
			sprintf(file_type_buf, "%s ", FILE_IMAGE_VIEW);
			break;
		case TYPE_DOCUMENT:
			sprintf(file_type_buf, "%s ", FILE_DOCUM_VIEW);
			break;
		case TYPE_ARCHIVE:
        case TYPE_DIR:
        case TYPE_UNKNOWN: 
		default:            break;
		
	}	
	DMCLOG_D("sort = %s",g_sort[file_list->sort_mode] + 9);
	sprintf(sql,"SELECT * FROM %s ORDER BY PARENT_ID ASC,%s",file_type_buf, g_sort[file_list->sort_mode] + 9);
	DMCLOG_D("sql = %s",sql);
	return RET_SUCCESS;
}

static error_t generate_get_list_by_path_sql_cmd(sqlite3 *database, char *sql, 
									const file_list_t *file_list)
{
    error_t errcode;
	char file_type_buf[128] = {0};
    char tail[64] = {0};
    char parent_folder_buf[256] = {0};
    char encrypt_buf[64] = {0};
	char *path_escape = db_path_escape(file_list->path);

	if(file_list->encryptType == 1)
	{
		sprintf(encrypt_buf,"%s","AND ENCRYPT_ID = 0");
	}
	
    sprintf(parent_folder_buf, "(PARENT_ID in (SELECT distinct ID FROM '%s' WHERE PATH = '%s'))", FILE_TABLE_NAME,path_escape);

	if(file_list->file_type > 0)
	{
		sprintf(file_type_buf, "AND TYPE=%d ", file_list->file_type);
	}
	
	/*switch(file_list->list_type)
	{
		case TYPE_ALL:sprintf(file_type_buf, "AND TYPE=%d ", file_list->file_type);
						break;
			
		case TYPE_AUDIO:
		case TYPE_VIDEO:
		case TYPE_IMAGE:
		case TYPE_DOCUMENT:
		case TYPE_ARCHIVE:
        case TYPE_DIR:
        case TYPE_UNKNOWN:  sprintf(file_type_buf, "AND TYPE=%d ", file_list->file_type);
							sprintf(tail,"LIMIT %u OFFSET %u", file_list->len, file_list->start_index);
							break;
		default:            break;
	}*/	
	
	//combine all of the condition..
	sprintf(sql, "SELECT * FROM %s WHERE %s %s %s %s %s", FILE_TABLE_NAME,parent_folder_buf,
		encrypt_buf,file_type_buf, g_sort[file_list->sort_mode], tail);
	safe_free(path_escape);
	return RET_SUCCESS;
}

static error_t generate_get_list_cnt_sql_cmd(sqlite3 *database, char *sql,
									const file_list_t *file_list)
{
	char file_type_buf[32] = {0};
	if(file_list->list_type != LIST_GROUP)
	{
		switch(file_list->file_type)
		{
			case TYPE_ALL: 
			case TYPE_AUDIO:
			case TYPE_VIDEO:
			case TYPE_IMAGE:
			case TYPE_DOCUMENT:
			case TYPE_ARCHIVE: 
            case TYPE_DIR:
            case TYPE_UNKNOWN:  sprintf(file_type_buf, "TYPE=%d ", file_list->file_type);
								break;
			default:            break;
		}	
	}


	//combine all of the condition..
	sprintf(sql, "SELECT COUNT(*) FROM %s WHERE %s", FILE_TABLE_NAME, file_type_buf);

	return RET_SUCCESS;
}
static error_t generate_get_dir_list_cnt_sql_cmd(sqlite3 *database, char *sql,
									const file_list_t *file_list)
{
	error_t errcode;
	char file_type_buf[128] = {0};
    char tail[64] = {0};
    char parent_folder_buf[256] = {0};
    sprintf(parent_folder_buf, "(ID in (SELECT distinct PARENT_ID FROM %s WHERE TYPE = %d))", FILE_TABLE_NAME,file_list->file_type);
//    select * from file where (file_id in (select distinct file_folder_id from file where file_type = 1)) or file_type = 1
	/*if(file_list->list_type != LIST_GROUP)
	{
		switch(file_list->file_type)
		{
			case TYPE_ALL: 
			case TYPE_AUDIO:
			case TYPE_VIDEO:
			case TYPE_IMAGE:
			case TYPE_DOCUMENT:
			case TYPE_ARCHIVE:
            case TYPE_DIR:
            case TYPE_UNKNOWN:  sprintf(file_type_buf, "AND TYPE=%d ", file_list->file_type);
								break;
			default:            break;
		}	
	}*/
	//combine all of the condition..
	sprintf(sql, "SELECT COUNT(*) FROM %s WHERE %s ", FILE_TABLE_NAME,parent_folder_buf);
	DMCLOG_D("sql = %s",sql);
	return RET_SUCCESS;
}

static error_t generate_get_list_dir_by_parent_id_sql_path(sqlite3 *database, char *sql,
									const file_list_t *file_list)
{
	ENTER_FUNC();
    error_t errcode;
    char parent_folder_buf[256] = {0};
    sprintf(parent_folder_buf, "(PARENT_ID in (SELECT distinct ID FROM '%s' WHERE PATH = '%s'))", FILE_TABLE_NAME,file_list->path);

	//combine all of the condition..
	sprintf(sql, "SELECT * FROM %s WHERE %s %s", FILE_TABLE_NAME,parent_folder_buf);
	DMCLOG_D("sql = %s",sql);
	EXIT_FUNC();
	return RET_SUCCESS;
}
static error_t generate_get_list_cnt_by_path_sql_cmd(sqlite3 *database, char *sql,
									const file_list_t *file_list)
{
	error_t errcode;
	char file_type_buf[128] = {0};
    char parent_folder_buf[256] = {0};
	char *path_escape = db_path_escape(file_list->path);
	sprintf(parent_folder_buf, "(PARENT_ID in (SELECT distinct ID FROM %s WHERE PATH='%s'))", FILE_TABLE_NAME,path_escape);

	if(file_list->file_type != 0)
	{
		if(file_list->list_type != LIST_GROUP)
		{
			switch(file_list->file_type)
			{
				case TYPE_ALL: 
				case TYPE_AUDIO:
				case TYPE_VIDEO:
				case TYPE_IMAGE:
				case TYPE_DOCUMENT:
				case TYPE_ARCHIVE:
	            case TYPE_DIR:
	            case TYPE_UNKNOWN:  sprintf(file_type_buf, "AND TYPE=%d ", file_list->file_type);
									break;
				default:            break;
			}	
		}
	}
	
	//combine all of the condition..
	sprintf(sql, "SELECT COUNT(*) FROM %s WHERE %s %s", FILE_TABLE_NAME, parent_folder_buf,file_type_buf);
	safe_free(path_escape);
	return RET_SUCCESS;
}

static error_t generate_get_list_size_by_path_sql_cmd(sqlite3 *database, char *sql,
									const file_list_t *file_list)
{
	error_t errcode;
	char file_type_buf[128] = {0};
    char parent_folder_buf[256] = {0};
	char *path_escape = db_path_escape(file_list->path);
    sprintf(parent_folder_buf, "(PARENT_ID in (SELECT distinct ID FROM %s WHERE PATH='%s'))", FILE_TABLE_NAME,path_escape);
	if(file_list->list_type != LIST_GROUP)
	{
		switch(file_list->file_type)
		{
			case TYPE_ALL: 
			case TYPE_AUDIO:
			case TYPE_VIDEO:
			case TYPE_IMAGE:
			case TYPE_DOCUMENT:
			case TYPE_ARCHIVE:
            case TYPE_DIR:
            case TYPE_UNKNOWN:  sprintf(file_type_buf, "AND TYPE=%d ", file_list->file_type);
								break;
			default:            break;
		}	
	}
	//combine all of the condition..
	sprintf(sql, "SELECT SUM(SIZE) FROM %s WHERE %s %s", FILE_TABLE_NAME, parent_folder_buf,file_type_buf);
	safe_free(path_escape);
	return RET_SUCCESS;
}

static error_t get_file_list(sqlite3 *database, file_list_t *file_list)
{
	ENTER_FUNC();
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode;
	file_info_t *item,*n;
    file_list->result_cnt = 0;

	//dl_list_init(&file_list->head);

	errcode = generate_get_list_by_view_sql_cmd(database, sql, file_list);
    if(errcode == EDB_EMPTY_LIST)
    {
        log_notice("get empty list");
        return RET_SUCCESS;
    }
	
	file_list->database = database;
	
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_file_list_callback, file_list))
			!= RET_SUCCESS)
	{
		#if 0
		dl_list_for_each_safe(item, n, &file_list->head, file_info_t, next)
	    {
	    	dl_list_del(&item->next);
	        free_db_fd(&item);
	    }
		#endif
		DMCLOG_E("get file list error");
		return errcode;
	}

	
	/*errcode = generate_get_list_cnt_by_view_sql_cmd(database, sql, file_list);
    if(errcode == EDB_EMPTY_LIST)
    {
        log_notice("get empty list");
        return RET_SUCCESS;
    }
	
	file_list->total_cnt = 0;
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_list_count_callback, &file_list->total_cnt))
			!= RET_SUCCESS)
	{
			
		return errcode;
	}
	DMCLOG_D("file_list->total_cnt = %u",file_list->total_cnt);*/
	EXIT_FUNC();
	return errcode;
}

error_t get_file_list_by_parent_id(sqlite3 *database, file_list_t *file_list)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode;
	errcode = generate_get_list_by_parent_id_sql_cmd(database, sql, file_list);
    if(errcode == EDB_EMPTY_LIST)
    {
        log_notice("get empty list");
        return RET_SUCCESS;
    }
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_file_list_by_parent_id_callback, file_list))
			!= RET_SUCCESS)
	{	
		return errcode;
	}
	return errcode;
}

error_t get_file_album_by_parent_id(sqlite3 *database, file_list_t *file_list)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode;
	errcode = generate_get_album_by_parent_id_sql_cmd(database, sql, file_list);
    if(errcode == EDB_EMPTY_LIST)
    {
        log_notice("get empty list");
        return RET_SUCCESS;
    }
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_file_list_by_parent_id_callback, file_list))
			!= RET_SUCCESS)
	{	
		return errcode;
	}
	return errcode;
}



static error_t get_dir_list(sqlite3 *database, file_list_t *file_list)
{
	ENTER_FUNC();
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode;

    file_list->result_cnt = 0;
	dl_list_init(&file_list->head);

	errcode = generate_get_dir_list_sql_cmd(database, sql, file_list);
    if(errcode == EDB_EMPTY_LIST)
    {
        log_notice("get empty list");
        return RET_SUCCESS;
    }
	file_list->database = database;
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_dir_list_callback, file_list))
			!= RET_SUCCESS)
	{
		file_info_t *item,*n;
		dl_list_for_each_safe(item, n, &file_list->head, file_info_t, next)
	    {
	        free_db_fd(&item);
	    }	
		return errcode;
	}
	EXIT_FUNC();
	return errcode;
}


static error_t get_file_list_by_path(sqlite3 *database, file_list_t *file_list)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode;

    file_list->result_cnt = 0;
	dl_list_init(&file_list->head);
	file_info_t *item,*n;

	errcode = generate_get_list_by_path_sql_cmd(database, sql, file_list);
	//DMCLOG_D("sql=%s",sql);
    if(errcode == EDB_EMPTY_LIST)
    {
        DMCLOG_D("get empty list");
        return RET_SUCCESS;
    }
    
	if(errcode != RET_SUCCESS)		
	{
	   DMCLOG_D("generate err");
		return errcode;
	}
	
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_file_list_by_path_callback, file_list))
			!= RET_SUCCESS)
	{
		dl_list_for_each_safe(item, n, &file_list->head, file_info_t, next)
	    {
	        free_db_fd(&item);
	    }	
		return errcode;
	}
    EXIT_FUNC();
	return errcode;
}




static error_t get_file_list_count(sqlite3 *database, file_list_t *plist)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode;

    plist->result_cnt = 0;
	
	errcode = generate_get_list_cnt_sql_cmd(database, sql, plist);
    if(errcode == EDB_EMPTY_LIST)
    {
        log_notice("get empty list");
        return RET_SUCCESS;
    }
    
	if(errcode != RET_SUCCESS)		
	{
		return errcode;    
	}

	return sqlite3_exec_busy_wait(database, sql, get_list_count_callback, &plist->result_cnt);	
}

static error_t get_file_dir_list_count(sqlite3 *database, file_list_t *plist)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode;

    plist->result_cnt = 0;
	
	errcode = generate_get_dir_list_cnt_sql_cmd(database, sql, plist);
    if(errcode == EDB_EMPTY_LIST)
    {
        log_notice("get empty list");
        return RET_SUCCESS;
    }
    
	if(errcode != RET_SUCCESS)		
	{
		return errcode;    
	}

	return sqlite3_exec_busy_wait(database, sql, get_list_count_callback, &plist->result_cnt);	
}

static error_t get_file_list_count_by_path(sqlite3 *database, file_list_t *plist)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode;

    plist->result_cnt = 0;
	
	errcode = generate_get_list_cnt_by_path_sql_cmd(database, sql, plist);
    if(errcode == EDB_EMPTY_LIST)
    {
        log_notice("get empty list");
        return RET_SUCCESS;
    }
    
	if(errcode != RET_SUCCESS)		
	{
		return errcode;    
	}

	return sqlite3_exec_busy_wait(database, sql, get_list_count_callback, &plist->result_cnt);	
}

static error_t get_file_list_size_by_path(sqlite3 *database, file_list_t *plist)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode;

    plist->result_size = 0;
	
	errcode = generate_get_list_size_by_path_sql_cmd(database, sql, plist);
    if(errcode == EDB_EMPTY_LIST)
    {
        log_notice("get empty list");
        return RET_SUCCESS;
    }
    
	if(errcode != RET_SUCCESS)		
	{
		return errcode;    
	}

	return sqlite3_exec_busy_wait(database, sql, get_list_count_callback, &plist->result_size);	
}


extern error_t query_file_info_by_parentid_and_name(sqlite3 *database,unsigned parent_id,char *name,file_info_t *pfi);

error_t file_table_query(sqlite3 *database, QueryApproach approach, void *buf)
{
	error_t errcode = RET_SUCCESS;
	
	if(database == NULL || buf == NULL)
	{
		return ENULL_POINT;
	}

	switch(approach)
	{
		case QUERY_FILE_PATH:
		{
			full_path_t *pfull_path = (full_path_t *)buf;

			errcode = get_file_path(database, pfull_path->id, &pfull_path->file_info);
			break;
		}
		case QUERY_FILE_INFO:
		{
			file_info_t *pfi = (file_info_t *)buf;

			errcode = query_file_by_id(database, pfi->index, pfi);
			if(errcode == EDB_RECORD_NOT_EXIST)
			{
				log_trace("query file not exist.");
				errcode = RET_SUCCESS;
			}
			break;
		}
		case QUERY_FILE_BY_UUID:
		{
			file_info_t *pfi = (file_info_t *)buf;
			DMCLOG_D("file_uuid = %s",pfi->file_uuid);
			errcode = query_file_by_uuid(database, pfi->file_uuid, pfi);
			break;
		}
		case QUERY_FILE_BY_NAME:
		case QUERY_FILE_BY_PATH:		
		{
			//DMCLOG_D("QUERY_FILE_BY_PATH");
			file_info_t *pfi = (file_info_t *)buf;
			int parent_id;
			errcode = file_get_parent_id(database,pfi->path,&parent_id);
			if(errcode != RET_SUCCESS)
			{
				DMCLOG_D("rename file doesn't exist,src_path=%s",pfi->path);
				return EDB_RECORD_NOT_EXIST;
			}
			char *name = bb_basename(pfi->path);
			errcode = query_file_info_by_parentid_and_name(database,parent_id,name, pfi);
			if(errcode == EDB_RECORD_NOT_EXIST)
			{
				log_trace("query file not exist.");
				errcode = RET_SUCCESS;
			}
			//DMCLOG_D("time=%ld,size=%ld",pfi->modify_time,pfi->file_size);
			break;
		}
		case QUERY_ENCRYPT_FILE_BY_PATH:
		{
			file_info_t *pfi = (file_info_t *)buf;
			int parent_id;
			//DMCLOG_D("pfi->path=%s",pfi->path);
			//根据文件路径，查找到父目录ID
			errcode = file_get_parent_id(database,pfi->path,&parent_id);
			if(errcode != RET_SUCCESS)
			{
				DMCLOG_D("file doesn't exist,src_path=%s",pfi->path);
				return EDB_RECORD_NOT_EXIST;
			}
            //DMCLOG_D("parent_id=%d",parent_id);
            file_info_t parentInfo;
            memset(&parentInfo,0,sizeof(file_info_t));
            //根据父目录ID 找到父目录加密ID
            errcode = query_file_by_id(database, parent_id, &parentInfo);
            if(errcode == EDB_RECORD_NOT_EXIST)
            {
                DMCLOG_E("query file not exist.");
                errcode = RET_SUCCESS;
                return EDB_RECORD_NOT_EXIST;
            }
            //DMCLOG_D("parentInfo.encryptId=%d",parentInfo.encryptId);            
            encrypt_info_t pInfo;
            memset(&pInfo,0,sizeof(encrypt_info_t));
            pInfo.id = parentInfo.encryptId;
           // DMCLOG_D("parentInfo.encryptId=%d",parentInfo.encryptId);
            if(parentInfo.encryptId == 0){
			    errcode = RET_SUCCESS;
		        DMCLOG_D("query file not exist.");
                return EDB_RECORD_NOT_EXIST;
            }
            //根据父目录加密ID，找到对应真实父目录PATH
            errcode = encrypt_table_query(database, &pInfo);
            if(errcode == EDB_RECORD_NOT_EXIST)
            {
                log_trace("query file not exist.");
                //errcode = RET_SUCCESS;
                return errcode;
            }
           // DMCLOG_D("pInfo.name=%s,pInfo.path=%s",pInfo.name,pInfo.path); 
            //char name[MAX_ENCRYPT_NAME_SIZE]={0};
			//strcpy(name,bb_basename(pfi->path));
			char *name = bb_basename(pfi->path); 
			//sprintf(pfi->encrypt_info.path,"%s/%s",pfi.path,pfi->encrypt_info.name);
			//sprintf(pfi->encrypt_info.path,"%s%s/%s",DOCUMENT_ROOT,pInfo.path,name);
			//DMCLOG_D("pfi->encrypt_info.path=%s",pfi->encrypt_info.path);

			//根据parent_id 和名字 锁定到文件加密后的ID
			errcode = query_file_info_by_parentid_and_name(database,parent_id,name, pfi);
			if(errcode == EDB_RECORD_NOT_EXIST)
			{
				log_trace("query file not exist.");
				return errcode;
			}
 			//得到加密后的文件ID
            pfi->encrypt_info.id = pfi->encryptId;

    		//根据加密后的文件ID，可锁定加密后的文件名
            errcode = encrypt_table_query(database, &pfi->encrypt_info);
            if(errcode == EDB_RECORD_NOT_EXIST)
            {
                DMCLOG_D("query file not exist.");
                return errcode;
            }
            //最后，拼装加密后的name和加密后的path，返回。
           // DMCLOG_D("pfi->encrypt_info.name = %s",pfi->encrypt_info.name);
            //如果是文件，而不是文件夹
            if(pfi->encrypt_info.path == NULL)
	        {
	        	pfi->encrypt_info.path = (char*)calloc(1,strlen(pInfo.path)+strlen(pfi->encrypt_info.name)+2);
	        	sprintf(pfi->encrypt_info.path,"%s/%s",pInfo.path,pfi->encrypt_info.name);
	        //	DMCLOG_D("pfi->encrypt_info.path = %s",pfi->encrypt_info.path);
	        }
 /*
            sprintf(pfi->encrypt_info.path,"%s%s/%s",DOCUMENT_ROOT,pInfo.path,pfi->encrypt_info.name);
            DMCLOG_D("pfi->encrypt_info.path = %s",pfi->encrypt_info.path);
            //sprintf(pfi->encrypt_info.path,"%s/%s",pfi->.path,pfi->encrypt_info.name);*/
 /*         //查询父目录加密后的PATH
         
 			errcode = encrypt_query_parent_path_by_id(database, &pfi->encrypt_info.parent_id,pfi->encrypt_info.path);           
			if(errcode == EDB_RECORD_NOT_EXIST)
            {
                DMCLOG_D("query file not exist.");
                errcode = RET_SUCCESS;
            }
*/         //DMCLOG_D("pfi->encrypt_info.name = %s",pfi->encrypt_info.name);
            //sprintf(pfi->encrypt_info.path,"%s/%s",pfi->.path,pfi->encrypt_info.name);
            //DMCLOG_D("pfi->encrypt_info.path = %s",pfi->encrypt_info.path);
			break;
		}
		case QUERY_ENCRYPT_FILE_BY_NAME:
		{
			file_info_t *pfi = (file_info_t *)buf;
			int id;
			//DMCLOG_D("pfi->name=%s",pfi->name);
			//1.根据name，反查encrypt_table的id,即file_table 的encrypt_id
			errcode = encrypt_get_encrypt_id_by_name(database,pfi->name,&id);
			if(errcode != RET_SUCCESS)
			{
				DMCLOG_D("file doesn't exist,name=%s",pfi->name);
				return EDB_RECORD_NOT_EXIST;
			}
			//2.根据file_table 中 encrypt_id，查到name，返回。
			errcode = file_get_name_by_encrypt_id(database,id,&pfi->encrypt_info.name);
			if(errcode != RET_SUCCESS)
			{
				DMCLOG_D("file doesn't exist,src_path=%s",pfi->name);
				return EDB_RECORD_NOT_EXIST;
			}
			//DMCLOG_D("pfi->encrypt_info.name=%s",pfi->encrypt_info.name);
			break;
		}
		
		case QUERY_FILE_LIST_COUNT:
		{
			file_list_t *pfile_list = (file_list_t *)buf;

			errcode = get_file_list_count(database, pfile_list);
			break;
		}
		case QUERY_FILE_DIR_LIST_COUNT:
		{
			file_list_t *pfile_list = (file_list_t *)buf;

			errcode = get_file_dir_list_count(database, pfile_list);
			break;
		}
		case QUERY_FILE_LIST_COUNT_BY_PATH:
		{
			ENTER_FUNC();
			file_list_t *pfile_list = (file_list_t *)buf;

			errcode = get_file_list_count_by_path(database, pfile_list);
			EXIT_FUNC();
			//errcode = get_file_list_size_by_path(database, pfile_list);
			break;
		}
		case QUERY_FILE_LIST:
		{
			file_list_t *pfile_list = (file_list_t *)buf;
			errcode = get_file_list(database, pfile_list);
			break;
		}
		case QUERY_DIR_LIST:
		{
			file_list_t *pfile_list = (file_list_t *)buf;
			errcode = get_dir_list(database, pfile_list);
			break;
		}
		case QUERY_FILE_LIST_BY_PATH:
		{
			file_list_t *pfile_list = (file_list_t *)buf;

			errcode = get_file_list_by_path(database, pfile_list);
			break;
		}
	
		default :break;
	}
	return errcode;
	
}

