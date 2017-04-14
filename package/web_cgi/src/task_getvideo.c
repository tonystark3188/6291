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
		char fw_code[CODE_LEN]="\0";
		char sid[SID_LEN]="\0";
		char fw_sid[SID_LEN]="\0";
		//char *vid=NULL;
		char p_vid[1024]="\0";
		char pid[32]="\0";
		char tmp_buf[256]="\0";
		char tmp_vid[32]="\0";
		int i,j,k;
		char *web_str=NULL;
		int ret=0;

		char uci_option_str[128]="\0";
		ctx=uci_alloc_context();
		
		strcpy(uci_option_str,"system.@system[0].sid");			//name
		uci_get_option_value(uci_option_str,fw_sid);
		memset(uci_option_str,'\0',128);

		strcpy(uci_option_str,"system.@system[0].code");			//name
		uci_get_option_value(uci_option_str,fw_code);
		memset(uci_option_str,'\0',128);
		
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
		
		processString(web_str,VID,p_vid);	

		processString(web_str,PID,pid);	

		char *pVid=urlDecode(p_vid);
		p_debug("pVid=%s",pVid);
		
		p_debug("p_vid=%s,urldecode_vid=%s",p_vid,pVid);

		if(!strcmp(sid,fw_sid)||!strcmp(code,fw_code)){//是管理员
			if(!strcmp(pVid,"")){
					sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":4,\"errorMessage\":\"vid is empty\"}");
					goto exit;
			}

			//remove
			sprintf(buf,"{\"%s\":{\"%s\":[",TASK_GET_VIDEO_INFO,VID);

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

			p_debug("buf=====%s",buf);
		
			
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

