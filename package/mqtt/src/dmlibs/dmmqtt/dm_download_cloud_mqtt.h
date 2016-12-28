#ifndef _DM_DOWNLOAD_CLOUD_WX_MQTT_H
#define _DM_DOWNLOAD_CLOUD_WX_MQTT_H

#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "MQTTAsync.h"
#include "MQTTClientPersistence.h"
#include "dm_mqtt_callback.h"
#include "my_debug.h"
#include "dm_mqtt_parser.h"

typedef enum{
    STATUS_RUNNING = 0,//上线
    WAITING_STATUS = 1,//待机
    DOWNLOAD_STATUS = 2,//关机
    NET_INVALID_STATUS = 3,//网络异常
}device_status;


//define
#define   WX_MQTT_USERNAME     			  			"dmrd_airdisk"
#define   WX_MQTT_PASSWORD				 			"airdisk8372!"
#define   WX_MQTT_SERVER_URL				 		"q.dmsys.com"
#define   WX_MQTT_SERVER_PORT                       "61613"//"51620","61613"
#define   WX_MQTT_CLIENTID_PREFIX		 			"DMremotedownload"

#define   FW_CLIENT_DEV
#ifdef    FW_CLIENT_DEV
#define   WX_MQTT_DEVICE_ID                         "222222222" //后期通过cfg接口获取设备唯一标识符,FW 设备ID
#else
#define   WX_MQTT_DEVICE_ID                         "987654321" //后期通过cfg接口获取设备唯一标识符,APP设备ID
#endif

#define   WX_MQTT_DEVICE           		 			 (0)
#define   WX_MQTT_QOS           		 			 (1)
#define   WX_MQTT_RETAIN         					 (0)
#define   WX_MQTT_PUB_DEFAULT_TIMEOUT        		 (10000)

char mqtt_device_id[32];

//define topic
#define COMSUMER_TOPIC_PREFIX    "dmrd_airdisk/p2p/consumerId@@@"
#define NOTIFY_PUBLISH_TOPIC     "dmrd_airdisk/deviceService"




//limit message_callback_thread
#define   MAX_MESSAGE_CALLBACK_HANDLE_THREAD    (2)


typedef enum {
    WX_MQTT_SUCCESS = 0,
    WX_MQTT_FAILURE = -1,
} rd_mqtt_return_e;




//func
extern rd_mqtt_return_e _dm_mqtt_start();

extern rd_mqtt_return_e _dm_mqtt_destory(void);

extern rd_mqtt_return_e _dm_mqtt_unsub_topic(void);

extern rd_mqtt_return_e _dm_mqtt_disconnect(void);

/**
 *  功能：向服务端发起请求并收到指定设备的答复
 *  权限：开放权限
 *
 *  @param cmd:任务命令，qos:发送质量等级，device_id:发送目的设备ID，clientInfo:请求数据
 *
 *  @return 0为成功，非0为异常。
 */
int dm_mqtt_send_message(int cmd,int qos,char *device_id,MqttClientInfo *clientInfo);
#endif

