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
#define DM_FW_VERSION "1.0.0"
typedef struct 
{
    uint32_t nandflash_size;    // in Mbytes
    uint16_t in_disk_size;      // in GBytes
    uint16_t memory_size;       // in Mbytes
    uint8_t  out_disk_count;
}SystemHWConfigInfo;

typedef struct
{
    // log info
    char log_name[64];  //full path
    MyLogLevel log_level;
    uint16_t log_buf_size;   // in KBytes

    // memory pool info
    uint16_t  mpool_unit_size;  // in KBytes
    uint16_t  mpool_unit_nums;

	// db memory pool info
	uint16_t db_mpool_unit_nums;   

    // message object pool info
    uint16_t mo_mpool_unit_nums;

    uint8_t  cgi_forwd_cli_threads;

}SystemSWConfigInfo;

typedef enum
{
    UNKNOW_PLATFORM_TYPE = 0,
    AIRNAS_PLATFORM,
    AIRDISK_PLATFORM
}PlatformType;

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
#define NAS_DATA_DIR_NAME                   "data"
#define NAS_SYSTEM_DIR_NAME                 "system"
#define NAS_DB_DIR_NAME                     "db"
#define NAS_SHARE_DIR_NAME                  "share"
#define NAS_LOG_DIR_NAME                    "log"
#define NAS_HTTP_LOG_DIR_NAME               "http_log"
#define NAS_THUMBNAIL_DIR_NAME              "thumb"
#define NAS_THUMBNAIL_SMALL_DIR_NAME        "small"
#define NAS_THUMBNAIL_MIDDLE_DIR_NAME       "middle"
#define NAS_BACKUP_FILE_DIR_NAME            "backup"

typedef struct
{
    char platform[16];  // For Now, we only support AirNas, AirDisk
    char fw_ver[16];
    PlatformType type;
    int fw_code;
    uint16_t api_ver_code;
    uint16_t force_update_flag; 
    char db_ver[8];
    uint16_t nas_server_port;
    uint16_t publish_server_port;
    uint16_t nas_dev_disc_port;
    char nas_root_path[16];
    char nas_data_path[32];
    char nas_system_path[32];
    char nas_db_path[32];
    char nas_share_path[32];
    char nas_log_path[32];
    char nas_backup_path[32];
    char eth_wan_inf[8];
    char wifi_ap_inf[8];
    uint8_t idle_threshold;
    uint8_t reserved;
    
    SystemHWConfigInfo hw_conf_info;
    SystemSWConfigInfo sw_conf_info;
}SystemConfigInfo;

uint16_t get_nas_dev_disc_port(void);
uint16_t get_nas_serv_port(void);
uint16_t get_nas_pub_serv_port(void);
const char * get_nas_fw_ver(void);
const char * get_sys_db_version(void);
void get_nas_fw_ver_ext(char *ver_str, size_t size);
int get_nas_fw_code(void);
uint16_t get_nas_api_ver_code(void);
uint16_t get_nas_force_update_flag(void);
uint8_t get_nas_idle_threshold(void);


PlatformType get_sys_platform_type(void);
const char * get_sys_nas_data_path(void);
const char * get_sys_nas_system_path(void);
const char * get_sys_nas_db_path(void);
const char * get_sys_nas_root_path(void);
const char * get_sys_nas_share_path(void);
const char * get_sys_nas_thumbnail_small_path(void);
const char * get_sys_nas_thumbnail_middle_path(void);
const char * get_sys_nas_backup_path(void);
const char * get_sys_nas_log_path(void);
const char * get_sys_nas_http_log_path(void);
const char * get_sys_nas_log_name(void);
const char *get_eth_wan_inf(void);
const char *get_wifi_ap_inf(void);



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

