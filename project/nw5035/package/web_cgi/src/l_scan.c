#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include "cgiWireless.h"
#include "uci_for_cgi.h"


int getScan( char *retstr)
{
	cgi_scan2("ra0", retstr);
	return 0;
}


int cgi_scan2(char *ifname, char *outstr)
{
	char buffer[1024];
	char tmp_buffer[512];
	FILE *read_fp; 
	int chars_read;
	char *encode_ssid;
	char *encode_password;
	int ret=0; 
	int ssid_leng_int;
	int i=0;
	unsigned char ch[4];
	unsigned char ssid[33];
	unsigned char ssid_leng[4];
	unsigned char bssid[20];
	unsigned char security[23];
	unsigned char siganl[9];
	unsigned char w_mode[8];
	unsigned char extch[7];
	unsigned char encrypt[20];
	unsigned char tkip_aes[20];
	unsigned char password[64];
	int type_ssid_code=0;
	system("iwpriv ra0 set SiteSurvey=1");
//	memset(buffer, 0, 1024 ); 
	sleep(1);
	read_fp = popen("iwpriv ra0 get_site_survey", "r");
	//strcat(outstr,"<APList>");
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
			if (chars_read > 0)   	//connect !
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
					type_ssid_code=is_UTF8_or_gb2312(ssid,strlen(ssid));
					if(type_ssid_code==is_gb2312)
					{
						continue;
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

					memcpy(siganl,buffer+84,9);
					del_space(siganl,9);
					memcpy(w_mode,buffer+93,8);
					del_space(w_mode,8);
					memcpy(extch,buffer+101,7);
					del_space(extch,7);
					encode_ssid=xmlEncode(ssid);
					memset(password,0,sizeof(password));
					if(is_connected_ap(bssid,password))
					{
						encode_password = xmlEncode(password);
						sprintf(tmp_buffer,"{\"ssid\":\"%s\",\"encrypt\":\"%s\",\"channel\":\"%s\",\"rssi\":%d,\"tkip_aes\":\"%s\",\"mac\":\"%s\",\"record\":\"1\",\"password\":\"%s\"},",encode_ssid,encrypt,ch,atoi(siganl)-120,tkip_aes,bssid,encode_password);
						free(encode_password);
					}
					else
					{
						sprintf(tmp_buffer,"{\"ssid\":\"%s\",\"encrypt\":\"%s\",\"channel\":\"%s\",\"rssi\":%d,\"tkip_aes\":\"%s\",\"mac\":\"%s\",\"record\":\"0\"},",encode_ssid,encrypt,ch,atoi(siganl)-120,tkip_aes,bssid);
					}
					free(encode_ssid);
					strcat(outstr,tmp_buffer);
				}
			} 
			else 					//disconnect !
			{ 
				//printf("scan error \n");
			} 
		}while(chars_read>0);
		pclose(read_fp); 
	}
	//strcat(outstr,"</APList>");
	
	return 0;

}

void main()
{
		char ret_buf[4096];
		ctx=uci_alloc_context();
		printf("Content-type:text/plain\r\n\r\n");
		sprintf(ret_buf,"%s","{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"ssidList\":[");
		getScan(ret_buf);
		strcat(ret_buf,"]}}");
		printf("%s",ret_buf);
		fflush(stdout);
		uci_free_context(ctx);
		return ;
}

