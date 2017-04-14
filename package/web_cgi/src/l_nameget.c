#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include "uci_for_cgi.h"


void main()
{
		char ret_buf[2048];
		char name[32]="\0";
		char app_sid[SID_LEN]="\0";
		char *web_str=NULL;


		char uci_option_str[64]="\0";
		ctx=uci_alloc_context();

		strcpy(uci_option_str,"system.@system[0].name");			//name
		uci_get_option_value(uci_option_str,name);
		memset(uci_option_str,'\0',64);

		printf("Content-type:text/plain\r\n\r\n");


		sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"name\":\"%s\"}}",name);
			
		
		printf("%s",ret_buf);
		fflush(stdout);
		uci_free_context(ctx);
		return ;
}

