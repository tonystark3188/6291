#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include "uci_for_cgi.h"
#define SID "sid"

void nameclear()
{
	system("uci set system.@system[0].sid=\"\"");
	system("uci commit");
}

void main()
{
		char ret_buf[2048];
		char fw_sid[SID_LEN]="\0";
		char app_sid[SID_LEN]="\0";
		char *web_str=NULL;
		char uci_option_str[UCI_BUF_LEN]="\0";



		ctx=uci_alloc_context();
		printf("Content-type:text/plain\r\n\r\n");

		strcpy(uci_option_str,"system.@system[0].sid");			//name
		uci_get_option_value(uci_option_str,fw_sid);
		memset(uci_option_str,'\0',UCI_BUF_LEN);

		if((web_str=GetStringFromWeb())==NULL)
		{
			fprintf(stdout,"can't get string from web\n");
			exit(1);
		}

		processString(web_str,SID,app_sid);
		if(!strcmp(fw_sid,app_sid))//是管理员
		{
			nameclear();
			sprintf(ret_buf,"%s","{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{}}");
		}else {
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"You are Not the admin.\"}");
		}		
		
		printf("%s",ret_buf);
		fflush(stdout);
		uci_free_context(ctx);
		return ;
}

