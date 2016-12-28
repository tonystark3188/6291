/*
 * =============================================================================
 *
 *       Filename:  router_task.c
 *
 *    Description:  query the router infomation and sending info to dev according to dev quest cmd
 *
 *        Version:  1.0
 *        Created:  2015/08/20 14:33
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver, 
 *   Organization:  
 *
 * =============================================================================*/
#include "router_task.h"
#include "base.h"
#include "usr_manage.h"
#include "util.h"
#include "hd_route.h"
#include "router_inotify.h"
#include "mcu.h"
#include "router_defs.h"

dev_list_t dev_list;

int _update_seq_to_dev(_In_ char *ip,_In_ unsigned seq,_In_ int type)
{
	dev_list_t *p_dev_list = &dev_list;
	if(ip == NULL)
	{
		DMCLOG_E("para error");
		return -1;
	}
	pthread_mutex_lock(&p_dev_list->mutex);
	dev_dnode_t *dev_dnode = NULL;
	dl_list_for_each(dev_dnode, &(p_dev_list->head), dev_dnode_t, next)
	{
		if(!strcmp(dev_dnode->ip,ip))
		{
			DMCLOG_D("the ip is exist");
			if(type == POWER_TYPE)
			{
				//power
				DMCLOG_D("power seq = %u",seq);
				dev_dnode->power_seq = seq;
			}else if(type == DISK_TYPE)
			{
				//disk
				DMCLOG_D("disk seq = %u",seq);
				dev_dnode->disk_seq = seq;
			}else if(type == SSID_TYPE)
			{
				//ssid
				dev_dnode->ssid_seq = seq;
			}else if(type == DB_TYPE)
			{
				//data base
				dev_dnode->db_seq= seq;
			}else if(type == PWD_TYPE)
			{
				//password
				dev_dnode->pwd_seq = seq;
				DMCLOG_D("pwd_seq = %u",dev_dnode->pwd_seq);
			}
			break;
		}
	}
	pthread_mutex_unlock(&p_dev_list->mutex);
	return -1;
}

int _dev_list_init()
{
	dev_list_t *p_dev_list = &dev_list;
	memset(p_dev_list,0,sizeof(dev_list_t));
	pthread_mutex_init(&p_dev_list->mutex,NULL);
	dl_list_init(&p_dev_list->head);
	DMCLOG_D("init dev list succ");
	return 0;
}

static int _get_list_len(dev_list_t *p_dev_list)
{
	struct dl_list *item;
	struct dl_list *list = &p_dev_list->head;
	int count = 0;
	for (item = list->next; item != list; item = item->next)
		count++;
	return count;
}


bool is_dev_exist(char *ip)
{
	dev_list_t *p_dev_list = &dev_list;
	dev_dnode_t *dev_dnode = NULL;
	pthread_mutex_lock(&p_dev_list->mutex);
	if(&p_dev_list->head == NULL || dl_list_empty(&p_dev_list->head))
	{
		DMCLOG_E("the dev list is null");
		pthread_mutex_unlock(&p_dev_list->mutex);
		return FALSE;
	}
	dl_list_for_each(dev_dnode, &(p_dev_list->head), dev_dnode_t, next)
	{
		if(!strcmp(dev_dnode->ip,ip))
		{
			DMCLOG_D("the ip is exist");
			pthread_mutex_unlock(&p_dev_list->mutex);
			return TRUE;
		}
	}	
	pthread_mutex_unlock(&p_dev_list->mutex);
	return FALSE;
}

int _update_dev_to_list(char *ip,uint16_t port,uint8_t request_type)
{
	dev_list_t *p_dev_list = &dev_list;
	if(ip == NULL)
	{
		DMCLOG_E("para error");
		return -1;
	}
	pthread_mutex_lock(&p_dev_list->mutex);
	dev_dnode_t *dev_dnode = NULL;
	dl_list_for_each(dev_dnode, &(p_dev_list->head), dev_dnode_t, next)
	{
		if(!strcmp(dev_dnode->ip,ip))
		{
			DMCLOG_D("the ip is exist");
			dev_dnode->port = port;
			dev_dnode->request_type = request_type;
			router_para_listen_port = port;
			router_request_type = request_type;
			pthread_mutex_unlock(&p_dev_list->mutex);
			return 0;
		}
	}
	pthread_mutex_unlock(&p_dev_list->mutex);
	return -1;
}


int _add_dev_to_list(char *ip,uint16_t port,uint8_t request_type)
{
	dev_list_t *p_dev_list = &dev_list;
	if(ip == NULL)
	{
		DMCLOG_E("para error");
		return -1;
	}
	
	DMCLOG_D("ip = %s",ip);
	if(is_dev_exist(ip))
	{
		DMCLOG_D("the ip:%s is exist",ip);
		_update_dev_to_list(ip,port,request_type);
		return 0;
	}
	DMCLOG_D("ip = %s",ip);
	pthread_mutex_lock(&p_dev_list->mutex);
	dev_dnode_t *fdi = (dev_dnode_t *)calloc(1,sizeof(dev_dnode_t));
	if(fdi == NULL)
	{
		pthread_mutex_unlock(&p_dev_list->mutex);
		return -1;
	}
	DMCLOG_D("ip = %s",ip);
	strcpy(fdi->ip,ip);
	fdi->port = port;
	fdi->request_type = request_type;
	router_para_listen_port = port;
	router_request_type = request_type;
	dl_list_add_tail(&p_dev_list->head,&fdi->next);
	DMCLOG_D("add succ ip:%s",ip);
	pthread_mutex_unlock(&p_dev_list->mutex);
	return 0;
}

int _check_ip_to_list(char *ip)
{
	dev_list_t *p_dev_list = &dev_list;
	if(is_dev_exist(ip))
	{
		DMCLOG_D("the ip:%s is exist",ip);
		return 0;
	}
	DMCLOG_D("ip = %s",ip);
	pthread_mutex_lock(&p_dev_list->mutex);
	dev_dnode_t *fdi = (dev_dnode_t *)calloc(1,sizeof(dev_dnode_t));
	if(fdi == NULL)
	{
		pthread_mutex_unlock(&p_dev_list->mutex);
		return -1;
	}
	DMCLOG_D("ip = %s",ip);
	strcpy(fdi->ip,ip);
	fdi->port = 13101;
	fdi->request_type = 7;
	router_para_listen_port = 13101;
	router_request_type = 7;
	dl_list_add_tail(&p_dev_list->head,&fdi->next);
	DMCLOG_D("add succ ip:%s",ip);
	pthread_mutex_unlock(&p_dev_list->mutex);
	return 0;
}

int _del_dev_from_list_by_ip(char *ip)
{
	dev_list_t *p_dev_list = &dev_list;
	if(&p_dev_list->head == NULL || dl_list_empty(&p_dev_list->head))
	{
		DMCLOG_E("the dev list is null");
		return -1;
	}
	ENTER_FUNC();
	
	if(is_dev_exist(ip))
	{
		pthread_mutex_lock(&p_dev_list->mutex);
		DMCLOG_D("the ip:%s is exist",ip);
		dev_dnode_t *dev_dnode = NULL;
		dev_dnode_t *n = NULL;
		dl_list_for_each_safe(dev_dnode,n,&p_dev_list->head,dev_dnode_t,next)
		{
			if(!strcmp(dev_dnode->ip,ip))
			{
				dl_list_del(&dev_dnode->next);
				DMCLOG_D("del ip:%s succ",ip);
				safe_free(dev_dnode);
				pthread_mutex_unlock(&p_dev_list->mutex);
				EXIT_FUNC();
				return 0;
			}
		}
		pthread_mutex_unlock(&p_dev_list->mutex);
	}
	EXIT_FUNC();
	return -1;
}

int _destory_dev_list()
{
	int ret = -1;
	dev_list_t *p_dev_list = &dev_list;
	if(&p_dev_list->head == NULL || dl_list_empty(&p_dev_list->head))
	{
		DMCLOG_E("the dev list is null");
		goto exit;
	}
	dev_dnode_t *dev_dnode = NULL;
	dev_dnode_t *n = NULL;
	dl_list_for_each_safe(dev_dnode,n,&p_dev_list->head,dev_dnode_t,next)
	{
		dl_list_del(&dev_dnode->next);
		safe_free(dev_dnode);
	}
	ret = 0;
exit:
	pthread_mutex_destroy(&p_dev_list->mutex);
	DMCLOG_D("destroy dev list succ");
	return 0;
}


struct dev_dnode **_get_dev_list(unsigned *nmount_p)
{
	dev_list_t *p_dev_list = &dev_list;
    dev_dnode_t **dnp = NULL;
    
    dev_dnode_t *fdi, *n;
    *nmount_p = 0;
    struct dl_list *phead = &p_dev_list->head;
    
    if(phead == NULL || dl_list_empty(phead))
    {
        return NULL;
    }
    PTHREAD_MUTEX_LOCK(&p_dev_list->mutex);
	unsigned cnt = _get_list_len(p_dev_list);
    dnp = calloc(1,cnt*sizeof(dev_dnode_t *));
    dl_list_for_each_safe(fdi,n,phead,dev_dnode_t,next)
    {
        dnp[(*nmount_p)++] = fdi;
    }
    PTHREAD_MUTEX_UNLOCK(&p_dev_list->mutex);
    return dnp;
}

void _free_dev_list(struct dev_dnode **dnp)
{
	if(dnp == NULL)
		return;
	safe_free(dnp);
}

int dm_get_status_changed()
{
	int fh = NULL;
	int statusFlag = 0;
	fh=open(rtl_encryp_control, O_RDWR);
	if(fh==NULL){
		DMCLOG_D("open MCU proc error");
		statusFlag = disk_ssid_changed;
	}
	else{
		DMCLOG_D("open MCU proc success");
		//int statusFlag = 0x000111;
		statusFlag = power_disk_ssid_changed;
		close(fh);
	}
	
	return statusFlag;
}


