/*
 * =============================================================================
 *
 *       Filename: bucket_table.h
 *
 *    Description: bucket table related data structure definition.
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
#ifndef BUCKET_TABLE_H
#define BUCKET_TABLE_H


#ifdef __cplusplus
extern "C"{
#endif

#include "db_base.h"

#define  INVALID_BUCKET_ID    ((uint32_t)-1)
#define  BUCKET_TABLE_NAME  "bucket_table"


typedef enum
{
    BUCKET_TABLE_UPDATE_BUCKET_NAME    = 0x01,
    BUCKET_TABLE_UPDATE_CREATE_USER_ID   = 0x02,
}bucket_update_cmd_t;

typedef enum
{
    BUCKET_TABLE_INSERT_INFO    = 0x01,
    BUCKET_TABLE_INSERT_LIST    = 0x02,
}bucket_insert_cmd_t;


typedef enum
{
    BUCKET_TABLE_QUERY_INFO    = 0x01,
    BUCKET_TABLE_QUERY_LIST    = 0x02,
}bucket_query_cmd_t;

typedef enum
{
    BUCKET_TABLE_DELETE_INFO    = 0x01,
    BUCKET_TABLE_DELETE_LIST    = 0x02,
}bucket_delete_cmd_t;


typedef struct BucketInfoTable
{
	/**
	 * bucket id
	 */
	int bucket_id;
	/**
	 * bucket 表名
	 */
	char bucket_table_name[MAX_BUCKET_NAME_LEN];
	/**
	 * 创建用户id
	 */
    int create_user_id;
	struct dl_list next;
} bucket_info_t;

typedef struct
{
	/**
	 * 数量
	 */
	int len;
	/**
	 * bucket信息列表;
	 */
	struct dl_list head; //list head for result
} bucket_list_t;


typedef struct
{
	bucket_info_t 			bucket_info;
	bucket_delete_cmd_t 	cmd;
}bucket_delete_t;

typedef struct
{
	bucket_list_t 			bucket_list;
	bucket_info_t 			bucket_info;
	bucket_query_cmd_t    	cmd;
}bucket_query_t;

typedef struct
{
	bucket_info_t 			bucket_info;
	bucket_update_cmd_t 	cmd;
}bucket_update_t;

typedef struct
{
	bucket_list_t 			bucket_list;
	bucket_info_t 			bucket_info;
	bucket_insert_cmd_t 	cmd;
}bucket_insert_t;


void register_bucket_table_ops(void);



#ifdef __cplusplus
}
#endif



#endif
