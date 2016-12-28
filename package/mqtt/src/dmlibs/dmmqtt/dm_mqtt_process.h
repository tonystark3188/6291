//
//  dm_mqtt_process.h
//  DM_MQTT_PROCESS
//
//  Created by Oiver on 16/6/28.
//  Copyright © 2016年 Oliver. All rights reserved.
//

#ifndef dm_mmqt_process_h
#define dm_mmqt_process_h

#include <stdio.h>
#include "my_json.h"
//#include "list.h"
#include "dmdownload.h"

/************************************************************************************
 *                          ENUM                                                    *
 ***********************************************************************************/

typedef enum {
    MQTT_SUCCES = 0,//操作成功
    MQTT_PARA_ERROR = 10000,//参数错误
    MQTT_TIMEOUT = 10001,//任务超时
    MQTT_ADD_TASK_ERROR = 10002,//添加任务失败
    MQTT_QUERY_TASK_ERROR = 10003,//获取任务列表失败
    MQTT_QUERY_DIR_ERROR = 10004,//获取任务目录失败
    MQTT_QUERY_GLOBAL_ERROR = 10005,//获取全局状况失败
} MQTT_RESULT;

/************************************************************************************
 *                          STRUCT                                                    *
 ***********************************************************************************/

/**
 *  功能：按照第一个参数获取下载任务列表
 *  权限：开放权限
 *
 *  @param status:0 :下载中的任务，1:暂停中的任务，2:等待中的任务，3:已完成的任务，4下载失败的任务，s_json:将结果封装成JSON数据输出
 *
 *  @return 0为成功，非0为异常。
 */
MQTT_RESULT mqtt_get_task_list_process(DownloadTaskInfo *dTaskInfo,JObj *s_json);

/**
 *  功能：获取下载目录列表
 *  权限：开放权限
 *
 *  @param s_json:将结果封装成JSON数据输出
 *
 *  @return 0为成功，非0为异常。
 */
MQTT_RESULT mqtt_get_task_dir_process(DownloadTaskInfo *dTaskInfo,JObj *s_json);

/**
 *  功能：获取下载模块的整体情况，包括下载速度，上传速度
 *  权限：开放权限
 *
 *  @param s_json:将结果封装成JSON数据输出
 *
 *  @return 0为成功，非0为异常。
 */
MQTT_RESULT mqtt_get_global_status_process(DownloadTaskInfo *dTaskInfo,JObj *s_json);

/**
 *  功能：添加下载任务
 *  权限：开放权限
 *
 *  @param dTaskInfo->head:需要添加的任务链表
 *
 *  @return 0为成功，非0为异常。
 */
MQTT_RESULT mqtt_set_add_task_process(DownloadTaskInfo *dTaskInfo);

/**
 *  功能：控制下载任务
 *  权限：开放权限
 *
 *  @param dTaskInfo->head:需要添加的任务链表
 *
 *  @return 0为成功，非0为异常。
 */
MQTT_RESULT mqtt_set_ctr_task_process(DownloadTaskInfo *dTaskInfo);


#endif /* dm_mqtt_process_h */
