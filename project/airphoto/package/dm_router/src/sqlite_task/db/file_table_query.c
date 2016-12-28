#include "db/nas_db.h"
#include "db/file_table.h"
#include "db/db_mm_pool.h"

const char *g_sort[] = {"ORDER BY NAME ASC",
				  	    "ORDER BY NAME DESC",
				   		"ORDER BY CREATE_TIME ASC",
				   		"ORDER BY CREATE_TIME DESC",
				   		"ORDER BY SIZE ASC",
				   		"ORDER BY SIZE DESC"};


extern error_t query_file_by_id(sqlite3 *database, uint32_t id, file_info_t *pfi);
extern void load_file_item(char **FieldName, char **FieldValue, int nFields, file_info_t *pfi);


static int get_file_list_callback(void *data, int nFields, char **FieldValue, char **FieldName)
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
	plist->result_size += pfi->file_size;
	//DMCLOG_D("plist->result_size = %llu",plist->result_size);
	return 0;
}


static int get_list_count_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	uint32_t *p =(uint32_t *)data;

	if(FieldValue[0] != NULL)
	{
		*p = strtoul(FieldValue[0], NULL, 10);
	}

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

static error_t generate_get_list_sql_cmd(sqlite3 *database, char *sql, 
									const file_list_t *file_list)
{
	char file_type_buf[128] = {0};
	char tail[64];
	error_t errcode;

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


	sprintf(tail,"LIMIT %u OFFSET %u", file_list->len, file_list->start_index);

	//combine all of the condition..
	sprintf(sql, "SELECT * FROM %s WHERE %s %s %s", FILE_TABLE_NAME,
		file_type_buf, g_sort[file_list->sort_mode], tail);
	DMCLOG_D("sql = %s",sql);
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



static error_t get_file_list(sqlite3 *database, file_list_t *file_list)
{
	char sql[SQL_CMD_QUERY_BUF_SIZE];
	error_t errcode;

    file_list->result_cnt = 0;
	dl_list_init(&file_list->head);

	errcode = generate_get_list_sql_cmd(database, sql, file_list);
    if(errcode == EDB_EMPTY_LIST)
    {
        log_notice("get empty list");
        return RET_SUCCESS;
    }
    
	if(errcode != RET_SUCCESS)		
	{
		return errcode;
	}
	DMCLOG_D("access");
	if((errcode = sqlite3_exec_busy_wait(database, sql, get_file_list_callback, file_list))
			!= RET_SUCCESS)
	{
		DMCLOG_D("access");
		file_info_t *item,*n;
		dl_list_for_each_safe(item, n, &file_list->head, file_info_t, next)
	    {
	        free_db_fd(&item);
	    }	
		return errcode;
	}
	DMCLOG_D("access");
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

extern error_t query_file_info(sqlite3 *database, const char *path, const char *name, file_info_t *pfi);
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
		case QUERY_FILE_BY_NAME:
		{
			file_info_t *pfi = (file_info_t *)buf;
			DMCLOG_D("access QUERY_FILE_BY_NAME pfi->name = %s",pfi->name);
			errcode = query_file_info(database, pfi->path, pfi->name, pfi);
			if(errcode == EDB_RECORD_NOT_EXIST)
			{
				log_trace("query file not exist.");
				errcode = RET_SUCCESS;
			}
			break;
		}
		case QUERY_FILE_LIST_COUNT:
		{
			file_list_t *pfile_list = (file_list_t *)buf;

			errcode = get_file_list_count(database, pfile_list);
			break;
		}
		case QUERY_FILE_LIST:
		{
			file_list_t *pfile_list = (file_list_t *)buf;

			errcode = get_file_list(database, pfile_list);
			break;
		}
	
		default :break;
	}

	return errcode;
	
}

