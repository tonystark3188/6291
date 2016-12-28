#include <pthread.h>

#include "ppc_token_list.h"
#include "base.h"
#include "hidisk_types.h"

dl_ppc_token_list ppc_token_list;

static void free_token_info(token_info **p_token_info)
{
	if(*p_token_info != NULL){
		safe_free((*p_token_info)->work_dir);
		safe_free(*p_token_info);		
		*p_token_info = NULL;
	}
}

int init_ppc_token_list()
{	
	if(0 == ppc_token_list.has_init){
		memset(&ppc_token_list, 0, sizeof(dl_ppc_token_list));
		pthread_mutex_init(&ppc_token_list.mutex, NULL);
		dl_list_init(&ppc_token_list.head);
		ppc_token_list.has_init = 1;
	}
	//else{
	//	ppc_token_list.has_init++;
	//}
	return 0;
}

int add_info_for_ppc_token_list(token_info *p_token_info)
{
	int ret = -1;
	if(p_token_info == NULL){
		DMCLOG_E("p_token_info is null!!");
		return -1;
	}
	
	if(ppc_token_list.has_init){
		pthread_mutex_lock(&ppc_token_list.mutex);
		dl_list_add_tail(&ppc_token_list.head, &p_token_info->next);
		ret = 0;
		pthread_mutex_unlock(&ppc_token_list.mutex);
	}
	return ret;
}

int del_info_for_ppc_token_list(_int64_t token)
{
	int ret = -1;
	token_info *item, *n;
	if(ppc_token_list.has_init){
		pthread_mutex_lock(&ppc_token_list.mutex);
		dl_list_for_each_safe(item, n, &ppc_token_list.head, token_info, next)
		{
			if(token == item->token){
				dl_list_del(&item->next);
				free_token_info(&item);
				ret = 0;
				break;
			}
		}
		pthread_mutex_unlock(&ppc_token_list.mutex);
	}
	return ret;
}

int get_info_from_ppc_token_list(token_info **p_token_info, _int64_t token)
{
	int ret = -1;
	token_info *item;
	if(p_token_info == NULL){
		DMCLOG_E("p_token_info is null!!");
		return -1;
	}

	if(ppc_token_list.has_init){
		pthread_mutex_lock(&ppc_token_list.mutex);
		dl_list_for_each(item, &ppc_token_list.head, token_info, next)
		{
			if(token == item->token){
				*p_token_info =(token_info *)calloc(1, sizeof(token_info));
				if(*p_token_info != NULL){
					(*p_token_info)->work_dir = (char *)calloc(1, strlen(item->work_dir)+1);
					if((*p_token_info)->work_dir != NULL){
						memcpy((*p_token_info)->work_dir, item->work_dir, strlen(item->work_dir));
						(*p_token_info)->token = item->token;
						ret = 0;
						break;	
					}
				}
			}
		}
		pthread_mutex_unlock(&ppc_token_list.mutex);
	}
	return ret;
}


int update_work_dir_for_ppc_token_list(char *work_dir, _int64_t token)
{
	int ret = -1;
	char *p_work_dir = NULL;
	if(work_dir == NULL){
		return -1;
	}
	if(ppc_token_list.has_init){
		pthread_mutex_lock(&ppc_token_list.mutex);
		token_info *token_dnode = NULL;
		dl_list_for_each(token_dnode, &(ppc_token_list.head), token_info, next)
		{
			if(token_dnode->token == token){
				token_dnode->work_dir = (char *)realloc(token_dnode->work_dir, strlen(work_dir) + 1);
				if(token_dnode->work_dir != NULL){
					memset(token_dnode->work_dir, '\0', strlen(work_dir) + 1);
					memcpy(token_dnode->work_dir, work_dir, strlen(work_dir));
					DMCLOG_D("token_dnode->work_dir: %s", token_dnode->work_dir);
					pthread_mutex_unlock(&ppc_token_list.mutex);
					return 0;
				}
				break;
			}
		}
		pthread_mutex_unlock(&ppc_token_list.mutex);
	}
	return -1;
}

char *get_work_dir_from_ppc_token_list(_int64_t token)
{
	char *work_dir = NULL;
	if(ppc_token_list.has_init){
		pthread_mutex_lock(&ppc_token_list.mutex);
		token_info *token_dnode = NULL;
		dl_list_for_each(token_dnode, &(ppc_token_list.head), token_info, next)
		{
			if(token_dnode->token == token){
				work_dir = (char *)calloc(1, strlen(token_dnode->work_dir)+1);
				if(work_dir != NULL){
					memcpy(work_dir, token_dnode->work_dir, strlen(token_dnode->work_dir));
					pthread_mutex_unlock(&ppc_token_list.mutex);
					return work_dir;
				}
				break;
			}
		}
		pthread_mutex_unlock(&ppc_token_list.mutex);
	}
	return NULL;
}


int free_ppc_token_list()
{
	#if 0
	if(ppc_token_list.has_init > 1){
		ppc_token_list.has_init--;
	}
	else if(ppc_token_list.has_init == 1){
	#endif
	if(ppc_token_list.has_init){
		token_info *item, *n;
		pthread_mutex_lock(&ppc_token_list.mutex);
		dl_list_for_each_safe(item, n, &ppc_token_list.head, token_info, next)
		{
			dl_list_del(&item->next);
			free_token_info(&item);
		}
		pthread_mutex_unlock(&ppc_token_list.mutex);
		pthread_mutex_destroy(&ppc_token_list.mutex);
		ppc_token_list.has_init = 0;
	}
	else{
		DMCLOG_E("no need free");
		return -1;
	}
	
	return 0;
}

