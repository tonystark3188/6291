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
#include "router_task.h"
int _handle_client_json_req(ClientTheadInfo *client_info)
{
 //   return api_process(client_info);
}

#define TYPE_VIDEO "0"
#define TYPE_VIDEO_COVER "1"
#define TYPE_ALBUM_COVER "2"

#define ROOT_FILE_PATH "/tmp/mnt/USB-disk-1/hack/"

#define BUF_LEN 65536 //32768

#define VIDEO_BUF_LEN 65536 // 32768 //65536  //131072 //524288//524288 1MB 1048576  4MB 4194304

int parse_range(char *buf, long long *range0, long long *range1)
{
#define MAX_RANGE 32 // 10^16

	p_debug("buf is %s", buf);

	//p_debug("Entry parse_range.........");
	char *p = strstr(buf, "=");
	char *q = strstr(buf, "-");
	char R0[MAX_RANGE] = {0};
	char R1[MAX_RANGE] = {0};

	*range0 = 0;
	*range1 = 0;
	
	if(p == NULL || q == NULL)
	{
		return 0;
		
	}else
	{
		p++; // after '='
		while(*p == ' ')
			p++;		

		int num = q-p;

		if(num < MAX_RANGE)
		{
			strncpy(R0, p, num);
			*range0 = atoll(R0);
			p_debug("range0 is %s(%lld),num = %d", R0,*range0, num);
		}else
		{
			*range0 = 0;
		}

		q = q+1;
		strcpy(R1,q);
		p_debug("rang1 is (%s),strlen=%d",R1,strlen(R1));
		*range1 = atoll(R1);

		p_debug("range0 is %lld, range1 is %lld(%lld)", *range0, *range1,atoll(R1));
		
	}

	return 0;
}

int destory_task_list(struct task_dnode *dn)
{
	unsigned i = 0;
	struct task_dnode *head;
	if(dn == NULL)
		return;
	for(i = 0;/*!dn*/;i++)
	{
		head = dn->dn_next;
		free(dn);
		if(!head)
			break;
		dn = head;
	}
	p_debug("free succ");
	return 0;
}

int read_list_from_file(const char *path,struct task_dnode **dn)//读入数据
 {	 
 //p_debug("access read_list_from_file");
	  FILE *record_fd;
	  int enRet;
	  struct task_dnode *cur=NULL;
	  int ret = 0;
	  int i = 0;
	  if(access(path,R_OK)!=0){// 不存在
		  if((record_fd = fopen(path,"wb+"))==NULL)//改动：路径
		  {
			 p_debug("task list file does not exist error1[errno:%d]",errno);
			 return -1;
		  }
	  }else  if((record_fd = fopen(path,"r"))==NULL)//改动：路径
	  {
	//	 p_debug("task list file does not exist error2[errno:%d]",errno);
		 return -1;
	  }
	  
	  while(!feof(record_fd))
	  {
		 cur = (struct task_dnode *)malloc(sizeof(*cur));
		 memset(cur,0,sizeof(*cur));
		 cur->dn_next=NULL;
		 enRet = fread(cur,sizeof(struct task_dnode),1,record_fd);
		 if(enRet <= 0)
	     {	
	     	if(cur!=NULL)free(cur);
			p_debug("fread error,enRet = %d,errno = %d",enRet,errno);
			break;
		 }
		 //p_debug("i:%d,pid = %s,vid=%s,download_status = %d",i,cur->pid,cur->vid,cur->download_status);
		
		if(!cur)
		{
			p_debug("malloc task_dn error");
			return -2;
		}
		cur->dn_next = *dn;
		*dn = cur;
		 i++;
	  }
	  fclose(record_fd);
//	  p_debug("leave read_list_from_file");

	  return ret;
 }

void turn_led_on(void){
	#if 1
		p_debug("access turn_led_on..");
		char isDownloading[8]={0};
		get_conf_str(isDownloading,"isDownloading");
		if(!strcmp(isDownloading,"true"))
		{
			return;
		}
		else{
			p_debug("stop breath ......");
		//	system("/usr/bin/mcu_control -s 1 >/dev/null");
			system("pwm_control 1  1 0;pwm_control 1 0 0 >/dev/null");
			updateSysVal("led_status","1");	
			return;
		}
	#endif
}
void main()
{
		char ret_buf[RET_BUF_LEN];
		char code[CODE_LEN_65]="\0";
		char save_code[CODE_LEN_65]="\0";		
		char sid[SID_LEN]="\0";
		char fw_sid[SID_LEN]="\0";
		char type[32]="\0";
		char vid[VID_LEN_33]="\0";
		char aid[VID_LEN_33]="\0";		
		char tmp_buf[256]="\0";
		char tmp_vid[VID_LEN_33]="\0";
		char path[256]="\0";
		int i,j,k;
		char *web_str=NULL;
		unsigned int ret=0;
		char ip[32]="\0";
		char uci_option_str[128]="\0";
		
		long long read_bytes=0;
		long long read_len=0;
		long long write_len=0;
		FILE  *fd;
		char range[32]="\0";
		char str_start[32]="\0";
		long long start=0;
		long long end=0;
		long long need_read_len=0;
		long long fsize=0;
		char led_status[8]="\0";
		ctx=uci_alloc_context();
		
		strcpy(uci_option_str,"system.@system[0].sid");			//name
		uci_get_option_value(uci_option_str,fw_sid);
		memset(uci_option_str,'\0',128);
		
		strcpy(uci_option_str,"system.@system[0].code");			//name
		uci_get_option_value(uci_option_str,save_code);
		memset(uci_option_str,'\0',128);	
#if 0
		strcpy(uci_option_str,"system.@system[0].led_status");			//name
		uci_get_option_value(uci_option_str,led_status);
		memset(uci_option_str,'\0',128);	
#endif
	//	p_debug(getenv("HTTP_USER_AGENT"));
		//p_debug(getenv("PATH_INFO"));
		//p_debug(getenv("CONTENT_TYPE"));
	//	p_debug(getenv("REMOTE_ADDR"));

		if(getenv("HTTP_HOST")!=NULL)
			strcpy(ip,getenv("HTTP_HOST"));
		else p_debug("get HTTP_HOST error!");

		if((web_str=GetStringFromWeb())==NULL)
		{
			printf("Content-type:text/plain\r\n\r\n");
			fprintf(stdout,"can't get string from web\n");
			exit(1);
		}
		processString(web_str,SID,sid);		
		
		processString(web_str,CODE,code);		
		
		processString(web_str,"type",type);	

		processString(web_str,VID,vid);	

		processString(web_str,"aid",aid);	


		if(!strcmp(sid,fw_sid)||!strcmp(save_code,code)){//是管理员，或者是有效访客
		//if(1){//是管理员

			if(!strcmp(type,TYPE_VIDEO)){
				
				if(getenv("HTTP_RANGE")==NULL){
					printf("Status:200 OK\r\n");

					sprintf(path,"%s%s.mp4",ROOT_FILE_PATH,vid);				
					fd=fopen(path,"rb");
					if(fd==NULL){
						printf("Status:404 Not Found\r\n");
						p_debug("open %s failed.",path);
						free(web_str);
						uci_free_context(ctx);
						turn_led_on();// wifi led on
						return;
					}
					fseek(fd, 0, SEEK_END);
					fsize = ftell(fd);
					rewind(fd);
					
					
					printf("Accept-Ranges: bytes\r\n");

					printf("Content-Range: bytes 0-%lld/%lld\r\n",fsize-1,fsize); 
					printf("Content-Type: video/mp4\r\n");
					fprintf(stdout,"Content-Length: %lld\r\n\r\n",fsize);
					fflush(stdout);
					//char fileBuf[VIDEO_BUF_LEN]="\0";
					while(!feof(fd)){
						 //cur = malloc(sizeof(*cur));
						 //unsigned int fsize = ftell(fd);
						 char *fileBuf=malloc(VIDEO_BUF_LEN);
						 memset(fileBuf,0,VIDEO_BUF_LEN);
						 if((fsize-read_bytes)>VIDEO_BUF_LEN)
							 read_len = fread(fileBuf,1,VIDEO_BUF_LEN,fd);
						 else 
							 read_len = fread(fileBuf,1,(fsize-read_bytes),fd);

	 					 //p_debug("read_bytes=%lld,read_len=%lld,fsize=%lld",read_bytes,read_len,fsize);
						 if(read_len <= 0)
					     {	
					 //    	p_debug("fread error,read_len = %lld,errno = %d",read_len,errno);
							free(web_str);
					 		free(fileBuf);
							fclose(fd);
							uci_free_context(ctx);	
							turn_led_on();// wifi led on
							return;
			 			 }
						 write_len=fwrite(fileBuf,read_len,1,stdout);
 						 fflush(stdout);
						 //p_debug("write len=%lld",write_len);
						 if(write_len==0)
						 { 
	        				p_debug("fwrite error,write_len = %d,errno = %d",write_len,errno);
							fclose(fd);
							p_debug("write error=%d",write_len);	
							free(web_str);
							free(fileBuf);
							uci_free_context(ctx);	
							turn_led_on();
						 	return;
						 }
						 free(fileBuf);
						 read_bytes=read_bytes+read_len;
					}

					free(web_str);
					uci_free_context(ctx);
					turn_led_on();// wifi led on

					fclose(fd);
					return;
				}
				else{
					printf("Status: 206 Partial Content\r\n");
					//p_debug("start 206 Partial Content download,");
					strcpy(range,getenv("HTTP_RANGE"));
					//p_debug("range====%s==",range);
					parse_range(range,&start,&end);
					//p_debug("start=%lld,end=%lld",start,end);

					sprintf(path,"%s%s.mp4",ROOT_FILE_PATH,vid);				
					fd=fopen(path,"rb");
					if(fd==NULL){
						printf("Status: 404 Not Found\r\n");
						//printf("open %s failed.",path);
						free(web_str);
						uci_free_context(ctx);
						turn_led_on();// wifi led on						
						return;
					}
					fseek(fd, 0, SEEK_END);
					fsize = ftell(fd);
					rewind(fd);
					
					fseek(fd,start,SEEK_SET);

					
					//printf("ETag: \"278099835\"\r\n");
					//printf("Last-Modified: Mon, 05 Nov 2012 23:06:34 GMT\r\n");
					//printf("Accept-Ranges:bytes\r\n");
					//printf("Connection: keep-alive\r\n");
					//if((end != 0) && (end !=1)){ 
					/*if(end==1) {
						need_read_len=983040;
						end=983039;
						printf("Content-Range:bytes %lld-%lld/%lld\r\n",start,end,fsize);
						p_debug("Content-Range:bytes %lld-%lld/%lld\r\n",start,end,fsize);		
					}else*/
					if((end != 0)){ 						
						need_read_len=end-start+1;
						//sprintf(stdout,"Content-Length:%lu",need_read_len);
						fprintf(stdout,"Content-length: %lld\r\n",need_read_len);
						fflush(stdout);//sprintf(stdout,"Content-Length:%lu",need_read_len);
						

						printf("Content-Range: bytes %lld-%lld/%lld\r\n",start,end,fsize);
			//			p_debug("Content-Range: bytes %lld-%lld/%lld\r\n",start,end,fsize);		
					}
					else {
						need_read_len=fsize-start+1;
						fprintf(stdout,"Content-length: %lld\r\n",need_read_len);
						fflush(stdout);//sprintf(stdout,"Content-Length:%lu",need_read_len);
						printf("Content-Range: bytes %lld-%lld/%lld\r\n",start,fsize-1,fsize);		
			//			p_debug("Content-Range: bytes %lld-%lld/%lld\r\n",start,fsize-1,fsize);		
					}

			//		p_debug("need_read_len=%lu",need_read_len);

					printf("Content-type: video/mp4\r\n\r\n");
					//char fileBuf[VIDEO_BUF_LEN]="\0";
					while(!feof(fd)){
						system("echo 3 > /proc/sys/vm/drop_caches ");
						 char *fileBuf=malloc(VIDEO_BUF_LEN);
						 memset(fileBuf,0,VIDEO_BUF_LEN);
						 //cur = malloc(sizeof(*cur));
						 //unsigned int fsize = ftell(fd);
						 if((need_read_len-read_bytes)>VIDEO_BUF_LEN)
							 read_len = fread(fileBuf,1,VIDEO_BUF_LEN,fd);
						 else 
							 read_len = fread(fileBuf,1,(need_read_len-read_bytes),fd);

	 		//			 p_debug("read_bytes=%lld,read_len=%lld,need_read_len=%lld",read_bytes,read_len,need_read_len);
						 if(read_len <= 0)
					     {	
			//		     	p_debug("fread error,read_len = %lld,errno = %d",read_len,errno);
							free(web_str);
							free(fileBuf);
							fclose(fd);
							uci_free_context(ctx);	
							turn_led_on();// wifi led on
							return;
			 			 }
						 write_len=fwrite(fileBuf,read_len,1,stdout);
						 fflush(stdout);
				//		 p_debug("write len=%lld",write_len);
						 if(write_len==0)
						 { 
	        				p_debug("fwrite error,write_len = %lld,errno = %d",write_len,errno);
							fclose(fd);
							free(fileBuf);
							p_debug("write error=%lld",write_len);		
							free(web_str);p_debug("access turn_led_on1..");	
							uci_free_context(ctx);	
							turn_led_on();// wifi led on
						 	return;
						 }
						 free(fileBuf);
						 read_bytes=read_bytes+read_len;
					//	updateSysVal("led_status","4");
					//	system("pwm_control 1 0 0;pwm_control 0 0 10000 >/dev/null");
						//system("mcu_control -s 1 >/dev/null");// 1s breath
					}
			//		p_debug("access turn_led_on2..");	

					fclose(fd);	
				//	p_debug("access turn_led_on0..");					
					turn_led_on();// wifi led on
					return;
					
				
					}				
			}else 	if(!strcmp(type,TYPE_VIDEO_COVER)){
				char fileBuf[BUF_LEN]="\0";

				if(getenv("HTTP_RANGE")==NULL)
					printf("Status:200 OK\r\n");
				else{
					printf("Status:206 Partial Content\r\n");
				}
				
				sprintf(path,"%s%s.jpg",ROOT_FILE_PATH,vid);				
				fd=fopen(path,"rb");
				if(fd==NULL){
					printf("Status:404 Not Found\r\n");
					//printf("open %s failed.",path);
					free(web_str);
					uci_free_context(ctx);
					return;
				}
				fseek(fd, 0, SEEK_END);
				fsize = ftell(fd);
				rewind(fd);

				printf("Content-type:image/jpeg\r\n");
				fprintf(stdout,"Content-Length:%lld\r\n\r\n",fsize);
				while(!feof(fd)){
					 //cur = malloc(sizeof(*cur));
					 //unsigned int fsize = ftell(fd);
					 if((fsize-read_bytes)>BUF_LEN)
						 read_len = fread(fileBuf,1,BUF_LEN,fd);
					 else 
						 read_len = fread(fileBuf,1,(fsize-read_bytes),fd);

 					 //p_debug("read_bytes=%lld,fsize=%lld",read_bytes,fsize);
					 if(read_len <= 0)
				     {	
						fclose(fd);
						p_debug("fread error,ret = %lld,errno = %d",read_len,errno);
						free(web_str);
						fclose(fd);
						uci_free_context(ctx);
						return;;
		 			 }
					 write_len=fwrite(fileBuf,read_len,1,stdout);
					 //p_debug("write len=%lld",write_len);
					 if(write_len==0)
						{ 
						fclose(fd);
						p_debug("write error=%d",write_len);	

						free(web_str);
						uci_free_context(ctx);

					 	return;
					 }
					 
					 read_bytes=read_bytes+read_len;
				}

				fclose(fd);
				
				
			}else 	if(!strcmp(type,TYPE_ALBUM_COVER)){
				char fileBuf[BUF_LEN]="\0";
				if(strcmp(aid,"")){
					sprintf(path,"%s%s.jpg",ROOT_FILE_PATH,aid);	
					fd=fopen(path,"rb");
					if(fd==NULL){//if albumCover is not exist, return videoCover
							printf("Status:404 Not Found\r\n");
							p_debug("open %s failed.",path);
							free(web_str);
							uci_free_context(ctx);
							return;
						}					
				}
				else
				{
					struct task_dnode *task_dn=NULL;

					read_list_from_file(LETV_TASKLIST_FILE_PATH,&task_dn);
					struct task_dnode *dn=task_dn;

					for(i=0;;i++){
						if(!dn)break;
						//p_debug("find the vid's pid=%s",dn->pid);
						if(!strcmp(dn->vid,vid)){
							//p_debug("find the vid's pid=%s",dn->pid);
							break;
						}
						dn=dn->dn_next;
					}
					
					

					if(getenv("HTTP_RANGE")==NULL)
						printf("Status:200 OK\r\n");
					else{
						printf("Status:206 Partial Content\r\n");
					}
					
					sprintf(path,"%s%s.jpg",ROOT_FILE_PATH,dn->pid);	
					//p_debug(path);

					fd=fopen(path,"rb");
					if(fd==NULL){//if albumCover is not exist, return videoCover
						memset(path,0,strlen(path));
						sprintf(path,"%s%s.jpg",ROOT_FILE_PATH,dn->vid);
						destory_task_list(task_dn);dn=NULL;	
						p_debug(path);
						fd=fopen(path,"rb");	
						if(fd==NULL){
							printf("Status:404 Not Found\r\n");
							//printf("open %s failed.",path);
							free(web_str);
							uci_free_context(ctx);
							return;
						}
					}
				}

				
				fseek(fd, 0, SEEK_END);
				fsize = ftell(fd);
				rewind(fd);

				printf("Content-type:image/jpeg\r\n");
				fprintf(stdout,"Content-Length:%lld\r\n\r\n",fsize);
				while(!feof(fd)){
					 //cur = malloc(sizeof(*cur));
					 //unsigned int fsize = ftell(fd);
					 if((fsize-read_bytes)>BUF_LEN)
						 read_len = fread(fileBuf,1,BUF_LEN,fd);
					 else 
						 read_len = fread(fileBuf,1,(fsize-read_bytes),fd);

 					 p_debug("read_bytes=%lld,fsize=%lld",read_bytes,fsize);
					 if(read_len <= 0)
				     {	
						fclose(fd);
						p_debug("fread error,ret = %lld,errno = %d",read_len,errno);
						free(web_str);
						fclose(fd);
						uci_free_context(ctx);
						return;;
		 			 }
					 write_len=fwrite(fileBuf,read_len,1,stdout);
					 //p_debug("write len=%lld",write_len);
					 if(write_len==0)
						{ 
						fclose(fd);
						p_debug("write error=%d",write_len);	

						free(web_str);
						uci_free_context(ctx);

					 	return;
					 }
					 
					 read_bytes=read_bytes+read_len;
				}

				fclose(fd);
				
			}
/*				
			//remove
			//if(strstr(vid,',')!=NULL)
			{
				sprintf(buf,"{\"%s\":{\"%s\":\"%s\",\"%s\":\"%s\",\"ip\":\"%s\"}}",FILE_DOWNLOAD,"type",type,VID,vid,ip);
			}
			//else{
			//	sprintf(buf,"{\"%s\":{\"%s\":\"%s\"}}",TASK_REMOVE,VID,);
			//}
			p_debug("buf=====%s",buf);
		
			
			ret = notify_server();
			if(ret <= 0){//通讯错误
				sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":13,\"errorMessage\":\"Communication Error with dm_letv\"}");
			}else//接收到消息
				sprintf(ret_buf,"%s",buf);
		}else{
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"Not Admin\"}");
		*/} 
			else
			{
				printf("Content-type:text/plain\r\n\r\n");
				sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"You are Not the admin or a valid guest!\"}");
		}
		
		printf("%s",ret_buf);
		fflush(stdout);
		free(web_str);
		uci_free_context(ctx);
		turn_led_on();// wifi led on
		return ;
}

