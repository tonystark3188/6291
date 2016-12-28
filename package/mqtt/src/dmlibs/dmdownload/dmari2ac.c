//
//  dmari2ac.c
//  DMARI2AC
//
//  Created by Oliver on 16/6/27.
//  Copyright © 2016年 Oliver. All rights reserved.
//

#include "dmari2ac.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "my_debug.h"
#include "my_json.h"
#include "list.h"
#include "ghttp.h"



int aGet(char* url, char* params, int timeout, char *result, int *result_len) {
    int ret = 0;
    ghttp_request *request = NULL;
    ghttp_status status;
    char *tempBuf;
    request = ghttp_request_new();
    
    if(params!=NULL&&strlen(params)>0)
    {
        char tmp[1024];
        strcpy(tmp,url);
        if(strchr(tmp, '?') == NULL)//url不存在
        {     strcat(tmp,"?")  ;
        }
        strcat(tmp,params) ;
        printf("%s\n",tmp);
        ghttp_set_uri(request, tmp);
    }else{
        ghttp_set_uri(request, url);
    }
    ghttp_set_type(request, ghttp_type_get); //get方法
    ghttp_set_header(request, http_hdr_Connection, "close");
    char timeout_str[10];
    sprintf(timeout_str, "%d", timeout);
    ghttp_set_header(request, http_hdr_Timeout, timeout_str);
    
    ghttp_prepare(request);
    status = ghttp_process(request);
    
    if(status == ghttp_error)
    {
        printf("ghttp_process error\n");
        ret = -1;
        goto EXIT;
    }
    printf("Status code -> %d\n",ghttp_status_code(request));
    tempBuf = ghttp_get_body(request);
    *result_len = ghttp_get_body_len(request);
    if(tempBuf)
    {
        strcpy(result, tempBuf);
    }
EXIT:
    ghttp_request_destroy(request);
    return ret;
}

int aPost(char* uri, char* params, int timeout, char **result) 
{
	ENTER_FUNC();
    ghttp_request *request = NULL;
    ghttp_status status;
    char *tempBuf;
    int len;
    request = ghttp_request_new();
    if (ghttp_set_uri(request, uri) == -1)
        return -1;
	
    if (ghttp_set_type(request, ghttp_type_post) == -1) //post
        return -1;

    ghttp_set_header(request, http_hdr_Content_Type,"application/x-www-form-urlencoded");
    char timeout_str[10];
    sprintf(timeout_str, "%d", timeout);
    ghttp_set_header(request, http_hdr_Timeout, timeout_str);
    //ghttp_set_sync(request, ghttp_sync); //set sync
    len = (int)strlen(params);
    ghttp_set_body(request, params, len); //
    ghttp_prepare(request);
	status = ghttp_process(request);
    if (status == ghttp_error)
   	{
   		ghttp_clean(request);
		ghttp_request_destroy(request);
   		EXIT_FUNC();
		return -1;
	}
       
    tempBuf = ghttp_get_body(request); //test
    len = ghttp_get_body_len(request);
    if(tempBuf)
    {	
    	*result=(char *)calloc(1,len + 1);
		if(*result == NULL){
			DMCLOG_E("malloc error.");	
		}else 
		{
			strcpy(*result, tempBuf);	
		    DMCLOG_D("*result=%s",*result);		
		}
    }
    ghttp_clean(request);
	ghttp_request_destroy(request);
	EXIT_FUNC();
    return 0;
}

/**
 *  功能：添加下载任务
 *  权限：开放权限
 *
 *  @param url:输入，资源地址；task_id:输出，任务唯一标识符
 *
 *  @return 0为成功，非0为异常。
 */
#define NEW_JSON_OBJ struct json_object* 
#define POST_TIME_OUT 20
#define URI "http://127.0.0.1:6800/jsonrpc?tm=1467437057221"
#define ADD_DOWNLOAD_URL "[{\"jsonrpc\":\"2.0\",\"method\":\"aria2.addUri\",\"id\":1,\"params\":[[\"%s\"],{\"split\":\"5\",\"max-connection-per-server\":\"5\",\"seed-ratio\":\"1.0\"}]}]"
DMDOWNLOAD_RESULT ari2ac_add_task(char *url,char *task_id)
{
    if(url == NULL||task_id == NULL)
    {
        return DMDOWNLOAD_PARA_ERROR;
    }
    //TODO
    
	int ret=DMDOWNLOAD_SUCCES;
    char *result=NULL;
	//int result_len;
	int post_data_len=strlen(ADD_DOWNLOAD_URL)+strlen(url);
	char *post_data=(char *)malloc(post_data_len+1);
	assert (post_data != NULL);
	sprintf(post_data,ADD_DOWNLOAD_URL,url);
	printf("post_data=%s\n",post_data);
    ret = aPost(URI,post_data,POST_TIME_OUT,&result);
	if(ret != 0)
	{
		ret = DMDOWNLOAD_ADD_TASK_ERROR;
		safe_free(post_data);
		return ret;
	}
	//parse result
	if(result){
		//success
		struct json_object* new_obj=json_tokener_parse(result);
		assert (new_obj != NULL);
		//json_object_to_json_string(new_obj));
		struct json_object* new_obj_first=json_object_array_get_idx(new_obj,0);
		assert (new_obj_first != NULL);
		struct json_object* json_taskid=json_object_object_get(new_obj_first,"result");
		if (json_taskid != NULL)
		{
			strcpy(task_id,json_object_get_string(json_taskid));
			DMCLOG_D("task_id=%s",task_id);
		}
		json_object_put(new_obj);	
		safe_free(result);
	}
	safe_free(post_data);
    return DMDOWNLOAD_SUCCES;
}

/**
 *  功能：控制下载任务
 *  权限：开放权限
 *
 *  @param ctrl 输入 0 :下载任务，1:暂停，2:等待，3:删除
 *         task_id 输入 任务唯一标识符
 *
 *  @return 0为成功，非0为异常。
 */
#define START_TASK "{\"jsonrpc\":\"2.0\",\"method\":\"aria2.unpause\",\"id\":1,\"params\":[\"%s\"]}"
#define REMOVE_TASK "[{\"jsonrpc\":\"2.0\",\"method\":\"aria2.removeDownloadResult\",\"id\":1,\"params\":[\"%s\"]}]"
#define PAUSE_TASK "{\"jsonrpc\":\"2.0\",\"method\":\"aria2.pause\",\"id\":1,\"params\":[\"%s\"]}"

DMDOWNLOAD_RESULT ari2ac_ctr_task(int ctrl,char *task_id)
{
    if(task_id == NULL)
    {
        return DMDOWNLOAD_PARA_ERROR;
    }
    //TODO
    int ret=DMDOWNLOAD_SUCCES;

	char *result=NULL;
	char *post_data=NULL;
	switch(ctrl){
		case 0:
			post_data=(char *)calloc(1,strlen(START_TASK) + strlen(task_id) + 1);
			assert (post_data != NULL);
			sprintf(post_data,START_TASK,task_id);				
			break;
        case 1:
            post_data=(char *)calloc(1,strlen(PAUSE_TASK) + strlen(task_id) + 1);
            assert (post_data != NULL);
            sprintf(post_data,PAUSE_TASK,task_id);
            break;
        case 2://waiting
//            post_data=(char *)malloc(strlen(PAUSE_TASK)+strlen(task_id));
//            assert (post_data != NULL);
//            sprintf(post_data,PAUSE_TASK,task_id);
            break;
		case 3:
			post_data=(char *)calloc(1,strlen(REMOVE_TASK) + strlen(task_id) + 1);
			assert (post_data != NULL);
			sprintf(post_data,REMOVE_TASK,task_id);				
			break;
		default:
			return DMDOWNLOAD_PARA_ERROR;
	}
	
	DMCLOG_D("post=%s",post_data);
	ret = aPost(URI,post_data,POST_TIME_OUT,&result);
	if(ret != 0)
	{
		ret = DMDOWNLOAD_CTRL_TASK_ERROR;
		safe_free(post_data);
		return ret;
	}
	if(result){
		NEW_JSON_OBJ new_obj=json_tokener_parse(result);
			assert (new_obj != NULL);
		NEW_JSON_OBJ error_obj=json_object_object_get(new_obj,"error");
		if(error_obj != NULL){
			//json_object_put(new_obj);
			switch(ctrl){
				case 0:
					ret=DMDOWNLOAD_START_TASK_ERROR;							
					break;
				case 1:
					ret=DMDOWNLOAD_REMOVE_TASK_ERROR;
					break;
				case 2:
					ret=DMDOWNLOAD_PAUSE_TASK_ERROR;	
					break;					
				default:
					break;
			}
		}else {
			//json_object_put(new_obj);
			//return DMDOWNLOAD_SUCCES;
		}
		safe_free(result);
	}

	safe_free(post_data);
    return ret;
}

/**
 *  功能：控制所有下载任务
 *  权限：开放权限
 *
 *  @param ctrl 输入 0 :下载任务，1:暂停，2:等待，3:删除
 *         task_id 输入 任务唯一标识符
 *
 *  @return 0为成功，非0为异常。
 */

#define START_ALL_TASK "{\"jsonrpc\":\"2.0\",\"method\":\"aria2.unpauseAll\",\"id\":1,\"params\":[]}"
//#define REMOVE_ALL_TASK
#define PAUSE_ALL_TASK "{\"jsonrpc\":\"2.0\",\"method\":\"aria2.pauseAll\",\"id\":1,\"params\":[]}"

DMDOWNLOAD_RESULT ari2ac_ctr_all_task(int ctrl)
{
	char *result=NULL;
	char *post_data=NULL;
	int ret=DMDOWNLOAD_SUCCES;
	switch(ctrl){
		case 0:
			post_data=(char *)calloc(1,strlen(START_ALL_TASK) + 1);
			assert (post_data != NULL);
			sprintf(post_data,START_ALL_TASK);				
			break;
		case 1:
			post_data=(char *)calloc(1,strlen(PAUSE_ALL_TASK) + 1);
			assert (post_data != NULL);
			sprintf(post_data,PAUSE_ALL_TASK);			
			break;
        case 2:
            //todo
            DMCLOG_D("NOT DONE");
            break;

		default:
			return DMDOWNLOAD_PARA_ERROR;	
			
	}
	
	DMCLOG_D("post=%s",post_data);
	ret = aPost(URI,post_data,POST_TIME_OUT,&result);
	if(ret != 0)
	{
		ret = DMDOWNLOAD_CTRL_TASK_ERROR;
		safe_free(post_data);
		return ret;
	}
	if(result){
		NEW_JSON_OBJ new_obj=json_tokener_parse(result);
			assert (new_obj != NULL);
		NEW_JSON_OBJ error_obj=json_object_object_get(new_obj,"error");
		if(error_obj != NULL){
			//json_object_put(new_obj);
			switch(ctrl){
				case 0:
					ret=DMDOWNLOAD_START_TASK_ERROR;							
					break;
				case 1:
					ret=DMDOWNLOAD_REMOVE_TASK_ERROR;
					break;
				case 2:
					ret=DMDOWNLOAD_REMOVE_TASK_ERROR;	
					break;					
				default:
					break;
			}
		}
		json_object_put(new_obj);
		safe_free(result);
	}	
	safe_free(post_data);
    return ret;
}

/**
 * func:contrl global speed follow by para
 *
 *  @param dGlobalStatus->download_speed >= 0:set download speed,dGlobalStatus->download_speed < 0:no need to set download speed
 *			dGlobalStatus->upload_speed >= 0:set upload speed,dGlobalStatus->uplload_speed < 0:no need to set upload speed
 *  @return 0为成功，非0为异常。
 */
//{"jsonrpc":"2.0","method":"aria2.changeGlobalOption","id":1,"params":[{"max-overall-download-limit":"500","max-overall-upload-limit":"400","max-concurrent-downloads":"16","min-split-size":"1 MiB","user-agent":"uTorrent/2210(25130)","dir":"/tmp/mnt/SD-disk-1/Download"}]}:
#define SET_LIMIT_SPEED "{\"jsonrpc\":\"2.0\",\"method\":\"aria2.changeGlobalOption\",\"id\":1,\"params\":[{\"max-overall-download-limit\":\"%dK\",\"max-overall-upload-limit\":\"%dK\"}]}"

DMDOWNLOAD_RESULT ari2ac_ctr_speed(DownloadGlobalStatus *dGlobalStatus)
{
	ENTER_FUNC();
	if(dGlobalStatus == NULL)
	{
		return DMDOWNLOAD_PARA_ERROR;
	}
	DMCLOG_D("download speed = %d,upload_speed = %d",dGlobalStatus->download_speed,dGlobalStatus->upload_speed);
	char *result=NULL;
	char *post_data=(char *)malloc(strlen(SET_LIMIT_SPEED)+sizeof(int)+sizeof(int)+1);
	assert (post_data != NULL);
	sprintf(post_data,SET_LIMIT_SPEED,dGlobalStatus->download_speed,dGlobalStatus->upload_speed);
	DMCLOG_D("post=%s",post_data);
	int ret = aPost(URI,post_data,POST_TIME_OUT,&result);
	if(ret != 0)
	{
		ret= DMDOWNLOAD_QUERY_GLOBAL_ERROR;
		safe_free(post_data);
	    return ret;
	}
	if(result){
		NEW_JSON_OBJ new_obj=json_tokener_parse(result);
			assert (new_obj != NULL);
		NEW_JSON_OBJ error_obj=json_object_object_get(new_obj,"error");
		if(error_obj != NULL){
			ret=DMDOWNLOAD_REMOVE_TASK_ERROR;	
		}
		json_object_put(new_obj);
		safe_free(result);
	}	
	safe_free(post_data);
	
	EXIT_FUNC();
    return ret;
}

/**
 *  功能：获取下载任务列表
 *  权限：开放权限
 *
 *  @param status 0 :下载中的任务，1:暂停中的任务，2:等待中的任务，3:已完成的任务，4下载失败的任务
 *          head 链表首指针
 *  @return 0为成功，非0为异常。
 */
#define QUERY_ACTIVE_TASK "{\"jsonrpc\":\"2.0\",\"method\":\"aria2.tellActive\",\"id\":1,\"params\":[[\"gid\",\"totalLength\",\"completedLength\",\"dir\",\"downloadSpeed\",\"uploadSpeed\",\"status\",\"uploadLength\",\"files\"]]}"
#define QUERY_WAITING_TASK "{\"jsonrpc\":\"2.0\",\"method\":\"aria2.tellWaiting\",\"id\":1,\"params\":[0,100,[\"gid\",\"totalLength\",\"completedLength\",\"dir\",\"downloadSpeed\",\"uploadSpeed\",\"status\",\"uploadLength\",\"files\"]]}"
#define QUERY_STOPPED_TASK "{\"jsonrpc\":\"2.0\",\"method\":\"aria2.tellStopped\",\"id\":1,\"params\":[0,100,[\"gid\",\"totalLength\",\"completedLength\",\"dir\",\"downloadSpeed\",\"uploadSpeed\",\"status\",\"uploadLength\",\"files\"]]}"

int parseResult(struct dl_list *head,NEW_JSON_OBJ task_obj,char *get_status, int isArray){
	
	NEW_JSON_OBJ status_obj=json_object_object_get(task_obj,"status");
		assert (status_obj != NULL);		
	char status[16]={0};
	strcpy(status,json_object_get_string(status_obj));
	if(((isArray==1)&&(!strcmp(status,get_status)))||isArray==0)
	{
		MDownloadTask *fdi = (MDownloadTask *)calloc(1,sizeof(MDownloadTask));
		dl_list_add_tail(head, &fdi->node);
		//get task_id
		NEW_JSON_OBJ gid_obj=json_object_object_get(task_obj,"gid");
			assert (gid_obj != NULL);			
		strcpy(fdi->task_id,json_object_get_string(gid_obj));
		//get status
		if(!strcmp(status,"active")){
			fdi->status = 0;
		}else if(!strcmp(status,"paused")){
			fdi->status = 1;
		}
		else if(!strcmp(status,"waiting")){
			fdi->status = 2;
		}
		else if(!strcmp(status,"complete")){
			fdi->status = 3;
		}
		else if(!strcmp(status,"error")){
			fdi->status = 4;
		}

		//get download_speed
		NEW_JSON_OBJ download_obj=json_object_object_get(task_obj,"downloadSpeed");	
		if(download_obj != NULL)
			fdi->download_speed=atoi(json_object_get_string(download_obj));

		//get dir
		NEW_JSON_OBJ dir_obj=json_object_object_get(task_obj,"dir");
		if (dir_obj != NULL)
		{
			char *dir = json_object_get_string(dir_obj);
			fdi->dir = (char*) calloc(1,strlen(dir) + 1);
			strcpy(fdi->dir,dir);
		}
		
		//get download path
		NEW_JSON_OBJ files_array=json_object_object_get(task_obj,"files");
		NEW_JSON_OBJ file_obj=json_object_array_get_idx(files_array,0);
		if(file_obj != NULL)
		{
			NEW_JSON_OBJ path_obj = json_object_object_get(file_obj,"path");
			if(path_obj != NULL)
			{
				char *path = json_object_get_string(path_obj);
				if(path != NULL&&*path)
				{
					fdi->download_path = (char*)calloc(1,strlen(path) + 1);
					strcpy(fdi->download_path,path);
				}
			}
			
			//get uris
			NEW_JSON_OBJ uris_array = json_object_object_get(file_obj,"uris");
			if(uris_array != NULL)
			{
				NEW_JSON_OBJ uri_obj = json_object_array_get_idx(uris_array,0);
				if(uri_obj != NULL)
				{
					NEW_JSON_OBJ p_uri_obj = json_object_object_get(uri_obj,"uri");
					if(p_uri_obj != NULL)
					{
						char *uri = json_object_get_string(p_uri_obj);
						if(uri != NULL)
						{
							DMCLOG_D("uri = %s",uri);
							fdi->uri = (char*)calloc(1,strlen(uri) + 1);
							strcpy(fdi->uri,uri);
						}
					}
				}
			}
		}
		
		//get completeLength
		NEW_JSON_OBJ completedLength=json_object_object_get(task_obj,"completedLength");
			assert (completedLength != NULL);			
		fdi->completed_length = atoll(json_object_get_string(completedLength));
		DMCLOG_D("fdi->completed_length = %lld",fdi->completed_length);

		//get Length
		NEW_JSON_OBJ length_obj=json_object_object_get(task_obj,"totalLength");
			assert (length_obj != NULL);			
		fdi->length = atoll(json_object_get_string(length_obj));
		DMCLOG_D("fdi->length = %lld",fdi->length);
		//get uploadLength
		NEW_JSON_OBJ uploadLength_obj=json_object_object_get(task_obj,"uploadLength");
			assert (uploadLength_obj != NULL);			
		fdi->upload_length=atoll(json_object_get_string(uploadLength_obj));
		//get uploadSpeed
		NEW_JSON_OBJ uploadSpeed_obj=json_object_object_get(task_obj,"uploadSpeed");
			assert (uploadSpeed_obj != NULL);			
		fdi->upload_speed = atoi(json_object_get_string(uploadSpeed_obj));
		
	}
	return DMDOWNLOAD_SUCCES;
}

int parseResultArray(struct dl_list *head,char *result,char *get_status){

	int ret=DMDOWNLOAD_SUCCES;
	NEW_JSON_OBJ new_obj=json_tokener_parse(result);
		assert (new_obj != NULL);
	NEW_JSON_OBJ error_obj=json_object_object_get(new_obj,"error");
	if(error_obj != NULL){
		return DMDOWNLOAD_QUERY_TASK_ERROR;
	}
	int i=0;	
	//if(status==0){//active
		struct json_object* result_obj=json_object_object_get(new_obj,"result");
		assert (result_obj != NULL);	
		int array_len=json_object_array_length(result_obj);
		//DMCLOG_D("array_len=%d",array_len);
		for( i=0;i<array_len;i++){
			NEW_JSON_OBJ task_obj=json_object_array_get_idx(result_obj,i);
			assert (task_obj != NULL);		
			//DMCLOG_D("i=%d",i);
			ret=parseResult(head,task_obj,get_status,1);
		}
		json_object_put(new_obj);
		return ret;

	//}
return DMDOWNLOAD_SUCCES;

}

		
DMDOWNLOAD_RESULT ari2ac_query_task_by_status(int status,struct dl_list *head)
{
    if(head == NULL)
    {
        return DMDOWNLOAD_PARA_ERROR;
    }

	char *result = NULL;
	char *post_data = NULL;
	int ret = DMDOWNLOAD_SUCCES;
	switch(status){
		case 0://active
			post_data=(char *)malloc(strlen(QUERY_ACTIVE_TASK)+1);
			assert (post_data != NULL);
			sprintf(post_data,"%s",QUERY_ACTIVE_TASK);				
			break;	
		case 1://pause
		case 2://waiting
			post_data=(char *)malloc(strlen(QUERY_WAITING_TASK)+1);
			assert (post_data != NULL);			
			sprintf(post_data,"%s",QUERY_WAITING_TASK);			
			break;			
		default://
			post_data=(char *)malloc(strlen(QUERY_STOPPED_TASK)+1);
			assert (post_data != NULL);
			sprintf(post_data,"%s",QUERY_STOPPED_TASK);	
			break;
	}
	
	DMCLOG_D("post=%s",post_data);
	ret = aPost(URI,post_data,POST_TIME_OUT,&result);
	if(ret != 0)
	{
		safe_free(post_data);
		return DMDOWNLOAD_QUERY_TASK_ERROR;
	}
	if(result){
		if(status==0){
			ret = parseResultArray(head,result,"active");
				
		}else if(status==1){//paused
			ret =  parseResultArray(head,result,"paused");

		}else if(status==2){//waiting
			ret = parseResultArray(head,result,"waiting");

		}else if(status==3){//complete
			ret = parseResultArray(head,result,"complete");

		}else if(status==4){//error
			ret = parseResultArray(head,result,"error");
		}
		safe_free(result);
	}
	safe_free(post_data);
	EXIT_FUNC();
    return ret;
}
#define QUERY_BY_GID "{\"jsonrpc\":\"2.0\",\"method\":\"aria2.tellStatus\",\"id\":1,\"params\":[\"%s\",[\"gid\",\"totalLength\",\"completedLength\",\"dir\",\"downloadSpeed\",\"uploadSpeed\",\"status\",\"uploadLength\",\"files\"]]}"
DMDOWNLOAD_RESULT ari2ac_query_task_by_gid(char *gid,struct dl_list *head)
{
    if(gid==NULL)
    {
        return DMDOWNLOAD_PARA_ERROR;
    }
    //DownloadTask *fdi = (DownloadTask *)calloc(1,sizeof(DownloadTask));
    int ret=DMDOWNLOAD_SUCCES;
    //TODO
	char *result=NULL;
	char *post_data=(char *)malloc(strlen(QUERY_BY_GID)+strlen(gid)+1);
	assert (post_data != NULL);
	sprintf(post_data,QUERY_BY_GID,gid);
	DMCLOG_D("post=%s",post_data);
	ret = aPost(URI,post_data,POST_TIME_OUT,&result);
	if(ret != 0)
	{
		ret= DMDOWNLOAD_QUERY_TASK_ERROR;
		goto exit;
	}
	if(result){
		//success
		NEW_JSON_OBJ new_obj=json_tokener_parse(result);
			assert (new_obj != NULL);
		NEW_JSON_OBJ error_obj=json_object_object_get(new_obj,"error");
		if(error_obj != NULL){
			ret= DMDOWNLOAD_QUERY_TASK_ERROR;
			goto exit;
		}
		int i=0;	
	//if(status==0){//active
		struct json_object* result_obj=json_object_object_get(new_obj,"result");
		return parseResult(head,result_obj,NULL,0);
	}
exit:
	safe_free(post_data);
	safe_free(result);
    return DMDOWNLOAD_SUCCES;
}

/**
 *  功能：获取下载目录列表
 *  权限：开放权限
 *
 *  @param head 链表首指针
 *
 *  @return 0为成功，非0为异常。
 */
#define DOWNLOAD_DIR "/USB-disk-1/.exsystem/Downloads"

int get_download_path(char *file_path, char *download_path)
{
	char tmp[512]="\0";

	if((file_path == NULL))
	{
		DMCLOG_D("argument invalid \n", file_path);
	}
	
	FILE *fp=fopen(file_path, "r");
	if(NULL == fp)
	{
		DMCLOG_D("open %s failed \n", file_path);
		return -1;
	}
	
	bzero(tmp,512);
	if(fgets(tmp,512,fp)!=NULL)
	{
		if('\n'==tmp[strlen(tmp)-1])
		{
			tmp[strlen(tmp)-1]=0;
		}
		//p_debug("get string from %s:%s\n", file_path, tmp);
		
		//bzero(tmp, 512);
	}
	strcpy(download_path,tmp);
	DMCLOG_D("download_path= %s", download_path);	
	fclose(fp);
	return 1;
}


DMDOWNLOAD_RESULT ari2ac_query_dir(struct dl_list *head)
{
    if(head == NULL)
    {
        return DMDOWNLOAD_PARA_ERROR;
    }
    DownloadDir *fdi = (DownloadDir *)calloc(1,sizeof(DownloadDir));

	fdi->dir_path =(char *)malloc(512);	
	get_download_path("/tmp/download_dir",fdi->dir_path);
	
    dl_list_add_tail(head, &fdi->node);
    //TODO
    return DMDOWNLOAD_SUCCES;
}
/**
 *  功能：获取下载模块全局状况
 *  权限：开放权限
 *
 *  @param dGlobalStatus 输出
 *
 *  @return 0为成功，非0为异常。
 */

//{"jsonrpc":"2.0","method":"aria2.getGlobalStat","id":1,"params":[]}
//#define URI "http://127.0.0.1:6800/jsonrpc?tm=1467437057221"
#define QUERY_GLOBAL "{\"jsonrpc\":\"2.0\",\"method\":\"aria2.getGlobalStat\",\"id\":1,\"params\":[]}"

DMDOWNLOAD_RESULT ari2ac_query_global(DownloadGlobalStatus *dGlobalStatus)
{
    if(dGlobalStatus == NULL)
    {
        return DMDOWNLOAD_PARA_ERROR;
    }
	int ret = DMDOWNLOAD_SUCCES;
	char *result=NULL;
	char *post_data=(char *)malloc(strlen(QUERY_GLOBAL)+1);
	assert (post_data != NULL);
	sprintf(post_data,"%s",QUERY_GLOBAL);
	DMCLOG_D("post=%s",post_data);
	ret = aPost(URI,post_data,POST_TIME_OUT,&result);
	if(ret != 0)
	{
		ret= DMDOWNLOAD_QUERY_GLOBAL_ERROR;
		safe_free(post_data);
	    return ret;
	}
	if(result){
		//success
		NEW_JSON_OBJ new_obj=json_tokener_parse(result);
			assert (new_obj != NULL);

		NEW_JSON_OBJ json_result=json_object_object_get(new_obj,"result");
			assert (json_result != NULL);		

		NEW_JSON_OBJ json_downloadSpeed=json_object_object_get(json_result,"downloadSpeed");
			assert (json_downloadSpeed != NULL);		
			
		NEW_JSON_OBJ json_uploadSpeed=json_object_object_get(json_result,"uploadSpeed");
			assert (json_uploadSpeed != NULL);		
		
		NEW_JSON_OBJ json_numActive=json_object_object_get(json_result,"numActive");
			assert (json_numActive != NULL);

		NEW_JSON_OBJ json_numStopped=json_object_object_get(json_result,"numStopped");
			assert (json_numStopped != NULL);		
			
		NEW_JSON_OBJ json_numWaiting=json_object_object_get(json_result,"numWaiting");
			assert (json_numWaiting != NULL);	
			
		dGlobalStatus->download_speed=atoi(json_object_get_string(json_downloadSpeed));
		dGlobalStatus->upload_speed=atoi(json_object_get_string(json_uploadSpeed));
		dGlobalStatus->numActive=atoi(json_object_get_string(json_numActive));
		dGlobalStatus->numStopped=atoi(json_object_get_string(json_numStopped));
		dGlobalStatus->numWaiting=atoi(json_object_get_string(json_numWaiting));				
		
		json_object_put(new_obj);
		safe_free(result);
	}
	safe_free(post_data);
    return DMDOWNLOAD_SUCCES;
}
#if 0
void main(int argc, char *argv[]){

	char taskid[32]={0};
	if(ari2ac_add_task("https://downloads.openwrt.org/chaos_calmer/15.05.1/adm5120/rb1xx/OpenWrt-ImageBuilder-15.05.1-adm5120-rb1xx.Linux-x86_64.tar.bz2",taskid)!=0){
		printf("ari2ac_add_task failed.\n");
	}else {
		printf("taskid=%s\n",taskid);
	};
	DownloadGlobalStatus *dGlobalStatus=(DownloadGlobalStatus *)malloc(sizeof(DownloadGlobalStatus));
	if(ari2ac_query_global(dGlobalStatus)!=0){
		printf("ari2ac_query_global failed.\n");
	}else {
		DMCLOG_D("dGlobalStatus->download_speed=%d",dGlobalStatus->download_speed);
		DMCLOG_D("dGlobalStatus->upload_speed=%d",dGlobalStatus->upload_speed);
		DMCLOG_D("dGlobalStatus->numActive=%d",dGlobalStatus->numActive);
		DMCLOG_D("dGlobalStatus->numStopped=%d",dGlobalStatus->numStopped);
		DMCLOG_D("dGlobalStatus->numWaiting=%d",dGlobalStatus->numWaiting);		
		//printf("taskid=%s\n",taskid);
	};
	safe_free(dGlobalStatus);

	MDownloadTask *task=(MDownloadTask*)malloc(sizeof(MDownloadTask));
	if(ari2ac_query_task_by_gid("1f93a5245838fd93",task)!=0)
		{
		printf("ari2ac_query_task_by_gid failed.\n");
	}else {
		
	}
	safe_free(task);
	
	struct dl_list *head=(struct dl_list *)malloc(sizeof(struct dl_list));
	dl_list_init(head);
//	if(ari2ac_query_task_by_status(2,head)!=0)
		{
		printf("ari2ac_query_task_by_status failed.\n");
	//}else {
		
	}
	

//	ari2ac_ctr_task(1,"56b34404c95e9634");
		
//	ari2ac_ctr_all_task(2);
//	ari2ac_ctr_task(1,"adcf521b04cdbf7c");

//	ari2ac_ctr_task(0,"4af5de5dcde29035");


	//DownloadTask *task=(DownloadTask*)malloc(sizeof(DownloadTask));

	ari2ac_query_task_by_gid("af34f1236ec09e74",head);
	ari2ac_query_task_by_status(0,head);	
	//safe_free(task);	

	safe_free(head);	
}
#endif
