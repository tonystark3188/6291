

/*
 * =============================================================================
 *
 *       Filename:  api_process.c
 *
 *    Description:  longsys sever module.
 *
 *        Version:  1.0
 *        Created:  2014/10/29 14:51:25
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 515296288jf@163.com
 *   Organization:  
 *
 * =============================================================================
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <mntent.h>
#include <sys/vfs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>     /*Unix 标准函数定义*/
#include <sys/types.h>  
#include <locale.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include "file_json.h"
#include "config.h"
#include "session.h"
#include "router_task.h"
#include "task/scan_task.h"
#include "api_process.h"
#include "uci_api.h"

typedef int (*API_FUNC)(ClientTheadInfo *p_client_info);

typedef struct _tag_handle
{
 uint16_t tag;
 API_FUNC tagfun;
}tag_handle;

tag_handle all_tag_handle[]=
{
	{FN_GET_ROUTE_IP, _dm_get_route_ip},
	{FM_GET_ROUTE_DETAIL, _dm_get_route_detail},
	{FM_GET_ROUTE_BEAT, _dm_get_route_beat},
	{FN_NOTIFY_DEL_IP,_dm_notify_del_ip},
};
#define TAGHANDLE_NUM (sizeof(all_tag_handle)/sizeof(all_tag_handle[0]))

extern struct usr_dnode *usr_dn;
static int dm_usr_logout(struct conn *c)
{
	ENTER_FUNC();
	int res = 0;
	DMCLOG_D("c->session = %s",c->session);
	res = handle_db_logout(c->session);
	if(res < 0)
	{
		return -1;
	}
	EXIT_FUNC();
	return 0;
}
static int dm_usr_login(struct conn *c)
{
	int res = 0;
	//l_dm_gen_session(c->session, c->username, c->password, c->cur_time);
	if(c->session == NULL||!*c->session||c->client_ip == NULL)
	{
		DMCLOG_D("get session error");
		return -1;
	}
	
	DMCLOG_D("c->ip = %s",c->client_ip);
	EnterCriticalSection(&c->ctx->mutex);
	res = handle_db_del_usr_for_ip(c->client_ip);
	LeaveCriticalSection(&c->ctx->mutex);
	if(res < 0)
	{
		DMCLOG_D("handle_db_del_usr_for_ip error");
		return -1;
	}
	DMCLOG_D("c->session = %s",c->session);
	EnterCriticalSection(&c->ctx->mutex);
	res = handle_db_login(c->session,c->device_uuid,c->device_name,c->client_ip,c->username,c->password);
	LeaveCriticalSection(&c->ctx->mutex);
	if(res < 0)
	{
		return -1;;
	}
	return 0;
}


int dm_tcp_scan_notify(int release_flag)
{
    int ret = 0;
    int time_out = 2000;
	int tcp_server_port = 13001;
	char *recv_buf = NULL;
	char *send_buf = NULL;
    int client_fd = DM_InetClientInit(AF_INET,tcp_server_port, SOCK_STREAM,LOCAL_ADDR);
    
    if(client_fd >= 0)
    {
    	send_buf = notify_disk_scanning(release_flag);
		ret = DM_MsgSend(client_fd,send_buf, strlen(send_buf));
        if(ret <= 0)
        {
            DMCLOG_E("send result to client failed, We will exit thread now!");
			ret = -1;
            goto _HANDLE_FAILED;
        }
        ret = DM_MsgReceive(client_fd, &recv_buf, &time_out);
        if(ret <= 0)
        {
            DMCLOG_E("_recv_req_from_client failed!");
			ret = -1;
            goto _HANDLE_FAILED;
        }
    }else{
		ret = -1;
	}
_HANDLE_FAILED:	
    DM_DomainClientDeinit(client_fd);
	safe_free(send_buf);
	safe_free(recv_buf);
    return ret;
}


int dm_tcp_scan_notify_no_wait(int release_flag)
{
	ENTER_FUNC();
    int ret = 0;
	int tcp_server_port = 13001;
	char *send_buf = NULL;
    int client_fd = DM_InetClientInit(AF_INET,tcp_server_port, SOCK_STREAM,LOCAL_ADDR);
    
    if(client_fd >= 0)
    {
    	send_buf = notify_disk_scanning(release_flag);
		ret = DM_MsgSend(client_fd,send_buf, strlen(send_buf));
        if(ret <= 0)
        {
            DMCLOG_E("send result to client failed, We will exit thread now!");
			ret = -1;
            goto _HANDLE_FAILED;
        }
    }else{
		ret = -1;
	}
_HANDLE_FAILED:	
    DM_DomainClientDeinit(client_fd);
	safe_free(send_buf);
	EXIT_FUNC();
    return ret;
}


int _get_route_ip(ClientTheadInfo *p_client_info)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* dev_json = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(dev_json, "ip",JSON_NEW_OBJECT("192.168.222.254",string));
	JSON_ARRAY_ADD_OBJECT (response_data_array,dev_json);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(p_client_info->_ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
	JSON_ADD_OBJECT(header_json, "device", JSON_NEW_OBJECT(p_client_info->device,int));
	JSON_ADD_OBJECT(header_json, "appid", JSON_NEW_OBJECT(p_client_info->appid,int));
	JSON_ADD_OBJECT(header_json, "code", JSON_NEW_OBJECT(p_client_info->code,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res_sz = 0;
	char *response_str = JSON_TO_STRING(response_json);
	if(response_str == NULL)
	{
		return -1;
	}
	res_sz = strlen(response_str);
	p_client_info->retstr = (char*)malloc(res_sz + 1);
	if(p_client_info->retstr == NULL)
	{
		return -1;
	}	
	strcpy(p_client_info->retstr,response_str);
	JSON_PUT_OBJECT(response_json);
	return 0;
}


int get_route_mac(char *mac)
{
	if(mac == NULL)
	{
		DMCLOG_E("para id null");
		return -1;
	}
	FILE *read_fp = NULL;
	if( (read_fp=fopen("/etc/mac.txt", "rb")) != NULL)
	{
		fread(mac,1,17,read_fp);
		DMCLOG_D("mac = %s",mac);
		fclose(read_fp);
		return 0;
	}
	return -1;
}

int get_route_id(char *device_id)
{
 	int ret = 0;
	if(device_id == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	ret = dm_get_device_id(device_id);
	if(ret != 0)
	{
		DMCLOG_E("get device id error");
		return -1;
	}
	DMCLOG_D("device_id = %s",device_id);
	//strcpy(device_id,"airdisk_0005");//"airdisk_0003",4,5
	return ret;
}


int get_route_name(char *name)
{
	int ret = 0;
	char cmd_buf[128] = {0};
	int iface = 0;//ap wifi ssid
	if(name == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	
    sprintf(cmd_buf,"wireless.@wifi-iface[%d].ssid",iface);
    return uci_get_option_value(cmd_buf, name);
}

int _get_route_details(ClientTheadInfo *p_client_info)
{
	char mac[32] = {0};
	char name[64] = {0};
	char device_id[32] = {0};
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* dev_json = JSON_NEW_EMPTY_OBJECT();
	//JObj* miscellaneous_json = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(dev_json, "type",JSON_NEW_OBJECT(0,int));//0:airdisk,1:airmusic,2:
	JSON_ADD_OBJECT(dev_json, "port",JSON_NEW_OBJECT(20520,int));
	JSON_ADD_OBJECT(dev_json, "suppServ",JSON_NEW_OBJECT(16,int));
	JSON_ADD_OBJECT(dev_json, "miscellaneous",JSON_NEW_OBJECT("0",string));

	if(get_route_name(name) == 0)
		JSON_ADD_OBJECT(dev_json, "device_name",JSON_NEW_OBJECT(name,string));
	
	if(get_route_id(device_id) == 0)
	JSON_ADD_OBJECT(dev_json, "device_id",JSON_NEW_OBJECT(device_id,string));

	if(get_route_mac(mac) == 0)
		JSON_ADD_OBJECT(dev_json, "mac",JSON_NEW_OBJECT(mac,string));
	
	//JSON_ADD_OBJECT(miscellaneous_json, "device_name",JSON_NEW_OBJECT("",string));
	//JSON_ADD_OBJECT(miscellaneous_json, "device_id",JSON_NEW_OBJECT(1234,int));
	//JSON_ADD_OBJECT(dev_json, "miscellaneous", miscellaneous_json);
	JSON_ARRAY_ADD_OBJECT (response_data_array,dev_json);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(p_client_info->_ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
	JSON_ADD_OBJECT(header_json, "device", JSON_NEW_OBJECT(p_client_info->device,int));
	JSON_ADD_OBJECT(header_json, "appid", JSON_NEW_OBJECT(p_client_info->appid,int));
	JSON_ADD_OBJECT(header_json, "code", JSON_NEW_OBJECT(p_client_info->code,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res_sz = 0;
	char *response_str = JSON_TO_STRING(response_json);
	if(response_str == NULL)
	{
		return -1;
	}
	res_sz = strlen(response_str);
	p_client_info->retstr = (char*)malloc(res_sz + 1);
	if(p_client_info->retstr == NULL)
	{
		return -1;
	}	
	strcpy(p_client_info->retstr,response_str);
	JSON_PUT_OBJECT(response_json);
	return 0;
}



int _get_route_beat(ClientTheadInfo *p_client_info)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(p_client_info->_ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
	JSON_ADD_OBJECT(header_json, "device", JSON_NEW_OBJECT(p_client_info->device,int));
	JSON_ADD_OBJECT(header_json, "appid", JSON_NEW_OBJECT(p_client_info->appid,int));
	JSON_ADD_OBJECT(header_json, "code", JSON_NEW_OBJECT(p_client_info->code,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res_sz = 0;
	char *response_str = JSON_TO_STRING(response_json);
	if(response_str == NULL)
	{
		return -1;
	}
	res_sz = strlen(response_str);
	p_client_info->retstr = (char*)malloc(res_sz + 1);
	if(p_client_info->retstr == NULL)
	{
		return -1;
	}	
	strcpy(p_client_info->retstr,response_str);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int _notify_del_ip(ClientTheadInfo *p_client_info)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* dev_json = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(dev_json, "ip",JSON_NEW_OBJECT(p_client_info->ip,string));
	JSON_ARRAY_ADD_OBJECT (response_data_array,dev_json);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->code,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res_sz = 0;
	char *response_str = JSON_TO_STRING(response_json);
	if(response_str == NULL)
	{
		return -1;
	}
	res_sz = strlen(response_str);
	p_client_info->retstr = (char*)calloc(1,res_sz + 1);
	if(p_client_info->retstr == NULL)
	{
		return -1;
	}	
	strcpy(p_client_info->retstr,response_str);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int _dm_notify_del_ip(ClientTheadInfo *p_client_info)
{
	int ret = _notify_del_ip(p_client_info);
	if(ret < 0)
	{
		return -1;
	}
	return 0;
}

int _dm_get_route_ip(ClientTheadInfo *p_client_info)
{
#ifdef DELETE_FUNC
	add_usr_to_list(p_client_info->ip);
#endif
 	int ret = _get_route_ip(p_client_info);
	if(ret < 0)
	{
		return -1;
	}
	return 0;
}

int _dm_get_route_detail(ClientTheadInfo *p_client_info)
{
 	int ret = _get_route_details(p_client_info);
	if(ret < 0)
	{
		return -1;
	}
	return 0;
}

int _dm_get_route_beat(ClientTheadInfo *p_client_info)
{
	int ret;
#ifdef DELETE_FUNC
	update_usr_time(p_client_info->ip);
#endif
 	ret = _get_route_beat(p_client_info);
	if(ret < 0)
	{
		return -1;
	}
	return 0;
}

int api_process(ClientTheadInfo *p_client_info)
{ 
	uint8_t i = 0;
	uint8_t switch_flag = 0;
	int ret = -1;
	char *cmd_buf = "input cmd is not finished!";
	for(i = 0; i<TAGHANDLE_NUM; i++)
	{
		if(p_client_info->cmd == all_tag_handle[i].tag)
		{
	       	 ret = all_tag_handle[i].tagfun(p_client_info);
		     switch_flag = 1;
		}
	}
	
	if(switch_flag == 0)
	{
		p_client_info->retstr = (char *)calloc(1,strlen(cmd_buf) + 1);
		if(p_client_info->retstr == NULL)
		{
			return -1;
		}
	    strcpy(p_client_info->retstr,cmd_buf);
	}
	return 0;
}


int dm_get_version(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	strcpy(c->ver,get_sys_dm_router_version());
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* ver_info = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(ver_info, "ver",JSON_NEW_OBJECT(c->ver,string));
	JSON_ARRAY_ADD_OBJECT (response_data_array,ver_info);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int dm_login(struct conn *c)
{
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	if(c->error != 0)
	{
		DMCLOG_D("json cmd is error");
		goto EXIT;
	}
	
	DMCLOG_D("device_name = %s",c->device_name);
	int res = dm_usr_login(c);
	if(res < 0)
	{
		c->error = ERROR_LOGIN;
		goto EXIT;
	}
	// 1:login
	JObj* session_info = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(session_info, "session",JSON_NEW_OBJECT(c->session,string));
	JSON_ARRAY_ADD_OBJECT (response_data_array,session_info);
	
	// 2:get version
	strcpy(c->ver,get_sys_dm_router_version());
	JObj* ver_info = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(ver_info, "ver",JSON_NEW_OBJECT(c->ver,string));
	JSON_ARRAY_ADD_OBJECT (response_data_array,ver_info);

	// 3:get status changed
	c->statusFlag = dm_get_status_changed();
	if(c->statusFlag < 0)
	{
		c->error = ERROR_GET_STATUS_CHANGED;
		goto EXIT;
	}
	JObj* status_info = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(status_info, "statusFlag",JSON_NEW_OBJECT(c->statusFlag,int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,status_info);

	// 4:get function list
	JObj* func_list = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(func_list, "funcListFlag",JSON_NEW_OBJECT(get_func_list_flag(),int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,func_list);

	//5:get database seq
	JObj* database_sign = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(database_sign, "databaseSign",JSON_NEW_OBJECT(get_database_sign(),int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,database_sign);
EXIT:
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int dm_logout(struct conn *c)
{
	ENTER_FUNC();
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	int res = dm_usr_logout(c);
	if(res  < 0)
	{
		c->error = ERROR_LOGOUT;
	}
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
	return 0;
}


int dm_get_service_info(struct conn *c)
{
	#if 0
	int i = 0;
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	service_list_t p_service_list;
	memset(&p_service_list,0,sizeof(service_list_t));
	int ret = get_service_list(&p_service_list);
	
	DMCLOG_D("count = %d",p_service_list.count);
	if(ret < 0)
	{
		c->error = ERROR_GET_SERVICE_LIST;
		goto EXIT;
	}
	JObj *response_data_array = JSON_NEW_ARRAY();
	for(i = 0;i < p_service_list.count;i++)
	{
		JObj *service_info = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(service_info, "name", JSON_NEW_OBJECT(p_service_list.service_info[i].name,string));
		JSON_ADD_OBJECT(service_info, "port", JSON_NEW_OBJECT(p_service_list.service_info[i].port,int));
		JSON_ARRAY_ADD_OBJECT (response_data_array,service_info);
	}
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
#endif
}

int _dm_del_client_info(struct conn *c)
{
	ENTER_FUNC();
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	//EnterCriticalSection(&c->ctx->mutex);
	int res = _del_dev_from_list_by_ip(c->client_ip);
	//LeaveCriticalSection(&c->ctx->mutex);
	if(res < 0)
	{
		DMCLOG_D("del client info error");
		c->error = ERROR_DEL_CLIENT_INFO;
		goto EXIT;
	}
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
	return 0;
}


int dm_router_get_option(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int dm_router_get_status_changed(struct conn *c)
{
	ENTER_FUNC();
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	c->statusFlag = dm_get_status_changed();
	if(c->statusFlag < 0)
	{
		c->error = ERROR_GET_STATUS_CHANGED;
	}else{
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj* status_info = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(status_info, "statusFlag",JSON_NEW_OBJECT(c->statusFlag,int));
		JSON_ARRAY_ADD_OBJECT (response_data_array,status_info);
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
	return 0;
}

int dm_router_get_func_list(struct conn *c)
{
	int res = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* status_info = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(status_info, "statusFlag",JSON_NEW_OBJECT(get_func_list_flag(),int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,status_info);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int dm_router_set_status_changed_listener(struct conn *c)
{
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	if(c->session&&!*c->session)
	{
		DMCLOG_D("session is error");
		goto EXIT;
	}
	DMCLOG_D("c->cient_port = %d",c->client_port);
	int res = _add_dev_to_list(c->session,c->client_ip,c->client_port,c->statusFlag);
	if(res < 0)
	{
		c->error = ERROR_SET_STATUS_CHANGED;
	}
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int Parser_GetVersion(struct conn *c)
{
	
}
int Parser_Login(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	JObj *username_json = JSON_GET_OBJECT(para_json,"username");
	JObj *password_json = JSON_GET_OBJECT(para_json,"password");
	JObj *deviceType_json = JSON_GET_OBJECT(para_json,"deviceType");
	JObj *deviceUuid_json = JSON_GET_OBJECT(para_json,"deviceUuid");
	JObj *deviceName_json = JSON_GET_OBJECT(para_json,"deviceName");
	DMCLOG_D("header.cmd = %d",c->cmd);
	if(username_json == NULL||password_json == NULL||deviceType_json == NULL||deviceUuid_json == NULL||deviceName_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	S_STRNCPY(c->username,JSON_GET_OBJECT_VALUE(username_json,string),64);
	S_STRNCPY(c->password,JSON_GET_OBJECT_VALUE(password_json,string),64);
	c->deviceTpye = JSON_GET_OBJECT_VALUE(deviceType_json,int);
	char *deviceUuid = JSON_GET_OBJECT_VALUE(deviceUuid_json,string);
	if(deviceUuid == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = SERVER_OUT_MEMORY;
		goto EXIT;
	}
	S_STRNCPY(c->device_uuid,deviceUuid,DEVICE_UUID_LEN);
	
	char *deviceName = JSON_GET_OBJECT_VALUE(deviceName_json,string);
	if(deviceName == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = SERVER_OUT_MEMORY;
		goto EXIT;
	}
	DMCLOG_D("deviceName = %s",deviceName);
	S_STRNCPY(c->device_name,deviceName,DEVICE_NAME_LEN);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;
}
int Parser_Logout(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	JObj *session_json = JSON_GET_OBJECT(para_json,"session");
	DMCLOG_D("header.cmd = %d",c->cmd);
	if(session_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	char *session = JSON_GET_OBJECT_VALUE(session_json,string);
	if(session == NULL)
	{
		DMCLOG_D("session is NULL");
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}else if(strlen(session) < 16)
	{
		DMCLOG_D("session is normal");
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	memset(c->session,0,sizeof(c->session));
	strcpy(c->session,session);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;
}
int Parser_GetServiceInfo(struct conn *c)
{
	
}
int Parser_DelClientInfo(struct conn *c)
{
	DMCLOG_D("c->cmd = %d",c->cmd);
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	JObj *ip_json = JSON_GET_OBJECT(para_json,"ip");
	if(ip_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	strcpy(c->client_ip, JSON_GET_OBJECT_VALUE(ip_json,string));
	DMCLOG_D("cleint ip = %s",c->client_ip);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;
}
int Parser_DiskScanning(struct conn *c)
{
	return 0;
}
int Parser_ReleaseDisk(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	JObj *release_flag_json = JSON_GET_OBJECT(para_json,"release_flag");
	JObj *event_json = JSON_GET_OBJECT(para_json,"event");
	JObj *action_node_json = JSON_GET_OBJECT(para_json,"action_node");
	if(release_flag_json != NULL && event_json != NULL && action_node_json != NULL)
	{
		c->release_flag = JSON_GET_OBJECT_VALUE(release_flag_json,int);
		c->event = JSON_GET_OBJECT_VALUE(release_flag_json,int);
		strcpy(c->action_node, JSON_GET_OBJECT_VALUE(release_flag_json,string));
		DMCLOG_D("release_flag: %d, event: %d, action_node: %s", c->release_flag, c->event, c->action_node);
	}
	else{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;
}
int Parser_RouterGetOption(struct conn *c)
{
	return 0;
}
int Parser_RouterGetStatusChanged(struct conn *c)
{
	return 0;
}
int Parser_RouterGetFuncList(struct conn *c)
{
	return 0;
}
int Parser_RouterSetStatusListen(struct conn *c)
{
	DMCLOG_D("c->cmd = %d",c->cmd);
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	JObj *port_json = JSON_GET_OBJECT(para_json,"port");
	JObj *statusFlag_json = JSON_GET_OBJECT(para_json,"statusFlag");
	if(port_json == NULL||statusFlag_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	c->client_port = JSON_GET_OBJECT_VALUE(port_json,int);
	c->statusFlag = JSON_GET_OBJECT_VALUE(statusFlag_json,int);
	DMCLOG_D("port = %d",c->client_port);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;
}



	


