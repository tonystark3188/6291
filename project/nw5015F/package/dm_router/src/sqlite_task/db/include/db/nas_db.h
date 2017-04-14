/*
 * =============================================================================
 *
 *       Filename:  nas_db.h
 *
 *    Description:  database operation interface definition and declaration
 *
 *        Version:  1.0
 *        Created:  2014/09/25 
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Tao Sheng (), vanbreaker90@gmail.com
 *   Organization:  
 *
 * =============================================================================
 */


#ifndef NAS_DB_H
#define NAS_DB_H

#ifdef __cplusplus
extern "C"{
#endif


#include "base.h"
#include "hidisk_errno.h"
#include "db/file_table.h"
#include "db/user_table.h"
#include "db/device_table.h"
#include "db/hdisk_table.h"
#include "db/version_table.h"
#include "db/backup_file_table.h"
#include "db/db_base.h"
#include "db/db_config.h"
#include "db/db_table.h"

#define DB_ERROR_TOLERANCE 5

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


typedef struct
{
	char disk_path[128];  //file information
	//uint32_t user_id;		// which user the file belongs to.
    pthread_mutex_t *mutex;
}scan_data_t;

typedef struct
{
	char disk_path[128];  //disk path
	uint32_t uuid;	    //which hdisk the file belongs to.
}disk_path_data_t;


//data collection of inserting new file record definition.
typedef struct
{
	file_info_t file_info;  //file information
}insert_data_t;


//data collection of file record to rename
typedef struct 
{
	uint32_t target_id;    		  //id of file record to rename
	char	*src_path;
	char	*des_path;
	char     *new_name; // new name
	DBCallBackObj cb_obj;
}rename_data_t;


//data collection of file record to copy
typedef struct
{
	uint32_t target_id;   //id of file record to copy
	uint32_t dest_id;     //destination id
}copy_data_t;




//data collection of full path query.
typedef struct
{
	uint32_t id;             //id of querying file.
	file_info_t file_info;
	//char path[PATH_LENGTH];  //buffer for storing the querying file's full path.
}full_path_t;


//data collection of user list querying
typedef struct
{
	struct dl_list head;//list head for result
	uint32_t result_cnt;//count of users
}user_list_t;


//data collection of device list querying 
typedef struct
{
	struct dl_list head;   //list head for result
	device_info_t  match;  //match condition.
	uint32_t result_cnt;   //count of device 
}device_list_t;

//data collection of file list querying
typedef struct
{
	struct dl_list head; //list head for result
	struct dl_list sur_head;//list head for surplus file's id
	uint32_t user_id;    //user id
	file_search_list_t list_type; //which type of list are we querying
	//uint32_t group_id;            //if we are querying the file list in a certain group, tell the group id
	file_type_t file_type;        //if we are querying the file list of a certain type, tell the type
	file_sort_mode_t sort_mode;   //the sort mode of query result.
	int start_index;         //offset of result set.
	int len;                 //how many results to we want
	uint32_t result_cnt;		  //actually results do we get(in dl_list).
	uint32_t total_cnt;
	uint64_t result_size;
	char *path;
	uint32_t parent_id;
	sqlite3 *database;
	uint32_t curParentId;
	bool curParentAttr;
	char curPath[4096];
}file_list_t;

typedef struct
{
	struct dl_list head; //list head for result
	uint32_t user_id;    //user id
	file_search_list_t list_type; //which type of list are we querying
	//uint32_t group_id;            //if we are querying the file list in a certain group, tell the group id
	file_type_t file_type;        //if we are querying the file list of a certain type, tell the type
	file_sort_mode_t sort_mode;   //the sort mode of query result.
	uint32_t start_index;         //offset of result set.
	uint32_t len;                 //how many results to we want
	uint32_t result_cnt;		  //actually results do we get(in dl_list).
	uint64_t result_size;
}dir_list_t;




typedef  struct
{
	struct dl_list head;
	uint32_t user_id;
	char device_uuid[64];
	backup_file_type_t type;
	file_sort_mode_t sort_mode;   //the sort mode of query result.
	uint32_t start_index;         //offset of result set.
	uint32_t len;                 //how many results to we want
	uint32_t result_cnt;
	uint64_t result_device_size;
	uint64_t result_entire_device_size;
}backup_file_list_t;

#define MAX_ABS_PATH_LIST_ITEM_NUMS  32

typedef struct
{
    uint32_t index;
    char *full_path;
}abs_path_list_item_t;

typedef struct
{
    uint16_t nums;
    abs_path_list_item_t index_arrays[MAX_ABS_PATH_LIST_ITEM_NUMS];
}abs_path_list_t;


//database operation interface definition
typedef struct DB_OprObj
{
	union
	{
		scan_data_t			scan_data;		//using for scanning all disk
		insert_data_t       insert_data;     //using for insert
		delete_data_t  		delete_data;     //using for delete file record and backup file record
		copy_data_t   	    copy_data;       //using for copy record
		rename_data_t  		rename_data;     //using for rename record
		move_data_t  	    move_data;       //using for move data
		user_del_data_t  	user_del_data;   //using for  user delete
		user_info_t 		user_data;       //using for user insert and query.
		user_update_t       user_update_data;//using for change password, invalid or valid user.
		file_info_t         file_data;       //single query
		full_path_t    	    qry_path_data;   //query file's full path
		user_list_t         user_list;       //using for user list query.
		device_info_t       device_data;     //using for device information query and insert
		device_list_t       device_list;	 //using for device list query
		file_list_t         file_list;       //using for file list query
		dir_list_t         dir_list;       //using for file list query
 		file_update_t       file_update_data;
		hdisk_info_t        hdisk_data;      //using for hdisk query and insert
 		version_info_t      version_data;    //using for version info insert and query
 		update_version_t    version_update_data; //using for update version information
 		backup_file_info_t  backup_file_data;//using for insert backup file record
 		backup_file_update_t backup_file_update_data;
 		backup_file_list_t  backup_list;     //using for query backup file list.
 		file_uuid_list_t	file_uuid_list;
        abs_path_list_t     abs_path_list;
		disk_path_data_t	disk_path_date;
	}data;
	DBCallBackObj cb_obj;
	error_t ret;
}DB_OprObj;

error_t db_scan_disk_file(sqlite3 *database, scan_data_t *data);
error_t db_insert_file(sqlite3 *database, insert_data_t *pdata);
error_t db_delete_file(sqlite3 *database, delete_data_t *pdata);
error_t db_move_file(sqlite3 *database, move_data_t *pdata);
error_t db_rename_file(sqlite3 *database, rename_data_t *pdata);
error_t db_update_file(sqlite3 *database, file_update_t *pdata);
error_t db_copy_file(sqlite3 *database, copy_data_t *pdata);
error_t db_delete_file(sqlite3 *database, delete_data_t *pdata);
error_t db_insert_user(sqlite3 *database, user_info_t *pdata);
error_t db_update_user(sqlite3 *database, user_update_t *pdata);
error_t db_delete_user(sqlite3 *database, user_del_data_t *pdata);
error_t db_insert_device(sqlite3 *database, device_info_t *pdi);
error_t db_insert_version(sqlite3 *database, version_info_t *pvi);
error_t db_update_version(sqlite3 *database, update_version_t *puv);
error_t db_insert_hdisk(sqlite3 *database, void *data);
error_t db_reindex_table(sqlite3 *database);
error_t db_query(sqlite3 *database, QueryApproach approach, void *data);
error_t db_module_init(bool hdisk_formated);
void register_db_table_ops(void);

error_t db_commit(sqlite3 *database);


#ifdef __cplusplus
}
#endif



#endif
