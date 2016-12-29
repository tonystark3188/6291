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
int handle_login_task(char *username,char *password, _int64_t *token);
    
//登出操作
int handle_logout_task(_int64_t token);

int handle_fopen_task(void* fp);

size_t handle_fread_task(int fd,size_t length,char *buf);

/*
 * 按照offset,length上传文件
 */
size_t handle_fwrite_task(int fd,size_t length,const char *buf);
    
void handle_fclose_task(void *arg);
int handle_opendir_task(const char *path, struct dirent ***p_data, _int64_t token);
//void handle_rewinddir_task(PPC_DIR *p_dir);
struct dirent *handle_readdir_task(struct dirent **p_data, off_t offset);
//void handle_seekdir_task(PPC_DIR *p_dir, off_t loc);
//off_t handle_telldir_task(PPC_DIR *p_dir);
int handle_mkdir_task(const char *dirname, mode_t mode, _int64_t token);
int handle_rmdir_task(const char *pathname, _int64_t token);
//int handle_closedir_task(PPC_DIR *p_dir);
int handle_stat_task(const char* src_path,struct stat *st, _int64_t token);
int handle_open_task(void* arg);
size_t handle_read_task(int fd,size_t length,char *buf);
size_t handle_write_task(int fd,size_t length,const char *buf);
int handle_rename_task(const char *oldname, const char *newname, _int64_t token);
int handle_unlink_task(const char *pathname, _int64_t token);
int handle_utimensat_task(const char *pathname, const struct timespec times[2], int flags, _int64_t token);
int handle_ftruncate_task(const char *pathname, off_t length, _int64_t token);
int handle_symlink_task(const char *oldname, const char *newname, _int64_t token);
int handle_link_task(const char *oldname, const char *newname, _int64_t token);
int handle_readlink_task(const char *pathname, char **link_buf, _int64_t token);
int handle_statvfs_task(const char *pathname, struct statvfs *buf, _int64_t token);

#ifdef __cplusplus
}
#endif


#endif

