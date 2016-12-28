/*
 * =============================================================================
 *
 *       Filename:  router_inotify.h
 *
 *    Description:  monitor router parameter changed
 *
 *        Version:  1.0
 *        Created:  2015/8/24 16:00
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _ROUTER_INOTIFY_H_
#define _ROUTER_INOTIFY_H_


#include "base.h"

#ifdef __cplusplus
extern "C"{
#endif

#define INOTIFY_DIR_PATH "/tmp/notify"
#define INOTIFY_POWER "power"
#define INOTIFY_DISK  "disk"
#define INOTIFY_SSID  "ssid"

#define power_changed			0x01
#define disk_changed			0x02
#define ssid_changed 			0x04

#define power_disk_changed		0x03
#define power_ssid_changed 		0x05
#define disk_ssid_changed 		0x06
#define power_disk_ssid_changed 0x07
typedef struct hardware_info{
	struct hd_dnode *dev_dnode;
	uint16_t power_seq;
	uint16_t disk_seq;
	uint16_t ssid_seq;
}hardware_info_t;
    
typedef struct copy_info{
    int sock;
    int cmd;
	char *file_uuid;
    char *src_path;
    char *des_path;
    char session[32];
    unsigned int *flags;
    unsigned seq;
    int status;//0:正在计算中，1:正在传输中，2:操作完成，3:操作异常中断
    char *cur_name;
    unsigned long total_size;
    unsigned long cur_progress;
    unsigned long total_progress;
    unsigned cur_nfiles;//当前已复制的文件个数
    unsigned nfiles;
    char ip[32];
    int port;
	char disk_uuid[32];
}copy_info_t;

int notify_router_para(const char *inotify_path,int pnTimeOut);
void notify_disk_scan_status(int status);
int notify_disk_scanning();
int copy_handle_inotify(copy_info_t *pInfo);



#ifdef __cplusplus
}
#endif

#endif

