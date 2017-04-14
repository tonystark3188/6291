#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/socket.h>
#include <linux/wireless.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
//#include <pcap.h>
#include "pthread.h"
#include "TXWifisync.h"
#include "TXDeviceSDK.h"

#define WAIT_SYNC_PACKAGE 		1
#define WAIT_DATE_PACKAGE 		2
#define SCAN_FINISH_PACKAGE		3
#define CHANNEL_SNIFFER_TIME 	6000
#define CHANNEL_WAIT_TIME 		3000

#define SCAN_TIMES   			(180)//500ms*180

#define PRINTF_FUNC() printf("%s:(%d) -- ", __FUNCTION__, __LINE__);
static char g_guid[32]={0};
static int  scan_channel[14] = {-1,1,6,11,2,3,4,5,7,8,9,10,12,13};
static int 	currentChannel;
static int 	snifferFlag;

static int exit_flag = 0; 

extern int cgi_scan(const char *pssid,char *pmac,char *pchanel,char *pencrypt,char *ptkip_aes);

void set_channel(int channel)
{
	char cmd[128] = "\0";
	printf("set channel %d !\n", channel);
	snprintf(cmd,128,"mlanutl mlan0 netmon 1 7 11 %d",channel);
	system(cmd);
	return;
}


void log_func(int level, const char* module, int line, const char* message)
{
	printf("%s\t%d\t%s\n", module, line, message);
	//return;
	if (level == 1)
	{
	    FILE * file = fopen("/tmp/devicelog", "aw+");
	    if (file)
	    {
	    	fprintf(file, "%s\t%d\t%s\n", module, line, message);
	        fclose(file);
	    }
	}
}


/**
 * 辅助函数: 从文件读取buffer
 * 这里用于读取 license 和 guid
 * 这样做的好处是不用频繁修改代码就可以更新license和guid
 */
static bool readBufferFromFile(char *pPath, unsigned char *pBuffer, int nInSize, int *pSizeUsed) 
{
	if (!pPath || !pBuffer) {
		return false;
	}

	int uLen = 0;
	FILE * file = fopen(pPath, "rb");
	if (!file) {
	    return false;
	}

	fseek(file, 0L, SEEK_END);
	uLen = ftell(file);
	fseek(file, 0L, SEEK_SET);

	if (0 == uLen || nInSize < uLen) {
		printf("invalide file or buffer size is too small...\n");
		return false;
	}

	*pSizeUsed = fread(pBuffer, 1, uLen, file);

	fclose(file);
	return true;
}

/** 
      * 计算两个时间的间隔，得到时间差 
      * @param struct timeval* resule 返回计算出来的时间 
      * @param struct timeval* x 需要计算的前一个时间 
      * @param struct timeval* y 需要计算的后一个时间 
      * return -1 failure ,0 success 
  **/ 
int timeval_subtract(struct timeval* result, struct timeval* x, struct timeval* y) 
{ 
    int nsec; 

    result->tv_sec = 0;
	result->tv_usec = 0;//init
    if ( x->tv_sec>y->tv_sec ) 
    {   printf("error in timeval_subtract\n");       return -1; }

    if ( (x->tv_sec==y->tv_sec) && (x->tv_usec>y->tv_usec) ) 
    {    printf("error in timeval_subtract222\n");      return -1; }

    result->tv_sec = ( y->tv_sec-x->tv_sec ); 
    result->tv_usec = ( y->tv_usec-x->tv_usec ); 

    if (result->tv_usec<0) 
    { 
              result->tv_sec--; 
              result->tv_usec+=1000000; 
    } 

    return 0; 
}


int symbol_encode(char *dest, char *src)
{
	char tmp[64];
	int len = 0, i = 0;
	memset(tmp, 0, 64);
	char *p_tmp = tmp;
	char *p_src = src;
	//printf("src = %s\n", src);
	len = strlen(src);
	for(i = 0; i < len; i++)
	{
		if((src[i] == 0x60) || (src[i] == 0x22) || (src[i] == 0x5C)) 
		{
			*p_tmp = 0x5C;
			p_tmp++;
		}
		*p_tmp = src[i];
		p_tmp++;
	}

	//printf("tmp = %s\n", tmp);
	strcpy(dest, tmp);
	return 0;
}


void on_smartlink_notify (tx_wifi_sync_param *pSmartlinkParam, void* pUserData)
{	
	printf("access on_smartlink_notify !!!!!!\r\n\r\n\n");
	int 	ret = -1;
	char 	mac[64];
	char 	macaddr[64];
	char 	channel[64];
	char 	encrypt[64];
	char 	tkip_aes[64];
	char 	encrypt_config[64];
	char 	tkip_aes_config[64];
	char 	cmd[128];
	int 	scanMatch = 0;
	int 	i= 0;

	//out monitor mode before scan
	system("mlanutl mlan0 netmon 0");
	
	printf("ssid:%s\n",pSmartlinkParam->sz_ssid);
	printf("psword:%s\n",pSmartlinkParam->sz_password);
	printf("ip:%s  port=%d\n",pSmartlinkParam->sz_ip,pSmartlinkParam->sh_port);	
	memset(cmd,0,128);
	sprintf(cmd,"echo \"ssid:%s\" >/dev/console",pSmartlinkParam->sz_ssid);
	system(cmd);
	memset(cmd,0,128);
	sprintf(cmd,"echo \"psword:%s\" >/dev/console",pSmartlinkParam->sz_password);
	system(cmd);
	memset(cmd,0,128);
	sprintf(cmd,"echo \"ip:%s  port=%d\" >/dev/console",pSmartlinkParam->sz_ip,pSmartlinkParam->sh_port);
	system(cmd);
	for(i = 0; i < 3;i++){
		memset(mac,0,64);
		memset(channel,0,64);
		memset(encrypt,0,64);
		memset(tkip_aes,0,64);
		memset(encrypt_config,0,64);
		memset(tkip_aes_config,0,64);
		ret = cgi_scan(pSmartlinkParam->sz_ssid,mac,channel,encrypt,tkip_aes);
		if(ret == 0){
			scanMatch = 1;
			break;
		}
	}
	if(scanMatch){
		printf("mac:%s\n",mac);
		printf("channel:%s\n",channel);
		printf("encrypt:%s\n",encrypt);
		printf("tkip_aes:%s\n",tkip_aes);

		//open client
		system("uci set wireless.@wifi-iface[1].disabled=0");
		//system("uci commit");

		memset(encrypt_config, 0,sizeof(encrypt_config));
		memset(tkip_aes_config, 0, sizeof(tkip_aes_config));
		if(strlen(encrypt)){
			if(!strcmp(encrypt,"NONE"))
				strcpy(encrypt_config,"none");
			else if(!strcmp(encrypt,"WEP"))
				strcpy(encrypt_config,"wep");
			else if(!strcmp(encrypt,"WPA-PSK"))
				strcpy(encrypt_config,"psk");
			else if(!strcmp(encrypt,"WPA2-PSK"))
				strcpy(encrypt_config,"psk2");
			else if(!strcmp(encrypt,"WPA/WPA2-PSK"))
				strcpy(encrypt_config,"mixed-psk");
			else
				printf("unknow encrypt: %s\n", encrypt);
		}

		if(strlen(tkip_aes)){
			if(!strcmp(tkip_aes,"tkip"))
				strcpy(tkip_aes_config,"tkip");
			else if(!strcmp(tkip_aes,"aes"))
				strcpy(tkip_aes_config,"ccmp");
			else if(!strcmp(tkip_aes,"tkip/aes"))
				strcpy(tkip_aes_config,"tkip+ccmp");
			else
				printf("unknow tkip_aes: %s\n", tkip_aes);
		}

		if(strlen(pSmartlinkParam->sz_ssid)){
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "uci set wireless.@wifi-iface[1].ssid=%s", pSmartlinkParam->sz_ssid);
			system(cmd);		
		}

		if(strlen(mac)){
			memset(macaddr, 0, sizeof(macaddr));
			macaddr[0]=*mac;macaddr[1]=*(mac+1);macaddr[2]=':';
			macaddr[3]=*(mac+2);macaddr[4]=*(mac+3);macaddr[5]=':';
			macaddr[6]=*(mac+4);macaddr[7]=*(mac+5);macaddr[8]=':';
			macaddr[9]=*(mac+6);macaddr[10]=*(mac+7);macaddr[11]=':';
			macaddr[12]=*(mac+8);macaddr[13]=*(mac+9);macaddr[14]=':';
			macaddr[15]=*(mac+10);macaddr[16]=*(mac+11);
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "uci set wireless.@wifi-iface[1].bssid=%s",macaddr);
			system(cmd);				
		}
		
		if(!strcmp(encrypt_config,"none") || !strcmp(encrypt_config,"wep")){
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd,"uci set wireless.@wifi-iface[1].encryption=%s",encrypt_config);
			system(cmd);
		}
		else{
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd,"uci set wireless.@wifi-iface[1].encryption=%s+%s",encrypt_config,tkip_aes_config);
			system(cmd);
		}

		if(strlen(pSmartlinkParam->sz_password)){
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd,"uci set wireless.@wifi-iface[1].key=%s",pSmartlinkParam->sz_password);
			system(cmd);
		}
#if 0
		if(channel!=NULL)
		{
			if(strlen(channel))
			{
				sprintf(str_sp,"wireless.radio0.channel=%s",channel);
				uci_set_option_value(str_sp);
				memset(str_sp,0,64);
			}

			channel_num=atoi(channel);
			if(channel_num>7)
			{
				sprintf(str_sp,"wireless.radio0.htmode=HT40-");
				uci_set_option_value(str_sp);
				memset(str_sp,0,64);
			}
			else
			{
				sprintf(str_sp,"wireless.radio0.htmode=HT40+");
				uci_set_option_value(str_sp);
				memset(str_sp,0,64);
			}

		}
#endif
		system("uci commit");
		system("control_dns.sh >/dev/null 2>&1 &");
		system("save_wifi &");
		printf("scan success\n");
	}
	else{
		system("control_dns.sh >/dev/null 2>&1 &");
		printf("scan fail\n");
	}
	snifferFlag = SCAN_FINISH_PACKAGE;
	return;
}


static void exit_smartlink(int sig) 
{   
	printf("exit_smartlink\n");
	stop_smartlink();
	system("mlanutl mlan0 netmon 0");
	system("iwpriv uap0 start");
	system("iwpriv uap0 bssstart");
	system("control_dns.sh >/dev/null 2>&1 &");
	system("echo none > /sys/class/leds/longsys\:green\:led/trigger");
	system("echo netdev > /sys/class/leds/longsys\:blue\:led/trigger");
	system("echo uap0 > /sys/class/leds/longsys\:blue\:led/device_name");
	system("echo link tx rx > /sys/class/leds/longsys\:blue\:led/mode");
    exit(1);
}


static void signal_handle(void)
{
    signal(SIGPIPE, &exit_smartlink);//pipe broken
    signal(SIGINT,  &exit_smartlink);//ctrl+c
    signal(SIGTERM, &exit_smartlink);//kill
    signal(SIGSEGV, &exit_smartlink);//segmentfault
    signal(SIGBUS,  &exit_smartlink);//bus error/**/
	//signal(EXITACTIVE,&exit_smartlink);
}


int main(int argc, char* argv[])
{
	int 			ret= -1;
	int 			channel;
	int 			readsize;
	void*			join_str = NULL;
	int 			scan_time = 0;
	int 			cannel_time = 0;

	signal_handle();

	system("echo timer > /sys/class/leds/longsys\:blue\:led/trigger ");
	system("echo 200 > /sys/class/leds/longsys\:blue\:led/delay_on ");
	system("echo 200 > /sys/class/leds/longsys\:blue\:led/delay_off ");
	
	memset(g_guid,0,sizeof(g_guid));
	if(!readBufferFromFile("/etc/qq/Guid_file.txt", (unsigned char*)g_guid, sizeof(g_guid), &readsize)){
		printf("[error]get guid from file failed...\n");
		ret = -1;
		goto EXIT2;
	}
	else{
		 printf("g_guid=%s len=%d\n",g_guid,strlen(g_guid));
	}

	printf(">>>>>>>>>>>>>>start monitor mode!!!\n\n");
	system("iwpriv uap0 stop");
	system("killall wpa_supplicant");
	system("mlanutl mlan0 netmon 1 7 11 1");

	ret = start_smartlink(&set_channel, "mlan0", &on_smartlink_notify, g_guid, WCT_80211RADIOTAP_STANDARD, NULL, NULL, NULL);
	if(ret != QLERROR_INIT_SUCCESS){
		printf("start smartlink fail, ret: %d\n", ret);
		goto EXIT1;
	}

	snifferFlag = WAIT_DATE_PACKAGE;
	for(scan_time = 0;scan_time < SCAN_TIMES && is_smartlink_running() && snifferFlag == WAIT_DATE_PACKAGE; scan_time++){
		//printf("is_smartlink_running\n");
		usleep(500000);
	}

	printf("stop_smartlink\n");
	stop_smartlink();
	
	if(snifferFlag != SCAN_FINISH_PACKAGE){
		printf("=====================================\n");
		printf("=================fail================\n");
		printf("=====================================\n");
		goto EXIT1;
	}
	else{
		printf("=====================================\n");
		printf("===============success===============\n");
		printf("=====================================\n");
		system("/etc/init.d/qq restart");
		goto EXIT2;
	}
	
EXIT1:
	system("mlanutl mlan0 netmon 0");
	system("control_dns.sh >/dev/null 2>&1 &");
EXIT2:
	system("iwpriv uap0 start");
	system("iwpriv uap0 bssstart");
	system("echo none > /sys/class/leds/longsys\:green\:led/trigger");
	system("echo netdev > /sys/class/leds/longsys\:blue\:led/trigger");
	system("echo uap0 > /sys/class/leds/longsys\:blue\:led/device_name");
	system("echo link tx rx > /sys/class/leds/longsys\:blue\:led/mode");
	return ret;
}

