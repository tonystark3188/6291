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

		char pid[1024]="\0";
		char tmp_buf[256]="\0";
		char tmp_pid[33]="\0";
		char type[8]="0";
		char needTag[8]="0";
		char needInfo[8]="0";
		char needAlbumInfo[8]="0";
		char needExt[8]="1";

		int i,j,k;
		char *web_str=NULL;
		int ret=0;

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
		
		processString(web_str,PID,pid);	
		
		processString(web_str,"type",type);			

		processString(web_str,"needTag",needTag); 

		processString(web_str,"needInfo",needInfo);	

		processString(web_str,"needAlbumInfo",needAlbumInfo);	

		if(!strcmp(sid,fw_sid)){//是管理员
			//remove
			//if(strstr(vid,',')!=NULL)
			sprintf(buf,"{\"%s\":{},\"needTag\":\"%s\",\"needInfo\":\"%s\",\"needAlbumInfo\":\"%s\",\"needExt\":\"%s\",\"type\":\"%s\",\"pid\":[",FOLLOW_GETALL,needTag,needInfo,needAlbumInfo,needExt,type);
			if(strlen(pid)==0){
				strcat(buf,tmp_buf);
				strcat(buf,"]}");
			}else{
				for(i=0,j=0,k=0;i<strlen(pid);i++)
					{
						if(pid[i]==',')
						{ 
							j++;
							//strncpy(tmp_pid,vid+i,(i+1));
							sprintf(tmp_buf,"\"%s\",",tmp_pid);
							strcat(buf,tmp_buf);
							memset(tmp_pid,0,strlen(tmp_pid));
							k=0;
						}
						else {
							tmp_pid[k]=pid[i];
							k++;
						}
				}
				sprintf(tmp_buf,"\"%s\"",tmp_pid);
				strcat(buf,tmp_buf);
				strcat(buf,"]}");
			}
			

			
			//else{
			//	sprintf(buf,"{\"%s\":{\"%s\":\"%s\"}}",TASK_REMOVE,VID,);
			//}

			//p_debug("buf=====%s",buf);
		
			
			ret = notify_server();
			if(ret <= 0){//通讯错误
				sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":13,\"errorMessage\":\"Communication Error with dm_letv\"}");
			}else//接收到消息
				fprintf(stdout,p_client_info.recv_buf);
				fflush(stdout);

				free(p_client_info.recv_buf);
				free(web_str);
				uci_free_context(ctx);
				return ;

		}else{
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"security error\"}");
		} 

		printf("%s",ret_buf);
		fflush(stdout);
		free(web_str);
		uci_free_context(ctx);
		return ;
}

