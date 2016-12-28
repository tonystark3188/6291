/*
 * =============================================================================
 *
 *       Filename:  usr_manage.h
 *
 *    Description: user infomation process for dm init module
 *
 *        Version:  1.0
 *        Created:  2015/8/21 10:13
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _USR_MANAGE_H_
#define _USR_MANAGE_H_

#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "base.h"


#ifdef __cplusplus
extern "C"{
#endif
typedef struct usr_dnode {
	struct usr_dnode *dn_next;
	char ip[32];
	int count;
	uint32_t start_time;
	uint32_t cur_time;
}usr_dnode_t;

typedef struct usr_info{
	struct usr_dnode *usr_dnode;
	uint8_t count;
}usr_info_t;
int update_usr_count(struct usr_dnode **dn,char *ip);


int update_usr_for_ip(struct usr_dnode **dn,char *ip,uint32_t cur_time);

/*
*	add client usr info to static list 
*     return < 0:error,0:sucess
*/
int add_usr_to_list(struct usr_dnode **dn,char *ip,uint32_t cur_time);
/*
*	delete client usr info from static list accorrding to client logout cmd 
*     return < 0:error,0:sucess
*/
int del_usr_from_list_for_ip(struct usr_dnode **head,char *ip);


int destory_usr_list(struct usr_dnode *dn);

void display_usr_dnode(struct usr_dnode *dn);


#ifdef __cplusplus
}
#endif

#endif


