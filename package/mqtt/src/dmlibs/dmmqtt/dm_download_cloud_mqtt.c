#include "dm_download_cloud_mqtt.h"
#include "dm_mqtt_parser.h"
#include "dm_mqtt_callback.h"
#include "my_debug.h"
#include <assert.h>
#include <errno.h>


#if defined (OPENSSL) || defined (TLSSOCKET)
static MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
static char *uristring;
#endif

static MQTTAsync wx_mqtt_client;
static MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
static MQTTAsync_willOptions  will_opts = MQTTAsync_willOptions_initializer;

static int client_subscribed = 0;
static int disconnected = 0;
static int client_connected;


int dm_mqtt_connect(void *context);

void onDisconnect(void* context, MQTTAsync_successData* response)
{
    disconnected = 1;
}


rd_mqtt_return_e dm_mqtt_pubmsg(void *context,char* topic,MQTTAsync_message msg,int timeout)
{
	MQTTAsync c = (MQTTAsync)context;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	if(0 != MQTTAsync_sendMessage(c, topic, &msg, &opts))
	{
		DMCLOG_E("MQTTAsync_publishMessage fail,topic:%s\r\n",topic);
		return WX_MQTT_FAILURE;
	}
    return 0;
}


rd_mqtt_return_e dm_mqtt_pub_notify(void *context,device_status status,int timeout)
{
    rd_mqtt_return_e ret = WX_MQTT_SUCCESS;
    MqttClientInfo *clientInfo = (MqttClientInfo *)calloc(1,sizeof(MqttClientInfo));
    if(clientInfo == NULL)
    {
        DMCLOG_E("malloc memory error");
        ret = WX_MQTT_FAILURE;
        goto EXIT;
    }
    
    strcpy(clientInfo->fromId,mqtt_device_id);
    clientInfo->fromType = WX_MQTT_DEVICE;
    clientInfo->operation_status = STATUS_RUNNING;
    
    ret = mqtt_conbin_client_notify(clientInfo);
    if(ret != 0)
    {
        DMCLOG_E("mqtt conbin client notify error");
        ret = WX_MQTT_FAILURE;
        goto EXIT;
    }

    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    pubmsg.payload = clientInfo->send_buf;
    pubmsg.payloadlen = (int)strlen(clientInfo->send_buf);
    pubmsg.qos = WX_MQTT_QOS;
    pubmsg.retained = WX_MQTT_RETAIN;
    if(WX_MQTT_SUCCESS!= dm_mqtt_pubmsg(context, NOTIFY_PUBLISH_TOPIC,pubmsg,timeout))
    {
        DMCLOG_E("dm mqtt_pubmsg fail,msg:%s",clientInfo->send_buf);
        ret = WX_MQTT_FAILURE;
        goto EXIT;
    }
EXIT:
    if(clientInfo != NULL)
    {
        safe_free(clientInfo->send_buf);
        safe_free(clientInfo);
    }
    return ret;
}


void client_onSubscribe(void* context, MQTTAsync_successData* response)
{
    MQTTAsync c = (MQTTAsync)context;
    
    DMCLOG_D( "In client subscribe onSuccess callback %p granted qos %d", c, response->alt.qos);
    int notify_cnt;
	for(notify_cnt = 0; notify_cnt < 3; notify_cnt++)
	{
		if(WX_MQTT_FAILURE == dm_mqtt_pub_notify(context,STATUS_RUNNING,WX_MQTT_PUB_DEFAULT_TIMEOUT))
		{
            sleep(1);
			continue;
		}
		break;
	}
    client_subscribed = 1;
}

void client_onFailure(void* context, MQTTAsync_failureData* response)
{
    MQTTAsync c = (MQTTAsync)context;
    DMCLOG_D("In failure callback");
    
    client_subscribed = -1;
}

void client_onConnectFailure(void* context, MQTTAsync_failureData* response)
{
    MQTTAsync c = (MQTTAsync)context;
    DMCLOG_D("In failure connect callback errno = %d",errno);
    
    client_connected = -1;
    sleep(1);
    if(disconnected != 1)//mqtt´¦ÓÚ¹Ø±Õ×´Ì¬£¬Ôò²»ÔÚÖØÐÂ½øÐÐÁ¬½Ó
        dm_mqtt_connect(context);
}




void client_onConnect(void* context,MQTTAsync_successData *response)
{
    MQTTAsync c = (MQTTAsync)context;
    MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;
    int rc;
    int qos =1;
    char topic[256] = {0};
    sprintf(topic,"%s%s",COMSUMER_TOPIC_PREFIX,mqtt_device_id);
    DMCLOG_D("topic:%s\r\n",topic);
    ropts.context = context;
    ropts.onSuccess = client_onSubscribe;
    ropts.onFailure = client_onFailure;
    if((rc = MQTTAsync_subscribe(c,topic,qos,&ropts)) != MQTTASYNC_SUCCESS)
    {
        DMCLOG_E("client MQTTAsync_subscribe failed %d",rc);
        client_subscribed = -1;
    }
	client_connected = 1;
}

/**
 *  åŠŸèƒ½ï¼šå‘æœåŠ¡ç«¯å‘èµ·è¯·æ±‚å¹¶æ”¶åˆ°æŒ‡å®šè®¾å¤‡çš„ç­”å¤
 *  æƒé™ï¼šå¼€æ”¾æƒé™
 *
 *  @param cmd:ä»»åŠ¡å‘½ä»¤ï¼Œtimeout:è¶…æ—¶æ—¶é—´ï¼Œdevice_id:ç›®çš„è®¾å¤‡ID,clientInfo:è¯·æ±‚æ•°æ®
 *
 *  @return 0ä¸ºæˆåŠŸï¼Œéž0ä¸ºå¼‚å¸¸ã€‚
 */
int dm_mqtt_send_message(int cmd,int qos,char *device_id,MqttClientInfo *clientInfo)
{
    int ret = WX_MQTT_SUCCESS;
    ret = mqtt_conbin_client_request(cmd,clientInfo);
    if(ret != 0)
    {
        DMCLOG_E("mqtt conbin client request error");
        return -1;
    }
    
    char topic_name[128] = {0};
    sprintf(topic_name,"%s%s",WX_MQTT_CLIENTID_PREFIX,device_id);
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    pubmsg.payload = clientInfo->send_buf;
    pubmsg.payloadlen = (int)strlen(clientInfo->send_buf);
    pubmsg.qos = 1;
    pubmsg.retained = 0;
    MQTTAsync_sendMessage(wx_mqtt_client, topic_name, &pubmsg, &opts);
    return ret;
    
}


int dm_mqtt_connect(void *context)
{
    ENTER_FUNC();
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
    int rc = 0;
    MQTTAsync c = (MQTTAsync)context;
    DMCLOG_D("Connecting");
    conn_opts.keepAliveInterval = 10;
    conn_opts.cleansession = 1;
    conn_opts.username = WX_MQTT_USERNAME;
    conn_opts.password = WX_MQTT_PASSWORD;
    conn_opts.onSuccess = client_onConnect;
    conn_opts.onFailure = client_onConnectFailure;
    conn_opts.context = context;
    ssl_opts.enableServerCertAuth = 0;
    conn_opts.retryInterval = 1;
    conn_opts.ssl = &ssl_opts;
    
    if ((rc = MQTTAsync_connect(c, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start connect, return code %d\n", rc);
        return 0;
    }
    EXIT_FUNC();
    return -1;
}


void connectionLost(void* context, char* cause)
{
	dm_mqtt_connect(context);
	DMCLOG_D("client_connected = %d",client_connected);
}



void connectionLost_callback(void* context, char* cause)
{
	MQTTAsync c = (MQTTAsync)context;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
    int rc = 0;
    
    DMCLOG_D("Connecting");
    conn_opts.keepAliveInterval = 10;
    conn_opts.cleansession = 1;
    conn_opts.username = WX_MQTT_USERNAME;
    conn_opts.password = WX_MQTT_PASSWORD;
    conn_opts.onSuccess = client_onConnect;
    conn_opts.onFailure = client_onConnectFailure;
    conn_opts.context = context;
    ssl_opts.enableServerCertAuth = 0;
    conn_opts.retryInterval = 1;
    conn_opts.ssl = &ssl_opts;
    
    
    if ((rc = MQTTAsync_connect(c, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start connect, return code %d\n", rc);
        return;
    }
}

int topic_callback_handle(char *recv_buf,char **send_buf,char **fromId)
{
    int res = 0;
    ENTER_FUNC();
    if(recv_buf == NULL)
    {
        DMCLOG_E("para is null");
        res = -1;
        return res;
    }
    
    MqttClientInfo *mqttClientInfo = (MqttClientInfo *)calloc(1,sizeof(MqttClientInfo));
    
    
    mqttClientInfo->recv_buf = (char *)calloc(1,strlen(recv_buf) + 1);
    strcpy(mqttClientInfo->recv_buf,recv_buf);
    res = mqtt_parser_client_request(mqttClientInfo);
    if(res != 0)
    {
        DMCLOG_E("mqtt parser client request error");
        goto EXIT;
    }
    
    res = mqtt_handle_client_request(mqttClientInfo);
    if(res != 0)
    {
        DMCLOG_E("mqtt handle client request error");
		mqttClientInfo->error_code = res;
    }
	*fromId = (char *)calloc(1,strlen(mqttClientInfo->fromId) + 1);
    strcpy(*fromId,mqttClientInfo->fromId);
	
	DMCLOG_D("mqttClientInfo->fromId = %s",mqttClientInfo->fromId);
	strcpy(mqttClientInfo->fromId,mqtt_device_id);
    res = mqtt_conbin_client_response(mqttClientInfo);
    if(res != 0)
    {
        DMCLOG_E("mqtt handle client response error");
        goto EXIT;
    }
   
    *send_buf = (char *)calloc(1,strlen(mqttClientInfo->send_buf) + 1);
    strcpy(*send_buf,mqttClientInfo->send_buf);
	DMCLOG_D("sendbuf=%s",mqttClientInfo->send_buf);
	
    
    
    
EXIT :
    if(mqttClientInfo!= NULL)
    {
        if(mqttClientInfo->clientService != NULL)
        {
            if(mqttClientInfo->clientService->clientDownloadInfo != NULL)
            {
                free_task_list(&mqttClientInfo->clientService->clientDownloadInfo->t_head);
                free_dir_list(&mqttClientInfo->clientService->clientDownloadInfo->d_head);
            }
            
            safe_free(mqttClientInfo->clientService);
        }
        safe_free(mqttClientInfo->recv_buf);
        safe_free(mqttClientInfo->send_buf);
        safe_free(mqttClientInfo);
    }
    EXIT_FUNC();
    return res;
}

static void* message_callback_thread(void * data)
{
    ENTER_FUNC();
    int res = 0;
    char topic_name[128] = {0};
    char *response_str = NULL;
    char *fromId = NULL;
	message_callback_handle_msg_t *pmsg = (message_callback_handle_msg_t *)data;
	MQTTAsync c = (MQTTAsync)pmsg->context;
	//handle data
    res = topic_callback_handle(pmsg->payload,&response_str,&fromId);
    if(res == 0)
    {
        MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
        pubmsg.qos = 0;
        pubmsg.retained = WX_MQTT_RETAIN;
        strcpy(topic_name,COMSUMER_TOPIC_PREFIX);
        sprintf(topic_name,"%s%s",COMSUMER_TOPIC_PREFIX,fromId);
        DMCLOG_D("fromId = %s,response_str = %s",fromId,response_str);
        pubmsg.payload = response_str;
        pubmsg.payloadlen = (int)strlen(response_str); 
        if(WX_MQTT_SUCCESS != dm_mqtt_pubmsg(c,topic_name,pubmsg,WX_MQTT_PUB_DEFAULT_TIMEOUT))
        {
            DMCLOG_E("dm_wx_mqtt_pubmsg fail\r\n");
            goto EXIT;
        }
        DMCLOG_D("pub msg succ");

        safe_free(response_str);
        safe_free(fromId);
    }
EXIT:
    safe_free(pmsg->payload);
    safe_free(pmsg->topic_name);
    safe_free(pmsg);
    EXIT_FUNC();
    return NULL;
}

/* if -1 (false) is returned,it will be retried later.
 *If 0 is returned then the message data must been freed,
 */
static int message_callback(void* context, char* topic_name, int topic_len, MQTTAsync_message* message)
{
    ENTER_FUNC();
	int ret = 1,err;

    pthread_t message_callback_handle_thread;
    message_callback_handle_msg_t *pmsg = (message_callback_handle_msg_t *)calloc(1,sizeof(message_callback_handle_msg_t));
    if(!pmsg)
    {
        DMCLOG_E("message_callback_handle_msg_t alloc fail\r\n");
        goto exit;
    }

	pmsg->context = context;
    pmsg->payload = (char *)calloc(1,message->payloadlen + 1);
    memcpy(pmsg->payload,message->payload,message->payloadlen);
    DMCLOG_D("pmsg->payload = %s",pmsg->payload);
    pmsg->topic_name = (char *)calloc(1,strlen(topic_name) + 1);
    strcpy(pmsg->topic_name,topic_name);
    pmsg->payloadlen = message->payloadlen;
    ret = pthread_create(&message_callback_handle_thread,NULL,message_callback_thread,(void *)pmsg);
    if (ret != 0)
    {
        DMCLOG_E("failed to create message_callback_thread: %d,errno = %d", err,errno);
        safe_free(pmsg);
        goto exit;
    }
	pthread_detach(message_callback_handle_thread);
exit:
    //MQTTAsync_free(topic_name);
    //MQTTAsync_freeMessage(&message);
    EXIT_FUNC();
	return 1;
}

static void deliveryComplete_callback(void* context, MQTTAsync_token dt)
{
    ENTER_FUNC();
    DMCLOG_D("deliveryComplete_callback");
    EXIT_FUNC();
	return;
}

/**
 *  ¹¦ÄÜ£º¹Ø±Õmqtt socket Á¬½Ó
 *  È¨ÏÞ£º¿ª·ÅÈ¨ÏÞ
 *
 *  @param
 *
 *  @return 0Îª³É¹¦£¬·Ç0ÎªÊ§°Ü¡£
 */
rd_mqtt_return_e _dm_mqtt_disconnect(void)
{
	
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
    DMCLOG_D("do dm_wx_mqtt_disconnect....\r\n");
    disc_opts.onSuccess = onDisconnect;
    int rc = 0;
    if ((rc = MQTTAsync_disconnect(wx_mqtt_client, &disc_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start disconnect, return code %d\n", rc);
        return WX_MQTT_FAILURE;
    }
    
    while(!disconnected)
#if defined(WIN32)
        Sleep(100);
#else
    usleep(10000L);
#endif
    //    MQTTAsync_destroy(&wx_mqtt_client);
    return WX_MQTT_SUCCESS;
}

rd_mqtt_return_e _dm_mqtt_destory(void)
{
    MQTTAsync_destroy(&wx_mqtt_client);
    return WX_MQTT_SUCCESS;
}


rd_mqtt_return_e _dm_mqtt_unsub_topic(void)
{
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	rd_mqtt_return_e ret = WX_MQTT_FAILURE;
	if (!MQTTAsync_isConnected(wx_mqtt_client)) 
	{
		DMCLOG_E("wx_mqtt_client doesn't Connected\r\n");
		ret =  WX_MQTT_FAILURE;
		goto exit;
	}
    char topic[256] = {0};
    sprintf(topic,"%s%s",COMSUMER_TOPIC_PREFIX,mqtt_device_id);
	DMCLOG_D("topic:%s",topic);
	if(0 != MQTTAsync_unsubscribe(wx_mqtt_client, topic,&opts))
	{
		DMCLOG_E("MQTTAsync_unsubscribe fail,topic:%s\r\n",topic);
		ret =  WX_MQTT_FAILURE;
		goto exit;
	}
	ret =  WX_MQTT_SUCCESS;
exit:
	return ret;
}

void del_n(char *str)
{
	int i=0;
	while(str[i])
	{
		if('\n' == str[i])
		{
			str[i]=0;
			if(i>0 && str[i-1]=='\r')
				str[i-1]=0;
		}
		i++;
	}
}


int get_cfg_str(char *param,char *ret_str)
{
	char get_str[128]={0};
	char tmp[128]={0};
	sprintf(get_str,"cfg get \'%s\'",param);
	FILE *fp = popen(get_str,"r");
	fgets(tmp,128,fp);
	pclose(fp);
	del_n(tmp);
	if(strlen(tmp)<=1)
		return 0;
	else
	{
		strcpy(ret_str,tmp+strlen(param)+1);
		return 1;
	}
}


int _dm_mqtt_start()
{
	ENTER_FUNC();
    int rc;
	char url[256] = {0};
	char client_id[128] = {0};
    sprintf(url,"%s:%s",WX_MQTT_SERVER_URL,WX_MQTT_SERVER_PORT);
	rc = get_cfg_str("dmdownload_id",mqtt_device_id);
	if(rc == 0)
	{
		return -1;
	}
	//strcpy(mqtt_device_id ,"airdisk_0005");
	DMCLOG_D("mqtt device id = %s",mqtt_device_id);
    sprintf(client_id,"%s-%s",WX_MQTT_CLIENTID_PREFIX,mqtt_device_id);
    
    DMCLOG_D("client_id = %s,url = %s",client_id,url);
    MQTTAsync_create(&wx_mqtt_client, url, client_id,
                      MQTTCLIENT_PERSISTENCE_DEFAULT, NULL);
	DMCLOG_D("MQTTAsync_setCallbacks");
    if(MQTTAsync_setCallbacks(wx_mqtt_client, wx_mqtt_client, connectionLost, message_callback, deliveryComplete_callback) != 0)
    {
        DMCLOG_E("Can't set callback\r\n");
#if defined (OPENSSL) || defined (TLSSOCKET)
        /*if (enable_ssl)
        {
            free(ssl_opts.trustStore);
            free(uristring);
        }*/
#endif
        return -1;
    }
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.username = WX_MQTT_USERNAME;
    conn_opts.password = WX_MQTT_PASSWORD;
    conn_opts.context = wx_mqtt_client;
    conn_opts.onSuccess = client_onConnect;
    conn_opts.onFailure = client_onConnectFailure;
    conn_opts.retryInterval = 1;
    
    if ((rc = MQTTAsync_connect(wx_mqtt_client, &conn_opts)) != 0)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
    return rc;
}





