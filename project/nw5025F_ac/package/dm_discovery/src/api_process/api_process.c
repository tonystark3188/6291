

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
#include "api_process.h"
#include "my_debug.h"

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
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(p_client_info->ver,int));
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


int _get_route_details(ClientTheadInfo *p_client_info)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* dev_json = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(dev_json, "type",JSON_NEW_OBJECT(0,int));//0:airdisk,1:airmusic,2:
	JSON_ADD_OBJECT(dev_json, "port",JSON_NEW_OBJECT(20520,int));
	JSON_ADD_OBJECT(dev_json, "suppServ",JSON_NEW_OBJECT(16,int));
	JSON_ADD_OBJECT(dev_json, "miscellaneous",JSON_NEW_OBJECT("0",string));
	
	JSON_ARRAY_ADD_OBJECT (response_data_array,dev_json);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(p_client_info->ver,int));
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
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(p_client_info->ver,int));
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
	p_client_info->retstr = (char*)malloc(res_sz + 1);
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
	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep);
	timep = mktime(p);
	//DMCLOG_D("time()->localtime()->mktime():%d",timep);
	update_usr_for_ip(&usr_dn,p_client_info->ip,timep);
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
	update_usr_count(&usr_dn,p_client_info->ip);
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
	int res_sz;
	int ret = -1;
	char *cmd_buf = "input cmd is not finished!";
	//DMCLOG_D("p_client_info->cmd = %d",p_client_info->cmd);
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
		res_sz = strlen(cmd_buf);
		p_client_info->retstr = (char *)malloc(res_sz + 1);
		if(p_client_info->retstr == NULL)
		{
			return -1;
		}
	    strcpy(p_client_info->retstr,cmd_buf);
	}
	return 0;
}


