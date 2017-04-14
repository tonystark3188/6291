/*
 * =============================================================================
 *
 *       Filename:  session_manage.h
 *
 *    Description: session management
 *
 *        Version:  1.0
 *        Created:  2016/8/10 17:10
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _SESSION_MANAGE_H_
#define _SESSION_MANAGE_H_
#include <stdbool.h>
#include "base.h"
#include "config.h"

#ifdef __cplusplus
extern "C"{
#endif

#define SESSION_MANAGE

#ifdef SESSION_MANAGE

typedef struct session_dnode {
	struct dl_list next;
	char 	session[64];
	bool 	isLogin;
	int32_t watch_time;
}session_dnode_t;

typedef struct session_list{
	struct dl_list head; //list head for result
	pthread_mutex_t mutex;
}session_list_t;

int session_list_init(session_list_t *p_session_list);

int session_list_destroy(session_list_t *p_session_list);

int reset_session_list(bool isLogin,session_list_t *p_session_list);

int update_session_time(char *session,session_list_t *p_session_list);

bool is_session_login(_In_ char *session,session_list_t *p_session_list);

int add_session_to_list(char *session,session_list_t *p_session_list); 

int del_session_from_list(char *session,session_list_t *p_session_list);

struct session_dnode **get_session_list(unsigned *nmount_p,session_list_t *p_session_list);

void free_session_list(struct session_dnode **dnp);
#endif

#ifdef __cplusplus
}
#endif

#endif

