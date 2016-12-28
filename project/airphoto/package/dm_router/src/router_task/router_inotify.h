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

#define power_changed 1
#define disk_changed 2
#define ssid_changed 4

#define power_disk_changed 3
#define power_ssid_changed 5
#define power_disk_ssid_changed 7
#define disk_ssid_changed 6
typedef struct hardware_info{
	struct hd_dnode *dev_dnode;
	uint16_t power_seq;
	uint16_t disk_seq;
	uint16_t ssid_seq;
}hardware_info_t;

int notify_router_para(const char *inotify_path,int pnTimeOut);
void notify_disk_scan_status(int status);
int notify_disk_scanning();


#ifdef __cplusplus
}
#endif

#endif

