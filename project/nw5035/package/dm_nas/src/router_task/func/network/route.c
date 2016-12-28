#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/socket.h>
#include <pwd.h>
#include <crypt.h>
#include <shadow.h>
#include "uci_api.h"
#include "route.h"
#include "router_defs.h"
#ifdef SUPPORT_LINUX_PLATFORM
#include "nor_control.h"
#endif

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

#if defined(SUPPORT_OPENWRT_PLATFORM)
#if defined(OPENWRT_MT7628)
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
#elif defined(OPENWRT_X1000)
	// TODO:devision 2.4G and 5G
	band1 = 0x01;//2.4G 
	band2 = 0;
	
	ret = band1 | band2;
#endif
#elif defined(SUPPORT_LINUX_PLATFORM)
	// TODO:devision 2.4G and 5G
	band1 = 0x01;//2.4G
	band2 = 0;
	
	ret = band1 | band2;
#endif
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
	char ifname[16]="\0";
	
#if defined(SUPPORT_OPENWRT_PLATFORM)
	if(m_wifi_info->wifi_type == 1){
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
    memset(temp_buf,0, sizeof(temp_buf));
    ret = uci_get_option_value(cmd_buf, &temp_buf);
    if(-1 != ret)
    {
    #if defined(OPENWRT_MT7628)
        get_string_only(&temp_buf);
        if(!strcmp(temp_buf,"none")){
            strcpy(m_wifi_info->encrypt,"NONE");
        }
        else if(!strcmp(temp_buf,"psk2")){
            strcpy(m_wifi_info->encrypt,"WPA2");
        }
        else if(!strcmp(temp_buf,"psk")){
            strcpy(m_wifi_info->encrypt,"WPA");
        }
        else if(!strcmp(temp_buf,"psk-mixed")){
            strcpy(m_wifi_info->encrypt,"WPA/WPA2");
        }
        else if(!strcmp(temp_buf,"wep")){
            strcpy(m_wifi_info->encrypt,"WEP");
        }
		else{
			DMCLOG_D("have no encrypt of %s", temp_buf);
        	return ROUTER_ERRORS_CMD_DATA;
		}
	#elif defined(OPENWRT_X1000)
		if(!strcmp(temp_buf,"none")){
			strcpy(m_wifi_info->encrypt,"NONE");
		}
		else if( !strcmp(temp_buf,"psk2+ccmp") || !strcmp(temp_buf,"psk2+tkip") || !strcmp(temp_buf,"psk2+tkip+ccmp") ){
			strcpy(m_wifi_info->encrypt,"WPA2");
		}
		else if( !strcmp(temp_buf,"psk+ccmp") || !strcmp(temp_buf,"psk+tkip") || !strcmp(temp_buf,"psk+tkip+ccmp") ){
			strcpy(m_wifi_info->encrypt,"WPA");
		}
		else if( !strcmp(temp_buf,"mixed-psk+ccmp") || !strcmp(temp_buf,"mixed-psk+tkip") || !strcmp(temp_buf,"mixed-psk+tkip+ccmp") ){
			strcpy(m_wifi_info->encrypt,"WPA/WPA2");
		}
		else if( !strcmp(temp_buf,"wep") ){
			strcpy(m_wifi_info->encrypt,"WEP");
		}
		else{
			DMCLOG_D("have no encrypt of %s", temp_buf);
        	return ROUTER_ERRORS_CMD_DATA;
		}
	#endif
    }
	else
	{
		DMCLOG_D("uci get encrypt error");
        return ROUTER_ERRORS_UCI;
	}

#if defined(OPENWRT_MT7628)
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
#elif defined(OPENWRT_X1000)
	memset(ifname, 0, sizeof(ifname));
	strcpy(ifname,"wlan0");
	memset(temp_buf, 0, TEMP_BUFFER_SIZE);
	ret = cgi_get_channel(ifname, &temp_buf); 
	if(ret == 0)
	{
		get_string_only(temp_buf);
        m_wifi_info->channel = atoi(temp_buf);
	}
#endif

	if(strcmp(m_wifi_info->encrypt,"NONE"))
	{
	    memset(cmd_buf,0,TEMP_BUFFER_SIZE);
	    sprintf(cmd_buf,"wireless.@wifi-iface[%d].key",iface);
	    ret = uci_get_option_value(cmd_buf, m_wifi_info->wifi_password);
	    if(-1 == ret)
	    {
	    	DMCLOG_D("uci get wifi_password error");
	        return ROUTER_ERRORS_UCI;
	    }
	}

#if defined(OPENWRT_MT7628)
	memset(cmd_buf,0,TEMP_BUFFER_SIZE);
	memset(temp_buf,0,TEMP_BUFFER_SIZE);
    sprintf(cmd_buf,"wireless.@wifi-iface[%d].disabled",iface);
	DMCLOG_D("cmd_buf = %s", cmd_buf);
    ret = uci_get_option_value(cmd_buf, temp_buf);
    if(-1 == ret)
    {
    	DMCLOG_D("uci get disabled error");
        return ROUTER_ERRORS_UCI;
    }
	else
	{
		if(!strcmp("0", temp_buf))
			m_wifi_info->disabled = 0;
		else
			m_wifi_info->disabled = 1;
	}
#endif

#if defined(OPENWRT_MT7628)
	read_fp = popen("hexdump -s 4 -n 6 -C /dev/mtd3 | head -n 1 | sed 's/\ \ /:/g' | cut -d: -f 2 | sed 's/\ /:/g' | tr \"[a-z]\" \"[A-Z]\"", "r");
	if(read_fp != NULL)
	{
		memset(tmp_mac,0,32);
		fgets(tmp_mac, 32-1, read_fp);
	}
	strncpy(m_wifi_info->mac,tmp_mac,17);
	pclose(read_fp);
#elif defined(OPENWRT_X1000)

	if( (read_fp=fopen("/etc/mac.txt", "rb")) != NULL)
	{
		fread(tmp_mac,1,17,read_fp);
		strncpy(m_wifi_info->mac,tmp_mac,17);
		fclose(read_fp);
	}
/*
	memset(cmd_buf,0,TEMP_BUFFER_SIZE);
	strcpy(cmd_buf,"wireless.radio0.macaddr");  				//mac
	ret = uci_get_option_value(cmd_buf,m_wifi_info->mac);
	if(-1 == ret)
    {
    	DMCLOG_D("uci get mac error");
        //return ROUTER_ERRORS_UCI;
    }
*/
#endif

#elif defined(SUPPORT_LINUX_PLATFORM)
	if (mozart_ini_getkey(NETCONFIGPATH, NETAPSECTION, "ssid", m_wifi_info->ssid))//ssid
	{
		DMCLOG_D("ini get ssid error");
        return ROUTER_ERRORS_INI;
	}

	memset(temp_buf,0, sizeof(temp_buf));
	if (mozart_ini_getkey(NETCONFIGPATH, NETAPSECTION, "encryption", temp_buf))//encrypt
	{
		DMCLOG_D("ini get encryption error");
        return ROUTER_ERRORS_INI;
	}
	else
	{
		if(!strcmp(temp_buf,"none")){
			strcpy(m_wifi_info->encrypt,"NONE");
		}
		else if( !strcmp(temp_buf,"psk2+ccmp") || !strcmp(temp_buf,"psk2+tkip") || !strcmp(temp_buf,"psk2+tkip+ccmp") ){
			strcpy(m_wifi_info->encrypt,"WPA2");
		}
		else if( !strcmp(temp_buf,"psk+ccmp") || !strcmp(temp_buf,"psk+tkip") || !strcmp(temp_buf,"psk+tkip+ccmp") ){
			strcpy(m_wifi_info->encrypt,"WPA");
		}
		else if( !strcmp(temp_buf,"mixed-psk+ccmp") || !strcmp(temp_buf,"mixed-psk+tkip") || !strcmp(temp_buf,"mixed-psk+tkip+ccmp") ){
			strcpy(m_wifi_info->encrypt,"WPA/WPA2");
		}
		else if( !strcmp(temp_buf,"wep") ){
			strcpy(m_wifi_info->encrypt,"WEP");
		}
		else{
			DMCLOG_D("ini get encryption error");
       		return ROUTER_ERRORS_INI;
		}
	}

	memset(ifname, 0, sizeof(ifname));
	strcpy(ifname,"wlan0");
	if(cgi_get_channel(ifname, m_wifi_info->channel)) 
	{
		DMCLOG_D("ini get channel error");
        return ROUTER_ERRORS_INI;
	}
	
	if (mozart_ini_getkey(NETCONFIGPATH, NETAPSECTION, "key", m_wifi_info->wifi_password))//password
	{
		DMCLOG_D("ini get key error");
        return ROUTER_ERRORS_INI;
	}
	if (mozart_ini_getkey("/factory/factoryconfig", "factory", "mac", m_wifi_info->mac))//mac
	{
		DMCLOG_D("ini get ssid error");
		return ROUTER_ERRORS_INI;
	}
	/*
    if(12 != strlen(mac))
    {
        memset(mac,0x0,sizeof(mac));
		strcpy(mac,"845dd7a89900");
	}*/
	m_wifi_info->disabled = 0;
#endif
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
	char encrypt_config_tmp[64]="\0";
    char encrypt_config[16]="\0";
    char tkip_aes_config[8]="\0";
    int iface = 0;

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
		#if defined(OPENWRT_MT7628)
			strcpy(encrypt_config,"psk-mixed");
		#elif defined(OPENWRT_X1000)
			strcpy(encrypt_config,"mixed-psk");
		#endif
    }else{
        return ROUTER_ERRORS_CMD_DATA;
    }

	if(m_wifi_info->tkip_aes != NULL&&*m_wifi_info->tkip_aes)
    {
        if(!strcmp(m_wifi_info->tkip_aes,"tkip"))
            strcpy(tkip_aes_config,"tkip");
        else if(!strcmp(m_wifi_info->tkip_aes,"aes"))
		#if defined(OPENWRT_MT7628)
            strcpy(tkip_aes_config,"aes");
		#elif defined(OPENWRT_X1000) || defined(LINUX_X1000) 
			strcpy(tkip_aes_config,"ccmp");
		#endif
        else if(!strcmp(m_wifi_info->tkip_aes,"tkip/aes"))
		#if defined(OPENWRT_MT7628)
            strcpy(tkip_aes_config,"tkip+aes");
		#elif defined(OPENWRT_X1000) || defined(LINUX_X1000) 
			strcpy(tkip_aes_config,"tkip+ccmp");
		#endif
		else
		#if defined(OPENWRT_MT7628)
            strcpy(tkip_aes_config,"aes");
		#elif defined(OPENWRT_X1000) || defined(LINUX_X1000) 
			strcpy(tkip_aes_config,"ccmp");
		#endif
    }else{
    	#if defined(OPENWRT_MT7628)
            strcpy(tkip_aes_config,"aes");
		#elif defined(OPENWRT_X1000) || defined(LINUX_X1000) 
			strcpy(tkip_aes_config,"ccmp");
		#endif
        //return -1;
    }

#if defined(SUPPORT_OPENWRT_PLATFORM)
    if(m_wifi_info->wifi_type == 1)
    {
        iface = 0;
    }else{
        iface = 2;
    }   

    if(m_wifi_info->ssid != NULL&&*m_wifi_info->ssid)//set ssid
    {
        memset(temp_buf,0,sizeof(temp_buf));
        sprintf(temp_buf,"wireless.@wifi-iface[%d].ssid=%s",iface,m_wifi_info->ssid);
        ret = uci_set_option_value(temp_buf);
		if(-1 == ret)
		{
			return ROUTER_ERRORS_UCI;
		}
	#if defined(OPENWRT_X1000)
		memset(temp_buf,0,sizeof(temp_buf));
		sprintf(temp_buf,"nor set ssid_name=%s",m_wifi_info->ssid);
		system(temp_buf);	
	#endif
    }else{
        return ROUTER_ERRORS_CMD_DATA;
    }
	
    if(!strcmp(encrypt_config,"none") || !strcmp(encrypt_config,"wep"))
    {
        memset(temp_buf,0,sizeof(temp_buf));
        sprintf(temp_buf,"wireless.@wifi-iface[%d].encryption=%s",iface,encrypt_config);
        ret = uci_set_option_value(temp_buf);
		if(-1 == ret)
		{
			return ROUTER_ERRORS_UCI;
		}
	#if defined(OPENWRT_X1000)
		memset(temp_buf,0,sizeof(temp_buf));
		sprintf(temp_buf,"nor set encryption=%s",encrypt_config);
		system(temp_buf);	
	#endif
    }
    else if(!strcmp(encrypt_config,"psk") || !strcmp(encrypt_config,"psk2") || !strcmp(encrypt_config,"psk-mixed") || !strcmp(encrypt_config,"mixed-psk"))
    {
    #if defined(OPENWRT_MT7628)
  	    memset(temp_buf,0,sizeof(temp_buf));
        sprintf(temp_buf,"wireless.@wifi-iface[%d].encryption=%s",iface,encrypt_config);
        ret = uci_set_option_value(temp_buf);
		if(-1 == ret)
		{
			return ROUTER_ERRORS_UCI;
		}
		
        if(strlen(tkip_aes_config)>0)
        {
			memset(temp_buf,0,sizeof(temp_buf));
            sprintf(temp_buf,"wireless.@wifi-iface[%d].wpa_crypto=%s",iface,tkip_aes_config);
            ret = uci_set_option_value(temp_buf);
			if(-1 == ret)
			{
				return ROUTER_ERRORS_UCI;
			}
        }
	#elif defined(OPENWRT_X1000)
		if(strlen(tkip_aes_config)>0)
        {
        	memset(encrypt_config_tmp,0,sizeof(encrypt_config_tmp));
			snprintf(encrypt_config_tmp,sizeof(encrypt_config_tmp),"%s+%s",encrypt_config,tkip_aes_config);
			memset(temp_buf,0,sizeof(temp_buf));
			sprintf(temp_buf,"wireless.@wifi-iface[%d].encryption=%s",iface,encrypt_config_tmp);
			ret = uci_set_option_value(temp_buf);
			if(-1 == ret)
			{
				return ROUTER_ERRORS_UCI;
			}
			memset(temp_buf,0,sizeof(temp_buf));
			sprintf(temp_buf,"nor set encryption=%s",encrypt_config_tmp);
			system(temp_buf);
        }
	#endif

		if(m_wifi_info->wifi_password != NULL&&*m_wifi_info->wifi_password)
	    {
	        memset(temp_buf,0,sizeof(temp_buf));
	        sprintf(temp_buf,"wireless.@wifi-iface[%d].key=%s",iface,m_wifi_info->wifi_password);
	        ret = uci_set_option_value(temp_buf);
			if(-1 == ret)
			{
				return ROUTER_ERRORS_UCI;
			}
		#if defined(OPENWRT_X1000)
			memset(temp_buf,0,sizeof(temp_buf));
			sprintf(temp_buf,"nor set ssid_password='%s'",m_wifi_info->wifi_password);
			system(temp_buf);			
		#endif
		}else{
	        return ROUTER_ERRORS_CMD_DATA;
	    }
    }
	else
	{
		return ROUTER_ERRORS_CMD_DATA;
	}
	/*
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
	*/
	//system("uci commit");
	//system("wifi >/dev/null 2>&1");
	//sleep(5);
	//system("/usr/mips/cgi-bin/script/control_newshair.sh start");
	#if defined(OPENWRT_MT7628)
    system("/usr/mips/cgi-bin/script/control_newshair.sh stop");
	system("`sleep 3;wifi;sleep 5;/usr/mips/cgi-bin/script/control_newshair.sh start` &");
	#elif defined(OPENWRT_X1000)
	system("set_ssid.sh & >/dev/null 2>&1");
	#endif

#elif defined(SUPPORT_LINUX_PLATFORM)
	if(m_wifi_info->ssid != NULL&&*m_wifi_info->ssid)//set ssid
    {
		if(mozart_ini_setkey(NETCONFIGPATH, NETAPSECTION, "ssid", m_wifi_info->ssid)){
			return ROUTER_ERRORS_INI;
		}
		memset(temp_buf, 0, sizeof(temp_buf));
		sprintf(temp_buf,"ssid_name");
		nor_set(temp_buf,m_wifi_info->ssid);
    }else{
        return ROUTER_ERRORS_CMD_DATA;
    }
	
    if(!strcmp(encrypt_config,"none") || !strcmp(encrypt_config,"wep"))
    {
		if(mozart_ini_setkey(NETCONFIGPATH, NETAPSECTION, "encryption", encrypt_config))
		{
			return ROUTER_ERRORS_INI;
		}
		memset(temp_buf, 0, sizeof(temp_buf));
		sprintf(temp_buf,"encryption");
		nor_set(temp_buf, encrypt_config);
    }
    else if(!strcmp(encrypt_config,"psk") || !strcmp(encrypt_config,"psk2") || !strcmp(encrypt_config,"mixed-psk"))
    {	
        if(strlen(tkip_aes_config)>0)
        {
        	memset(encrypt_config_tmp,0,sizeof(encrypt_config_tmp));
			snprintf(encrypt_config_tmp,sizeof(encrypt_config_tmp),"%s+%s",encrypt_config,tkip_aes_config);
        	if(mozart_ini_setkey(NETCONFIGPATH, NETAPSECTION, "encryption", encrypt_config_tmp))
			{
				return ROUTER_ERRORS_INI;
			}
			memset(temp_buf, 0, sizeof(temp_buf));
			sprintf(temp_buf,"encryption");
			nor_set(temp_buf, encrypt_config_tmp);
        }

		if(m_wifi_info->wifi_password != NULL&&*m_wifi_info->wifi_password)
	    {
			if(mozart_ini_setkey(NETCONFIGPATH, NETAPSECTION, "key", m_wifi_info->wifi_password))
			{
				return ROUTER_ERRORS_UCI;
			}
			memset(temp_buf, 0, sizeof(temp_buf));
			sprintf(temp_buf,"ssid_password");
			nor_set(temp_buf, m_wifi_info->wifi_password);
	    }else{
	        return ROUTER_ERRORS_CMD_DATA;
	    }
    }
	else
	{
		return ROUTER_ERRORS_CMD_DATA;
	}
		
#endif
	return ROUTER_OK;
}



int get_remote_ap(hd_remoteap_info *m_remote_info)
{
	int socket_id;
	struct ifreq ifr_remote_ap;
	int tmp = 0;
	int ret = 0;
	int inet_sock_remote_ap;
	//char channel[4] = "\0";
	char data[128];
	char cmd_buf[TEMP_BUFFER_SIZE];
    char temp_buf[TEMP_BUFFER_SIZE];
	char ifname[10]="\0";
#if defined(SUPPORT_OPENWRT_PLATFORM)
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
	#if defined(OPENWRT_MT7628)
        else if(!strcmp(temp_buf,"WPA2-PSK")||!strcmp(temp_buf,"WPA2")||!strcmp(temp_buf,"wpa2"))
	#elif defined(OPENWRT_X1000)
		else if(!strcmp(temp_buf,"psk2+ccmp") || !strcmp(temp_buf,"psk2+tkip") || !strcmp(temp_buf,"psk2+tkip+ccmp"))
	#endif
		{
            strcpy(m_remote_info->encrypt,"WPA2");
        }
	#if defined(OPENWRT_MT7628)
        else if(!strcmp(temp_buf,"WPA-PSK")||!strcmp(temp_buf,"WPA")||!strcmp(temp_buf,"WPA1")||!strcmp(temp_buf,"wpa")||!strcmp(temp_buf,"wpa1"))
	#elif defined(OPENWRT_X1000)
		else if(!strcmp(temp_buf,"psk+ccmp") || !strcmp(temp_buf,"psk+tkip") || !strcmp(temp_buf,"psk+tkip+ccmp") )
	#endif
		{
            strcpy(m_remote_info->encrypt,"WPA");
        }
	#if defined(OPENWRT_MT7628)
        else if(!strcmp(temp_buf,"WPA/WPA2-PSK")||!strcmp(temp_buf,"Mixed")||!strcmp(temp_buf,"MIXED")||!strcmp(temp_buf,"mixed"))
	#elif defined(OPENWRT_X1000)	
		else if( !strcmp(temp_buf,"mixed-psk+ccmp") || !strcmp(temp_buf,"mixed-psk+tkip") || !strcmp(temp_buf,"mixed-psk+tkip+ccmp") )
	#endif
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

	#if defined(OPENWRT_MT7628)
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
	#endif

	#if defined(OPENWRT_MT7628)
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
	#elif defined(OPENWRT_X1000)
	strcpy(ifname,"wlan0");
	memset(temp_buf,0,TEMP_BUFFER_SIZE);
	ret = cgi_get_channel(ifname, &temp_buf);  //channel
	if(ret == 0)
	{
		get_string_only(temp_buf);
        m_remote_info->channel = atoi(temp_buf);
	}
	#endif

	memset(cmd_buf,0,TEMP_BUFFER_SIZE);
	sprintf(cmd_buf,"wireless.@wifi-iface[1].key");
	ret = uci_get_option_value(cmd_buf, &m_remote_info->password);
    if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}

	#if defined(OPENWRT_MT7628)
	memset(cmd_buf,0,TEMP_BUFFER_SIZE);
	sprintf(cmd_buf,"wireless.@wifi-iface[1].mac");
	ret = uci_get_option_value(cmd_buf, &m_remote_info->mac);
	if(ret < 0)
	{
		DMCLOG_D("no mac");
		//return ROUTER_ERRORS_UCI;
	}
	#elif defined(OPENWRT_X1000)
	memset(cmd_buf,0,TEMP_BUFFER_SIZE);
	sprintf(cmd_buf,"wireless.@wifi-iface[1].bssid");
	ret = uci_get_option_value(cmd_buf, &m_remote_info->mac);
	if(ret < 0)
	{
		DMCLOG_D("no mac");
		//return ROUTER_ERRORS_UCI;
	}	
	#endif

	#if defined(OPENWRT_MT7628)
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
	#elif defined(OPENWRT_X1000)
	system("/usr/mips/cgi-bin/script/ClientStatus.sh");
	FILE *fp_client=NULL;
	if( (fp_client=fopen("/tmp/client_is_connected","r")) != NULL ){
		m_remote_info->is_connect = 1;
		fclose(fp_client);
		system("rm -f /tmp/client_is_connected");
	}
	else{
		m_remote_info->is_connect = 0;
	}
	#endif
#elif defined(SUPPORT_LINUX_PLATFORM)
#endif
    return ROUTER_OK;
}

#ifdef OPENWRT_X1000

int get_wifi_mode()
{
	char uci_option_str[64]="\0";
	char mode[16]="\0";
	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"wireless2.@wifi[0].mode"); 
	uci_get_option_value(uci_option_str,mode);

	if(strcmp(mode,"2g")==0)
		return M2G;
	else
		return M5G;
}

wifilist_t *get_wifi_list(int wifimode,int *number)
{
	char uci_option_str[64]="\0";
	char wifinum[16]="\0";
	int i_wifinum=0;
	int i=0;
	int j;
	// ctx=uci_alloc_context();

	if(wifimode==M2G)
	{
		strcpy(uci_option_str,"wifilist.number2g.num"); 
		uci_get_option_value(uci_option_str,wifinum);
		memset(uci_option_str,'\0',64);
	}
	else
	{
		strcpy(uci_option_str,"wifilist.number5g.num"); 
		uci_get_option_value(uci_option_str,wifinum);
		memset(uci_option_str,'\0',64);
	}
	i_wifinum=atoi(wifinum);
	*number=i_wifinum;

	if(i_wifinum==0)
		return NULL;

	wifilist_t *wifi_list=(wifilist_t *)malloc(sizeof(wifilist_t)*i_wifinum);
	memset(wifi_list,0,sizeof(wifilist_t)*i_wifinum);
	for(i=0;i<i_wifinum;i++)
	{
		if(wifimode==M2G)
		{
			sprintf(uci_option_str,"wifilist.@wifi2g[%d].ssid",i);
			uci_get_option_value(uci_option_str,(wifi_list+i)->ssid);
			memset(uci_option_str,'\0',64);
			sprintf(uci_option_str,"wifilist.@wifi2g[%d].encryption",i);
			uci_get_option_value(uci_option_str,(wifi_list+i)->encryption);
			memset(uci_option_str,'\0',64);
			if(strcmp((wifi_list+i)->encryption,"none")!=0)
			{
				sprintf(uci_option_str,"wifilist.@wifi2g[%d].key",i);
				uci_get_option_value(uci_option_str,(wifi_list+i)->key);
				memset(uci_option_str,'\0',64);
			}
		}
		else
		{
			sprintf(uci_option_str,"wifilist.@wifi5g[%d].ssid",i);
			uci_get_option_value(uci_option_str,(wifi_list+i)->ssid);
			memset(uci_option_str,'\0',64);
			sprintf(uci_option_str,"wifilist.@wifi5g[%d].encryption",i);
			uci_get_option_value(uci_option_str,(wifi_list+i)->encryption);
			memset(uci_option_str,'\0',64);
			if(strcmp((wifi_list+i)->encryption,"none")!=0)
			{
				sprintf(uci_option_str,"wifilist.@wifi5g[%d].key",i);
				uci_get_option_value(uci_option_str,(wifi_list+i)->key);
				memset(uci_option_str,'\0',64);
			}
		}
		//i++;
	}

	// uci_free_context(ctx);
	// for(i=0;i<i_wifinum;i++)
	// // 	d_printf("%s %s %s\n", (wifi_list+i)->ssid,(wifi_list+i)->encryption,(wifi_list+i)->key);
	// 	printstr((wifi_list+i)->ssid);
	return wifi_list;
}

#endif

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
#ifdef SUPPORT_OPENWRT_PLATFORM
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
	#if defined(OPENWRT_MT7628)
        else if(!strcmp(m_remote_info->encrypt,"WPA"))
            strcpy(encrypt_config,"wpa"); 
        else if(!strcmp(m_remote_info->encrypt,"WPA2"))
            strcpy(encrypt_config,"wpa2");
        else if(!strcmp(m_remote_info->encrypt,"WPA/WPA2")) 
            strcpy(encrypt_config,"mixed"); 
	#elif defined(OPENWRT_X1000)
		else if(!strcmp(m_remote_info->encrypt,"WPA-PSK"))
			strcpy(encrypt_config,"psk");
		else if(!strcmp(m_remote_info->encrypt,"WPA2-PSK"))
			strcpy(encrypt_config,"psk2");
		else if(!strcmp(m_remote_info->encrypt,"WPA/WPA2-PSK"))
			strcpy(encrypt_config,"mixed-psk");
	#endif
		else
			return ROUTER_ERRORS_CMD_DATA;
    }
#if defined(OPENWRT_MT7628)
	if( !strcmp(encrypt_config,"wpa") || !strcmp(encrypt_config,"wpa2") || !strcmp(encrypt_config,"mixed") )
#elif defined(OPENWRT_X1000)
	if( !strcmp(encrypt_config,"psk") || !strcmp(encrypt_config,"psk2") || !strcmp(encrypt_config,"mixed-psk") )
#endif
	{
		if(m_remote_info->tkip_aes != NULL&&*m_remote_info->tkip_aes)//tkip_aes
	    {
	    	#if defined(OPENWRT_MT7628)
	        if(!strcmp(m_remote_info->tkip_aes,"tkip"))
	            strcpy(tkip_aes_config,"TKIP");
	        else if(!strcmp(m_remote_info->tkip_aes,"aes"))
	            strcpy(tkip_aes_config,"AES");
	        else if(!strcmp(m_remote_info->tkip_aes,"tkip/aes"))
	            strcpy(tkip_aes_config,"TKIP+AES");
			else
				return ROUTER_ERRORS_CMD_DATA;
			#elif defined(OPENWRT_X1000)
			if(!strcmp(m_remote_info->tkip_aes,"tkip"))
				strcpy(tkip_aes_config,"tkip");
			else if(!strcmp(m_remote_info->tkip_aes,"aes"))
				strcpy(tkip_aes_config,"ccmp");
			else if(!strcmp(m_remote_info->tkip_aes,"tkip/aes"))
				strcpy(tkip_aes_config,"tkip+ccmp");
			else
				return ROUTER_ERRORS_CMD_DATA;
			#endif
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

	#if defined(OPENWRT_MT7628)
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
	#endif	

	#if defined(OPENWRT_X1000)
	char mac[32]="\0";
	char macaddr[32]="\0";
	if(m_remote_info->mac != NULL&&*m_remote_info->mac)//mac
    {
    	memcpy(macaddr,m_remote_info->mac,strlen(m_remote_info->mac));
    	/*
    		memcpy(mac,m_remote_info->mac,strlen(m_remote_info->mac));
    		macaddr[0]=mac[0];macaddr[1]=mac[1];macaddr[2]=':';
		macaddr[3]=mac[2];macaddr[4]=mac[3];macaddr[5]=':';
		macaddr[6]=mac[4];macaddr[7]=mac[5];macaddr[8]=':';
		macaddr[9]=mac[6];macaddr[10]=mac[7];macaddr[11]=':';
		macaddr[12]=mac[8];macaddr[13]=mac[9];macaddr[14]=':';
		macaddr[15]=mac[10];macaddr[16]=mac[11];
		*/
		memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
       	sprintf(cmd_buf, "wireless.@wifi-iface[1].bssid=%s", macaddr);
        ret = uci_set_option_value(cmd_buf);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
	
    }
    // else{
        
    //     system("uci delete wireless.@wifi-iface[1].mac");
        
    // }
	#endif	

	#if defined(OPENWRT_MT7628)
	if(!strcmp(encrypt_config,"none") || !strcmp(encrypt_config,"wep") || !strcmp(encrypt_config,"wpa") || \
		!strcmp(encrypt_config,"wpa2") || !strcmp(encrypt_config,"mixed"))
	{
		memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
        sprintf(cmd_buf, "wireless.@wifi-iface[1].encryption=%s", encrypt_config);
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
	    	if(!strlen(m_remote_info->password) || strlen(m_remote_info->password)>32){
				return ROUTER_ERRORS_CMD_DATA;
			}
			else{
		        memset(temp_buf,0,TEMP_BUFFER_SIZE);
		        sprintf(temp_buf,"wireless.@wifi-iface[1].key=%s",m_remote_info->password);
		        ret = uci_set_option_value(temp_buf);
				if(ret < 0)
				{
					return ROUTER_ERRORS_UCI;
				}
			}
	    }else{
	        return ROUTER_ERRORS_CMD_DATA;
	    }
	}
	#elif defined(OPENWRT_X1000)
	memset(cmd_buf,0,sizeof(cmd_buf));
	if(!strcmp(encrypt_config,"none") || !strcmp(encrypt_config,"wep"))
	{
		sprintf(cmd_buf,"uci set wireless.@wifi-iface[1].encryption=%s",encrypt_config);
		system(cmd_buf);
		#if 0
		sprintf(cmd_buf,"wireless.@wifi-iface[1].encryption=%s",encrypt_config);
		ret = uci_set_option_value(temp_buf);
		if(ret < 0){
			return ROUTER_ERRORS_UCI;
		}
		#endif
	}
	else
	{
		sprintf(cmd_buf,"uci set wireless.@wifi-iface[1].encryption=%s+%s",encrypt_config,tkip_aes_config);
		system(cmd_buf);
		#if 0
		sprintf(cmd_buf,"wireless.@wifi-iface[1].encryption=%s+%s",encrypt_config,tkip_aes_config);
		ret = uci_set_option_value(temp_buf);
		if(ret < 0){
			return ROUTER_ERRORS_UCI;
		}
		#endif
		if(m_remote_info->password!=NULL&&*m_remote_info->password)
	    {
	    	if(!strlen(m_remote_info->password) || strlen(m_remote_info->password)>32){
				return ROUTER_ERRORS_CMD_DATA;
			}
			else{
		        memset(temp_buf,0,TEMP_BUFFER_SIZE);
		        sprintf(temp_buf,"wireless.@wifi-iface[1].key=%s",m_remote_info->password);
		        ret = uci_set_option_value(temp_buf);
				if(ret < 0)
				{
					return ROUTER_ERRORS_UCI;
				}
			}
	    }else{
	        return ROUTER_ERRORS_CMD_DATA;
	    }
	}
	#endif	

	#if defined(OPENWRT_MT7628)
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
	#endif

	
   	system("uci commit");
   	#if defined(OPENWRT_MT7628)
	system("killall refresh_aplist");
	system("/usr/bin/refresh_aplist &");
	system("`sleep 3;wifi;ifup wan` &");
	#elif defined(OPENWRT_X1000)
	system("control_dns.sh >/dev/null 2>&1 &");
	system("save_wifi &");
	#endif
#elif defined(SUPPORT_LINUX_PLATFORM)
#endif
    return 0;
}


int set_add_network(hd_remoteap_info *m_remote_info)
{	
	ap_info_t ap_info_add_network;
    int ret;
	char mac[32]="\0";
	char macaddr[32]="\0";
    char encrypt_config[16]="\0";
    char tkip_aes_config[16]="\0";
    char temp_buf[TEMP_BUFFER_SIZE];
	char cmd_buf[TEMP_BUFFER_SIZE];
    char WiredMode[8]="\0";

	char ifname[10];
	memset(ifname, 0, sizeof(ifname));
	memset(&ap_info_add_network, 0, sizeof(ap_info_t));
	strcpy(ifname, "wlan0");	
	ret = cgi_get_scan_nl80211_add_network(ifname, m_remote_info->ssid, &ap_info_add_network);
	if(0 != ret){
		return ROUTER_ERRORS_ADD_NETWORK;
	}
	else{
		strcpy(m_remote_info->encrypt, ap_info_add_network.encrypt);
		strcpy(m_remote_info->tkip_aes, ap_info_add_network.tkip_aes);
		strcpy(m_remote_info->mac, ap_info_add_network.mac);
		m_remote_info->channel = ap_info_add_network.channel;
	}

	memset(cmd_buf,0,TEMP_BUFFER_SIZE);
	sprintf(cmd_buf, "wireless.@wifi-iface[1].disabled=0");
	ret = uci_set_option_value(cmd_buf);
	if(ret < 0){
		return ROUTER_ERRORS_UCI;
	}

	memset(cmd_buf,0,TEMP_BUFFER_SIZE);
	sprintf(cmd_buf, "network.wan.workmode=1");
	ret = uci_set_option_value(cmd_buf);
	if(ret < 0){
		return ROUTER_ERRORS_UCI;
	}
	memset(cmd_buf,0,TEMP_BUFFER_SIZE);
	sprintf(cmd_buf,"network.wan.proto");
	memset(temp_buf,0,TEMP_BUFFER_SIZE);
	ret = uci_get_option_value(cmd_buf, &temp_buf);
    if(ret >= 0){
        get_string_only(temp_buf);
        memcpy(WiredMode, temp_buf, 8);
    }

    if(m_remote_info->encrypt != NULL&&*m_remote_info->encrypt)//encrypt
    {
        if(!strcmp(m_remote_info->encrypt,"NONE"))
            strcpy(encrypt_config,"none");
        else if(!strcmp(m_remote_info->encrypt,"WEP"))
            strcpy(encrypt_config,"wep");
		else if(!strcmp(m_remote_info->encrypt,"WPA-PSK"))
			strcpy(encrypt_config,"psk");
		else if(!strcmp(m_remote_info->encrypt,"WPA2-PSK"))
			strcpy(encrypt_config,"psk2");
		else if(!strcmp(m_remote_info->encrypt,"WPA/WPA2-PSK"))
			strcpy(encrypt_config,"mixed-psk");
		else
			return ROUTER_ERRORS_CMD_DATA;
    }

	if( !strcmp(encrypt_config,"psk") || !strcmp(encrypt_config,"psk2") || !strcmp(encrypt_config,"mixed-psk") )
	{
		if(m_remote_info->tkip_aes != NULL&&*m_remote_info->tkip_aes)//tkip_aes
	    {
	    	#if defined(OPENWRT_MT7628)
	        if(!strcmp(m_remote_info->tkip_aes,"tkip"))
	            strcpy(tkip_aes_config,"TKIP");
	        else if(!strcmp(m_remote_info->tkip_aes,"aes"))
	            strcpy(tkip_aes_config,"AES");
	        else if(!strcmp(m_remote_info->tkip_aes,"tkip/aes"))
	            strcpy(tkip_aes_config,"TKIP+AES");
			else
				return ROUTER_ERRORS_CMD_DATA;
			#elif defined(OPENWRT_X1000)
			if(!strcmp(m_remote_info->tkip_aes,"tkip"))
				strcpy(tkip_aes_config,"tkip");
			else if(!strcmp(m_remote_info->tkip_aes,"aes"))
				strcpy(tkip_aes_config,"ccmp");
			else if(!strcmp(m_remote_info->tkip_aes,"tkip/aes"))
				strcpy(tkip_aes_config,"tkip+ccmp");
			else
				return ROUTER_ERRORS_CMD_DATA;
			#endif
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
    	memcpy(macaddr,m_remote_info->mac,strlen(m_remote_info->mac));
		memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
       	sprintf(cmd_buf, "wireless.@wifi-iface[1].bssid=%s", macaddr);
        ret = uci_set_option_value(cmd_buf);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
	
    }
	
	memset(cmd_buf,0,sizeof(cmd_buf));
	if(!strcmp(encrypt_config,"none") || !strcmp(encrypt_config,"wep"))
	{
		sprintf(cmd_buf,"uci set wireless.@wifi-iface[1].encryption=%s",encrypt_config);
		system(cmd_buf);
	}
	else
	{
		sprintf(cmd_buf,"uci set wireless.@wifi-iface[1].encryption=%s+%s",encrypt_config,tkip_aes_config);
		system(cmd_buf);
		if(m_remote_info->password!=NULL&&*m_remote_info->password)
	    {
	    	if(!strlen(m_remote_info->password) || strlen(m_remote_info->password)>32){
				return ROUTER_ERRORS_CMD_DATA;
			}
			else{
		        memset(temp_buf,0,TEMP_BUFFER_SIZE);
		        sprintf(temp_buf,"wireless.@wifi-iface[1].key=%s",m_remote_info->password);
		        ret = uci_set_option_value(temp_buf);
				if(ret < 0)
				{
					return ROUTER_ERRORS_UCI;
				}
			}
	    }else{
	        return ROUTER_ERRORS_CMD_DATA;
	    }
	}
	
   	system("uci commit");
	system("control_dns.sh >/dev/null 2>&1 &");
	system("save_wifi &");

    return 0;
}

int _get_wlan_con_mode()
{
	int work_mode = 0;
	int ret = -1;
	char value[4]="\0";
	char cmd_buf[TEMP_BUFFER_SIZE];
	char temp_buf[TEMP_BUFFER_SIZE];

#ifdef SUPPORT_OPENWRT_PLATFORM
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
#endif
#ifdef SUPPORT_LINUX_PLATFORM
#endif
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
#ifdef SUPPORT_OPENWRT_PLATFORM
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
#endif
#ifdef SUPPORT_LINUX_PLATFORM
#endif
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
#ifdef SUPPORT_OPENWRT_PLATFORM
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
	
	//system("uci commit");
	system("/usr/mips/cgi-bin/script/control_newshair.sh stop");

	if(con_type == DHCP_MODE)
	{
		system("uci delete network.wan.dns");
		system("uci delete network.wan.gateway");
		system("uci delete network.wan.ipaddr");
		system("uci delete network.wan.netmask");
		system("uci delete network.wan.username");
		system("uci delete network.wan.password");
		//system("uci commit");
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
			
			//system("uci commit");
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
		//system("uci commit");
		
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
		//system("uci commit");

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
	//system("uci commit");
	system("sync");
//	shell_uci_set_value("killall wanlan");
//	sleep(1);
	system("ifup wan");
#endif
#ifdef SUPPORT_LINUX_PLATFORM
#endif
	return ROUTER_OK;
}


int _get_client_status()
{
	int ret = -1;
	char cmd_buf[TEMP_BUFFER_SIZE];
    char temp_buf[TEMP_BUFFER_SIZE];
#ifdef SUPPORT_OPENWRT_PLATFORM
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
#endif
#ifdef SUPPORT_LINUX_PLATFORM
#endif
	return ret;
}

int _set_client_status(int status)
{
	int ret = -1;
	char cmd_buf[TEMP_BUFFER_SIZE];
    char temp_buf[TEMP_BUFFER_SIZE];
	int old_status = 0;
#if defined(SUPPORT_OPENWRT_PLATFORM)
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
		if(status == CLIENT_ON){
			memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
			sprintf(cmd_buf, "wireless.@wifi-iface[1].disabled=%d",status);
			ret = uci_set_option_value(cmd_buf);
			if(ret < 0){
				return ROUTER_ERRORS_UCI;
			}		
			memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
			sprintf(cmd_buf,"nor set client_disabled=%d",status);
			system(cmd_buf);
			
		}else if(status == CLIENT_OFF){
			memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
			sprintf(cmd_buf, "wireless.@wifi-iface[1].disabled=%d",status);
			ret = uci_set_option_value(cmd_buf);
			if(ret < 0){
				return ROUTER_ERRORS_UCI;
			}			
			memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
			sprintf(cmd_buf,"nor set client_disabled=%d",status);
			system(cmd_buf);
			

		}
		else{
			return ROUTER_ERRORS_CMD_DATA;
		}	
	}

	#if defined(OPENWRT_MT7628)
	system("`sleep 3;wifi;ifup wan` &");
	#elif defined(OPENWRT_X1000)
	DMCLOG_M("control dns!!!");
	system("control_dns.sh >/dev/null 2>&1 &");
	#endif
#elif defined(SUPPORT_LINUX_PLATFORM)
#endif
	return ROUTER_OK;
}

int _set_forget_wifi_info(forget_wifi_info_t *p_forget_wifi_info)
{
	int ret = -1;
	char uci_option_str[64]="\0";
	int ap_sum = 0;
	int tmp_sum = 0;
	char sum[8] = "\0";
	char disabled[8] = "\0";
	char wireless_mac[32] = "\0";
	char wifilist_mac[32] = "\0";
	char wireless_ssid[64] = "\0";
	char wifilist_ssid[64] = "\0";
	int i;
#if defined(SUPPORT_OPENWRT_PLATFORM)
	#if defined(OPENWRT_MT7628)
	memset(uci_option_str, 0, sizeof(uci_option_str));
	memset(sum, 0, sizeof(sum));
	sprintf(uci_option_str,"remoteAPlist.ap.sum");
	ret = uci_get_option_value(uci_option_str,sum);
	if(ret < 0){
		return ROUTER_ERRORS_UCI;
	}
	ap_sum=atoi(sum);
	tmp_sum = ap_sum;
	for(i = 0;i < ap_sum; i++){
		memset(uci_option_str,0,sizeof(uci_option_str));
		memset(disabled,0,sizeof(disabled));
		sprintf(uci_option_str,"remoteAPlist.@ap[%d].disabled",i);
		ret = uci_get_option_value(uci_option_str,disabled);	
		if(ret < 0){
			return ROUTER_ERRORS_UCI;
		}
		if(strcmp(disabled,"1") == 0){
			tmp_sum++;	
		}
	}
	for(i = 0;i < tmp_sum; i++){
		memset(uci_option_str,0,sizeof(uci_option_str));
		memset(wifilist_mac,0,sizeof(wifilist_mac));
		sprintf(uci_option_str,"remoteAPlist.@ap[%d].mac",i);
		ret = uci_get_option_value(uci_option_str,mac);	
		if(ret < 0){
			return ROUTER_ERRORS_UCI;
		}
		if(strcasecmp(wifilist_mac,p_forget_wifi_info->mac) == 0)
		{
			memset(uci_option_str,0,sizeof(uci_option_str));
			memset(disabled,0,sizeof(disabled));
			sprintf(uci_option_str,"remoteAPlist.@ap[%d].disabled",i);
			ret = uci_get_option_value(uci_option_str,disabled);	
			if(ret < 0){
				return ROUTER_ERRORS_UCI;
			}
			if(strcmp(disabled,"1") == 0){
				DMCLOG_M("mac of %s has already forgot", p_forget_wifi_info->mac);
			}
			else{
				memset(uci_option_str, 0, sizeof(uci_option_str));
				sprintf(uci_option_str,"remoteAPlist.@ap[%d].disabled=1",i);
				ret = uci_set_option_value(uci_option_str);
				if(ret < 0){
					return ROUTER_ERRORS_UCI;
				}
				memset(uci_option_str, 0, sizeof(uci_option_str));
				sprintf(uci_option_str,"remoteAPlist.ap.sum=%d",ap_sum-1);
				ret = uci_set_option_value(uci_option_str);
				if(ret < 0){
					return ROUTER_ERRORS_UCI;
				}
				//system("uci commit");
				memset(uci_option_str, 0, sizeof(uci_option_str));
				memset(wireless_mac, 0, sizeof(wireless_mac));
				sprintf(uci_option_str,"wireless.@wifi-iface[1].mac");
				ret = uci_get_option_value(uci_option_str,mac);	
				if(ret < 0){
					return ROUTER_ERRORS_UCI;
				}
				if(strcasecmp(wireless_mac,p_forget_wifi_info->mac) == 0)
				{
					system("iwpriv apcli0 set ApCliEnable=0");
					system("ifconfig apcli0 down");
				}
			}
		}
	}
	#elif defined(OPENWRT_X1000)

	int wifimode=get_wifi_mode();
	memset(uci_option_str, 0, sizeof(uci_option_str));
	memset(sum, 0, sizeof(sum));
	if(wifimode==M2G)
	{
		strcpy(uci_option_str,"wifilist.number2g.num"); 
		ret = uci_get_option_value(uci_option_str,sum);
		if(ret < 0){
			return ROUTER_ERRORS_UCI;
		}
	}
	else
	{
		strcpy(uci_option_str,"wifilist.number5g.num"); 
		ret = uci_get_option_value(uci_option_str,sum);
		if(ret < 0){
			return ROUTER_ERRORS_UCI;
		}
	}

	ap_sum=atoi(sum);
	//tmp_sum = ap_sum;
	for(i = 0; i < ap_sum; i++){
		if(wifimode==M2G){
			memset(wifilist_ssid, 0, sizeof(wifilist_ssid));
			memset(uci_option_str, 0, sizeof(uci_option_str));
			sprintf(uci_option_str,"wifilist.@wifi2g[%d].ssid",i);
			ret = uci_get_option_value(uci_option_str, wifilist_ssid);
			if(ret < 0){
				return ROUTER_ERRORS_UCI;
			}

			if(strcasecmp(wifilist_ssid, p_forget_wifi_info->ssid) == 0){
				memset(uci_option_str, 0, sizeof(uci_option_str));
				sprintf(uci_option_str, "uci del wifilist.@wifi2g[%d]", i);
				system(uci_option_str);
				memset(uci_option_str, 0, sizeof(uci_option_str));
				sprintf(uci_option_str,"wifilist.number2g.num=%d",ap_sum-1);
				ret = uci_set_option_value(uci_option_str);
				if(ret < 0){
					return ROUTER_ERRORS_UCI;
				}

				memset(uci_option_str, 0, sizeof(uci_option_str));
				memset(wireless_ssid, 0, sizeof(wireless_ssid));
				sprintf(uci_option_str,"wireless.@wifi-iface[1].ssid");
				ret = uci_get_option_value(uci_option_str,wireless_ssid);	
				if(ret < 0){
					return ROUTER_ERRORS_UCI;
				}
				if(strcasecmp(wireless_ssid,p_forget_wifi_info->ssid) == 0)
				{
					memset(uci_option_str, 0, sizeof(uci_option_str));
					sprintf(uci_option_str,"wireless.@wifi-iface[1].key=1");
					ret = uci_set_option_value(uci_option_str);
					if(ret < 0){
						return ROUTER_ERRORS_UCI;
					}
					system("killall wpa_supplicant");
				}
				break;//out of for cycle
			}
		}
		else{
			memset(wifilist_ssid, 0, sizeof(wifilist_ssid));
			memset(uci_option_str, 0, sizeof(uci_option_str));
			sprintf(uci_option_str,"wifilist.@wifi5g[%d].ssid",i);
			ret = uci_get_option_value(uci_option_str, wifilist_ssid);
			if(ret < 0){
				return ROUTER_ERRORS_UCI;
			}
			
			if(strcasecmp(wifilist_ssid,p_forget_wifi_info->ssid) == 0){
				memset(uci_option_str, 0, sizeof(uci_option_str));
				sprintf(uci_option_str, "uci del wifilist.@wifi5g[%d]", i);
				system(uci_option_str);
				memset(uci_option_str, 0, sizeof(uci_option_str));
				sprintf(uci_option_str,"wifilist.number5g.num=%d",ap_sum-1);
				ret = uci_set_option_value(uci_option_str);
				if(ret < 0){
					return ROUTER_ERRORS_UCI;
				}

				memset(uci_option_str, 0, sizeof(uci_option_str));
				memset(wireless_ssid, 0, sizeof(wireless_ssid));
				sprintf(uci_option_str,"wireless.@wifi-iface[1].ssid");
				ret = uci_get_option_value(uci_option_str,wireless_ssid);	
				if(ret < 0){
					return ROUTER_ERRORS_UCI;
				}
				if(strcasecmp(wireless_ssid,p_forget_wifi_info->ssid) == 0)
				{
					memset(uci_option_str, 0, sizeof(uci_option_str));
					sprintf(uci_option_str,"wireless.@wifi-iface[1].key=1");
					ret = uci_set_option_value(uci_option_str);
					if(ret < 0){
						return ROUTER_ERRORS_UCI;
					}
					system("killall wpa_supplicant");
				}
				break;//out of for cycle
			}
		}
	}	
	#endif
#elif defined(SUPPORT_LINUX_PLATFORM)
#endif
	return ROUTER_OK;
}


int _get_wifi_type(int *p_wifi_type)
{
	int wifi_mode;
	wifi_mode = get_wifi_mode();
	if(wifi_mode == M2G)
		*p_wifi_type = WIFI_TYPE_2G;
	else
		*p_wifi_type = WIFI_TYPE_5G;
	return 0;
}

int _set_wifi_type(int wifi_type)
{
	DMCLOG_D("access _set_wifi_type");
	int ret = 0;
	int wifi_mode;
	wifi_mode = get_wifi_mode();
	if(wifi_mode == M2G){
		if(wifi_type == WIFI_TYPE_5G){
			system("`sleep 2;/sbin/2g_5g_switch.sh `&");
		}
		else{
			ret = -1;
		}
	}
	else if(wifi_mode == M5G){
		if(wifi_type == WIFI_TYPE_2G){
			system("`sleep 2;/sbin/2g_5g_switch.sh `&");
		}
		else{
			ret = -1; 
		}
	}
	else
		ret = -1;
	DMCLOG_D("out _set_wifi_type");
	return ret;
}

