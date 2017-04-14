#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include "msg.h"

#include "uci_for_cgi.h"
// /etc/config/system sid
#define SID "sid"
char tmp_mac[32];

int getMac(){
	FILE *read_fp = NULL;
	read_fp = popen("hexdump -s 4 -n 6 -C /dev/mtd3 | head -n 1 | sed 's/\ \ /:/g' | cut -d: -f 2 | sed 's/\ /:/g' | tr \"[a-z]\" \"[A-Z]\"", "r");
	if(read_fp != NULL)
	{
		memset(tmp_mac,0,32);
		fgets(tmp_mac, 18, read_fp);
		return 0;
	}
	else
		return -1;
	p_debug("tmp_mac=%s",tmp_mac);
}

void main()
{
		char ret_buf[2048];
		char fw_sid[SID_LEN]="\0";
		char app_sid[SID_LEN]="\0";
		char save_code[CODE_LEN]="\0";	
		char code[CODE_LEN]="\0";
		char *web_str=NULL;
		char ssid[33]={};
		char mac[32]={};
		char key[64]={};
		char led_status[8]={};
		char wifimode[8]={0};

		char uci_option_str[UCI_BUF_LEN]="\0";
		ctx=uci_alloc_context();

#if 0
		strcpy(uci_option_str,"system.@system[0].led_status");			//name
		uci_get_option_value(uci_option_str,led_status);
		memset(uci_option_str,'\0',64);
#endif
		strcpy(uci_option_str,"system.@system[0].sid");			//name
		uci_get_option_value(uci_option_str,fw_sid);
		memset(uci_option_str,'\0',UCI_BUF_LEN);

		strcpy(uci_option_str,"system.@system[0].wifimode");			//name
		uci_get_option_value(uci_option_str,wifimode);
		memset(uci_option_str,'\0',UCI_BUF_LEN);

		strcpy(uci_option_str,"system.@system[0].code");			//name
		uci_get_option_value(uci_option_str,save_code);
		memset(uci_option_str,'\0',UCI_BUF_LEN);
		if(!strcmp(wifimode,"sta")){
				strcpy(uci_option_str,"wireless.@wifi-iface[0].p2p_go_ssid");			//name
				uci_get_option_value(uci_option_str,ssid);
				memset(uci_option_str,'\0',UCI_BUF_LEN);

				strcpy(uci_option_str,"wireless.@wifi-iface[0].p2p_go_key");			//name
				uci_get_option_value(uci_option_str,key);
				memset(uci_option_str,'\0',UCI_BUF_LEN);
		}
		else {
				strcpy(uci_option_str,"wireless.@wifi-iface[0].ssid");			//name
				uci_get_option_value(uci_option_str,ssid);
				memset(uci_option_str,'\0',UCI_BUF_LEN);

				strcpy(uci_option_str,"wireless.@wifi-iface[0].key");			//name
				uci_get_option_value(uci_option_str,key);
				memset(uci_option_str,'\0',UCI_BUF_LEN);
			
		}


//		strcpy(uci_option_str,"wireless.@wifi-iface[0].mac");			//name
//		uci_get_option_value(uci_option_str,mac);
//		memset(uci_option_str,'\0',UCI_BUF_LEN);


		printf("Content-type:text/plain\r\n\r\n");

		if((web_str=GetStringFromWeb())==NULL)
		{
			fprintf(stdout,"can't get string from web\n");
			exit(1);
		}
		processString(web_str,SID,app_sid);
		processString(web_str,"code",code);		
		getMac();

		if(!strcmp(fw_sid,"")){//第一次调用login接口的为管理员，即sid为空时
			get_conf_str(led_status,"led_status");
			if(!strcmp(led_status,"3")){
				system("pwm_control 1  1 0;pwm_control 1 0 0 >/dev/null");// wifi led on
				updateSysVal("led_status","1");
			}
			sprintf(uci_option_str,"system.@system[0].sid=%s",app_sid);
			uci_set_option_value(uci_option_str);
			system("uci commit");
			memset(uci_option_str,'\0',UCI_BUF_LEN);//{SCHEME:"LEHE",VERSION:"1",MAC:"84:5D:D7:33:99:65",SSID:"letv_xwm",PASSWORD:"88888888"}
			sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"isAdmin\":1,\"info\":{\"SCHEME\":\"LEHE\",\"VERSION\":\"1\",\"MAC\":\"%s\",\"SSID\":\"%s\",\"password\":\"%s\"}}}",tmp_mac,ssid,key);
		}
		else if(!strcmp(fw_sid,app_sid))//是管理员
		{
			if(!strcmp(led_status,"3")){
				system("pwm_control 1  1 0;pwm_control 1 0 0 >/dev/null");// wifi led on
				updateSysVal("led_status","1");
			}
			if(strcmp(save_code,code))//如果是管理员，随机码，则更新
				{
					sprintf(uci_option_str,"system.@system[0].code=%s",code);
					uci_set_option_value(uci_option_str);
					memset(uci_option_str,'\0',UCI_BUF_LEN);
					system("uci commit");
					
			}
			sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"isAdmin\":1,\"info\":{\"SCHEME\":\"LEHE\",\"VERSION\":\"1\",\"MAC\":\"%s\",\"SSID\":\"%s\",\"PASSWORD\":\"%s\"}}}",tmp_mac,ssid,key);			
		}else if(!strcmp(save_code,code)){//有效访客
			sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"isAdmin\":0,\"info\":{}}}");
			
		}else{//无效访客
			sprintf(ret_buf,"{\"status\":0,\"errorCode\":1,\"errorMessage\":\"security error\",\"data\":{}}");
		}
		p_debug(ret_buf);
		printf("%s",ret_buf);
		fflush(stdout);
		free(web_str);
		uci_free_context(ctx);
		return ;
}

