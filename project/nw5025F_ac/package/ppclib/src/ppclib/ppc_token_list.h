#ifndef _PPC_TOKEN_LIST_H_
#define _PPC_TOKEN_LIST_H_

#ifdef __cplusplus
extern "C"{
#endif
#include "list.h"
#include "base.h"

typedef struct _dl_ppc_token_list
{
	struct dl_list head;
	int has_init;
	pthread_mutex_t mutex;
}dl_ppc_token_list;


typedef struct _token_info
{
	_int64_t token;
	char *work_dir;
	struct dl_list next;
}token_info;

int init_ppc_token_list();
int add_info_for_ppc_token_list(token_info *p_token_info);
int del_info_for_ppc_token_list(_int64_t token);
int get_info_from_ppc_token_list(token_info **p_token_info, _int64_t token);
int update_work_dir_for_ppc_token_list(char *work_dir, _int64_t token);
char *get_work_dir_from_ppc_token_list(_int64_t token);
int free_ppc_token_list();

#ifdef __cplusplus
}
#endif

#endif
