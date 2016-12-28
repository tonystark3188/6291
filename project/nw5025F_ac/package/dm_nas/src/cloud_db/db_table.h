#ifndef DB_TABLE_H
#define DB_TABLE_H

#include "db_base.h"
#include "v_file_table.h"
#include "user_table.h"
#include "authority_table.h"
#include "disk_table.h"
#include "bucket_table.h"
#include "xxpinyin.h"

#define  SEPERATOR '/'
#define  ROOT_LEVEL   0//default 3

#define  ROOT_DIR_ID  0




typedef error_t (*insert_item_fn)(sqlite3 *database, void *data);
typedef error_t (*delete_item_fn)(sqlite3 *database,void *data);
typedef error_t (*update_item_fn)(sqlite3 *database, void *data);
typedef error_t (*query_item_fn)(sqlite3 *database,void *data);


typedef struct
{
	v_file_delete_t v_file_delete;
	bucket_delete_t bucket_delete;
	user_delete_t	user_delete;
	authority_delete_t	authority_delete;
	disk_delete_t		disk_delete;
}delete_data_t;

typedef struct 
{
	disk_update_t		disk_update;
	bucket_update_t		bucket_update;
	authority_update_t	authority_update;
	user_update_t		user_update;
	v_file_update_t 	v_file_update;
}update_data_t;

typedef struct
{
	bucket_query_t		bucket_query;
	v_file_query_t		v_file_query;
	user_query_t		user_query;
	authority_query_t 	authority_query;
	disk_query_t 		disk_query;
}query_data_t;

typedef struct
{
	bucket_insert_t		bucket_insert;
	v_file_insert_t		v_file_insert;
	user_insert_t		user_insert;
	authority_insert_t 	authority_insert;
	disk_insert_t 		disk_insert;
}insert_data_t;


typedef struct DB_Data
{
	insert_data_t       insert_data;     //using for insert
	delete_data_t  		delete_data;     //using for delete file record and backup file record
	update_data_t       update_data;
	query_data_t		query_data;
}DBData;

typedef struct DB_OprObj
{
	DBData 			data;
	CallBackFunc 	cb_obj;
	error_t 		ret;
}DB_OprObj;


typedef struct 
{
     insert_item_fn          insert;
	 delete_item_fn			 delete;
	 query_item_fn 	         query;
	 update_item_fn			 update;
}table_ops;


typedef struct
{
    table_ops ops;
}db_table_t;

extern db_table_t g_user_table;
extern db_table_t g_authority_table;
extern db_table_t g_bucket_table;
extern db_table_t g_v_file_table;
extern db_table_t g_disk_table;









#endif

