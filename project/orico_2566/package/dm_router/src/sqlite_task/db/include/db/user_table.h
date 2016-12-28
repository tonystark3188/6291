/*
 * =============================================================================
 *
 *       Filename: user_table.h
 *
 *    Description: user table related data structure definition.
 *
 *        Version:  1.0
 *        Created:  2014/09/16 
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Tao Sheng (), vanbreaker90@gmail.com
 *   Organization:  
 *
 * =============================================================================
 */


#ifndef USER_TABLE_H
#define USER_TABLE_H


#ifdef __cplusplus
extern "C"{
#endif


#include "base.h"
#include "hidisk_errno.h"
#include "db/db_table.h"

//#define USER_NAME_SIZE 32
//#define PASSWORD_SIZE  16

#define MAX_USER_QUANTITY 128

#define UPDATE_USER_PASSWORD 0
#define UPDATE_USER_STATE    1

#define MAX_USER_NAME_LEN   64
#define MAX_USER_PASSWD_LEN 64 

#define INVALID_USER_ID 0

typedef struct UserInfoTable
{
	uint32_t id;
	char session[32];
	char *user_name;
	char *password;
	char ip[32];
	char device_name[MAX_USER_DEV_NAME_LEN];
	char device_name_escape[MAX_USER_DEV_NAME_LEN];
	char device_uuid[64];
	struct dl_list node;
}user_info_t;


typedef struct
{
	user_info_t user_info;
	uint8_t cmd;
	DBCallBackObj cb_obj;
}user_update_t;


//data collection of user delete data.
typedef struct
{
	char session[32];
	char ip[32];
	uint32_t target_id;  //user id
	uint32_t user_dir_id;
	DBCallBackObj cb_obj;
}user_del_data_t;


error_t user_table_init(void);
void register_user_table_ops(void);

#ifdef __cplusplus
}
#endif



#endif
