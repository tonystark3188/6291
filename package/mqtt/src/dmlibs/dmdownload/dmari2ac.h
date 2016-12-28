//
//  dmari2ac.h
//  DMARI2AC
//
//  Created by Oiver on 16/6/27.
//  Copyright © 2016年 Oliver. All rights reserved.
//

#ifndef dmari2ac_h
#define dmari2ac_h

#include <stdio.h>
#include "dmdownload.h"

/**
 *  功能：添加下载任务
 *  权限：开放权限
 *
 *  @param url:输入，资源地址；task_id:输出，任务唯一标识符
 *
 *  @return 0为成功，非0为异常。
 */
DMDOWNLOAD_RESULT ari2ac_add_task(char *url,char *task_id);

/**
 *  功能：控制下载任务
 *  权限：开放权限
 *
 *  @param ctrl 输入 0 :下载任务，1:暂停，2:等待，3:删除
 *         task_id 输入 任务唯一标识符
 *
 *  @return 0为成功，非0为异常。
 */
DMDOWNLOAD_RESULT ari2ac_ctr_task(int ctrl,char *task_id);

/**
 *  功能：控制所有下载任务
 *  权限：开放权限
 *
 *  @param ctrl 输入 0 :下载任务，1:暂停，2:等待，3:删除
 *
 *  @return 0为成功，非0为异常。
 */
DMDOWNLOAD_RESULT ari2ac_ctr_all_task(int ctrl);

/**
 *  功能：根据状态获取下载任务列表
 *  权限：开放权限
 *
 *  @param status 0 :下载中的任务，1:暂停中的任务，2:等待中的任务，3:已完成的任务，4下载失败的任务
 *          head 链表首指针
 *  @return 0为成功，非0为异常。
 */
//DMDOWNLOAD_RESULT ari2ac_query_task(int status,struct dl_list *head);

DMDOWNLOAD_RESULT ari2ac_query_task_by_status(int status,struct dl_list *head);


/**
 *  功能：根据id获取下载任务详情
 *  权限：开放权限
 *
 *  @param gid : 任务id
 *          head 链表首指针
 *  @return 0为成功，非0为异常。
 */
DMDOWNLOAD_RESULT ari2ac_query_task_by_gid(char *gid,struct dl_list *head);


/**
 *  功能：获取下载目录列表
 *  权限：开放权限
 *
 *  @param head 链表首指针
 *
 *  @return 0为成功，非0为异常。
 */
DMDOWNLOAD_RESULT ari2ac_query_dir(struct dl_list *head);
/**
 *  功能：获取下载模块全局状况
 *  权限：开放权限
 *
 *  @param dGlobalStatus 输出
 *
 *  @return 0为成功，非0为异常。
 */
DMDOWNLOAD_RESULT ari2ac_query_global(DownloadGlobalStatus *dGlobalStatus);

/**
 * func:contrl global speed follow by para
 *
 *  @param dGlobalStatus->download_speed >= 0:set download speed,dGlobalStatus->download_speed < 0:no need to set download speed
 *			dGlobalStatus->upload_speed >= 0:set upload speed,dGlobalStatus->uplload_speed < 0:no need to set upload speed
 *  @return 0为成功，非0为异常。
 */
DMDOWNLOAD_RESULT ari2ac_ctr_speed(DownloadGlobalStatus *dGlobalStatus);

#endif /* dmari2ac_h */
