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

int escapeSpace(char *strings){
	int i,j=0;
	char *tmp=malloc(strlen(strings));
	memset(tmp,0,sizeof(tmp));
	int len=strlen(strings);
	for(i=0;i<len;i++){
		if(*(strings+i)!=' ')//del escape
			{
			*(tmp+j)=*(strings+i);
			j++;
		}
	}
	strcpy(strings,tmp);
	strings[j]='\0';	
	if(!tmp){free(tmp);tmp=NULL;}
	return 0;
}
void main()
{
		char ret_buf[2048];
		char code[CODE_LEN]="\0";
		char sid[SID_LEN]="\0";
		char fw_sid[SID_LEN]="\0";
		char vid[33]="\0";
		char pid[33]="\0";
		char ext[256]="\0";
		char tag[1024]="\0";
		
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
		
		processString(web_str,VID,vid);	

		processString(web_str,EXT,ext);	
		//p_debug("ext1=%s",ext);

		processString(web_str,TAG,tag);	

		processString(web_str,PID,pid);	
		

		char *pExt=urlDecode(ext);
		//p_debug("pExt=%s",pExt);
		//escapeSpace(pExt);
		//p_debug("pExt2=%s",pExt);


		if(pExt[0]=='\0')
					sprintf(pExt,"%s","{}");

		f_tag=urlDecode(tag);
		if(f_tag[0]=='\0')
			sprintf(f_tag,"%s","{}");
		
		//if(!strcmp(sid,fw_sid)){//是管理员
		if(1){//是管理员
			//add	
			if(!strcmp(vid,"")||!strcmp(vid,"0")){
					sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":4,\"errorMessage\":\"vid is empty\"}");
					goto exit;
			}
			if(!strcmp(pExt,"")){
					//sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":4,\"errorMessage\":\"ext is empty\"}");
					;//goto exit;
			}

			if(!strcmp(pid,""))//pid为空，单个视频的下载
				{
				if(f_tag[0]!='\0')sprintf(buf,"{\"%s\":{\"%s\":\"%s\",\"%s\":\"-%s\",\"%s\":%s,\"%s\":%s}}",TASK_ADD,VID,vid,PID,vid,EXT,pExt,TAG,f_tag);
				else
					sprintf(buf,"{\"%s\":{\"%s\":\"%s\",\"%s\":\"-%s\",\"%s\":%s,\"%s\":%s}}",TASK_ADD,VID,vid,PID,vid,EXT,pExt,TAG,"{}");
				}
			else{
				if(f_tag[0]!='\0')sprintf(buf,"{\"%s\":{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":%s,\"%s\":%s}}",TASK_ADD,VID,vid,PID,pid,EXT,pExt,TAG,f_tag);
				else
					sprintf(buf,"{\"%s\":{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":%s,\"%s\":%s}}",TASK_ADD,VID,vid,PID,pid,EXT,pExt,TAG,"{}");
				
			}

			free(f_tag);free(pExt);
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
		p_debug("%s",ret_buf);
exit:
		printf("%s",ret_buf);
		fflush(stdout);
		free(web_str);
		uci_free_context(ctx);
		return ;
}

/*
int update_usr_time(struct usr_dnode **dn,uint32_t cur_time)
{
	unsigned i = 0;
	int expire_time;
	char client_ip[32];
	int del_flag = 0;
	int ret = 0;
	struct usr_dnode *tmp = *dn;
	for(i = 0;;i++)
	{
		if(!tmp)
			break;
		expire_time = cur_time - tmp->start_time;
		p_debug("i = %d,tmp->ip = %s,expire_time = %d,tmp->count = %d",i,tmp->ip,expire_time,tmp->count);
		if(expire_time >= 2&&6*expire_time > tmp->count)
		{
			//通知服务端释放指定客户端的资源
			p_debug("notify_server_disconnect_status");
			ret = notify_server_disconnect_status(tmp->ip);
			if(ret >= 0)
			{
				del_flag = 1;
				memset(client_ip,0,32);
				strcpy(client_ip,tmp->ip);
				break;
			}
		}
		tmp= tmp->dn_next;
	}
	if(del_flag == 1)
		del_usr_from_list_for_ip(dn,client_ip);
	return 0;
}


int _udp_listen_clients(void)
{
	int ret = -1;
	ClientTheadInfo p_client_info;
	memset(&p_client_info,0,sizeof(ClientTheadInfo));
	p_client_info.client_fd = init_udp_server(LISTEN_PORT);	
	p_client_info.time_out = 1000;//default 3000
	time_t timep;
	struct tm *p;
	uint32_t cur_time;
	while(1)
	{
		ret = recv_req_from_udp_client(&p_client_info);
        if(ret == 1)
		{
#ifdef DELETE_FUNC
			time(&timep);
			p = localtime(&timep);
			timep = mktime(p);
			//p_debug("time()->localtime()->mktime():%d",timep);
			extern struct usr_dnode *usr_dn;
			update_usr_time(&usr_dn,timep);
#endif
			//p_debug("timeout");
			continue;
		}
		else if(ret < 0)
		{
		   p_debug("_recv_req_from_client failed!");
		   break;
		}
#ifdef DELETE_FUNC
		time(&timep);
		p = localtime(&timep);
		timep = mktime(p);
		p_debug("time()->localtime()->mktime():%d",timep);
		extern struct usr_dnode *usr_dn;
		update_usr_time(&usr_dn,timep);
#endif
		ret = _parse_client_header_json(&p_client_info);
		if(ret == -1)
		{
			p_debug("parse error");
			safe_free(p_client_info.recv_buf);
			continue;
		}
        _handle_client_json_req(&p_client_info);
		p_client_info.client_fd_send = DM_UdpClientInit(PF_INET, SEND_PORT, SOCK_DGRAM, p_client_info.ip, &p_client_info.clientAddrSend);
		if(send_result_to_udp_client(&p_client_info) != 0)
        {
            p_debug("send result to client failed, We will exit thread now!");
			safe_free(p_client_info.recv_buf);
			safe_free(p_client_info.retstr);
			safe_close(p_client_info.client_fd_send);
			continue;
        }
		safe_free(p_client_info.recv_buf);			
		safe_free(p_client_info.retstr);
		safe_close(p_client_info.client_fd_send);
	}
	stop_udp_listen_task(p_client_info.client_fd);
	return 0;
}
*/


