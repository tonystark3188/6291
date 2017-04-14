/*
 * =============================================================================
 *
 *       Filename:  server.c
 *
 *    Description:  hidisk server module.
 *
 *        Version:  1.0
 *        Created:  2015/3/19 10:54
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 515296288jf@163.com
 *   Organization:  
 *
 * =============================================================================
 */

#include "msg.h"

int _handle_client_json_req(ClientTheadInfo *client_info)
{
 //   return api_process(client_info);
}


void main()
{
		char ret_buf[RET_BUF_LEN];
		char code[CODE_LEN]="\0";
		char fw_code[CODE_LEN]="\0";
		char sid[SID_LEN]="\0";
		char fw_sid[SID_LEN]="\0";
		char type[32]="\0";
		char pid[1024]="\0";
		char tmp_buf[256]="\0";
		char tmp_vid[32]="\0";
		int i,j,k;
		char *web_str=NULL;
		int ret=0;
		char pwd_type[8]={0};
		char right[32]={0};

		char uci_option_str[UCI_BUF_LEN]="\0";
		ctx=uci_alloc_context();
		
		strcpy(uci_option_str,"system.@system[0].sid");			//name
		uci_get_option_value(uci_option_str,fw_sid);
		memset(uci_option_str,'\0',UCI_BUF_LEN);

		strcpy(uci_option_str,"system.@system[0].code");			//name
		uci_get_option_value(uci_option_str,fw_code);
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
		
		//processString(web_str,PID,pid);	

		processString(web_str,"right",right); 


		if(!strcmp(sid,fw_sid)&&(!strcmp(code,fw_code))){//是管理员
		//if(1){//是管理员
			//remove

			if(!strcmp(right,"ro"))
				{
					strcpy(uci_option_str,"samba.@sambashare[0].read_only=yes");			//name
					uci_set_option_value(uci_option_str);
					memset(uci_option_str,'\0',UCI_BUF_LEN);
					system("uci commit");
					system("/etc/init.d/samba restart >/dev/null");	
					sprintf(ret_buf,"%s","{\"status\":1,\"data\":{},\"errorCode\":0,\"errorMessage\":\"success\"}");
				}
			else if(!strcmp(right,"rw"))
				{
				strcpy(uci_option_str,"samba.@sambashare[0].read_only=no");			//name
				uci_set_option_value(uci_option_str);
				memset(uci_option_str,'\0',UCI_BUF_LEN);
				system("uci commit");
				system("/etc/init.d/samba restart >/dev/null");
				sprintf(ret_buf,"%s","{\"status\":1,\"data\":{},\"errorCode\":0,\"errorMessage\":\"success\"}");
			}
			else 
				sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":3,\"errorMessage\":\"parameter error!\"}");
					
		}else{
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"Not Admin\"}");
		} 
exit:
		printf("%s",ret_buf);
		fflush(stdout);
		free(web_str);
		uci_free_context(ctx);
		return ;
}

