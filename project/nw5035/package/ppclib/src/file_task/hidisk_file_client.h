/*
 * =============================================================================
 *
 *       Filename:  hidisk_tcp_client.h
 *
 *    Description:  tcp client for Hidisk
 *
 *        Version:  1.0
 *        Created:  2014/9/28 13:05:49
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver
 *   Organization:  
 *
 * =============================================================================
 */
#ifndef _HIDISK_TCP_CLIENT_H_
#define _HIDISK_TCP_CLIENT_H_


#ifdef __cplusplus
extern "C"{
#endif
#include "file_process.h"
#define DM_DOWN_PERSIZE        16384
int dm_check_prcs_task(ClientTheadInfo *p_client_info);
int dm_fopen_prcs_task(ClientTheadInfo *p_client_info);
/*
 * 获取文件列表
 */
int dm_file_list_prcs_task(ClientTheadInfo *p_client_info);

int dm_open_prcs_task(ClientTheadInfo *p_client_info);


#ifdef __cplusplus
}
#endif


#endif

