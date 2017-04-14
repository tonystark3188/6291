#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include "uci_for_cgi.h"
#include "msg.h"
#define NAME "name"
#define SID "sid"


void main()
{
		char ret_buf[2048];
		char *name=NULL;
		char sid[SID_LEN]="\0";
		char fw_sid[SID_LEN]="\0";

		char *web_str=NULL;
		char tmp_name[256]="\0";

		char uci_option_str[256]="\0";
		ctx=uci_alloc_context();
		
		strcpy(uci_option_str,"system.@system[0].sid");			//name
		uci_get_option_value(uci_option_str,fw_sid);
		memset(uci_option_str,'\0',sizeof(uci_option_str));

		
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
		
		processString(web_str,NAME,tmp_name);		

		name=urlDecode(tmp_name);
		
		p_debug("set tmp_name=%s,len=%d",tmp_name,strlen(tmp_name));
		p_debug("set name=%s,len=%d",name,strlen(name));
		if(!strcmp(sid,fw_sid)){//是管理员
			sprintf(uci_option_str,"system.@system[0].name=%s",name);
			uci_set_option_value(uci_option_str);
			memset(uci_option_str,'\0',sizeof(uci_option_str));
			system("uci commit");
			sprintf(ret_buf,"%s","{\"status\":1,\"data\":{},\"errorCode\":0,\"errorMessage\":\"success\"}");
		}else{
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"Not Admin\"}");
		} 

		fprintf(stdout,ret_buf);
		fflush(stdout);
		free(web_str);
		free(name);
		
		uci_free_context(ctx);
		return ;
}

