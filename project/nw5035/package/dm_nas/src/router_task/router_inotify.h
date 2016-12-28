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
#include "router_defs.h"
#include "defs.h"


#ifdef __cplusplus
extern "C"{
#endif

#define INOTIFY_DIR_PATH 	"/tmp/notify"
#define INOTIFY_POWER 		"power"
#define INOTIFY_DISK  		"disk"
#define INOTIFY_SSID  		"ssid"
#define INOTIFY_DATA_BASE  	"data_base"
#define INOTIFY_PWD  		"pwd"

#define power_changed			0x01
#define disk_changed			0x02
#define ssid_changed 			0x04
#define data_base_changed		0x08
#define pwd_changed				0x10


#define power_disk_changed		0x03
#define power_ssid_changed 		0x05
#define disk_ssid_changed 		0x06
#define power_disk_ssid_changed 0x07
typedef struct hardware_info{
	struct dev_dnode *dev_dnode;
	uint16_t power_seq;
	uint16_t disk_seq;
	uint16_t ssid_seq;
	uint16_t db_seq;
	uint16_t pwd_seq;
}hardware_info_t;


int notify_router_para(const char *inotify_path,int pnTimeOut);


#ifdef __cplusplus
}
#endif

#endif

