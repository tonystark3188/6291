#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sys/ioctl.h>
#include <linux/wireless.h>
#include "uci_for_cgi.h"
#include "my_debug.h"
#include "msg.h"

#define BUFSIZ 512

#define IDLE_WIFI_TIME	(750) 
#define __DEBUG__  

#if WIRELESS_EXT <= 11
#ifndef SIOCDEVPRIVATE
#define SIOCDEVPRIVATE      0x8BE0
#endif
#define SIOCIWFIRSTPRIV      SIOCDEVPRIVATE
#endif

#define RT_PRIV_IOCTL    (SIOCIWFIRSTPRIV + 0x01)
#define RTPRIV_IOCTL_SET       (SIOCIWFIRSTPRIV + 0x02)
#define RTPRIV_IOCTL_BBP   (SIOCIWFIRSTPRIV + 0x03)
#define RTPRIV_IOCTL_MAC    (SIOCIWFIRSTPRIV + 0x05)
#define RTPRIV_IOCTL_E2P    (SIOCIWFIRSTPRIV + 0x07)
#define RTPRIV_IOCTL_STATISTICS    (SIOCIWFIRSTPRIV + 0x09)
#define RTPRIV_IOCTL_ADD_PMKID_CACHE    (SIOCIWFIRSTPRIV + 0x0A)
#define RTPRIV_IOCTL_RADIUS_DATA     (SIOCIWFIRSTPRIV + 0x0C)
#define RTPRIV_IOCTL_GSITESURVEY    (SIOCIWFIRSTPRIV + 0x0D)
#define RTPRIV_IOCTL_ADD_WPA_KEY    (SIOCIWFIRSTPRIV + 0x0E)
#define RTPRIV_IOCTL_GET_MAC_TABLE    (SIOCIWFIRSTPRIV + 0x0F)
#define RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT	(SIOCIWFIRSTPRIV + 0x1F)
#define RTPRIV_IOCTL_SHOW_CONNSTATUS	(SIOCIWFIRSTPRIV + 0x1B)
#define RTPRIV_IOCTL_GET_APCLI_RSSI	    (SIOCIWFIRSTPRIV + 0x1C)

#define MAC_ADDR_LEN        6
#define ETH_LENGTH_OF_ADDRESS      6
#define MAX_LEN_OF_MAC_TABLE      64

typedef union _MACHTTRANSMIT_SETTING {
	struct {
		unsigned short MCS:7;	/* MCS */
		unsigned short BW:1;	/*channel bandwidth 20MHz or 40 MHz */
		unsigned short ShortGI:1;
		unsigned short STBC:2;	/*SPACE */
		unsigned short rsv:3;
		unsigned short MODE:2;	/* Use definition MODE_xxx. */
	} field;
	unsigned short word;
} MACHTTRANSMIT_SETTING, *PMACHTTRANSMIT_SETTING;

typedef struct _RT_802_11_MAC_ENTRY {
	unsigned char ApIdx;
	unsigned char Addr[MAC_ADDR_LEN];
	unsigned char Aid;
	unsigned char Psm;		/* 0:PWR_ACTIVE, 1:PWR_SAVE */
	unsigned char MimoPs;		/* 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled */
	signed char AvgRssi0;
	signed char AvgRssi1;
	signed char AvgRssi2;
	unsigned int ConnectedTime;
	MACHTTRANSMIT_SETTING TxRate;
	unsigned int LastRxRate;
	signed short StreamSnr[3];				/* BF SNR from RXWI. Units=0.25 dB. 22 dB offset removed */
	signed short SoundingRespSnr[3];			/* SNR from Sounding Response. Units=0.25 dB. 22 dB offset removed */
/*	signed short TxPER;	*/					/* TX PER over the last second. Percent */
/*	signed short reserved;*/

} RT_802_11_MAC_ENTRY, *PRT_802_11_MAC_ENTRY;

typedef struct _RT_802_11_MAC_TABLE {
	unsigned long Num;
	RT_802_11_MAC_ENTRY Entry[MAX_LEN_OF_MAC_TABLE];
} RT_802_11_MAC_TABLE, *PRT_802_11_MAC_TABLE;

//FILE *fp = fopen("/tmp/mnt/USB-disk-1/log3.txt","a+");\
#define p_debug(args...) do{\
        struct timeval tnow;\
        FILE *fp = fopen("/dev/console","w");\
        if(fp){\
                gettimeofday(&tnow, NULL);\
                fprintf(fp,"[daemon][%08d.%06d][%s][-%d] ", (unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec, __FUNCTION__, __LINE__);\
                fprintf(fp,##args);\
                fprintf(fp,"\n");\
                fclose(fp);\
        }\
}while(0)


int isPlaying(){
	char buffer[BUFSIZ]; 
	FILE *read_fp; 
	int chars_read; 
	int count=0; 
	memset( buffer, 0, BUFSIZ ); 
	char tmpStr[64];

	sprintf(tmpStr,"%s","ps |grep file_download|wc -l");

	read_fp=popen(tmpStr,"r");
	if(read_fp!=NULL)
	{
		chars_read = fread(buffer, sizeof(char), BUFSIZ-1, read_fp); 
		if (chars_read > 0){
			//p_debug("buffer==%s==",buffer);
			//count=atoi(buffer);
			count=buffer[0]-'0';
			count=count-2;
			p_debug("count=%d",count);
			pclose(read_fp);
			//return count;
		}else {
			p_debug("chars_read=%d",chars_read);
			pclose(read_fp);
			//return 0;
		}

	}else {
		p_debug("read fp error");
		//return 0;
	}

	//sprintf(retstr,"<Users count=\"%d\"/>",count);
	//p_debug(retstr);
	return count;



}

int letvIsAlive(){


	char buffer[BUFSIZ]; 
	FILE *read_fp; 
	int chars_read; 
	int count=0; 
	memset( buffer, 0, BUFSIZ ); 
	char tmpStr[64];

	sprintf(tmpStr,"%s","ps |grep dm_letv|wc -l");

	read_fp=popen(tmpStr,"r");
	if(read_fp!=NULL)
	{
		chars_read = fread(buffer, sizeof(char), BUFSIZ-1, read_fp); 
		if (chars_read > 0){
			//printf("buffer==%s==",buffer);
			//count=atoi(buffer);
			count=buffer[0]-'0';
			count=count-2;
			printf("count=%d\n",count);
			pclose(read_fp);
			//return count;
		}else {
			printf("chars_read=%d\n",chars_read);
			pclose(read_fp);
			//return 0;
		}

	}else {
		printf("read fp error\n");
		//return 0;
	}

	//sprintf(retstr,"<Users count=\"%d\"/>",count);
	//printf(retstr);
	return count;

}
#define data_len 4096
int isPhoneConnected( char *mode){
	//printf("access isPhoneConnected\n");

	char data[data_len];
	int    socket_id;
	struct   iwreq wrq;
	int client_status=0;
	int ret = -1;
	int current_sta_cnt=0;
	FILE *read_fp; 
	int chars_read = 0;
	char buffer[64] = "\0";
	if(strcmp(mode,"ap")==0){
		socket_id = socket(AF_INET, SOCK_DGRAM, 0);
		if(socket_id < 0)
		{
			printf("error::Open socket error!\n\n");
			return -1;
		}
		//printf("creat socket ok\n");

		memset(data, 0x00, data_len);
		strcpy(wrq.ifr_name, "ra0");
		wrq.u.data.length = data_len;
		wrq.u.data.pointer = data;
		wrq.u.data.flags = 0;
		//printf("creat socket ok\n");

		ret = ioctl(socket_id, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &wrq);
		
		if(ret != 0)
		{
			printf("error::get mac table\n\n");
			close(socket_id);
			return 0;
		}
		//printf("creat socket ok\n");
		//printf("ioctl socket ok\n");
		RT_802_11_MAC_TABLE    *mp;
		
		//printf("ioctl socket ok\n");
		mp = (RT_802_11_MAC_TABLE *)wrq.u.data.pointer;
		//printf("ioctl socket ok2222\n");
		current_sta_cnt = mp->Num;
		printf("connected phone =%d\n",current_sta_cnt);
		//printf("ioctl socket ok2222\n");

		if(mp->Entry->ConnectedTime>5)
			current_sta_cnt=mp->Num;			
		else 	current_sta_cnt=0;
		//intf("connected time =%d\n",mp->Entry->ConnectedTime);
		//printf("connected phone =%s\n",mp->Entry->Addr);		
		close(socket_id);


	}else if(strcmp(mode,"sta")==0){
		read_fp=popen("wpa_cli all_sta | grep dot11RSNAStatsSTAAddress | wc -l","r");
		if(read_fp != NULL)
		{
			chars_read = fread(buffer, sizeof(char), 64-1, read_fp); 
			if (chars_read > 0)
			{
				current_sta_cnt = atoi(buffer);
				pclose(read_fp);
				
			}else
			{
				pclose(read_fp);
				//sprintf(retstr,"<Return status=\"false\">popen error!</Return>");
				return -1;
			}
		}
		else
		{
			//sprintf(retstr,"<Return status=\"false\">popen error!</Return>");
			return -1;
		}

	}
	//printf("leave isPhoneConnected\n");

	return current_sta_cnt;

}

int isClientConnected(char *mode){
	//printf("access isClientConnected\n");
	char data[data_len];
	int    socket_id;
	struct   iwreq wrq;
	int client_status=0;
	int ret = -1;

	if(strcmp(mode,"sta")==0) return 0;

	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_id < 0)
	{
		printf("error::Open socket error!\n\n");
		return -1;
	}
	strcpy(wrq.ifr_name, "ra0");
	wrq.u.data.length = data_len;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;
	while(ret != 0)
	{
		ret = ioctl(socket_id, SIOCGIWNAME, &wrq);
		if(ret != 0)
		{
			printf("error::get wireless name\n\n");
			sleep(5);
			ret = ioctl(socket_id, SIOCGIWNAME, &wrq);
			if(ret != 0) break;
		}
	}
	//sleep(10);
	memset(data, 0x00, data_len);
	strcpy(wrq.ifr_name, "apcli0");
	wrq.u.data.length = data_len;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;
	ret = ioctl(socket_id, RTPRIV_IOCTL_SHOW_CONNSTATUS, &wrq);
	if(ret != 0)
	{
		printf("error::get apcli0 connstatus error\n\n");
		return -1;
	}
	close(socket_id);
	client_status = *(unsigned int *)wrq.u.data.pointer;
	if(client_status == 1)
	{
		printf("router connected\n");
		//system("uci set wireless.@wifi-iface[1].connected=1");
		//system("uci commit");
	}
	//printf("leave isClientConnected\n");
	return client_status;

}

typedef struct _http_tcpclient{
	int 	socket;
	int 	remote_port;
	char 	remote_ip[16];
	struct sockaddr_in _addr; 
	int 	connected;
	char 	host[256];
	int 	time_out;
	char 	*request;
	char 	*headers;
} http_tcpclient;
/** Number of micro-seconds in a milli-second. */
#define USECS_IN_MSEC 1000

/** Number of milliseconds in 1 second */
#define MSECS_IN_SEC  1000

int http_tcpclient_create(http_tcpclient *pclient,const char *host, int port,int time_out)
{
	int ret=0;
	//printf("access http_tcpclient_create host = %s,port = %d", host, port);
	struct hostent *he;
	struct timeval tv_out;

	if(pclient != NULL) 
	{
		if((he = gethostbyname(host))==NULL){
			ret=-2;
		}else{
			strcpy(pclient->host,host);
			pclient->remote_port = port;
			strcpy(pclient->remote_ip,inet_ntoa( *((struct in_addr *)he->h_addr) ));

			pclient->_addr.sin_family = AF_INET;
			pclient->_addr.sin_port = htons(pclient->remote_port);
			pclient->_addr.sin_addr = *((struct in_addr *)he->h_addr);

			if((pclient->socket = socket(AF_INET,SOCK_STREAM,0))==-1){
				ret=-3;
			}else{
				pclient->time_out = time_out;

				/* setsockopt set send and rev time out  */
				tv_out.tv_sec = time_out / MSECS_IN_SEC;
				tv_out.tv_usec = (time_out % MSECS_IN_SEC) * USECS_IN_MSEC;
				setsockopt(pclient->socket, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));
				setsockopt(pclient->socket, SOL_SOCKET, SO_SNDTIMEO, &tv_out, sizeof(tv_out));
			}
		}
	}else ret=-1;
	//if(ret<0)	report_status_err_time++;
	//printf("leave http_tcpclient_create ret=%d",ret);
	return ret;
}

int http_tcpclient_conn(http_tcpclient *pclient)
{
	//p_debug("access http_tcpclient_conn");
	

	//none block way
	int ret=0;
	int res=connect(pclient->socket, (struct sockaddr *)&pclient->_addr,sizeof(struct sockaddr));

	if (0 == res)  
	 {  	
	 	res=connect(pclient->socket, (struct sockaddr *)&pclient->_addr,sizeof(struct sockaddr));
	  	p_debug("socket connect succeed immediately.\n");  
	    ret = 0;  
	 }  
	 else  
	 {  
  		 //p_debug("get the connect result by select().\n");  
  		 if (errno == EINPROGRESS)  
  		 {  
            int times = 0;  
            while (times++ < 5)  
            {  
                fd_set rfds, wfds;  
                struct timeval tv;  
                  
                //p_debug("errno = %d\n", errno);  
                FD_ZERO(&rfds);  
                FD_ZERO(&wfds);  
                FD_SET(pclient->socket, &rfds);  
                FD_SET(pclient->socket, &wfds);  
                  
                /* set select() time out */  
                tv.tv_sec = 10;   
                tv.tv_usec = 0;  
                int selres = select(pclient->socket + 1, &rfds, &wfds, NULL, &tv);  
                switch (selres)  
                {  
                    case -1:  
                        p_debug("select error\n");  
                        ret = -1;  
                        break;  
                    case 0:  
                        p_debug("select time out\n");  
                        ret = -2;  
                        break;  
                    default:  
                        if (FD_ISSET(pclient->socket, &rfds) || FD_ISSET(pclient->socket, &wfds))  
                        {  
                      	 	#if 1  
                            connect(pclient->socket, (struct sockaddr *)&pclient->_addr, sizeof(struct sockaddr_in));  
                            int err = errno;  
                            if  (err == EISCONN)  
                            {  
	                            pclient->connected = 1;
                                p_debug("connect success.\n");  
                                ret = 0;  
								
                            }  
                            else  
                            {  
                                p_debug("connect failed. errno = %d\n, retry", errno);  
                                //p_debug("FD_ISSET(sock_fd, &rfds): %d\n FD_ISSET(sock_fd, &wfds): %d\n", FD_ISSET(pclient->socket, &rfds) , FD_ISSET(pclient->socket, &wfds));  
                                ret = -3;  
                            }  

                 	      	#endif  
                       
                        }  
                        else  
                        {  
                            printf("haha\n");  
                        }  
                }  
                  
                if (-1 != selres && (ret != 0))  
                {  
                    p_debug("check connect result again... %d\n", times);  
                    continue;  
                }  
                else  
                {  
                    break;  
                }  
            }  
	  }  
	  else  
	  {  
		   p_debug("connect to host failed.\n");  
		   ret = -4;  
	  }  
	 } 
	

	return ret;
}
int http_tcpclient_close(http_tcpclient *pclient)
{
	close(pclient->socket);

	return 0;
}

#define TIMEOUT 20
#define TEST_LETV_SERVER_PORT	5540
#define TEST_LETV_SERVER_IP 		"115.182.94.152"

#define LETV_SERVER_PORT	80
#define LETV_SERVER_IP 		"lebao.leinlife.com"

int isInternetOK( char *wifimode){
	
	char server[64]="play.g3proxy.lecloud.com";
	int port=80;
	int ret=0;
	http_tcpclient pclient;
	memset(&pclient, '\0', sizeof(http_tcpclient));
	#if 0
	int ret1=http_tcpclient_create(pclient,server,port,TIMEOUT);
	if(ret1<0) {
		printf("Create socket error.n = %d\n",n);
		return -1;
	}
	int ret2=http_tcpclient_conn(pclient);
	if(ret2<0) return -2;
	#endif

	//0.check router is connected?
	if(isClientConnected(wifimode)!=1){
		printf("router is not connected.\n");		
		ret=-1;
		return ret;
	}
	//1.check dns resolve
	if((gethostbyname(LETV_SERVER_IP))==NULL){
		printf("can't resolve server ip\n");		
		ret=-2;
		return ret;
	}
	
	//2.check connect to letv's ip

	int ret1=http_tcpclient_create(&pclient, LETV_SERVER_IP, LETV_SERVER_PORT, TIMEOUT);

	if ( ret1< 0) {
		printf("Create socket error\n");
		return -3;
	}
	
	int ret2=http_tcpclient_conn(&pclient);
	if(ret2<0){
		printf("connect server error\n");
		return -4;
	}
	http_tcpclient_close(&pclient);
	return 0;
	
}

#define AP_SUM  20

struct ap_info{
	char mac[33];
	int  signal;
	char channel[4];
};
struct ap_info aplist[AP_SUM];
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

int isRouterAvailable()
{
	char buffer[1024]="\0"; 
	char uci_option_str[64]="\0";
	char sum[8]="\0";
	int remote_ap_sum = 0;
	int tmp_sum = 0;
	FILE *read_fp; 
	char *chars_read;  
	char bssid[20]="\0"; 
	char ssid[33]="\0";
	char remote_ap_mac[33]="\0";
	char remote_ap_ssid[33]="\0";	
	unsigned char security[23];	
	//unsigned char encrypt[20];
	//unsigned char tkip_aes[20];	
	char encrypt_config[32]="\0";
	char tkip_aes_config[16]="\0";
	
	char ssid_leng[4]="\0";
	char channel[4]="\0";
	char saved_signal[4]="\0";
	int rssi=0;
	int saved_rssi=0;
	char disabled[8] = "\0";
	int ssid_leng_int;
	int i,j=0;
	
	unsigned char signal[9]="\0"; 
	ctx=uci_alloc_context();



	memset(sum,0,sizeof(sum));
	sprintf(uci_option_str,"remoteAPlist.ap.sum");
	uci_get_option_value(uci_option_str,sum);
	memset(uci_option_str,'\0',64);
	remote_ap_sum=atoi(sum);
	tmp_sum = remote_ap_sum;

	remote_ap_sum = tmp_sum;
	system("iwpriv p2p-p2p0-0 set SiteSurvey=1");
	sleep(1);
	read_fp = popen("iwpriv p2p-p2p0-0 get_site_survey", "r"); 
	if(read_fp!=NULL)
	{
		do
		{ 
			memset(buffer, 0, 1024 );
			memset(bssid,0,20); 
			memset(signal,0,9);  
			memset(ssid,0,33); 
			memset(ssid_leng,0,4); 
			memset(channel,0,4);
			chars_read = fgets(buffer, 1024-1, read_fp); 
			if (chars_read != NULL)   	 
			{ 
				if((58>buffer[0])&&(buffer[0]>=48))
				{
					memcpy(channel,buffer,4);
					del_space(channel,4);
					memcpy(ssid,buffer+4,33);
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
					printf("scaned_ssid=%s\n",ssid);
					memcpy(bssid,buffer+41,20);
					del_space(bssid,20); 
					
					memcpy(signal,buffer+84,9);
					del_space(signal,9); 	
					#if 0
					memcpy(security,buffer+61,23);
					del_space(security,23);
					if(!strcmp(security,encrypt_none))
					{

						strcpy(tkip_aes_config,"");
						strcpy(encrypt_config,"none");
					}else if(!strcmp(security,encrypt_wep))
					{

						strcpy(tkip_aes_config,"");
						strcpy(encrypt_config,"wep");
					}else
					{
						if(!strcmp(security,encrypt_wpapsk_tkip_aes))
						{
					
							strcpy(encrypt_config,"wpa");
							strcpy(tkip_aes_config,"TKIP+AES");
						}else
						{
							if(!strcmp(security,encrypt_wpa2psk_tkip_aes))
							{
					
								strcpy(encrypt_config,"wpa2");
								strcpy(tkip_aes_config,"TKIP+AES");
							}else
							{
								if(!strcmp(security,encrypt_wpa2_wpa1_psk_tkip_aes))
								{
			
									strcpy(encrypt_config,"mixed");
									strcpy(tkip_aes_config,"TKIP+AES");
								}else
								{
									if(!strcmp(security,encrypt_wpapsk_tkip))
									{
							
										strcpy(encrypt_config,"wpa");
										strcpy(tkip_aes_config,"TKIP");
									}else
									{
										if(!strcmp(security,encrypt_wpapsk_aes))
										{
		
											strcpy(encrypt_config,"wpa");
											strcpy(tkip_aes_config,"AES");
										}else
										{
											if(!strcmp(security,encrypt_wpa2psk_tkip))
											{

												strcpy(encrypt_config,"wpa2");
												strcpy(tkip_aes_config,"TKIP");
											}else
											{
												if(!strcmp(security,encrypt_wpa2psk_aes))
												{
		
													strcpy(encrypt_config,"wpa2");
													strcpy(tkip_aes_config,"AES");
												}else
												{
													if(!strcmp(security,encrypt_wpa2_wpa1_psk_tkip))
													{
												
														strcpy(encrypt_config,"mixed");
														strcpy(tkip_aes_config,"TKIP");
													}else
													{
														if(!strcmp(security,encrypt_wpa2_wpa1_psk_aes))
														{
								
															strcpy(encrypt_config,"mixed");
															strcpy(tkip_aes_config,"AES");
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
					#endif

					for(i = 0;i < remote_ap_sum; i++)
					{
						memset(remote_ap_ssid,0,sizeof(remote_ap_ssid));
						sprintf(uci_option_str,"remoteAPlist.@ap[%d].ssid",i);
						uci_get_option_value(uci_option_str,remote_ap_ssid);
						memset(uci_option_str,'\0',64);
						//printf("scaned_______ssid=(%s)\n",ssid);
						//printf("saved_remote_ssid=(%s)\n",remote_ap_ssid);
						if(strcmp(ssid,remote_ap_ssid) == 0)
						{	
							j++;
							memset(saved_signal,0,sizeof(saved_signal));
							sprintf(uci_option_str,"remoteAPlist.@ap[%d].signal",i);
							uci_get_option_value(uci_option_str,saved_signal);
							memset(uci_option_str,0,64);
							rssi=atoi(signal);
							saved_rssi=atoi(saved_signal);
							printf("scaned_rssi=%d\n",rssi);
							printf("saved_rssi=%d\n",saved_rssi);
							if(0){//save the stronger signal
								printf("save the stronger signal...\n");

								sprintf(uci_option_str,"remoteAPlist.@ap[%d].signal=%d",i,rssi);
								uci_set_option_value(uci_option_str);
								memset(uci_option_str,0,64);
								
								sprintf(uci_option_str,"remoteAPlist.@ap[%d].mac=%s",i,bssid);
								uci_set_option_value(uci_option_str);
								memset(uci_option_str,0,64);

								sprintf(uci_option_str,"remoteAPlist.@ap[%d].channel=%s",i,channel);
								uci_set_option_value(uci_option_str);
								memset(uci_option_str,0,64);

								sprintf(uci_option_str,"remoteAPlist.@ap[%d].encrypt=%s",i,encrypt_config);
								uci_set_option_value(uci_option_str);
								memset(uci_option_str,0,64);
								sprintf(uci_option_str,"remoteAPlist.@ap[%d].wpa_crypto=%s",i,tkip_aes_config);
								uci_set_option_value(uci_option_str);
								memset(uci_option_str,0,64);
								

							}
								//strcpy(p[i].channel,channel);
								//strcpy(p[i].mac,bssid);
								//p[i].signal += atoi(signal);
								printf("ssid: %s---mac:%s---signal :%d\n",ssid,bssid,rssi);}
					
					}
				}
			}  
		}while(chars_read != NULL);
		pclose(read_fp); 
	} 
	uci_free_context(ctx);
	printf("finally scaned AP sum=%d\n",j);
	return j;

}
int getTimeOfDay(){
	char buffer[64]={0};
	FILE *read_fp;
	int chars_read = 0;
	int Hour=0;
	read_fp=popen("/bin/date +%H","r");
	if(read_fp != NULL)
	{
		chars_read = fread(buffer, sizeof(char), 64-1, read_fp); 
		if (chars_read > 0)
		{
			Hour = atoi(buffer);
			printf("Hour=%d\n",Hour);
			pclose(read_fp);
			return Hour;
		}else
		{
			pclose(read_fp);
			//sprintf(retstr,"<Return status=\"false\">popen error!</Return>");
			return -1;
		}
	}
	else
	{
		//sprintf(retstr,"<Return status=\"false\">popen error!</Return>");
		return -1;
	}
	

}

void main()
{
	int i=0;
	int ret=0;
	int sleep_time=0;
	int phoneNotConnectedTime=0;
	int led_flag=1;
	int PhoneConnected=0;
	char isFollow[8]={0};
	char isDownloading[8]={0};
	int check_internet=60;
	char wifimode[8]={0};
	char uci_option_str[64]="\0";
	int DownloadingTime=3;
	int query=1;
	int playing=0;
	int net=0;
	char start_upgrade[8]={0};

	getTimeOfDay();
	char led_status[8]="\0";
	while(1)
		{

		get_conf_str(start_upgrade,"start_upgrade");
		if(!strcmp(start_upgrade,"1")){//
			break;
		}else if(!strcmp(start_upgrade,"2")){
			break;
		}else if(!strcmp(start_upgrade,"3")){
			break;
		}else{
			system("letv_gpio 3 &");
			
			get_conf_str(start_upgrade,"start_upgrade");

			if(!strcmp(start_upgrade,"3"))
				break;

			ctx=uci_alloc_context();
			sprintf(uci_option_str,"system.@system[0].wifimode");
			memset(wifimode,0,sizeof(wifimode));		
			uci_get_option_value(uci_option_str,wifimode);
			uci_free_context(ctx);
			printf("wifimode=%s\n",wifimode);
			if(isInternetOK(wifimode)<0 && query==1)
			{
				printf("internet is not ok.\n");
				//if(check_internet>100){
				
					//updateSysVal("net","false");
					//check_internet=0;
				if(strcmp(wifimode,"ap")==0){
					updateSysVal("net","false");
					check_internet=50;
					//system("wifi_connect &");
				}
	
				if(!strcmp(wifimode,"sta")){
					updateSysVal("net","false");
					if(isRouterAvailable()>0)
						updateSysVal("isCanLinkNet","1");
					else
						updateSysVal("isCanLinkNet","0");
				}
				query=0;
			}
	
			
			ret=letvIsAlive();
			if(ret==0){
				//
				usleep(200);
				ret=letvIsAlive();
				if(ret==0){
					system("ps w |grep dm_letv >/dev/console");
	
					system("killall -9 dm_letv >/dev/console");
					system("dm_letv &");
					i++;
					printf("now restart %d times\n",i);
					sleep(3);
				}
			}else if(ret>1){
				usleep(200);
				ret=letvIsAlive();
				if(ret>1){
					system("ps w |grep dm_letv >/dev/console");
						
					printf("|||||||||||||||||||||||||||||");
					printf("||||||too many dm_letv|||||||");
					printf("|||||||||||||||||||||||||||||");
	
					system("killall -9 dm_letv");
				}
			}else if(ret==1){
				printf("restart %d times\n",i);
			}
			sleep(1);
	//////////////////////
			if(isPlaying()>=1){//is playing
	
				get_conf_str(led_status,"led_status");
				if(strcmp(led_status,"4")){//没有下载，也没有播放
//					system("mcu_control -s 4");
					system("pwm_control 1 0 0;pwm_control 0 0 10000");
					updateSysVal("led_status","4");
					playing=1;
				}
	
			}else{
				if(playing==1)//播放之后，没有再播
					{	
//						system("mcu_control -s 1");//on
						system("pwm_control 1  1 0;pwm_control 1 0 0");
						updateSysVal("led_status","1");
						playing=0;
					}
			}
	//////////////////////
			
			sleep_time++;
			check_internet++;
			if(check_internet>10){
	
				if(!strcmp(wifimode,"ap")){
					if((net=isInternetOK(wifimode))<0)
					{
						printf("internet is not ok.\n");
						if(check_internet>90){
							updateSysVal("net","false");
							check_internet=0;
							system("wifi_connect &");
						}
					}else{
						check_internet=0;
						updateSysVal("net","true");
						printf("internet is ok.\n");
					}
				}else if(!strcmp(wifimode,"sta")){
					updateSysVal("net","false");
					
					if(check_internet>60){
						check_internet=0;
						if(isRouterAvailable()>0){
							updateSysVal("isCanLinkNet","1");
							if(isPhoneConnected(wifimode)<0)
								{
								system("/bin/wifi_switch.sh ap");
							}
						}
						else
							updateSysVal("isCanLinkNet","0");
					}
				}
	
			}
			if((net<0&&net>-4)||(!strcmp(wifimode,"sta"))){//internet is not good
				updateSysVal("net","false");
				get_conf_str(led_status,"led_status");
				if((playing!=1)&&(strcmp(led_status,"1"))&&(strcmp(led_status,"3"))){//没有播放灯也不是常亮,也不是开机启动后闪烁
//					system("mcu_control -s 1");
					system("pwm_control 1  1 0;pwm_control 1 0 0");
					updateSysVal("led_status","1");
				}
					
			}
			PhoneConnected=isPhoneConnected(wifimode);
	
			if(PhoneConnected){
				phoneNotConnectedTime=0;
			}else 
				phoneNotConnectedTime++;
			
			if(PhoneConnected){
				sleep_time=0;
			}else if(isClientConnected(wifimode)){
				sleep_time=0;					
			} 
			printf("wifimode=%s\n",wifimode);
			//
			if(PhoneConnected ){			

				get_conf_str(led_status,"led_status");
				if(!strcmp(led_status,"3")){
					//system("mcu_control -s 1");
					system("pwm_control 1  1 0;pwm_control 1 0 0");					
					updateSysVal("led_status","1");
				}
					
			
	
				
				//led_flag=0;
#if 0
				printf("restart xxx2 times\n");
				memset(uci_option_str,'\0',64);
				//ctx=uci_alloc_context();
				strcpy(uci_option_str,"system.@system[0].led_status");			//name
				printf("restart xxx3 times\n");
				uci_get_option_value(uci_option_str,led_status);
				printf("restart xxx4 times\n");
				printf("led_status=%s\n",led_status);
				if(!strcmp(led_status,"3"))system("mcu_control -s 1");
				memset(uci_option_str,'\0',64);
				strcpy(uci_option_str,"system.@system[0].led_status=1");			//name
				uci_set_option_value(uci_option_str);
				system("uci commit");
				//uci_free_context(ctx);
				led_flag=0;
#endif
			}else if(PhoneConnected==0){
				get_conf_str(isDownloading,"isDownloading");
				get_conf_str(led_status,"led_status");
				if(strcmp(isDownloading,"true")){
					if(strcmp(led_status,"3")){
//						system("mcu_control -s 3");
						system("pwm_control 1  1 0;pwm_control 2 0 0");
						updateSysVal("led_status","3");
					}
				}	
			}
			if(sleep_time>1800)
				{
				printf("|||||||||FORCE POWER OFF||||||||||");
				system("cp -f /tmp/mnt/USB-disk-1/hack/.tasklist /tmp/mnt/USB-disk-1/hack/.tasklist.bak");
				p_debug("cp -f /tmp/mnt/USB-disk-1/hack/.tasklist");	
				system("sync");
				usleep(100000);
				system("killall -9 dm_letv >/dev/null");
//				system("mcu_control -s 7");// force power off
				system("pwm_control 1 1 0;pwm_control 1 0 100");
				system("letv_gpio 2 &");
			}
	
			//printf("restart xxx4 times\n");
	
			if(phoneNotConnectedTime>3600)
				{
				
				get_conf_str(isFollow,"isFollow");
				get_conf_str(isDownloading,"isDownloading");
				if((!strcmp(isDownloading,"false"))){
					if((!strcmp(isFollow,"false")))//没有追剧，没有手机连接，没有下载，1个小时，关机。
					{
						printf("|||||||||FORCE POWER OFF||||||||||");
						system("cp -f /tmp/mnt/USB-disk-1/hack/.tasklist /tmp/mnt/USB-disk-1/hack/.tasklist.bak");
						p_debug("cp -f /tmp/mnt/USB-disk-1/hack/.tasklist");	
						system("sync");
						usleep(100000);						
						system("killall -9 dm_letv >/dev/null");
///						system("mcu_control -s 7");// force power off
						system("pwm_control 1 1 0;pwm_control 1 0 100");
						system("letv_gpio 2 &");

					}else if(!strcmp(isFollow,"true"))
					{

						if(getTimeOfDay()==(DownloadingTime+1)){// 追剧之后一小时内关机
							printf("|||||||||FORCE POWER OFF||||||||||");
							printf("|||||||||FORCE POWER OFF||||||||||");
							system("cp -f /tmp/mnt/USB-disk-1/hack/.tasklist /tmp/mnt/USB-disk-1/hack/.tasklist.bak");
							p_debug("cp -f /tmp/mnt/USB-disk-1/hack/.tasklist");	
							system("sync");
							usleep(100000);													
							system("killall -9 dm_letv >/dev/null");
							//system("mcu_control -s 7");// force power off
							system("pwm_control 1 1 0;pwm_control 1 0 100");
							system("letv_gpio 2 &");
						}
						
					}
				}else {
					//if(getTimeOfDay()==24) DownloadingTime=-1;
					if(getTimeOfDay()==0)DownloadingTime=0;
					if(getTimeOfDay()==1)DownloadingTime=1;
					if(getTimeOfDay()==2)DownloadingTime=2;
					if(getTimeOfDay()==3)DownloadingTime=3;

				}
	
				//printf("restart xxx5 times\n");
			}		

		}
		
	}

	if(!strcmp(start_upgrade,"1")){//
		system("killall -9 dm_letv >/dev/null");
	//	system("dd if=/tmp/mnt/USB-disk-1/ota/update.bin of=/tmp/image1_fw bs=1k conv=notrunc count=7936");
	//	system("dd if=/tmp/mnt/USB-disk-1/ota/update.bin of=/tmp/image1_md5 bs=1k skip=7936 conv=notrunc count=64");
	//	system("dd if=/tmp/mnt/USB-disk-1/ota/update.bin of=/tmp/image2_fw bs=1k skip=8000 conv=notrunc count=7936");
	//	system("dd if=/tmp/mnt/USB-disk-1/ota/update.bin of=/tmp/image2_md5 bs=1k skip=15936 conv=notrunc count=64");
	//	system("sync");
	//	system("rm /tmp/mnt/USB-disk-1/ota/update.bin");
//		system("mcu_control -s 6");
		system("pwm_control 1 0 0;pwm_control 0 0 5000");
		system("sysupgrade /tmp/mnt/USB-disk-1/ota/update.bin");

		return 0;
	}else if(!strcmp(start_upgrade,"2")){
		system("killall -9 dm_letv >/dev/null");
	//	system("dd if=/tmp/mnt/USB-disk-1/letv_ota/update.bin of=/tmp/image1_fw bs=1k conv=notrunc count=7936");
	//	system("dd if=/tmp/mnt/USB-disk-1/letv_ota/update.bin of=/tmp/image1_md5 skip=7936 bs=1k conv=notrunc count=64");
	//	system("dd if=/tmp/mnt/USB-disk-1/letv_ota/update.bin of=/tmp/image2_fw bs=1k skip=8000 conv=notrunc count=7936");
	//	system("dd if=/tmp/mnt/USB-disk-1/letv_ota/update.bin of=/tmp/image2_md5 bs=1k skip=15936 conv=notrunc count=64");
	//	system("sync");
	//	system("mcu_control -s 6");
	//	system("pwm_control 1 0 0;pwm_control 0 0 10000");
	//	system("rm /tmp/mnt/USB-disk-1/ota/update.bin");
		system("sysupgrade /tmp/mnt/USB-disk-1/ota/update.bin");
		return 0;
	} else if(!strcmp(start_upgrade,"3")){
		system("pwm_control 1 1 0;pwm_control 1 0 100;");//wifi led off
		system("mtd -r erase rootfs_data;");
		return 0;
	}

}
