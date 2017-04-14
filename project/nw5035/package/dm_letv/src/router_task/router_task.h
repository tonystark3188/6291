/*
 * =============================================================================
 *
 *       Filename:  router_task.h
 *
 *    Description:  json process operation
 *
 *        Version:  1.0
 *        Created:  2015/8/20 11:00
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _ROUTER_TASK_H_
#define _ROUTER_TASK_H_

#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "base.h"



#define DOWNLOADING 1
#define WAITING 0
#define PAUSE 3
#define REMOVE 10
#define DONE 2
#define DISK_UNWRITEABLE 4
#define INTERNET_ERROR 5
#define SERVER_ERROR 6
#define DOWNLOAD_URL_ERROR 7
#define RETRY_DOWNLOAD 8
#define START_TO_DOWNLOAD 9

#define ALBUM 2
#define VIDEO 0
#define IMAGE 3
#define FIRMWARE	4
#define PID_LEN 33
#define VID_LEN 33


#define LETV_DOWNLOAD_DIR_PATH "/tmp/mnt/USB-disk-1/hack"
#define LETV_VID_DIR_PATH "/tmp/mnt/USB-disk-1/hack/%s/"
#define LETV_PID_DIR_PATH "/tmp/mnt/USB-disk-1/hack/%s/"

#define LETV_VID_IMG_FILE_PATH "/tmp/mnt/USB-disk-1/hack/%s/%s.jpg"
#define LETV_VID_VIDEO_FILE_PATH "/tmp/mnt/USB-disk-1/hack/%s/%s.mp4"

#define LETV_TASKLIST_FILE_PATH_BACKUP "/tmp/mnt/USB-disk-1/hack/.tasklist.bak"

#define LETV_TASKLIST_FILE_PATH "/tmp/mnt/USB-disk-1/hack/.tasklist"
#define LETV_ALBUMLIST_FILE_PATH "/tmp/mnt/USB-disk-1/hack/.albumlist"


#ifdef __cplusplus
extern "C"{
#endif
typedef void (*DOWNLOAD_CALLBACK)(void *self);

#define URL_LEN 1024+1
#define URL_LEN2 2048+1
#define TAG_LEN 512
#define EXT_LEN 65

#define URL_ARGV_LEN_256 256


typedef struct task_dnode{//单个下载任务
	struct task_dnode *dn_next;
    char pid[PID_LEN];    /*剧集ID*/
    char vid[VID_LEN];    /*单个视频ID*/
    char ext[EXT_LEN]; 
    char tag[TAG_LEN]; 
    char img_url[URL_LEN];    /*视频封面图片的下载链接*/
    char vid_url[URL_LEN];    /*视频的下载链接*/
	char vid_re_url[URL_LEN];    /*视频的重定向下载链接*/
    char info[URL_LEN2];    /*2K 加密字段，从服务器得到后提供给APP自己解析*/
    long long total_size;    /*文件总大小*/
    long long downloaded_size;    /*已下载的大小*/
    int download_status;    /*下载状态，参考交互文档中定义*/
    char vid_path[256];    /*本地提供给APP的下载链接*/
    char img_path[256];    /*本地提供给APP的下载链接*/
	//DOWNLOAD_CALLBACK download_cb;//涓嬭浇鍥炶皟鍑芥暟
	char error_msg[128];
	int errorCode;
	float percent;
	time_t add_task_time;
	time_t update_task_time;
	int isAutoAdd;
	int isDeleted;
	long long  total_img_size;
	long long download_img_size;
	char unuse[240];// for later use	
	//time_t finish_task_time;
	//int status;    /**/
}TaskDnode;

struct task_dnode *task_dn;

typedef struct album_node{//剧集信息
	struct album_node *dn_next;
	char pid[PID_LEN];
	char info[URL_LEN2];
	char coverImgUrl[URL_LEN];
	char album_img_path[URL_LEN2];
	int  status;
	int  errorCode;	
	char error_msg[128];
	char ext[EXT_LEN];
	char tag[TAG_LEN];
	char tag2[TAG_LEN];
	int isEnd;
	unsigned long update_time;
	char unuse[252];// for later use	

}AlbumNode;

struct album_node *album_dn;
pthread_rwlock_t album_list_lock;
pthread_rwlock_t task_list_lock;


void display_hd_dnode(struct task_dnode *dn);
int add_task_to_list(struct task_dnode **dn,struct task_dnode *cur, int type);
int update_task_to_list(struct task_dnode *cur);
int update_task_status_to_list(
	char *pid,
	char *vid,
	int download_status);

int del_task_from_list(struct task_dnode **head,char*pid,char *vid);
int destory_task_list(struct task_dnode *dn);
int write_list_to_file(int type);//写出数据
int read_list_from_file(const char *path,struct task_dnode **dn);//读入数据
int IsInt(char * str);
#ifdef __cplusplus
}
#endif

#endif

