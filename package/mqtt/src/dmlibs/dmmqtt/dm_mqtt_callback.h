//
//  dm_mqtt_callback.h
//  DM_MQTT_CALLBCK
//
//  Created by Oiver on 16/6/30.
//  Copyright © 2016年 Oliver. All rights reserved.
//

#ifndef dm_mmqt_callback_h
#define dm_mmqt_callback_h

#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "my_debug.h"

typedef struct
{
    void* payload;
    int payloadlen;
    char *topic_name;
	void *context;
}message_callback_handle_msg_t;

typedef int sub_callback(char *recv_buf,char **send_buf,char **fromId);
typedef void pub_callback(int dt);

typedef struct t_sub_callback {
    char*                   topic;
    int                     qos;
    sub_callback*           callback;
    struct t_sub_callback*  next;
} t_sub_callback;


typedef struct t_pub_callback {
    int                       dt;
    pub_callback*             callback;
    struct t_pub_callback*    next;
} t_pub_callback;

/**
 *  功能：根据topic向订阅的指针添加回调函数
 *  权限：开放权限
 *
 *  @param topic:订阅和消费的主题，qos:服务质量level,callback:订阅当前事件的回调函数
 *
 *  @return 0为成功，非0为异常。
 */
int add_sub_callback(char* topic, int qos, sub_callback *callback);
/**
 *  功能:释放订阅的回调函数相关的资源
 *  权限：开放权限
 *
 *  @param
 *
 *  @return 0为成功，非0为为完成任务。
 */
int destory_sub_callback();
/**
 *  功能：根据topic删除订阅的触发事件
 *  权限：开放权限
 *
 *  @param topic:订阅和消费的主题
 *
 *  @return 0为成功，非0为异常。
 */
int rm_sub_callback(char* topic);

int add_pub_callback(int dt, pub_callback *callback);

/**
 *  功能:释放发布的回调函数相关的资源
 *  权限：开放权限
 *
 *  @param
 *
 *  @return 0为成功，非0为为完成任务。
 */
int destory_pub_callback();
/**
 *  功能：根据token判定订阅的任务是否已完成
 *  权限：开放权限
 *
 *  @param topic:订阅和消费的主题，qos:服务质量level,callback:发布事件的回调函数
 *
 *  @return 0为成功，非0为为完成任务。
 */
int pub_qos_match(int token);
#endif /* dm_mqtt_parser_h */
