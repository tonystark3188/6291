/*
 * =============================================================================
 *
 *       Filename: authority_table.h
 *
 *    Description: authority table related data structure definition.
 *
 *        Version:  1.0
 *        Created:  2016/10/19 
 *       Revision:  none
 *       Compiler:  gcc
 *
 *          Author:  Oliver <henry.jin@dmsys.com>
 *   Organization:  longsys
 *
 * =============================================================================
 */
#ifndef AUTHORITY_TABLE_H
#define AUTHORITY_TABLE_H


#ifdef __cplusplus
extern "C"{
#endif

#include "db_base.h"

#define  INVALID_AUTHORITY_ID    ((uint32_t)-1)
#define  AUTHORITY_TABLE_NAME  "authority_table"

typedef enum
{
    AUTHORITY_TABLE_UPDATE_AUTH    = 0x01,
}authority_update_cmd_t;

typedef enum
{
    AUTHORITY_TABLE_QUERY_INFO    = 0x01,
    AUTHORITY_TABLE_QUERY_LIST    = 0x02,
}authority_query_cmd_t;

typedef enum
{
    AUTHORITY_TABLE_INSERT_INFO    = 0x01,
    AUTHORITY_TABLE_INSERT_LIST    = 0x02,
}authority_insert_cmd_t;

typedef enum
{
    AUTHORITY_TABLE_DELETE_INFO    = 0x01,
    AUTHORITY_TABLE_DELETE_LIST    = 0x02,
}authority_delete_cmd_t;


typedef struct AuthorityTableInfo
{
	/**
	 * 权限id
	 */
	int authority_id;
	/**
	 * 用户id
	 */
	int user_id;
	/**
	 * bucket id
	 */
	int bucket_id;
	/**
	 *  0:不可读不可写
	 *  1：可读不可写
	 *  2：可写不可读（可写不可读）
	 *  3：可写可读
	 */
	int authority;
	struct dl_list next;
} authority_info_t;

typedef struct
{
	/**
	 * 数量
	 */
	int len;

	/**
	 * authority 信息
	 */
	struct dl_list head; //list head for result

} authority_list_t;

typedef struct
{
	authority_info_t 		authority_info;
	authority_update_cmd_t 	cmd;
}authority_update_t;

typedef struct
{
	authority_list_t 		authority_list;
	authority_info_t 		authority_info;
	authority_query_cmd_t   cmd;
}authority_query_t;

typedef struct
{
	authority_list_t 		authority_list;
	authority_info_t 		authority_info;
	authority_insert_cmd_t   cmd;
}authority_insert_t;

typedef struct
{
	authority_list_t 		authority_list;
	authority_info_t 		authority_info;
	authority_delete_cmd_t   cmd;
}authority_delete_t;


void register_authority_table_ops(void);



#ifdef __cplusplus
}
#endif



#endif
