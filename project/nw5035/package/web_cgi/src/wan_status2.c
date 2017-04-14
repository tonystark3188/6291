#include "msg.h"
#include "socket_uart.h" 
#include <linux/wireless.h>


int internet(){
	int ret=0;
	return ret;
}

int get_mode(){
	int ret=0;// 1 AP 2 P2P-GO
	return ret;
}

#define PATH "/tmp/upgrade_info"

int getUpgradeInfo(){
	FILE *fd=NULL;
	int ret=0;
	char buf[8]="\0";
	if((fd=fopen(PATH,"r"))==NULL)
	{
		//p_debug("open file error[errno = %d]",errno);
		return 0;
	}
	fread(buf,1,1,fd);
	p_debug("read buf===%s",buf);
	ret=atoi(buf);
	p_debug("ret=%d",ret);
	fclose(fd);
	return ret;
}

#if WIRELESS_EXT <= 11
#ifndef SIOCDEVPRIVATE
#define SIOCDEVPRIVATE      0x8BE0
#endif
#define SIOCIWFIRSTPRIV      SIOCDEVPRIVATE
#endif
#define RTPRIV_IOCTL_SHOW_CONNSTATUS	(SIOCIWFIRSTPRIV + 0x1B)
#define RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT	(SIOCIWFIRSTPRIV + 0x1F)

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


int getOnline()
{
		int    socket_id;
		struct	 iwreq wrq;
		char data[4096]="\0";
		int    ret;
		RT_802_11_MAC_TABLE    *mp;
		int count = 0;
		char mode[64] = "\0";
		char uci_option_str[64] = "\0";
		FILE *read_fp; 
		int chars_read = 0;
		char buffer[64] = "\0";
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"system.@system[0].wifimode"); 
		uci_get_option_value(uci_option_str,mode);
		if(strcmp(mode,"ap") == 0)
		{	
		
			socket_id = socket(AF_INET, SOCK_DGRAM, 0);
			if(socket_id < 0)
			{
				//sprintf(ret_buf,"{\"status\":0,\"errorCode\":3,\"errorMessage\":\"socket error!\",\"data\":{}}");
				//sprintf(ret_buf,"<Return status=\"false\">socket error!</Return>");
				return -1;
			}
			memset(data, 0x00, 4096);
			strcpy(wrq.ifr_name, "ra0");
			wrq.u.data.length = 4096;
			wrq.u.data.pointer = data;
			wrq.u.data.flags = 0;
			ret = ioctl(socket_id, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &wrq);
			if(ret != 0)
			{
				//sprintf(ret_buf,"{\"status\":0,\"errorCode\":3,\"errorMessage\":\"ioctl error!\",\"data\":{}}");
			//	sprintf(retstr,"<Return status=\"false\">ioctl error!</Return>");
				close(socket_id);
				return -1;
			}
			mp = (RT_802_11_MAC_TABLE *)wrq.u.data.pointer;
			count = mp->Num;
			close(socket_id);
		}
		else if (strcmp(mode,"sta") == 0)
		{
			read_fp=popen("wpa_cli all_sta | grep dot11RSNAStatsSTAAddress | wc -l","r");
			if(read_fp != NULL)
			{
				chars_read = fread(buffer, sizeof(char), 64-1, read_fp); 
				if (chars_read > 0)
				{
					count = atoi(buffer);
					pclose(read_fp);
					
				}else
				{
					pclose(read_fp);
					//sprintf(ret_buf,"{\"status\":0,\"errorCode\":3,\"errorMessage\":\"popen error!\",\"data\":{}}");
	//				sprintf(retstr,"<Return status=\"false\">popen error!</Return>");
					return -1;
				}
			}
			else
			{
				//sprintf(ret_buf,"{\"status\":0,\"errorCode\":3,\"errorMessage\":\"popen error!\",\"data\":{}}");
	//			sprintf(retstr,"<Return status=\"false\">popen error!</Return>");
				return -1;
			}
		}
		else
		{
			//sprintf(ret_buf,"{\"status\":0,\"errorCode\":3,\"errorMessage\":\"wifi mode error!\",\"data\":{}}");
			//sprintf(retstr,"<Return status=\"false\">wifi mode error!</Return>");
			return -1;
		}
		//sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"count\":%d}}",count);
		//sprintf(retstr,"<Users count=\"%d\"></Users>",count);
		//p_debug(ret_buf);
		return count;
}


int getPower()
{
	char bat[8]={0};
	system("letv_gpio 1 & >/dev/null");
	get_conf_str(bat,"power");
//	p_debug("bat=%s\n",bat);
	return atoi(bat);
	

/*


	int percent=0;
	unsigned char stat=0;
	unsigned char tmpStatus=0;
	int bat;
	unsigned char chargflag = 0;
	int count = 0;
	int fh=NULL;
	int ret=0;
	socket_uart_cmd_t g_uart_cmd;
	bzero(&g_uart_cmd, sizeof(socket_uart_cmd_t));

	p_debug("ret===sssssssss");
	g_uart_cmd.mode = UART_R;
	g_uart_cmd.regaddr =SOCKET_UART_GET_POWER_PERCENT;

	ret=SocketUartClientStart(&g_uart_cmd);
	p_debug("ret===%d",ret);

	if(ret==0)
		bat = g_uart_cmd.data & 0xFF;
	else{
		//error_num++;
		//sprintf(retstr,"{\"status\":0,\"errorCode\":3,\"errorMessage\":\"get power error\",\"data\":{}}");
		return 0;
	}
	//p_debug("bat=%d",bat);
	
	if(bat<=5)percent=0;
	else if(bat>5&&bat<=10)percent=10;
	else if(bat>10&&bat<=25)percent=25;
	else if(bat>25&&bat<=50)percent=50;
	else if(bat>50&&bat<=75)percent=75;
	else if(bat>75&&bat<=100)percent=100;
	
	//sprintf(retstr,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"power\":%d}}",percent);
	return bat;
*/	
}


int get_conf_str(char *dest,char *var)
{
	FILE *fp=fopen("/tmp/state/status","r");
	if(NULL == fp)
	{
		//printf("open /etc/config/nrender.conf failed \n");
		return 0;
	}
	char tmp[128];
	char *ret_str;
	bzero(tmp,128);
	while(fgets(tmp,128,fp)!=NULL)
	{
		if('\n'==tmp[strlen(tmp)-1])
		{
			tmp[strlen(tmp)-1]=0;
		}
		//printf("get string from /etc/config/nrender.conf:%s\n",tmp);
		if(!strncmp(var,tmp,strlen(var)))
		{
			ret_str = malloc(strlen(tmp)-strlen(var));
			if(!ret_str)
			{
				fclose(fp);
				return 0;
			}
			bzero(ret_str,strlen(tmp)-strlen(var));
			strcpy(ret_str,tmp+strlen(var)+1);
			
			//printf("ret string:%s\n",ret_str);
			fclose(fp);
			strcpy(dest,ret_str);
			free(ret_str);
			return 0;
		}
		
	}
	fclose(fp);
	return 0;
}

void main()
{
		char ret_buf[RET_BUF_LEN]={0};
		char code[CODE_LEN]="\0";
		char fw_code[CODE_LEN]="\0";		
		char sid[SID_LEN]="\0";
		char fw_sid[SID_LEN]="\0";
		char type[32]="\0";

		char tmp_buf[256]="\0";
		char tmp_vid[32]="\0";
		char speed[16]="0";			
		char mode[8]="0";		
		char ssid[33]="0";
		char net[8]="true";
		//char updateState[8]="\0";
		int  updateState=0;
		char vid[VID_LEN_33]="0";		
		char percent[8]="0";		
		char unFinishedVer[16]="1";
		char completedVer[16]="2";
		char isCanLinkNet[8]="0";
		char progress[32]="0";
		char totalSize[32]="0";

		int i,j,k;
		
		char *web_str=NULL;
		int ret=0;
		int upgrade_flag=0;
		int power=0;
		int count=0;
		char uci_option_str[UCI_BUF_LEN]="\0";
		printf("Content-type:text/plain\r\n\r\n");
		if((web_str=GetStringFromWeb())==NULL)
		{
			fprintf(stdout,"can't get string from web\n");
			exit(1);
		}		
		processString(web_str,SID,sid);
		processString(web_str,"code",code);		
		//p_debug(sid);
		//p_debug(code);

		updateState=getUpgradeInfo();
		if(updateState==0){//not upgrading
			ctx=uci_alloc_context();
			#if 1			
			strcpy(uci_option_str,"system.@system[0].sid");			//name
			uci_get_option_value(uci_option_str,fw_sid);
			memset(uci_option_str,'\0',UCI_BUF_LEN);
			
			strcpy(uci_option_str,"system.@system[0].code");			//name
			uci_get_option_value(uci_option_str,fw_code);
			memset(uci_option_str,'\0',UCI_BUF_LEN);

			strcpy(uci_option_str,"system.@system[0].mode");			//mode
			uci_get_option_value(uci_option_str,mode);
			memset(uci_option_str,'\0',UCI_BUF_LEN);
			
            if(strcmp(mode,"0")==0) {//ap mode
                strcpy(uci_option_str,"wireless.@wifi-iface[1].ssid");			//name
                uci_get_option_value(uci_option_str,ssid);
                memset(uci_option_str,'\0',UCI_BUF_LEN);
            }else {// p2p mode
                strcpy(uci_option_str,"wireless.@wifi-iface[0].p2p_go_ssid");			//name
                uci_get_option_value(uci_option_str,ssid);
                memset(uci_option_str,'\0',UCI_BUF_LEN);
            }
            #endif

			get_conf_str(vid,"vid");
			get_conf_str(speed,"speed");
			get_conf_str(net,"net");
			get_conf_str(progress,"progress");
			get_conf_str(totalSize,"totalSize");
			get_conf_str(percent,"percent");
			get_conf_str(unFinishedVer,"unfinishedVer");
			get_conf_str(completedVer,"completedVer");
			get_conf_str(isCanLinkNet,"isCanLinkNet");
			if(!strcmp(speed,"")) speed[0]='0';
			if(!strcmp(vid,"")) vid[0]='0';
			if(!strcmp(net,"")) net[0]='0';
			if(strcmp(net,"true")) memset(ssid,0,strlen(ssid));
			if(!strcmp(totalSize,"")) totalSize[0]='0';
			if(!strcmp(percent,"")) percent[0]='0';
			if(!strcmp(unFinishedVer,"")) unFinishedVer[0]='0';
			if(!strcmp(completedVer,"")) completedVer[0]='0';
			if(!strcmp(isCanLinkNet,"")) isCanLinkNet[0]='0';
			p_debug(sid);
			p_debug(fw_sid);
			p_debug(fw_code);
			p_debug(code);

			if((!strcmp(sid,fw_sid))&&(strcmp(code,fw_code)))//管理员更新CODE
			{	
				memset(uci_option_str,'\0',UCI_BUF_LEN);
				sprintf(uci_option_str,"system.@system[0].code=%s",code);
				uci_set_option_value(uci_option_str);
				p_debug(uci_option_str);
				system("uci commit");
			}

			#if 0
			uci_add_delta_path(ctx,"/tmp/state");
			uci_set_confdir(ctx,"/tmp/state");

			
			strcpy(uci_option_str,"status.@downloading[0].vid");			//name
			uci_get_option_value(uci_option_str,vid);
			memset(uci_option_str,'\0',64);
			p_debug("vid=%s",vid);

			strcpy(uci_option_str,"status.@downloading[0].speed");			//name
			uci_get_option_value(uci_option_str,speed);
			p_debug("speed=%s",speed);
			memset(uci_option_str,'\0',64);
			
			strcpy(uci_option_str,"status.@downloading[0].net");			//name
			uci_get_option_value(uci_option_str,net);
			memset(uci_option_str,'\0',64);


			strcpy(uci_option_str,"status.@downloading[0].progress");			//name
			uci_get_option_value(uci_option_str,progress);
			memset(uci_option_str,'\0',64);
			//p_debug("progress=%s",progress);
			strcpy(uci_option_str,"status.@downloading[0].totalSize");			//name
			uci_get_option_value(uci_option_str,totalSize);
			memset(uci_option_str,'\0',64);		
			//p_debug("totalSize=%s",totalSize);
			/*
			strcpy(uci_option_str,"system.@system[0].updateState");			//name
			uci_get_option_value(uci_option_str,updateState);
			memset(uci_option_str,'\0',64);
			*/
			//updateState=getUpgradeInfo();
			
			strcpy(uci_option_str,"status.@downloading[0].vid");			//name
			uci_get_option_value(uci_option_str,vid);
			memset(uci_option_str,'\0',64);

			strcpy(uci_option_str,"status.@downloading[0].percent");			//name
			uci_get_option_value(uci_option_str,percent);
			memset(uci_option_str,'\0',64);


			strcpy(uci_option_str,"status.@downloading[0].unfinishedVer");			//name
			uci_get_option_value(uci_option_str,unFinishedVer);
			memset(uci_option_str,'\0',64);

			strcpy(uci_option_str,"status.@downloading[0].finishedVer");			//name
			uci_get_option_value(uci_option_str,completedVer);
			memset(uci_option_str,'\0',64);
			#endif


		}
		else 
			upgrade_flag=1;
			
		if((web_str=GetStringFromWeb())==NULL)
		{
			fprintf(stdout,"can't get string from web\n");
			exit(1);
		}
		processString(web_str,SID,sid);		
		
		processString(web_str,CODE,code);		
		free(web_str);

		if(strcmp(mode,"1")==0) {
			strcpy(net,"false");
			//memset(ssid,0,strlen(ssid));
		}

		if(updateState)// 升级时的心跳包单独处理。
		{
			power=50;
			count=1;
			sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"deviceState\":{\"mode\":\"%s\",\"ssid\":\"%s\",\"hasInternet\":%s,\"isCanLinkNet\":%s,\"power\":%d,\"count\":%d},\"otaState\":{\"updateState\":%d},\"taskState\":{\"taskPro\":{\"vid\":\"%s\",\"pr\":%s,\"progress\":%s,\"totalSize\":%s,\"speed\":%s},\"taskVer\":{\"unFinishedVer\":\"%s\",\"completedVer\":\"%s\"}}}}",\
						mode,ssid,net,isCanLinkNet,power,count,updateState,vid,percent,progress,totalSize,speed,unFinishedVer,completedVer);		
			p_debug("buf=====%s,len=%d",ret_buf,strlen(ret_buf));

			fprintf(stdout,ret_buf);
			fflush(stdout);fflush(stdout);
			if(ctx)uci_free_context(ctx);
			return ;						
		}
		if(!strcmp(sid,fw_sid)){
			power=getPower();
			p_debug("p=%d,%d",power,count);
			count=getOnline();

			sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"deviceState\":{\"mode\":\"%s\",\"ssid\":\"%s\",\"hasInternet\":%s,\"isCanLinkNet\":%s,\"power\":%d,\"count\":%d},\"otaState\":{\"updateState\":%d},\"taskState\":{\"taskPro\":{\"vid\":\"%s\",\"pr\":%s,\"progress\":%s,\"totalSize\":%s,\"speed\":%s},\"taskVer\":{\"unFinishedVer\":\"%s\",\"completedVer\":\"%s\"}}}}",\
				mode,ssid,net,isCanLinkNet,power,count,updateState,vid,percent,progress,totalSize,speed,unFinishedVer,completedVer);		
			p_debug("buf=====%s,len=%d",ret_buf,strlen(ret_buf));

		}

		else if(!strcmp(code,fw_code)){//不是管理员
			power=getPower();
			count=getOnline();
			
			sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"deviceState\":{\"mode\":\"%s\",\"ssid\":\"%s\",\"hasInternet\":%s,\"isCanLinkNet\":%s,\"power\":%d,\"count\":%d},\"otaState\":{\"updateState\":%d},\"taskState\":{\"taskPro\":{\"vid\":\"%s\",\"pr\":%s,\"progress\":%s,\"totalSize\":%s,\"speed\":%s},\"taskVer\":{\"unFinishedVer\":\"%s\",\"completedVer\":\"%s\"}}}}",mode,ssid,net,isCanLinkNet,power,count,updateState,vid,percent,progress,totalSize,speed,unFinishedVer,completedVer);
			//sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"taskState\":{\"taskVer\":{\"unFinishedVer\":\"%s\",\"completedVer\":\"%s\"}}}}",\
				unFinishedVer,completedVer);
		
			if(updateState==0)p_debug("buf=====%s,len=%d",ret_buf,strlen(ret_buf));
			p_debug("buf=====%s,len=%d",ret_buf,strlen(ret_buf));
			/*ret = notify_server();
			if(ret <= 0){//通讯错误
				sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":13,\"errorMessage\":\"Communication Error with dm_letv\"}");
			}else//接收到消息
				sprintf(ret_buf,"%s",p_client_info.recv_buf);
			*/
		}else{
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"security error\"}");
		} 

		fprintf(stdout,ret_buf);
		fflush(stdout);fflush(stdout);
		
		if(ctx)uci_free_context(ctx);
		return ;
}


