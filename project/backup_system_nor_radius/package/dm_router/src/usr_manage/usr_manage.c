/*
 * =============================================================================
 *
 *       Filename:  usr_manage.c
 *
 *    Description:  user infomation process for dm init module
 *
 *        Version:  1.0
 *        Created:  2015/08/21 10:41
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver, 
 *   Organization:  
 *
 * =============================================================================*/
#include <sys/sysinfo.h>
#include <stdbool.h>

#include "usr_manage.h"
#include "base.h"
#include "router_task.h"
#include "util.h"
struct usr_dnode usr_list;

int usr_list_init()
{
	usr_list_t *p_usr_list = &usr_list;
	memset(p_usr_list,0,sizeof(usr_list_t));
	pthread_mutex_init(&p_usr_list->mutex,NULL);
	dl_list_init(&p_usr_list->head);
	DMCLOG_D("init usr list succ");
	return 0;
}

int usr_list_destroy()
{
	usr_list_t *p_usr_list = &usr_list;
	if(&p_usr_list->head == NULL || dl_list_empty(&p_usr_list->head))
	{
		DMCLOG_E("the dev list is null");
		return -1;
	}
	usr_dnode_t *usr_dnode = NULL;
	usr_dnode_t *n = NULL;
	dl_list_for_each_safe(usr_dnode,n,&p_usr_list->head,usr_dnode_t,next)
	{
		dl_list_del(&usr_dnode->next);
		safe_free(usr_dnode);
	}
	pthread_mutex_destroy(&p_usr_list->mutex);
	DMCLOG_D("destroy usr list succ");
	return 0;
}

static bool is_usr_exist(char *ip)
{
	usr_list_t *p_usr_list = &usr_list;
	usr_dnode_t *usr_dnode = NULL;
	pthread_mutex_lock(&p_usr_list->mutex);
	if(&p_usr_list->head == NULL || dl_list_empty(&p_usr_list->head))
	{
		DMCLOG_E("the usr list is null");
		pthread_mutex_unlock(&p_usr_list->mutex);
		return FALSE;
	}
	dl_list_for_each(usr_dnode, &(p_usr_list->head), usr_dnode_t, next)
	{
		if(!strcmp(usr_dnode->ip,ip))
		{
			//DMCLOG_D("the ip is exist");
			pthread_mutex_unlock(&p_usr_list->mutex);
			return TRUE;
		}
	}	
	pthread_mutex_unlock(&p_usr_list->mutex);
	return FALSE;
}

int add_usr_to_list(char *ip)
{
	usr_list_t *p_usr_list = &usr_list;
	if(ip == NULL)
	{
		DMCLOG_E("para error");
		return -1;
	}
	if(is_usr_exist(ip))
	{
		//DMCLOG_D("the ip:%s is exist",ip);
		update_usr_time(ip);
		return 0;
	}
	DMCLOG_D("ip = %s",ip);
	int32_t start_time;
	struct sysinfo info; 
	sysinfo(&info); 
	start_time = info.uptime;
	pthread_mutex_lock(&p_usr_list->mutex);
	usr_dnode_t *fdi = (usr_dnode_t *)calloc(1,sizeof(usr_dnode_t));
	if(fdi == NULL)
	{
		pthread_mutex_unlock(&p_usr_list->mutex);
		return -1;
	}
	strcpy(fdi->ip,ip);
	fdi->start_time = start_time;
	dl_list_add_tail(&p_usr_list->head,&fdi->next);
	DMCLOG_D("add succ ip:%s",ip);
	pthread_mutex_unlock(&p_usr_list->mutex);
	return 0;	
}

int del_usr_from_list(char *ip)
{
	usr_list_t *p_usr_list = &usr_list;
	if(&p_usr_list->head == NULL || dl_list_empty(&p_usr_list->head))
	{
		DMCLOG_E("the usr list is null");
		return -1;
	}
	ENTER_FUNC();
	
	if(is_usr_exist(ip))
	{
		pthread_mutex_lock(&p_usr_list->mutex);
		DMCLOG_D("the ip:%s is exist",ip);
		usr_dnode_t *usr_dnode = NULL;
		usr_dnode_t *n = NULL;
		dl_list_for_each_safe(usr_dnode,n,&p_usr_list->head,usr_dnode_t,next)
		{
			if(!strcmp(usr_dnode->ip,ip))
			{
				dl_list_del(&usr_dnode->next);
				safe_free(usr_dnode);
				DMCLOG_D("del ip:%s succ",ip);
				pthread_mutex_unlock(&p_usr_list->mutex);
				EXIT_FUNC();
				return 0;
			}
		}
		pthread_mutex_unlock(&p_usr_list->mutex);
	}
	EXIT_FUNC();
	return -1;	
}

int update_usr_time(char *ip)
{
	struct  sysinfo info; 
	int32_t timep;
	usr_list_t *p_usr_list = &usr_list;
	if( ip == NULL)
	{
		DMCLOG_E("para error");
		return -1;
	}
	pthread_mutex_lock(&p_usr_list->mutex);
	usr_dnode_t *usr_dnode = NULL;
	dl_list_for_each(usr_dnode, &(p_usr_list->head), usr_dnode_t, next)
	{
		if(!strcmp(usr_dnode->ip,ip))
		{
			//DMCLOG_D("the ip is exist");
			sysinfo(&info); 
			timep = info.uptime;
			usr_dnode->start_time = timep;
			pthread_mutex_unlock(&p_usr_list->mutex);
			return 0;
		}
	}
	pthread_mutex_unlock(&p_usr_list->mutex);
	return -1;
}
static int _get_usr_list_len(usr_list_t *p_usr_list)
{
	struct dl_list *item;
	struct dl_list *list = &p_usr_list->head;
	int count = 0;
	for (item = list->next; item != list; item = item->next)
		count++;
	return count;
}

struct usr_dnode **_get_usr_list(unsigned *nmount_p)
{
	usr_list_t *p_usr_list = &usr_list;
    usr_dnode_t **dnp = NULL;
    
    usr_dnode_t *fdi, *n;
    *nmount_p = 0;
    struct dl_list *phead = &p_usr_list->head;
    
    if(phead == NULL || dl_list_empty(phead))
    {
        return NULL;
    }
    PTHREAD_MUTEX_LOCK(&p_usr_list->mutex);
	unsigned cnt = _get_usr_list_len(p_usr_list);
    dnp = calloc(1,cnt*sizeof(usr_dnode_t *));
    dl_list_for_each_safe(fdi,n,phead,usr_dnode_t,next)
    {
        dnp[(*nmount_p)++] = fdi;
    }
    PTHREAD_MUTEX_UNLOCK(&p_usr_list->mutex);
    return dnp;
}

void _free_usr_list(struct usr_dnode **dnp)
{
	if(dnp == NULL)
		return;
	safe_free(dnp);
}



