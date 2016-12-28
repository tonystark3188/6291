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
typedef struct hd_dnode {
	struct hd_dnode *dn_next;
	char session[32];
	char ip[32];
	uint16_t port;
	uint8_t request_type;// for example,111:the dev register lower,disk and other hardware info
	uint16_t power_seq;
	uint16_t disk_seq;
	uint16_t ssid_seq;
}hd_dnode_t;

/*
*	add dev to static list
*/
int add_dev_to_list(struct hd_dnode **dn,char *session_id,uint16_t port,uint8_t request_type);
int del_dev_from_list(struct hd_dnode **head,char *session_id);
int del_dev_from_list_for_ip(struct hd_dnode **head,char *ip);

int destory_router_list(struct hd_dnode *dn);
int dm_get_status_changed();
void display_hd_dnode(struct hd_dnode *dn);
int get_info_from_hd_list(_In_ const char *session,_Out_ char *ip,_Out_ int *port);




#ifdef __cplusplus
}
#endif

#endif

