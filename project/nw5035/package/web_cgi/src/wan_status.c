
#include "msg.h"
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
			
			strcpy(uci_option_str,"wireless.@wifi-iface[1].ssid");			//name
			uci_get_option_value(uci_option_str,ssid);
			memset(uci_option_str,'\0',UCI_BUF_LEN);
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
			if(ctx)uci_free_context(ctx);


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
			memset(ssid,0,strlen(ssid));
		}
		
		if(updateState)// 升级时的心跳包单独处理。
		{
			sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"deviceState\":{\"mode\":\"%s\",\"ssid\":\"%s\",\"hasInternet\":%s,\"isCanLinkNet\":%s},\"otaState\":{\"updateState\":%d},\"taskState\":{\"taskPro\":{\"vid\":\"%s\",\"pr\":%s,\"progress\":%s,\"totalSize\":%s,\"speed\":%s},\"taskVer\":{\"unFinishedVer\":\"%s\",\"completedVer\":\"%s\"}}}}",\
						mode,ssid,net,isCanLinkNet,updateState,vid,percent,progress,totalSize,speed,unFinishedVer,completedVer);		
			p_debug("buf=====%s,len=%d",ret_buf,strlen(ret_buf));

			fprintf(stdout,ret_buf);
			fflush(stdout);fflush(stdout);
			return ;						
		}
		if(!strcmp(sid,fw_sid)){
			sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"deviceState\":{\"mode\":\"%s\",\"ssid\":\"%s\",\"hasInternet\":%s,\"isCanLinkNet\":%s},\"otaState\":{\"updateState\":%d},\"taskState\":{\"taskPro\":{\"vid\":\"%s\",\"pr\":%s,\"progress\":%s,\"totalSize\":%s,\"speed\":%s},\"taskVer\":{\"unFinishedVer\":\"%s\",\"completedVer\":\"%s\"}}}}",\
				mode,ssid,net,isCanLinkNet,updateState,vid,percent,progress,totalSize,speed,unFinishedVer,completedVer);		
			p_debug("buf=====%s,len=%d",ret_buf,strlen(ret_buf));

		}

		else if(!strcmp(code,fw_code)){//不是管理员
			
			//sprintf(ret_buf,"{\"status\":1,\"data\":{\"deviceState\":{\"mode\":\"%s\",\"ssid\":\"%s\",\"hasInternet\":%s,\"isCanLinkNet\":%s},\"otaState\":{\"updateState\":%d},\"taskState\":{\"taskPro\":{\"vid\":\"%s\",\"pr\":%s,\"progress\":%s,\"totalSize\":%s,\"speed\":%s}},\"taskVer\":{\"unFinishedVer\":\"%s\",\"completedVer\":\"%s\"}}}",
			sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"taskState\":{\"taskVer\":{\"unFinishedVer\":\"%s\",\"completedVer\":\"%s\"}}}}",\
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
		
		
		return ;
}


