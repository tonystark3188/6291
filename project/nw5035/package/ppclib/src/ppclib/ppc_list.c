#include <pthread.h>

#include "ppc_list.h"
#include "base.h"

dl_ppc_fd_list ppc_fd_list;

void free_fd_info(fd_info **p_fd_info)
{
	safe_free((*p_fd_info)->path);
	safe_free(*p_fd_info);
	*p_fd_info = NULL;
}

int init_ppc_fd_list()
{	
	memset(&ppc_fd_list, 0, sizeof(dl_ppc_fd_list));
	pthread_mutex_init(&ppc_fd_list.mutex, NULL);
	dl_list_init(&ppc_fd_list.head);
	return 0;
}

int add_info_for_ppc_fd_list(fd_info *p_fd_info)
{
	ENTER_FUNC();
	if(p_fd_info == NULL){
		DMCLOG_E("p_fd_info is null!!");
		return -1;
	}
		
	pthread_mutex_lock(&ppc_fd_list.mutex);
	dl_list_add_tail(&ppc_fd_list.head, &p_fd_info->next);
	pthread_mutex_unlock(&ppc_fd_list.mutex);
	EXIT_FUNC();
	return 0;
}

int del_info_for_ppc_fd_list(int fd)
{
	int ret = -1;
	fd_info *item, *n;
	pthread_mutex_lock(&ppc_fd_list.mutex);
	dl_list_for_each_safe(item, n, &ppc_fd_list.head, fd_info, next)
	{
		if(fd == item->fd){
			dl_list_del(&item->next);
			free_fd_info(&item);
			ret = 0;
			break;
		}
	}
	pthread_mutex_unlock(&ppc_fd_list.mutex);
	return ret;
}


int get_info_from_ppc_fd_list(int fd, fd_info **p_fd_info)
{
	int ret = -1;
	fd_info *item;
	if(p_fd_info == NULL){
		DMCLOG_E("p_fd_info is null!!");
		return -1;
	}
	pthread_mutex_lock(&ppc_fd_list.mutex);
	dl_list_for_each(item, &ppc_fd_list.head, fd_info, next)
	{
		if(fd == item->fd){
			*p_fd_info =(fd_info *)calloc(1, sizeof(fd_info));
			if(*p_fd_info != NULL){
				(*p_fd_info)->path = (char *)calloc(1, strlen(item->path)+1);
				if((*p_fd_info)->path != NULL){
					memcpy((*p_fd_info)->path, item->path, strlen(item->path));
					ret = 0;
					break;	
				}
			}
		}
	}
	pthread_mutex_unlock(&ppc_fd_list.mutex);
	return ret;
}

int free_ppc_fd_list()
{
	fd_info *item, *n;
	pthread_mutex_lock(&ppc_fd_list.mutex);
	dl_list_for_each_safe(item, n, &ppc_fd_list.head, fd_info, next)
	{
		dl_list_del(&item->next);
		free_fd_info(&item);
	}
	pthread_mutex_unlock(&ppc_fd_list.mutex);
	pthread_mutex_destroy(&ppc_fd_list.mutex);
	return 0;
}

