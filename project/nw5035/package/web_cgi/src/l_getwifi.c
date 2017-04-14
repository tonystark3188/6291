#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include "msg.h"


void main()
{
		char ret_buf[2048];
		char name[33]="\0";
		char key[33]="\0";
		char fw_sid[SID_LEN]="\0";
		char app_sid[SID_LEN]="\0";
		char *web_str=NULL;


		char uci_option_str[UCI_BUF_LEN]="\0";
		printf("Content-type:text/plain\r\n\r\n");

		ctx=uci_alloc_context();
		strcpy(uci_option_str,"system.@system[0].sid");			//name
		uci_get_option_value(uci_option_str,fw_sid);
		memset(uci_option_str,'\0',UCI_BUF_LEN);
		if((web_str=GetStringFromWeb())==NULL)
		{
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":4,\"errorMessage\":\"Can not get any parameters.\"}");
			printf("%s",ret_buf);
			fflush(stdout);
			uci_free_context(ctx);
			return ;
		}
		processString(web_str,SID,app_sid);
		
		if(!strcmp(fw_sid,app_sid))//是管理员
		{
			strcpy(uci_option_str,"wireless.@wifi-iface[0].ssid");			//name
			uci_get_option_value(uci_option_str,name);
			memset(uci_option_str,'\0',UCI_BUF_LEN);
			
			strcpy(uci_option_str,"wireless.@wifi-iface[0].key");			//name
			uci_get_option_value(uci_option_str,key);
			memset(uci_option_str,'\0',UCI_BUF_LEN);
			sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"ssid\":\"%s\",\"key\":\"%s\",\"result\":\"0\"}}",name,key);

		}else {//访客
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"Not Admin\"}");
		}
		
		printf("%s",ret_buf);
		p_debug(ret_buf);
		fflush(stdout);
		uci_free_context(ctx);
		return ;
}

