//
//  dm_mqtt.c
//  DM MQTT
//
//  Created by Oliver on 16/7/1.
//  Copyright © 2016年 Oliver. All rights reserved.
//
#include "dm_download_cloud_mqtt.h"
/**
 *  功能：释放mqtt客户端所有资源
 *  权限：开放权限
 *
 *  @param
 *
 *  @return 0为成功，非0为为完成任务。
 */
int dm_mqtt_destroy(void)
{
    _dm_mqtt_unsub_topic();
    
    _dm_mqtt_disconnect();
    
    _dm_mqtt_destory();
    
    return 0;
}
/**
 *  功能：启动mqtt客户端服务，暂时不可重入
 *  权限：开放权限
 *
 *  @param
 *
 *  @return 0为成功，非0为为完成任务。
 */
int dm_mqtt_start(void)
{
	ENTER_FUNC();
	return _dm_mqtt_start();
}

/**
 *  功能：根据cmd命令处理任务
 *  权限：开放权限
 *
 *  @param cmd:输入, cmd = 1:添加任务，2:控制所有任务，3:获取目录，4:获取任务列表；
 *         fromId:设备标识符
 *         device_id:目的设备ID
 *  @return 0为成功，非0为异常。
 */
int dm_mqtt_process_task(int cmd,int qos,char *device_id,void *clientInfo)
{
    int ret = dm_mqtt_send_message(cmd,qos,device_id,clientInfo);
    if(ret < 0)
    {
        DMCLOG_E("mqtt conbin client request error");
        return -1;
    }
    return 0;
}