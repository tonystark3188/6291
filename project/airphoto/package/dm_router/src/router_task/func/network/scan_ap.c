#include <iwlib.h> 
#include <sys/wait.h>
#include <termios.h>
#include <ctype.h>
#include "net_config.h"
#include "hd_wifi.h"



static int change_fre_device_name(char *fre,char *device_name)
{
	if(fre==NULL||device_name==NULL)
		return -1;
	if(strcmp(fre,WIFI_2_4G)==0)
		strcpy(device_name,WIFI_DEVICE_24G);
	else if(strcmp(fre,WIFI_5G)==0)
		strcpy(device_name,WIFI_DEVICE_5G);
	else
		return -1;
	return 0;
}

static int judge_encrypt_tkip(char *security,char *encrypt,char *tkip_aes)
{
	if(!strncmp(security, encrypt_none, strlen(encrypt_none))){
		strcpy(encrypt,"NONE");
		strcpy(tkip_aes,"");
	}else if(!strncmp(security,encrypt_wep, strlen(encrypt_wep))){
		strcpy(encrypt,"WEP");
		strcpy(tkip_aes,"");
	}else if(!strncmp(security,encrypt_wpapsk_tkip_aes, strlen(encrypt_wpapsk_tkip_aes))){
		strcpy(encrypt,"WPA");
		strcpy(tkip_aes,"tkip/aes");
	}else if(!strncmp(security,encrypt_wpa2psk_tkip_aes, strlen(encrypt_wpa2psk_tkip_aes))){
		strcpy(encrypt,"WPA2");
		strcpy(tkip_aes,"tkip/aes");
	}else if(!strncmp(security,encrypt_wpa2_wpa1_psk_tkip_aes, strlen(encrypt_wpa2_wpa1_psk_tkip_aes))){
		strcpy(encrypt,"WPA/WPA2");
		strcpy(tkip_aes,"tkip/aes");
	}else if(!strncmp(security,encrypt_wpapsk_tkip, strlen(encrypt_wpapsk_tkip))){
		strcpy(encrypt,"WPA");
		strcpy(tkip_aes,"tkip");
	}else if(!strncmp(security,encrypt_wpapsk_aes, strlen(encrypt_wpapsk_aes))){
		strcpy(encrypt,"WPA");
		strcpy(tkip_aes,"aes");
	}else if(!strncmp(security,encrypt_wpa2psk_tkip, strlen(encrypt_wpa2psk_tkip))){
		strcpy(encrypt,"WPA2");
		strcpy(tkip_aes,"tkip");
	}else if(!strncmp(security,encrypt_wpa2psk_aes, strlen(encrypt_wpa2psk_aes))){
		strcpy(encrypt,"WPA2");
		strcpy(tkip_aes,"aes");
	}else if(!strncmp(security,encrypt_wpa2_wpa1_psk_tkip, strlen(encrypt_wpa2_wpa1_psk_tkip))){
		strcpy(encrypt,"WPA/WPA2");
		strcpy(tkip_aes,"tkip");
	}else if(!strncmp(security,encrypt_wpa2_wpa1_psk_aes, strlen(encrypt_wpa2_wpa1_psk_aes))){
		strcpy(encrypt,"WPA/WPA2");
		strcpy(tkip_aes,"aes");
	}
	return 0;
}
static int get_member(char *buffer,int rank,char *member)
{
	char *start;
	char *head;
	int i = 0;
	start = buffer;
	head = buffer;
	while(i <= rank){
		DMCLOG_D("head = %s",head);
		start = strchr(head,'\t');
		if(start == NULL)
		{
			DMCLOG_D("NULL");
		}
		DMCLOG_D("start = %s",start);
		i++;
		head = start + 1;
	}
	memcpy(member,head,start-head);
	DMCLOG_D("member = %s",member);
	return 0;
}
void del_space(unsigned char *tmp_buf,int leng)
{
	int i;
	for(i=leng;i>0;i--)
	{
		if(tmp_buf[i-1]==32)
		{
			tmp_buf[i-1]=0;
		}else
		{
			break;
		}
	}
}

int dm_wlan_scan(ap_list_info_t *ap_list_info)
{
	char cmd[128];
	char device_name[16];
	char buffer[1024];
	char security[32];
	int chars_read = 0;
	FILE *read_fp;
	int i = 0;
	char member[64];
	memset(cmd,0,128);
	memset(device_name,0,16);
	int temp_ret = change_fre_device_name(ap_list_info->fre,device_name);
	if(temp_ret < 0)
		return -2;
	sprintf(cmd,"iwpriv %s set SiteSurvey=1",device_name);
	system(cmd);
	sleep(1);
	read_fp = popen("iwpriv ra0 get_site_survey", "r");
	if(read_fp != NULL)
	{
		do{
			memset(buffer,0,1024);
			chars_read = fgets(buffer, 1024-1, read_fp);
			//DMCLOG_D("buffer = %s",buffer);
			if(chars_read > 0)
			{
				if((58>buffer[0])&&(buffer[0]>=48))
				{
					memset(security,0,32);
					memset(member,0,64);
					memcpy(member,buffer,4);
					//DMCLOG_D("member1 = %s",member);
					ap_list_info->ap_info[ap_list_info->count].channel = atoi(member);
					
					memcpy(ap_list_info->ap_info[ap_list_info->count].ssid,buffer+4,33);

					memset(member,0,64);
					memcpy(member,buffer+37,4);
					//DMCLOG_D("member2 = %s",member);
					ap_list_info->ap_info[ap_list_info->count].ssid_len=atoi(member);
					if(ap_list_info->ap_info[ap_list_info->count].ssid_len == 0)
					{
						continue;
					}
					for(i = ap_list_info->ap_info[ap_list_info->count].ssid_len;i < 33;i++)
					{
						ap_list_info->ap_info[ap_list_info->count].ssid[i]=0;
					}
					memcpy(ap_list_info->ap_info[ap_list_info->count].mac,buffer+41,20);

					memset(member,0,64);
					memcpy(member,buffer+61,23);
					//DMCLOG_D("member3 = %s",member);
					judge_encrypt_tkip(member,ap_list_info->ap_info[ap_list_info->count].encrypt,ap_list_info->ap_info[ap_list_info->count].tkip_aes);
					memset(member,0,64);
					memcpy(member,buffer+84,9);
					//DMCLOG_D("member4 = %s",member);
					ap_list_info->ap_info[ap_list_info->count].wifi_signal = atoi(member);
					ap_list_info->count++;
					
				}
			}
		}while(chars_read > 0);
	}
	else
	{
		return ROUTER_ERRORS_SHELL_HANDLE;
	}
	pclose(read_fp);
	return 0;
}











