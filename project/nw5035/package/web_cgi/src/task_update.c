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
//#include "server.h"
//#include "hidisk_udp_server.h"
//#include "usr_manage.h"
#include "msg.h"

int _handle_client_json_req(ClientTheadInfo *client_info)
{
 //   return api_process(client_info);
}
/*
unsigned char* urlDecode(char *string)  
{
	int destlen = 0;
	unsigned char *src, *dest;
	unsigned char *newstr;

	if (string == NULL) return NULL;

	for (src = string; *src != '\0'; src++)   	
	{
	   if (*src == '%')
	   	{
		   	destlen++;
			src++;
		}
	   else destlen++;
	}
	newstr = (unsigned char *)malloc(destlen + 1);
	src = string;
	dest = newstr;

	while (*src != '\0')  
	{
		if (*src == '%')
		{
			char h = toupper(src[1]);
			char l = toupper(src[2]);
			int vh, vl;
			vh = isalpha(h) ? (10+(h-'A')) : (h-'0');
			vl = isalpha(l) ? (10+(l-'A')) : (l-'0');
			*dest++ = ((vh<<4)+vl);
			src += 3;
		} 
		else if (*src == '+') 
		{
			*dest++ = ' ';
			src++;
		} 
		else
		{
			*dest++ = *src++;
		}
	}
	
	*dest = 0;

   return newstr;
}

*/
void main()
{
		char ret_buf[2048];
		char code[CODE_LEN]="\0";
		char sid[SID_LEN]="\0";
		char fw_sid[SID_LEN]="\0";

		char vid[33]="\0";
		char pid[33]="\0";
		char ext[65]="\0";
		//char tag[512]="\0";
		char *tag=NULL;
		char *web_str=NULL;
		char *f_tag=NULL;
		int ret=0;

		char uci_option_str[UCI_BUF_LEN]="\0";
		ctx=uci_alloc_context();
		
		strcpy(uci_option_str,"system.@system[0].sid");			//name
		uci_get_option_value(uci_option_str,fw_sid);
		memset(uci_option_str,'\0',UCI_BUF_LEN);

		
		printf("Content-type:text/plain\r\n\r\n");

		if(checkUdisk()==0){
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":4,\"errorMessage\":\"Disk is not mounted\"}");
			goto exit;
		}

		if((web_str=GetStringFromWeb())==NULL)
		{
			fprintf(stdout,"can't get string from web\n");
			exit(1);
		}
		processString(web_str,SID,sid);		
		
		processString(web_str,CODE,code);		
		
		//processString(web_str,VID,vid);	
		tag=(char*)malloc(strlen(web_str)+1);
		if(tag==NULL)
			{
			sprintf(ret_buf,"malloc tag error.");goto exit;
		}
		memset(tag,0,strlen(web_str)+1);
//		p_debug("webstr=%s",web_str);
		processString(web_str,TAG,tag);			
//		p_debug("tag=%s",tag);

		f_tag=urlDecode(tag);
//		p_debug("f_tag=%s",f_tag);
		
		//if(!strcmp(sid,fw_sid)){//是管理员
		if(1){//是管理员
			//add	

			if(!strcmp(tag,""))//pid为空，单个视频的下载
				{
					sprintf(ret_buf,"tag is empty.");
					goto exit;
				}
			else{
				sprintf(buf,"{\"%s\":{\"%s\":%s}}",TASK_UPDATE,TAG,f_tag);
			}

			free(f_tag);
			ret = notify_server();
			
			if(ret <= 0){//通讯错误
				sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":13,\"errorMessage\":\"Communication Error with download server.\"}");
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
//		p_debug("%s",ret_buf);
exit:
		printf("%s",ret_buf);
		fflush(stdout);
		free(web_str);
		uci_free_context(ctx);
		return ;
}

