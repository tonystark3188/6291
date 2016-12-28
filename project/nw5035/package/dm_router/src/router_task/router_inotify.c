/*
 * =============================================================================
 *
 *       Filename:  router_inotify.c
 *
 *    Description:  monitor the router hardware parameter changed
 *
 *        Version:  1.0
 *        Created:  2015/08/20 14:33
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver, 
 *   Organization:  
 *
 * =============================================================================*/
#include <stdio.h>//printf  
#include <string.h> //strcmp  
#include <unistd.h>//close  
#include "router_inotify.h"
#include "router_task.h"
#include "msg_server.h"
#include "router_defs.h"
#include "format.h"
#include "socket_uart.h"
#include "router_cycle.h"
#include "defs.h"
#include "search_task.h"

     
#define ERR_EXIT(msg,flag)  {perror(msg);goto flag;}  
extern int exit_flag;
extern inotify_power_info_t power_info_global;
#ifdef DM_CYCLE_DISK
extern int disk_status;//0:on cp;1:on board
#endif
struct hardware_info p_hardware_info;
int notify_disk_scanning_send_buf(ClientTheadInfo *p_client_info)
{
    int res_sz = 0;
	int cmd = 6;
	int seq = 0;
	int error = 0;
    JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	char *response_str = JSON_TO_STRING(response_json);
	if(response_str == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}
	res_sz = strlen(response_str);
	p_client_info->retstr = (char*)malloc(res_sz + 1);
	if(p_client_info->retstr == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}	
	strcpy(p_client_info->retstr,response_str);
    DMCLOG_D("send_buf = %s",p_client_info->retstr);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

char *notify_disk_scanning(int release_flag)
{
    JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* para_info = JSON_NEW_EMPTY_OBJECT();

	JSON_ADD_OBJECT(para_info, "release_flag",JSON_NEW_OBJECT(release_flag,int));
	JSON_ADD_OBJECT(para_info, "event",JSON_NEW_OBJECT(0,int));
	JSON_ADD_OBJECT(para_info, "action_node",JSON_NEW_OBJECT("null",string));
	JSON_ARRAY_ADD_OBJECT (response_data_array,para_info);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	char *response_str = JSON_TO_STRING(response_json);
	if(response_str == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}
	int res_sz = strlen(response_str);
	char *send_buf = (char*)calloc(1,res_sz + 1);
	if(send_buf == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}	
	strcpy(send_buf,response_str);
    DMCLOG_D("send_buf = %s",send_buf);
	JSON_PUT_OBJECT(response_json);
	return send_buf;
}

int parser_scan_notify_json(char *recv_buf, int *p_release_flag, int *p_event, char *p_action_node)
{
	int ret = 0;
	if(recv_buf == NULL)
		return -1;
	JObj *r_json = JSON_PARSE(recv_buf);
	JObj *data_json = JSON_GET_OBJECT(r_json,"data");
	if(data_json == NULL)
	{
		ret = -1;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		ret = -1;
		goto EXIT;
	}
	JObj *statusFlag_json = JSON_GET_OBJECT(para_json,"release_flag");
	JObj *event_json = JSON_GET_OBJECT(para_json,"event");
	JObj *action_node_json = JSON_GET_OBJECT(para_json,"action_node");
	if(statusFlag_json == NULL || event_json == NULL || action_node_json == NULL){
		ret = -1;
		goto EXIT;
	}
	*p_release_flag = JSON_GET_OBJECT_VALUE(statusFlag_json,int);
	*p_event = JSON_GET_OBJECT_VALUE(event_json,int);
	strcpy(p_action_node, JSON_GET_OBJECT_VALUE(action_node_json,string));
	DMCLOG_D("release_flag: %d, event: %d, action_node: %s", *p_release_flag, *p_event, p_action_node);
EXIT:
	if(r_json != NULL)
		JSON_PUT_OBJECT(r_json);
	return ret;
}
char *comb_release_disk_json(int status)
{
	int cmd = 7;
	int ver = 0;
	int seq = 0;
	int error = 0;
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* status_json = JSON_NEW_EMPTY_OBJECT();
	DMCLOG_D("status = %d", status);
	JSON_ADD_OBJECT(status_json, "status",JSON_NEW_OBJECT(status,int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,status_json);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	char *response_str = JSON_TO_STRING(response_json);
	if(response_str == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}
	int res_sz = strlen(response_str);
	char *send_buf = (char*)calloc(1,res_sz + 1);
	if(send_buf == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}	
	strcpy(send_buf,response_str);
    DMCLOG_D("send_buf = %s",send_buf);
	JSON_PUT_OBJECT(response_json);
	return send_buf;
}

char *notify_comb_power_send_buf(power_info_t *power_info, int status, int statusCode)
{
	char *send_buf = NULL;
    int res_sz = 0;
	int cmd = 18;//FN_ROUTER_NOTIFY_STATUS_CHANGED;
	int seq = 0;
	int error = 0;
    JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj* response_data_array = JSON_NEW_ARRAY();
	JObj* para_info = JSON_NEW_EMPTY_OBJECT();
	JObj* status_info = JSON_NEW_EMPTY_OBJECT();
	//JObj* status_info_array = JSON_NEW_ARRAY();

	JSON_ADD_OBJECT(para_info, "status",JSON_NEW_OBJECT(status,int));
	JSON_ADD_OBJECT(para_info, "statusCode",JSON_NEW_OBJECT(statusCode,int));
	JSON_ADD_OBJECT(status_info, "power",JSON_NEW_OBJECT(power_info->power,int));
	JSON_ADD_OBJECT(status_info, "power_status",JSON_NEW_OBJECT(power_info->power_status,int));
	//JSON_ARRAY_ADD_OBJECT(status_info_array, status_info);
	//JSON_ADD_OBJECT(para_info, "statusInfo", status_info_array);
	JSON_ADD_OBJECT(para_info, "statusInfo", status_info);
	JSON_ARRAY_ADD_OBJECT (response_data_array,para_info);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	char *response_str = JSON_TO_STRING(response_json);
	if(response_str == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}
	res_sz = strlen(response_str);
	send_buf = (char*)calloc(1,res_sz + 1);
	if(send_buf == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}	
	strcpy(send_buf,response_str);
	JSON_PUT_OBJECT(response_json);
	return send_buf;
}

char * notify_comb_disk_send_buf(all_disk_t *alldisk, int status, int statusCode)
{
	char *send_buf = NULL;
    int res_sz = 0;
	int cmd = 18;//FN_ROUTER_NOTIFY_STATUS_CHANGED;
	int seq = 0;
	int error = 0;
	int i = 0;
    JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* para_info = JSON_NEW_EMPTY_OBJECT();
	JObj *disk_info_array = JSON_NEW_ARRAY();
	JObj* disk_info_json = JSON_NEW_EMPTY_OBJECT();
	
	JSON_ADD_OBJECT(para_info, "status",JSON_NEW_OBJECT(status,int));
	JSON_ADD_OBJECT(para_info, "statusCode",JSON_NEW_OBJECT(statusCode,int));

	for(i = 0;i < alldisk->count;i++)
	{
		JObj* drive_info = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(drive_info, "displayName",JSON_NEW_OBJECT(alldisk->disk[i].name,string));
		JSON_ADD_OBJECT(drive_info, "path",JSON_NEW_OBJECT(alldisk->disk[i].path,string));
		JSON_ADD_OBJECT(drive_info, "size",JSON_NEW_OBJECT(alldisk->disk[i].total_size,int64));
		JSON_ADD_OBJECT(drive_info, "avSize",JSON_NEW_OBJECT(alldisk->disk[i].free_size,int64));
		JSON_ADD_OBJECT(drive_info, "diskType",JSON_NEW_OBJECT(alldisk->disk[i].type,int));
		JSON_ADD_OBJECT(drive_info, "auth",JSON_NEW_OBJECT(alldisk->disk[i].auth,int));
		JSON_ADD_OBJECT(drive_info, "fsType",JSON_NEW_OBJECT(alldisk->disk[i].fstype,string));
		JSON_ARRAY_ADD_OBJECT (disk_info_array,drive_info);	
	}
	JSON_ADD_OBJECT(disk_info_json, "diskDir",JSON_NEW_OBJECT(alldisk->storage_dir, int));
	JSON_ADD_OBJECT(disk_info_json, "diskInfoList", disk_info_array);
	JSON_ADD_OBJECT(para_info, "statusInfo", disk_info_json);
	
	JSON_ARRAY_ADD_OBJECT (response_data_array,para_info);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	char *response_str = JSON_TO_STRING(response_json);
	if(response_str == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}
	res_sz = strlen(response_str);
	send_buf = (char*)malloc(res_sz + 1);
	if(send_buf == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}	
	strcpy(send_buf,response_str);
    DMCLOG_D("send_buf = %s",send_buf);
	JSON_PUT_OBJECT(response_json);
	return send_buf;
}

char * notify_comb_ssid_send_buf(int status,int statusCode)
{
	char *send_buf = NULL;
    int res_sz = 0;
	int cmd = 18;//FN_ROUTER_NOTIFY_STATUS_CHANGED;
	int seq = 0;
	int error = 0;
    JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* para_info = JSON_NEW_EMPTY_OBJECT();

	JSON_ADD_OBJECT(para_info, "status",JSON_NEW_OBJECT(status,int));
	JSON_ADD_OBJECT(para_info, "statusCode",JSON_NEW_OBJECT(statusCode,int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,para_info);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	char *response_str = JSON_TO_STRING(response_json);
	if(response_str == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}
	res_sz = strlen(response_str);
	send_buf = (char*)malloc(res_sz + 1);
	if(send_buf == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}	
	strcpy(send_buf,response_str);
    DMCLOG_D("send_buf = %s",send_buf);
	JSON_PUT_OBJECT(response_json);
	return send_buf;
}

char * notify_comb_db_send_buf(int status,int statusCode,unsigned data_base_seq)
{
	char *send_buf = NULL;
    int res_sz = 0;
	int cmd = 18;//FN_ROUTER_NOTIFY_STATUS_CHANGED;
	int seq = 0;
	int error = 0;
    JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* para_info = JSON_NEW_EMPTY_OBJECT();

	JSON_ADD_OBJECT(para_info, "status",JSON_NEW_OBJECT(status,int));
	JSON_ADD_OBJECT(para_info, "statusCode",JSON_NEW_OBJECT(statusCode,int));
	JSON_ADD_OBJECT(para_info, "dataBaseSeq",JSON_NEW_OBJECT(data_base_seq,int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,para_info);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	char *response_str = JSON_TO_STRING(response_json);
	if(response_str == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}
	res_sz = strlen(response_str);
	send_buf = (char*)malloc(res_sz + 1);
	if(send_buf == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}	
	strcpy(send_buf,response_str);
    DMCLOG_D("send_buf = %s",send_buf);
	JSON_PUT_OBJECT(response_json);
	return send_buf;
}

char * notify_comb_pwd_send_buf(int status,int statusCode)
{
	char *send_buf = NULL;
    int res_sz = 0;
	int cmd = 18;//FN_ROUTER_NOTIFY_STATUS_CHANGED;
	int seq = 0;
	int error = 0;
    JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* para_info = JSON_NEW_EMPTY_OBJECT();

	JSON_ADD_OBJECT(para_info, "status",JSON_NEW_OBJECT(status,int));
	JSON_ADD_OBJECT(para_info, "statusCode",JSON_NEW_OBJECT(statusCode,int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,para_info);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	char *response_str = JSON_TO_STRING(response_json);
	if(response_str == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}
	res_sz = strlen(response_str);
	send_buf = (char*)malloc(res_sz + 1);
	if(send_buf == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}	
	strcpy(send_buf,response_str);
    DMCLOG_D("send_buf = %s",send_buf);
	JSON_PUT_OBJECT(response_json);
	return send_buf;
}


char *notify_scanning_send_buf(int status)
{
	char *send_buf = NULL;
    int res_sz = 0;
	int cmd = 122;//FN_FILE_GET_SCAN_STATUS;
	int seq = 0;
	int error = 0;
    JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* para_info = JSON_NEW_EMPTY_OBJECT();

	JSON_ADD_OBJECT(para_info, "status",JSON_NEW_OBJECT(status,int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,para_info);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	char *response_str = JSON_TO_STRING(response_json);
	if(response_str == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}
	res_sz = strlen(response_str);
	send_buf = (char*)malloc(res_sz + 1);
	if(send_buf == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}	
	strcpy(send_buf,response_str);
    DMCLOG_D("send_buf = %s",send_buf);
	JSON_PUT_OBJECT(response_json);
	return send_buf;
}


char *notify_copy_send_buf(copy_info_t *pInfo)
{
    char *send_buf = NULL;
    int res_sz = 0;
    int cmd = 112;//FN_FILE_COPY_INOTIFY
    int seq = 0;
    int error = 0;
        
    JObj* response_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj *response_data_array = JSON_NEW_ARRAY();
    JObj* para_info = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(para_info, "status",JSON_NEW_OBJECT(pInfo->status,int));
    JSON_ADD_OBJECT(para_info, "seq",JSON_NEW_OBJECT(pInfo->seq,int));
    if(pInfo->status == 1)
    {
    	if(pInfo->cur_name != NULL)
        	JSON_ADD_OBJECT(para_info, "curName",JSON_NEW_OBJECT(pInfo->cur_name,string));
		if(pInfo->cur_progress != 0)
        	JSON_ADD_OBJECT(para_info, "curProgress",JSON_NEW_OBJECT(pInfo->cur_progress,int64));
		if(pInfo->total_size != 0)
       		JSON_ADD_OBJECT(para_info, "totalSize",JSON_NEW_OBJECT(pInfo->total_size,int64));
        JSON_ADD_OBJECT(para_info, "curCount",JSON_NEW_OBJECT(pInfo->cur_nfiles,int));
        JSON_ADD_OBJECT(para_info, "fileCount",JSON_NEW_OBJECT(pInfo->nfiles,int));
    }
	
    JSON_ARRAY_ADD_OBJECT (response_data_array,para_info);
    JSON_ADD_OBJECT(response_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(cmd,int));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(pInfo->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(error,int));
    JSON_ADD_OBJECT(response_json, "header", header_json);
    char *response_str = JSON_TO_STRING(response_json);
    if(response_str == NULL)
    {
        JSON_PUT_OBJECT(response_json);
        return NULL;
    }
    res_sz = strlen(response_str);
    send_buf = (char*)calloc(1,res_sz + 1);
    if(send_buf == NULL)
    {
        JSON_PUT_OBJECT(response_json);
        return NULL;
    }
    strcpy(send_buf,response_str);
    DMCLOG_D("send_buf = %s",send_buf);
    JSON_PUT_OBJECT(response_json);
    return send_buf;
}

char *notify_del_send_buf(del_info_t *pInfo)
{
    JObj* response_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj *response_data_array = JSON_NEW_ARRAY();
    JObj* para_info = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(para_info, "status",JSON_NEW_OBJECT(pInfo->status,int));
	if(pInfo->cur_name != NULL)
    	JSON_ADD_OBJECT(para_info, "curName",JSON_NEW_OBJECT(pInfo->cur_name,string));

    JSON_ADD_OBJECT(para_info, "curCount",JSON_NEW_OBJECT(pInfo->cur_nfiles,int));//cur count
    JSON_ADD_OBJECT(para_info, "fileCount",JSON_NEW_OBJECT(pInfo->nfiles,int));//total count deflaut :0
	
    JSON_ARRAY_ADD_OBJECT (response_data_array,para_info);
    JSON_ADD_OBJECT(response_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(115,int));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(pInfo->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(pInfo->error,int));
    JSON_ADD_OBJECT(response_json, "header", header_json);
    char *response_str = JSON_TO_STRING(response_json);
    if(response_str == NULL)
    {
        JSON_PUT_OBJECT(response_json);
        return NULL;
    }

    char *send_buf = (char*)calloc(1,strlen(response_str) + 1);
    if(send_buf == NULL)
    {
        JSON_PUT_OBJECT(response_json);
        return NULL;
    }
    strcpy(send_buf,response_str);
    DMCLOG_D("send_buf = %s",send_buf);
    JSON_PUT_OBJECT(response_json);
    return send_buf;
}



char *notify_search_comb_send_buf(file_search_info_t *pInfo, file_list_t *plist)
{
    char *send_buf = NULL;
    int res_sz = 0;
    int cmd = 114;//FN_FILE_SEARCH_INOTIFY
    int seq = 0;
    int error = 0;
	file_info_t *item;

	DMCLOG_D("status: %d", pInfo->status);
    JObj* response_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj *response_data_array = JSON_NEW_ARRAY();
    JObj* para_info = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(para_info, "status",JSON_NEW_OBJECT(pInfo->status,int));
	JSON_ADD_OBJECT(para_info, "seq",JSON_NEW_OBJECT(pInfo->search_seq,int));
	if(pInfo->status == 1)
    {    	
		DMCLOG_D("statusCode: %d, curCount: %d, list_nfiles: %d", pInfo->statusCode, pInfo->cur_nfiles, pInfo->list_nfiles);
    	JSON_ADD_OBJECT(para_info, "statusCode",JSON_NEW_OBJECT(pInfo->statusCode,int));
    	JSON_ADD_OBJECT(para_info, "curCount",JSON_NEW_OBJECT(pInfo->cur_nfiles,int));
		JSON_ADD_OBJECT(para_info, "listCount",JSON_NEW_OBJECT(pInfo->list_nfiles,int));
		if(plist != NULL){
			JObj *file_info_array = JSON_NEW_ARRAY();
			dl_list_for_each(item, &plist->head, file_info_t, next)
			{
				JObj* file_info_json = JSON_NEW_EMPTY_OBJECT();
				if(item->path != NULL){
					//DMCLOG_D("file_path: %s", item->path);
					JSON_ADD_OBJECT(file_info_json, "filePath",JSON_NEW_OBJECT(item->path,string));
				}
				else{
					//DMCLOG_D("file_path is null");
					JSON_ADD_OBJECT(file_info_json, "filePath",JSON_NEW_OBJECT("",string));
				}
				//DMCLOG_D("isFolder:%d", item->isFolder);
				JSON_ADD_OBJECT(file_info_json, "isFolder",JSON_NEW_OBJECT(item->isFolder, int));				
				JSON_ADD_OBJECT(file_info_json, "attr",JSON_NEW_OBJECT(item->attr, boolean));
				JSON_ARRAY_ADD_OBJECT (file_info_array, file_info_json);
			}
			JSON_ADD_OBJECT(para_info, "fileList", file_info_array);
		}
	}
	
    JSON_ARRAY_ADD_OBJECT (response_data_array,para_info);
    JSON_ADD_OBJECT(response_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(cmd,int));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(error,int));
    JSON_ADD_OBJECT(response_json, "header", header_json);
    char *response_str = JSON_TO_STRING(response_json);
    if(response_str == NULL)
    {
        JSON_PUT_OBJECT(response_json);
        return NULL;
    }
    res_sz = strlen(response_str);
    send_buf = (char*)calloc(1,res_sz + 1);
    if(send_buf == NULL)
    {
        JSON_PUT_OBJECT(response_json);
        return NULL;
    }
    strcpy(send_buf,response_str);
    DMCLOG_D("send_buf = %s",send_buf);
    JSON_PUT_OBJECT(response_json);
    return send_buf;
}

int notify_search_parse_recv_buf(file_search_info_t *pInfo, char *recv_buf)
{
	int res = 0;
	int status = 0;
	JObj *r_json = JSON_PARSE(recv_buf);
    if(r_json == NULL)
    {
        DMCLOG_D("access NULL");
        res = -1;
        goto exit;
    }
    if(is_error(r_json))
    {
        DMCLOG_D("### error:post data is not a json string");
        res = -1;
        goto exit;
    }

	JObj *data_json = JSON_GET_OBJECT(r_json,"data");
    if(data_json == NULL)
    {
        DMCLOG_D("access NULL");
        res = -1;
        goto exit;
    }
    JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
    if(para_json == NULL)
    {
        DMCLOG_D("access NULL");
        res = -1;
        goto exit;
    }

	JObj *status_json = JSON_GET_OBJECT(para_json,"status");
    if(status_json  == NULL)
    {
        DMCLOG_D("access NULL");
        goto exit;
    }
    status = JSON_GET_OBJECT_VALUE(status_json,int);

	if(status == 1){
		JObj *statusCode_json = JSON_GET_OBJECT(para_json,"statusCode");
	    if(statusCode_json  == NULL)
	    {
	        DMCLOG_D("access NULL");
	        goto exit;
	    }
	    pInfo->recvStatusCode = JSON_GET_OBJECT_VALUE(statusCode_json,int);
	}

exit:
	if(r_json != NULL)
        JSON_PUT_OBJECT(r_json);
    return res;
}

/*
 * Desc: the entry function of handle client tcp request and response it.
 *       It just recv client request, handle the request, and response result to client.
 *
 */
int router_handle_client_request(char *send_buf,struct dev_dnode *dn)
{
	ENTER_FUNC();
    int enRet;
	int client_fd;
	char *rcv_buf = NULL;
	int seq = 0;
	struct sockaddr_in clientAddr;
/*
 * Create a unix domain socket.
*/
	client_fd = DM_UdpClientInit(PF_INET, dn->port, SOCK_DGRAM,dn->ip,&clientAddr);
	if(client_fd <= 0)
	{
		DMCLOG_D("client_fd = %d",client_fd);
		enRet = -1;
		return enRet;
	}
	DMCLOG_D("dn->ip = %s",dn->ip);
	enRet = DM_UdpSend(client_fd,send_buf, strlen(send_buf),&clientAddr);
	if(enRet < 0)
	{
		DMCLOG_D("sendto fail,errno = %d",errno);
		goto exit;
	}
	int time_out = 500;
	enRet = DM_UdpReceive(client_fd, &rcv_buf, &time_out, &clientAddr);
	if (enRet > 0)
	{
		DMCLOG_D("rcv_buf:%s",rcv_buf);
		
	}else{
		DMCLOG_D("rcv_buf timeout");
		enRet = -1;
	}
exit:
	safe_free(send_buf);
	safe_free(rcv_buf);
	DM_DomainClientDeinit(client_fd);
	EXIT_FUNC();
    return enRet;
}

int copy_handle_inotify(copy_info_t *pInfo)
{
    int enRet;
    int client_fd;
    char *send_buf;
    send_buf = notify_copy_send_buf(pInfo);
    if(send_buf == NULL)
    {
        enRet = -1;
        goto exit;;
    }
    enRet = DM_UdpSend(pInfo->client_fd,send_buf, strlen(send_buf),&pInfo->clientAddr);
    if(enRet < 0)
    {
        DMCLOG_D("sendto fail,errno = %d",errno);
		enRet = -1;
        goto exit;
    }
exit:
    safe_free(send_buf);
    return enRet;
}

int del_handle_inotify(del_info_t *pInfo)
{
	if(pInfo->c == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}

	char *buf = notify_del_send_buf(pInfo);
	if(buf == NULL)
	{
		DMCLOG_E("alloc error");
		return -1;
	}
	read_stream_comb(&pInfo->c->loc,buf);
	safe_free(buf);
	return 0;
}


int search_handle_udp_inotify(file_search_info_t *pInfo, file_list_t *plist)
{
    int enRet;
    int client_fd;
    char *send_buf = NULL;
	char *recv_buf = NULL;
	unsigned int time_out = 1000;

	pInfo->statusCode++;
    send_buf = notify_search_comb_send_buf(pInfo, plist);
    if(send_buf == NULL){
        enRet = -1;
        goto exit;;
    }
    enRet = DM_UdpSend(pInfo->client_fd,send_buf, strlen(send_buf),&pInfo->clientAddr);
    if(enRet < 0){
        DMCLOG_D("sendto fail,errno = %d",errno);
		enRet = -1;
        goto exit;
    }
	
	enRet = DM_UdpReceive(pInfo->client_fd, &recv_buf, &time_out,&pInfo->clientAddr);
	if(enRet < 0){
        DMCLOG_D("receive fail,errno = %d",errno);
		enRet = -1;
        goto exit;
    }

	enRet = notify_search_parse_recv_buf(pInfo, recv_buf);
	if(enRet < 0){
        DMCLOG_D("parse recv buf fail,errno = %d",errno);
		enRet = -1;
        goto exit;
    }
	
	if(pInfo->status == 1){
		DMCLOG_D("pInfo->statusCode: %d, pInfo->recvStatusCode: %d", pInfo->statusCode, pInfo->recvStatusCode);
		if(pInfo->statusCode == pInfo->recvStatusCode){
			DMCLOG_D("response success");
		}
		else{
			DMCLOG_D("response fail");
		}
	}
	
exit:
    safe_free(send_buf);
	safe_free(recv_buf);
    return enRet;
}

int notify_scanning_status(struct dev_dnode *dn,int status)
{
	int enRet;
	int client_fd;
	char *send_buf;
	char *rcv_buf;
	struct sockaddr_in clientAddr;
	 /*
         * Create a unix domain socket.
        */
	client_fd = DM_UdpClientInit(PF_INET, dn->port, SOCK_DGRAM,dn->ip,&clientAddr);
	if(client_fd <= 0)
	{
		DMCLOG_D("client_fd = %d",client_fd);
		enRet = -1;
		return enRet;
	}
	DMCLOG_D("client_fd = %d,port = %d,ip = %s",client_fd,dn->port,dn->ip);
	send_buf = notify_scanning_send_buf(status);
	if(send_buf == NULL)
	{
		enRet = -1;
		goto exit;;
	}
	DMCLOG_D("access udp send");
	enRet = DM_UdpSend(client_fd,send_buf, strlen(send_buf),&clientAddr);
	if(enRet == -1)
	{
		DMCLOG_D("sendto fail,errno = %d",errno);
		goto exit;
	}
	DMCLOG_D("DM_UdpReceive");
	int time_out = 3000;
	enRet = DM_UdpReceive(client_fd, &rcv_buf, &time_out, &clientAddr);
	if (enRet > 0)
	{
		DM_MsgPrintf((void *)rcv_buf, "reply message from server module", enRet, 1);
		
	}else{
		enRet = -1;
	}
exit:
	safe_free(send_buf);
	safe_free(rcv_buf);
	DM_DomainClientDeinit(client_fd);
    return enRet;
}


void dev_info_reverse_trans(dev_dnode_t *dn)
{
	char *send_buf = NULL;
	all_disk_t mAll_disk_t;
	int disk_info_flag = 0;//0:no get ; 1:has get
	int res = 0;

	//DMCLOG_D("dn->power_seq = %u,p_hardware_info.power_seq = %u",dn->power_seq,p_hardware_info.power_seq);
	if(dn->power_seq != p_hardware_info.power_seq)
	{
		if(dn->request_type & power_changed){			
			EnterCriticalSection(&power_info_global.mutex);
			send_buf = notify_comb_power_send_buf(&power_info_global.power_info, power_changed, dn->power_seq);
			LeaveCriticalSection(&power_info_global.mutex);
			if(NULL != send_buf)
			{
				if(router_handle_client_request(send_buf, dn)>0)
				{
					_update_seq_to_dev(dn->ip,p_hardware_info.power_seq,POWER_TYPE);
				}
			}	
		}
	}
	
	//DMCLOG_D("dn->disk_seq = %u,p_hardware_info.disk_seq = %u",dn->disk_seq,p_hardware_info.disk_seq);
	if(dn->disk_seq != p_hardware_info.disk_seq)
	{
		DMCLOG_D("disk_changed = %d,request_type & disk_changed = %d",disk_changed,dn->request_type & disk_changed);
		if(dn->request_type & disk_changed){
			if(0 == disk_info_flag){
				memset(&mAll_disk_t,0,sizeof(all_disk_t));
				mAll_disk_t.storage_dir = STORAGE_BOARD;
				res = dm_get_storage(&mAll_disk_t);
				if(res == 0)
				{
					DMCLOG_D("drive_count = %d",mAll_disk_t.count);
#ifdef MCU_COMMUNICATE_SUPPORT
					#ifdef DM_CYCLE_DISK
					mAll_disk_t.storage_dir = disk_status;
					#else
					if(0 != dm_get_storage_dir(&mAll_disk_t.storage_dir))
						mAll_disk_t.storage_dir = STORAGE_BOARD;
					#endif
#endif	
					disk_info_flag = 1;
				}
			}
			if(1 == disk_info_flag)
				send_buf = notify_comb_disk_send_buf(&mAll_disk_t, disk_changed, dn->disk_seq);

			if(NULL != send_buf){
				if(router_handle_client_request(send_buf,dn)>0){
					_update_seq_to_dev(dn->ip,p_hardware_info.disk_seq,DISK_TYPE);
				}
			}
		}
	}

	if(dn->ssid_seq != p_hardware_info.ssid_seq)
	{
		if(dn->request_type & ssid_changed){
			send_buf = notify_comb_ssid_send_buf(ssid_changed, dn->power_seq);
			if(NULL != send_buf){
				if(router_handle_client_request(send_buf,dn)>0){
					_update_seq_to_dev(dn->ip,p_hardware_info.ssid_seq,SSID_TYPE);
				}
			}
		}
	}
	
	if(dn->db_seq != p_hardware_info.db_seq)
	{
		if(dn->request_type & data_base_changed)
		{
			send_buf = notify_comb_db_send_buf(data_base_changed, dn->db_seq, get_database_sign());
			if(NULL != send_buf){
				if(router_handle_client_request(send_buf,dn)>0){
					_update_seq_to_dev(dn->ip,p_hardware_info.db_seq,DB_TYPE);
					DMCLOG_D("dn->db_seq = %u,p_hardware_info.db_seq = %u",dn->db_seq,p_hardware_info.db_seq);
				}
			}
		}
	}
	//DMCLOG_D("dn->pwd_seq = %u,p_hardware_info.pwd_seq = %u",dn->pwd_seq,p_hardware_info.pwd_seq);
	if(dn->pwd_seq != p_hardware_info.pwd_seq)
	{
		if(dn->request_type & pwd_changed)
		{
			send_buf = notify_comb_pwd_send_buf(pwd_changed, dn->pwd_seq);
			if(NULL != send_buf){
				if(router_handle_client_request(send_buf,dn)>0){
					_update_seq_to_dev(dn->ip,p_hardware_info.pwd_seq,PWD_TYPE);
					DMCLOG_D("dn->pwd_seq = %u,p_hardware_info.pwd_seq = %u",dn->pwd_seq,p_hardware_info.pwd_seq);
				}
			}
		}
	}
}

void update_hd_dnode()
{
	dev_dnode_t *dev_dnode = NULL;
	unsigned cnt = 0;
	int i = 0;
	
	dev_dnode_t **dnp = _get_dev_list(&cnt);

	if(dnp == NULL || cnt == 0)
		return;
	//DMCLOG_D("cnt = %u",cnt);
	
	for( i = 0;i < cnt;i++ )
	{
		//DMCLOG_D("i = %d,session = %s,ip = %s",i,dnp[i]->session,dnp[i]->ip);
		dev_info_reverse_trans(dnp[i]);
	}
	_free_dev_list(dnp);
}
void notify_disk_scan_status(int status)
{
	dev_dnode_t *dev_dnode = NULL;
	unsigned cnt = 0;
	int i = 0;
	
	dev_dnode_t **dnp = _get_dev_list(&cnt);

	if(dnp == NULL || cnt == 0)
		return;
	
	for( i = 0;i < cnt;i++ )
	{
		DMCLOG_D("i = %d,session = %s,ip = %s",i,dnp[i]->session,dnp[i]->ip);
		notify_scanning_status(dnp[i],status);
	}
	_free_dev_list(dnp);
}
int notify_router_para(const char *inotify_path,int pnTimeOut)   
{  
    int fd;  
    int wd;   
	int enRet = 0;
  
	fd = DM_InotifyInit();
	if(fd < 0)
		ERR_EXIT("inotify_init",inotify_init_err);  
    wd = DM_InotifyAddWatch(fd,inotify_path);
  	if(wd < 0)
		ERR_EXIT("inofity_add_watch", inotify_add_watch_err);

    while(exit_flag == 0)  
    {  
        enRet = DM_InotifyReceive(fd,&pnTimeOut);
		if(enRet > 0 || enRet == -9004)
		{
			//更新路由器硬件参数流水号
			if(enRet > 0)
			{
				if(enRet&power_changed)
				{
					p_hardware_info.power_seq += 1;
				}
				
				if(enRet&disk_changed)
				{
					p_hardware_info.disk_seq += 1;
					//dm_tcp_scan_notify_no_wait(AIRDISK_OFF_PC);
				}

				if(enRet&ssid_changed)
				{
					p_hardware_info.ssid_seq += 1;
				}

				if(enRet&data_base_changed)
				{
					p_hardware_info.db_seq += 1;
				}

				if(enRet&pwd_changed)
				{
					p_hardware_info.pwd_seq += 1;
				}
			}
			update_hd_dnode();
		}
		// 更新指定登录的用户硬件参数流水号并往指定目标发包
    }  
success_exit:  
    ( void ) DM_InotifyRmWatch(fd,wd);  
    ( void ) DM_InotifyClose(fd);  
    return 0;  
  
read_err:  
select_err:  
inotify_add_watch_err:  
    ( void ) DM_InotifyRmWatch( fd, wd );  
inotify_init_err:  
    ( void ) DM_InotifyClose( fd );  
    return -1;  
}  

