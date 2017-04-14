#ifndef DB_TABLE_H
#define DB_TABLE_H

#include "hidisk_errno.h"
#include "db/db_base.h"
#include <stdbool.h>


#define NAME_SIZE               256 //128
#define UNIQUE_NAME_SIZE        (NAME_SIZE+30)
#define TRASH_NAME_SIZE         32
#define SHARE_NAME_SIZE         (NAME_SIZE+30)
#define BACKUP_NAME_SIZE        SHARE_NAME_SIZE
#define BACKUP_PATH_SIZE        512
#define PATH_LENGTH             2048


#define  SEPERATOR '/'


#define  UPDATE_DEL_TRASH      0
#define  UPDATE_DEL_NORM       1
#define  UPDATE_ADD            2
#define  UPDATE_TRASH          3
#define  UPDATE_RECYCLE        4
#define  UPDATE_DELETE_USER    5
#define  UPDATE_REMOUNT		   6

#define  CLEAN_UP_FILE_TABLE      0
#define  CLEAN_UP_BKP_FILE_TABLE  1


//update table structure def
//used when we delete old file records or insert new file records
//eg. when we delete some old file records, we should update the according user's table and hdisk table.
typedef struct
{
	char uuid[16];
	uint8_t  action;			//what do we want to update
	uint32_t user_id;      
	off_t video_size;
	off_t audio_size;
	off_t photo_size;
	off_t recycle_size;
	off_t total_update_size;
	//uint8_t  valid;             //used when updating user state
	uint32_t to_user_id;		//if we are moving or copying file, we should tell the destination user id.
	//char password[20];          //used when updating user password
}update_info_t;

//query approach define...
typedef enum
{
    //file table query
    QUERY_FILE_INFO,             //query single file informaion
    QUERY_FILE_BY_UUID,             //query single file informaion by uuid
    QUERY_FILE_BY_NAME,          //
    QUERY_FILE_DIR_LIST_COUNT,       //query record count in a certain file list
    QUERY_FILE_LIST_COUNT,       //query record count in a certain file list
    QUERY_FILE_LIST_COUNT_BY_PATH,       //query record count in a certain file list
    QUERY_FILE_LIST,             //query file list
    QUERY_DIR_LIST,             //query file list
    QUERY_FILE_LIST_BY_PATH,
    QUERY_HASH_CODE_FILE_LIST,
    QUERY_GROUP_LIST,			 //query group list
    QUERY_FILE_PATH,			 //query file's full path
    QUERY_BATCH_FILE_PATH,       //query a batch of files' path
    QUERY_NO_THM_PHOTO_LIST,     //query photo list without thumbnail
    QUERY_MAX_THUMB_NO,			 //query max thumbnail number used

	//user table query
	QUERY_USER_INFO_BY_ID,       //query user's information by user id.
	QUERY_USER_INFO_BY_SESSION,	 //query user's information by user session
	QUERY_USER_INFO_BY_IP,	 	//query user's information by user ip
	QUERY_USER_INFO_BY_NAME,	 //query user's information by user name
	QUERY_USER_LIST,             //query  all the users existed.

	//media tables query
	QUERY_VIDEO_INFO_BY_ID,     //query video information record in video table by id
	QUERY_AUDIO_INFO_BY_ID,	    //query audio information record in audio table by id
	QUERY_IMAGE_INFO_BY_ID,		//query image information record in image table by id.

	//device table query
	QUERY_DEVICE_BY_UUID,
	QUERY_DEVICE_BY_USER,       //query all the device records in device table owned by a appointed user.
	QUERY_DEVICE_BY_USER_MAC,   //query all the device records in device table owned by a apoointed user with mac addr.
	QUERY_DEVICE_BY_USER_MAC_NAME, //query the device record in device table owned by a appointed user with mac addr
										//and device name.

	QUERY_HDISK_INFO,          //query hdisk information

	QUERY_BACKUP_FILE_LIST,    //query backup file list

	QUERY_BACKUP_FILE,    //query backup file list

	QUERY_BACKUP_DEVICE_INFO,    //query backup file list

	QUERY_VERSION_INFO,        //query fw version

    QUERY_INVALID_PARAM
}QueryApproach;


typedef struct 
{
	CallBackFunc new_name_cb_func;
	void *new_name_cb_arg;

	CallBackFunc rename_cb_func;
	void *rename_cb_arg;

	CallBackFunc mv_cb_func;
	void *mv_cb_arg;

	CallBackFunc trash_cb_func;
	void *trash_cb_arg;

	CallBackFunc del_cb_func;
	void *del_cb_arg;

	CallBackFunc cp_cb_func;
	void *cp_cb_arg;

	CallBackFunc recover_cb_func;
	void *recover_cb_arg;
}DBCallBackObj;


typedef enum
{
    FILE_TABLE_DELETE_INFO    				= 0x01,
    FILE_TABLE_DELETE_TYPE_BY_PATH    		= 0x02,
    FILE_TABLE_DELETE_LIST    				= 0x03,
}file_delete_cmd_t;


typedef struct
{
	file_delete_cmd_t cmd;//0:delete normal file,1:delete type file by path;3:delete file list
	DBCallBackObj cb_obj;
    char *path;
	int file_type;
	char **file_list;
}delete_data_t;


//data collection of moving file records definition
typedef struct
{
	uint32_t target_id;  //id of file to move
	uint32_t new_parent; // destination id.
	DBCallBackObj cb_obj;
}move_data_t;

typedef error_t (*delete_callback)(sqlite3 *database, void *private_data, update_info_t *pupdate_info);
typedef error_t (*scan_disk_fn)(sqlite3 *database, void *path_info);
typedef error_t (*insert_item_fn)(sqlite3 *database, void *item_info);
typedef error_t (*generic_delete_item_fn)(sqlite3 *database,void *target);
typedef error_t (*special_delete_item_fn)(sqlite3 *database, void *target, delete_callback callback);
typedef error_t (*delete_item_fn)(sqlite3 *database, void *target);

typedef error_t (*lazy_delete_item_fn)(sqlite3 *database, void *target);
typedef error_t (*copy_item_fn) (sqlite3 *database, uint32_t id, uint32_t dest_id, update_info_t *pui);
typedef error_t (*move_item_fn) (sqlite3 *database, move_data_t *data);
typedef error_t (*query_item_fn)(sqlite3 *database, QueryApproach query_app, void *data);
typedef error_t (*rename_item)(sqlite3 *database, const char *src_path, const char *des_path);
typedef error_t (*passive_update_item)(sqlite3 *database, void *update_info);
typedef error_t (*active_update_item)(sqlite3 *database, void *update_info);
typedef error_t (*update_item)(sqlite3 *database, void *update_info);
typedef error_t (*reindex_fn)(sqlite3 *database);


typedef struct 
{
	 scan_disk_fn			 scan;
     insert_item_fn          insert;
	 generic_delete_item_fn  generic_delete;
	 special_delete_item_fn  special_delete;
	 delete_item_fn			 dm_delete;
	 lazy_delete_item_fn     lazy_delete;
	 copy_item_fn 	         copy;
	 move_item_fn	         move;
	 query_item_fn 	         query;
	 rename_item             rename;
	 passive_update_item     passive_update;
	 active_update_item      active_update;
	 update_item			 update;
	 reindex_fn              reindex_table; 
}table_ops;


typedef struct
{
    table_ops ops;
}db_table_t;

#endif

