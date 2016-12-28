//
//  dmdownload.c
//  DMDOWNLOAD
//
//  Created by Oliver on 16/6/27.
//  Copyright © 2016年 Oliver. All rights reserved.
//

#include "dm_mqtt_process.h"
#include "my_debug.h"
#include "dmdownload.h"
#include <string.h>
#include <stdlib.h>
/**
 *  功能：按照第一个参数获取下载任务列表
 *  权限：开放权限
 *
 *  @param dTaskInfo->status:0 :下载中的任务，1:暂停中的任务，2:等待中的任务，3:已完成的任务，4下载失败的任务，s_json:将结果封装成JSON数据输出
 *
 *  @return 0为成功，非0为异常。
 */
MQTT_RESULT mqtt_get_task_list_process(DownloadTaskInfo *dTaskInfo,JObj *s_json)
{
    int res = MQTT_SUCCES;
    dl_list_init(&dTaskInfo->t_head);
    res = download_query_task(dTaskInfo);
    if(res != 0)
    {
        return res;
    }
    return res;
}

/**
 *  功能：获取下载目录列表
 *  权限：开放权限
 *
 *  @param s_json:将结果封装成JSON数据输出
 *
 *  @return 0为成功，非0为异常。
 */
MQTT_RESULT mqtt_get_task_dir_process(DownloadTaskInfo *dTaskInfo,JObj *s_json)
{
    int res = MQTT_SUCCES;
    dl_list_init(&dTaskInfo->d_head);
    res = download_query_dir(dTaskInfo);
    if(res != 0)
    {
        return res;
    }
    return res;
}

/**
 *  功能：获取下载模块的整体情况，包括下载速度，上传速度
 *  权限：开放权限
 *
 *  @param s_json:将结果封装成JSON数据输出
 *
 *  @return 0为成功，非0为异常。
 */
MQTT_RESULT mqtt_get_global_status_process(DownloadTaskInfo *dTaskInfo,JObj *s_json)
{
    int res = MQTT_SUCCES;
    
    return res;
}

/**
 *  功能：添加下载任务
 *  权限：开放权限
 *
 *  @param dTaskInfo->head:需要添加的任务链表
 *
 *  @return 0为成功，非0为异常。
 */
MQTT_RESULT mqtt_set_add_task_process(DownloadTaskInfo *dTaskInfo)
{
    int res = MQTT_SUCCES;
    res = download_add_task(dTaskInfo);
    if(res != 0)
    {
        return res;
    }
    return res;
}

/**
 *  功能：控制下载任务
 *  权限：开放权限
 *
 *  @param dTaskInfo->head:需要添加的任务链表
 *
 *  @return 0为成功，非0为异常。
 */
MQTT_RESULT mqtt_set_ctr_task_process(DownloadTaskInfo *dTaskInfo)
{
    int res = MQTT_SUCCES;
    res = download_ctr_task(dTaskInfo);
    if(res != 0)
    {
        return res;
    }
    return res;
}