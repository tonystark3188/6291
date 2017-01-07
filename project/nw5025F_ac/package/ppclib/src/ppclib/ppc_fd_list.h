#ifndef _PPC_LIST_H_
#define _PPC_LIST_H_

#ifdef __cplusplus
extern "C"{
#endif
#include "base.h"
#include "list.h"

typedef struct _dl_ppc_fd_list
{
	struct dl_list head;
	int has_init;
	pthread_mutex_t mutex;
}dl_ppc_fd_list;

enum{
	fd_type_open = 1,
	fd_type_fopen = 2,
	fd_type_opendir = 3,
}fd_type;

typedef struct _fd_info
{
	int v_fd;		//虚拟的fd
	int type;		//fd类型
	int socket_fd;	//socket的fd
	int file_fd;	//真实的文件fd
	char *path;		//文件路径
	off_t offset;	//文件偏移
	off_t file_len; //文件长度
	int 	flag;	//打开文件的标志
	mode_t 	mode;	//打开文件的模式
	char	f_mode[8];//fopen的mode
	struct dirent **p_data;
	int 	file_count;
	struct dl_list next;
}fd_info;

int init_ppc_fd_list();
int add_info_for_ppc_fd_list(fd_info *p_fd_info);
int del_info_for_ppc_fd_list(int v_fd);
int inc_offset_for_ppc_fd_list(int v_fd, off_t offset);
int set_offset_for_ppc_fd_list(int v_fd, off_t offset);
int get_socket_fd_from_ppc_fd_list(int v_fd);
int ch_socket_fd_for_ppc_fd_list(int v_fd, int socket_fd);
int get_info_from_ppc_fd_list(int v_fd, fd_info **p_info);
int free_ppc_fd_list();

#ifdef __cplusplus
}
#endif

#endif
