/*
 * =============================================================================
 *
 *       Filename:  session_manage.c
 *
 *    Description:  session management
 *
 *        Version:  1.0
 *        Created:  2016/08/10 15:28
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver, 
 *   Organization:  
 *
 * =============================================================================*/
#include "session_manage.h"
#ifdef SESSION_MANAGE

extern int exit_flag;
void session_watch_dog_task(void *self)
{	
	ENTER_FUNC();
	session_list_t *p_session_list = self;
	while(exit_flag == 0)
	{
		pthread_mutex_lock(&p_session_list->mutex);
		session_dnode_t *session_dnode ,*n;
		dl_list_for_each_safe(session_dnode,n,&p_session_list->head,session_dnode_t,next)
		{
			if(session_dnode->watch_time > 0)
			{
				session_dnode->watch_time -= 1;
				
				if(session_dnode->watch_time <= 0)
				{
					DMCLOG_D("delete session :%s",session_dnode->session);
					dl_list_del(&session_dnode->next);
					safe_free(session_dnode);
				}
			}
		}
		pthread_mutex_unlock(&p_session_list->mutex);
		sleep(1);
	}
	EXIT_FUNC();
	return;
	
}

int session_list_init(session_list_t *p_session_list)
{
	pthread_mutex_init(&p_session_list->mutex,NULL);
	dl_list_init(&p_session_list->head);
	DMCLOG_D("init session list succ");

	PTHREAD_T tid_watch_dog;
	if (0 != PTHREAD_CREATE(&tid_watch_dog, NULL, (void *)session_watch_dog_task, p_session_list))
    { 
        DMCLOG_E("Create watch dog prcs thread failed!");
        return;
	}
	PTHREAD_DETACH(tid_watch_dog);
	return 0;
}

int session_list_destroy(session_list_t *p_session_list)
{
	pthread_mutex_lock(&p_session_list->mutex);
	if(&p_session_list->head == NULL || dl_list_empty(&p_session_list->head))
	{
		DMCLOG_E("the session list is null");
		return -1;
	}
	session_dnode_t *session_dnode = NULL;
	session_dnode_t *n = NULL;
	dl_list_for_each_safe(session_dnode,n,&p_session_list->head,session_dnode_t,next)
	{
		dl_list_del(&session_dnode->next);
		safe_free(session_dnode);
	}
	pthread_mutex_unlock(&p_session_list->mutex);
	pthread_mutex_destroy(&p_session_list->mutex);
	DMCLOG_D("destroy session list succ");
	return 0;
}

bool is_session_login(_In_ char *session,session_list_t *p_session_list)
{
	if(session == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	session_dnode_t *session_dnode = NULL;
	pthread_mutex_lock(&p_session_list->mutex);
	if(&p_session_list->head == NULL || dl_list_empty(&p_session_list->head))
	{
	 DMCLOG_E("the session list is null");
	 pthread_mutex_unlock(&p_session_list->mutex);
	 return FALSE;
	}
	dl_list_for_each(session_dnode, &(p_session_list->head), session_dnode_t, next)
	{
	 if(!strcmp(session_dnode->session,session))
	 {
		 DMCLOG_D("the session:%s is exist",session);
		 pthread_mutex_unlock(&p_session_list->mutex);
		 return session_dnode->isLogin;
	 }
	}	 
	pthread_mutex_unlock(&p_session_list->mutex);
	return FALSE;
}

int set_session_login(_In_ char *session,_In_ bool isLogin,session_list_t *p_session_list)
{
	if(session == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	session_dnode_t *session_dnode = NULL;
	pthread_mutex_lock(&p_session_list->mutex);
	if(&p_session_list->head == NULL || dl_list_empty(&p_session_list->head))
	{
	 DMCLOG_E("the session list is null");
	 pthread_mutex_unlock(&p_session_list->mutex);
	 return 0;
	}
	dl_list_for_each(session_dnode, &(p_session_list->head), session_dnode_t, next)
	{
	 if(!strcmp(session_dnode->session,session))
	 {
		 DMCLOG_D("the session:%s is exist",session);
		 session_dnode->isLogin = TRUE;
		 pthread_mutex_unlock(&p_session_list->mutex);
		 return 0;
	 }
	}	 
	pthread_mutex_unlock(&p_session_list->mutex);
	return -1;
}


bool is_session_exist(_In_ char *session,session_list_t *p_session_list)
{
	if(session == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	session_dnode_t *session_dnode = NULL;
	pthread_mutex_lock(&p_session_list->mutex);
	if(&p_session_list->head == NULL || dl_list_empty(&p_session_list->head))
	{
	 DMCLOG_E("the session list is null");
	 pthread_mutex_unlock(&p_session_list->mutex);
	 return FALSE;
	}
	dl_list_for_each(session_dnode, &(p_session_list->head), session_dnode_t, next)
	{
	 if(!strcmp(session_dnode->session,session))
	 {
		 DMCLOG_D("the session:%s is exist",session);
		 pthread_mutex_unlock(&p_session_list->mutex);
		 return TRUE;
	 }
	}	 
	pthread_mutex_unlock(&p_session_list->mutex);
	return FALSE;
}


int add_session_to_list(char *session,session_list_t *p_session_list)
{
	char root_pwd[32] = {0};
	if(session == NULL)
	{
		DMCLOG_E("para error");
		return -1;
	}
	if(is_session_exist(session,p_session_list))
	{
		DMCLOG_D("the session:%s is exist",session);
		update_session_time(session,p_session_list);
		return 0;
	}
	DMCLOG_D("session = %s",session);
	pthread_mutex_lock(&p_session_list->mutex);
	session_dnode_t *fdi = (session_dnode_t *)calloc(1,sizeof(session_dnode_t));
	if(fdi == NULL)
	{
		pthread_mutex_unlock(&p_session_list->mutex);
		return -1;
	}
	strcpy(fdi->session,session);
	if(dm_get_root_pwd(root_pwd) == 0)
	{
		DMCLOG_D("root_pwd = %s",root_pwd);
		fdi->isLogin = FALSE;
	}else{
		fdi->isLogin = TRUE;
	}
		
	fdi->watch_time = get_session_watch_time();
	dl_list_add_tail(&p_session_list->head,&fdi->next);
	DMCLOG_D("add succ session:%s,watch_time = %u",session,fdi->watch_time);
	pthread_mutex_unlock(&p_session_list->mutex);
	return 0;	 
}

int del_session_from_list(_In_ char *session,session_list_t *p_session_list)
{
	ENTER_FUNC();
	if(&p_session_list->head == NULL || dl_list_empty(&p_session_list->head))
	{
		DMCLOG_E("the usr list is null");
		return -1;
	}
	
	if(is_session_exist(session,p_session_list))
	{
		pthread_mutex_lock(&p_session_list->mutex);
		DMCLOG_D("the session:%s is exist",session);
		session_dnode_t *session_dnode = NULL;
		session_dnode_t *n = NULL;
		dl_list_for_each_safe(session_dnode,n,&p_session_list->head,session_dnode_t,next)
		{
			if(!strcmp(session_dnode->session,session))
			{
				dl_list_del(&session_dnode->next);
				safe_free(session_dnode);
				DMCLOG_D("del session:%s succ",session);
				pthread_mutex_unlock(&p_session_list->mutex);
				EXIT_FUNC();
				return 0;
			}
		}
		pthread_mutex_unlock(&p_session_list->mutex);
	}
	EXIT_FUNC();
	return -1;  
}

int update_session_time(_In_ char *session,session_list_t *p_session_list)
{
	if( session == NULL)
	{
		DMCLOG_E("para error");
		return -1;
	}
	pthread_mutex_lock(&p_session_list->mutex);
	session_dnode_t *session_dnode = NULL;
	dl_list_for_each(session_dnode, &(p_session_list->head), session_dnode_t, next)
	{
		if(!strcmp(session_dnode->session,session))
		{
			session_dnode->watch_time = get_session_watch_time();
			pthread_mutex_unlock(&p_session_list->mutex);
			return 0;
		}
	}
	pthread_mutex_unlock(&p_session_list->mutex);
	return -1;
}


int reset_session_list(bool isLogin,session_list_t *p_session_list)
{
	pthread_mutex_lock(&p_session_list->mutex);
	session_dnode_t *session_dnode = NULL;
	dl_list_for_each(session_dnode, &(p_session_list->head), session_dnode_t, next)
	{
		session_dnode->isLogin = isLogin;
	}
	pthread_mutex_unlock(&p_session_list->mutex);
	return 0;
}

static int get_session_list_len(session_list_t *p_session_list)
{
	int count = 0;

	struct dl_list *item;
	struct dl_list *list = &p_session_list->head;
	pthread_mutex_lock(&p_session_list->mutex);
	for (item = list->next; item != list; item = item->next)
		count++;
	pthread_mutex_unlock(&p_session_list->mutex);
	return count;
}

struct session_dnode **get_session_list(unsigned *nmount_p,session_list_t *p_session_list)
{
	session_dnode_t **dnp = NULL;

	session_dnode_t *fdi, *n;
	*nmount_p = 0;
	struct dl_list *phead = &p_session_list->head;

	if(phead == NULL || dl_list_empty(phead))
	{
		return NULL;
	}
	PTHREAD_MUTEX_LOCK(&p_session_list->mutex);
	unsigned cnt = get_session_list_len(p_session_list);
	dnp = calloc(1,cnt*sizeof(session_dnode_t *));
	dl_list_for_each_safe(fdi,n,phead,session_dnode_t,next)
	{
		dnp[(*nmount_p)++] = fdi;
	}
	PTHREAD_MUTEX_UNLOCK(&p_session_list->mutex);
	return dnp;
}

void free_session_list(struct session_dnode **dnp)
{
	if(dnp == NULL)
	 	return;
	safe_free(dnp);
}
#endif
 

