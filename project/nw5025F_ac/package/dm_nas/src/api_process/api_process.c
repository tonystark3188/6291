

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
#include "uci_api.h"
#include "api_process.h"

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
	const char *response_str = JSON_TO_STRING(response_json);
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
	const char *response_str = JSON_TO_STRING(response_json);
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
	const char *response_str = JSON_TO_STRING(response_json);
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
	const char *response_str = JSON_TO_STRING(response_json);
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



	


