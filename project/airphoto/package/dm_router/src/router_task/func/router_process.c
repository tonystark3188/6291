

/*
 * =============================================================================
 *
 *       Filename:  router_process.c
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
#include "hd_wifi.h"
#include "hd_net.h"
#include "hd_route.h"
#include "format.h"
#include "net_config.h"
#include "hidisk_errno.h"

#define NTFS_TYPE 1

int dm_router_errors(int ret_value)
{
	int error_code = 0;
	if(ROUTER_ERRORS_UCI == ret_value)
    	error_code = DM_ERROR_UCI;
	else if(ROUTER_ERRORS_CMD_DATA == ret_value)
		error_code = REQUEST_FORMAT_ERROR;
	else if(ROUTER_ERRORS_SHELL_HANDLE == ret_value)
		error_code= DM_ERROR_SHELL_HANDLE;
	else if(ROUTER_ERRORS_MCU_IOCTL == ret_value)
		error_code= DM_ERROR_MCU_IOCTL_ERROR;
	else if(ROUTER_ERRORS_SOCKET_IOCTL == ret_value)
		error_code= DM_ERROR_SOCKET_IOCTL_ERROR;
	else 
		error_code = DM_ERROR_UNKNOW;

	return error_code;
}


static int _is_client_close(int fd)
{
    if(fd < 0)
        return 1;

    int ret;
    struct timeval tv;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    memset(&tv, 0, sizeof(struct timeval));
    ret = select(fd+1, &rfds, NULL, NULL, &tv);
    if(ret == 0)
    {
        // timeout, fd not ready to read
        return 0;
    }
    else if(ret > 0)
    {
        // fd is ready to read. Maybe client has close this fd.
        //char tmp;
        //if(read(fd, &tmp, 1) == 0)

        return 1;
		
    }
    else
    {
        // select error
        DMCLOG_D("select failed, error(%d)", errno);
        return 1;
    }
}

/*return < 0 :can not get ip*/
int get_wlan_ip_status()
{
	FILE *read_fp; 
	int chars_read; 
	char buffer[8]={0};
	int ret=-1;
	read_fp = popen("route | grep eth1 | wc -l", "r");
	if(read_fp!=NULL)
	{
		chars_read = fread(buffer, sizeof(char), sizeof(buffer)-1, read_fp); 
		if (chars_read > 0&&atoi(buffer) == 2) 
		{ 
			ret = 0;
		} 
	}
	pclose(read_fp);
	return ret;
}
int _get_wifi_settings(struct conn *c)
{
	int band_flag = 0;//0:no band;  1:2.4G;  2:5G;  3:2.4G+5G
	int ret = 0;
	hd_wifi_info m_wifi_info;
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj* wifi_info[2];
    wifi_info[0] = JSON_NEW_EMPTY_OBJECT();
    wifi_info[1] = JSON_NEW_EMPTY_OBJECT();   

	ret = router_get_wifi_band();
	if(ret <= 0)
	{
		
		return -1;
	}else
	{
		band_flag = ret;
	}
	
	if(band_flag & 0x01)
	{
		memset(&m_wifi_info,0,sizeof(hd_wifi_info));
		m_wifi_info.wifi_type = 1;
		ret = dm_get_wifi_settings(&m_wifi_info);
		DMCLOG_D("ret = %d",ret);
		if(ret >= 0)
		{
	        JSON_ADD_OBJECT(wifi_info[0], "wifi_type",JSON_NEW_OBJECT(m_wifi_info.wifi_type,int));
	        JSON_ADD_OBJECT(wifi_info[0], "disabled",JSON_NEW_OBJECT(m_wifi_info.disabled,boolean));
	        JSON_ADD_OBJECT(wifi_info[0], "ssid",JSON_NEW_OBJECT(m_wifi_info.ssid,string));
	        JSON_ADD_OBJECT(wifi_info[0], "encrypt",JSON_NEW_OBJECT(m_wifi_info.encrypt,string));
	        JSON_ADD_OBJECT(wifi_info[0], "wifi_password", JSON_NEW_OBJECT(m_wifi_info.wifi_password,string));
	        JSON_ADD_OBJECT(wifi_info[0], "channel", JSON_NEW_OBJECT(m_wifi_info.channel,int));
	        JSON_ADD_OBJECT(wifi_info[0], "encrypt_len", JSON_NEW_OBJECT(m_wifi_info.encrypt_len,int));
	        JSON_ADD_OBJECT(wifi_info[0], "format", JSON_NEW_OBJECT(m_wifi_info.format,string));
	        JSON_ADD_OBJECT(wifi_info[0], "mac", JSON_NEW_OBJECT(m_wifi_info.mac,string));
	        JSON_ARRAY_ADD_OBJECT(response_data_array,wifi_info[0]);
	    }else{
	    	c->error = dm_router_errors(ret);
	    }
	}

	if(band_flag & 0x02)
	{
		memset(&m_wifi_info,0,sizeof(hd_wifi_info));
		ret = dm_get_wifi_settings(&m_wifi_info);
		if(ret >= 0)
		{
	        JSON_ADD_OBJECT(wifi_info[1], "wifi_type",JSON_NEW_OBJECT(m_wifi_info.wifi_type,int));
	        JSON_ADD_OBJECT(wifi_info[1], "disabled",JSON_NEW_OBJECT(m_wifi_info.disabled,boolean));
	        JSON_ADD_OBJECT(wifi_info[1], "ssid",JSON_NEW_OBJECT(m_wifi_info.ssid,string));
	        JSON_ADD_OBJECT(wifi_info[1], "encrypt",JSON_NEW_OBJECT(m_wifi_info.encrypt,string));
	        JSON_ADD_OBJECT(wifi_info[1], "wifi_password", JSON_NEW_OBJECT(m_wifi_info.wifi_password,string));
	        JSON_ADD_OBJECT(wifi_info[1], "channel", JSON_NEW_OBJECT(m_wifi_info.channel,int));
	        JSON_ADD_OBJECT(wifi_info[1], "encrypt_len", JSON_NEW_OBJECT(m_wifi_info.encrypt_len,int));
	        JSON_ADD_OBJECT(wifi_info[1], "format", JSON_NEW_OBJECT(m_wifi_info.format,string));
	        JSON_ADD_OBJECT(wifi_info[1], "mac", JSON_NEW_OBJECT(m_wifi_info.mac,string));
	        JSON_ARRAY_ADD_OBJECT(response_data_array,wifi_info[1]);
		}else{
	        c->error = dm_router_errors(ret);
	    }
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int _set_wifi_settings(hd_wifi_info *m_wifi_info,struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = -1;
	ret = dm_set_wifi_settings(m_wifi_info);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}
int get_remote_ap_info(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = -1;
	hd_remoteap_info m_remoteap_info;
	memset(&m_remoteap_info,0,sizeof(hd_remoteap_info));
	ret = dm_get_remote_ap_info(&m_remoteap_info);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}else{
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj* rep_bri_info = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(rep_bri_info, "mac",JSON_NEW_OBJECT(m_remoteap_info.mac,string));
		JSON_ADD_OBJECT(rep_bri_info, "ssid",JSON_NEW_OBJECT(m_remoteap_info.ssid,string));
		JSON_ADD_OBJECT(rep_bri_info, "password",JSON_NEW_OBJECT(m_remoteap_info.password,string));
		JSON_ADD_OBJECT(rep_bri_info, "channel",JSON_NEW_OBJECT(m_remoteap_info.channel,int));
		JSON_ADD_OBJECT(rep_bri_info, "encrypt",JSON_NEW_OBJECT(m_remoteap_info.encrypt,string));
		JSON_ADD_OBJECT(rep_bri_info, "tkip_aes",JSON_NEW_OBJECT(m_remoteap_info.tkip_aes,string));
		JSON_ADD_OBJECT(rep_bri_info, "is_connect",JSON_NEW_OBJECT(m_remoteap_info.is_connect,boolean));
		JSON_ARRAY_ADD_OBJECT (response_data_array,rep_bri_info);
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}
	
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}
int set_remote_ap_info(hd_remoteap_info *m_remoteap_info,struct conn *c)
{
	
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = -1;
	ret = dm_set_remote_ap_info(m_remoteap_info);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int get_wlan_con_mode(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = -1;
	ret = dm_get_wlan_con_mode();
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}else{
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj* con_mode_info = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(con_mode_info, "value",JSON_NEW_OBJECT(ret,int));
		JSON_ARRAY_ADD_OBJECT (response_data_array,con_mode_info);
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}
int set_wired_con_mode(wired_con_mode_array *m_wired_con_array,struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = -1;
	ret = dm_set_wired_con_mode(m_wired_con_array);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}
int get_wired_con_mode(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = -1;
	wired_con_mode_array m_wired_con_array;
	memset(&m_wired_con_array,0,sizeof(wired_con_mode_array));
	ret = dm_get_wired_con_mode(&m_wired_con_array);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}else{
		int i = 0;
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj *response_pppoe_json=JSON_NEW_EMPTY_OBJECT();
		JObj *response_dhcp_json=JSON_NEW_EMPTY_OBJECT();
		JObj *response_static_json=JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(response_data_array, "status",JSON_NEW_OBJECT(m_wired_con_array.status,int));
		JSON_ADD_OBJECT(response_data_array, "enable",JSON_NEW_OBJECT(m_wired_con_array.enable,int));
		for(i = 0;i < 3;i++)
		{
			if(m_wired_con_array.m_wired_mode[i].con_type == 1)
			{
				JSON_ADD_OBJECT(response_pppoe_json, "con_type", JSON_NEW_OBJECT(m_wired_con_array.m_wired_mode[i].con_type,int));
				JSON_ADD_OBJECT(response_pppoe_json, "enable", JSON_NEW_OBJECT(m_wired_con_array.m_wired_mode[i].enable,boolean));
				JSON_ADD_OBJECT(response_pppoe_json, "is_connect", JSON_NEW_OBJECT(m_wired_con_array.m_wired_mode[i].is_connect,boolean));
				JSON_ADD_OBJECT(response_pppoe_json, "adsl_name",JSON_NEW_OBJECT(m_wired_con_array.m_wired_mode[i].adsl_name,string));
				JSON_ADD_OBJECT(response_pppoe_json, "adsl_password",JSON_NEW_OBJECT(m_wired_con_array.m_wired_mode[i].adsl_password,string));
				JSON_ARRAY_ADD_OBJECT (response_data_array,response_pppoe_json);
			}else if(m_wired_con_array.m_wired_mode[i].con_type == 2)
			{
				JSON_ADD_OBJECT(response_dhcp_json, "contype", JSON_NEW_OBJECT(m_wired_con_array.m_wired_mode[i].con_type,int));
				JSON_ADD_OBJECT(response_dhcp_json, "enable", JSON_NEW_OBJECT(m_wired_con_array.m_wired_mode[i].enable,boolean));
				JSON_ADD_OBJECT(response_dhcp_json, "is_connect", JSON_NEW_OBJECT(m_wired_con_array.m_wired_mode[i].is_connect,boolean));
				JSON_ARRAY_ADD_OBJECT (response_data_array,response_dhcp_json);
			}else if(m_wired_con_array.m_wired_mode[i].con_type == 3)
			{
				JSON_ADD_OBJECT(response_static_json, "contype", JSON_NEW_OBJECT(m_wired_con_array.m_wired_mode[i].con_type,int));
				JSON_ADD_OBJECT(response_static_json, "enable", JSON_NEW_OBJECT(m_wired_con_array.m_wired_mode[i].enable,boolean));
				JSON_ADD_OBJECT(response_static_json, "is_connect", JSON_NEW_OBJECT(m_wired_con_array.m_wired_mode[i].is_connect,boolean));
				JSON_ADD_OBJECT(response_static_json, "ip",JSON_NEW_OBJECT(m_wired_con_array.m_wired_mode[i].ip,string));
				JSON_ADD_OBJECT(response_static_json, "dns1_ip",JSON_NEW_OBJECT(m_wired_con_array.m_wired_mode[i].dns1_ip,string));
				JSON_ADD_OBJECT(response_static_json, "dns2_ip",JSON_NEW_OBJECT(m_wired_con_array.m_wired_mode[i].dns2_ip,string));
				JSON_ADD_OBJECT(response_static_json, "netmask",JSON_NEW_OBJECT(m_wired_con_array.m_wired_mode[i].netmask,string));
				JSON_ADD_OBJECT(response_static_json, "gateway",JSON_NEW_OBJECT(m_wired_con_array.m_wired_mode[i].gateway,string));
				JSON_ARRAY_ADD_OBJECT (response_data_array,response_static_json);
			}
		}
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int get_wlan_list(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	uint8_t i = 0;
	int list_ret = 0;
	ap_list_info_t ap_list_info;
	memset(&ap_list_info,0,sizeof(ap_list_info_t));
	strcpy(ap_list_info.fre,"24G");
	list_ret = dm_get_wlan_list(&ap_list_info);
	if(list_ret >= 0)
	{
		JObj *response_data_array = JSON_NEW_ARRAY();
		if(ap_list_info.count > 100)
		{
			ap_list_info.count = 100;
		}
		JObj *ap_info[ap_list_info.count];
		for(i = 0;i < ap_list_info.count;i++)
		{
			ap_info[i] = JSON_NEW_EMPTY_OBJECT();
			JSON_ADD_OBJECT(ap_info[i], "ssid",JSON_NEW_OBJECT(ap_list_info.ap_info[i].ssid,string));			
			JSON_ADD_OBJECT(ap_info[i], "mac",JSON_NEW_OBJECT(ap_list_info.ap_info[i].mac,string));
			JSON_ADD_OBJECT(ap_info[i], "channel",JSON_NEW_OBJECT(ap_list_info.ap_info[i].channel,int));
			//JSON_ADD_OBJECT(ap_info[i], "Is_encrypt",JSON_NEW_OBJECT(ap_list_info.ap_info[i].Is_encrypt,boolean));
			JSON_ADD_OBJECT(ap_info[i], "encrypt",JSON_NEW_OBJECT(ap_list_info.ap_info[i].encrypt,string));
			JSON_ADD_OBJECT(ap_info[i], "tkip_aes",JSON_NEW_OBJECT(ap_list_info.ap_info[i].tkip_aes,string));
			JSON_ADD_OBJECT(ap_info[i], "wifi_signal",JSON_NEW_OBJECT(ap_list_info.ap_info[i].wifi_signal,int));;
			JSON_ARRAY_ADD_OBJECT (response_data_array,ap_info[i]);
		}
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}
	else
	{
		c->error = dm_router_errors(list_ret);
	}
	JSON_ADD_OBJECT(header_json , "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int get_power(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = 0;
	power_info_t m_power_info;
	memset(&m_power_info,0,sizeof(power_info_t));
	ret = dm_get_power(&m_power_info);
	if(ret >= 0)
	{
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj* power_info = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(power_info, "power",JSON_NEW_OBJECT(m_power_info.power,int));
		JSON_ADD_OBJECT(power_info, "power_status",JSON_NEW_OBJECT(m_power_info.power_status,int));
		JSON_ARRAY_ADD_OBJECT (response_data_array,power_info);
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}else{
		c->error = dm_router_errors(ret);
	}
	JSON_ADD_OBJECT(header_json , "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}


int get_storage(struct conn *c)
{
	return 0;
}



int format_disk(char * drivname,char * drivdev,struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret =dm_format_disk(drivname,drivdev,NTFS_TYPE);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}
int get_ftp_settings(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	dm_net_info m_net_info;
	memset(&m_net_info,0,sizeof(dm_net_info));
	int ret = dm_get_ftp_settings(&m_net_info);
	if(ret >= 0)
	{
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj* ftp_info = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(ftp_info, "user",JSON_NEW_OBJECT(m_net_info.user,string));
		JSON_ADD_OBJECT(ftp_info, "password",JSON_NEW_OBJECT(m_net_info.password,string));
		JSON_ADD_OBJECT(ftp_info, "port",JSON_NEW_OBJECT(m_net_info.port,int));
		JSON_ADD_OBJECT(ftp_info, "path",JSON_NEW_OBJECT(m_net_info.path,string));
		JSON_ADD_OBJECT(ftp_info, "status",JSON_NEW_OBJECT(m_net_info.status,int));
		JSON_ADD_OBJECT(ftp_info, "anonymous_en",JSON_NEW_OBJECT(m_net_info.anonymous_en,int));
		JSON_ADD_OBJECT(ftp_info, "enable",JSON_NEW_OBJECT(m_net_info.enable,boolean));
		JSON_ARRAY_ADD_OBJECT (response_data_array,ftp_info);
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}else{
		c->error = ERROR_GET_FTP_SETTINGS;
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int set_ftp_settings(dm_net_info *m_net_info,struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_set_ftp_settings(m_net_info);
	if(ret < 0)
	{
		c->error = ERROR_SET_FTP_SETTINGS;
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}
int get_smb_settings(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	dm_net_info m_net_info;
	memset(&m_net_info,0,sizeof(dm_net_info));
	int ret = dm_get_smb_settings(&m_net_info);
	if(ret >= 0)
	{
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj* smb_info = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(smb_info, "user",JSON_NEW_OBJECT(m_net_info.user,string));
		JSON_ADD_OBJECT(smb_info, "password",JSON_NEW_OBJECT(m_net_info.password,string));
		JSON_ADD_OBJECT(smb_info, "port",JSON_NEW_OBJECT(m_net_info.port,int));
		JSON_ADD_OBJECT(smb_info, "path",JSON_NEW_OBJECT(m_net_info.path,string));
		JSON_ADD_OBJECT(smb_info, "status",JSON_NEW_OBJECT(m_net_info.status,int));
		JSON_ADD_OBJECT(smb_info, "anonymous_en",JSON_NEW_OBJECT(m_net_info.anonymous_en,int));
		JSON_ADD_OBJECT(smb_info, "enable",JSON_NEW_OBJECT(m_net_info.enable,boolean));
		JSON_ARRAY_ADD_OBJECT (response_data_array,smb_info);
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}else{
		c->error = dm_router_errors(ret);
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int set_smb_settings(dm_net_info *m_net_info,struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_set_smb_settings(m_net_info);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int get_dms_settings(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	dm_net_info m_net_info;
	memset(&m_net_info,0,sizeof(dm_net_info));
	int ret = dm_get_dms_settings(&m_net_info);
	if(ret >= 0)
	{
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj* dms_info = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(dms_info, "name",JSON_NEW_OBJECT(m_net_info.name,string));
		JSON_ADD_OBJECT(dms_info, "path",JSON_NEW_OBJECT(m_net_info.path,string));
		JSON_ADD_OBJECT(dms_info, "status",JSON_NEW_OBJECT(m_net_info.status,int));
		JSON_ADD_OBJECT(dms_info, "enable",JSON_NEW_OBJECT(m_net_info.enable,boolean));
		JSON_ARRAY_ADD_OBJECT (response_data_array,dms_info);
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}else{
		c->error = dm_router_errors(ret);
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int set_dms_settings(dm_net_info *m_net_info,struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_set_dms_settings(m_net_info);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int get_ddns_settings(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int i = 0;
	dm_ddns_list m_ddns_list;
	memset(&m_ddns_list,0,sizeof(dm_ddns_list));
	int ret = dm_get_ddns_settings(&m_ddns_list);
	if(ret >= 0&&m_ddns_list.count > 0)
	{
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj *enable_json = JSON_NEW_EMPTY_OBJECT();
		JObj* ddns_info[m_ddns_list.count];
		for(i = 0;i < m_ddns_list.count;i++)
		{
			ddns_info[i] = JSON_NEW_EMPTY_OBJECT();
			JSON_ADD_OBJECT(ddns_info[i], "name",JSON_NEW_OBJECT(m_ddns_list.m_ddns_settings[i].name,string));
			JSON_ADD_OBJECT(ddns_info[i], "domain",JSON_NEW_OBJECT(m_ddns_list.m_ddns_settings[i].domain,string));
			JSON_ADD_OBJECT(ddns_info[i], "user",JSON_NEW_OBJECT(m_ddns_list.m_ddns_settings[i].user,int));
			JSON_ADD_OBJECT(ddns_info[i], "password",JSON_NEW_OBJECT(m_ddns_list.m_ddns_settings[i].password,int));
			JSON_ADD_OBJECT(ddns_info[i], "status",JSON_NEW_OBJECT(m_ddns_list.m_ddns_settings[i].status,int));
			JSON_ARRAY_ADD_OBJECT (response_data_array,ddns_info[i]);
		}
		JSON_ADD_OBJECT(enable_json, "enable",JSON_NEW_OBJECT(m_ddns_list.enable,boolean));
		JSON_ARRAY_ADD_OBJECT (response_data_array,enable_json);
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}else{
		c->error = ERROR_GET_DDNS_SETTINGS;
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int set_ddns_settings(dm_ddns_list *m_ddns_list,struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_set_ddns_settings(m_ddns_list);
	if(ret < 0)
	{
		c->error = ERROR_SET_DDNS_SETTINGS;
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int get_webdav_settings(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	dm_net_info m_net_info;
	memset(&m_net_info,0,sizeof(dm_net_info));
	int ret = dm_get_webdav_settings(&m_net_info);
	if(ret >= 0)
	{
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj* webdav_info = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(webdav_info, "user",JSON_NEW_OBJECT(m_net_info.user,string));
		JSON_ADD_OBJECT(webdav_info, "password",JSON_NEW_OBJECT(m_net_info.password,string));
		JSON_ADD_OBJECT(webdav_info, "port",JSON_NEW_OBJECT(m_net_info.port,int));
		JSON_ADD_OBJECT(webdav_info, "path",JSON_NEW_OBJECT(m_net_info.path,string));
		JSON_ADD_OBJECT(webdav_info, "status",JSON_NEW_OBJECT(m_net_info.status,int));
		JSON_ADD_OBJECT(webdav_info, "enable",JSON_NEW_OBJECT(m_net_info.enable,boolean));
		JSON_ARRAY_ADD_OBJECT (response_data_array,webdav_info);
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}else{
		c->error = ERROR_GET_WEBDAV_SETTINGS;
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int set_webdav_settings(dm_net_info *m_net_info,struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_set_webdav_settings(m_net_info);
	if(ret < 0)
	{
		c->error = ERROR_SET_WEBDAV_SETTINGS;
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int get_elec_lock(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_get_elec_lock();
	if(ret >= 0)
	{
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj* lock_info = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(lock_info, "status",JSON_NEW_OBJECT(ret,string));
		JSON_ARRAY_ADD_OBJECT (response_data_array,lock_info);
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}else{
		c->error = ERROR_GET_ELEC_LOCK;
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
}

int set_elec_lock(int status,struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_set_elec_lock(status);
	if(ret < 0)
	{
		c->error = ERROR_SET_ELEC_LOCK;
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int get_3g_access_info(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	dm_3g_access_info m_3g_info;
	memset(&m_3g_info,0,sizeof(dm_3g_access_info));
	int ret = dm_get_3g_access_info(&m_3g_info);
	if(ret >= 0)
	{
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj* g_json = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(g_json, "apnmode",JSON_NEW_OBJECT(m_3g_info.apnmode,int));
		JSON_ADD_OBJECT(g_json, "dialnumber",JSON_NEW_OBJECT(m_3g_info.dialnumber,string));
		JSON_ADD_OBJECT(g_json, "status",JSON_NEW_OBJECT(m_3g_info.status,int));
		JSON_ADD_OBJECT(g_json, "status2",JSON_NEW_OBJECT(m_3g_info.status2,int));
		JSON_ADD_OBJECT(g_json, "enable",JSON_NEW_OBJECT(m_3g_info.enable,boolean));
		JSON_ARRAY_ADD_OBJECT (response_data_array,g_json);
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}else{
		c->error = dm_router_errors(ret);
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int set_3g_access_info(dm_3g_access_info *m_3g_info,struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_set_3g_access_info(m_3g_info);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int get_client_status(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_get_client_status();
	if(ret >= 0)
	{
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj* client_json = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(client_json, "enable",JSON_NEW_OBJECT(ret,boolean));
		JSON_ARRAY_ADD_OBJECT (response_data_array,client_json);
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}else{
		c->error = dm_router_errors(ret);
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int set_client_status(int status,struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_set_client_status(status);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int upgrade_fw(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_upgrade_fw();
	if(ret < 0)
	{
		c->error = ERROR_UPGRADE_FW;
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int get_internet_status(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_get_internet_status();
	if(ret >= 0)
	{
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj* internet_json = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(internet_json, "is_internet",JSON_NEW_OBJECT(ret,boolean));
		JSON_ARRAY_ADD_OBJECT (response_data_array,internet_json);
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}else{
		c->error = ERROR_GET_CLIENT_STATUS;
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}
int sync_time(dm_time_info *time_info_t, struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_sync_time(time_info_t);
	if(ret < 0)
	{
		c->error = ERROR_SET_CLIENT_STATUS;
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int get_ota_info(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	dm_ota_info m_ota_info;
	memset(&m_ota_info,0,sizeof(dm_ota_info));
	int ret = dm_get_ota_info(&m_ota_info);
	if(ret >= 0)
	{
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj* ota_json = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(ota_json, "customCode",JSON_NEW_OBJECT(m_ota_info.customCode,string));
		JSON_ADD_OBJECT(ota_json, "versionCode",JSON_NEW_OBJECT(m_ota_info.versionCode,string));
		JSON_ADD_OBJECT(ota_json, "mac",JSON_NEW_OBJECT(m_ota_info.mac,string));
		JSON_ADD_OBJECT(ota_json, "version_flag",JSON_NEW_OBJECT(m_ota_info.version_flag,int));
		JSON_ADD_OBJECT(ota_json, "time",JSON_NEW_OBJECT(m_ota_info.time,int));
		JSON_ARRAY_ADD_OBJECT (response_data_array,ota_json);
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}else{
		c->error = dm_router_errors(ret);
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}


int get_version_flag(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int version_flag;
	int ret = dm_get_version_flag(&version_flag);
	if(ret >= 0)
	{
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj* version_flag_json = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(version_flag_json, "version_flag",JSON_NEW_OBJECT(version_flag,int));
		JSON_ARRAY_ADD_OBJECT (response_data_array,version_flag_json);
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}else{
		c->error = dm_router_errors(ret);
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}


int set_version_flag(int version_flag,struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_set_version_flag(version_flag);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int get_fw_version(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	char fw_version[64];
	memset(fw_version, 0, 32);
	int ret = dm_get_fw_version(&fw_version);
	if(ret >= 0)
	{
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj* fw_version_json = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(fw_version_json, "fw_version",JSON_NEW_OBJECT(fw_version,string));
		JSON_ARRAY_ADD_OBJECT (response_data_array,fw_version_json);
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}else{
		c->error = dm_router_errors(ret);
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	int res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}


int _dm_get_wifi_settings(struct conn *c)
{
	int ret =  _get_wifi_settings(c);
	if(ret < 0)
	{
		goto EXIT;
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;	
}
int _dm_set_wifi_settings(struct conn *c)
{
	int ret = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		ret = -1;
		goto EXIT;
	}
	JObj *para_24G_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	JObj *para_5G_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,1);
	if(para_24G_json != NULL)
	{
		JObj *fre_24G_json = JSON_GET_OBJECT(para_24G_json,"wifi_type");
		JObj *online_24G_json = JSON_GET_OBJECT(para_24G_json,"disabled");
		JObj *ssid_24G_json = JSON_GET_OBJECT(para_24G_json,"ssid");
		JObj *encrypt_24G_json = JSON_GET_OBJECT(para_24G_json,"encrypt");
		JObj *password_24G_json = JSON_GET_OBJECT(para_24G_json,"wifi_password");
		if(fre_24G_json != NULL&&online_24G_json != NULL&&ssid_24G_json != NULL&&encrypt_24G_json != NULL)
		{
			hd_wifi_info m_wifi_info;
			memset(&m_wifi_info,0,sizeof(hd_wifi_info));
			m_wifi_info.wifi_type = JSON_GET_OBJECT_VALUE(fre_24G_json,int);
			m_wifi_info.disabled = JSON_GET_OBJECT_VALUE(online_24G_json,boolean);
			strcpy(m_wifi_info.ssid,JSON_GET_OBJECT_VALUE(ssid_24G_json,string));
			strcpy(m_wifi_info.encrypt,JSON_GET_OBJECT_VALUE(encrypt_24G_json,string));
			strcpy(m_wifi_info.wifi_password,JSON_GET_OBJECT_VALUE(password_24G_json,string));
			ret = _set_wifi_settings(&m_wifi_info,c);
			if(ret < 0)
			{
				goto EXIT;
			}

		}
	}
	if(para_5G_json != NULL)
	{
		JObj *fre_5G_json = JSON_GET_OBJECT(para_5G_json,"wifi_type");
		JObj *online_5G_json = JSON_GET_OBJECT(para_5G_json,"disabled");
		JObj *ssid_5G_json = JSON_GET_OBJECT(para_5G_json,"ssid");
		JObj *encrypt_5G_json = JSON_GET_OBJECT(para_5G_json,"encrypt");
		JObj *password_5G_json = JSON_GET_OBJECT(para_5G_json,"wifi_password");
		if(fre_5G_json != NULL&&online_5G_json != NULL&&ssid_5G_json != NULL&&encrypt_5G_json != NULL)
		{
			hd_wifi_info m_wifi_info;
			memset(&m_wifi_info,0,sizeof(hd_wifi_info));
			m_wifi_info.wifi_type = JSON_GET_OBJECT_VALUE(fre_5G_json,int);
			m_wifi_info.disabled = JSON_GET_OBJECT_VALUE(online_5G_json,boolean);
			strcpy(m_wifi_info.ssid,JSON_GET_OBJECT_VALUE(ssid_5G_json,string));
			strcpy(m_wifi_info.encrypt,JSON_GET_OBJECT_VALUE(encrypt_5G_json,string));
			strcpy(m_wifi_info.wifi_password,JSON_GET_OBJECT_VALUE(password_5G_json,string));
			ret = _set_wifi_settings(&m_wifi_info,c);
			if(ret < 0)
			{
				goto EXIT;
			}
		}
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;	
}
int _dm_get_remote_ap_info(struct conn *c)
{
	int ret =  get_remote_ap_info(c);
	if(ret < 0)
	{
		goto EXIT;
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;	
}


int _dm_set_remote_ap_info(struct conn *c)
{
	int ret = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		ret = -1;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json != NULL)
	{
		JObj *mac_json = JSON_GET_OBJECT(para_json,"mac");
		JObj *ssid_json = JSON_GET_OBJECT(para_json,"ssid");
		JObj *password_json = JSON_GET_OBJECT(para_json,"password");
		JObj *channel_json = JSON_GET_OBJECT(para_json,"channel");
		JObj *encrypt_json = JSON_GET_OBJECT(para_json,"encrypt");
		JObj *tkip_aes_json = JSON_GET_OBJECT(para_json,"tkip_aes");

		if(ssid_json != NULL&&channel_json != NULL&&encrypt_json != NULL&&tkip_aes_json != NULL)
		{
			hd_remoteap_info m_remoteap_info;
			memset(&m_remoteap_info,0,sizeof(hd_remoteap_info));
			
			//strcpy(m_remoteap_info.mac,JSON_GET_OBJECT_VALUE(mac_json,string));
			strcpy(m_remoteap_info.ssid,JSON_GET_OBJECT_VALUE(ssid_json,string));
			strcpy(m_remoteap_info.mac,JSON_GET_OBJECT_VALUE(mac_json,string));
			strcpy(m_remoteap_info.password,JSON_GET_OBJECT_VALUE(password_json,string));
			m_remoteap_info.channel = JSON_GET_OBJECT_VALUE(channel_json,int);
			strcpy(m_remoteap_info.encrypt,JSON_GET_OBJECT_VALUE(encrypt_json,string)); 
			strcpy(m_remoteap_info.tkip_aes,JSON_GET_OBJECT_VALUE(tkip_aes_json,string));
			ret = set_remote_ap_info(&m_remoteap_info,c);
			if(ret < 0)
			{
				goto EXIT;
			}
		}
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;	
}
int _dm_get_wlan_con_mode(struct conn *c)
{
	int ret = 0;
	ret = get_wlan_con_mode(c);
	if(ret < 0)
	{
		goto EXIT;
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;	
}
int _dm_set_wired_con_mode(struct conn *c)
{
	int ret = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		ret = -1;
		goto EXIT;
	}
	JObj *para_json[4];
	para_json[0] = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	para_json[1] = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,1);
	para_json[2] = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,2);
	para_json[3] = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,3);
	int i=0;
	wired_con_mode_array m_wired_con_array;
	memset(&m_wired_con_array,0,sizeof(wired_con_mode_array));
	if(para_json[0] != NULL)
	{
		JObj *enable_json = JSON_GET_OBJECT(para_json[0],"enable");
		if(enable_json != NULL)
		{
			m_wired_con_array.enable= JSON_GET_OBJECT_VALUE(enable_json,boolean);
			if(m_wired_con_array.enable == 1)
			{
				//for(i = 0;i < 3;i++)
				i=0;
				{
					m_wired_con_array.m_wired_mode[i].con_type = JSON_GET_OBJECT_VALUE(JSON_GET_OBJECT(para_json[i+1], "con_type"),int);
					m_wired_con_array.m_wired_mode[i].is_connect = JSON_GET_OBJECT_VALUE(JSON_GET_OBJECT(para_json[i+1], "is_connect"),boolean);
					if(m_wired_con_array.m_wired_mode[i].con_type == 1)
					{
						strcpy(m_wired_con_array.m_wired_mode[i].adsl_name,JSON_GET_OBJECT_VALUE(JSON_GET_OBJECT(para_json[i+1], "adsl_name"),string));
						strcpy(m_wired_con_array.m_wired_mode[i].adsl_password,JSON_GET_OBJECT_VALUE(JSON_GET_OBJECT(para_json[i+1], "adsl_password"),string));
					}else if(m_wired_con_array.m_wired_mode[i].con_type == 3)
					{
						strcpy(m_wired_con_array.m_wired_mode[i].ip,JSON_GET_OBJECT_VALUE(JSON_GET_OBJECT(para_json[i+1], "ip"),string));
						strcpy(m_wired_con_array.m_wired_mode[i].dns1_ip,JSON_GET_OBJECT_VALUE(JSON_GET_OBJECT(para_json[i+1], "dns1_ip"),string));
						strcpy(m_wired_con_array.m_wired_mode[i].dns2_ip,JSON_GET_OBJECT_VALUE(JSON_GET_OBJECT(para_json[i+1], "dns2_ip"),string));
						strcpy(m_wired_con_array.m_wired_mode[i].netmask,JSON_GET_OBJECT_VALUE(JSON_GET_OBJECT(para_json[i+1], "netmask"),string));
						strcpy(m_wired_con_array.m_wired_mode[i].gateway,JSON_GET_OBJECT_VALUE(JSON_GET_OBJECT(para_json[i+1], "gateway"),string));
					}
				}	
			}		
			ret = set_wired_con_mode(&m_wired_con_array,c);
			if(ret < 0)
				goto EXIT;
		}
	}else{
		goto EXIT;
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;	
}
int _dm_get_wired_con_mode(struct conn *c)
{
	int ret = 0;
	ret = get_wired_con_mode(c);
	if(ret < 0)
	{
		goto EXIT;
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}
int _dm_get_wlan_list(struct conn *c)
{
	int ret = 0;
	ret = get_wlan_list(c);
	if(ret < 0)
	{
		goto  EXIT;
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}
int _dm_get_power(struct conn *c)
{
	int ret = 0;
	ret = get_power(c);  
	if(ret < 0)
		goto EXIT;
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}

int _handle_getStorageInfo(struct conn *c)
{
	int ret = get_storage(c);
	if(ret != 0)
	{
		goto EXIT;
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}

int _dm_format_disk(struct conn *c)
{
	int ret = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		ret = -1;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	JObj *disk_json = JSON_GET_OBJECT(para_json,"disk");
	JObj *dev_node_json = JSON_GET_OBJECT(para_json,"dev_node");
	JObj *is_format_json = JSON_GET_OBJECT(para_json,"is_format");
	if(JSON_GET_OBJECT_VALUE(is_format_json,boolean))
	{
		char *disk_name = JSON_GET_OBJECT_VALUE(disk_json,string);
		char *dev_node = JSON_GET_OBJECT_VALUE(dev_node_json,string);
		if(disk_name != NULL&&dev_node != NULL)
		{
			ret = format_disk(disk_name,dev_node,c);  
			if(ret < 0)
				goto EXIT;
		}
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}
//4.12	��ȡFTP��Ϣ  cmd = 0x020B
int _dm_get_ftp_settings(struct conn *c)
{
	int ret = get_ftp_settings(c);
	if(ret < 0)
	{
		goto EXIT;
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}

//4.13	����FTP��Ϣ  cmd = 0x020C 
int _dm_set_ftp_settings(struct conn *c)
{
	int ret = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		ret = -1;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	JObj *user_json = JSON_GET_OBJECT(para_json,"user");
	JObj *password_json = JSON_GET_OBJECT(para_json,"password");
	JObj *port_json = JSON_GET_OBJECT(para_json,"port");
	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	JObj *status_json = JSON_GET_OBJECT(para_json,"status");
	JObj *anonymous_en_json = JSON_GET_OBJECT(para_json,"anonymous_en_json");
	JObj *enable_json = JSON_GET_OBJECT(para_json,"enable");
	if(user_json != NULL&&password_json != NULL&&port_json != NULL
		&&path_json != NULL&&status_json != NULL&&anonymous_en_json != NULL&&enable_json != NULL)
	{
		dm_net_info m_net_info;
		memset(&m_net_info,0,sizeof(dm_net_info));
		strcpy(m_net_info.user,JSON_GET_OBJECT_VALUE(user_json,string));
		strcpy(m_net_info.password,JSON_GET_OBJECT_VALUE(password_json,string));
		m_net_info.port = JSON_GET_OBJECT_VALUE(port_json,int);
		strcpy(m_net_info.path,JSON_GET_OBJECT_VALUE(path_json,string));
		m_net_info.status= JSON_GET_OBJECT_VALUE(status_json,int);
		m_net_info.anonymous_en= JSON_GET_OBJECT_VALUE(anonymous_en_json,int);
		m_net_info.enable= JSON_GET_OBJECT_VALUE(enable_json,boolean);
		int ret = set_ftp_settings(&m_net_info,c);
		if(ret < 0)
		{
			goto EXIT;
		}
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}
//4.14	��ȡsamba����  cmd = 0x020D
int _dm_get_smb_settings(struct conn *c)
{
	int ret = get_smb_settings(c);
	if(ret < 0)
	{
		goto EXIT;
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}
//4.15	����samba��Ϣ  cmd = 0x020E
int _dm_set_smb_settings(struct conn *c)
{
	
	int ret = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		ret = -1;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	JObj *user_json = JSON_GET_OBJECT(para_json,"user");
	JObj *password_json = JSON_GET_OBJECT(para_json,"password");
	JObj *port_json = JSON_GET_OBJECT(para_json,"port");
	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	JObj *status_json = JSON_GET_OBJECT(para_json,"status");
	JObj *anonymous_en_json = JSON_GET_OBJECT(para_json,"anonymous_en");
	//JObj *enable_json = JSON_GET_OBJECT(para_json,"enable");
	if(user_json != NULL&&password_json != NULL&&port_json != NULL
		&&path_json != NULL&&status_json != NULL&&anonymous_en_json != NULL)//&&enable_json != NULL)
	{
		
		dm_net_info m_net_info;
		memset(&m_net_info,0,sizeof(dm_net_info));
		strcpy(m_net_info.user,JSON_GET_OBJECT_VALUE(user_json,string));
		strcpy(m_net_info.password,JSON_GET_OBJECT_VALUE(password_json,string));
		m_net_info.port = JSON_GET_OBJECT_VALUE(port_json,int);
		strcpy(m_net_info.path,JSON_GET_OBJECT_VALUE(path_json,string));
		m_net_info.status= JSON_GET_OBJECT_VALUE(status_json,int);
		m_net_info.anonymous_en= JSON_GET_OBJECT_VALUE(anonymous_en_json,int);
		//m_net_info.enable= JSON_GET_OBJECT_VALUE(enable_json,boolean);
		int ret = set_smb_settings(&m_net_info,c);
		if(ret < 0)
		{
			goto EXIT;
		}
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}

//4.16	��ȡDMS����  cmd = 0x020F
int _dm_get_dms_settings(struct conn *c)
{
	int ret = get_dms_settings(c);
	if(ret < 0)
	{
		goto EXIT;
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}
//4.17	����DMS��Ϣ  cmd = 0x0210
int _dm_set_dms_settings(struct conn *c)
{
	int ret = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		ret = -1;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	JObj *name_json = JSON_GET_OBJECT(para_json,"name");
	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	JObj *status_json = JSON_GET_OBJECT(para_json,"status");
	//JObj *enable_json = JSON_GET_OBJECT(para_json,"enable");
	if(name_json != NULL&&path_json != NULL&&status_json != NULL)//&&enable_json != NULL)
	{
		dm_net_info m_net_info;
		memset(&m_net_info,0,sizeof(dm_net_info));
		strcpy(m_net_info.name,JSON_GET_OBJECT_VALUE(name_json,string));
		strcpy(m_net_info.path,JSON_GET_OBJECT_VALUE(path_json,string));
		m_net_info.status= JSON_GET_OBJECT_VALUE(status_json,int);
		//m_net_info.enable= JSON_GET_OBJECT_VALUE(enable_json,boolean);
		int ret = set_dms_settings(&m_net_info,c);
		if(ret < 0)
		{
			goto EXIT;
		}
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}

//4.18	��ȡDDNS����  cmd = 0x0211
int _dm_get_ddns_settings(struct conn *c)
{
	int ret = get_ddns_settings(c);
	if(ret < 0)
	{
		goto EXIT;
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}

//4.19	����DDNS��Ϣ  cmd = 0x0212
int _dm_set_ddns_settings(struct conn *c)
{
	
}
//4.20	��ȡWebDAV����  cmd = 0x0213
int _dm_get_webdav_settings(struct conn *c)
{
	int ret = get_webdav_settings(c);
	if(ret < 0)
	{
		goto EXIT;
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}
//4.21	����WebDAV��Ϣ  cmd = 0x0214
int _dm_set_webdav_settings(struct conn *c)
{
	int ret = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		ret = -1;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	JObj *user_json = JSON_GET_OBJECT(para_json,"user");
	JObj *password_json = JSON_GET_OBJECT(para_json,"password");
	JObj *port_json = JSON_GET_OBJECT(para_json,"port");
	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	JObj *status_json = JSON_GET_OBJECT(para_json,"status");
	JObj *enable_json = JSON_GET_OBJECT(para_json,"enable");
	if(user_json != NULL&&password_json != NULL&&port_json != NULL
		&&path_json != NULL&&status_json != NULL&&enable_json != NULL)
	{
		dm_net_info m_net_info;
		memset(&m_net_info,0,sizeof(dm_net_info));
		strcpy(m_net_info.user,JSON_GET_OBJECT_VALUE(user_json,string));
		strcpy(m_net_info.password,JSON_GET_OBJECT_VALUE(password_json,string));
		m_net_info.port = JSON_GET_OBJECT_VALUE(port_json,int);
		strcpy(m_net_info.path,JSON_GET_OBJECT_VALUE(path_json,string));
		m_net_info.status= JSON_GET_OBJECT_VALUE(status_json,int);
		m_net_info.enable= JSON_GET_OBJECT_VALUE(enable_json,boolean);
		int ret = set_webdav_settings(&m_net_info,c);
		if(ret < 0)
		{
			goto EXIT;
		}
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}
//4.22	��ȡ��������Ϣ  cmd = 0x0215
int _dm_get_elec_lock(struct conn *c)
{
	int ret = get_elec_lock(c);
	if(ret < 0)
	{
		goto EXIT;
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}
//4.23	���õ�������Ϣ  cmd = 0x0216
int _dm_set_elec_lock(struct conn *c)
{
	int ret = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		ret = -1;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	JObj *status_json = JSON_GET_OBJECT(para_json,"status");
	if(status_json != NULL)
	{
		int ret = set_elec_lock(JSON_GET_OBJECT_VALUE(status_json,int),c);
		if(ret < 0)
		{
			goto EXIT;
		}
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}

//4.24	��ȡ3G������Ϣ  cmd = 0x0217
int _dm_get_3g_access_info(struct conn *c)
{
	int ret = get_3g_access_info(c);
	if(ret < 0)
	{
		goto EXIT;
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}

//4.25	����3G������Ϣ  cmd = 0x0218
int _dm_set_3g_access_info(struct conn *c)
{
	int ret = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		ret = -1;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	JObj *mode_json = JSON_GET_OBJECT(para_json,"apnmode");
	JObj *number_json = JSON_GET_OBJECT(para_json,"dialnumber");
	JObj *status_json = JSON_GET_OBJECT(para_json,"status");
	JObj *status2_json = JSON_GET_OBJECT(para_json,"status2");
	JObj *enable_json = JSON_GET_OBJECT(para_json,"enable");
	if(mode_json != NULL&&number_json != NULL&&status_json != NULL&&status2_json != NULL&&enable_json != NULL)
	{
		dm_3g_access_info m_3g_info;
		memset(&m_3g_info,0,sizeof(dm_3g_access_info));
		strcpy(m_3g_info.apnmode,JSON_GET_OBJECT_VALUE(mode_json,int));
		strcpy(m_3g_info.dialnumber,JSON_GET_OBJECT_VALUE(number_json,string));
		m_3g_info.status = JSON_GET_OBJECT_VALUE(status_json,int);
		m_3g_info.status2 = JSON_GET_OBJECT_VALUE(status2_json,int);
		m_3g_info.enable = JSON_GET_OBJECT_VALUE(enable_json,boolean);
		int ret = set_3g_access_info(&m_3g_info,c);
		if(ret < 0)
		{
			goto EXIT;
		}
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}

//4.26	��ȡclient����״̬  cmd = 0x0219
int _dm_get_client_status(struct conn *c)
{
	int ret = get_client_status(c);
	if(ret < 0)
	{
		goto EXIT;
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}
//4.27	����client����״̬  cmd = 0x021A
int _dm_set_client_status(struct conn *c)
{
	int ret = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		ret = -1;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	JObj *status_json = JSON_GET_OBJECT(para_json,"disabled");
	if(status_json != NULL)
	{
		int ret = set_client_status(JSON_GET_OBJECT_VALUE(status_json,int),c);
		if(ret < 0)
		{	
			goto EXIT;
		}
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}

//4.28	ִ�д�U������  cmd = 0x021B
int _dm_upgrade_fw(struct conn *c)
{
	int ret = upgrade_fw(c);
	if(ret < 0)
	{
		goto EXIT;
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}
//4.29	��ȡ�Ƿ��Ѿ����ϻ����� cmd = 0x021C
int _dm_get_internet_status(struct conn *c)
{
	int ret = get_internet_status(c);
	if(ret < 0)
	{
		goto EXIT;
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}
//4.30	ͬ��ʱ��  cmd = 0x021D
int _dm_sync_time(struct conn *c)
{
	int ret = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		ret = -1;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	JObj *value_json = JSON_GET_OBJECT(para_json,"time_value");
	JObj *zone_json = JSON_GET_OBJECT(para_json,"time_zone");
	if(value_json != NULL)
	{
		dm_time_info time_info_t;
		memset(&time_info_t,0,sizeof(time_info_t));
		strcpy(time_info_t.time_value, JSON_GET_OBJECT_VALUE(value_json,string));
		strcpy(time_info_t.time_zone, JSON_GET_OBJECT_VALUE(zone_json,string));
		//DMCLOG_D("time_info_t.time_value = %s,time_info_t.time_zone = %s",time_info_t.time_value, time_info_t.time_zone);
		int ret = sync_time(&time_info_t, c);
		if(ret < 0)
		{
			goto EXIT;
		}
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}

int _dm_get_ota_info(struct conn *c)
{
	int ret = get_ota_info(c);
	if(ret < 0)
	{
		goto  EXIT;
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}


int _dm_get_version_flag(struct conn *c)
{
	int ret = get_version_flag(c);
	if(ret < 0)
	{
		goto  EXIT;
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
	
}

int _dm_set_version_flag(struct conn *c)
{
	int ret = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		ret = -1;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	JObj *version_flag_json = JSON_GET_OBJECT(para_json,"version_flag");
	if(version_flag_json != NULL)
	{
		int ret = set_version_flag(JSON_GET_OBJECT_VALUE(version_flag_json,int),c);
		if(ret < 0)
		{	
			goto EXIT;
		}
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}

int _dm_get_fw_version(struct conn *c)
{
	int ret = get_fw_version(c);
	if(ret < 0)
	{
		goto EXIT;
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}

int Parser_RouterGetWifiSettings(struct conn *c)
{
	return 0;
}
int Parser_RouterSetWifiSettings(struct conn *c)
{
	return 0;
}
int Parser_RouterGetRemoteInfo(struct conn *c)
{
	return 0;
}
int Parser_RouterSetRemoteInfo(struct conn *c)
{
	return 0;
}
int Parser_RouterGetWlanConMode(struct conn *c)
{
	return 0;
}
int Parser_RouterGetWireConMode(struct conn *c)
{
	return 0;
}
int Parser_RouterSetWireConMode(struct conn *c)
{
	return 0;
}
int Parser_RouterGetWlanList(struct conn *c)
{}
int Parser_RouterGetPower(struct conn *c)
{return 0;}
int Parser_RouterGetStorageInfo(struct conn *c)
{return 0;}
int Parser_RouterFormatDisk(struct conn *c)
{return 0;}
int Parser_RouterGetFtpSettings(struct conn *c)
{return 0;}
int Parser_RouterSetFtpSettings(struct conn *c)
{return 0;}
int Parser_RouterGetSMBSettings(struct conn *c)
{return 0;}
int Parser_RouterSetSMBSettings(struct conn *c)
{return 0;}
int Parser_RouterGetDMSSettings(struct conn *c)
{return 0;}
int Parser_RouterSetDMSSettings(struct conn *c)
{return 0;}
int Parser_RouterGetDDNSSettings(struct conn *c)
{return 0;}
int Parser_RouterSetDDNSSettings(struct conn *c)
{return 0;}
int Parser_RouterGetWebDavSettings(struct conn *c)
{return 0;}
int Parser_RouterSetWebDavSettings(struct conn *c)
{return 0;}
int Parser_RouterGetElecSettings(struct conn *c)
{return 0;}
int Parser_RouterSetElecSettings(struct conn *c)
{return 0;}
int Parser_RouterGet3gSettings(struct conn *c)
{return 0;}
int Parser_RouterSet3gSettings(struct conn *c)
{return 0;}
int Parser_RouterGetClientStatus(struct conn *c)
{return 0;}
int Parser_RouterSetClientStatus(struct conn *c)
{return 0;}
int Parser_RouterUpgradeFw(struct conn *c)
{return 0;}
int Parser_RouterGetInternetStatus(struct conn *c)
{return 0;}
int Parser_RouterSyncTime(struct conn *c)
{return 0;}
int Parser_RouterGetFwVersion(struct conn *c)
{return 0;}
int Parser_RouterGetOtaInfo(struct conn *c)
{return 0;}
int Parser_RouterGetVersionFlag(struct conn *c)
{return 0;}
int Parser_RouterSetVersionFlag(struct conn *c)
{return 0;}

