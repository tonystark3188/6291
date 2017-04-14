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
		char sid[SID_LEN]="\0";
		char fw_sid[SID_LEN]="\0";
		char type[32]="\0";
		char pid[1024]="\0";
		char tmp_buf[256]="\0";
		char tmp_vid[32]="\0";
		int i,j,k;
		char *web_str=NULL;
		int ret=0;
		char admin_pwd[32]={0};
		char guest_pwd[32]={0};
		char right[8]={0};
		char read_only[8]={0};

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
		
		//processString(web_str,PID,pid);	

		processString(web_str,"adminPwd",admin_pwd); 
		processString(web_str,"guestPwd",guest_pwd); 
		processString(web_str,"right",right); 

	
		if(!strcmp(sid,fw_sid)){//是管理员
			//remove
			if(strcmp(admin_pwd,""))
			{
				sprintf(uci_option_str,"samba.@samba[0].password=%s",admin_pwd);			//name
				uci_set_option_value(uci_option_str);
				memset(uci_option_str,'\0',UCI_BUF_LEN);
				//sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":0,\"errorMessage\":\"success\"}");
					
			}
			if(strcmp(guest_pwd,"")){
				sprintf(uci_option_str,"samba.@samba[0].guest_password=%s",guest_pwd);			//name
				uci_set_option_value(uci_option_str);
				memset(uci_option_str,'\0',UCI_BUF_LEN);

				//system("uci commit; /etc/init.d/samba enable; /etc/init.d/samba restart > /dev/null &");
			}
			if((!strcmp(right,"ro"))||(!strcmp(right,"rw")))
			{
				if(!strcmp(right,"ro")) 
					strcpy(read_only,"yes");
				else strcpy(read_only,"no");
				sprintf(uci_option_str,"samba.@sambashare[0].read_only=%s",read_only);			//name
				uci_set_option_value(uci_option_str);
				memset(uci_option_str,'\0',UCI_BUF_LEN);
			}else if(strcmp(right,""))// not empty
			{
				sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\" parameter \'right\' error\"}");	
				goto exit;
			}
			sprintf(ret_buf,"%s","{\"status\":1,\"data\":{},\"errorCode\":0,\"errorMessage\":\"success\"}");							
			system("uci commit;/etc/init.d/samba enable; /etc/init.d/samba restart > /dev/null &");
			
		}
		else{
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"Not Admin\"}");
		} 
exit:
		printf("%s",ret_buf);
		fflush(stdout);
		free(web_str);
		uci_free_context(ctx);
		return ;
}

