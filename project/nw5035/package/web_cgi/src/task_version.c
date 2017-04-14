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
		char ret_buf[2048];
		char code[CODE_LEN]="\0";
		char sid[SID_LEN]="\0";
		char fw_sid[SID_LEN]="\0";

		char type[32]="\0";
		char pid[32]="\0";
		char tmp_buf[256]="\0";
		char tmp_vid[32]="\0";
		int i,j,k;
		char *web_str=NULL;
		int ret=0;
		char uVer[32]="0";
		char fVer[32]="0";

	
		get_conf_str(uVer,"unfinishedVer");
		get_conf_str(fVer,"completedVer");	
		if(!strcmp(uVer,"")||!strcmp(fVer,""))
		{//第一次开机启动值为空，可能导致APP不请求列表	
			strcpy(uVer,"1111");
			strcpy(fVer,"1222");
		}
		
		printf("Content-type:text/plain\r\n\r\n");

		if((web_str=GetStringFromWeb())==NULL)
		{
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":4,\"errorMessage\":\"Can not get any parameters.\"}");
			printf("%s",ret_buf);
			fflush(stdout);
			//uci_free_context(ctx);
			return ;

		}
		processString(web_str,SID,sid);		
		
		processString(web_str,CODE,code);		
		
		processString(web_str,"type",type);	

		processString(web_str,PID,pid);	



		//if(!strcmp(sid,fw_sid)){//是管理员
		if(1){//是管理员			//remove
			//if(strstr(vid,',')!=NULL)

			//else{
			//	sprintf(buf,"{\"%s\":{\"%s\":\"%s\"}}",TASK_REMOVE,VID,);
			//}

		
			
			//ret = notify_server();
		
			sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"unFinishedVer\":\"%s\",\"completedVer\":\"%s\"}}",uVer,fVer);
			p_debug("buf=====%s",buf);

		}

		printf("%s",ret_buf);
		fflush(stdout);
		free(web_str);
		//uci_free_context(ctx);
		return ;
}

