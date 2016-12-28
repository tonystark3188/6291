/*
 * =============================================================================
 *
 *       Filename:  token_manage.c
 *
 *    Description:  token management
 *
 *        Version:  1.0
 *        Created:  2016/10/25 15:28
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver, 
 *   Organization:  
 *
 * =============================================================================*/
#include "token_manage.h"

extern int exit_flag;
void token_watch_dog_task(void *self)
{	
	ENTER_FUNC();
	token_list_t *p_token_list = self;
	while(exit_flag == 0)
	{
		pthread_mutex_lock(&p_token_list->mutex);
		token_dnode_t *token_dnode ,*n;
		dl_list_for_each_safe(token_dnode,n,&p_token_list->head,token_dnode_t,next)
		{
			if(token_dnode->watch_time > 0)
			{
				token_dnode->watch_time -= 1;
				
				if(token_dnode->watch_time <= 0)
				{
					dl_list_del(&token_dnode->next);
					safe_free(token_dnode);
				}
			}
		}
		pthread_mutex_unlock(&p_token_list->mutex);
		sleep(1);
	}
	EXIT_FUNC();
	return;
	
}

int token_list_init(token_list_t *p_token_list)
{
	pthread_mutex_init(&p_token_list->mutex,NULL);
	dl_list_init(&p_token_list->head);
	DMCLOG_D("init token list succ");

	PTHREAD_T tid_watch_dog;
	if (0 != PTHREAD_CREATE(&tid_watch_dog, NULL, (void *)token_watch_dog_task, p_token_list))
    { 
        DMCLOG_E("Create watch dog prcs thread failed!");
        return;
	}
	PTHREAD_DETACH(tid_watch_dog);
	return 0;
}

int token_list_destroy(token_list_t *p_token_list)
{
	pthread_mutex_lock(&p_token_list->mutex);
	if(&p_token_list->head == NULL || dl_list_empty(&p_token_list->head))
	{
		DMCLOG_E("the token list is null");
		return -1;
	}
	token_dnode_t *token_dnode = NULL;
	token_dnode_t *n = NULL;
	dl_list_for_each_safe(token_dnode,n,&p_token_list->head,token_dnode_t,next)
	{
		dl_list_del(&token_dnode->next);
		safe_free(token_dnode);
	}
	pthread_mutex_unlock(&p_token_list->mutex);
	pthread_mutex_destroy(&p_token_list->mutex);
	DMCLOG_D("destroy token list succ");
	return 0;
}

bool is_token_login(_In_ uint32_t token,token_list_t *p_token_list)
{
	token_dnode_t *token_dnode = NULL;
	pthread_mutex_lock(&p_token_list->mutex);
	if(&p_token_list->head == NULL || dl_list_empty(&p_token_list->head))
	{
	 DMCLOG_E("the token list is null");
	 pthread_mutex_unlock(&p_token_list->mutex);
	 return FALSE;
	}
	dl_list_for_each(token_dnode, &(p_token_list->head), token_dnode_t, next)
	{
		DMCLOG_D("token_dnode = %u,token = %u",token_dnode,token);
		if(token_dnode == token)
		{
			DMCLOG_D("the token:%u is exist",token);
			pthread_mutex_unlock(&p_token_list->mutex);
			return true;
		}
	}	 
	pthread_mutex_unlock(&p_token_list->mutex);
	return FALSE;
}

struct token_dnode *get_token_info(uint32_t token,token_list_t *p_token_list)
{
	token_dnode_t *token_dnode = NULL;
	pthread_mutex_lock(&p_token_list->mutex);
	if(&p_token_list->head == NULL || dl_list_empty(&p_token_list->head))
	{
	 DMCLOG_E("the token list is null");
	 pthread_mutex_unlock(&p_token_list->mutex);
	 return 0;
	}
	dl_list_for_each(token_dnode, &(p_token_list->head), token_dnode_t, next)
	{
	 if(token_dnode == token)
	 {
		 DMCLOG_D("the token:%s is exist",token);
		 pthread_mutex_unlock(&p_token_list->mutex);
		 return token_dnode;
	 }
	}	 
	pthread_mutex_unlock(&p_token_list->mutex);
	return NULL;
}


int set_token_login(_In_ uint32_t token,_In_ bool isLogin,token_list_t *p_token_list)
{
	token_dnode_t *token_dnode = NULL;
	pthread_mutex_lock(&p_token_list->mutex);
	if(&p_token_list->head == NULL || dl_list_empty(&p_token_list->head))
	{
	 DMCLOG_E("the token list is null");
	 pthread_mutex_unlock(&p_token_list->mutex);
	 return 0;
	}
	dl_list_for_each(token_dnode, &(p_token_list->head), token_dnode_t, next)
	{
	 if(token_dnode == token)
	 {
		 DMCLOG_D("the token:%s is exist",token);
		 token_dnode->isLogin = TRUE;
		 pthread_mutex_unlock(&p_token_list->mutex);
		 return 0;
	 }
	}	 
	pthread_mutex_unlock(&p_token_list->mutex);
	return -1;
}


bool is_token_exist(_In_ uint32_t token,token_list_t *p_token_list)
{
	token_dnode_t *token_dnode = NULL;
	pthread_mutex_lock(&p_token_list->mutex);
	if(&p_token_list->head == NULL || dl_list_empty(&p_token_list->head))
	{
	 DMCLOG_E("the token list is null");
	 pthread_mutex_unlock(&p_token_list->mutex);
	 return FALSE;
	}
	dl_list_for_each(token_dnode, &(p_token_list->head), token_dnode_t, next)
	{
	 if(token_dnode == token)
	 {
		 DMCLOG_D("the token:%u is exist",token);
		 pthread_mutex_unlock(&p_token_list->mutex);
		 return TRUE;
	 }
	}	 
	pthread_mutex_unlock(&p_token_list->mutex);
	return FALSE;
}


uint32_t add_token_to_list(int user_id,char *bucket_name,int authority,bool isPublicUser,token_list_t *p_token_list)
{
	pthread_mutex_lock(&p_token_list->mutex);
	token_dnode_t *fdi = (token_dnode_t *)calloc(1,sizeof(token_dnode_t));
	if(fdi == NULL)
	{
		pthread_mutex_unlock(&p_token_list->mutex);
		return -1;
	}
	fdi->user_id = user_id;
	S_STRNCPY(fdi->bucket_name,bucket_name,32);
	fdi->authority = authority;
	fdi->watch_time = get_token_watch_time();
	fdi->isPublicUser = isPublicUser;
	dl_list_add_tail(&p_token_list->head,&fdi->next);
	DMCLOG_D("add succ user_id:%d,watch_time = %u",user_id,fdi->watch_time);
	pthread_mutex_unlock(&p_token_list->mutex);
	return (uint32_t)fdi;	 
}

int del_token_from_list(_In_ uint32_t token,token_list_t *p_token_list)
{
	ENTER_FUNC();
	if(&p_token_list->head == NULL || dl_list_empty(&p_token_list->head))
	{
		DMCLOG_E("the usr list is null");
		return -1;
	}
	
	if(is_token_exist(token,p_token_list))
	{
		pthread_mutex_lock(&p_token_list->mutex);
		DMCLOG_D("the token:%u is exist",token);
		token_dnode_t *token_dnode = NULL;
		token_dnode_t *n = NULL;
		dl_list_for_each_safe(token_dnode,n,&p_token_list->head,token_dnode_t,next)
		{
			if(token_dnode == token)
			{
				dl_list_del(&token_dnode->next);
				safe_free(token_dnode);
				DMCLOG_D("del token:%u succ",token);
				pthread_mutex_unlock(&p_token_list->mutex);
				EXIT_FUNC();
				return 0;
			}
		}
		pthread_mutex_unlock(&p_token_list->mutex);
	}
	EXIT_FUNC();
	return -1;  
}

int update_token_time(uint32_t token,token_list_t *p_token_list)
{
	pthread_mutex_lock(&p_token_list->mutex);
	token_dnode_t *token_dnode = NULL;
	dl_list_for_each(token_dnode, &(p_token_list->head), token_dnode_t, next)
	{
		if(token_dnode == token)
		{
			token_dnode->watch_time = get_token_watch_time();
			pthread_mutex_unlock(&p_token_list->mutex);
			return 0;
		}
	}
	pthread_mutex_unlock(&p_token_list->mutex);
	return -1;
}


int reset_token_list(bool isLogin,token_list_t *p_token_list)
{
	pthread_mutex_lock(&p_token_list->mutex);
	token_dnode_t *token_dnode = NULL;
	dl_list_for_each(token_dnode, &(p_token_list->head), token_dnode_t, next)
	{
		token_dnode->isLogin = isLogin;
	}
	pthread_mutex_unlock(&p_token_list->mutex);
	return 0;
}

static int get_token_list_len(token_list_t *p_token_list)
{
	int count = 0;

	struct dl_list *item;
	struct dl_list *list = &p_token_list->head;
	pthread_mutex_lock(&p_token_list->mutex);
	for (item = list->next; item != list; item = item->next)
		count++;
	pthread_mutex_unlock(&p_token_list->mutex);
	return count;
}

struct token_dnode **get_token_list(unsigned *nmount_p,token_list_t *p_token_list)
{
	token_dnode_t **dnp = NULL;

	token_dnode_t *fdi, *n;
	*nmount_p = 0;
	struct dl_list *phead = &p_token_list->head;

	if(phead == NULL || dl_list_empty(phead))
	{
		return NULL;
	}
	PTHREAD_MUTEX_LOCK(&p_token_list->mutex);
	unsigned cnt = get_token_list_len(p_token_list);
	dnp = calloc(1,cnt*sizeof(token_dnode_t *));
	dl_list_for_each_safe(fdi,n,phead,token_dnode_t,next)
	{
		dnp[(*nmount_p)++] = fdi;
	}
	PTHREAD_MUTEX_UNLOCK(&p_token_list->mutex);
	return dnp;
}

void free_token_list(token_dnode_t **dnp)
{
	if(dnp == NULL)
	 	return;
	safe_free(dnp);
}

 

