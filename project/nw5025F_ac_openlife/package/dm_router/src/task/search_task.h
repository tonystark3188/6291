/*
 * =============================================================================
 *
 *       Filename:  scan_task.h
 *
 *    Description:  handle scanning disk.
 *
 *        Version:  1.0
 *        Created:  2015/9/24 15:08:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver ()
 *   Organization:  
 *
 * =============================================================================
 */
#ifndef _SEARCH_TASK_H_
#define _SEARCH_TASK_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <pthread.h>
#include "base.h"

#define SEARCH_TASK_TCP

#define SEND_LIST_INFO_MIN		10
#define SEND_LIST_INFO_MAX 		20
#define SEND_ERROR_TIMES_MAX 	100	
#define SEND_LIST_INFO_TIME		3 	//s

typedef struct file_search_info{
	struct conn *c;
	int sock;
    int cmd;
	char *search_path;
    char *search_string;
    char session[32];
    unsigned int *flags;
    unsigned seq;
	unsigned search_seq;
	char ip[32];
	int port;
	char disk_uuid[64];
	int client_fd;
	struct sockaddr_in clientAddr;
	long cur_time;
	long record_time;
	int status;
	int statusCode;
	int recvStatusCode;
	unsigned nfiles;
	unsigned cur_nfiles;//当前已搜索的文件个数
	unsigned list_nfiles;//当前文件链表的文件个数
	unsigned send_nfiles;//发送文件链表的文件个数
	int currentOnly;
}file_search_info_t;


//data collection of file list querying
typedef struct{
	struct dl_list head; //list head for result
	uint32_t listCount;    //list count
	pthread_mutex_t mutex;
}search_manage_list_t;

typedef struct{
	struct dl_list head; //list head for result
	unsigned seq;
	int client_fd;
	struct sockaddr_in clientAddr;
	char ip[32];
	int port;
	int searchStatus;
	int statusCode;
	int recvStatusCode;
	unsigned curNfiles;//当前已搜索的文件个数
	unsigned listNfiles;//当前文件链表的文件个数
	struct dl_list next;
}search_manage_info_t;


#define MAX_MIME_TYPE_SIZE     32

typedef struct{
	off_t file_size;//文件大小
	unsigned create_time;//创建时间
	unsigned modify_time;//修改时间
	unsigned access_time;
	char isFolder;//是否是目录文件
	char *name;	//文件名
	char *dir;
	char *path;   //name
	char *path_escape;
	char mime_type[MAX_MIME_TYPE_SIZE];
	int  file_type;		   // see file_type_t
	uint8_t  file_state;
	char file_uuid[64];
	uint32_t file_count;
	struct dl_list next;
}search_file_info_t;

void search_inotify_func(void *self);

void search_manage_task_func();


#ifdef __cplusplus
}
#endif


#endif
