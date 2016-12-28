/*
 * =============================================================================
 *
 *       Filename: token_manage.h
 *
 *    Description: token management
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

#ifndef _TOKEN_MANAGE_H_
#define _TOKEN_MANAGE_H_
#include <stdbool.h>
#include "base.h"
#include "config.h"

#ifdef __cplusplus
extern "C"{
#endif

#define TOKEN_MANAGE

#ifdef TOKEN_MANAGE

typedef struct token_dnode {
	struct dl_list next;
	char	bucket_name[32];
	int 	user_id;
	int 	authority;
	int 	bucket_id;
	bool 	isLogin;
	int32_t watch_time;
	bool	isPublicUser;
	bool	isPublicPath;
}token_dnode_t;

typedef struct token_list{
	struct dl_list head; //list head for result
	pthread_mutex_t mutex;
}token_list_t;

int token_list_init(token_list_t *p_session_list);

int token_list_destroy(token_list_t *p_session_list);

int reset_token_list(bool isLogin,token_list_t *p_session_list);

int update_token_time(uint32_t token,token_list_t *p_session_list);

bool is_token_login(_In_ uint32_t token,token_list_t *p_session_list);

uint32_t add_token_to_list(int user_id,char *bucket_name,int authority,bool isPublicUser,token_list_t *p_session_list); 

int del_token_from_list(uint32_t token,token_list_t *p_session_list);

struct token_dnode **get_token_list(unsigned *nmount_p,token_list_t *p_session_list);

struct token_dnode *get_token_info(uint32_t token,token_list_t *p_session_list);

void free_token_list(token_dnode_t **dnp);
#endif

#ifdef __cplusplus
}
#endif

#endif

