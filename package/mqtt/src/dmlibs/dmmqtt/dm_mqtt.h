#ifndef _DM_MQTT_H
#define _DM_MQTT_H

#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
//func
/**
 *  功能：启动mqtt客户端服务，不可重入(暂时)
 *  权限：开放权限
 *
 *  @param
 *
 *  @return 0为成功，非0为为完成任务。
 */
extern int dm_mqtt_start(void);

/**
 *  功能：释放mqtt客户端所有资源
 *  权限：开放权限
 *
 *  @param
 *
 *  @return 0为成功，非0为为完成任务。
 */
extern int dm_mqtt_destroy(void);

/**
 *  功能：根据cmd命令处理任务
 *  权限：开放权限
 *
 *  @param cmd:输入, cmd = 1:添加任务，2:控制所有任务，3:获取目录，4:获取任务列表；
 *         fromId:设备标识符 填充到结构体
 *         fromType:设备类型 填充到结构体
 *         qos:传输质量等级，默认为1
 *         deviceId:目标设备ID
 *  @return 0为成功，非0为异常。
 */
int dm_mqtt_process_task(int cmd,int qos,char *deviceId,void *clientInfo);
#endif

