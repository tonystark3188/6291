#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <netinet/in.h>
#include <pwd.h>
#include <crypt.h>
#include <shadow.h>

#include "uci_api.h"
#include "net_config.h"
#include "route.h"

int get_string_only(char *src)
{
if(src==NULL)
	return -1;
char *dst=src;
int str_len=strlen(src);
int i;
if(src[str_len-1]=='\''||src[str_len-1]=='\"')
	src[str_len-1]=0;
if(src[0]=='\''||src[0]=='\"')
	{
	for(i=0;i<str_len-1;i++)
		dst[i]=src[i+1];
	dst[i]=0;
	}
return 0;
}
int _router_get_wifi_band(void)
{
	int enRet;
	int ret;
	int band1 = 0;
	int band2 = 0;
	char temp_buf[TEMP_BUFFER_SIZE];
    char cmd_buf[TEMP_BUFFER_SIZE];
	
	memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
    sprintf(cmd_buf, "wireless.@wifi-device[0].band");
	memset(temp_buf, 0, TEMP_BUFFER_SIZE);
	enRet = uci_get_option_value(cmd_buf, &temp_buf);
	if(-1 == enRet)
    {
        band1 = 0;
    }
	if(!strcmp(temp_buf, "2.4G"))
	{
    	band1 = 0x01;
    }else
    {
		band1 = 0;
	}

	memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
    sprintf(cmd_buf, "wireless.@wifi-device[1].band");
	memset(temp_buf, 0, TEMP_BUFFER_SIZE);
	enRet = uci_get_option_value(cmd_buf, &temp_buf);
	if(-1 == enRet)
    {
        band2 = 0;
    }
	if(!strcmp(temp_buf, "5G"))
	{
    	band2 = 0x02;
    }else
    {
		band2 = 0;
	}

	ret = band1 | band2;
	return ret;
}


/*******************************************************************************
 * Function:
 * int get_wifi_settings(hd_wifi_info *m_wifi_info)
 * Description:
 * get hidisk wifi information
 * Parameters:
 *    m_wifi_info   [OUT] wifi para
 * Returns:
 *    0:success,-1:others,-2:uci error,-3:cmd data error,-4:shell handle error
 *******************************************************************************/
int get_wifi_settings(hd_wifi_info *m_wifi_info)
{
	DMCLOG_D("get_wifi_settings");
    int ret;
    int iface = 0;
	FILE *read_fp = NULL;
	char tmp_mac[32]="\0";
    char temp_buf[TEMP_BUFFER_SIZE];
    char cmd_buf[TEMP_BUFFER_SIZE];
	char mac_buffer[TEMP_BUFFER_SIZE];

    if(m_wifi_info->wifi_type == 1)
    {
        iface = 0;
    }else{
        iface = 2;
    }

    memset(cmd_buf,0,TEMP_BUFFER_SIZE);
    sprintf(cmd_buf,"wireless.@wifi-iface[%d].ssid",iface);
    ret = uci_get_option_value(cmd_buf, m_wifi_info->ssid);
    if(-1 == ret)
    {
    	DMCLOG_D("uci get ssid error");
        return ROUTER_ERRORS_UCI;
    }

	memset(cmd_buf,0,TEMP_BUFFER_SIZE);
    sprintf(cmd_buf,"wireless.@wifi-iface[%d].encryption",iface);
    memset(temp_buf,0,TEMP_BUFFER_SIZE);
    ret = uci_get_option_value(cmd_buf, &temp_buf);
    if(-1 != ret)
    {
        get_string_only(&temp_buf);
        if(!strcmp(temp_buf,"none"))
        {
            strcpy(m_wifi_info->encrypt,"NONE");
        }
        else if(!strcmp(temp_buf,"psk2"))
        {
            strcpy(m_wifi_info->encrypt,"WPA2");
        }
        else if(!strcmp(temp_buf,"psk"))
        {
            strcpy(m_wifi_info->encrypt,"WPA");
        }
        else if(!strcmp(temp_buf,"psk-mixed"))
        {
            strcpy(m_wifi_info->encrypt,"WPA/WPA2");
        }
        else if(!strcmp(temp_buf,"wep"))
        {
            strcpy(m_wifi_info->encrypt,"WEP");
        }
		else
		{
			DMCLOG_D("have no encrypt of %s", temp_buf);
        	return ROUTER_ERRORS_CMD_DATA;
		}
    }
	else
	{
		DMCLOG_D("uci get encrypt error");
        return ROUTER_ERRORS_UCI;
	}

	#if 0
    memset(temp_buf,0,TEMP_BUFFER_SIZE);
    is_exit = uci_get_option_value("wireless.mt7628.channel",&temp_buf);
    if(-1 != is_exit)
    {
        get_string_only(&temp_buf);
        m_wifi_info->channel = atoi(temp_buf);
    }
	else
	{
		DMCLOG_D("uci get channel error");
        return -1;
	}
	#endif
	
    memset(cmd_buf,0,TEMP_BUFFER_SIZE);
    sprintf(cmd_buf,"wireless.@wifi-iface[%d].key",iface);
    ret = uci_get_option_value(cmd_buf, m_wifi_info->wifi_password);
    if(-1 == ret)
    {
    	DMCLOG_D("uci get wifi_password error");
        return ROUTER_ERRORS_UCI;
    }
	
	memset(cmd_buf,0,TEMP_BUFFER_SIZE);
	memset(temp_buf,0,TEMP_BUFFER_SIZE);
    sprintf(cmd_buf,"wireless.@wifi-iface[%d].disabled",iface);
	DMCLOG_D("cmd_buf = %s", cmd_buf);
    ret = uci_get_option_value(cmd_buf, temp_buf);
    if(-1 == ret)
    {
    	DMCLOG_D("uci get ssid error");
        return ROUTER_ERRORS_UCI;
    }
	else
	{
		if(!strcmp("0", temp_buf))
			m_wifi_info->disabled = 0;
		else
			m_wifi_info->disabled = 1;
	}
   
	read_fp = popen("hexdump -s 4 -n 6 -C /dev/mtd3 | head -n 1 | sed 's/\ \ /:/g' | cut -d: -f 2 | sed 's/\ /:/g' | tr \"[a-z]\" \"[A-Z]\"", "r");
	if(read_fp != NULL)
	{
		memset(tmp_mac,0,32);
		fgets(tmp_mac, 32-1, read_fp);
	}
	strncpy(m_wifi_info->mac,tmp_mac,17);
	pclose(read_fp);

	return ROUTER_OK;
}


/*******************************************************************************
 * Function:
 * int set_wifi_settings(hd_wifi_info *m_wifi_info)
 * Description:
 * get hidisk wifi information
 * Parameters:
 *    m_wifi_info   [OUT] wifi para
 * Returns:
 *    0:success,-1:others,-2:uci error,-3:cmd data error
 *******************************************************************************/
int set_wifi_settings(hd_wifi_info *m_wifi_info)
{
	DMCLOG_D("access set_wifi_settings");
	int ret = -1;
	char temp_buf[TEMP_BUFFER_SIZE];
    char encrypt_config[16]="\0";
    char tkip_aes_config[8]="\0";
    int iface = 0;
    if(m_wifi_info->wifi_type == 1)
    {
        iface = 0;
    }else{
        iface = 2;
    }
    if(m_wifi_info->encrypt != NULL&&*m_wifi_info->encrypt)
    {
        if(!strcmp(m_wifi_info->encrypt,"NONE"))
            strcpy(encrypt_config,"none");
        else if(!strcmp(m_wifi_info->encrypt,"WEP"))
            strcpy(encrypt_config,"wep");
        else if(!strcmp(m_wifi_info->encrypt,"WPA"))
            strcpy(encrypt_config,"psk");
        else if(!strcmp(m_wifi_info->encrypt,"WPA2"))
            strcpy(encrypt_config,"psk2");
        else if(!strcmp(m_wifi_info->encrypt,"WPA/WPA2"))
            strcpy(encrypt_config,"mixed-psk");
    }else{
        return ROUTER_ERRORS_CMD_DATA;
    }

    if(m_wifi_info->ssid != NULL&&*m_wifi_info->ssid)//set ssid
    {
        memset(temp_buf,0,TEMP_BUFFER_SIZE);
        sprintf(temp_buf,"wireless.@wifi-iface[%d].ssid=%s",iface,m_wifi_info->ssid);
        ret = uci_set_option_value(temp_buf);
		if(-1 == ret)
		{
			return ROUTER_ERRORS_UCI;
		}
    }else{
        return ROUTER_ERRORS_CMD_DATA;
    }
	
    if(!strcmp(encrypt_config,"none") || !strcmp(encrypt_config,"wep"))
    {
        memset(temp_buf,0,TEMP_BUFFER_SIZE);
        sprintf(temp_buf,"wireless.@wifi-iface[%d].encryption=%s",iface,encrypt_config);
        ret = uci_set_option_value(temp_buf);
		if(-1 == ret)
		{
			return ROUTER_ERRORS_UCI;
		}
    }
    else if(!strcmp(encrypt_config,"psk") || !strcmp(encrypt_config,"psk2") || !strcmp(encrypt_config,"mixed-psk"))
    {
  	    if(m_wifi_info->tkip_aes != NULL&&*m_wifi_info->tkip_aes)
	    {
	        if(!strcmp(m_wifi_info->tkip_aes,"tkip"))
	            strcpy(tkip_aes_config,"tkip");
	        else if(!strcmp(m_wifi_info->tkip_aes,"aes"))
	            strcpy(tkip_aes_config,"aes");
	        else if(!strcmp(m_wifi_info->tkip_aes,"tkip/aes"))
	            strcpy(tkip_aes_config,"tkip+aes");
	    }else{
	        //return -1;
	    }
        if(strlen(tkip_aes_config)>0)
        {
            memset(temp_buf,0,TEMP_BUFFER_SIZE);
            sprintf(temp_buf,"wireless.@wifi-iface[%d].encryption=%s",iface,encrypt_config);
            ret = uci_set_option_value(temp_buf);
			if(-1 == ret)
			{
				return ROUTER_ERRORS_UCI;
			}

			memset(temp_buf,0,TEMP_BUFFER_SIZE);
            sprintf(temp_buf,"wireless.@wifi-iface[%d].wpa_crypto=%s",iface,tkip_aes_config);
            ret = uci_set_option_value(temp_buf);
			if(-1 == ret)
			{
				return ROUTER_ERRORS_UCI;
			}
        }

		if(m_wifi_info->wifi_password != NULL&&*m_wifi_info->wifi_password)
	    {
	        memset(temp_buf,0,TEMP_BUFFER_SIZE);
	        sprintf(temp_buf,"wireless.@wifi-iface[%d].key=%s",iface,m_wifi_info->wifi_password);
	        ret = uci_set_option_value(temp_buf);
			if(-1 == ret)
			{
				return ROUTER_ERRORS_UCI;
			}
	    }else{
	        return ROUTER_ERRORS_CMD_DATA;
	    }
    }
	else
	{
		return ROUTER_ERRORS_CMD_DATA;
	}
	
    memset(temp_buf,0,TEMP_BUFFER_SIZE);
    sprintf(temp_buf,"wireless.@wifi-iface[%d].disabled=%d",iface,m_wifi_info->disabled);
    ret = uci_set_option_value(temp_buf);
	if(-1 == ret)
	{
		return ROUTER_ERRORS_UCI;
	}
	
    if(m_wifi_info->channel >= 0)
    {
        memset(temp_buf,0,TEMP_BUFFER_SIZE);
        sprintf(temp_buf,"wireless.mt7628.channel=%d",m_wifi_info->channel);
        ret = uci_set_option_value(temp_buf);
		if(-1 == ret)
		{
			return ROUTER_ERRORS_UCI;
		}
    }
	
    system("uci commit");
    system("/usr/mips/cgi-bin/script/control_newshair.sh stop");
    system("wifi >/dev/null 2>&1");
    sleep(5);
	system("/usr/mips/cgi-bin/script/control_newshair.sh start");
    return ROUTER_OK;
}



int get_remote_ap(hd_remoteap_info *m_remote_info)
{
	int socket_id;
	struct ifreq ifr_remote_ap;
	int tmp = 0;
	int ret = 0;
	int inet_sock_remote_ap;
	char channel[4] = "\0";
	char data[128];
	char cmd_buf[TEMP_BUFFER_SIZE];
    char temp_buf[TEMP_BUFFER_SIZE];
	
	memset(cmd_buf,0,TEMP_BUFFER_SIZE);
    sprintf(cmd_buf,"wireless.@wifi-iface[1].ssid");
    memset(temp_buf,0,TEMP_BUFFER_SIZE);
    ret = uci_get_option_value(cmd_buf, &m_remote_info->ssid);
	if(ret < 0)
    {
        return ROUTER_ERRORS_UCI;
    }
	
	memset(cmd_buf,0,TEMP_BUFFER_SIZE);
	sprintf(cmd_buf,"wireless.@wifi-iface[1].encryption");
	memset(temp_buf,0,TEMP_BUFFER_SIZE);
	ret = uci_get_option_value(cmd_buf, &temp_buf);
	if(ret >= 0)
    {
        get_string_only(temp_buf);
        if(!strcmp(temp_buf,"none"))
        {
            strcpy(m_remote_info->encrypt,"NONE");
        }
        else if(!strcmp(temp_buf,"WPA2-PSK")||!strcmp(temp_buf,"WPA2")||!strcmp(temp_buf,"wpa2"))
        {
            strcpy(m_remote_info->encrypt,"WPA2");
        }
        else if(!strcmp(temp_buf,"WPA-PSK")||!strcmp(temp_buf,"WPA")||!strcmp(temp_buf,"WPA1")||!strcmp(temp_buf,"wpa")||!strcmp(temp_buf,"wpa1"))
        {
            strcpy(m_remote_info->encrypt,"WPA");
        }
        else if(!strcmp(temp_buf,"WPA/WPA2-PSK")||!strcmp(temp_buf,"Mixed")||!strcmp(temp_buf,"MIXED")||!strcmp(temp_buf,"mixed"))
        {
            strcpy(m_remote_info->encrypt,"WPA/WPA2");
        }
        else if(!strcmp(temp_buf,"wep"))
        {
            strcpy(m_remote_info->encrypt,"WEP");
        }
		else
		{
			strcpy(m_remote_info->encrypt, temp_buf);
			//return ROUTER_ERRORS_CMD_DATA;
		}
    }
	else
	{
		return ROUTER_ERRORS_UCI;
	}

	memset(cmd_buf,0,TEMP_BUFFER_SIZE);
	sprintf(cmd_buf,"wireless.@wifi-iface[1].wpa_crypto");
	memset(temp_buf,0,TEMP_BUFFER_SIZE);
	ret = uci_get_option_value(cmd_buf, &temp_buf);
	if(ret >= 0)
    {
        get_string_only(temp_buf);
        if(!strcmp(temp_buf,"AES"))
        {
            strcpy(m_remote_info->tkip_aes,"aes");
        }
        else if(!strcmp(temp_buf,"TKIP"))
        {
            strcpy(m_remote_info->tkip_aes,"tkip");
        }
        else if(!strcmp(temp_buf,"TKIP+AES"))
        {
            strcpy(m_remote_info->tkip_aes,"tkip/aes");
        }
		else
		{
			strcpy(m_remote_info->tkip_aes, temp_buf);
			//return ROUTER_ERRORS_CMD_DATA;
		}
    }
	else
	{
		return ROUTER_ERRORS_UCI;
	}

   	memset(cmd_buf,0,TEMP_BUFFER_SIZE);
	sprintf(cmd_buf,"wireless.mt7628.channel");
	memset(temp_buf,0,TEMP_BUFFER_SIZE);
	ret = uci_get_option_value(cmd_buf, &temp_buf);
	if(ret >= 0)
    {
        get_string_only(temp_buf);
        m_remote_info->channel = atoi(temp_buf);
    }
	else
	{
		return ROUTER_ERRORS_UCI;
	}

	memset(cmd_buf,0,TEMP_BUFFER_SIZE);
	sprintf(cmd_buf,"wireless.@wifi-iface[1].key");
	ret = uci_get_option_value(cmd_buf, &m_remote_info->password);
    if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}

	memset(cmd_buf,0,TEMP_BUFFER_SIZE);
	sprintf(cmd_buf,"wireless.@wifi-iface[1].mac");
	ret = uci_get_option_value(cmd_buf, &m_remote_info->mac);
	if(ret < 0)
	{
		DMCLOG_D("no mac");
		//return ROUTER_ERRORS_UCI;
	}

	strcpy(ifr_remote_ap.ifr_name, AEMOTE_AP_DEVICE_24G);
	inet_sock_remote_ap = socket(AF_INET, SOCK_DGRAM, 0);
	if(-1 == inet_sock_remote_ap)
	{
		return ROUTER_ERRORS_SOCKET_IOCTL;
	}
	if (ioctl(inet_sock_remote_ap, SIOCGIFADDR, &ifr_remote_ap) < 0)
	{
		m_remote_info->is_connect = 0;
	}else
	{
		m_remote_info->is_connect = 1;
	}
	close(inet_sock_remote_ap);
	
    return ROUTER_OK;
}


int set_remote_ap(hd_remoteap_info *m_remote_info)
{	
	DMCLOG_D("access set_remote_ap");
    int ret;
    char encrypt_config[16]="\0";
    char tkip_aes_config[16]="\0";
    char temp_buf[TEMP_BUFFER_SIZE];
	char cmd_buf[TEMP_BUFFER_SIZE];
    char WiredMode[8]="\0";
    //system("killall wpa_supplicant");
    //system("killall udhcpc");

	memset(cmd_buf,0,TEMP_BUFFER_SIZE);
	sprintf(cmd_buf, "wireless.@wifi-iface[1].disabled=0");
	ret = uci_set_option_value(cmd_buf);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}

	memset(cmd_buf,0,TEMP_BUFFER_SIZE);
	sprintf(cmd_buf, "network.wan.workmode=1");
	ret = uci_set_option_value(cmd_buf);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}

	memset(cmd_buf,0,TEMP_BUFFER_SIZE);
	sprintf(cmd_buf,"network.wan.proto");
	memset(temp_buf,0,TEMP_BUFFER_SIZE);
	ret = uci_get_option_value(cmd_buf, &temp_buf);
    if(ret >= 0)
    {
        get_string_only(temp_buf);
        memcpy(WiredMode, temp_buf, 8);
    }

    if(m_remote_info->encrypt != NULL&&*m_remote_info->encrypt)//encrypt
    {
        if(!strcmp(m_remote_info->encrypt,"NONE"))
            strcpy(encrypt_config,"none");
        else if(!strcmp(m_remote_info->encrypt,"WEP"))
            strcpy(encrypt_config,"wep");
        else if(!strcmp(m_remote_info->encrypt,"WPA"))
            strcpy(encrypt_config,"wpa");
        else if(!strcmp(m_remote_info->encrypt,"WPA2"))
            strcpy(encrypt_config,"wpa2");
        else if(!strcmp(m_remote_info->encrypt,"WPA/WPA2"))
            strcpy(encrypt_config,"mixed");
		else
			return ROUTER_ERRORS_CMD_DATA;
    }
	
	if( !strcmp(encrypt_config,"wpa") || !strcmp(encrypt_config,"wpa2") || !strcmp(encrypt_config,"mixed") )
	{
		if(m_remote_info->tkip_aes != NULL&&*m_remote_info->tkip_aes)//tkip_aes
	    {
	        if(!strcmp(m_remote_info->tkip_aes,"tkip"))
	            strcpy(tkip_aes_config,"TKIP");
	        else if(!strcmp(m_remote_info->tkip_aes,"aes"))
	            strcpy(tkip_aes_config,"AES");
	        else if(!strcmp(m_remote_info->tkip_aes,"tkip/aes"))
	            strcpy(tkip_aes_config,"TKIP+AES");
			else
				return ROUTER_ERRORS_CMD_DATA;
	    }
	}
	
	if(!strcmp(encrypt_config,"WEP"))
	{
		if( !strlen(m_remote_info->password) || ( strlen(m_remote_info->password)!=5 && \
			strlen(m_remote_info->password)!=10 && strlen(m_remote_info->password)!=13 && \
			strlen(m_remote_info->password)!=26 ))
		{
			return ROUTER_ERRORS_CMD_DATA;
		}
	}
   
	if(m_remote_info->ssid != NULL&&*m_remote_info->ssid)//ssid
    {
    	if(!strlen(m_remote_info->ssid) || strlen(m_remote_info->ssid)>32)
    	{
    		
			return ROUTER_ERRORS_CMD_DATA;
		}
		else
		{
			memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
	        sprintf(cmd_buf, "wireless.@wifi-iface[1].ssid=%s", m_remote_info->ssid);
	        ret = uci_set_option_value(cmd_buf);
			if(ret < 0)
			{
				return ROUTER_ERRORS_UCI;
			}
		}
    }else{
		return ROUTER_ERRORS_CMD_DATA;
    }
	
	if(m_remote_info->mac != NULL&&*m_remote_info->mac)//mac
    {
		memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
        sprintf(cmd_buf, "wireless.@wifi-iface[1].mac=%s", m_remote_info->mac);
        ret = uci_set_option_value(cmd_buf);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
	
    }else{
        //return ROUTER_ERRORS_CMD_DATA;
        system("uci delete wireless.@wifi-iface[1].mac");
    }

	if(!strcmp(encrypt_config,"none") || !strcmp(encrypt_config,"wep") || !strcmp(encrypt_config,"wpa") || \
		!strcmp(encrypt_config,"wpa2") || !strcmp(encrypt_config,"mixed"))
	{
		memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
        sprintf(cmd_buf, "wireless.@wifi-iface[1].encrypt_config=%s", encrypt_config);
        ret = uci_set_option_value(cmd_buf);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
	}
	else
	{
		return ROUTER_ERRORS_CMD_DATA;
	}

	if( !strcmp(encrypt_config,"wpa") || !strcmp(encrypt_config,"wpa2") || !strcmp(encrypt_config,"mixed") )
	{
		if(!strcmp(tkip_aes_config,"TKIP") || !strcmp(tkip_aes_config,"AES") || !strcmp(tkip_aes_config,"TKIP+AES"))
		{
			memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
			sprintf(cmd_buf, "wireless.@wifi-iface[1].wpa_crypto=%s", tkip_aes_config);
			ret = uci_set_option_value(cmd_buf);
			if(ret < 0)
			{
				return ROUTER_ERRORS_UCI;
			}
		}
		else
		{
			return ROUTER_ERRORS_CMD_DATA;
		}

	    if(m_remote_info->password!=NULL&&*m_remote_info->password)
	    {
	        memset(temp_buf,0,TEMP_BUFFER_SIZE);
	        sprintf(temp_buf,"wireless.@wifi-iface[1].key=%s",m_remote_info->password);
	        ret = uci_set_option_value(temp_buf);
			if(ret < 0)
			{
				return ROUTER_ERRORS_UCI;
			}
	    }else{
	        return ROUTER_ERRORS_CMD_DATA;
	    }
	}
	
    if(m_remote_info->channel > 0)
    {
        memset(temp_buf,0,TEMP_BUFFER_SIZE);
        sprintf(temp_buf, "wireless.mt7628.channel=%d", m_remote_info->channel);
        ret = uci_set_option_value(temp_buf);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
    }else{
        return ROUTER_ERRORS_CMD_DATA;
    }
	
    system("uci commit");
	system("`sleep 3;wifi;ifup wan` &");

    return 0;
}


int _get_wlan_con_mode()
{
	int work_mode = 0;
	int ret = -1;
	char value[4]="\0";
	char cmd_buf[TEMP_BUFFER_SIZE];
	char temp_buf[TEMP_BUFFER_SIZE];
	memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
	memset(temp_buf, 0, TEMP_BUFFER_SIZE);
	strcpy(cmd_buf, "network.wan.workmode"); 
	ret = uci_get_option_value(cmd_buf, temp_buf);
	if(ret >= 0)
	{
		work_mode = atoi(temp_buf);
	}
	else{
		work_mode = ROUTER_ERRORS_UCI;
	}
	return work_mode;
}




/*******************************************************************************
 * Function:
 * int _get_wired_con_mode(wired_con_mode_array *m_wired_con_array);
 * Description:
 * get wired 0x0206
 * Parameters:
 *    m_wired_con_array [OUT] wired connection array
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int _get_wired_con_mode(wired_con_mode_array *m_wired_con_array)
{
	DMCLOG_D("access dm_get_wired_con_mode");
	//char port[2]="0";
	int ret;
	char mode[3]="\0";
	char WiredMode[16]="\0";
	char user[32]="\0";
	char password[32]="\0";
	char ip[32]="\0";
	char mask[32]="\0";
	char gateway[32]="\0";
	char dns1[32]="\0";
	char dns2[32]="\0";
	char dns[64]="\0";
	char str_sp[64]="\0";
	char uci_option_str[64]="\0";
	int i=0;
	int j=0;
	struct uci_context *ctx;
	
	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"network.wan.proto"); 
	ret = uci_get_option_value(uci_option_str,WiredMode);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	
	if(!strcmp(WiredMode,"dhcp"))
	{
		uci_free_context(ctx);
		ctx=uci_alloc_context();
		uci_add_delta_path(ctx,"/tmp/state");
		uci_set_savedir(ctx,"/tmp/state");

		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.ipaddr"); 
		ret = uci_get_option_value(uci_option_str,ip);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
		
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.netmask"); 
		ret = uci_get_option_value(uci_option_str,mask);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
		
		memset(uci_option_str,'\0',64); 
		strcpy(uci_option_str,"network.wan.gateway"); 
		ret  = uci_get_option_value(uci_option_str,gateway);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
		
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.dns");
		ret = uci_get_option_value(uci_option_str,dns);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
				
		if( strchr(dns,' ')==NULL )
		{
			strcpy(dns1,dns);
		}
		else
		{
			while(dns[i]!=' ')
			{
				i++;
			}
			for(j=0;j<i;j++)
			{
				dns1[j]=dns[j];
			}
			for(j=0,i=i+1;i<strlen(dns);j++,i++)
			{
				dns2[j]=dns[i];
			}
		}
		uci_free_context(ctx);

		m_wired_con_array->enable = 1;
		m_wired_con_array->m_wired_mode[0].enable = 1;
		m_wired_con_array->m_wired_mode[0].con_type = DHCP_MODE;
		strcpy(m_wired_con_array->m_wired_mode[0].ip, ip);
		strcpy(m_wired_con_array->m_wired_mode[0].netmask, mask);
		strcpy(m_wired_con_array->m_wired_mode[0].gateway, gateway);
		strcpy(m_wired_con_array->m_wired_mode[0].dns1_ip, dns1);
		strcpy(m_wired_con_array->m_wired_mode[0].dns2_ip, dns2);
		
	}
	else if(!strcmp(WiredMode,"pppoe"))
	{
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.username");
		ret = uci_get_option_value(uci_option_str,user);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
		
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.password"); 
		ret = uci_get_option_value(uci_option_str,password);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
		

		uci_free_context(ctx);
		ctx=uci_alloc_context();
		uci_add_delta_path(ctx,"/tmp/state");
		uci_set_savedir(ctx,"/tmp/state");
		
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.ipaddr"); 
		ret = uci_get_option_value(uci_option_str,ip);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}

		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.netmask"); 
		ret = uci_get_option_value(uci_option_str,mask);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}

		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.gateway"); 
		ret = uci_get_option_value(uci_option_str,gateway);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
		
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.dns"); 
		ret = uci_get_option_value(uci_option_str,dns);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
		
		if( strchr(dns,' ')==NULL )
		{
			strcpy(dns1,dns);
		}
		else
		{
			while(dns[i]!=' ')
			{
				i++;
			}
			for(j=0;j<i;j++)
			{
				dns1[j]=dns[j];
			}
			for(j=0,i=i+1;i<strlen(dns);j++,i++)
			{
				dns2[j]=dns[i];
			}
		}
		uci_free_context(ctx);

		m_wired_con_array->enable = 1;
		m_wired_con_array->m_wired_mode[0].enable = 1;
		m_wired_con_array->m_wired_mode[0].con_type = PPPOE_MODE;
		strcpy(m_wired_con_array->m_wired_mode[0].adsl_name, user);
		strcpy(m_wired_con_array->m_wired_mode[0].adsl_password, password);
		strcpy(m_wired_con_array->m_wired_mode[0].ip, ip);
		strcpy(m_wired_con_array->m_wired_mode[0].netmask, mask);
		strcpy(m_wired_con_array->m_wired_mode[0].gateway, gateway);
		strcpy(m_wired_con_array->m_wired_mode[0].dns1_ip, dns1);
		strcpy(m_wired_con_array->m_wired_mode[0].dns2_ip, dns2);		
	}
	else if(!strcmp(WiredMode,"static"))
	{
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.ipaddr"); 
		ret = uci_get_option_value(uci_option_str,ip);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}

		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.netmask"); 
		ret = uci_get_option_value(uci_option_str,mask);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
		
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.gateway"); 
		ret = uci_get_option_value(uci_option_str,gateway);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
		
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.dns"); 
		ret = uci_get_option_value(uci_option_str,dns);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
		
		
		if( strchr(dns,' ')==NULL )
		{
			strcpy(dns1,dns);
		}
		else
		{
			while(dns[i]!=' ')
			{
				i++;
			}
			for(j=0;j<i;j++)
			{
				dns1[j]=dns[j];
			}
			for(j=0,i=i+1;i<strlen(dns);j++,i++)
			{
				dns2[j]=dns[i];
			}
			
		}	
		m_wired_con_array->enable = 1;
		m_wired_con_array->m_wired_mode[0].enable = 1;
		m_wired_con_array->m_wired_mode[0].con_type = STATIC_MODE;
		strcpy(m_wired_con_array->m_wired_mode[0].ip, ip);
		strcpy(m_wired_con_array->m_wired_mode[0].netmask, mask);
		strcpy(m_wired_con_array->m_wired_mode[0].gateway, gateway);
		strcpy(m_wired_con_array->m_wired_mode[0].dns1_ip, dns1);
		strcpy(m_wired_con_array->m_wired_mode[0].dns2_ip, dns2);
		
	}

	return ROUTER_OK;
}


/*******************************************************************************
 * Function:
 * int _set_wired_con_mode(wired_con_mode_array *m_wired_con_array);
 * Description:
 * set wired  0x0205
 * Parameters:
 *    m_wired_con_array [INT] wired connection array
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int _set_wired_con_mode(wired_con_mode_array *m_wired_con_array)
{
	DMCLOG_D("access set_wired_con_mode");

	int ret = -1;
	int con_type; 
	char user[64] = "\0";
	char password[64] = "\0";
	char ip[32] = "\0";
	char mask[32] = "\0";
	char gateway[32] = "\0";
	char dns1[32] = "\0";
	char dns2[32] = "\0";
	char cur_proto[32]="\0";
	char wan_hostname[32]="\0";
	
	char str_sp[64]="\0";
	char uci_option_str[64]="\0";
	int j;

	con_type = m_wired_con_array->m_wired_mode[0].con_type;
	
	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"network.wan.proto"); 
	ret = uci_get_option_value(uci_option_str,cur_proto);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	
	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"network.wan.hostname"); 
	ret = uci_get_option_value(uci_option_str,wan_hostname);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}

	//system("touch /tmp/for_auto_connect"); 
	ret = uci_set_option_value("wireless.@wifi-iface[1].disabled=1");
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	
	system("uci commit");
	system("/usr/mips/cgi-bin/script/control_newshair.sh stop");

	if(con_type == DHCP_MODE)
	{
		system("uci delete network.wan.dns");
		system("uci delete network.wan.gateway");
		system("uci delete network.wan.ipaddr");
		system("uci delete network.wan.netmask");
		system("uci delete network.wan.username");
		system("uci delete network.wan.password");
		system("uci commit");
		ret = uci_set_option_value("network.wan.proto=dhcp");
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
		if(!strcmp(cur_proto,"dhcp"))
		{
			ret = uci_set_option_value("uci set network.wan.workmode=0");
			if(ret < 0)
			{
				return ROUTER_ERRORS_UCI;
			}
			
			system("uci commit");
	//		shell_uci_set_value("brctl delif br-lan eth2.1");		
	//		shell_uci_set_value("killall udhcpc >/dev/null 2>&1");
	//		shell_uci_set_value("kill `cat /var/run/udhcpc-eth0.2.pid`");
		//	shell_uci_set_value("udhcpc -t 0 -i eth2.1  -b -p /var/run/dhcp-eth2.1.pid -O rootpath -R >/dev/null &");
			system("iwpriv apcli0 set ApCliEnable=0");
			system("ifconfig apcli0 down");

			system("/usr/mips/cgi-bin/script/control_newshair.sh start");
			return 0;
		}	
	}

	if(con_type == PPPOE_MODE)
	{
		strcpy(user, m_wired_con_array->m_wired_mode[0].adsl_name);
		strcpy(password, m_wired_con_array->m_wired_mode[0].adsl_password);
		system("uci delete network.wan.dns");
		system("uci delete network.wan.gateway");
		system("uci delete network.wan.ipaddr");
		system("uci delete network.wan.netmask");
		//shell_uci_set_value("uci set network.wan.ifname=eth2.1");
		//shell_uci_set_value("uci set network.lan.ifname=eth2.2");
		system("uci commit");
		
		ret = uci_set_option_value("network.wan.proto=pppoe");
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
		
		memset(str_sp, '\0', 64);
		sprintf(str_sp,"network.wan.username=%s",user);
		ret = uci_set_option_value(str_sp);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
		
		memset(str_sp, '\0', 64);
		sprintf(str_sp,"network.wan.password=%s",password);
		ret = uci_set_option_value(str_sp);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
	}

	if(con_type == STATIC_MODE)
	{
		strcpy(ip, m_wired_con_array->m_wired_mode[0].ip);
		strcpy(mask, m_wired_con_array->m_wired_mode[0].netmask);
		strcpy(gateway, m_wired_con_array->m_wired_mode[0].gateway);
		strcpy(dns1, m_wired_con_array->m_wired_mode[0].dns1_ip);
		strcpy(dns2, m_wired_con_array->m_wired_mode[0].dns2_ip);

		system("uci delete network.wan.username");
		system("uci delete network.wan.password");
		
		ret = uci_set_option_value("network.wan.proto=static");
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
		//shell_uci_set_value("uci set network.wan.ifname=eth2.1");
		//shell_uci_set_value("uci set network.lan.ifname=eth2.2");
		system("uci commit");

		if(ip != "\0")
		{
			memset(str_sp, '\0', 64);
			sprintf(str_sp, "network.wan.ipaddr=%s",ip);
			uci_set_option_value(str_sp);
		}
		if(mask != "\0")
		{
			memset(str_sp, '\0', 64);
			sprintf(str_sp, "network.wan.netmask=%s",mask);
			ret = uci_set_option_value(str_sp);
			if(ret < 0)
			{
				return ROUTER_ERRORS_UCI;
			}
		}
		if(gateway != "\0")
		{
			memset(str_sp, '\0', 64);
			sprintf(str_sp,"network.wan.gateway=%s",gateway);
			ret= uci_set_option_value(str_sp);
			if(ret < 0)
			{
				return ROUTER_ERRORS_UCI;
			}
		}
		if(dns1 != "\0" || dns2 != "\0")
		{
			if(dns1 != "\0" && dns2 != "\0")
			{
				memset(str_sp, '\0', 64);
				sprintf(str_sp,"network.wan.dns=%s %s",dns1,dns2);
				ret = uci_set_option_value(str_sp);
				if(ret < 0)
				{
					return ROUTER_ERRORS_UCI;
				}
			}
			else if(dns1 != "\0")
				{
					memset(str_sp, '\0', 64);
					sprintf(str_sp,"network.wan.dns=%s",dns1);
					ret = uci_set_option_value(str_sp);
					if(ret < 0)
					{
						return ROUTER_ERRORS_UCI;
					}
			}
			else
				{
					memset(str_sp, '\0', 64);
					sprintf(str_sp,"network.wan.dns=%s",dns2);
					ret = uci_set_option_value(str_sp);
					if(ret < 0)
					{
						return ROUTER_ERRORS_UCI;
					}
			}
			
		}
		
	}

	system("ifconfig apcli0 down");
	system("iwpriv apcli0 set ApCliEnable=0");
	ret = uci_set_option_value("uci set network.wan.workmode=0");
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	system("uci commit");
	system("sync");
//	shell_uci_set_value("killall wanlan");
//	sleep(1);
	system("ifup wan");

	return ROUTER_OK;
}


int _get_client_status()
{
	int ret = -1;
	char cmd_buf[TEMP_BUFFER_SIZE];
    char temp_buf[TEMP_BUFFER_SIZE];

	memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
	strcpy(cmd_buf,"wireless.@wifi-iface[1].disabled"); 
	memset(temp_buf, 0, TEMP_BUFFER_SIZE);
	ret = uci_get_option_value(cmd_buf, &temp_buf);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	else
	{
		return atoi(temp_buf);
	}
}

int _set_client_status(int status)
{
	int ret = -1;
	char cmd_buf[TEMP_BUFFER_SIZE];
    char temp_buf[TEMP_BUFFER_SIZE];
	int old_status = 0;

	memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
	sprintf(cmd_buf, "wireless.@wifi-iface[1].disabled");
	memset(temp_buf, 0, TEMP_BUFFER_SIZE);
	ret = uci_get_option_value(cmd_buf, &temp_buf);
	if(ret <0 )
	{
		return ROUTER_ERRORS_UCI;
	}
	else
	{
		old_status = atoi(temp_buf);
	}

	if(old_status != status)
	{
		if((status == 0) || (status == 1))
		{
			memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
			sprintf(cmd_buf, "wireless.@wifi-iface[1].disabled=%d",status);
			ret = uci_set_option_value(cmd_buf);
			if(ret < 0)
			{
				return ROUTER_ERRORS_UCI;
			}		
		}else
		{
			return ROUTER_ERRORS_CMD_DATA;
		}

		system("`sleep 3;wifi;ifup wan` &");
	}
	return ROUTER_OK;
}

//iptables -t nat -A POSTROUTING -s 192.168.222.0/24 -o wlan1-1 -j MASQUERADE

