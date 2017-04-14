#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <linux/wireless.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#include "uci_for_cgi.h"


#if WIRELESS_EXT <= 11
#ifndef SIOCDEVPRIVATE
#define SIOCDEVPRIVATE      0x8BE0
#endif
#define SIOCIWFIRSTPRIV      SIOCDEVPRIVATE
#endif

#define RTPRIV_IOCTL_GET_80211_DATA	(SIOCIWFIRSTPRIV + 0x1D)
#define RTPRIV_IOCTL_SHOW_CONNSTATUS	(SIOCIWFIRSTPRIV + 0x1B)

#define RX_80211_PKT_SIZE   (300)
#define RX_BUF_SIZE         (3000)

#define encrypt_none "NONE"
#define encrypt_wep "WEP"
#define encrypt_wpapsk_tkip "WPAPSK/TKIP"
#define encrypt_wpapsk_aes "WPAPSK/AES"
#define encrypt_wpapsk_tkip_aes "WPAPSK/TKIPAES"
#define encrypt_wpa2psk_aes "WPA2PSK/AES"
#define encrypt_wpa2psk_tkip "WPA2PSK/TKIP"
#define encrypt_wpa2psk_tkip_aes "WPA2PSK/TKIPAES"
#define encrypt_wpa2_wpa1_psk_aes "WPA1PSKWPA2PSK/AES"
#define encrypt_wpa2_wpa1_psk_tkip "WPA1PSKWPA2PSK/TKIP"
#define encrypt_wpa2_wpa1_psk_tkip_aes "WPA1PSKWPA2PSK/TKIPAES"


#define encrypt_wpa_tkip "WPA/TKIP"
#define encrypt_wpa_aes "WPA/AES"
#define encrypt_wpa_tkip_aes "WPA/TKIPAES"
#define encrypt_wpa2_aes "WPA2/AES"
#define encrypt_wpa2_tkip "WPA2/TKIP"
#define encrypt_wpa2_tkip_aes "WPA2/TKIPAES"
#define encrypt_wpa2_wpa1_aes "WPA1PSKWPA2/AES"
#define encrypt_wpa2_wpa1_tkip "WPA1PSKWPA2/TKIP"
#define encrypt_wpa2_wpa1_tkip_aes "WPA1PSKWPA2/TKIPAES"


void del_space(char *tmp_buf,int leng)
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

int cgi_scan(const char *pssid,char *pmac,char *pchanel,char *pencrypt,char *ptkip_aes)
{
	int ret = -1;
	char buffer[1024];
	char tmp_buffer[512];
	FILE *read_fp; 
	char *chars_read;
	char *encode_ssid;
	char *encode_password;
	int ssid_leng_int;
	int i=0;
	char ch[4];
	char ssid[33];
	char ssid_leng[4];
	char bssid[20];
	char security[23];
	char siganl[9];
	char w_mode[8];
	char extch[7];
	char encrypt[20];
	char tkip_aes[20];
	char password[64];
	system("iwpriv ra0 set SiteSurvey=1");
//	memset(buffer, 0, 1024 ); 
	sleep(1);
	read_fp = popen("iwpriv ra0 get_site_survey", "r");
	if(read_fp!=NULL)
	{
		do
		{
	//		i++;
			memset(buffer, 0, 1024 );
			memset(ch,0,4);
			memset(ssid,0,33);
			memset(ssid_leng,0,4);
			memset(bssid,0,20);
			memset(security,0,23);
			memset(siganl,0,9);
			memset(w_mode,0,8);
			memset(extch,0,7);
			memset(encrypt,0,20);
			memset(tkip_aes,0,20);
			memset(tmp_buffer,0,512);
			chars_read = fgets(buffer, 1024-1, read_fp);
	//		chars_read = fread(buffer, sizeof(char), 1024-1, read_fp); 
			if (chars_read != NULL)   	//connect !
			{ 
				if((58>buffer[0])&&(buffer[0]>=48))
				{
					memcpy(ch,buffer,4);
					del_space(ch,4);
					memcpy(ssid,buffer+4,33);
//					del_space(ssid,33);
					memcpy(ssid_leng,buffer+37,4);
					del_space(ssid_leng,4);
					ssid_leng_int=atoi(ssid_leng);
					if(ssid_leng_int == 0)
					{
						continue;
					}
					for(i=ssid_leng_int;i<33;i++)
					{
						ssid[i]=0;
					}
					memcpy(bssid,buffer+41,20);
					del_space(bssid,20);
					memcpy(security,buffer+61,23);
					del_space(security,23);
					if(!strcmp(security,encrypt_none))
					{
						strcpy(encrypt,"NONE");
						strcpy(tkip_aes,"");
					}else if(!strcmp(security,encrypt_wep))
					{
						strcpy(encrypt,"WEP");
						strcpy(tkip_aes,"");
					}else
					{
						if(!strcmp(security,encrypt_wpapsk_tkip_aes))
						{
							strcpy(encrypt,"WPA-PSK");
							strcpy(tkip_aes,"tkip/aes");
						}else
						{
							if(!strcmp(security,encrypt_wpa2psk_tkip_aes))
							{
								strcpy(encrypt,"WPA2-PSK");
								strcpy(tkip_aes,"tkip/aes");
							}else
							{
								if(!strcmp(security,encrypt_wpa2_wpa1_psk_tkip_aes))
								{
									strcpy(encrypt,"WPA/WPA2-PSK");
									strcpy(tkip_aes,"tkip/aes");
								}else
								{
									if(!strcmp(security,encrypt_wpapsk_tkip))
									{
										strcpy(encrypt,"WPA-PSK");
										strcpy(tkip_aes,"tkip");
									}else
									{
										if(!strcmp(security,encrypt_wpapsk_aes))
										{
											strcpy(encrypt,"WPA-PSK");
											strcpy(tkip_aes,"aes");
										}else
										{
											if(!strcmp(security,encrypt_wpa2psk_tkip))
											{
												strcpy(encrypt,"WPA2-PSK");
												strcpy(tkip_aes,"tkip");
											}else
											{
												if(!strcmp(security,encrypt_wpa2psk_aes))
												{
													strcpy(encrypt,"WPA2-PSK");
													strcpy(tkip_aes,"aes");
												}else
												{
													if(!strcmp(security,encrypt_wpa2_wpa1_psk_tkip))
													{
														strcpy(encrypt,"WPA/WPA2-PSK");
														strcpy(tkip_aes,"tkip");
													}else
													{
														if(!strcmp(security,encrypt_wpa2_wpa1_psk_aes))
														{
															strcpy(encrypt,"WPA/WPA2-PSK");
															strcpy(tkip_aes,"aes");
														}else
														{
																continue;
													
														}

													}

												}

											}
										}
									}

								}

							}
						}

					}

					if(strcmp(pssid,ssid) == 0)
					{
						strcpy(pmac,bssid);
						strcpy(pchanel,ch);
						strcpy(pencrypt,encrypt);
						strcpy(ptkip_aes,tkip_aes);
						ret = 0;
						break;
					}
				}
			} 
			else 					//disconnect !
			{ 
				//printf("scan error \n");
			} 
		}while(chars_read != NULL);
		pclose(read_fp); 
	}	
	return ret;

}


int setJoinWireless( const char *ssid, const char *pwd)  //wlan1 disabled=0;network restart
{

	char *name=NULL;
	char mac[64];
	char channel[64];
	char encrypt[64];
	char tkip_aes[64];
	

	int channel_num=0;

	char cur_mode[3]="\0";
	char mac_handle[20]="\0";
	char encrypt_config[32]="\0";
	char tkip_aes_config[16]="\0";
	
	char str_sp[64]="\0";


	char data[128];


	char uci_option_str[64]="\0";

	char cmd[64]="\0";

	unsigned int i;
	int ret;
	
	int    socket_id;
	struct   iwreq wrq;
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_id < 0)
	{
		printf("error::Open socket error!\n\n");
		return ;
	}


	if( !strlen(ssid) || strlen(ssid)>32)
	{
		return -1;
	}

	for(i = 0; i< 3;i++)
	{
		memset(mac,0,64);
		memset(channel,0,64);
		memset(encrypt,0,64);
		memset(tkip_aes,0,64);
		memset(encrypt_config,0,64);
		memset(tkip_aes_config,0,64);
		ret = cgi_scan(ssid,mac,channel,encrypt,tkip_aes);
		if(ret == 0)
			break;
	}
		if(i != 3)
		{
			printf("mac:%s\n",mac);
			printf("channel:%s\n",channel);
			printf("encrypt:%s\n",encrypt);
			printf("tkip_aes:%s\n",tkip_aes);
	
			if(!strcmp(encrypt,"NONE"))
				strcpy(encrypt_config,"none");
			else if(!strcmp(encrypt,"WEP"))
				strcpy(encrypt_config,"wep");
			else if(!strcmp(encrypt,"WPA-PSK"))
				strcpy(encrypt_config,"wpa");
			else if(!strcmp(encrypt,"WPA2-PSK"))
				strcpy(encrypt_config,"wpa2");
			else if(!strcmp(encrypt,"WPA/WPA2-PSK"))
				strcpy(encrypt_config,"mixed");
	
			if(!strcmp(tkip_aes,"tkip"))
				strcpy(tkip_aes_config,"TKIP");
			else if(!strcmp(tkip_aes,"aes"))
				strcpy(tkip_aes_config,"AES");
			else if(!strcmp(tkip_aes,"tkip/aes"))
				strcpy(tkip_aes_config,"TKIP+AES");
		
			memset(cmd,0,128);
			sprintf(cmd,"uci set wireless.@wifi-iface[1].mac=%s",mac);
			system(cmd);
			memset(cmd,0,128);
			sprintf(cmd,"uci set wireless.@wifi-iface[1].ssid=\"%s\"",ssid);
			system(cmd);
			memset(cmd,0,128);
			sprintf(cmd,"uci set wireless.@wifi-iface[1].encryption=%s",encrypt_config);
			system(cmd);
			if(strcmp(encrypt_config,"none") != 0)
			{
				memset(cmd,0,128);
				sprintf(cmd,"uci set wireless.@wifi-iface[1].key=%s",pwd);
				system(cmd);
				memset(cmd,0,128);
				sprintf(cmd,"uci set wireless.@wifi-iface[1].wpa_crypto=%s",tkip_aes_config);
				system(cmd);
			}
			else
			{
				system("uci delete wireless.@wifi-iface[1].key");
				system("uci delete wireless.@wifi-iface[1].wpa_crypto");
			}
			memset(cmd,0,128);
			sprintf(cmd,"uci set wireless.mt7628.channel=%s",channel);
			system(cmd);
			system("uci set wireless.@wifi-iface[1].disabled=0");
			system("uci commit");
			system("/usr/mips/cgi-bin/script/config_dns.sh on &");
			system("wifi >/dev/null");
			sleep(5);
			for(i = 0; i < 10;i++)
			{
				sleep(2);
				memset(data, 0x00, 128);
				strcpy(wrq.ifr_name, "apcli0");
				wrq.u.data.length = 128;
				wrq.u.data.pointer = data;
				wrq.u.data.flags = 0;
				ret = ioctl(socket_id, RTPRIV_IOCTL_SHOW_CONNSTATUS, &wrq);
				if(ret != 0)
				{
					printf("error::get apcli0 connstatus error\n\n");
					continue;
				}
				ret = *(unsigned int *)wrq.u.data.pointer;
				if(ret == 1)
				{
					printf(">>>>>>>>>>>>>>>>>apcli0 connected\n");
					break;
				}
			}
			close(socket_id);
			if(i != 10)
			{
				//inet_pton(AF_INET,pSmartlinkParam->sz_ip,&ipval);
				//tx_ack_app(ipval,pSmartlinkParam->sh_port);
				
				system("killall refresh_aplist");
				system("refresh_aplist &");
			}
		}
	return 0;
}
/*
void cgi_log( char *string){
	FILE *fw_fp;
	int f_size=0;
	if( (fw_fp=fopen("/tmp/log.txt","ab+"))==NULL)    // write and read,binary
	{
		exit(1);
	}		
	f_size=fwrite(string,1,strlen(string),fw_fp);
	fclose(fw_fp);
	return;
}
*/

void main()
{
		char ret_buf[2048]="\0";
		char ssid[32]="\0";
		char key[32]="\0";
		char new_code[CODE_LEN]="\0";		
		char *web_str=NULL;
		int ret=0;

		char uci_option_str[64]="\0";
		ctx=uci_alloc_context();

		printf("Content-type:text/plain\r\n\r\n");
		sprintf(ret_buf,"%s","{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{}}");
		printf("%s",ret_buf);

		fflush(stdout);

		if((web_str=GetStringFromWeb())==NULL)
		{
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":4,\"errorMessage\":\"Can not get any parameters.\"}");
			printf("%s",ret_buf);
			fflush(stdout);
			uci_free_context(ctx);
			return ;

		}
		processString(web_str,"ssid",ssid);
		cgi_log(ssid);
		cgi_log("====");

		processString(web_str,"key",key);
		cgi_log(key);
		cgi_log("++++++++");

		ret=setJoinWireless(ssid,key);
		cgi_log(ret);

		free(web_str);
		uci_free_context(ctx);
		return ;
}

