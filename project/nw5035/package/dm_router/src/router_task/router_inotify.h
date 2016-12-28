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
#include "search_task.h"
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
    
typedef struct copy_info{
	struct conn *c;
    int sock;
    int cmd;
	int error;
	char *file_uuid;
    char *src_path;
    char *des_path;
    char session[32];
    unsigned int *flags;
    unsigned seq;
    int status;//0:正在计算中，1:正在传输中，2:操作完成，3:操作异常中断
    char *cur_name;
    off_t total_size;
    off_t cur_progress;
    off_t total_progress;
    unsigned cur_nfiles;//当前已复制的文件个数
    unsigned nfiles;
    char ip[32];
    int port;
	char disk_uuid[64];
	int client_fd;
	struct sockaddr_in clientAddr;
	long cur_time;
	long record_time;
	file_uuid_list_t *flist;
	int file_type;
	unsigned long long		content_len;
	int loc_fd;
	unsigned	*watch_dog_time;
	bool attr;
	bool album;
	char **file_list;
	char **file_dnode;
}copy_info_t,del_info_t,download_info_t,hide_info_t;

typedef struct release_info{
    int sock;
    int cmd;
    char session[32];
	int release_flag;
    unsigned int *flags;
}release_info_t;

typedef struct backup_info{
	char file_uuid[64];
	char device_uuid[64];
	char disk_uuid[64];
	char *path;
	int bIsRegularFile;// 0:dir,1:file,2:backup file
}backup_info_t,ifile_info_t;



int notify_router_para(const char *inotify_path,int pnTimeOut);
void notify_disk_scan_status(int status);

int copy_handle_inotify(copy_info_t *pInfo);
int search_handle_udp_inotify(file_search_info_t *pInfo, file_list_t *plist);

char *notify_disk_scanning(int release_flag);
int parser_scan_notify_json(char *recv_buf, int *p_release_flag, int *p_event, char *p_action_node);


#ifdef __cplusplus
}
#endif

#endif

