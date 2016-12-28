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
	struct dl_list next;
	struct usr_dnode *dn_next;
	char ip[32];
	char username[128];
	char password[128];
	int count;
	int32_t start_time;
	int32_t cur_time;
}usr_dnode_t;

typedef struct usr_list{
	struct dl_list head; //list head for result
	pthread_mutex_t mutex;
}usr_list_t;

/*int update_usr_count(struct usr_dnode **dn,char *ip);

int update_usr_for_ip(struct usr_dnode **dn,char *ip);

int del_usr_from_list_for_ip(struct usr_dnode **head,char *ip);

int destory_usr_list(struct usr_dnode *dn);*/

int usr_list_init();

int usr_list_destroy();

int update_usr_time(char *ip);

int add_usr_to_list(char *ip); 

int del_usr_from_list(char *ip);

struct usr_dnode **_get_usr_list(unsigned *nmount_p);

void _free_usr_list(struct usr_dnode **dnp);



#ifdef __cplusplus
}
#endif

#endif

