/*
 * =============================================================================
 *
 *       Filename: user_table.h
 *
 *    Description: user table related data structure definition.
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
#ifndef USER_TABLE_H
#define USER_TABLE_H


#ifdef __cplusplus
extern "C"{
#endif

#include "db_base.h"

#define  INVALID_USER_ID    ((uint32_t)-1)
#define  USER_TABLE_NAME  "user_table"

#define	MAX_USER_NAME_LEN 	32
#define MAX_PASSWORD_LEN	32

typedef enum
{
    USER_TABLE_UPDATE_PASSWORD    = 0x01,
    USER_TABLE_UPDATE_NICK_NAME   = 0x02,
}user_update_cmd_t;

typedef enum
{
    USER_TABLE_QUERY_INFO    = 0x01,
    USER_TABLE_QUERY_LIST    = 0x02,
    USER_TABLE_QUERY_INFO_BY_ID    = 0x03,
}user_query_cmd_t;

typedef enum
{
    USER_TABLE_INSERT_INFO    = 0x01,
    USER_TABLE_INSERT_LIST    = 0x02,
}user_insert_cmd_t;

typedef enum
{
    USER_TABLE_DELETE_INFO    = 0x01,
    USER_TABLE_DELETE_LIST    = 0x02,
}user_delete_cmd_t;




typedef struct UserInfoTable
{
	/**
	 * 用户唯一标示符
	 */
	int		user_id;
	/**
	 * 用户名称
	 */
	char	user_name[MAX_USER_NAME_LEN];
	char	password[MAX_PASSWORD_LEN];
	/**
	 * 用户昵称
	 */
	char	nick_name[MAX_USER_NAME_LEN];

	struct dl_list next;
}user_info_t;

typedef struct UserTableList
{
	/**
	 * 用户数量
	 */
	int len;

	/**
	 * 用户信息列表;
	 */
	struct dl_list head; //list head for result

}user_list_t;


typedef struct
{
	user_info_t 		user_info;
	user_update_cmd_t 	cmd;
}user_update_t;

typedef struct
{
	user_list_t 		user_list;
	user_info_t 		user_info;
	user_query_cmd_t    cmd;
}user_query_t;

typedef struct
{
	user_list_t 		user_list;
	user_info_t 		user_info;
	user_insert_cmd_t   cmd;
}user_insert_t;

typedef struct
{
	user_list_t 		user_list;
	user_info_t 		user_info;
	user_delete_cmd_t   cmd;
}user_delete_t;



void register_user_table_ops(void);


#ifdef __cplusplus
}
#endif



#endif
