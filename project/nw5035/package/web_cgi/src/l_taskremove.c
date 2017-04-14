#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include "uci_for_cgi.h"

#define CODE "code"
#define SID "sid"

void main()
{
		char ret_buf[2048];
		char code[32]="\0";
		char sid[SID_LEN]="\0";
		char fw_sid[SID_LEN]="\0";
		char vid[32]="\0";

		char *web_str=NULL;


		char uci_option_str[UCI_BUF_LEN]="\0";
		ctx=uci_alloc_context();
		
		strcpy(uci_option_str,"system.@system[0].sid");			//name
		uci_get_option_value(uci_option_str,fw_sid);
		memset(uci_option_str,'\0',UCI_BUF_LEN);

		
		printf("Content-type:text/plain\r\n\r\n");

		if((web_str=GetStringFromWeb())==NULL)
		{
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":4,\"errorMessage\":\"Can not get any parameters.\"}");
			printf("%s",ret_buf);
			fflush(stdout);
			uci_free_context(ctx);
			return ;

		}
		processString(web_str,SID,sid);		
		
		processString(web_str,CODE,code);		
		
		processString(web_str,"vid",vid);	

		if(!strcmp(sid,fw_sid)){//是管理员
			//remove	
			sprintf(ret_buf,"%s","{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{}}");
		}else{
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"Not Admin\"}");
		} 

		printf("%s",ret_buf);
		fflush(stdout);
		free(web_str);
		uci_free_context(ctx);
		return ;
}

