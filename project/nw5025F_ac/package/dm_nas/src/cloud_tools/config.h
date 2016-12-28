/*
 * =============================================================================
 *
 *       Filename:  config.h
 *
 *    Description:  AirNas config module
 *
 *        Version:  1.0
 *        Created:  2014/9/5 13:51:37
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ye Wenhao (), wenhaoye@126.com
 *   Organization:  
 *
 * =============================================================================
 */
#ifndef _AIRNAS_CONFIG_H_
#define _AIRNAS_CONFIG_H_

#include "base.h"


#ifdef __cplusplus
extern "C"{
#endif

typedef struct 
{
    uint32_t nandflash_size;    // in Mbytes
    uint16_t in_disk_size;      // in GBytes
    uint16_t memory_size;       // in Mbytes
    uint8_t  out_disk_count;
}SystemHWConfigInfo;

typedef enum
{
    INITIALING          = 0,  // internal
    NORMAL              = 20100,
    HD_ERROR            = 20101,
    DB_RECONSTRUING     = 20102,
    HD_FORMATING        = 20103,
    DB_BACKUPING        = 20104,
    DB_RECOVERING       = 20105,
    DB_UPDATEING        = 20106,
    DB_INITING          = 20107,
    DB_ERROR            = 20108,
    HD_NOT_EXIST        = 20109,
    UPGRADING           = 20110,
    
    SYS_EXITING         // internal
}SystemStatus;
#define AIRDISK_ROOT_PATH					"/root"
#define NAS_DB_DIR_NAME                     "db"

typedef struct
{
	char router_ver[16];
    char fw_ver[16];
    char db_ver[16];
	char db_name[16];
	char uuid_name[16];
	char product_model[16];
    uint16_t airdisk_file_port;
	uint16_t airdisk_router_port;
	uint16_t airdisk_init_port;
	uint16_t session_watch_time;
    char nas_root_path[16];
    char nas_data_path[32];
    char nas_db_path[32];
    uint8_t reserved;
	uint16_t fun_list_flag;
	uint16_t database_sign;
    
    SystemHWConfigInfo hw_conf_info;
}SystemConfigInfo;
const char * get_sys_db_version(void);
const char * get_sys_dm_router_version(void);
const char * get_sys_fw_version(void);
const char * get_sys_product_model(void);
const char * get_sys_nas_data_path(void);
const char * get_sys_nas_root_path(void);
const char * get_sys_disk_uuid_name(void);
const char * get_sys_db_name(void);
int get_sys_file_port(void);
int get_sys_router_port(void);

int get_sys_init_port(void);
int get_func_list_flag(void);

int get_database_sign(void);

int set_database_sign(uint16_t sign);

int get_session_watch_time(void);







typedef struct digit_version
{
    int major;
	int minor;
	int revision;
}digit_version_t;

void version_to_digit(const char *version, digit_version_t *dv);


#ifdef __cplusplus
}
#endif


#endif

