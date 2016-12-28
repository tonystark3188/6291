/*
 * =============================================================================
 *
 *       Filename:  media_manage.c
 *
 *    Description:  media infomation process for dm init module
 *
 *        Version:  1.0
 *        Created:  2016/11/22 10:41
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver, 
 *   Organization:  
 *
 * =============================================================================*/
#include <sys/sysinfo.h>
#include <stdbool.h>

#include "media_manage.h"
//#include "media_process.h"
#include "util.h"

static media_list_t g_media_list;
static unsigned int g_local_pri = 0;
extern int exit_flag;

//return true  时，优先级排序: 0 优先级最高
static int cmp_pri(pqueue_pri_t next, pqueue_pri_t curr)
{
	if(next.user_pri > curr.user_pri)
	{
		return 1;
	}
	else if (next.user_pri == curr.user_pri)
	{
		return (next.local_pri > curr.local_pri);
	}
	else
	{
		return 0;
	}
}

static pqueue_pri_t get_pri(void *a)
{
	return ((media_dnode_t *) a)->pri;
}

static void set_pri(void *a, pqueue_pri_t pri)
{
	memcpy(&((media_dnode_t *) a)->pri,&pri,sizeof(pqueue_pri_t));
}


static size_t get_pos(void *a)
{
	return ((media_dnode_t *) a)->pos;
}

static void set_pos(void *a, size_t pos)
{
	((media_dnode_t *) a)->pos = pos;
}

static void media_watch_dog_task(void *self)
{	
	ENTER_FUNC();
	media_list_t *p_media_list = (media_list_t *)self;
	media_dnode_t *media_dnode;
	while(exit_flag == 0)
	{
		pthread_mutex_lock(&p_media_list->mutex);
		media_dnode = pqueue_pop(p_media_list->pqueue);
		//如果优先级队列里面没有数据，将g_local_pri归零
		if(!media_dnode)
		{
			g_local_pri = 0;
		}
		pthread_mutex_unlock(&p_media_list->mutex);
		if(media_dnode)
		{
			DMCLOG_D("user_pri:%d , local_pri:%d",media_dnode->pri.user_pri,media_dnode->pri.local_pri);
			if(!media_dnode->on_new_file_handle)
			{
				DMCLOG_E("media_dnode->on_new_file_handle is null");
				media_dnode_free(media_dnode);
			}
			else
			{
				if(media_prc_thpool_add_work(media_dnode->on_new_file_handle,media_dnode) != 0)
				{
					DMCLOG_E("media_prc_thpool_add_work fail");
					media_dnode_free(media_dnode);
				}
			}	
			continue;
		}
		sleep(1);
	}
	EXIT_FUNC();
	return;
	
}


int media_list_init()
{
	int ret = -1;
	media_list_t *p_media_list = &g_media_list;
	p_media_list->pqueue = pqueue_init(10, cmp_pri, get_pri, set_pri, get_pos, set_pos);
	if(!p_media_list->pqueue)
	{
		DMCLOG_E("pqueue_init fail");
		goto exit;
	}
	PTHREAD_T tid_media_dog;
	if (0 != PTHREAD_CREATE(&tid_media_dog, NULL, (void *)media_watch_dog_task,(void *)p_media_list))
    { 
        DMCLOG_E("Create watch dog prcs thread failed!");
        goto exit;
	}
	PTHREAD_DETACH(tid_media_dog);
	pthread_mutex_init(&p_media_list->mutex,NULL);	
	ret = 0;
exit:
	if(ret < 0)
	{
		if(p_media_list->pqueue)
		{
			pqueue_free(p_media_list->pqueue);
			p_media_list->pqueue = NULL;
		}
	}
	return ret;
}

int media_list_destroy()
{
	media_list_t *p_media_list = &g_media_list;
	if(p_media_list->pqueue)
	{
		media_dnode_t *media_dnode;
		while((media_dnode = pqueue_pop(p_media_list->pqueue)))
		{
			media_dnode_free(media_dnode);
		}
		pqueue_free(p_media_list->pqueue);
		p_media_list->pqueue = NULL;
	}
	pthread_mutex_destroy(&p_media_list->mutex);
	return 0;
}

int add_media_to_list(char *path,int media_type,
					ON_NEW_FILE_HANDLE *on_new_file_handle,
					unsigned int pri)
{
	ENTER_FUNC();
	int ret = -1;
	media_dnode_t *fdi = NULL;
	media_list_t *p_media_list = &g_media_list;
	if(!path)
	{
		DMCLOG_E("path is null");
		goto exit;
	}
	int pathlen = strlen(path) + 1;
	fdi = (media_dnode_t *)calloc(1,sizeof(media_dnode_t));
	if(!fdi)
	{
		DMCLOG_E("fdi calloc fail");
		goto exit;
	}
	fdi->path = (char *)calloc(1,pathlen);
	if(!fdi->path)
	{
		DMCLOG_E("fdi->path calloc fail");
		goto exit;
	}
	if(!p_media_list->pqueue)
	{
		DMCLOG_E("pqueue uninit");
		goto exit;
	}
	pthread_mutex_lock(&p_media_list->mutex);
	strcpy(fdi->path,path);
	fdi->media_type = media_type;
	fdi->on_new_file_handle = on_new_file_handle;
	fdi->pri.user_pri = pri;
	fdi->pri.local_pri = g_local_pri++;
	DMCLOG_D("pri.user_pri:%d,pri.local_pri:%d",fdi->pri.user_pri,fdi->pri.local_pri);
	if(pqueue_insert(p_media_list->pqueue,(void *)fdi) != 0)
	{
		DMCLOG_E("pqueue_insert fail");
		pthread_mutex_unlock(&p_media_list->mutex);
		goto exit;
	}
	pthread_mutex_unlock(&p_media_list->mutex);
	ret = 0;
exit:
	if(ret < 0)
	{
		media_dnode_free(fdi);
	}
	EXIT_FUNC();
	return ret;	
}



