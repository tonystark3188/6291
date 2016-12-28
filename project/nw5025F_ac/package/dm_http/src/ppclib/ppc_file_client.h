/*
 * =============================================================================
 *
 *       Filename:  hidisk_file_client.h
 *
 *    Description:  tcp client for Hidisk
 *
 *        Version:  1.0
 *        Created:  2016/10/17 13:05:49
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver
 *   Organization:  
 *
 * =============================================================================
 */
#ifndef _HIDISK_FILE_CLIENT_H_
#define _HIDISK_FILE_CLIENT_H_
#include <sys/types.h>

#ifdef __cplusplus
extern "C"{
#endif
#include "base.h"
//注册操作
int handle_register_task(char *username,char *password);

//登陆操作
int handle_login_task(char *username,char *password,char **token);
    
//登出操作
int handle_logout_task(char *token);

int handle_open_task(void* fp);

size_t handle_read_task(void* fp,size_t length,char *buf);

/*
 * 按照offset,length上传文件
 */
size_t handle_write_task(void* fp,size_t length,const char *buf);
    
void handle_close_task(void *arg);
int handle_opendir_task(const char *path,PPC_DIR *p_dir,char *token);
struct ppc_dirent *handle_readdir_task(PPC_DIR *p_dir);
struct ppc_dirent *handle_seekdir_task(PPC_DIR *p_dir, long loc);
struct ppc_dirent *handle_telldir_task(PPC_DIR *p_dir);
int handle_closedir_task(PPC_DIR *p_dir);
int handle_stat_task(const char* src_path,struct ppc_stat *st,char *token);


#ifdef __cplusplus
}
#endif


#endif

