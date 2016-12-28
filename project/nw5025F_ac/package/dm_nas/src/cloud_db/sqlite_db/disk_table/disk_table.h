/*
 * =============================================================================
 *
 *       Filename: disk_table.h
 *
 *    Description: disk table related data structure definition.
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
#ifndef DISK_TABLE_H
#define DISK_TABLE_H


#ifdef __cplusplus
extern "C"{
#endif

#include "db_base.h"

#define  INVALID_DISK_ID    ((uint32_t)-1)
#define  DISK_TABLE_NAME  	"disk_table"

#define  MAX_UUID_LEN		32
#define  MAX_DISK_NAME_LEN	32

typedef enum
{
    DISK_TABLE_DELETE_BY_UUID    = 0x01,
}disk_delete_cmd_t;


typedef enum
{
    DISK_TABLE_UPDATE_CAPACITY    = 0x01,
}disk_update_cmd_t;

typedef enum
{
    DISK_TABLE_INSERT_INFO    = 0x01,
    DISK_TABLE_INSERT_LIST    = 0x02,
}disk_insert_cmd_t;


typedef enum
{
    DISK_TABLE_QUERY_INFO    = 0x01,
    DISK_TABLE_QUERY_LIST    = 0x02,
}disk_query_cmd_t;


typedef struct DiskInfoTable
{
	int disk_id;
	char uuid[MAX_UUID_LEN];
	char name[MAX_DISK_NAME_LEN];
	unsigned long long total_size; 		/* total size 1KB */
	unsigned long long free_size;  		/* free size 1KB */
	struct dl_list next;
}disk_info_t;

typedef struct DiskTableList
{
	/**
	 * 盘符数量
	 */
	int len;

	/**
	 * 用户信息列表;
	 */
	struct dl_list head; //list head for result

}disk_list_t;



typedef struct
{
	disk_info_t 		disk_info;
	disk_delete_cmd_t 	cmd;
}disk_delete_t;

typedef struct
{
	disk_info_t 		disk_info;
	disk_update_cmd_t 	cmd;
}disk_update_t;

typedef struct
{
	disk_list_t 		disk_list;
	disk_info_t 		disk_info;
	disk_query_cmd_t    cmd;
}disk_query_t;

typedef struct
{
	disk_list_t 		disk_list;
	disk_info_t 		disk_info;
	disk_query_cmd_t    cmd;
}disk_insert_t;


void register_disk_table_ops(void);



#ifdef __cplusplus
}
#endif



#endif
