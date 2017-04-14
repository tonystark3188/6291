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
		char fw_code[CODE_LEN]="\0";			
		char sid[SID_LEN]="\0";
		char fw_sid[SID_LEN]="\0";
		char type[32]="\0";
		char vid[1024]="\0";
		char tmp_buf[256]="\0";
		char tmp_vid[32]="\0";
		int i,j,k;
		char *web_str=NULL;
		int ret=0;

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
			fprintf(stdout,"can't get string from web\n");
			exit(1);
		}
		processString(web_str,SID,sid);		
		
		processString(web_str,CODE,code);		
		
		processString(web_str,VID,vid);	

		char *pVid=urlDecode(vid);
		//p_debug("pVid=%s",pVid);


		if(!strcmp(sid,fw_sid)||!strcmp(code,fw_code)){//是管理员
			//remove
			if(!strcmp(vid,"")){
				sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":4,\"errorMessage\":\"vid is empty\"}");
				goto exit;
			}	
			//if(strstr(vid,',')!=NULL)
			{
			
				sprintf(buf,"{\"%s\":{\"%s\":[",TASK_STATE,VID);

				//char *str=strstr(vid,',');
	//			for(i=0,j=0,k=0;i<strlen(vid);i++)
				for(i=0,j=0,k=0;i<strlen(pVid);i++)
					{
						if((pVid[i]==',')&&(pVid[i+1]!='\0'))
						{ 
							j++;
							//strncpy(tmp_vid,vid+i,(i+1));
							sprintf(tmp_buf,"\"%s\",",tmp_vid);
							strcat(buf,tmp_buf);
							memset(tmp_vid,0,strlen(tmp_vid));
							k=0;
						}
						else if((pVid[i]==',')&&(pVid[i+1]=='\0')){
							tmp_vid[k]='\0';
						}else {
							tmp_vid[k]=pVid[i];
							k++;
						}
				}
				free(pVid);pVid=NULL;


				sprintf(tmp_buf,"\"%s\"",tmp_vid);
				strcat(buf,tmp_buf);
				strcat(buf,"]}}");
			}
			//else{
			//	sprintf(buf,"{\"%s\":{\"%s\":\"%s\"}}",TASK_REMOVE,VID,);
			//}

		//	p_debug("buf=====%s",buf);
		
			
			ret = notify_server();
			if(ret <= 0){//通讯错误
				sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":13,\"errorMessage\":\"Communication Error with dm_letv\"}");
			}else//接收到消息
				printf("%s",p_client_info.recv_buf);
				fflush(stdout);

				free(p_client_info.recv_buf);
				free(web_str);
				uci_free_context(ctx);
				return ;

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

