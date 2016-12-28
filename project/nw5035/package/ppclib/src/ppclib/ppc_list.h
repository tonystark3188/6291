#ifndef _PPC_LIST_H_
#define _PPC_LIST_H_

#ifdef __cplusplus
extern "C"{
#endif
#include "list.h"

typedef struct _dl_ppc_fd_list
{
	struct dl_list head;
	pthread_mutex_t mutex;
}dl_ppc_fd_list;


typedef struct _fd_info
{
	int fd;
	int socket_fd;
	char *path;
	struct dl_list next;
}fd_info;

int init_ppc_fd_list();
int add_info_for_ppc_fd_list(fd_info *p_fd_info);
int del_info_for_ppc_fd_list(int fd);
int get_info_from_ppc_fd_list(int fd, fd_info **p_info);
int free_ppc_fd_list();

#endif
