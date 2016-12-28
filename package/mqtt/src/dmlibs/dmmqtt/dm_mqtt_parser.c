//
//  dm_mqtt_parser.c
//  DM MQTT PARSER
//
//  Created by Oliver on 16/6/27.
//  Copyright © 2016年 Oliver. All rights reserved.
//

#include "dm_mqtt_parser.h"
#include "my_debug.h"
#include "my_json.h"
#include "dmari2ac.h"
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define   WX_MQTT_MSGID_START						(1024)

static uint64_t mqtt_msgid   = WX_MQTT_MSGID_START;


#define ULONGLONG_MAX UINT_MAX //4294967295


const char* bb_basename(const char *name)
{
    const char *cp = strrchr(name, '/');
    if (cp)
        return cp + 1;
    return name;
}


uint64_t dm_wx_mqtt_get_msgid(void)
{
    unsigned msgid;
    //	MSG ID LOCK
    msgid = mqtt_msgid;
    mqtt_msgid++;
    if(mqtt_msgid == ULONGLONG_MAX)
    {
        mqtt_msgid = WX_MQTT_MSGID_START;
    }
    //	MSG ID UNLOCK
    return msgid;
}

/**
 *  功能：解析接收到的数据，将结果置于结构体中
 *  权限：开放权限
 *
 *  @param clientInfo->recv_buf:从mmqt服务端接收到的数据，clientInfo:将结果封装在指针指向的结构体对象空间
 *
 *  @return 0为成功，非0为异常。
 */
int mqtt_parser_client_request(MqttClientInfo *clientInfo)
{
    int res = 0,i;
    JObj *r_json = JSON_PARSE(clientInfo->recv_buf);
    if(r_json == NULL)
    {
        DMCLOG_E("access NULL");
        res = -1;
        return res;
    }
    if(is_error(r_json))
    {
        DMCLOG_E("### error:post data is not a json string");
        res = -1;
        return res;
    }
    JObj *msgType_json = JSON_GET_OBJECT(r_json,"msgType");
    JObj *fromId_json = JSON_GET_OBJECT(r_json,"fromId");
    JObj *fromType_json = JSON_GET_OBJECT(r_json,"fromType");
    JObj *msgDate_json = JSON_GET_OBJECT(r_json,"msgDate");
    JObj *msgId_json = JSON_GET_OBJECT(r_json,"msgId");
    
    if(msgType_json == NULL||fromId_json == NULL||fromType_json == NULL||msgDate_json == NULL||msgId_json == NULL)
    {
        DMCLOG_E("access NULL");
        res = -1;
        return res;
    }
    
    if(msgType_json != NULL)
        strcpy(clientInfo->msgType,JSON_GET_OBJECT_VALUE(msgType_json,string));
    if(fromId_json != NULL)
        strcpy(clientInfo->fromId,JSON_GET_OBJECT_VALUE(fromId_json,string));
    if(fromType_json != NULL)
        clientInfo->fromType = JSON_GET_OBJECT_VALUE(fromType_json,int);
    if(msgDate_json != NULL)
        clientInfo->msgData = JSON_GET_OBJECT_VALUE(msgDate_json,int);
    if(msgId_json != NULL)
        clientInfo->msgId = JSON_GET_OBJECT_VALUE(msgId_json,int);
    
    JObj *services_json = JSON_GET_OBJECT(r_json,"services");
    if(services_json == NULL)
    {
        DMCLOG_E("access service NULL");
        res = -1;
        return res;
    }
    clientInfo->clientService = (MqttClientServices *)calloc(1,sizeof(MqttClientServices));
    JObj *music_json = JSON_GET_OBJECT(services_json,"dmmsg_music");
    if(music_json != NULL)
    {
        DMCLOG_D("access music");
        clientInfo->serviceFlag |= 0x02;
        //TODO
    }
    JObj *download_json = JSON_GET_OBJECT(services_json,"dmmsg_download");
    if(download_json != NULL)
    {
        DMCLOG_D("access download");
        clientInfo->serviceFlag |= 0x01;
        clientInfo->clientService->clientDownloadInfo = (DownloadTaskInfo *)calloc(1,sizeof(DownloadTaskInfo));
        dl_list_init(&clientInfo->clientService->clientDownloadInfo->d_head);
        dl_list_init(&clientInfo->clientService->clientDownloadInfo->t_head);
        
        clientInfo->clientService->clientDownloadInfo->add_task_cb = ari2ac_add_task;
        clientInfo->clientService->clientDownloadInfo->ctr_all_task_cb = ari2ac_ctr_all_task;
        clientInfo->clientService->clientDownloadInfo->ctr_task_cb = ari2ac_ctr_task;
        clientInfo->clientService->clientDownloadInfo->query_task_by_status_cb = ari2ac_query_task_by_status;
        clientInfo->clientService->clientDownloadInfo->query_task_by_gid_cb = ari2ac_query_task_by_gid;
        clientInfo->clientService->clientDownloadInfo->query_dir_cb = ari2ac_query_dir;
        clientInfo->clientService->clientDownloadInfo->query_global_cb = ari2ac_query_global;
		clientInfo->clientService->clientDownloadInfo->ctr_speed_cb = ari2ac_ctr_speed;
        
        
        JObj *downloadDir_json = JSON_GET_OBJECT(download_json,"download_dir");
        if(downloadDir_json != NULL)
        {
            int array_len = JSON_GET_ARRAY_LEN(downloadDir_json);
            DMCLOG_D("array_len = %d",array_len);
            if(array_len > 0)
            {
                clientInfo->clientService->clientDownloadInfo->set_flag |= 0x04; //设置目录
            }else{
                clientInfo->clientService->clientDownloadInfo->get_flag |= 0x04; //获取目录
            }
            
            for(i = 0;i < array_len;i++)
            {
                JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(downloadDir_json,i);
                if(para_json == NULL)
                {
                    DMCLOG_D("access NULL");
                    res = -1;
                    return res;
                }
                
                JObj *name_json = JSON_GET_OBJECT(para_json,"name");
                JObj *path_json = JSON_GET_OBJECT(para_json,"path");
                if(name_json == NULL||path_json == NULL)
                {
                    DMCLOG_D("access NULL");
                    res = -1;
                    return res;
                }
                const char *name = JSON_GET_OBJECT_VALUE(name_json,string);
                const char *path = JSON_GET_OBJECT_VALUE(path_json,string);
                DownloadDir *fdi = (DownloadDir *)calloc(1,sizeof(DownloadDir));
                fdi->dir_name = (char *)calloc(1,strlen(name) + 1);
                strcpy(fdi->dir_name,name);
                fdi->dir_path = (char *)calloc(1,strlen(path) + 1);
                strcpy(fdi->dir_path,path);
                dl_list_add_tail(&clientInfo->clientService->clientDownloadInfo->d_head, &fdi->node);
            }
        }
        
        JObj *taskList_json = JSON_GET_OBJECT(download_json,"task_list");
        if(taskList_json != NULL)
        {
            int array_len = JSON_GET_ARRAY_LEN(taskList_json);
            DMCLOG_D("array_len = %d",array_len);
            if(array_len > 0)
            {
                clientInfo->clientService->clientDownloadInfo->set_flag |= 0x02; //控制队列任务
            }else{
                clientInfo->clientService->clientDownloadInfo->get_flag |= 0x02; //获取任务队列
            }
            for(i = 0;i < array_len;i++)
            {
                JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(taskList_json,i);
                if(para_json != NULL)
                {
                    MDownloadTask *fdi = (MDownloadTask *)calloc(1,sizeof(MDownloadTask));
                    /*JObj *name_json = JSON_GET_OBJECT(para_json,"name");
                    if(name_json != NULL)
                    {
                        const char *name = JSON_GET_OBJECT_VALUE(name_json,string);
                        fdi->file_name = (char *)calloc(1,strlen(name) + 1);
                        strcpy(fdi->file_name,name);
                    }*/
					JObj *taskId_json = JSON_GET_OBJECT(para_json,"task_id");
                    if(taskId_json != NULL)
                    {
                        const char *task_id = JSON_GET_OBJECT_VALUE(taskId_json,string);
                        strcpy(fdi->task_id,task_id);
                    }
                    JObj *path_json = JSON_GET_OBJECT(para_json,"download_path");
                    if(path_json != NULL)
                    {
                        const char *path = JSON_GET_OBJECT_VALUE(path_json,string);
                        fdi->download_path = (char *)calloc(1,strlen(path) + 1);
                        strcpy(fdi->download_path,path);
						fdi->file_name = bb_basename(fdi->download_path);
                    }
                    
                    JObj *url_json = JSON_GET_OBJECT(para_json,"url");
                    if(url_json != NULL)
                    {
                        const char *url = JSON_GET_OBJECT_VALUE(url_json,string);
                        fdi->url = (char *)calloc(1,strlen(url) + 1);
                        strcpy(fdi->url,url);
                    }
                    
                    JObj *progress_json = JSON_GET_OBJECT(para_json,"progress");
                    if(progress_json != NULL)
                    {
                        fdi->progress = JSON_GET_OBJECT_VALUE(progress_json,int);
                    }
                    
                    JObj *tSize_json = JSON_GET_OBJECT(para_json,"tSize");
                    if(tSize_json != NULL)
                    {
                        fdi->length = JSON_GET_OBJECT_VALUE(tSize_json,int64);
                    }
                    
                    JObj *pSize_json = JSON_GET_OBJECT(para_json,"pSize");
                    if(pSize_json != NULL)
                    {
                        fdi->completed_length = JSON_GET_OBJECT_VALUE(pSize_json,int64);
                    }
                    
                    JObj *status_json = JSON_GET_OBJECT(para_json,"status");
                    if(status_json != NULL)
                    {
                        fdi->status = JSON_GET_OBJECT_VALUE(status_json,int);
                    }
                    
                    JObj *ctrl_json = JSON_GET_OBJECT(para_json,"ctrl");
                    if(ctrl_json != NULL)
                    {
                        fdi->ctrl = JSON_GET_OBJECT_VALUE(ctrl_json,int);
                    }
                    
                    JObj *dSpeed_json = JSON_GET_OBJECT(para_json,"dSpeed");
                    if(dSpeed_json != NULL)
                    {
                        fdi->download_speed = JSON_GET_OBJECT_VALUE(dSpeed_json,int64);
                    }
                    
                    JObj *uSpeed_json = JSON_GET_OBJECT(para_json,"uSpeed");
                    if(uSpeed_json != NULL)
                    {
                        fdi->upload_speed = JSON_GET_OBJECT_VALUE(uSpeed_json,int64);
                    }

					JObj *type_json = JSON_GET_OBJECT(para_json,"type");
                    if(type_json != NULL)
                    {
                        fdi->type = JSON_GET_OBJECT_VALUE(type_json,int);
                    }
                    dl_list_add_tail(&clientInfo->clientService->clientDownloadInfo->t_head, &fdi->node);
                }
            }
        }
        
        JObj *allTask_json = JSON_GET_OBJECT(download_json,"allTask");
        if(allTask_json != NULL)
        {
            JObj *ctrl_json = JSON_GET_OBJECT(allTask_json,"ctrl");
            if(ctrl_json != NULL)
            {
                clientInfo->clientService->clientDownloadInfo->set_flag |= 0x01; //control all task list
                clientInfo->clientService->clientDownloadInfo->ctrl = JSON_GET_OBJECT_VALUE(ctrl_json,int);
            }else{
				clientInfo->clientService->clientDownloadInfo->get_flag |= 0x01; //get global status
			}
            JObj *dSpeed_json = JSON_GET_OBJECT(allTask_json,"dSpeed");
			JObj *uSpeed_json = JSON_GET_OBJECT(allTask_json,"uSpeed");
			
            if(dSpeed_json != NULL&&uSpeed_json != NULL)
            {
            	clientInfo->clientService->clientDownloadInfo->set_flag |= 0x08; //control global  speed
                clientInfo->clientService->clientDownloadInfo->dGlobalStatus.download_speed = JSON_GET_OBJECT_VALUE(dSpeed_json,int);
				clientInfo->clientService->clientDownloadInfo->dGlobalStatus.upload_speed = JSON_GET_OBJECT_VALUE(uSpeed_json,int);
				DMCLOG_D("download_speed = %d",clientInfo->clientService->clientDownloadInfo->dGlobalStatus.download_speed);
				DMCLOG_D("upload_speed = %d",clientInfo->clientService->clientDownloadInfo->dGlobalStatus.upload_speed);
            }
        }
        
    }
    if(r_json != NULL)
    {
        JSON_PUT_OBJECT(r_json);
    }
    return res;
}

void free_task_list(struct dl_list *phead)
{
    MDownloadTask *item, *n;
    
    
    if(phead == NULL || dl_list_empty(phead))
    {
        return;
    }
    
    dl_list_for_each_safe(item,n,phead,MDownloadTask,node)
    {
    	safe_free(item->download_path);
		safe_free(item->url);
        dl_list_del(&item->node);
        free(item);
    }
    
}

void free_dir_list(struct dl_list *phead)
{
    DownloadDir *item, *n;
    
    if(phead == NULL || dl_list_empty(phead))
    {
        return;
    }
    
    dl_list_for_each_safe(item,n,phead,DownloadDir,node)
    {
        dl_list_del(&item->node);
        free(item);
    }
    
}


/**
 *  功能：根据相关的请求进行处理
 *  权限：开放权限
 *
 *  @param clientInfo:将结果封装在指针指向的结构体对象空间
 *
 *  @return 0为成功，非0为异常。
 */
int mqtt_handle_client_request(MqttClientInfo *clientInfo)
{
    ENTER_FUNC();
    int res = DMDOWNLOAD_SUCCES;
    if(clientInfo == NULL)
    {
        DMCLOG_E("para is null");
        return -1;
    }
    if(clientInfo->serviceFlag & 0x01)
    {
        //进入下载模块的管理
        if(clientInfo->clientService == NULL||clientInfo->clientService->clientDownloadInfo == NULL)
        {
            DMCLOG_E("para is null");
            return -1;
        }
        
        if(clientInfo->clientService->clientDownloadInfo->get_flag&0x01)//get dir|get task list|get global status
        {
            if((res = download_query_global(clientInfo->clientService->clientDownloadInfo)) != DMDOWNLOAD_SUCCES)
            {
                DMCLOG_E("download query global error");
                return res;
            }
        }
        
        if(clientInfo->clientService->clientDownloadInfo->get_flag&0x02)//get dir|get task list|get global status
        {
            if((res = download_query_task(clientInfo->clientService->clientDownloadInfo)) != DMDOWNLOAD_SUCCES)
            {
                DMCLOG_E("download query task error");
                return res;
            }
        }
        
        if(clientInfo->clientService->clientDownloadInfo->get_flag&0x04)//get dir|get task list|get global status
        {
            if((res = download_query_dir(clientInfo->clientService->clientDownloadInfo)) != DMDOWNLOAD_SUCCES)
            {
                DMCLOG_E("download query dir error");
                return res;
            }
        }
        
        if(clientInfo->clientService->clientDownloadInfo->set_flag&0x01)//set global speed |set dir|set task list|set global task
        {
        	if((res = download_ctr_all_task(clientInfo->clientService->clientDownloadInfo)) != DMDOWNLOAD_SUCCES)
            {
                DMCLOG_E("download ctr all task error");
                return res;
            }
        }
        
        if(clientInfo->clientService->clientDownloadInfo->set_flag&0x02)//set global speed |set dir|set task list|set global task
        {
            if((res = download_ctr_task(clientInfo->clientService->clientDownloadInfo)) != DMDOWNLOAD_SUCCES)
            {
                DMCLOG_E("download ctr task error");
                return res;
            }
        }
        if(clientInfo->clientService->clientDownloadInfo->set_flag&0x04)//set global speed |set dir|set task list|set global task
        {
//            if((res = download_query_dir(clientInfo->clientService->clientDownloadInfo)) != DMDOWNLOAD_SUCCES)
//            {
//                DMCLOG_E("download query global error");
//                return -1;
//            }
        }
		
		if(clientInfo->clientService->clientDownloadInfo->set_flag&0x08)//set global speed |set dir|set task list|set global task
        {
            if((res = download_ctr_speed(clientInfo->clientService->clientDownloadInfo)) != DMDOWNLOAD_SUCCES)
            {
                DMCLOG_E("download ctr speed error");
                return res;
            }
        }
    }
    EXIT_FUNC();
    return 0;
}

/**
 *  功能：将处理后的数据封装成JSON数据字符串格式进行转发
 *  权限：开放权限
 *
 *  @param clientInfo:将结果封装在指针指向的结构体对象空间,clientInfo->send_buf:后台处理后发往服务器的数据
 *
 *  @return 0为成功，非0为异常。
 */
int mqtt_conbin_client_response(MqttClientInfo *clientInfo)
{
    const char *response_str = NULL;
    JObj *response_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(response_json, "fromId", JSON_NEW_OBJECT(clientInfo->fromId,string));
    JSON_ADD_OBJECT(response_json, "fromType", JSON_NEW_OBJECT(clientInfo->fromType,int));
    JSON_ADD_OBJECT(response_json, "error_code", JSON_NEW_OBJECT(clientInfo->error_code,int));
    if(clientInfo->error_msg != NULL)
        JSON_ADD_OBJECT(response_json, "error_msg", JSON_NEW_OBJECT(clientInfo->error_msg,string));
    if(!strcmp(clientInfo->msgType,"set"))
        JSON_ADD_OBJECT(response_json, "msgType", JSON_NEW_OBJECT("setResp",string));
    else
        JSON_ADD_OBJECT(response_json, "msgType", JSON_NEW_OBJECT("getResp",string));
    JSON_ADD_OBJECT(response_json, "msgDate", JSON_NEW_OBJECT(time(NULL),int));
    JSON_ADD_OBJECT(response_json, "msgId", JSON_NEW_OBJECT(clientInfo->msgId,int));
    
    if(!strcmp(clientInfo->msgType,"set"))
    {
        DMCLOG_D("no need service response");
        goto EXIT;
    }
    JObj *services_json = JSON_NEW_EMPTY_OBJECT();
    
    if(clientInfo->serviceFlag&0x01)//dmmsg_music|dmmsg_download
    {
        JObj *dmmsg_download_json = JSON_NEW_EMPTY_OBJECT();
        if(clientInfo->clientService->clientDownloadInfo->get_flag&0x01)//获取目录｜获取任务列表｜获取全局状态
        {
            JObj *allTask_json = JSON_NEW_EMPTY_OBJECT();
            JSON_ADD_OBJECT(allTask_json, "uSpeed", JSON_NEW_OBJECT(clientInfo->clientService->clientDownloadInfo->dGlobalStatus.upload_speed,int));
            JSON_ADD_OBJECT(allTask_json, "dSpeed", JSON_NEW_OBJECT(clientInfo->clientService->clientDownloadInfo->dGlobalStatus.download_speed,int));
            JSON_ADD_OBJECT(dmmsg_download_json, "allTask", allTask_json);
        }
        
        if(clientInfo->clientService->clientDownloadInfo->get_flag&0x02)//获取目录｜获取任务列表｜获取全局状态
        {
            JObj *task_array = JSON_NEW_ARRAY();
            MDownloadTask *fdi = NULL;
            dl_list_for_each(fdi, &clientInfo->clientService->clientDownloadInfo->t_head, MDownloadTask, node)
            {
                JObj *task_info = JSON_NEW_EMPTY_OBJECT();
                JSON_ADD_OBJECT(task_info, "task_id", JSON_NEW_OBJECT(fdi->task_id, string));
                if(fdi->download_path != NULL)	
               	{
					JSON_ADD_OBJECT(task_info, "download_path", JSON_NEW_OBJECT(fdi->download_path, string));
					if(fdi->dir != NULL)
					{
						char *tmp = fdi->download_path + strlen(fdi->dir) + 1;
						if(tmp != NULL)
						{
							DMCLOG_D("tmp = %s",tmp);
							char *end = strchr(tmp,'/');
							if(end != NULL)
							{
								*end = 0;
								fdi->file_name = (char *)calloc(1,strlen(tmp) + 1);
								strcpy(fdi->file_name,tmp);
								*end = '/';
							}else{
								fdi->file_name = (char *)calloc(1,strlen(tmp) + 1);
								strcpy(fdi->file_name,tmp);
							}
							DMCLOG_D("fdi->file_name = %s",fdi->file_name);
						}
					}
               		JSON_ADD_OBJECT(task_info, "name", JSON_NEW_OBJECT(fdi->file_name, string));	
				}else{
					if(fdi->uri != NULL)
					{
						JSON_ADD_OBJECT(task_info, "name", JSON_NEW_OBJECT(fdi->uri, string));
					}
				}
                
                JSON_ADD_OBJECT(task_info, "progress", JSON_NEW_OBJECT(fdi->progress, int));
                JSON_ADD_OBJECT(task_info, "tSize", JSON_NEW_OBJECT(fdi->length, int64));
                JSON_ADD_OBJECT(task_info, "pSize", JSON_NEW_OBJECT(fdi->completed_length, int64));
                JSON_ADD_OBJECT(task_info, "status", JSON_NEW_OBJECT(fdi->status, int));
                JSON_ADD_OBJECT(task_info, "ctrl", JSON_NEW_OBJECT(fdi->ctrl, int));
                JSON_ADD_OBJECT(task_info, "dSpeed", JSON_NEW_OBJECT(fdi->download_speed, int));
                JSON_ADD_OBJECT(task_info, "uSpeed", JSON_NEW_OBJECT(fdi->upload_speed, int));
                JSON_ARRAY_ADD_OBJECT(task_array, task_info);
            }
            JSON_ADD_OBJECT(dmmsg_download_json, "task_list", task_array);
        }
        
        if(clientInfo->clientService->clientDownloadInfo->get_flag&0x04)//获取目录｜获取任务列表｜获取全局状态
        {
            JObj *dir_array = JSON_NEW_ARRAY();
            DownloadDir *fdi = NULL;
            dl_list_for_each(fdi, &clientInfo->clientService->clientDownloadInfo->d_head, DownloadDir, node)
            {
                JObj *dir_info = JSON_NEW_EMPTY_OBJECT();
                
                if(fdi->dir_name != NULL)
                    JSON_ADD_OBJECT(dir_info, "dir_name", JSON_NEW_OBJECT(fdi->dir_name, string));
                
                if(fdi->dir_path != NULL)
                    JSON_ADD_OBJECT(dir_info, "dir_path", JSON_NEW_OBJECT(fdi->dir_path, string));
                
                JSON_ADD_OBJECT(dir_info, "file_count", JSON_NEW_OBJECT(fdi->file_count, int));
                JSON_ARRAY_ADD_OBJECT(dir_array, dir_info);
            }
            JSON_ADD_OBJECT(dmmsg_download_json, "download_dir", dir_array);
        }
        JSON_ADD_OBJECT(services_json, "dmmsg_download", dmmsg_download_json);
    }
    
    
    if(clientInfo->serviceFlag&0x02)
    {
        //TODO
    }
    JSON_ADD_OBJECT(response_json, "services", services_json);
EXIT:
    response_str = JSON_TO_STRING(response_json);
    if(response_str == NULL)
    {
        JSON_PUT_OBJECT(response_json);
        return -1;
    }
    clientInfo->send_buf = (char *)calloc(1,strlen(response_str) + 1);
    strcpy(clientInfo->send_buf,response_str);
    JSON_PUT_OBJECT(response_json);
    return 0;
}


/**
 *  功能：封装上报(notify)的字符串
 *  权限：开放权限
 *
 *  @param clientInfo:将结果封装在指针指向的结构体对象空间,clientInfo->send_buf:后台处理后发往服务器的数据
 *
 *  @return 0为成功，非0为异常。
 */
int mqtt_conbin_client_notify(MqttClientInfo *clientInfo)
{
    JObj *response_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(response_json, "msgType", JSON_NEW_OBJECT("notify", string));
    JSON_ADD_OBJECT(response_json, "fromId", JSON_NEW_OBJECT(clientInfo->fromId, string));
    JSON_ADD_OBJECT(response_json, "fromType", JSON_NEW_OBJECT(clientInfo->fromType, int));
    JSON_ADD_OBJECT(response_json, "msgId", JSON_NEW_OBJECT(dm_wx_mqtt_get_msgid(), int64));
    
    JObj *services_json = JSON_NEW_EMPTY_OBJECT();
    JObj *operation_status_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(operation_status_json, "status", JSON_NEW_OBJECT(clientInfo->operation_status, int));
    JSON_ADD_OBJECT(services_json, "operation_status", operation_status_json);
    JSON_ADD_OBJECT(response_json, "services", services_json);
    const char *response_str = JSON_TO_STRING(response_json);
    if(response_str == NULL)
    {
        JSON_PUT_OBJECT(response_json);
        return -1;
    }
    
    clientInfo->send_buf = (char *)calloc(1,strlen(response_str) + 1);
    strcpy(clientInfo->send_buf,response_str);
    JSON_PUT_OBJECT(response_json);
    return 0;
}

/**
 *  功能：根据请求任务封装json数据 cmd = 0:添加任务，1:控制所有任务，2:获取目录，3:获取任务列表,4:获取全局状态
 *  权限：开放权限
 *
 *  @param clientInfo:将结果封装在指针指向的结构体对象空间,clientInfo->send_buf:后台处理后发往服务器的数据
 *
 *  @return 0为成功，非0为异常。
 */
int mqtt_conbin_client_request(int cmd,MqttClientInfo *clientInfo)
{
    const char *response_str = NULL;
    JObj *response_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(response_json, "fromId", JSON_NEW_OBJECT(clientInfo->fromId,string));
    JSON_ADD_OBJECT(response_json, "fromType", JSON_NEW_OBJECT(clientInfo->fromType,int));
    if(cmd == 0 || cmd == 1)
    {
        JSON_ADD_OBJECT(response_json, "msgType", JSON_NEW_OBJECT("set",string));
    }else if(cmd == 2||cmd == 3||cmd == 4)
    {
        JSON_ADD_OBJECT(response_json, "msgType", JSON_NEW_OBJECT("get",string));
    }

    JSON_ADD_OBJECT(response_json, "msgDate", JSON_NEW_OBJECT(time(NULL),int));
    JSON_ADD_OBJECT(response_json, "msgId", JSON_NEW_OBJECT(dm_wx_mqtt_get_msgid(),int));
    
    
    JObj *services_json = JSON_NEW_EMPTY_OBJECT();
    
    JObj *dmmsg_download_json = JSON_NEW_EMPTY_OBJECT();
    
    if(cmd == 0||cmd == 1)
    {
        JObj *task_array = JSON_NEW_ARRAY();
        MDownloadTask *fdi = NULL;
        dl_list_for_each(fdi, &clientInfo->clientService->clientDownloadInfo->t_head, MDownloadTask, node)
        {
            JObj *task_info = JSON_NEW_EMPTY_OBJECT();
            
            JSON_ADD_OBJECT(task_info, "task_id", JSON_NEW_OBJECT(fdi->task_id, string));
            if(fdi->file_name != NULL)
                JSON_ADD_OBJECT(task_info, "name", JSON_NEW_OBJECT(fdi->file_name, string));
            
            if(fdi->download_path != NULL)
                JSON_ADD_OBJECT(task_info, "download_path", JSON_NEW_OBJECT(fdi->download_path, string));
            
            if(fdi->url != NULL)
                JSON_ADD_OBJECT(task_info, "url", JSON_NEW_OBJECT(fdi->url, string));
            
            JSON_ADD_OBJECT(task_info, "progress", JSON_NEW_OBJECT(fdi->progress, int));
            JSON_ADD_OBJECT(task_info, "tSize", JSON_NEW_OBJECT(fdi->length, int64));
            JSON_ADD_OBJECT(task_info, "pSize", JSON_NEW_OBJECT(fdi->completed_length, int64));
            JSON_ADD_OBJECT(task_info, "status", JSON_NEW_OBJECT(fdi->status, int));
            JSON_ADD_OBJECT(task_info, "ctrl", JSON_NEW_OBJECT(fdi->ctrl, int));
            JSON_ADD_OBJECT(task_info, "dSpeed", JSON_NEW_OBJECT(fdi->download_speed, int));
            JSON_ADD_OBJECT(task_info, "uSpeed", JSON_NEW_OBJECT(fdi->upload_speed, int));
            JSON_ARRAY_ADD_OBJECT(task_array, task_info);
        }
        JSON_ADD_OBJECT(dmmsg_download_json, "task_list", task_array);
    }else if(cmd == 2)
    {
        JObj *dir_array = JSON_NEW_ARRAY();
        JSON_ADD_OBJECT(dmmsg_download_json, "download_dir", dir_array);
    }else if(cmd == 3)
    {
        JObj *task_array = JSON_NEW_ARRAY();
        JSON_ADD_OBJECT(dmmsg_download_json, "task_list", task_array);
    }else if(cmd == 4)
    {
        JObj *allTask_json = JSON_NEW_EMPTY_OBJECT();
        JSON_ADD_OBJECT(dmmsg_download_json, "allTask", allTask_json);
    }
    
    JSON_ADD_OBJECT(services_json, "dmmsg_download", dmmsg_download_json);
    JSON_ADD_OBJECT(response_json, "services", services_json);
EXIT:
    response_str = JSON_TO_STRING(response_json);
    if(response_str == NULL)
    {
        JSON_PUT_OBJECT(response_json);
        return -1;
    }
    clientInfo->send_buf = (char *)calloc(1,strlen(response_str) + 1);
    strcpy(clientInfo->send_buf,response_str);
    JSON_PUT_OBJECT(response_json);
    return 0;
}




