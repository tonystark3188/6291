/*
 * =============================================================================
 *
 *       Filename:  get_file_list.h
 *
 *    Description:  handle get file list cmd.
 *
 *        Version:  1.0
 *        Created:  2015/10/29 17:55:17
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver ()
 *   Organization:  
 *
 * =============================================================================
 */
#ifndef _GET_FILE_LIST_CMD_H_
#define _GET_FILE_LIST_CMD_H_

#include "base.h"
#ifdef __cplusplus
extern "C"{
#endif
int handle_get_file_size_cmd(int file_type,unsigned long offset,unsigned long length,int sort_mode,JObj *response_para_json,char *uuid);
int handle_get_file_list_count_cmd(int  file_type,unsigned int *type_count,char *uuid);
int handle_get_file_list_count_by_path_cmd(int  file_type,unsigned int *type_count,char *path,char *uuid);
//int handle_get_file_list_cmd(int file_type,unsigned long offset,unsigned long length,int sort_mode,JObj *response_para_json,char *uuid);
//int _handle_get_file_list_cmd(int cmd,int file_type,unsigned long offset,unsigned long length,int sort_mode,char **buf,char *uuid,unsigned *nfiles,char *path);
//int	_handle_get_file_list_cmd(int sort_mode,char **buf,size_t *n,struct conn *c);

int handle_get_dir_list_cmd(int file_type,unsigned long offset,unsigned long length,int sort_mode,JObj *response_para_json,char *uuid);


int handle_file_rename_cmd(char *src_path,char *des_path,char *uuid);
int handle_file_hide_cmd(char *src_path,bool attr,char *uuid,bool album);

int handle_file_insert_cmd(char *file_uuid,char *src_path,char bIsRegularFile,char *uuid);

int handle_file_delete_cmd(char *src_path,char *uuid);
int handle_get_disk_info_cmd(char *uuid,disk_info_t *disk_pfi);
int handle_insert_disk_table(disk_info_t *disk_info);
int handle_update_disk_table(disk_info_t *disk_info);

int handle_db_login(char *session,char *deviceUuid,char *deviceName,char *ip,char *username,char *password);
int handle_db_logout(char *session);
int handle_db_del_usr_for_ip(const char *client_ip);

int get_ip_from_usr_table(_In_ const char *session,_Out_ char *ip);
int get_devuuid_from_usr_table(_In_ const char *session,_Out_ char *device_uuid,_Out_ char *device_name);
int bind_devuuid_to_diskuuid(_In_ const char *device_uuid,_In_ const char *device_name,_In_ const char *disk_uuid);
int get_bind_disk_uuid_from_db(_In_ const char *device_uuid,_Out_ char *disk_uuid);
///*判断上传的文件UUID是否存在于数据库中*/
//int handle_file_uuid_exist_cmd(char *file_uuid,char *device_uuid,char *uuid);

int handle_query_file_path_by_uuid(char *file_uuid,char *device_uuid,char *uuid,char **file_path);

int handle_file_uuid_exist_cmd(char *file_uuid,char *device_uuid,char *uuid,int *file_type);


/*将备份的文件信息存入数据库*/
//int handle_backfile_insert_cmd(char *file_uuid,char *device_uuid,char *path,char *disk_uuid);

//int handle_backfile_insert_cmd(struct conn *c,char file_status);

//int handle_backfile_update_cmd(char *file_uuid,char *device_uuid,char file_status,char *uuid);

int handle_file_list_uuid_exist_cmd(file_uuid_list_t *flist,JObj*response_para_json,char *disk_uuid);
//int db_backup_file_rm(const char *path,const char *disk_uuid);
//int handle_backfile_special_delete_cmd(file_uuid_list_t *flist,char *uuid);
//int handle_query_file_by_id_cmd(unsigned id,char *path,struct disk_node *disk_info);



#ifdef __cplusplus
}
#endif

#endif
