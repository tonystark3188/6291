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
#define DM_FW_VERSION "airdisk_1.0.0"
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
#define AIRDISK_ROOT_PATH					"/tmp"
#define NAS_DB_DIR_NAME                     "db"

typedef struct
{
    char fw_ver[16];
    char db_ver[8];
    uint16_t nas_server_port;
    char nas_root_path[16];
    char nas_data_path[32];
    char nas_db_path[32];
    uint8_t reserved;
    
    SystemHWConfigInfo hw_conf_info;
}SystemConfigInfo;
const char * get_sys_db_version(void);


const char * get_sys_nas_data_path(void);
const char * get_sys_nas_root_path(void);


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

