
#include "msg.h"
#include "uci_for_cgi.h"

static int my_system(const char * cmd) 
{ 
	FILE * fp; 
	int res; char buf[1024]; 
	if (cmd == NULL) 
	{ 
		printf("my_system cmd is NULL!\n");
		return -1;
	}

	if ((fp = popen(cmd, "r") ) == NULL) 
	{ 
		perror("popen");
		printf("popen error: %s/n", strerror(errno)); return -1; 
	} 
	else
	{
		while(fgets(buf, sizeof(buf), fp)) 
		{ 
			printf("%s", buf); 
		} 
		if ( (res = pclose(fp)) == -1) 
		{ 
			printf("close popen file pointer fp error!\n"); return res;
		} 
		else if (res == 0) 
		{
			return res;
		} 
		else 
		{ 
			printf("popen res is :%d\n", res); return res; 
		} 
	}
	return 0;
}


int setWifiSwitch(char *wifiMode){
	//char *wifiMode=NULL;//0 for AP+STA; 1 for P2P-GO
	char *pxml=NULL;
	char *ntmp=NULL;
	int flag=0;
	int ret;
	/**/
	char ret_buf[2048];

//	printf("Content-type:text/plain\r\n\r\n");

	p_debug("wifiMode=%s\n",wifiMode);
	if(!strcmp(wifiMode,"0"))//ap mode
	{
		flag=1;
		sprintf(ret_buf,"%s","{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{}}");
		printf("%s",ret_buf);
		fflush(stdout);		
	}else if(!strcmp(wifiMode,"1"))//p2p mode
	{
		flag=2;//system("wifi_switch2.sh sta");
		sprintf(ret_buf,"%s","{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{}}");
		printf("%s",ret_buf);
		fflush(stdout);
	}
	else {
		//fprintf(stdout,"<%s><Return status=\"false\" delay=\"10\">Please check your parameter</Return></%s>\r\n",SETSYSSTR,SETSYSSTR);
		sprintf(ret_buf,"%s","{\"status\":0,\"errorCode\":4,\"errorMessage\":\"parameter error\",\"data\":{}}");
		printf("%s",ret_buf);
		fflush(stdout);
	}

	if(flag==1)	
		ret = my_system("/sbin/sw.sh ap >/dev/null");
	else if(flag==2)	
		ret = my_system("/sbin/sw.sh sta >/dev/null");
	p_debug("wifi_switch return value:%d\n",ret);
	return 0;
}
char ret_buf[256];
int main(){
		char wifimode[8]={};

		
		char code[CODE_LEN]="\0";
		char sid[SID_LEN]="\0";
		char fw_sid[SID_LEN]="\0";
		char *web_str=NULL;
		char uci_option_str[UCI_BUF_LEN]="\0";
		ctx=uci_alloc_context();

		strcpy(uci_option_str,"system.@system[0].sid");			//name
		uci_get_option_value(uci_option_str,fw_sid);
		memset(uci_option_str,'\0',UCI_BUF_LEN);

		
		printf("Content-type:text/plain\r\n\r\n");

		if((web_str=GetStringFromWeb())==NULL)
		{
			fprintf(stdout,"can't get string from web\n");
			exit(1);
		}
		processString(web_str,SID,sid);		
		//processString(web_str,CODE,code);		
		processString(web_str,CODE,code);	

		processString(web_str,"mode",wifimode);	
		
		if(!strcmp(sid,fw_sid)){
			setWifiSwitch(wifimode);
		}else {
			setWifiSwitch("3");
		}

}

