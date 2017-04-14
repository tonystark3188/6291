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

		char fw_sid[SID_LEN]="\0";
		char app_sid[SID_LEN]="\0";
		char *web_str=NULL;

		char ssid[33]="\0";
		char key[33]="\0";

		char uci_option_str[UCI_BUF_LEN]="\0";
		ctx=uci_alloc_context();
		
		printf("Content-type:text/plain\r\n\r\n");

		
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
		p_debug("sid=%s",app_sid);

		processString(web_str,"ssid",ssid);
		p_debug("ssid=%s",ssid);

		processString(web_str,"key",key);
		
		if(!strcmp(fw_sid,app_sid)&&(strcmp(ssid,""))&&(strcmp(key,"")))//是管理员
		{
			sprintf(uci_option_str,"wireless.@wifi-iface[0].ssid=%s",ssid);			//name
			uci_set_option_value(uci_option_str);
			memset(uci_option_str,'\0',UCI_BUF_LEN);
			
			sprintf(uci_option_str,"wireless.@wifi-iface[0].key=%s",key);			//name
			uci_set_option_value(uci_option_str);
			memset(uci_option_str,'\0',UCI_BUF_LEN);

			sprintf(ret_buf,"%s","{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{}}");
		
			system("uci commit");
			printf("%s",ret_buf);
			system("wifi up > /dev/null");
			uci_free_context(ctx);
			return ;

		}else {//访客
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"Not Admin\"}");
			printf("%s",ret_buf);
			fflush(stdout);
			uci_free_context(ctx);
			return ;
		}
}

