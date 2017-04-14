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
#include "my_json.h"
#include "router_task.h"


int _handle_client_json_req(ClientTheadInfo *client_info)
{
 //   return api_process(client_info);
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
	//p_debug("free succ");
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
		  if((record_fd = fopen(path,"rb"))==NULL)//改动：路径
		  {
			 p_debug("task list file does not exist error1[errno:%d]",errno);
			 return -1;
		  }
	  }else  if((record_fd = fopen(path,"r"))==NULL)//改动：路径
	  {
		 p_debug("task list file does not exist error2[errno:%d]",errno);
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
		//	p_debug("fread error,enRet = %d,errno = %d",enRet,errno);
			break;
		 }
		 //p_debug("i:%d,pid = %s,vid=%s,download_status = %d",i,cur->pid,cur->vid,cur->download_status);
		
		if(!cur)
		{
			p_debug("malloc task_dn error");
			 fclose(record_fd);
			return -2;
		}
		cur->dn_next = *dn;
		*dn = cur;
		 i++;
	  }
	  fclose(record_fd);
	  //p_debug("leave read_list_from_file");

	  return ret;
 }

char tmp_pid[33]="\0";
char needInfo[32]="\0";
char type[32]="\0";
char *all_buf=NULL;

int Parser_GetAllTask(int isAdmin)
{
	int ret=0;
	int array_len=0;
	int j,i=0;
	//char type[8]="\0";
	//char needInfo[8]="\0";
	char tmp_vid[33]="\0";
	char is_backup[8]="\0";
	//char tmp_pid[33]="\0";

//	p_debug("access Parser_GetAllTask");
	get_conf_str(is_backup,"is_backup");
	while((is_backup[0] != '1')&&(i<3)){
		sleep(1);
		i++;
	}

	char status[8]="\0";
		char pr[8]="\0";
	char tag[512]="\0";
	float lu_pr;
	struct task_dnode *task_dn=NULL;
	read_list_from_file(LETV_TASKLIST_FILE_PATH,&task_dn);
	struct task_dnode *dn=task_dn;

	//if(type[0]=='0'&&(isAdmin==1)){//all videos
	if((type[0]=='0')){//all videos
		JObj *my_array = JSON_NEW_ARRAY();
		JObj *my_obj = JSON_NEW_EMPTY_OBJECT();
		JObj *final_obj = JSON_NEW_EMPTY_OBJECT();

		for(i=0;;i++){
				
			if(!dn)break;
			if(dn->isDeleted==1) {
				dn=dn->dn_next;
				continue;
			}

			if((strcmp(tmp_pid,""))){// return the pid's vid info
				if(strcmp(dn->pid,tmp_pid))
					{
					dn=(dn)->dn_next;
					continue;
				}
			}
			
		//	JObj *tmp_obj = JSON_NEW_EMPTY_OBJECT();//JSON_NEW_OBJECT(member,type);
			

			JObj *tmp_obj = JSON_NEW_EMPTY_OBJECT();//JSON_NEW_OBJECT(member,type);
			sprintf(tmp_vid,"%s",dn->vid);
			
			//JSON_ADD_OBJECT(tmp_obj,"vid",json_object_new_string(tmp_vid));
			JSON_ADD_OBJECT(tmp_obj,"vid",json_object_new_string(dn->vid));
			JSON_ADD_OBJECT(tmp_obj,"pid",json_object_new_string(dn->pid));

			if((needInfo[0] == '\0')||(needInfo[0] == '1')) {
				JObj *info_json=JSON_PARSE(dn->info);
				if(info_json==NULL) 	JSON_ADD_OBJECT(tmp_obj,"info",JSON_NEW_EMPTY_OBJECT());
				else
					JSON_ADD_OBJECT(tmp_obj,"info",info_json);
			}

			JSON_ADD_OBJECT(tmp_obj,"completeTime",json_object_new_int(dn->update_task_time));

			sprintf(status,"%d",(dn)->download_status);
			JSON_ADD_OBJECT(tmp_obj,"status",json_object_new_string(status));
			//JSON_ADD_OBJECT(tmp_obj,"status",json_object_new_string((dn)->download_status));

			if(dn->total_size!=0)lu_pr=(float)dn->downloaded_size/(float)dn->total_size*100;
			else lu_pr=0;
			sprintf(pr,"%.0f",lu_pr);
			
			JSON_ADD_OBJECT(tmp_obj,"pr",json_object_new_int((int)(lu_pr)));


			sprintf(tag,"%s",dn->tag);
			//p_debug("tag len=%d",strlen(tag));
			JObj *json_tag=JSON_PARSE(dn->tag);
			if(json_tag==NULL)	{//tag 为空，返回info
				//JSON_ADD_OBJECT(tmp_obj,"tag",JSON_NEW_EMPTY_OBJECT());
				JSON_ADD_OBJECT(tmp_obj,"info",JSON_PARSE(dn->info));
			}else
			JSON_ADD_OBJECT(tmp_obj,"tag",json_tag);


			JSON_ADD_OBJECT(tmp_obj,"progress",json_object_new_int(dn->downloaded_size));

			JSON_ADD_OBJECT(tmp_obj,"totalSize",json_object_new_int(dn->total_size));
			//p_debug("tmpobj===%s",JSON_TO_STRING(tmp_obj));

			JSON_ARRAY_ADD_OBJECT(my_array,tmp_obj);//add {vid,status,pr} to array
			//p_debug("%s",dn->vid);

			dn=(dn)->dn_next;
		
			//JSON_PUT_OBJECT(tmp_obj);
		}
		//p_debug("my_array===%s",JSON_TO_STRING(my_array));

		JSON_ADD_OBJECT(my_obj,"list",my_array);
		JSON_ADD_OBJECT(final_obj,"status",json_object_new_int(1));
		JSON_ADD_OBJECT(final_obj,"errorCode",json_object_new_int(0));		
		JSON_ADD_OBJECT(final_obj,"errorMessage",json_object_new_string("success"));		
		JSON_ADD_OBJECT(final_obj,"data",my_obj);

		//p_debug("getalltask===%s",JSON_TO_STRING(final_obj));
		
		all_buf=JSON_TO_STRING(final_obj);
		//sprintf(c->loc.io.buf,"%s",);
		//p_debug("c->loc.io.buf=====%s",c->loc.io.buf);
		//JSON_PUT_OBJECT(final_obj);//just need to free the final_obj
	}
	else if(type[0]=='1'&&(isAdmin!=2)){//already downloaded videos
		JObj *my_array = JSON_NEW_ARRAY();
		JObj *my_obj = JSON_NEW_EMPTY_OBJECT();
		JObj *final_obj = JSON_NEW_EMPTY_OBJECT();
		for(i=0;;i++){
			if(!dn)break;
			if(dn->isDeleted==1) {
				dn=dn->dn_next;
				continue;
			}

			if((strcmp(tmp_pid,""))){// return the pid's vid info
				if(strcmp(dn->pid,tmp_pid))
					{
					dn=(dn)->dn_next;
					continue;
				}
			}
			if(dn->download_status==DONE){
				JObj *tmp_obj = JSON_NEW_EMPTY_OBJECT();//JSON_NEW_OBJECT(member,type);
				sprintf(tmp_vid,"%s",dn->vid);
				//p_debug("tmp_vid=%s",tmp_vid);
				//JSON_ADD_OBJECT(tmp_obj,"vid",json_object_new_string(tmp_vid));
				JSON_ADD_OBJECT(tmp_obj,"vid",json_object_new_string(dn->vid));
				JSON_ADD_OBJECT(tmp_obj,"pid",json_object_new_string(dn->pid));
				if((needInfo[0] == '\0')||(needInfo[0] == '1')) {
					JObj *info_json=JSON_PARSE(dn->info);
					if(info_json==NULL)		JSON_ADD_OBJECT(tmp_obj,"info",JSON_NEW_EMPTY_OBJECT());
					else
						JSON_ADD_OBJECT(tmp_obj,"info",info_json);
				}
				JSON_ADD_OBJECT(tmp_obj,"completeTime",json_object_new_int(dn->update_task_time));
				sprintf(status,"%d",(dn)->download_status);
				JSON_ADD_OBJECT(tmp_obj,"status",json_object_new_string(status));
				//JSON_ADD_OBJECT(tmp_obj,"status",json_object_new_string((dn)->download_status));
				lu_pr=100;//((dn)->downloaded_size*100)/((dn)->total_size);
				sprintf(pr,"%.0f",lu_pr);
				JSON_ADD_OBJECT(tmp_obj,"pr",json_object_new_int((int)(lu_pr)));	

				sprintf(tag,"%s",dn->tag);
				//p_debug("tag=(%s)",tag);
				JObj *json_tag=JSON_PARSE(dn->tag);	
				if(json_tag==NULL)	{//tag 为空，返回info
					//JSON_ADD_OBJECT(tmp_obj,"tag",JSON_NEW_EMPTY_OBJECT());
					JSON_ADD_OBJECT(tmp_obj,"info",JSON_PARSE(dn->info));
				}
				else	JSON_ADD_OBJECT(tmp_obj,"tag",json_tag);
				
				JSON_ADD_OBJECT(tmp_obj,"progress",json_object_new_int(dn->downloaded_size));
				JSON_ADD_OBJECT(tmp_obj,"totalSize",json_object_new_int(dn->total_size));

				//p_debug("tmpobj===%s",JSON_TO_STRING(tmp_obj));
				JSON_ARRAY_ADD_OBJECT(my_array,tmp_obj);//add {vid,status,pr} to array
				//p_debug("%s",dn->vid);
			
			}	
			dn=(dn)->dn_next;
			//JSON_PUT_OBJECT(tmp_obj);
		}
		//p_debug("my_array===%s",JSON_TO_STRING(my_array));

		JSON_ADD_OBJECT(my_obj,"list",my_array);
		JSON_ADD_OBJECT(final_obj,"status",json_object_new_int(1));
		JSON_ADD_OBJECT(final_obj,"errorCode",json_object_new_int(0));		
		JSON_ADD_OBJECT(final_obj,"errorMessage",json_object_new_string("success"));				
		JSON_ADD_OBJECT(final_obj,"data",my_obj);

		//char *ssss
		all_buf=JSON_TO_STRING(final_obj);
		//c->loc.io.buf=JSON_TO_STRING(final_obj);
		//p_debug("getalltask===(%s),strlen=%d",c->loc.io.buf,strlen(c->loc.io.buf));
		//sprintf(c->loc.io.buf,"%s",JSON_TO_STRING(final_obj));
		//p_debug("strlen=%d",strlen(ssss));		

		//JSON_PUT_OBJECT(final_obj);//just need to free the final_obj

	}else if(type[0]=='2'&&(isAdmin==1)){//not downloaded videos
		JObj *my_array = JSON_NEW_ARRAY();
		JObj *my_obj = JSON_NEW_EMPTY_OBJECT();
		JObj *final_obj = JSON_NEW_EMPTY_OBJECT();
		for(i=0;;i++){
			if(!dn)break;
			if(dn->isDeleted==1) {
				dn=dn->dn_next;
				continue;
			}

			if((strcmp(tmp_pid,""))){// return the pid's vid info
				if(strcmp(dn->pid,tmp_pid))
					{
					dn=(dn)->dn_next;
					continue;
				}

			}	
			if(dn->download_status!=DONE){
				JObj *tmp_obj = JSON_NEW_EMPTY_OBJECT();//JSON_NEW_OBJECT(member,type);
				sprintf(tmp_vid,"%s",dn->vid);//p_debug("tmp_vid=%s",tmp_vid);
				//JSON_ADD_OBJECT(tmp_obj,"vid",json_object_new_string(tmp_vid));
				JSON_ADD_OBJECT(tmp_obj,"vid",json_object_new_string(dn->vid));
				JSON_ADD_OBJECT(tmp_obj,"pid",json_object_new_string(dn->pid));
				if((needInfo[0] == '\0')||(needInfo[0] == '1')) {
					JObj *info_json=JSON_PARSE(dn->info);
					if(info_json==NULL)		JSON_ADD_OBJECT(tmp_obj,"info",JSON_NEW_EMPTY_OBJECT());
					else
						JSON_ADD_OBJECT(tmp_obj,"info",info_json);
				}
				JSON_ADD_OBJECT(tmp_obj,"completeTime",json_object_new_int(dn->update_task_time));
				sprintf(status,"%d",(dn)->download_status);
				JSON_ADD_OBJECT(tmp_obj,"status",json_object_new_string(status));
				//JSON_ADD_OBJECT(tmp_obj,"status",json_object_new_string((dn)->download_status));
				if(dn->total_size!=0)lu_pr=(float)dn->downloaded_size/(float)dn->total_size*100;
				else lu_pr=0;;
				sprintf(pr,"%.0f",lu_pr);
				JSON_ADD_OBJECT(tmp_obj,"pr",json_object_new_int((int)(lu_pr)));
				//sprintf(tag,"%s",dn->tag);
				JObj *json_tag=JSON_PARSE(dn->tag);
				if(json_tag==NULL)	{//tag 为空，返回info
					//JSON_ADD_OBJECT(tmp_obj,"tag",JSON_NEW_EMPTY_OBJECT());
					//p_debug("tagggggggggggggg is null");
					JSON_ADD_OBJECT(tmp_obj,"info",JSON_PARSE(dn->info));
				}else{
					//p_debug("tagggggggggggggg is not null");
					//p_debug("tag=(%s)",JSON_TO_STRING(json_tag));
					JSON_ADD_OBJECT(tmp_obj,"tag",json_tag);
				}
				JSON_ADD_OBJECT(tmp_obj,"progress",json_object_new_int(dn->downloaded_size));				
				JSON_ADD_OBJECT(tmp_obj,"totalSize",json_object_new_int(dn->total_size));

				
				//p_debug("tmpobj===%s",JSON_TO_STRING(tmp_obj));
				JSON_ARRAY_ADD_OBJECT(my_array,tmp_obj);//add {vid,status,pr} to array
				//p_debug("%s",dn->vid);
				
			}
			dn=(dn)->dn_next;
			//JSON_PUT_OBJECT(tmp_obj);
		}
		//p_debug("my_array===%s",JSON_TO_STRING(my_array));

		JSON_ADD_OBJECT(my_obj,"list",my_array);
		JSON_ADD_OBJECT(final_obj,"status",json_object_new_int(1));
		JSON_ADD_OBJECT(final_obj,"errorCode",json_object_new_int(0));		
		JSON_ADD_OBJECT(final_obj,"errorMessage",json_object_new_string("success"));		
		JSON_ADD_OBJECT(final_obj,"data",my_obj);
		//char *ssss=JSON_TO_STRING(final_obj);
		//p_debug("getalltask===(%s),strlen=%d",ssss,strlen(ssss));
		all_buf=JSON_TO_STRING(final_obj);
		//		c->loc.io.buf=JSON_TO_STRING(final_obj);
		//p_debug("strlen=%d",strlen(ssss));		
		//JSON_PUT_OBJECT(final_obj);//just need to free the final_obj

	}else{
		ret=-1;
		p_debug("error type");
		//c->errorCode=4;
		//strcpy(c->error_msg,"type value error");	

		goto EXIT;
	}



EXIT:	
	//p_debug("c->loc.io.buf=%s",all_buf);
	//p_debug("leave Parser_GetAllTask");
	destory_task_list(task_dn);
return ret;

}
#if 1

void main()
{
		char ret_buf[RET_BUF_LEN];
		char code[CODE_LEN_65]="\0";
		char fw_code[CODE_LEN_65]="\0";		
		char sid[SID_LEN]="\0";
		char fw_sid[SID_LEN]="\0";


		char pid[PID_LEN_33]="\0";
		char tmp_buf[256]="\0";
		char tmp_vid[SID_LEN_33]="\0";
		int i,j,k;
		char *web_str=NULL;
		int ret=0;
		int isAdmin=0;
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
			fprintf(stdout,"can't get string from web\n");
			exit(1);
		}
		processString(web_str,SID,sid);		
		
		processString(web_str,CODE,code);		
		
		processString(web_str,"type",type);	

		//processString(web_str,PID,pid);	
		processString(web_str,PID,tmp_pid);	
		processString(web_str,"needInfo",needInfo);	
		
		p_debug("fw_sid=(%s)",fw_sid);
		p_debug("sid=(%s)",sid);

		p_debug("fw_code=(%s)",fw_code);
		p_debug("code=(%s)",code);
		p_debug("type=(%s)",type);
		//p_debug("needInfo=(%s)",needInfo);
		//p_debug("tmp_pid=(%s)",tmp_pid);

		if(!strcmp(sid,fw_sid)) 
			isAdmin=1;
		else
		if(!strcmp(code,fw_code)) 
			isAdmin=0;
		else isAdmin=2;
		
		if(isAdmin==1||isAdmin==0)
		{//是管理员或访客
		if(Parser_GetAllTask(isAdmin)<0)
			{
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"can't get it\"}");;
			printf("%s",ret_buf);
			//p_debug("ret_buf=%s",ret_buf);
			fflush(stdout);
			free(web_str);
			uci_free_context(ctx);
			return ;
		}
		
		/*
			//remove
			//if(strstr(vid,',')!=NULL)
			{
				if(needInfo[0] == '\0')sprintf(buf,"{\"%s\":{\"%s\":\"%s\",\"%s\":\"%s\"}}",TASK_GETALL,"type",type,PID,pid);
				else 	sprintf(buf,"{\"%s\":{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"}}",TASK_GETALL,"type",type,PID,pid,"needInfo",needInfo);
			}
			//else{
			//	sprintf(buf,"{\"%s\":{\"%s\":\"%s\"}}",TASK_REMOVE,VID,);
			//}
			p_debug("buf=====%s",buf);
		
			
			ret = notify_server();
			if(ret <= 0){//通讯错误
				sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":13,\"errorMessage\":\"Communication Error with dm_letv\"}");
			}else//接收到消息
			*/	//sprintf(ret_buf,"%s",buf);
				fprintf(stdout,all_buf);
				fflush(stdout);

				if(all_buf!=NULL)free(all_buf);
				free(web_str);
				uci_free_context(ctx);
				return ;
	
		}else{
			sprintf(ret_buf,"%s","{\"status\":0,\"errorCode\":1,\"errorMessage\":\"security error\",\"data\":{}}");
		} 

		printf("%s",ret_buf);
		//p_debug("ret_buf=%s",ret_buf);
		fflush(stdout);
		free(web_str);
		uci_free_context(ctx);
		return ;
}
#endif

#if 0
void main()
{
		char ret_buf[RET_BUF_LEN];
		char code[CODE_LEN_65]="\0";
		char fw_code[CODE_LEN_65]="\0";		
		char sid[SID_LEN]="\0";
		char fw_sid[SID_LEN]="\0";
		char type[32]="\0";
		char needInfo[32]="\0";
		char pid[PID_LEN_33]="\0";
		char tmp_buf[256]="\0";
		char tmp_vid[SID_LEN_33]="\0";
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
			fprintf(stdout,"can't get string from web\n");
			exit(1);
		}
		processString(web_str,SID,sid);		
		
		processString(web_str,CODE,code);		
		
		processString(web_str,"type",type);	

		processString(web_str,PID,pid);	
		
		processString(web_str,"needInfo",needInfo);	

		p_debug("fw_code=(%s)",fw_code);
		p_debug("code=(%s)",code);


		if(!strcmp(sid,fw_sid)||!strcmp(code,fw_code)){//是管理员
			//remove
			//if(strstr(vid,',')!=NULL)
			{
				if(needInfo[0] == '\0')sprintf(buf,"{\"%s\":{\"%s\":\"%s\",\"%s\":\"%s\"}}",TASK_GETALL,"type",type,PID,pid);
				else 	sprintf(buf,"{\"%s\":{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"}}",TASK_GETALL,"type",type,PID,pid,"needInfo",needInfo);
			}
			//else{
			//	sprintf(buf,"{\"%s\":{\"%s\":\"%s\"}}",TASK_REMOVE,VID,);
			//}
			p_debug("buf=====%s",buf);
		
			
			ret = notify_server();
			if(ret <= 0){//通讯错误
				sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":13,\"errorMessage\":\"Communication Error with dm_letv\"}");
			}else//接收到消息
				//sprintf(ret_buf,"%s",buf);
				fprintf(stdout,p_client_info.recv_buf);
				fflush(stdout);

				free(p_client_info.recv_buf);
				free(web_str);
				uci_free_context(ctx);
				return ;
	
		}else{
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"Not Admin\"}");
		} 

		printf("%s",ret_buf);
		//p_debug("ret_buf=%s",ret_buf);
		fflush(stdout);
		free(web_str);
		uci_free_context(ctx);
		return ;
}
#endif

