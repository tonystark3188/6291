//
//  dm_mqtt_callback.c
//  DM MQTT CALLBACK
//
//  Created by Oliver on 16/6/30.
//  Copyright © 2016年 Oliver. All rights reserved.
//
#include "dm_mqtt_callback.h"
#include "my_debug.h"


static t_sub_callback *t_sub_callbacks=NULL;
static t_pub_callback *t_pub_callbacks=NULL;



/**
 *  功能：根据topic向订阅的指针添加回调函数
 *  权限：开放权限
 *
 *  @param topic:订阅和消费的主题，qos:服务质量level,callback:订阅当前事件的回调函数
 *
 *  @return 0为成功，非0为异常。
 */
int add_sub_callback(char* topic, int qos, sub_callback *callback)
{
    //SUB CALLBACK LOCK
	t_sub_callback **_t_sub_callbacks=&t_sub_callbacks;
	while(*_t_sub_callbacks)
	{
		if (strcmp((*_t_sub_callbacks)->topic, topic) == 0)
		{
			(*_t_sub_callbacks)->qos = qos;
			(*_t_sub_callbacks)->callback = callback;
            //SUB CALLBACK UNLOCK
			return 0;
		}
		_t_sub_callbacks = &(*_t_sub_callbacks)->next;
	}
	if ((*_t_sub_callbacks = (t_sub_callback*)calloc(1,sizeof(t_sub_callback))) == NULL)
	{
        //SUB CALLBACK UNLOCK
		return -1;
	}
	memset(*_t_sub_callbacks,0,sizeof(t_sub_callback));
	if (((*_t_sub_callbacks)->topic = (char*)calloc(1,strlen(topic) + 1)) == NULL)
	{
		free(*_t_sub_callbacks);
        //SUB CALLBACK UNLOCK
		return -1;
	}
	strcpy((*_t_sub_callbacks)->topic, topic);
	(*_t_sub_callbacks)->qos = qos;
	(*_t_sub_callbacks)->callback = callback;
	(*_t_sub_callbacks)->next = NULL;
    //SUB CALLBACK UNLOCK
	return 0;
}

/**
 *  功能:释放订阅的回调函数相关的资源
 *  权限：开放权限
 *
 *  @param
 *
 *  @return 0为成功，非0为为完成任务。
 */
int destory_sub_callback()
{
    //SUB CALLBACK LOCK
    t_sub_callback **_t_sub_callbacks=&t_sub_callbacks;
    while(*_t_sub_callbacks)
    {
        t_sub_callback *next_node = (*_t_sub_callbacks)->next;
        DMCLOG_D("rm topic(%s) sub_callbacks\r\n",(*_t_sub_callbacks)->topic);
        free((*_t_sub_callbacks)->topic);
        free(*_t_sub_callbacks);
        *_t_sub_callbacks = next_node;
    }
    //SUB CALLBACK UNLOCK
    return 0;
}
/**
 *  功能：根据topic删除订阅的触发事件
 *  权限：开放权限
 *
 *  @param topic:订阅和消费的主题
 *
 *  @return 0为成功，非0为异常。
 */
int rm_sub_callback(char* topic)
{
    //SUB CALLBACK LOCK
	t_sub_callback **_t_sub_callbacks=&t_sub_callbacks;
	while(*_t_sub_callbacks)
	{
		if (strcmp((*_t_sub_callbacks)->topic, topic) == 0) 
		{
			t_sub_callback *next_node = (*_t_sub_callbacks)->next;
			free((*_t_sub_callbacks)->topic);
			free(*_t_sub_callbacks);
			*_t_sub_callbacks = next_node;
			break;
		}
		_t_sub_callbacks = &(*_t_sub_callbacks)->next;
	}
    //SUB CALLBACK UNLOCK
    return 0;
}
/**
 *  功能：给发布事件的回调函数赋值
 *  权限：开放权限
 *
 *  @param topic:订阅和消费的主题，qos:服务质量level,callback:发布事件的回调函数
 *
 *  @return 0为成功，非0为异常。
 */
int add_pub_callback(int dt, pub_callback *callback)
{
    //PUB CALLBACK LOCK
	t_pub_callback **_t_pub_callbacks=&t_pub_callbacks;
	while(*_t_pub_callbacks)
	{
		_t_pub_callbacks = &(*_t_pub_callbacks)->next;
	}
	if ((*_t_pub_callbacks = (t_pub_callback*)calloc(1,sizeof(t_pub_callback))) == NULL)
	{
        //PUB CALLBACK UNLOCK
		return -1;
	}
	(*_t_pub_callbacks)->dt = dt;
	(*_t_pub_callbacks)->callback = callback;
	(*_t_pub_callbacks)->next = NULL;
    //PUB CALLBACK UNLOCK
	return 0;
}

/**
 *  功能:释放发布的回调函数相关的资源
 *  权限：开放权限
 *
 *  @param
 *
 *  @return 0为成功，非0为为完成任务。
 */
int destory_pub_callback()
{
    //PUB CALLBACK LOCK
    t_pub_callback **_t_pub_callbacks=&t_pub_callbacks;
    while(*_t_pub_callbacks)
    {
        t_pub_callback *next_node = (*_t_pub_callbacks)->next;
        free(*_t_pub_callbacks);
        *_t_pub_callbacks = next_node;
    }
    //PUB CALLBACK UNLOCK
    DMCLOG_D("rm all pub_callbacks end\r\n");
    return 0;
}

/**
 *  功能：根据token判定订阅的任务是否已完成
 *  权限：开放权限
 *
 *  @param topic:订阅和消费的主题，qos:服务质量level,callback:发布事件的回调函数
 *
 *  @return 0为成功，非0为为完成任务。
 */
int pub_qos_match(int token)
{
    //PUB CALLBACK LOCK
    t_pub_callback **_t_pub_callbacks=&t_pub_callbacks;
    while(*_t_pub_callbacks)
    {
        if ((*_t_pub_callbacks)->dt == token)
        {
            break;
        }
        _t_pub_callbacks = &(*_t_pub_callbacks)->next;
    }
    
    if(*_t_pub_callbacks == NULL)
    {
        //msg push success
        //PUB CALLBACK UNLOCK
        return 0;
    }
    else
    {
        //PUB CALLBACK UNLOCK
        return -1;
    }
}
