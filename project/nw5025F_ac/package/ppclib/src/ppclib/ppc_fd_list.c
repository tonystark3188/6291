#include <pthread.h>

#include "ppc_fd_list.h"
#include "base.h"

#define V_FD_MAX	65536
#define V_FD_MIN	1
dl_ppc_fd_list ppc_fd_list;

void free_fd_info(fd_info **p_fd_info)
{
	if(*p_fd_info != NULL){
		safe_free((*p_fd_info)->path);
		if((*p_fd_info)->p_data != NULL){
			int i = 0;
			for(i = 0;;i++){
		        if((*p_fd_info)->p_data[i] == NULL)
		            break;
		        safe_free((*p_fd_info)->p_data[i]);
		    }
		    safe_free((*p_fd_info)->p_data);
		}
		
		safe_free(*p_fd_info);		
		*p_fd_info = NULL;
	}
}

int apply_v_fd()
{
	int v_fd = 0;
	int has_match = 0;

	for(has_match = 0, v_fd = V_FD_MIN; v_fd < V_FD_MAX; has_match = 0, v_fd++){
		fd_info *item;
		dl_list_for_each(item, &ppc_fd_list.head, fd_info, next)
		{
			 //DMCLOG_D("111item->v_fd: %d, v_fd: %d", item->v_fd, v_fd);
			 if(v_fd == item->v_fd){
				has_match = 1;
				break;
			 }
		}

		if(!has_match){
			return v_fd;
		}
	}

	return -1;
}

int init_ppc_fd_list()
{	
	if(0 == ppc_fd_list.has_init){
		memset(&ppc_fd_list, 0, sizeof(dl_ppc_fd_list));
		pthread_mutex_init(&ppc_fd_list.mutex, NULL);
		dl_list_init(&ppc_fd_list.head);
		ppc_fd_list.has_init = 1;
	}
	return 0;
}

int add_info_for_ppc_fd_list(fd_info *p_fd_info)
{
	int ret = -1;
	int v_fd;
	if(p_fd_info == NULL){
		DMCLOG_E("p_fd_info is null!!");
		return ret;
	}
	if(ppc_fd_list.has_init){	
		DMCLOG_D("has init");
		pthread_mutex_lock(&ppc_fd_list.mutex);
		v_fd = apply_v_fd();
		if(v_fd < V_FD_MIN){
			DMCLOG_E("invalid fd");
			pthread_mutex_unlock(&ppc_fd_list.mutex);
			return ret;
		}
		DMCLOG_D("v_fd: %d", v_fd);
		p_fd_info->v_fd  = v_fd;
		dl_list_add_tail(&ppc_fd_list.head, &p_fd_info->next);
		ret = 0;
		pthread_mutex_unlock(&ppc_fd_list.mutex);
	}
	else
		DMCLOG_D("not init");
	return ret;
}

int del_info_for_ppc_fd_list(int v_fd)
{
	int ret = -1;
	fd_info *item, *n;
	if(ppc_fd_list.has_init){
		pthread_mutex_lock(&ppc_fd_list.mutex);
		dl_list_for_each_safe(item, n, &ppc_fd_list.head, fd_info, next)
		{
			if(v_fd == item->v_fd){
				dl_list_del(&item->next);
				free_fd_info(&item);
				ret = 0;
				break;
			}
		}
		pthread_mutex_unlock(&ppc_fd_list.mutex);
	}
	return ret;
}

int inc_offset_for_ppc_fd_list(int v_fd, off_t offset)
{
	int ret = -1;
	fd_info *item;
	if(ppc_fd_list.has_init){
		pthread_mutex_lock(&ppc_fd_list.mutex);
		dl_list_for_each(item, &ppc_fd_list.head, fd_info, next)
		{
			if(v_fd == item->v_fd){
				if(item->type == fd_type_opendir){
					if(item->offset + offset <= item->file_count + 1){
						item->offset += offset;
						ret = 0;
						break;
					}
				}
				else{
					//if(item->offset + offset <= item->file_len){
						item->offset += offset;
						ret = 0;
						break;
					//}				
				}
			}
		}
		pthread_mutex_unlock(&ppc_fd_list.mutex);
	}
	return ret;
}

int set_offset_for_ppc_fd_list(int v_fd, off_t offset)
{
	int ret = -1;
	fd_info *item;
	if(ppc_fd_list.has_init){
		pthread_mutex_lock(&ppc_fd_list.mutex);
		dl_list_for_each(item, &ppc_fd_list.head, fd_info, next)
		{
			if(v_fd == item->v_fd){
				if(item->type == fd_type_opendir){
					if(offset <= item->file_count){
						item->offset = offset;
						ret = 0;
						break;
					}
				}
				else{
					//if(offset <= item->file_len){
						item->offset = offset;
						ret = 0;
						break;
					//}				
				}
			}
		}
		pthread_mutex_unlock(&ppc_fd_list.mutex);
	}
	return ret;
}

int get_socket_fd_from_ppc_fd_list(int v_fd)
{
	int ret = -1;
	int socket_fd = 0;
	int has_match = 0;
	fd_info *item;

	if(ppc_fd_list.has_init){
		pthread_mutex_lock(&ppc_fd_list.mutex);
		dl_list_for_each(item, &ppc_fd_list.head, fd_info, next)
		{
			if(v_fd == item->v_fd){
				socket_fd = item->socket_fd;
				has_match = 1;
				break;
			}
		}
		pthread_mutex_unlock(&ppc_fd_list.mutex);
	}
	
	if(has_match)
		return socket_fd;
	else
		return -1;
}

off_t get_offset_fd_from_ppc_fd_list(int v_fd)
{
	int ret = -1;
	off_t offset = 0;
	int has_match = 0;
	fd_info *item;

	if(ppc_fd_list.has_init){
		pthread_mutex_lock(&ppc_fd_list.mutex);
		dl_list_for_each(item, &ppc_fd_list.head, fd_info, next)
		{
			if(v_fd == item->v_fd){
				offset = item->offset;
				has_match = 1;
				break;
			}
		}
		pthread_mutex_unlock(&ppc_fd_list.mutex);
	}
	if(has_match)
		return offset;
	else
		return -1;
}

int ch_socket_fd_for_ppc_fd_list(int v_fd, int socket_fd)
{
	int has_match = 0;
	fd_info *item;
	if(ppc_fd_list.has_init){
		pthread_mutex_lock(&ppc_fd_list.mutex);
		dl_list_for_each(item, &ppc_fd_list.head, fd_info, next)
		{
			if(v_fd == item->v_fd){
				item->socket_fd = socket_fd; 
				has_match = 1;
				break;
			}
		}
		pthread_mutex_unlock(&ppc_fd_list.mutex);
	}
	
	if(has_match)
		return 0;
	else
		return -1;
}

int get_info_from_ppc_fd_list(int v_fd, fd_info **p_fd_info)
{
	int ret = -1;
	fd_info *item;
	if(p_fd_info == NULL){
		DMCLOG_E("p_fd_info is null!!");
		return ret;
	}
	
	if(ppc_fd_list.has_init){
		pthread_mutex_lock(&ppc_fd_list.mutex);
		dl_list_for_each(item, &ppc_fd_list.head, fd_info, next)
		{
			if(v_fd == item->v_fd){
				*p_fd_info =(fd_info *)calloc(1, sizeof(fd_info));
				if(*p_fd_info != NULL){
					(*p_fd_info)->path = (char *)calloc(1, strlen(item->path)+1);
					if((*p_fd_info)->path != NULL){
						(*p_fd_info)->v_fd = item->v_fd;
						(*p_fd_info)->type = item->type;
						(*p_fd_info)->offset = item->offset;
						memcpy((*p_fd_info)->path, item->path, strlen(item->path));
						if(item->type == fd_type_open){
							(*p_fd_info)->socket_fd = item->socket_fd;
							(*p_fd_info)->file_fd = item->file_fd;
							(*p_fd_info)->file_len = item->file_len;
							(*p_fd_info)->mode = item->mode;
							(*p_fd_info)->flag = item->flag;
						}
						else if(item->type == fd_type_fopen){
							(*p_fd_info)->socket_fd = item->socket_fd;
							(*p_fd_info)->file_fd = item->file_fd;
							(*p_fd_info)->file_len = item->file_len;
							memcpy((*p_fd_info)->f_mode, item->f_mode, strlen(item->f_mode));
						}
						else if(item->type == fd_type_opendir){
							(*p_fd_info)->p_data = item->p_data;
							(*p_fd_info)->file_count = item->file_count;
						}
						ret = 0;
						break;	
					}
				}
			}
		}
		pthread_mutex_unlock(&ppc_fd_list.mutex);
	}
	return ret;
}

int free_ppc_fd_list()
{
	if(ppc_fd_list.has_init){
		fd_info *item, *n;
		pthread_mutex_lock(&ppc_fd_list.mutex);
		dl_list_for_each_safe(item, n, &ppc_fd_list.head, fd_info, next)
		{
			dl_list_del(&item->next);
			free_fd_info(&item);
		}
		pthread_mutex_unlock(&ppc_fd_list.mutex);
		pthread_mutex_destroy(&ppc_fd_list.mutex);
	}
	else{
		DMCLOG_E("no need free");
		return -1;
	}
	
	return 0;
}

