

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
#include <unistd.h>     /*Unix 鏍囧噯鍑芥暟瀹氫箟*/
#include <sys/types.h>  
#include <locale.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include "file_json.h"
#include "hd_wifi.h"
#include "hd_net.h"
#include "hd_route.h"
#include "cloud_errno.h"
#include "router_defs.h"

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
	else if(ROUTER_ERROR_FILE_NOT_EXIST == ret_value)
		error_code= FILE_IS_NOT_EXIST;
	else if(ROUTER_ERRORS_ADD_NETWORK == ret_value)
		error_code= DM_ERROR_ADD_NETWORK_NO_FIND;
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
	JObj* wifi_info[2];
    wifi_info[0] = JSON_NEW_EMPTY_OBJECT();
    wifi_info[1] = JSON_NEW_EMPTY_OBJECT();   

	ret = router_get_wifi_band();
	if(ret <= 0)
	{
		c->error = DM_ERROR_UCI;
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
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int _set_wifi_settings(hd_wifi_info *m_wifi_info,struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	int ret = -1;
	ret = dm_set_wifi_settings(m_wifi_info);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}
int get_remote_ap_info(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
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
	
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}
int set_remote_ap_info(hd_remoteap_info *m_remoteap_info,struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	int ret = -1;
	ret = dm_set_remote_ap_info(m_remoteap_info);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int _set_add_network(hd_remoteap_info *m_remoteap_info,struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int ret = -1;
	ret = dm_set_add_network(m_remoteap_info);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(c->ver,string));
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
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}
int set_wired_con_mode(wired_con_mode_array *m_wired_con_array,struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	int ret = -1;
	ret = dm_set_wired_con_mode(m_wired_con_array);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}
int get_wired_con_mode(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
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
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int get_wlan_list(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
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
			JSON_ADD_OBJECT(ap_info[i], "wifi_signal",JSON_NEW_OBJECT(ap_list_info.ap_info[i].wifi_signal,int));
			JSON_ADD_OBJECT(ap_info[i], "record",JSON_NEW_OBJECT(ap_list_info.ap_info[i].record,int));
			if(ap_list_info.ap_info[i].record)
				JSON_ADD_OBJECT(ap_info[i], "password",JSON_NEW_OBJECT(ap_list_info.ap_info[i].password, string));
			JSON_ARRAY_ADD_OBJECT (response_data_array,ap_info[i]);
		}
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}
	else
	{
		c->error = dm_router_errors(list_ret);
	}
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int get_power(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
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
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int get_client_status(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_get_client_status();
	if(ret >= 0)
	{
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj* client_json = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(client_json, "disable",JSON_NEW_OBJECT(ret,boolean));
		JSON_ARRAY_ADD_OBJECT (response_data_array,client_json);
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}else{
		c->error = dm_router_errors(ret);
	}
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int set_client_status(int status,struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_set_client_status(status);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int set_udisk_upgrade(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_udisk_upgrade();
	if(ret < 0)
	{
		c->error = ERROR_UPGRADE_FW;
	}
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int sync_time(dm_time_info *time_info_t, struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_sync_time(time_info_t);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}


int sync_system(system_sync_info_t *p_system_sync_info, struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_sync_system(p_system_sync_info);
	if(ret < 0){
		c->error = dm_router_errors(ret);
	}
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}


int get_ota_info(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
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
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}


int get_version_flag(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
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
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}


int set_version_flag(int version_flag,struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_set_version_flag(version_flag);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int get_fw_version(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	char fw_version[32];
	memset(fw_version, 0, 32);
	int ret = dm_get_fw_version(fw_version);
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
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int set_upgrade_fw(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_upgrade_fw();
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int set_forget_wifi_info(forget_wifi_info_t *p_forget_wifi_info, struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_set_forget_wifi_info(p_forget_wifi_info);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}


int get_disk_direction(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	int ret = -1;
	int detect_st = 0;
	int disk_st = 0;

	#ifdef MCU_COMMUNICATE_SUPPORT
	//ret = dm_get_storage_dir(&detect_st);
	if(ret < 0){
		c->error = dm_router_errors(ret);
	}

	ret = dm_get_file_storage(&disk_st);
	if(ret < 0){
		c->error = dm_router_errors(ret);
	}
	else{		
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj* disk_direction_json = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(disk_direction_json, "detect_st",JSON_NEW_OBJECT(detect_st,int));
		JSON_ADD_OBJECT(disk_direction_json, "disk_st",JSON_NEW_OBJECT(disk_st,int));
		JSON_ARRAY_ADD_OBJECT (response_data_array, disk_direction_json);		
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}
	#else
	c->error = DM_ERROR_MCU_IOCTL_ERROR;
	#endif

	file_json_to_string(c,response_json);

	JSON_PUT_OBJECT(response_json);
	return 0;
}

int set_disk_direction(int disk_st, struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_set_disk_direction(disk_st);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}
	
	file_json_to_string(c,response_json);

	JSON_PUT_OBJECT(response_json);
	return 0;
}

int get_wifi_type(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	int ret = -1;
	int wifi_type = 0;

	ret = dm_get_wifi_type(&wifi_type);	
	if(ret < 0){
		c->error = dm_router_errors(ret);
	}
	else{
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj* wifi_type_json = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(wifi_type_json, "wifi_type",JSON_NEW_OBJECT(wifi_type,int));
		JSON_ARRAY_ADD_OBJECT (response_data_array, wifi_type_json);	
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}
	
	file_json_to_string(c,response_json);

	JSON_PUT_OBJECT(response_json);
	return 0;
}


int set_wifi_type(int wifi_type,struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_set_wifi_type(wifi_type);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}
	
	file_json_to_string(c,response_json);
	
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int get_safe_exit(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	int safe_exit_flag;
	int ret = dm_get_safe_exit(&safe_exit_flag);
	if(ret >= 0)
	{
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj* version_flag_json = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(version_flag_json, "safe_exit_flag",JSON_NEW_OBJECT(safe_exit_flag,int));
		JSON_ARRAY_ADD_OBJECT (response_data_array,version_flag_json);
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}else{
		c->error = dm_router_errors(ret);
	}
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}


int set_safe_exit(int safe_exit_flag, struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_set_safe_exit(safe_exit_flag);
	if(ret < 0)
	{
		c->error = dm_router_errors(ret);
	}
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}


int _dm_get_wifi_settings(struct conn *c)
{
	_get_wifi_settings(c);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;	
}
int _dm_set_wifi_settings(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
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
			if(_set_wifi_settings(&m_wifi_info,c) < 0)
			{
				goto EXIT;
			}
		}
		else{
			DMCLOG_D("access null");
			goto EXIT;
		}
	}
	else if(para_5G_json != NULL)
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
			if(_set_wifi_settings(&m_wifi_info,c) < 0)
			{
				goto EXIT;
			}
		}
		else{
			DMCLOG_D("access null");
			goto EXIT;
		}
	}
	else{
		DMCLOG_D("access null");
		goto EXIT;
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;	
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
		else{
			DMCLOG_D("access null");
			ret = -1;
			goto EXIT;
		}
	}
	else{
		DMCLOG_D("access null");
		ret = -1;
		goto EXIT;
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;	
}

int _dm_set_add_network(struct conn *c)
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
		JObj *ssid_json = JSON_GET_OBJECT(para_json,"ssid");
		JObj *password_json = JSON_GET_OBJECT(para_json,"password");

		if(ssid_json != NULL&&password_json != NULL)
		{
			hd_remoteap_info m_remoteap_info;
			memset(&m_remoteap_info,0,sizeof(hd_remoteap_info));
			strcpy(m_remoteap_info.ssid,JSON_GET_OBJECT_VALUE(ssid_json,string));
			strcpy(m_remoteap_info.password,JSON_GET_OBJECT_VALUE(password_json,string));
			ret = _set_add_network(&m_remoteap_info, c);
			if(ret < 0){
				goto EXIT;
			}
		}
		else{
			DMCLOG_D("access null");
			ret = -1;
			goto EXIT;
		}
	}
	else{
		DMCLOG_D("access null");
		ret = -1;
		goto EXIT;
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
	}
	else{
		DMCLOG_D("access null");
		ret = -1;
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
//4.26	获取client功能状态  cmd = 0x0219
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
//4.27	设置client功能状态  cmd = 0x021A
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
	JObj *status_json = JSON_GET_OBJECT(para_json,"disable");
	if(status_json != NULL)
	{
		int ret = set_client_status(JSON_GET_OBJECT_VALUE(status_json,int),c);
		if(ret < 0)
		{	
			goto EXIT;
		}
	}
	else{
		DMCLOG_D("access null");
		ret = -1;
		goto EXIT;
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}

//4.28	执行从U盘升级  cmd = 0x021B
int _dm_udisk_upgrade(struct conn *c)
{
	int ret = set_udisk_upgrade(c);
	if(ret < 0)
	{
		goto EXIT;
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}

//4.30	同步时间  cmd = 0x021D
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
	else{
		DMCLOG_D("access null");
		ret = -1;
		goto EXIT;
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}

int _dm_sync_system(struct conn *c)
{
	int ret = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL){
		ret = -1;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL){
		ret = -1;
		goto EXIT;
	}
	
	JObj *sync_json = JSON_GET_OBJECT(para_json,"sync");
	if(sync_json == NULL){
		ret = -1;
		goto EXIT;
	}
	
	JObj *clean_cache_json = JSON_GET_OBJECT(para_json,"clean_cache");
	if(clean_cache_json == NULL){
		ret = -1;
		goto EXIT;
	}
	
	if(sync_json != NULL && clean_cache_json != NULL)
	{
		system_sync_info_t system_sync_t;
		memset(&system_sync_t,0,sizeof(system_sync_t));
		system_sync_t.sync = JSON_GET_OBJECT_VALUE(sync_json, int);
		system_sync_t.clean_cache = JSON_GET_OBJECT_VALUE(clean_cache_json, int);
		int ret = sync_system(&system_sync_t, c);
		if(ret < 0)
		{
			goto EXIT;
		}
	}
	else{
		DMCLOG_D("access null");
		ret = -1;
		goto EXIT;
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
		goto EXIT;
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
	else{
		DMCLOG_D("access null");
		ret = -1;
		goto EXIT;
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


int _dm_set_upload_fw(struct conn *c)
{
	ENTER_FUNC();
	int res = 0;
	struct stat	st;
	if (c->rem.content_len == 0) {
		c->error = LENGTH_REQUIRED;
		goto EXIT;
	} else if ((c->loc.chan.vf = bfavfs_fopen(c->tmp_path, "W", (char *)c->token)) == NULL) {
		c->loc.flags |= FLAG_CLOSED;
		c->error = CREATE_FILE_ERROR;
		goto EXIT;
	} else {
	
		DMCLOG_D("PUT file [%s]", c->des_path);
		c->loc.io_class = &io_file;
		//c->loc.flags |= FLAG_W | FLAG_ALWAYS_READY ;
		c->loc.flags |= FLAG_W;
		DMCLOG_D("upload c->offset = %lld",c->offset);
		(void) bfavfs_fseek(c->loc.chan.vf, c->offset, SEEK_SET);
		if(bfavfs_fstat(c->cfg_path, &st,c->token) != 0)
		{
			//如果记录文件块起始地址的配置文件不存在，则将信息写入
			int i = 0;
			off_t percent = c->fileSize/THREAD_COUNT;
			c->dn = NULL;
			for(; i < THREAD_COUNT;i++)
		    {
		        if(i == THREAD_COUNT - 1)
					add_record_to_list(&c->dn,i,i * percent,c->fileSize);
		        else
					add_record_to_list(&c->dn,i,i * percent,(i + 1)* percent);
		    }
			display_record_dnode(c->dn);
		}else{
			c->dn = NULL;
			if(read_list_from_file(c->cfg_path,&c->dn,c->token) < 0)
			{
				c->error = ERROR_FILE_UPLOAD_CHECK;
				goto EXIT;
			}
		}
		if((c->record_fd = bfavfs_fopen(c->cfg_path,"w+",c->token))== NULL)
		{
			DMCLOG_D("open file error[errno = %d]",errno);
			c->error = ERROR_FILE_UPLOAD_CHECK;
			goto EXIT;
		}
	}
EXIT:	
	EXIT_FUNC();
	return 0;
}

int _dm_set_upgrade_fw(struct conn *c)
{
	int ret = set_upgrade_fw(c);
	if(ret < 0)
	{
		goto EXIT;
	}
	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}

int _dm_set_forget_wifi_info(struct conn *c)
{

	int ret = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		ret = -1;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	JObj *mac_json = JSON_GET_OBJECT(para_json,"mac");
	JObj *ssid_json = JSON_GET_OBJECT(para_json,"ssid");
	if(mac_json != NULL && ssid_json != NULL)
	{	
		forget_wifi_info_t dm_forget_wifi_info;
		memset(&dm_forget_wifi_info, 0, sizeof(forget_wifi_info_t));
		strcpy(dm_forget_wifi_info.ssid, JSON_GET_OBJECT_VALUE(ssid_json,string));
		strcpy(dm_forget_wifi_info.mac, JSON_GET_OBJECT_VALUE(mac_json,string));
		int ret = set_forget_wifi_info(&dm_forget_wifi_info, c);
		if(ret < 0)
		{	
			goto EXIT;
		}
	}
	else{
		DMCLOG_D("access null");
		ret = -1;
		goto EXIT;
	}

	EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}


int _dm_get_disk_direction(struct conn *c)
{
	int ret = get_disk_direction(c);
	if(ret < 0)
	{
		goto  EXIT;
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}


int _dm_set_disk_direction(struct conn *c)
{
	int ret = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		ret = -1;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	JObj *disk_st_json = JSON_GET_OBJECT(para_json,"disk_st");
	if(disk_st_json != NULL)
	{
		int ret = set_disk_direction(JSON_GET_OBJECT_VALUE(disk_st_json,int), c);
		if(ret < 0)
		{	
			goto EXIT;
		}
	}
	else{
		DMCLOG_D("access null");
		ret = -1;
		goto EXIT;
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}


int _dm_get_wifi_type(struct conn *c)
{
	int ret = get_wifi_type(c);
	if(ret < 0)
	{
		goto  EXIT;
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}


int _dm_set_wifi_type(struct conn *c)
{
	int ret = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		ret = -1;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	JObj *wifi_type_json = JSON_GET_OBJECT(para_json,"wifi_type");
	if(wifi_type_json != NULL)
	{
		int ret = set_wifi_type(JSON_GET_OBJECT_VALUE(wifi_type_json,int), c);
		if(ret < 0)
		{	
			goto EXIT;
		}
	}
	else{
		DMCLOG_D("access null");
		ret = -1;
		goto EXIT;
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	DMCLOG_D("ret  = %d", ret);
	return ret;
}


int _dm_set_password(struct conn *c)
{
	#if 0
	JObj* response_json = NULL;
	
	int ret = dm_set_password(c->password);
	if(ret != 0)
	{
		c->error = ERROR_PASSWORD_SET;
		goto EXIT;
	}
	dm_cycle_change_inotify(pwd_changed);					
	DMCLOG_D("***********************pwd have change***********************");
	if(c->ctx->token_reset != NULL)
		c->ctx->token_reset(false,&c->ctx->p_token_list);
EXIT:
	response_json=JSON_NEW_EMPTY_OBJECT();
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	#endif
	return 0;
	
}

int _dm_reset_password(struct conn *c)
{
	#if 0
	JObj* response_json = NULL;

	int ret = dm_reset_password(c->password,c->new_password);
	if(ret != 0)
	{
		c->error = ERROR_PASSWORD_OLD;
		goto EXIT;
	}
	dm_cycle_change_inotify(pwd_changed);					
	DMCLOG_D("***********************pwd have change***********************");
	if(c->ctx->token_reset != NULL)
		c->ctx->token_reset(false,&c->ctx->p_token_list);
EXIT:
	response_json=JSON_NEW_EMPTY_OBJECT();
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	#endif
	return 0;
}

int _dm_get_pwd_status(struct conn *c)
{
	#if 0
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	int ret = dm_password_exist();
	if(ret >= 0)
	{
		JObj *response_data_array = JSON_NEW_ARRAY();
		JObj* pwd_flag_json = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(pwd_flag_json, "status",JSON_NEW_OBJECT(dm_password_exist(),int));
		JSON_ARRAY_ADD_OBJECT (response_data_array,pwd_flag_json);
		JSON_ADD_OBJECT(response_json, "data", response_data_array);
	}
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	#endif
	return 0;
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
int Parser_RouterSetAddNetwork(struct conn *c)
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
/*int Parser_RouterFormatDisk(struct conn *c)
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
{return 0;}*/
int Parser_RouterGetClientStatus(struct conn *c)
{return 0;}
int Parser_RouterSetClientStatus(struct conn *c)
{return 0;}
int Parser_RouterUdiskUpgrade(struct conn *c)
{return 0;}
int Parser_RouterSyncTime(struct conn *c)
{return 0;}
int Parser_RouterSyncSystem(struct conn *c)
{return 0;}
int Parser_RouterGetFwVersion(struct conn *c)
{return 0;}
int Parser_RouterGetOtaInfo(struct conn *c)
{return 0;}
int Parser_RouterGetVersionFlag(struct conn *c)
{return 0;}
int Parser_RouterSetVersionFlag(struct conn *c)
{return 0;}


/*
 * upload firmware
 */
int Parser_RouterSetUploadFw(struct conn *c)
{	
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}

	JObj *fileSize_json = JSON_GET_OBJECT(para_json,"fileSize");
	if(fileSize_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	c->fileSize = JSON_GET_OBJECT_VALUE(fileSize_json,int64);
	c->modifyTime = 0;
	c->offset = 0;
	c->length = c->fileSize;
	
	c->des_path = (char *)calloc(1,strlen(FW_FILE) + 1);
	if(c->des_path == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = SERVER_OUT_MEMORY;
		goto EXIT;
	}
	sprintf(c->des_path,"%s",FW_FILE);
	DMCLOG_D("des_path = %s",c->des_path);

	if(is_file_exist_ext(c->des_path) > 0)
	{
		DMCLOG_D("the upgrade file is exist and need to delete");
		rm(c->des_path); 	
	}
	
	c->cfg_path = (char *)malloc(strlen(c->des_path)+strlen(CFG_PATH_NAME)+1);
	if(c->cfg_path == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = SERVER_OUT_MEMORY;
		goto EXIT;
	}
	sprintf(c->cfg_path,"%s%s",c->des_path, CFG_PATH_NAME);
	DMCLOG_D("cfg_path = %s",c->cfg_path);

	c->tmp_path = (char *)malloc(strlen(c->des_path)+strlen(TMP_PATH_NAME)+1);
	if(c->tmp_path == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = SERVER_OUT_MEMORY;
		goto EXIT;
	}
	sprintf(c->tmp_path,"%s%s",c->des_path, TMP_PATH_NAME);
	DMCLOG_D("tmp_path = %s",c->tmp_path);
	c->rem.content_len = c->length - c->offset;
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0; 
}

int Parser_RouterSetUpgradeFw(struct conn *c)
{return 0;}

int Parser_RouterSetForgetWifiInfo(struct conn *c)
{return 0;}

int Parser_RouterSetPassword(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}

	JObj *password_json = JSON_GET_OBJECT(para_json,"password");
	if(password_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	strcpy(c->password,JSON_GET_OBJECT_VALUE(password_json,string));
	
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0; 
}

int Parser_RouterResetPassword(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}

	JObj *password_json = JSON_GET_OBJECT(para_json,"password");
	if(password_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	strcpy(c->password,JSON_GET_OBJECT_VALUE(password_json,string));

	JObj *new_password_json = JSON_GET_OBJECT(para_json,"new_password");
	if(new_password_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	strcpy(c->new_password,JSON_GET_OBJECT_VALUE(new_password_json,string));
	
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0; 
}

int Parser_RouterGetDiskDirection(struct conn *c)
{return 0;}

int Parser_RouterSetDiskDirection(struct conn *c)
{return 0;}

int Parser_RouterGetWifiType(struct conn *c)
{return 0;}

int Parser_RouterSetWifiType(struct conn *c)
{return 0;}


