//
//  dm_mqtt_parser.h
//  DM_MQTT_PAESER
//
//  Created by Oiver on 16/6/28.
//  Copyright © 2016年 Oliver. All rights reserved.
//

#ifndef dm_mmqt_parser_h
#define dm_mmqt_parser_h

#include <stdio.h>
#include "my_json.h"
#include "dm_mqtt_process.h"


#define FROM_ID_LEN  64
#define MSG_TYPE_LEN 32

#define AIRDISK_FROM_TYPE   0

/************************************************************************************
 *                          ENUM                                                    *
 ***********************************************************************************/



/************************************************************************************
 *                          STRUCT                                                    *
 ***********************************************************************************/

typedef struct{
    
}MusicTaskInfo;



typedef struct{
    MusicTaskInfo    *clientMusicInfo;
    DownloadTaskInfo *clientDownloadInfo;
}MqttClientServices;

typedef struct{
    int     error_code;//0或非零
    char    *error_msg;//错误码的文本信息
    char    *recv_buf;//从mmqt服务端接收到的数据
    char    *send_buf;//后台处理后发往服务器的数据
    char    msgType[MSG_TYPE_LEN];//get or set
    char    fromId[FROM_ID_LEN];
    int     fromType;//0:设备，1:应用，2:服务器
    long    msgData;//发送方生成时间
    unsigned long long    msgId;//消息消费方
    int     serviceFlag;//dmmsg_music|dmmsg_download
    int     operation_status;//0:上线，1:待机，2:关机，3:网络异常
    MqttClientServices *clientService;
}MqttClientInfo;
/**
 *  功能：解析接收到的数据，将结果置于结构体中
 *  权限：开放权限
 *
 *  @param clientInfo->recv_buf:从mmqt服务端接收到的数据，clientInfo:将结果封装在指针指向的结构体对象空间
 *
 *  @return 0为成功，非0为异常。
 */
int mqtt_parser_client_request(MqttClientInfo *clientInfo);

/**
 *  功能: 释放任务列表的资源
 *  权限：开放权限
 *
 *  @param phead:下载任务列表的指针
 *
 *  @return 0为成功，非0为异常。
 */
void free_task_list(struct dl_list *phead);
/**
 *  功能：释放目录列表的资源
 *  权限：开放权限
 *
 *  @param phead:目录列表的指针
 *
 *  @return 0为成功，非0为异常。
 */
void free_dir_list(struct dl_list *phead);


/**
 *  功能：根据相关的请求进行处理
 *  权限：开放权限
 *
 *  @param clientInfo:将结果封装在指针指向的结构体对象空间
 *
 *  @return 0为成功，非0为异常。
 */
int mqtt_handle_client_request(MqttClientInfo *clientInfo);

/**
 *  功能：将处理后的数据封装成JSON数据字符串格式进行转发
 *  权限：开放权限
 *
 *  @param clientInfo:将结果封装在指针指向的结构体对象空间,clientInfo->send_buf:后台处理后发往服务器的数据
 *
 *  @return 0为成功，非0为异常。
 */
int mqtt_conbin_client_response(MqttClientInfo *clientInfo);

/**
 *  功能：封装上报(notify)的字符串
 *  权限：开放权限
 *
 *  @param clientInfo:将结果封装在指针指向的结构体对象空间,clientInfo->send_buf:后台处理后发往服务器的数据
 *
 *  @return 0为成功，非0为异常。
 */
int mqtt_conbin_client_notify(MqttClientInfo *clientInfo);

/**
 *  功能：根据请求任务封装json数据 cmd = 1:添加任务，2:控制所有任务，3:获取目录，4:获取任务列表
 *  权限：开放权限
 *
 *  @param clientInfo:将结果封装在指针指向的结构体对象空间,clientInfo->send_buf:后台处理后发往服务器的数据
 *
 *  @return 0为成功，非0为异常。
 */
int mqtt_conbin_client_request(int cmd,MqttClientInfo *clientInfo);


#endif /* dm_mqtt_parser_h */
