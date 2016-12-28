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
#include "base.h"

#ifdef __cplusplus
extern "C"{
#endif
int router_para_listen_port;
int router_request_type;

#define POWER_TYPE 0
#define DISK_TYPE  1
#define SSID_TYPE  2
#define DB_TYPE    3
#define PWD_TYPE   4

typedef struct dev_dnode
{
	struct dl_list next;
	char ip[32];
	uint16_t port;
	uint8_t request_type;// for example,111:the dev register lower,disk and other hardware info
	uint16_t data_base_sign;//data base seq changed
	uint16_t power_seq;
	uint16_t disk_seq;
	uint16_t ssid_seq;
	uint16_t db_seq;
	uint16_t pwd_seq;
}dev_dnode_t;


typedef struct dev_list
{
	struct dl_list head; //list head for result
	pthread_mutex_t mutex;
}dev_list_t;

int dm_get_status_changed();

int _dev_list_init();

int _destory_dev_list();

void _display_dev_dnode();

dev_list_t *get_dev_list();

void _free_dev_list(struct dev_dnode **dnp);

int _del_dev_from_list_by_ip(char *ip);

int _add_dev_to_list(_In_ char *ip,_In_ uint16_t port,_In_ uint8_t request_type);

int _check_ip_to_list(char *ip);
/*************************************************
Function: _update_seq_to_dev
Description:根据ip 更新序列号 
Calls: // 被本函数调用的函数清单
Called By: // 调用本函数的函数清单
Input: ip,seq,type 0:power seq,1:disk seq,2:ssid seq,3:data base seq
Output: NULL
Return: 0 success,-1,failed
Others: // 其它说明
*************************************************/
int _update_seq_to_dev(_In_ char *ip,_In_ unsigned seq,_In_ int type);


#ifdef __cplusplus
}
#endif

#endif

