/*
 * =============================================================================
 *
 *       Filename:  file_json.c
 *
 *    Description:  process json data
 *
 *        Version:  1.0
 *        Created:  2015/04/03 14:33
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver, 
 *   Organization:  
 *
 * =============================================================================*/
#include <stdio.h>
#include "file_json.h"
#include "base.h"
#include "../http_client/http_client.h"
#include <pthread.h>
#include "uci_for_cgi.h"
#include "dm_sort.h"

typedef int (*API_FUNC)(struct conn *c);
typedef int (*PARSE_FUN)(struct conn *c);


typedef struct _file_tag_handle
{
 char tag[32];
 API_FUNC tagfun;
 PARSE_FUN parsefun;
}file_tag_handle;
/*
file_tag_handle all_file_handle[]=
{
	{FN_ADD_TASK,Letv_AddTask,Parser_AddTask},
	{FN_REMOVE_TASK,Letv_RemoveTask,Parser_RemoveTask},
	{FN_PAUSE_TASK,Letv_PauseTask,Parser_PauseTask},
	{FN_TOWAIT_TASK,Letv_ToWaitTask,Parser_ToWaitTask},
	{FN_START_TASK,Letv_StartTask,Parser_StartTask},
	{FN_GET_ALL_TASK,Letv_GetAllTask,Parser_GetAllTask},
	{FN_BY_STATUS_TASK,Letv_GetByStatusTask,Parser_GetByStatusTask},
	{FN_GET_NOW_PRO_TASK,Letv_GetNowProTask,Parser_GetNowProTask},
	{FN_GET_VERSION,Letv_GetVersion,Parser_GetVersion},
	{FN_GET_ALBUM,Letv_GetAlbum,Parser_GetAlbum},
	{FN_ADD_FOLLOW,Letv_AddFollow,Parser_AddFollow},
	{FN_DEL_FOLLOW,Letv_DelFollow,Parser_DelFollow},
	{FN_GET_ALL_FOLLOW,Letv_GetAllFollow,Parser_GetAllFollow},
	{FN_LETV_LOGIN,Letv_Login,Parser_Login},
	{FN_LETV_CLEAR,Letv_Clear,Parser_Clear},
};
*/
file_tag_handle all_file_handle[]=
{
	{CMD_ADD_TASK,Letv_AddTask,Parser_AddTask},
	{CMD_REMOVE_TASK,Letv_RemoveTask,Parser_RemoveTask},
	{CMD_PAUSE_TASK,Letv_PauseTask,Parser_PauseTask},
	{CMD_TOWAIT_TASK,Letv_ToWaitTask,Parser_ToWaitTask},
	{CMD_START_TASK,Letv_StartTask,Parser_StartTask},
	{CMD_GET_ALL_TASK,Letv_GetAllTask,Parser_GetAllTask},
	{CMD_BY_STATUS_TASK,Letv_GetByStatusTask,Parser_GetByStatusTask},
	{CMD_GET_NOW_PRO_TASK,Letv_GetNowProTask,Parser_GetNowProTask},
	{CMD_GET_ALBUM,Letv_GetAlbum,Parser_GetAlbum},
	{CMD_SET_ALBUM,Letv_SetAlbum,Parser_SetAlbum},	
	{CMD_ADD_FOLLOW,Letv_AddFollow,Parser_AddFollow},
	{CMD_DEL_FOLLOW,Letv_DelFollow,Parser_DelFollow},
	{CMD_GET_ALL_FOLLOW,Letv_GetAllFollow,Parser_GetAllFollow},
	{CMD_LETV_FILE_DOWNLOAD,Letv_fileDownload,fileDownload},
	{CMD_LETV_GET_VIDEO_INFO,Letv_GetVideoInfo,Parse_GetVideoInfo},
	{CMD_LETV_GET_VERSION,Letv_GetVersion,Parser_GetVersion},
	{CMD_LETV_TASK_UPDATE,Letv_TaskUpdate,Parser_TaskUpdate},
};


#define FILE_TAGHANDLE_NUM (sizeof(all_file_handle)/sizeof(all_file_handle[0]))
extern struct task_dnode *task_dn;

static void download_event(void *self)
{
    struct task_dnode *routerTask = (struct task_dnode *)self;
    p_debug("DM Downloading... (total:%lu, already:%lu,status:%d)", routerTask->total_size, routerTask->downloaded_size,routerTask->download_status);
    return ;
}
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

void updateVer(int flag){
	char set_str[128]={0};
	char tmp[128]={0};
	
	if(flag==0){

		sprintf(set_str,"sed -e \'/%s/d\' -i /tmp/state/status","unfinishedVer");
		system(set_str);
		memset(set_str,0,sizeof(set_str));
		
		sprintf(set_str,"echo \'%s=%d\' >> /tmp/state/status","unfinishedVer", time(NULL));
		system(set_str);
	
	}else if(flag==1){
		sprintf(set_str,"sed -e \'/%s/d\' -i /tmp/state/status","completedVer");
		system(set_str);
		memset(set_str,0,sizeof(set_str));
		
		sprintf(set_str,"echo \'%s=%d\' >> /tmp/state/status","completedVer", time(NULL));
		system(set_str);

	}else {
		sprintf(set_str,"sed -e \'/%s/d\' -i /tmp/state/status","unfinishedVer");
		system(set_str);
		memset(set_str,0,sizeof(set_str));
		
		sprintf(set_str,"echo \'%s=%d\' >> /tmp/state/status","unfinishedVer", time(NULL));
		system(set_str);
		
		sprintf(set_str,"sed -e \'/%s/d\' -i /tmp/state/status","completedVer");
		system(set_str);
		memset(set_str,0,sizeof(set_str));
		
		sprintf(set_str,"echo \'%s=%d\' >> /tmp/state/status","completedVer", time(NULL));
		system(set_str);
	
		
	}


}

int saveOneTaskToFile(struct task_dnode *dn){
	p_debug("saveOneTaskToFile");

	char path[256]={0};
	FILE *record_fd;
	int enRet;
//	struct task_dnode *cur=NULL;
	int ret = 0;
	int i = 0;
	if(dn!=NULL)//pclient->dn 可能为null 的情况
			p_debug("dn->vid=%s",dn->vid);
	else 
		return;
	if((dn->vid[0] != '\0')&&(IsInt(dn->vid))){
		p_debug("dn->downloaded_size=%lld",dn->downloaded_size);
		sprintf(path,"%s/%s.vid",LETV_DOWNLOAD_DIR_PATH,dn->vid);

		if(access(path,R_OK)!=0){// 不存在
			if((record_fd = fopen(path,"wb+"))==NULL)//改动：路径
			{
			 p_debug("task list file does not exist error1[errno:%d]",errno);
			 return -1;
			}
		}else  if((record_fd = fopen(path,"wb"))==NULL)//改动：路径
		{
			 p_debug("task list file does not exist error2[errno:%d]",errno);
			 return -1;
		}	

		
		int enRet = fwrite(dn,sizeof(struct task_dnode),1,record_fd);
		fclose(record_fd);
		if(enRet>0){		
			//system("cp /tmp/mnt/USB-disk-1/letv/.tasklist /tmp/mnt/USB-disk-1/letv/.tasklist.bak");
			//usleep(100000);
			p_debug("save one task ok");
			return 0;
		}
	}
	else{ 
			sprintf(path,"rm %s/%s.vid",LETV_DOWNLOAD_DIR_PATH,dn->vid);
			system(path);
			return -2;
	}

}
int Letv_AddTask(struct conn *c){
		p_debug("access Letv_AddTask");
		int ret = 0;
		struct task_dnode *cur;
		cur = xzalloc(sizeof(*cur));
		if(!cur)
		{
			ret=-1;
			p_debug("add dev error");
			goto FAIL;
		}
		strcpy(cur->pid,c->pid);
		strcpy(cur->vid,c->vid);
		if((cur->vid[0]=='\0')||(cur->pid[0]=='\0')){ 
			c->errorCode=7,
			strcpy(c->error_msg,"pid or vid is empty");
			ret =-1;
			goto FAIL;
	
		}
		strcpy(cur->ext,c->ext);
		strcpy(cur->tag,c->tag);
		p_debug("%s=======%s",cur->ext,c->tag);
		cur->total_size = c->total_size;
		cur->downloaded_size = c->downloaded_size;
		cur->download_status = c->download_status;
		if(c->img_url)
		{
			//cur->img_url = (char *)malloc(strlen(c->img_url) + 1);
			if(cur->img_url == NULL)
			{
				ret=-2;
				c->error = FILE_IS_NOT_EXIST;//下载地址为空
				p_debug("FILE_IS_NOT_EXIST");
				c->errorCode=7,
				strcpy(c->error_msg,"image url malloc error");
				safe_free(cur);
				goto FAIL;
			}
			strcpy(cur->img_url,c->img_url);
		}
		if(c->vid_url)
		{
			//cur->vid_url = (char *)malloc(strlen(c->vid_url) + 1);
			if(cur->vid_url == NULL)
			{
				ret=-3;
				c->error = FILE_IS_NOT_EXIST;//下载地址为空
				c->errorCode=7,
				strcpy(c->error_msg,"Video url malloc error");
				p_debug("FILE_IS_NOT_EXIST");
				safe_free(cur);
				goto FAIL;
			}
			strcpy(cur->vid_url,c->vid_url);
		}
		cur->isDeleted=0;
		cur->isAutoAdd=0;
		//cur->download_cb = download_event;
		pthread_rwlock_trywrlock(&task_list_lock); 
		ret=add_task_to_list(&task_dn,cur,1);
		pthread_rwlock_unlock(&task_list_lock);
	//	download_img_first=1;

		saveOneTaskToFile(cur);
		
		if(ret<0){
			safe_free(cur);
			c->errorCode=7,
			strcpy(c->error_msg,"Video url malloc error");
			goto FAIL;
		}else	
		if(ret==0){
			//memset(c->loc.io.buf,0,sizeof());
			//p_debug("^^^^^^^^^^^^^^^1");
			strcpy(c->loc.io.buf, "{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{}}");
		}else if(ret==1){//already exist
			strcpy(c->loc.io.buf, "{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{}}");
			safe_free(cur);
		}
		//p_debug("^^^^^^^^^^^^^^^2");
	
		updateVer(0);
		//p_debug("^^^^^^^^^^^^^^^3");	
		//ret = _handle_file_json_req(cur);
	FAIL:
		return ret;

}
#if 0
int Letv_AddTask(struct conn *c)
{
	p_debug("access Letv_AddTask");
	int ret = 0;
	struct task_dnode *cur;
	cur = xzalloc(sizeof(*cur));
	if(!cur)
	{
		ret=-1;
		p_debug("add dev error");
		goto FAIL;
	}
	strcpy(cur->pid,c->pid);
	strcpy(cur->vid,c->vid);
	if((cur->vid[0]=='\0')||(cur->pid[0]=='\0')){ 
		c->errorCode=7,
		strcpy(c->error_msg,"pid or vid is empty");
		ret =-1;
		goto FAIL;

	}
	strcpy(cur->ext,c->ext);
	strcpy(cur->tag,c->tag);
	p_debug("%s=======%s",cur->ext,c->tag);
	cur->total_size = c->total_size;
	cur->downloaded_size = c->downloaded_size;
	cur->download_status = c->download_status;
	if(c->img_url)
	{
		//cur->img_url = (char *)malloc(strlen(c->img_url) + 1);
		if(cur->img_url == NULL)
		{
			ret=-2;
			c->error = FILE_IS_NOT_EXIST;//下载地址为空
			p_debug("FILE_IS_NOT_EXIST");
			c->errorCode=7,
			strcpy(c->error_msg,"image url malloc error");
			safe_free(cur);
			goto FAIL;
		}
		strcpy(cur->img_url,c->img_url);
	}
	if(c->vid_url)
	{
		//cur->vid_url = (char *)malloc(strlen(c->vid_url) + 1);
		if(cur->vid_url == NULL)
		{
			ret=-3;
			c->error = FILE_IS_NOT_EXIST;//下载地址为空
			c->errorCode=7,
			strcpy(c->error_msg,"Video url malloc error");
			p_debug("FILE_IS_NOT_EXIST");
			safe_free(cur);
			goto FAIL;
		}
		strcpy(cur->vid_url,c->vid_url);
	}
	cur->isDeleted=0;
	cur->isAutoAdd=0;
	//cur->download_cb = download_event;
	pthread_rwlock_trywrlock(&task_list_lock); 
	ret=add_task_to_list(&task_dn,cur,1);
	pthread_rwlock_unlock(&task_list_lock);
	download_img_first=1;
	if(ret<0){
		safe_free(cur);
		c->errorCode=7,
		strcpy(c->error_msg,"Video url malloc error");
		goto FAIL;
	}else	
	if(ret==0){
		//memset(c->loc.io.buf,0,sizeof());
		//p_debug("^^^^^^^^^^^^^^^1");
		strcpy(c->loc.io.buf, "{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{}}");
	}else if(ret==1){//already exist
		strcpy(c->loc.io.buf, "{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{}}");
		safe_free(cur);
	}
	//p_debug("^^^^^^^^^^^^^^^2");

	updateVer(0);
	//p_debug("^^^^^^^^^^^^^^^3");	
	//ret = _handle_file_json_req(cur);
FAIL:
	return ret;

}
#endif
int Letv_RemoveTask(struct conn *c)
{
	updateVer(3);

	return 0;
}

int Letv_PauseTask(struct conn *c)
{
		strcpy(c->loc.io.buf, "{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{}}");
		updateVer(0);

		return 0;
}

int Letv_ToWaitTask(struct conn *c)
{
	strcpy(c->loc.io.buf, "{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{}}");
	updateVer(0);

return 0;

}
//从备份的配置文件重新读出下载的状态开始下载
int Letv_StartTask(struct conn *c)
{
	strcpy(c->loc.io.buf, "{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{}}");
	updateVer(0);
	return 0;

}

int Letv_GetAllTask(struct conn *c)
{
	return 1;
}

int Letv_GetByStatusTask(struct conn *c)
{
	return 1;
}

int Letv_GetVideoInfo(struct conn *c)
{
	return 1;
}

int Letv_GetNowProTask(struct conn *c)
{
	return 1;
}


int Letv_GetVersion(struct conn *c)
{
	return 1;
}

int Letv_TaskUpdate(struct conn *c)
{
	updateVer(3);
	return 1;
	
}

int Letv_GetAlbum(struct conn *c)
{
	return 1;
}

int Letv_SetAlbum(struct conn *c)
{
	return 1;
}

int Letv_AddFollow(struct conn *c)
{
	//strcpy(c->loc.io.buf, "{\"status\":1,\"data\":{}}");
	p_debug("Letv_AddFollow");
	flag_new_follow_add=1;
	return 0;
}

int Letv_DelFollow(struct conn *c)
{
	return 0;
}

int Letv_GetAllFollow(struct conn *c)
{
	return 1;
}

int Letv_Login(struct conn *c)
{
	return 0;
}

int Letv_Clear(struct conn *c)
{
	return 0;
}

int Letv_fileDownload(struct conn *c){
	return 1;
}


int file_process(struct conn *c)
{ 
/*
	uint8_t i = 0;
	uint8_t switch_flag = 0;
	int ret = 0;
	p_debug("c->cmd = %d",c->cmd);
	for(i = 0; i<FILE_TAGHANDLE_NUM; i++)
	{
		if(c->cmd == all_file_handle[i].tag)
		{
	       	 ret = all_file_handle[i].tagfun(c);
		     switch_flag = 1;
		}
	}
	if(switch_flag == 0)
    {
        c->error = REQUEST_FORMAT_ERROR;//命令无法识别
		p_debug("cmd not found");
    }else if(ret < 0)
    {
		c->error = REQUEST_FORMAT_ERROR;//命令的格式错误
	}	
	return ret;
	*/
	uint8_t i = 0;
	uint8_t switch_flag = 0;
	int ret = 0;
	p_debug("c->cmd = %s",c->cmd);
	for(i = 0; i<FILE_TAGHANDLE_NUM; i++)
	{
		if(!strcmp(c->cmd, all_file_handle[i].tag))
		{
	       	 ret = all_file_handle[i].tagfun(c);
		     switch_flag = 1;
			 break;
		}
	}
	
	if(switch_flag == 0)
    {
        c->errorCode = 4;//命令的格式错误
		strcpy(c->error_msg,"REQUEST_FORMAT_ERROR");
		p_debug("cmd not found");
		goto FAIL;
    }else if(ret < 0)
    {
		c->errorCode= 4;//命令的格式错误
		strcpy(c->error_msg,"REQUEST_FORMAT_ERROR");
		p_debug("REQUEST_FORMAT_ERROR");
		goto FAIL;
	}	

	if(ret==0)	{
		strcpy(c->loc.io.buf, "{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{}}");
		//p_debug("c->loc.io.buf=%s",c->loc.io.buf);
		goto SUCC;

	}else if(ret>0){
		goto SUCC;
	}
	

FAIL:
	sprintf(c->loc.io.buf, "{\"status\":0,\"data\":{},\"errorCode\":%d,\"errorMessage\":\"s\"}",c->errorCode,c->error_msg);
	c->loc.io.size = strlen(c->loc.io.buf);
	c->loc.io.head = c->loc.io.size;
	c->loc.io.tail = 0;
	c->loc.io.total = c->loc.io.size;
	return -1;
SUCC:
	
	c->loc.io.size = strlen(c->loc.io.buf);
	c->loc.io.head = c->loc.io.size;
	c->loc.io.tail = 0;
	c->loc.io.total = c->loc.io.size;
	return 0;

}

int Parser_AddTask(struct conn *c)
{
	int ret=0;
	char ext[256]="\0";
	p_debug("access Parser_AddTask");
	JObj *cmd_json = JSON_GET_OBJECT(c->r_json, CMD_ADD_TASK);
	if(cmd_json == NULL)
	{
		p_debug("cmd json null");
		c->error = 4;
		ret=-1;
		c->errorCode=4;
		strcpy(c->error_msg,"cmd json null");
		goto EXIT;
	}

	JObj *vid_json = JSON_GET_OBJECT(cmd_json, "vid");
	if(vid_json == NULL)
	{
		p_debug("vid json null");
		c->error = DM_ERROR_CMD_PARAMETER;
		ret=-2;
		c->errorCode=4;
		strcpy(c->error_msg,"vid json null");		
		goto EXIT;
	}

	JObj *ext_json = JSON_GET_OBJECT(cmd_json, "ext");
	if(ext_json == NULL)
	{
		p_debug("ext json null");
		strcpy(c->ext, "");
	/*	c->error = DM_ERROR_CMD_PARAMETER;
		ret=-2;
		errcode=4;
		strcpy(errmsg,"ext json null");		
		goto EXIT;
	*/
	}else {
		strcpy(ext,JSON_GET_OBJECT_VALUE(ext_json,string));
		escapeSpace(ext);
		sprintf(c->ext,"%s",ext);
		//p_debug("c->ext=%s",c->ext);
	}

	JObj *tag_json = JSON_GET_OBJECT(cmd_json, "tag");
	if(tag_json == NULL)
	{
		p_debug("tag json null");
		strcpy(c->tag, "");
	/*	c->error = DM_ERROR_CMD_PARAMETER;
		ret=-2;
		errcode=4;
		strcpy(errmsg,"ext json null");		
		goto EXIT;
	*/
	}else{
		strcpy(c->tag, JSON_GET_OBJECT_VALUE(tag_json,string));
	}


	JObj *pid_json = JSON_GET_OBJECT(cmd_json, "pid");
	if(pid_json == NULL)
	{
		p_debug("pid json null");
		ret=-3;
		c->errorCode=4;
		strcpy(c->error_msg,"pid json null");
		c->error = DM_ERROR_CMD_PARAMETER;
		goto EXIT;
	}
	if(vid_json != NULL)
		strcpy(c->vid, JSON_GET_OBJECT_VALUE(vid_json,string));
	if(pid_json != NULL)
		strcpy(c->pid, JSON_GET_OBJECT_VALUE(pid_json,string));


	
	p_debug("vid = %s,pid = %s,ext=%s,tag=%s", c->vid, c->pid,c->ext,c->tag);

	if((!strcmp(c->vid,""))||(!strcmp(c->pid,"")))
		{
			ret=-4;
			c->errorCode=4;
			strcpy(c->error_msg,"vid or pid is empty");
			c->error = DM_ERROR_CMD_PARAMETER;
			goto EXIT;
	}
EXIT:	
	return ret;
}

int Parser_RemoveTask(struct conn *c)
{
	int ret=0;
	int array_len=0;
	int j,i=0;
	char vid[33]="\0";
	char tmp_vid[33]="\0";	
	char del_file_path[128]="\0";
	p_debug("access Parser_RemoveTask");
	JObj *cmd_json = JSON_GET_OBJECT(c->r_json, CMD_REMOVE_TASK);
	if(cmd_json == NULL)
	{
		p_debug("cmd json null");
		c->error = REQUEST_FORMAT_ERROR;
		ret=-1;
		errcode=4;
		strcpy(errmsg,"cmd json null");
		goto EXIT;
	}

	JObj *vid_json = JSON_GET_OBJECT(cmd_json, "vid");
	if(vid_json == NULL)
	{
		p_debug("vid json null");
		c->error = DM_ERROR_CMD_PARAMETER;
		ret=-2;
		errcode=4;
		strcpy(errmsg,"vid json null"); 	
		goto EXIT;
	}

	array_len =JSON_GET_ARRAY_LEN(vid_json);
	if(array_len==0){
		ret=-4;
		errcode=4;
		strcpy(errmsg,"vid or pid is empty");
		c->error = DM_ERROR_CMD_PARAMETER;
		goto EXIT;

	}

	JObj *medi_array_obj;
	if(vid_json != NULL){
		for (i = 0; i < array_len; i++) {
		  // get the i-th object in medi_array
		  //p_debug("iiiiiiiiiiiiiiii=%d",i);

		  medi_array_obj = json_object_array_get_idx(vid_json, i);
		  strcpy(vid, json_object_to_json_string(medi_array_obj));
		 //p_debug("++++++++++++vid=%s++++++++++++++++",vid);
	
		 strncpy(tmp_vid,vid+1,strlen(vid)-2);
		 //p_debug("++++++++++++tmp_vid=%s++++++++++++++++",tmp_vid);
		 
		 pthread_rwlock_trywrlock(&task_list_lock); 

		 ret=update_task_status_to_list(NULL,tmp_vid,PAUSE);//stop it first.
		 if(ret!=0)break;//some may not find or deleted , break here.

		 ret=del_task_from_list(&task_dn,NULL,tmp_vid);
		 pthread_rwlock_unlock(&task_list_lock);
		 
		 sprintf(del_file_path,"rm /tmp/mnt/USB-disk-1/hack/%s.mp4",tmp_vid);
 		 system(del_file_path);
		 sprintf(del_file_path,"rm /tmp/mnt/USB-disk-1/hack/%s.jpg",tmp_vid);
 		 system(del_file_path);		 

		 if(ret!=0)break;//some may not find or deleted , break here.
		}
	}
	if(i==(array_len-1)){
		ret=0;
	}
	
EXIT:	
	return ret;

}

int Parser_PauseTask(struct conn *c)
{
		int ret=0;
		int array_len=0;
		int j,i=0;
		char vid[33]="\0";
		char tmp_vid[33]="\0";

		p_debug("access Parser_PauseTask");
		JObj *cmd_json = JSON_GET_OBJECT(c->r_json, CMD_PAUSE_TASK);
		if(cmd_json == NULL)
		{
			p_debug("cmd json null");
			c->error = REQUEST_FORMAT_ERROR;
			ret=-1;
			errcode=4;
			strcpy(errmsg,"cmd json null");
			goto EXIT;
		}
	
		JObj *vid_json = JSON_GET_OBJECT(cmd_json, "vid");
		if(vid_json == NULL)
		{
			p_debug("vid json null");
			c->error = DM_ERROR_CMD_PARAMETER;
			ret=-2;
			errcode=4;
			strcpy(errmsg,"vid json null"); 	
			goto EXIT;
		}
		array_len =JSON_GET_ARRAY_LEN(vid_json);
		if(array_len==0){
			ret=-4;
			errcode=4;
			strcpy(errmsg,"vid or pid is empty");
			c->error = DM_ERROR_CMD_PARAMETER;
			goto EXIT;

		}

		JObj *medi_array_obj;
		if(vid_json != NULL){
			for (i = 0; i < array_len; i++) {
			  // get the i-th object in medi_array
			  medi_array_obj = json_object_array_get_idx(vid_json, i);
	          strcpy(vid, json_object_to_json_string(medi_array_obj));
			 //p_debug("++++++++++++vid=%s++++++++++++++++",vid);

			 strncpy(tmp_vid,vid+1,strlen(vid)-2);
			 p_debug("i=%d++++++++++++tmp_vid=%s++++++++++++++++",i,tmp_vid);
			 ret=update_task_status_to_list(NULL,tmp_vid,PAUSE);
			 if(ret!=0)break;//some may not find or changed, break here.
			}
		}
		if(array_len>1){
			//system("mcu_control -s 1");
			system("pwm_control 1 1 0;pwm_control 1 0 0");
			updateSysVal("led_status","1");

		}
		if(i==(array_len-1)){
			ret=0;
		}

	EXIT:	
		return ret;

}

int Parser_ToWaitTask(struct conn *c)
{
		int ret=0;
		int array_len=0;
		int j,i=0;
		char vid[33]="\0";
		char tmp_vid[33]="\0";

		p_debug("access Parser_ToWaitTask");
		JObj *cmd_json = JSON_GET_OBJECT(c->r_json, CMD_TOWAIT_TASK);
		if(cmd_json == NULL)
		{
			p_debug("cmd json null");
			c->error = REQUEST_FORMAT_ERROR;
			ret=-1;
			errcode=4;
			strcpy(errmsg,"cmd json null");
			goto EXIT;
		}
	
		JObj *vid_json = JSON_GET_OBJECT(cmd_json, "vid");
		if(vid_json == NULL)
		{
			p_debug("vid json null");
			c->error = DM_ERROR_CMD_PARAMETER;
			ret=-2;
			errcode=4;
			strcpy(errmsg,"vid json null"); 	
			goto EXIT;
		}
		array_len =JSON_GET_ARRAY_LEN(vid_json);
		if(array_len==0){
			ret=-4;
			errcode=4;
			strcpy(errmsg,"vid or pid is empty");
			c->error = DM_ERROR_CMD_PARAMETER;
			goto EXIT;

		}

		JObj *medi_array_obj;
		if(vid_json != NULL){
			for (i = 0; i < array_len; i++) {
			  // get the i-th object in medi_array
			  medi_array_obj = json_object_array_get_idx(vid_json, i);
	          strcpy(vid, json_object_to_json_string(medi_array_obj));
			 //p_debug("++++++++++++vid=%s++++++++++++++++",vid);

			 strncpy(tmp_vid,vid+1,strlen(vid)-2);
			 //p_debug("++++++++++++tmp_vid=%s++++++++++++++++",tmp_vid);
			 ret=update_task_status_to_list(NULL,tmp_vid,WAITING);
			 //if(ret!=0)break;//some may not find or changed, break here.
			}
		}
		if(i==(array_len-1)){
			ret=0;
		}

	EXIT:	
		return ret;

}

int Parser_StartTask(struct conn *c)
{
		int ret=0;
		int array_len=0;
		int j,i=0;
		char vid[33]="\0";
		char tmp_vid[33]="\0";
		struct task_dnode *dn;
		dn=task_dn;
		p_debug("access Parser_StartTask");
		JObj *cmd_json = JSON_GET_OBJECT(c->r_json, CMD_START_TASK);
		if(cmd_json == NULL)
		{
			p_debug("cmd json null");
			c->error = REQUEST_FORMAT_ERROR;
			ret=-1;
			errcode=4;
			strcpy(errmsg,"cmd json null");
			goto EXIT;
		}
	
		JObj *vid_json = JSON_GET_OBJECT(cmd_json, "vid");
		if(vid_json == NULL)
		{
			p_debug("vid json null");
			c->error = DM_ERROR_CMD_PARAMETER;
			ret=-2;
			errcode=4;
			strcpy(errmsg,"vid json null"); 	
			goto EXIT;
		}
		array_len =JSON_GET_ARRAY_LEN(vid_json);
		if(array_len==0){
			ret=-4;
			errcode=4;
			strcpy(errmsg,"vid or pid is empty");
			c->error = DM_ERROR_CMD_PARAMETER;
			goto EXIT;

		}

		JObj *medi_array_obj;
		if(vid_json != NULL){

			for(i=0;;i++){// STOP the downloading ones
				if(!dn)break;
				if(dn->download_status==DOWNLOADING)
					ret=update_task_status_to_list(NULL,dn->vid,WAITING);		
				dn = dn->dn_next;
			}
			for (i = 0; i < array_len; i++) {
			  // get the i-th object in medi_array
			  medi_array_obj = json_object_array_get_idx(vid_json, i);
	          strcpy(vid, json_object_to_json_string(medi_array_obj));
			 //p_debug("++++++++++++vid=%s++++++++++++++++",vid);

			 strncpy(tmp_vid,vid+1,strlen(vid)-2);
			 //p_debug("++++++++++++tmp_vid=%s++++++++++++++++",tmp_vid);
			 ret=update_task_status_to_list(NULL,tmp_vid,DOWNLOADING);
			 if(ret!=0){
			 	c->errorCode=4;
			 	sprintf(c->error_msg,"%s","cmd execute failed,please check vid.");
				break;//some may not find or changed, break here.
			 	}
			}
		}
		if((i==(array_len-1))&&(ret==0)){
			ret=0;
		}

EXIT:	
	return ret;


}

int Parser_GetAllTask(struct conn *c)
{
	int ret=0;
	int array_len=0;
	int j,i=0;
	char type[8]="\0";
	char needInfo[8]="\0";
	char tmp_vid[33]="\0";
	char tmp_pid[33]="\0";

	p_debug("access Parser_GetAllTask");
	JObj *cmd_json = JSON_GET_OBJECT(c->r_json, CMD_GET_ALL_TASK);
	if(cmd_json == NULL)
	{
		p_debug("cmd json null");
		c->error = REQUEST_FORMAT_ERROR;
		ret=-1;
		c->errorCode=4;
		strcpy(c->error_msg,"cmd json null");
		goto EXIT;
	}

	JObj *type_json = JSON_GET_OBJECT(cmd_json, "type");
	if(type_json == NULL)
	{
		p_debug("type json null");
		c->error = DM_ERROR_CMD_PARAMETER;
		ret=-2;
		c->errorCode=4;
		strcpy(c->error_msg,"type json null");		
	
		goto EXIT;
	}else
		strcpy(type, JSON_GET_OBJECT_VALUE(type_json,string));

	JObj *pid_json = JSON_GET_OBJECT(cmd_json, "pid");
	if(pid_json == NULL)
	{
		//question here.
	}else{
		strcpy(tmp_pid, JSON_GET_OBJECT_VALUE(pid_json,string));
		p_debug("get pid==%s==",tmp_pid);
	}

	JObj *need_json = JSON_GET_OBJECT(cmd_json, "needInfo");
	if(need_json == NULL)
	{
		//question here.
	}else{
		strcpy(needInfo, JSON_GET_OBJECT_VALUE(need_json,string));
	}



	char status[8]="\0";
		char pr[8]="\0";
	char tag[512]="\0";
	float lu_pr;
	struct task_dnode *dn;
	dn=task_dn;
	if(type[0]=='0'){//all videos
		JObj *my_array = JSON_NEW_ARRAY();
		JObj *my_obj = JSON_NEW_EMPTY_OBJECT();
		JObj *final_obj = JSON_NEW_EMPTY_OBJECT();

		for(i=0;;i++){
				
			if(!dn)break;
			if(dn->isDeleted==1) {
				dn=dn->dn_next;
				continue;
			}

			if((pid_json != NULL)&&(strcmp(tmp_pid,""))){// return the pid's vid info
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
			
			JSON_ADD_OBJECT(tmp_obj,"pr",json_object_new_string(pr));


			sprintf(tag,"%s",dn->tag);
			p_debug("tag len=%d",strlen(tag));
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
			p_debug("%s",dn->vid);

			dn=(dn)->dn_next;
		
			//JSON_PUT_OBJECT(tmp_obj);
		}
		//p_debug("my_array===%s",JSON_TO_STRING(my_array));

		JSON_ADD_OBJECT(my_obj,"list",my_array);
		JSON_ADD_OBJECT(final_obj,"data",my_obj);
		JSON_ADD_OBJECT(final_obj,"status",json_object_new_int(1));
		//p_debug("getalltask===%s",JSON_TO_STRING(final_obj));
		c->loc.io.buf=JSON_TO_STRING(final_obj);
		//sprintf(c->loc.io.buf,"%s",);
		//p_debug("c->loc.io.buf=====%s",c->loc.io.buf);
		//JSON_PUT_OBJECT(final_obj);//just need to free the final_obj
	}else if(type[0]=='1'){//already downloaded videos
		JObj *my_array = JSON_NEW_ARRAY();
		JObj *my_obj = JSON_NEW_EMPTY_OBJECT();
		JObj *final_obj = JSON_NEW_EMPTY_OBJECT();
		for(i=0;;i++){
			if(!dn)break;
			if(dn->isDeleted==1) {
				dn=dn->dn_next;
				continue;
			}

			if((pid_json != NULL)&&(strcmp(tmp_pid,""))){// return the pid's vid info
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
				JSON_ADD_OBJECT(tmp_obj,"pr",json_object_new_string(pr));	

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
		JSON_ADD_OBJECT(final_obj,"data",my_obj);
		JSON_ADD_OBJECT(final_obj,"status",json_object_new_int(1));
		//char *ssss
		c->loc.io.buf=JSON_TO_STRING(final_obj);
		//p_debug("getalltask===(%s),strlen=%d",c->loc.io.buf,strlen(c->loc.io.buf));
		//sprintf(c->loc.io.buf,"%s",JSON_TO_STRING(final_obj));
		//p_debug("strlen=%d",strlen(ssss));		

		//JSON_PUT_OBJECT(final_obj);//just need to free the final_obj

	}else if(type[0]=='2'){//not downloaded videos
		JObj *my_array = JSON_NEW_ARRAY();
		JObj *my_obj = JSON_NEW_EMPTY_OBJECT();
		JObj *final_obj = JSON_NEW_EMPTY_OBJECT();
		for(i=0;;i++){
			if(!dn)break;
			if(dn->isDeleted==1) {
				dn=dn->dn_next;
				continue;
			}

			if((pid_json != NULL)&&(strcmp(tmp_pid,""))){// return the pid's vid info
				if(strcmp(dn->pid,tmp_pid))
					{
					dn=(dn)->dn_next;
					continue;
				}

			}	
			if(dn->download_status!=DONE){
				JObj *tmp_obj = JSON_NEW_EMPTY_OBJECT();//JSON_NEW_OBJECT(member,type);
				sprintf(tmp_vid,"%s",dn->vid);p_debug("tmp_vid=%s",tmp_vid);
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
				JSON_ADD_OBJECT(tmp_obj,"pr",json_object_new_string(pr));
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
		JSON_ADD_OBJECT(final_obj,"data",my_obj);
		JSON_ADD_OBJECT(final_obj,"status",json_object_new_int(1));
		//char *ssss=JSON_TO_STRING(final_obj);
		//p_debug("getalltask===(%s),strlen=%d",ssss,strlen(ssss));
		c->loc.io.buf=JSON_TO_STRING(final_obj);
		//p_debug("strlen=%d",strlen(ssss));		
		//JSON_PUT_OBJECT(final_obj);//just need to free the final_obj

	}else{
		ret=-1;
		
		c->errorCode=4;
		strcpy(c->error_msg,"type value error");	

		goto EXIT;
	}



EXIT:	
	//p_debug("c->loc.io.buf=%s",c->loc.io.buf);
	p_debug("leave Parser_GetAllTask");
return ret;

}

int Parser_GetByStatusTask(struct conn *c)
{

	int ret=0;
	int array_len=0;
	int j,i=0;
	int flag=0;
	char vid[33]="\0";
	char tmp_vid[33]="\0";
	struct task_dnode *dn;
	pthread_rwlock_trywrlock(&task_list_lock); 

	dn=task_dn;
	p_debug("access Parser_StartTask");
	JObj *cmd_json = JSON_GET_OBJECT(c->r_json, CMD_BY_STATUS_TASK);
	if(cmd_json == NULL)
	{
		p_debug("cmd json null");
		c->error = REQUEST_FORMAT_ERROR;
		ret=-1;
		errcode=4;
		strcpy(errmsg,"cmd json null");
		goto EXIT;
	}

	JObj *vid_json = JSON_GET_OBJECT(cmd_json, "vid");
	if(vid_json == NULL)
	{
		p_debug("vid json null");
		c->error = DM_ERROR_CMD_PARAMETER;
		ret=-2;
		errcode=4;
		strcpy(errmsg,"vid json null"); 	
		goto EXIT;
	}
	array_len =JSON_GET_ARRAY_LEN(vid_json);
	if(array_len==0){
		ret=-4;
		errcode=4;
		strcpy(errmsg,"vid or pid is empty");
		c->error = DM_ERROR_CMD_PARAMETER;
		goto EXIT;

	}

	JObj *medi_array_obj;
	if(vid_json != NULL){
		for (i = 0; i < array_len; i++) {
		  // get the i-th object in medi_array
		  medi_array_obj = json_object_array_get_idx(vid_json, i);
		  strcpy(vid, json_object_to_json_string(medi_array_obj));
		 //p_debug("++++++++++++vid=%s++++++++++++++++",vid);
		 strncpy(tmp_vid,vid+1,strlen(vid)-2);
		 //p_debug("++++++++++++tmp_vid=%s++++++++++++++++",tmp_vid);
		 //return the state
			
		 for(j=0;;j++){
		 	if(!dn)break;
			if(dn->isDeleted==1) {
				dn=dn->dn_next;
				continue;
			}

			if(!strcmp(dn->vid,tmp_vid)){
				flag=1;
				sprintf(c->loc.io.buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"task\":{\"vid\":\"%s\",\"downloadState\":%d,\"pr\":%.0f,\"progress\":%lld,\"totalSize\":%lld}}}",dn->vid,dn->download_status,(float)dn->downloaded_size/(float)dn->total_size*100,dn->downloaded_size,dn->total_size);
				break;
			}
			dn=dn->dn_next;
		 }
		 if(flag==0){
			 sprintf(c->loc.io.buf,"{\"status\":0,\"data\":{},\"errorCode\":4,\"errorMessage\":\"%s\"}","Can't find the vid");			 
		 }
		}
	}


EXIT:	
	pthread_rwlock_unlock(&task_list_lock);
	return ret;
	
}

int Parser_GetNowProTask(struct conn *c)
{
	int ret=0;
	int array_len=0;
	int j=0;
	int i=0;
	char vid[33]="\0";
	char tmp_vid[33]="\0";
	struct task_dnode *dn;
	dn=task_dn;
	p_debug("access Parser_StartTask");
	JObj *cmd_json = JSON_GET_OBJECT(c->r_json, CMD_GET_NOW_PRO_TASK);
	if(cmd_json == NULL)
	{
		p_debug("cmd json null");
		c->error = REQUEST_FORMAT_ERROR;
		ret=-1;
		errcode=4;
		strcpy(errmsg,"cmd json null");
		goto EXIT;
	}else {
		for(i=0;;i++){
			if(!dn)break;
			if(dn->isDeleted==1) {
				dn=dn->dn_next;
				continue;
			}
			if(dn->download_status==DOWNLOADING){
				//return the status
				j=1;
				sprintf(c->loc.io.buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"task\":{\"vid\":\"%s\",\"pr\":\"%.0f\"}}}",dn->vid,(float)dn->downloaded_size/(float)dn->total_size*100);
				break;
			}
			dn=dn->dn_next;
		}
		if(j!=1){// no downloading ones
			sprintf(c->loc.io.buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"task\":{\"vid\":\"\",\"pr\":\"\"}}}");
		}
	}
	
EXIT:	
return ret;
}


int Parse_GetVideoInfo(struct conn *c){
	int ret=0;
	int array_len=0;
	int j,i=0;
	char vid[33]="\0";
	char tmp_vid[33]="\0";

	p_debug("access Parse_GetVideoInfo");
	JObj *cmd_json = JSON_GET_OBJECT(c->r_json, CMD_LETV_GET_VIDEO_INFO);
	if(cmd_json == NULL)
	{
		p_debug("cmd json null");
		c->error = REQUEST_FORMAT_ERROR;
		ret=-1;
		errcode=4;
		strcpy(errmsg,"cmd json null");
		goto EXIT;
	}

	JObj *vid_json = JSON_GET_OBJECT(cmd_json, "vid");
	if(vid_json == NULL)
	{
		p_debug("vid json null");
		c->error = DM_ERROR_CMD_PARAMETER;
		ret=-2;
		errcode=4;
		strcpy(errmsg,"vid json null"); 	
		goto EXIT;
	}
	array_len =JSON_GET_ARRAY_LEN(vid_json);
	if(array_len==0){
		ret=-4;
		errcode=4;
		strcpy(errmsg,"vid or pid is empty");
		c->error = DM_ERROR_CMD_PARAMETER;
		goto EXIT;

	}

	JObj *medi_array_obj;
	if(vid_json != NULL){
		JObj *list=JSON_NEW_ARRAY();
		for (i = 0; i < array_len; i++) {
		  // get the i-th object in medi_array
		  medi_array_obj = json_object_array_get_idx(vid_json, i);
          strcpy(vid, json_object_to_json_string(medi_array_obj));
		 //p_debug("++++++++++++vid=%s++++++++++++++++",vid);

		 strncpy(tmp_vid,vid+1,strlen(vid)-2);
		 //p_debug("++++++++++++tmp_vid=%s++++++++++++++++",tmp_vid);
		struct task_dnode *tmp_node=task_dn;
		for(j=0;;j++){
			if(!tmp_node)break;
			if(tmp_node->isDeleted==1){
				tmp_node=tmp_node->dn_next;
				continue;
			}
			if(!strcmp(tmp_node->vid,tmp_vid)){
				break;
			}
			tmp_node=tmp_node->dn_next;
		}
		JObj *list_member=JSON_NEW_EMPTY_OBJECT();
		if(tmp_node!=NULL)
		{
			JSON_ADD_OBJECT(list_member,"vid",json_object_new_string(tmp_node->vid));
			JSON_ADD_OBJECT(list_member,"info",JSON_PARSE(tmp_node->info));
		}else{
			JSON_ADD_OBJECT(list_member,"vid",json_object_new_string(tmp_vid));
			JSON_ADD_OBJECT(list_member,"info",json_object_new_string("not found"));
		}
		JSON_ARRAY_ADD_OBJECT(list,list_member);

		p_debug("list=%s",JSON_TO_STRING(list));		
		}
		JObj *list_obj=JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(list_obj,"list",list);
		
		JObj *reply=JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(reply,"data",list_obj);

		JSON_ADD_OBJECT(reply,"status",json_object_new_int(1));
		p_debug("reply=%s",JSON_TO_STRING(reply));

		c->loc.io.buf=JSON_TO_STRING(reply);
		
		//JSON_PUT_OBJECT(reply);//just need to free the final_obj
	}

	if(i==(array_len-1)){
		ret=0;
	}

EXIT:	
	return ret;

}
int get_conf_str2(char *dest,char *var)
{
	FILE *fp=fopen("/tmp/state/status","r");
	if(NULL == fp)
	{
		//printf("open /etc/config/nrender.conf failed \n");
		return 0;
	}
	char tmp[128];
	char *ret_str;
	bzero(tmp,128);
	while(fgets(tmp,128,fp)!=NULL)
	{
		if('\n'==tmp[strlen(tmp)-1])
		{
			tmp[strlen(tmp)-1]=0;
		}
		//printf("get string from /etc/config/nrender.conf:%s\n",tmp);
		if(!strncmp(var,tmp,strlen(var)))
		{
			ret_str = malloc(strlen(tmp)-strlen(var));
			if(!ret_str)
			{
				fclose(fp);
				return 0;
			}
			bzero(ret_str,strlen(tmp)-strlen(var));
			strcpy(ret_str,tmp+strlen(var)+1);
			
			//printf("ret string:%s\n",ret_str);
			fclose(fp);
			strcpy(dest,ret_str);
			free(ret_str);
			return 0;
		}
		
	}
	fclose(fp);
	return 0;
}

int Parser_GetVersion(struct conn *c)
{
	char uci_option_str[64]="\0";
	char uVer[32]="\0";
	char fVer[32]="\0";
	//get_conf_str(uVer,"unfinishedVer");
	//get_conf_str(fVer,"completedVer");	

#if	0
	ctx=uci_alloc_context();
	memset(uci_option_str,'\0',64);	
	strcpy(uci_option_str,"system.@system[0].unfinishedVer"); 		
	uci_get_option_value(uci_option_str,uVer);


	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"system.@system[0].finishedVer"); 		
	uci_get_option_value(uci_option_str,fVer);

	sprintf(c->loc.io.buf,"{\"status\":1,\"data\":{\"unFinishedVer\":\"%s\",\"completedVer\":\"%s\"}}",uVer,fVer);
	uci_free_context(ctx);
#endif
	sprintf(c->loc.io.buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"unFinishedVer\":\"%s\",\"completedVer\":\"%s\"}}",uVer,fVer);

	return 0;
}

int Parser_TaskUpdate(struct conn *c){
	p_debug("access Parser_TaskUpdate");
	int array_len=0;
	char vid[33]="\0";
	char tmp_vid[33]="\0";	
	int i,j;
	int ret=0;
	JObj *cmd_json = JSON_GET_OBJECT(c->r_json, CMD_LETV_TASK_UPDATE);
	if(cmd_json == NULL)
	{
		p_debug("cmd json null");
		c->error = REQUEST_FORMAT_ERROR;
		ret=-1;
		c->errorCode=4;
		strcpy(c->error_msg,"cmd json null");
		goto EXIT;
	}
	
	JObj *tag_json = JSON_GET_OBJECT(cmd_json, "tag");
	if(tag_json == NULL)
	{
		p_debug("tag json null");
		c->error = REQUEST_FORMAT_ERROR;
		ret=-1;
		c->errorCode=4;
		strcpy(c->error_msg,"tag json null");
		goto EXIT;

	}

	array_len =JSON_GET_ARRAY_LEN(tag_json);
	for (i = 0; i < array_len; i++) {
			  // get the i-th object in medi_array
		JObj *medi_array_obj = json_object_array_get_idx(tag_json, i);
		JObj *vid_json=JSON_GET_OBJECT(medi_array_obj, "vid");
		strcpy(vid, json_object_to_json_string(vid_json));
		//p_debug("vid=%s",vid);
		strncpy(tmp_vid,vid+1,strlen(vid)-2);

		if(strcmp(tmp_vid,"")){
			struct task_dnode *dn=task_dn;
			for(j=0;;j++){
				if(!dn)break;
				//p_debug("tmp_vid=%s,dn->vid=%s",tmp_vid,dn->vid);
				if(!strcmp(dn->vid,tmp_vid))
					{
					JObj *tmp_tag_json=JSON_GET_OBJECT(medi_array_obj, "tag");			
					strcpy(dn->tag,json_object_to_json_string(tmp_tag_json));
					update_task_to_list(dn);
					p_debug("dn->tag=%s",dn->tag);
					break;
				}
				dn=dn->dn_next;
			}


		}else{
			ret=-1;
			c->errorCode=4;
			strcpy(c->error_msg,"tag json null");
			goto EXIT;		
		}
	}
	sprintf(c->loc.io.buf,"%s","{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{}}");


EXIT:	
	return ret;
}


int Parser_GetAlbum(struct conn *c)
{
		int ret=0;
		int array_len=0;
		int j,i=0;
		char pid[33]="\0";
		char tmp_pid[33]="\0";
		struct album_node*album;
		album=album_dn;
		p_debug("access Parser_StartTask");
		JObj *cmd_json = JSON_GET_OBJECT(c->r_json, CMD_GET_ALBUM);
		if(cmd_json == NULL)
		{
			p_debug("cmd json null");
			c->error = REQUEST_FORMAT_ERROR;
			ret=-1;
			c->errorCode=4;
			strcpy(c->error_msg,"cmd json null");
			goto EXIT;
		}
	
		JObj *pid_json = JSON_GET_OBJECT(cmd_json, "pid");
		if(pid_json == NULL)
		{
			p_debug("pid json null");
			c->error = DM_ERROR_CMD_PARAMETER;
			ret=-2;
			c->errorCode=4;
			strcpy(c->error_msg,"pid json null"); 	
			goto EXIT;
		}
		array_len =JSON_GET_ARRAY_LEN(pid_json);
		if(array_len==0){
			ret=-4;
			c->errorCode=4;
			strcpy(c->error_msg,"pid is empty");
			c->error = DM_ERROR_CMD_PARAMETER;
			goto EXIT;
	
		}
	
		JObj *medi_array_obj;
		if(pid_json != NULL){
			for (i = 0; i < array_len; i++) {
			  // get the i-th object in medi_array
			 medi_array_obj = json_object_array_get_idx(pid_json, i);
			 strcpy(pid, json_object_to_json_string(medi_array_obj));
			 //p_debug("++++++++++++pid=%s++++++++++++++++",pid);
			 strncpy(tmp_pid,pid+1,strlen(pid)-2);
			 //p_debug("++++++++++++tmp_pid=%s++++++++++++++++",tmp_pid);
			 //return the state
			 for(j=0;;j++){
				if(!album)break;
				if(!strcmp(album->pid,tmp_pid)){
					p_debug("find it,info=%s",album->info);
					sprintf(c->loc.io.buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"info\":%s,\"tag\":%s}}",album->info,album->tag2);
					return ret;
					//break;
				}
				album=album->dn_next;
			 }
			c->errorCode=8;
			strcpy(c->error_msg,"pid is not found.");
			ret=-5;
			}
		}
	
	
	EXIT:	
		return ret;

}



int Parser_SetAlbum(struct conn *c)
{
		int ret=0;
		int array_len=0;
		int j,i=0;
		char pid[33]="\0";
		char tmp_pid[33]="\0";
		char tag[512]="\0";
		struct album_node*album;
		album=album_dn;
		p_debug("access Parser_StartTask");
		JObj *cmd_json = JSON_GET_OBJECT(c->r_json, CMD_SET_ALBUM);
		if(cmd_json == NULL)
		{
			p_debug("cmd json null");
			c->error = REQUEST_FORMAT_ERROR;
			ret=-1;
			c->errorCode=4;
			strcpy(c->error_msg,"cmd json null");
			goto EXIT;
		}
	
		JObj *pid_json = JSON_GET_OBJECT(cmd_json, "pid");
		if(pid_json == NULL)
		{
			p_debug("pid json null");
			c->error = DM_ERROR_CMD_PARAMETER;
			ret=-2;
			c->errorCode=4;
			strcpy(c->error_msg,"pid json null"); 	
			goto EXIT;
		}
		strcpy(pid, json_object_to_json_string(pid_json));

		JObj *tag_json = JSON_GET_OBJECT(cmd_json, "tag");
		if(tag_json == NULL)
		{
			p_debug("tag json null");
			c->error = DM_ERROR_CMD_PARAMETER;
			ret=-3;
			c->errorCode=4;
			strcpy(c->error_msg,"tag json null"); 	
			goto EXIT;
		}
		strcpy(tag, json_object_to_json_string(tag_json));
		p_debug("tag=%s",tag);

		 //p_debug("++++++++++++pid=%s++++++++++++++++",pid);
		 strncpy(tmp_pid,pid+1,strlen(pid)-2);
		 //p_debug("++++++++++++tmp_pid=%s++++++++++++++++",tmp_pid);
		 //return the state
		 for(j=0;;j++){
			if(!album)break;
			if(!strcmp(album->pid,tmp_pid)){
				//p_debug("find it,info=%s",album->info);
				strcpy(album->tag,tag);
				//sprintf(c->loc.io.buf,"{\"status\":1,\"data\":{\"info\":%s}}",album->info);
				return ret;
				//break;
			}
				album=album->dn_next;
		}
			 
		#if 0
		array_len =JSON_GET_ARRAY_LEN(pid_json);
		if(array_len==0){
			ret=-4;
			c->errorCode=4;
			strcpy(c->error_msg,"pid is empty");
			c->error = DM_ERROR_CMD_PARAMETER;
			goto EXIT;
	
		}
		
		JObj *medi_array_obj;
		if(pid_json != NULL){
			for (i = 0; i < array_len; i++) {
			  // get the i-th object in medi_array
			 medi_array_obj = json_object_array_get_idx(pid_json, i);
			 strcpy(pid, json_object_to_json_string(medi_array_obj));
			 //p_debug("++++++++++++pid=%s++++++++++++++++",pid);
			 strncpy(tmp_pid,pid+1,strlen(pid)-2);
			 //p_debug("++++++++++++tmp_pid=%s++++++++++++++++",tmp_pid);
			 //return the state
			 for(j=0;;j++){
				if(!album)break;
				if(!strcmp(album->pid,tmp_pid)){
					p_debug("find it,info=%s",album->info);
					sprintf(c->loc.io.buf,"{\"status\":1,\"data\":{\"info\":%s}}",album->info);
					return ret;
					//break;
				}
				album=album->dn_next;
			 }
			c->errorCode=8;
			strcpy(c->error_msg,"pid is not found.");
			ret=-5;
			}
		}
		#endif
	
	EXIT:	
		return ret;

}


int Parser_AddFollow(struct conn *c)
{
		int ret=0;
		//int array_len=0;
		int j=0;
		int i=0;
		char pid[33]="\0";
		char tmp_pid[33]="\0";
		//char ext[65]="\0";

		p_debug("access Parser_AddFollow");

		http_tcpclient	t_client;
		memset(&t_client, '\0', sizeof(http_tcpclient));

		JObj *cmd_json = JSON_GET_OBJECT(c->r_json, CMD_ADD_FOLLOW);
		if(cmd_json == NULL)
		{
			p_debug("cmd json null");
			c->error = REQUEST_FORMAT_ERROR;
			ret=-1;
			c->errorCode=4;
			strcpy(c->error_msg,"cmd json is null");
			goto EXIT;
		}
	
		JObj *pid_json = JSON_GET_OBJECT(cmd_json, "pid");
		if(pid_json == NULL)
		{
			p_debug("pid json null");
			c->error = DM_ERROR_CMD_PARAMETER;
			ret=-2;
			c->errorCode=4;
			strcpy(c->error_msg,"pid json is empty");	
			goto EXIT;
		}
		
		JObj *ext_json = JSON_GET_OBJECT(cmd_json, "ext");
		if(ext_json == NULL)
		{
			p_debug("ext json null");
			c->error = DM_ERROR_CMD_PARAMETER;
			ret=-2;
			c->errorCode=4;
			strcpy(c->error_msg,"ext json is empty");	
			goto EXIT;
		}

		JObj *tag_json = JSON_GET_OBJECT(cmd_json, "tag");
		if(tag_json == NULL)
		{
			p_debug("tag json null");
			c->error = DM_ERROR_CMD_PARAMETER;
			ret=-2;
			c->errorCode=4;
			strcpy(c->error_msg,"tag json is empty");	
			goto EXIT;
		}
		
		
		struct album_node *album;
		album = xzalloc(sizeof(*album));
		if(!album)
		{
			p_debug("malloc album error");
			goto EXIT;
		}
		strcpy(pid, json_object_to_json_string(pid_json));
		//p_debug("++++++++++++pid=%s++++++++++++++++",pid);
		strncpy(tmp_pid,pid+1,strlen(pid)-2);
		//p_debug("++++++++++++tmp_pid=%s++++++++++++++++",tmp_pid);
		
		strcpy(album->pid,tmp_pid);
		char tmp_ext[256]="\0";
		char *ext =  json_object_to_json_string(ext_json);
		p_debug("ext=%s,%d",ext,strlen(ext));
		
		strcpy(tmp_ext,ext);
		p_debug("tmp_ext=%s",tmp_ext);
		
		char tmp[256]="\0";
		for(i=0;i<strlen(tmp_ext);i++){
			if(*(tmp_ext+i)!=' ')//del escape
				{
				tmp[j]=*(tmp_ext+i);
				j++;
			}
		}
		p_debug("tmp=%s,%d",tmp,strlen(tmp));
		if(strlen(tmp)>65){
			strcpy(c->error_msg,"ext too long!");
			p_debug("tmp=%s,%d",tmp,strlen(tmp));
			goto EXIT;
		}
		strcpy(album->ext,tmp);
		strcpy(album->tag,json_object_to_json_string(tag_json));
		//p_debug("album->ext=%s",album->ext);
		//p_debug("album->tag=%s",album->tag);		
		//check if exsit
		pthread_rwlock_trywrlock(&album_list_lock); 
		ret=add_album_to_list(&album_dn,album);
		pthread_rwlock_unlock(&album_list_lock);
		if(ret==1){//already exsit.
		   safe_free(album);
		   ret=0;
		   goto EXIT;
		}
		#if 0
		t_client.adn=album;//get the album info.
		ret=letv_client_get_source(&t_client,REQ_QUERY_ALBUM_INFO);
		
		pthread_rwlock_trywrlock(&album_list_lock); 
		ret=add_album_to_list(&album_dn,album);
		pthread_rwlock_unlock(&album_list_lock);
		if(ret<0){
		   safe_free(album);
		   c->errorCode=10;
		   strcpy(c->error_msg,"follow add save failed");
		   goto EXIT;
		}else  if(ret==1){//already exsit or add success.
		   ret=0;
		}
		#endif
		//p_debug("++++++++++++ret=%d++++++++++++++++",ret);
		
		if(ret==0){
		   flag_new_follow_add=1;
		}else{//failed
			//sprintf(c->loc.io.buf,"%s",JSON_TO_STRING(final_obj));
			c->errorCode=10;
			strcpy(c->error_msg,"follow add pid failed");
			goto EXIT;
		 }
	
		#if 0
		array_len =JSON_GET_ARRAY_LEN(pid_json);
		if(array_len==0){
			ret=-4;
			c->errorCode=4;
			strcpy(c->error_msg,"pid is empty");			
			c->error = DM_ERROR_CMD_PARAMETER;
			goto EXIT;
	
		}
	
		//JObj *medi_array_obj=JSON_NEW_EMPTY_OBJECT();
		if(pid_json != NULL){
			for (i = 0; i < array_len; i++) {
			  // get the i-th object in medi_array
			  struct album_node *album;
			  album = xzalloc(sizeof(*album));
			  if(!album)
			  {
				  p_debug("add dev error");
				  goto EXIT;
			  }

			  
			 JObj * medi_array_obj = json_object_array_get_idx(pid_json, i);
			 strcpy(pid, json_object_to_json_string(medi_array_obj));
			 p_debug("++++++++++++pid=%s++++++++++++++++",pid);
			 strncpy(tmp_pid,pid+1,strlen(pid)-2);
			 p_debug("++++++++++++tmp_pid=%s++++++++++++++++",tmp_pid);
			 //return the state
			 strcpy(album->pid,tmp_pid);

			 //check if exsit
			 pthread_rwlock_trywrlock(&album_list_lock); 
			 ret=add_album_to_list(&album_dn,album);
			 pthread_rwlock_unlock(&album_list_lock);
			 if(ret==1){//already exsit.
				safe_free(album);
				ret=0;
				goto EXIT;
			 }
			 
			 t_client.adn=album;
			 ret=letv_client_get_source(&t_client,REQ_QUERY_ALBUM_INFO);

			 pthread_rwlock_trywrlock(&album_list_lock); 
			 ret=add_album_to_list(&album_dn,album);
			 pthread_rwlock_unlock(&album_list_lock);
			 if(ret<0){
				safe_free(album);
				c->errorCode=10;
				strcpy(c->error_msg,"follow add save failed");
				goto EXIT;
			 }else  if(ret==1){//already exsit.
				ret=0;
			 }
			 p_debug("++++++++++++ret=%d++++++++++++++++",ret);

			 if(ret==0){
			 	flag_new_follow_add=1;
			 /*	
				 p_debug("++++++++++++tmp_pid=%s++++++++++++++++",tmp_pid);

				 struct task_dnode *video;
				 video = xzalloc(sizeof(*video));
				 if(!video)
				 {
					 p_debug("add dev error");
					 goto EXIT;
				 }

				 
				 t_client.dn=video;
				 strcpy(t_client.dn->pid,tmp_pid);			 	
				 p_debug("++++++++++++tmp_pid=%s++++++++++++++++",t_client.dn->pid);
	 
				 ret=letv_client_get_source(&t_client,ALBUM);
				 if(ret==1){//already exsit.
					safe_free(video);
					ret=0;
					goto EXIT;
				 }
				 */
			 }else{//failed
				//sprintf(c->loc.io.buf,"%s",JSON_TO_STRING(final_obj));
				c->errorCode=10;
				strcpy(c->error_msg,"follow add pid failed");
				goto EXIT;
			 }
			 JSON_PUT_OBJECT(medi_array_obj);
			}
			
		}
		#endif
		display_album_dnode(album_dn);

EXIT:	

	return ret;

}

int Parser_DelFollow(struct conn *c)
{
		int ret=0;
		int array_len=0;
		int j,i,m=0;
		char pid[33]="\0";
		char tmp_pid[33]="\0";
		struct album_node*cur;
		struct task_dnode *task_node;
/*		cur = xzalloc(sizeof(*cur));
		if(!cur)
		{
			p_debug("add dev error");
			goto EXIT;
		}

*/		p_debug("access Parser_DelFollow");
		JObj *cmd_json = JSON_GET_OBJECT(c->r_json, CMD_DEL_FOLLOW);
		if(cmd_json == NULL)
		{
			p_debug("cmd json null");
			ret=-1;
			c->errorCode=4;
			strcpy(c->error_msg,"cmd json null");
			goto EXIT;
		}
	
		JObj *pid_json = JSON_GET_OBJECT(cmd_json, "pid");
		if(pid_json == NULL)
		{
			p_debug("pid json null");
			c->error = DM_ERROR_CMD_PARAMETER;
			ret=-2;
			c->errorCode=4;
			strcpy(c->error_msg,"pid json null");	
			goto EXIT;
		}
		
		array_len =JSON_GET_ARRAY_LEN(pid_json);
		if(array_len==0){
			ret=-4;
			c->errorCode=4;
			strcpy(c->error_msg,"pid is empty");
			c->error = DM_ERROR_CMD_PARAMETER;
			goto EXIT;
	
		}
	
		JObj *medi_array_obj;
		if(pid_json != NULL){
			for (i = 0; i < array_len; i++) {
			  // get the i-th object in medi_array
			 medi_array_obj = json_object_array_get_idx(pid_json, i);
			 strcpy(pid, json_object_to_json_string(medi_array_obj));
			 //p_debug("++++++++++++pid=%s++++++++++++++++",pid);
			 strncpy(tmp_pid,pid+1,strlen(pid)-2);
			 //p_debug("++++++++++++tmp_pid=%s++++++++++++++++",tmp_pid);
			 //return the state
			 //strcpy(cur->pid,tmp_pid);

			 pthread_rwlock_trywrlock(&album_list_lock); 
			//pthread_rwlock_trywrlock(&task_list_lock); 
			 display_album_dnode(album_dn);
			 cur=album_dn;
			 for(j=0;;j++){
			 	if(!cur)break;
				if(!strcmp(cur->pid,tmp_pid)){
					//del pid from albumlist

		
					//pause the  vid if it's downloading from tasklist					
					task_node=task_dn;
					//display_task_dnode(task_node);
					while(task_node!=NULL){
						p_debug("vvvv=%s,p=%s,tp=%s",task_node->vid,task_node->pid,tmp_pid);

						if((!strcmp(task_node->pid,tmp_pid))){
							task_node->isAutoAdd=0;
							if(task_node->download_status==DOWNLOADING)
								update_task_status_to_list(task_node->pid,task_node->vid,PAUSE);//stop it first.
							if(task_node->isDeleted==1)
								{
								del_task_from_list(&task_dn,task_node->pid,task_node->vid);
								task_node=task_dn;
								continue;
							}
							//break;
						}//else 
							task_node=task_node->dn_next;
						//p_debug("2vvvv=%s,p=%s,tp=%s",task_node->vid,task_node->pid,tmp_pid);
						//display_task_dnode(task_dn);
						//p_debug("deee-------");
					}
				
					//ret=del_album_from_list(&album_dn,tmp_pid);
					ret=update_album_status_to_list(tmp_pid,REMOVE);
					if(ret==0){
						p_debug("del pid succ",tmp_pid);
					}
				/*	for(m=0;;m++){
						if(!task_node)break;
						p_debug("vvvv=%s,p=%s,tp=%s",task_node->vid,task_node->pid,tmp_pid);
						if(!strcmp(task_node->pid,tmp_pid)){
							
							ret=update_task_status_to_list(task_node->pid,task_node->vid,REMOVE);//stop it first.
							//ret=del_task_from_list(&task_dn,task_node->pid,task_node->vid);
						}
						p_debug("2vvvv=%s,p=%s,tp=%s",task_node->vid,task_node->pid,tmp_pid);

						task_node=task_node->dn_next;

						p_debug("deee=vid %s",task_node->vid);
					}*/
				}
				cur=cur->dn_next;
				//p_debug("array_len=%d",j);
			 }
			 //p_debug("array_len iiiii=%d",i);

			}
		}
		p_debug("leave Parser_DelFollow");
	display_album_dnode(album_dn);
	//pthread_rwlock_unlock(&task_list_lock);
	pthread_rwlock_unlock(&album_list_lock);

EXIT:	
	p_debug("leave Parser_DelFollow");
	return ret;

}

int fileDownload(struct conn *c){
	p_debug("access fileDownload");
	int ret=0;
	int i;
	char type[8]="\0";
	char ip[32]="\0";
	char tmp_vid[VID_LEN];

	JObj *cmd_json = JSON_GET_OBJECT(c->r_json, CMD_LETV_FILE_DOWNLOAD);
	if(cmd_json == NULL)
	{
		p_debug("cmd json null");
		ret=-1;
		c->errorCode=4;
		strcpy(c->error_msg,"cmd json null");
		goto EXIT;
	}

	JObj *vid_json = JSON_GET_OBJECT(cmd_json, "vid");
	if(vid_json == NULL)
	{
		p_debug("vid json null");
		c->error = DM_ERROR_CMD_PARAMETER;
		ret=-2;
		c->errorCode=4;
		strcpy(c->error_msg,"vid json null");	
		goto EXIT;
	}

	
	JObj *ip_json = JSON_GET_OBJECT(cmd_json, "ip");
	if(ip_json == NULL)
	{
		p_debug("ip json null");
		c->error = DM_ERROR_CMD_PARAMETER;
		ret=-2;
		c->errorCode=4;
		strcpy(c->error_msg,"ip json null");	
		goto EXIT;
	}

	JObj *type_json = JSON_GET_OBJECT(cmd_json, "type");
	if(type_json == NULL)
	{
		p_debug("type json null");
		c->error = DM_ERROR_CMD_PARAMETER;
		ret=-2;
		c->errorCode=4;
		strcpy(c->error_msg,"type json null");	
		goto EXIT;
	}

	sprintf(tmp_vid,"%s",JSON_GET_OBJECT_VALUE(vid_json,string));
	
	sprintf(ip,"%s",JSON_GET_OBJECT_VALUE(ip_json,string));

	sprintf(type,"%s",JSON_GET_OBJECT_VALUE(type_json,string));

	struct task_dnode *tmp_node=task_dn;
	for(i=0;;i++){
		if(tmp_node==NULL)break;
		if(!strcmp(tmp_node->vid,tmp_vid)){
			if(!strcmp(type,"0"))
				sprintf(c->loc.io.buf,"http://%s%s",ip,tmp_node->vid_path);
			else {
				sprintf(c->loc.io.buf,"http://%s%s",ip,tmp_node->img_path);
			}
			p_debug("leave fileDownload succ");
			return 1;
		}		
		tmp_node=tmp_node->dn_next;
	}
	c->errorCode=8;
	strcpy(c->error_msg,"file not found");
	ret=-3;

EXIT:
	p_debug("leave fileDownload failed");
	return ret;
		
}
int lenList(struct album_node* head){ 

    if(head==NULL) return 0; 
    struct album_node *p = head; 
    int sum=0; 
    while(p!=NULL){ 
        sum+=1; 
        p=p->dn_next; 
    } 
    return sum; 
} 

void dfree_file_dnode(struct album_node **dnp)
{
    ENTER_FUNC();
    unsigned i;
    if(dnp == NULL)
        return;
    for(i = 0;dnp[i];i++)
    {
        struct album_node *cur = dnp[i];
        if(cur != NULL)
        {
            safe_free(cur);
        }
    }
    free(dnp);
    EXIT_FUNC();
}
int add_to_list(struct album_node*dest,struct album_node* src){


	dest->dn_next=src->dn_next;
	strcpy(dest->pid,src->pid);
	strcpy(dest->info,src->info);
	strcpy(dest->coverImgUrl,src->coverImgUrl);
	strcpy(dest->album_img_path,src->album_img_path);	
	strcpy(dest->tag,src->tag);	
	strcpy(dest->tag2,src->tag2);	
	strcpy(dest->ext,src->ext);	
	strcpy(dest->error_msg,src->error_msg);
	dest->status=src->status;
	dest->errorCode=src->errorCode;
	dest->update_time=src->update_time;
	dest->isEnd=src->isEnd;
	return 1;
}
void printList(struct album_node * head){ 
    struct album_node *p = head; 
    
    while(p!=NULL){ 
        p_debug("p[%s]->updatetime=%d",p->pid,p->update_time); 
        p=p->dn_next; 
    } 
    
} 

struct album_node ** sort_list(struct album_node* head){
	int i;
	int len = lenList(album_dn);
	p_debug("len=%d",len);
	
	struct album_node **fdnp = (struct album_node **)calloc(1,(len + 1)*sizeof(struct album_node *));
	struct album_node *p = head; 
	//struct album_node *q = head;     
	//p_debug("sizeof=%d",sizeof(p->update_time));
/*	
	for(i=0; i<len; ++i){ 
		//p_debug("1p->update_time=%d",p->update_time);
		p->update_time=i;
		p_debug("i=[%d][%s].ut=%d",i,p->pid,p->update_time);
		p=p->dn_next;
	}
	*/
	

	struct album_node *fdn =  head;
	 for(i = 0;/*i < nfiles - detect via !dn below*/;)
    {
        //if(fdn->isFolder == 0)
        fdnp[i++] = fdn;/*save pointer to node in array*/
		//p_debug("[%d]->update_time=%d",i,fdn->update_time);

        fdn = fdn->dn_next;

        if(!fdn)
            break;
    }

	if(len>1) dnsort(fdnp, len );
	return fdnp;
	

}

int Parser_GetAllFollow(struct conn *c)
{
	int ret=0;
	int array_len=0;
	int j,k,i=0;
	char pid[1024]="\0";
	char tmp_pid[33]="\0";
	
	char type[8]="\0";
	char needTag[8]="\0";
	char needInfo[8]="\0";
	char needAlbumInfo[8]="0";
	char needExt[8]="\0";
	struct album_node **adn;

	pthread_rwlock_trywrlock(&album_list_lock); 
	//sortList(&album_dn);
	//struct album_node *adn=album_dn;
	int len = lenList(album_dn);
	if(len==0)
	{
		adn=NULL;
	}
	else  
 		adn= sort_list(album_dn);


	for(i=0;i<len;i++)
		{
		p_debug("[%s].ut=%d",adn[i]->pid,adn[i]->update_time);
	}

	//p_debug("access Parser_GetAllFollow");
	JObj *cmd_json = JSON_GET_OBJECT(c->r_json, CMD_GET_ALL_FOLLOW);
	if(cmd_json == NULL)
	{
		p_debug("cmd json null");
		c->error = REQUEST_FORMAT_ERROR;
		ret=-1;
		c->errorCode=4;
		strcpy(c->error_msg,"cmd json null");
		goto EXIT;
	}

	JObj *pid_json = JSON_GET_OBJECT(c->r_json, "pid");
	if(pid_json == NULL)
	{
		p_debug("pid_json null");
		c->error = REQUEST_FORMAT_ERROR;
		ret=-1;
		c->errorCode=4;
		strcpy(c->error_msg,"pid_json  null");
		goto EXIT;
	}
	
	JObj *type_json = JSON_GET_OBJECT(c->r_json, "type");

	if(type_json == NULL)
	{
		p_debug("type_json null");
		c->error = REQUEST_FORMAT_ERROR;
		ret=-1;
		c->errorCode=4;
		strcpy(c->error_msg,"type_json null");
		goto EXIT;
	}

	strcpy(type,JSON_GET_OBJECT_VALUE(type_json,string));

	JObj *needTag_json = JSON_GET_OBJECT(c->r_json, "needTag");
	if(needTag_json == NULL)
	{
		p_debug("needTag json null");
		c->error = REQUEST_FORMAT_ERROR;
		ret=-1;
		c->errorCode=4;
		strcpy(c->error_msg,"needTag json null");
		goto EXIT;
	}
	strcpy(needTag,JSON_GET_OBJECT_VALUE(needTag_json,string));

	JObj *needInfo_json = JSON_GET_OBJECT(c->r_json, "needInfo");
	if(needInfo_json == NULL)
	{
		p_debug("needInfo json null");
		c->error = REQUEST_FORMAT_ERROR;
		ret=-1;
		c->errorCode=4;
		strcpy(c->error_msg,"needInfo json null");
		goto EXIT;
	}
	strcpy(needInfo,JSON_GET_OBJECT_VALUE(needInfo_json,string));

	JObj *needAlbumInfo_json = JSON_GET_OBJECT(c->r_json, "needAlbumInfo");
	if(needAlbumInfo_json == NULL)
	{/*
		p_debug("needAlbumInfo json null");
		c->error = REQUEST_FORMAT_ERROR;
		ret=-1;
		c->errorCode=4;
		strcpy(c->error_msg,"needAlbumInfo json null");
		goto EXIT;
	*/
	}else
		strcpy(needAlbumInfo,JSON_GET_OBJECT_VALUE(needAlbumInfo_json,string));

	JObj *needExt_json = JSON_GET_OBJECT(c->r_json, "needExt");
	if(needExt_json == NULL)
	{
		p_debug("needExt json null");
		c->error = REQUEST_FORMAT_ERROR;
		ret=-1;
		c->errorCode=4;
		strcpy(c->error_msg,"needExt json null");
		goto EXIT;
	}
	//p_debug("needExt=%s",needExt);
	strcpy(needExt,JSON_GET_OBJECT_VALUE(needExt_json,string));
	//p_debug("needExt=%s",needExt);


	int	pid_array_len=JSON_GET_ARRAY_LEN(pid_json);
	p_debug("pid_array_len=%d",pid_array_len);

		
	JObj *j_array = JSON_NEW_ARRAY();

	JObj *j_data = JSON_NEW_EMPTY_OBJECT();

	JObj *j_reply = JSON_NEW_EMPTY_OBJECT();

	if(pid_array_len == 0){
			
		for(i = 0;i<len;i++)
		{
			if(!(adn[i]))
				break;
		//	p_debug("adn[i]->pid=%s",adn[i]->pid);
			if(adn[i]->status != REMOVE){
				JObj *j_list = JSON_NEW_EMPTY_OBJECT();
				//p_debug("adn[i]->pid=%s",adn[i]->pid);
				JObj *tag_json=JSON_NEW_EMPTY_OBJECT();
				JObj *album_json=JSON_NEW_EMPTY_OBJECT();
				JObj *ext_json=JSON_NEW_EMPTY_OBJECT();
				JObj *h_videoInfoList=JSON_NEW_ARRAY();
				if(!strcmp(needTag,"1"))
					{ 
					if(strcmp(adn[i]->tag,"")) tag_json=JSON_PARSE(adn[i]->tag);
					JSON_ADD_OBJECT(j_list,"tag",tag_json);
				}
				if(!strcmp(needAlbumInfo,"1"))
					 {
					 if(strcmp(adn[i]->info,"")) album_json=JSON_PARSE(adn[i]->info);
					 JSON_ADD_OBJECT(j_list,"albumInfo",album_json);

				}

				if(strcmp(needExt,"0"))
					 {
					 char tmp_end[16]={0};
					 sprintf(tmp_end,"{\"isEnd\":%d}",adn[i]->isEnd);
					 //p_debug("tmp_end=%s",tmp_end);
					 //if(strcmp(adn[i]->isEnd,"")) ext_json=JSON_PARSE(adn[i]->ext);
					 JSON_ADD_OBJECT(j_list,"ext",JSON_PARSE(tmp_end));
				}
				if(!strcmp(type,"0")){//pid's all vid
					if(!strcmp(needInfo,"1"))
					{	//video info
						struct task_dnode *dn=task_dn;
						
						for(k=0;;k++){
							if(!dn) break;
							if((!strcmp(dn->pid,adn[i]->pid)))
							{
								JObj *videoInfoList=JSON_NEW_EMPTY_OBJECT();
								JObj *vid_json =json_object_new_string(dn->vid);
								JObj *info_json=JSON_NEW_EMPTY_OBJECT();
								if(strcmp(dn->info,""))
									info_json =JSON_PARSE(dn->info);
								JSON_ADD_OBJECT(videoInfoList,"vid",vid_json);
								JSON_ADD_OBJECT(videoInfoList,"info",info_json);

								JSON_ARRAY_ADD_OBJECT(h_videoInfoList,videoInfoList);
							}								
							dn=dn->dn_next;
						}	
						JSON_ADD_OBJECT(j_list,"videoInfoList",h_videoInfoList);
						
						
						JSON_ADD_OBJECT(j_list,"pid",json_object_new_string(adn[i]->pid)); 

					}else {
						
						JSON_ADD_OBJECT(j_list,"pid",json_object_new_string(adn[i]->pid)); 
					}
					
				}else if(!strcmp(type,"2")){//pid's all finished vid
					if(!strcmp(needInfo,"1"))
					{	//video info
						struct task_dnode *dn=task_dn;
						
						for(k=0;;k++){
							if(!dn) break;
							if((!strcmp(dn->pid,adn[i]->pid))&&(dn->download_status!=2))
							{
								JObj *videoInfoList=JSON_NEW_EMPTY_OBJECT();
								JObj *vid_json =json_object_new_string(dn->vid);
								JObj *info_json=JSON_NEW_EMPTY_OBJECT();
								if(strcmp(dn->info,""))
									info_json =JSON_PARSE(dn->info);

								JSON_ADD_OBJECT(videoInfoList,"vid",vid_json);
								JSON_ADD_OBJECT(videoInfoList,"info",info_json);

								JSON_ARRAY_ADD_OBJECT(h_videoInfoList,videoInfoList);
							}								
							dn=dn->dn_next;
						}	
						JSON_ADD_OBJECT(j_list,"videoInfoList",h_videoInfoList);
						//JSON_ADD_OBJECT(j_list,"tag",tag_json);
						JSON_ADD_OBJECT(j_list,"pid",json_object_new_string(adn[i]->pid)); 

					}else {
						//JSON_ADD_OBJECT(j_list,"tag",tag_json);
						JSON_ADD_OBJECT(j_list,"pid",json_object_new_string(adn[i]->pid)); 
					}

				}else {//pid's all unfinished vid
					if(!strcmp(needInfo,"1"))
						{	//video info
							struct task_dnode *dn=task_dn;
							
							for(k=0;;k++){
								if(!dn) break;
								if((!strcmp(dn->pid,adn[i]->pid))&&(dn->download_status==2))
								{
									JObj *videoInfoList=JSON_NEW_EMPTY_OBJECT();
									JObj *vid_json =json_object_new_string(dn->vid);
									JObj *info_json=JSON_NEW_EMPTY_OBJECT();
									if(strcmp(dn->info,""))
										info_json =JSON_PARSE(dn->info);

									JSON_ADD_OBJECT(videoInfoList,"vid",vid_json);
									JSON_ADD_OBJECT(videoInfoList,"info",info_json);
					
									JSON_ARRAY_ADD_OBJECT(h_videoInfoList,videoInfoList);
								}								
								dn=dn->dn_next;
							}	
							JSON_ADD_OBJECT(j_list,"videoInfoList",h_videoInfoList);
							//JSON_ADD_OBJECT(j_list,"tag",tag_json);
							JSON_ADD_OBJECT(j_list,"pid",json_object_new_string(adn[i]->pid)); 
						}else {
							//JSON_ADD_OBJECT(j_list,"tag",tag_json);
							JSON_ADD_OBJECT(j_list,"pid",json_object_new_string(adn[i]->pid)); 
						}

				}
				JSON_ARRAY_ADD_OBJECT(j_array,j_list);		
			}
			//adn[i] = adn[i]->dn_next;
		}
		
		JSON_ADD_OBJECT(j_reply,"status",json_object_new_int(1));
		JSON_ADD_OBJECT(j_data,"list",j_array);
		JSON_ADD_OBJECT(j_reply,"data",j_data);
		//JSON_ADD_OBJECT(j_reply,"list",j_array));
		//p_debug("j_reply=%s",JSON_TO_STRING(j_reply));

	}
	else{
		for(j=0;j<pid_array_len-1;j++)
		{
			JObj *pid_j=json_object_array_get_idx(pid_json,j);
			char pid_j_val[33];
			strcpy(pid_j_val,JSON_GET_OBJECT_VALUE(pid_j,string));
			strncpy(tmp_pid,pid_j_val+1,strlen(pid_j_val)-2);
			p_debug("tmp_pid=%s",tmp_pid);
			
			JObj *j_list = JSON_NEW_EMPTY_OBJECT();
			for(i = 0;i<len;i++)
			{
				if((!adn[i]))
					break;
				if(adn[i]->status != REMOVE){
				p_debug("adn[i]->pid=%s",adn[i]->pid);
				if((!strcmp(adn[i]->pid,tmp_pid))&&(tmp_pid[0]!='\0')){
						JObj *tag_json=JSON_NEW_EMPTY_OBJECT();
						JObj *h_videoInfoList=JSON_NEW_ARRAY();
					if(!strcmp(needTag,"1"))
							 if(strcmp(adn[i]->tag,"")) tag_json=JSON_PARSE(adn[i]->tag);
					
					if(!strcmp(type,"0")){//pid's all vid
						if(!strcmp(needInfo,"1"))
						{	//video info
							struct task_dnode *dn=task_dn;
							
							for(k=0;;k++){
								if(!dn)	break;
								if(!strcmp(dn->pid,tmp_pid))
								{
									JObj *videoInfoList=JSON_NEW_EMPTY_OBJECT();
									JObj *vid_json =json_object_new_string(dn->vid);
									JObj *info_json=JSON_NEW_EMPTY_OBJECT();
									if(strcmp(dn->info,""))
										info_json =JSON_PARSE(dn->info);

									JSON_ADD_OBJECT(videoInfoList,"vid",vid_json);
									JSON_ADD_OBJECT(videoInfoList,"info",info_json);

									JSON_ARRAY_ADD_OBJECT(h_videoInfoList,videoInfoList);
								}								
								dn=dn->dn_next;
							}	
							JSON_ADD_OBJECT(j_list,"videoInfoList",h_videoInfoList);

						}else {
							JSON_ADD_OBJECT(j_list,"tag",tag_json);
							JSON_ADD_OBJECT(j_list,"pid",json_object_new_string(adn[i]->pid));	
						}
						
					}else if(!strcmp(type,"2")){//pid's all finished vid
						if(!strcmp(needInfo,"1"))
						{	//video info
							struct task_dnode *dn=task_dn;
							
							for(k=0;;k++){
								if(!dn)	break;
								if((!strcmp(dn->pid,tmp_pid))&&(dn->download_status!=2))
								{
									JObj *videoInfoList=JSON_NEW_EMPTY_OBJECT();
									JObj *vid_json =json_object_new_string(dn->vid);
									JObj *info_json=JSON_NEW_EMPTY_OBJECT();
									if(strcmp(dn->info,""))
										info_json =JSON_PARSE(dn->info);

									JSON_ADD_OBJECT(videoInfoList,"vid",vid_json);
									JSON_ADD_OBJECT(videoInfoList,"info",info_json);

									JSON_ARRAY_ADD_OBJECT(h_videoInfoList,videoInfoList);
								}								
								dn=dn->dn_next;
							}	
							JSON_ADD_OBJECT(j_list,"videoInfoList",h_videoInfoList);

						}else {
							JSON_ADD_OBJECT(j_list,"tag",tag_json);
							JSON_ADD_OBJECT(j_list,"pid",json_object_new_string(adn[i]->pid));	
						}

					}else {//pid's all unfinished vid
						if(!strcmp(needInfo,"1"))
							{	//video info
								struct task_dnode *dn=task_dn;
								
								for(k=0;;k++){
									if(!dn) break;
									if((!strcmp(dn->pid,tmp_pid))&&(dn->download_status!=2))
									{
										JObj *videoInfoList=JSON_NEW_EMPTY_OBJECT();
										JObj *vid_json =json_object_new_string(dn->vid);
										JObj *info_json=JSON_NEW_EMPTY_OBJECT();
										if(strcmp(dn->info,""))
											info_json =JSON_PARSE(dn->info);

										JSON_ADD_OBJECT(videoInfoList,"vid",vid_json);
										JSON_ADD_OBJECT(videoInfoList,"info",info_json);
						
										JSON_ARRAY_ADD_OBJECT(h_videoInfoList,videoInfoList);
									}								
									dn=dn->dn_next;
								}	
								JSON_ADD_OBJECT(j_list,"videoInfoList",h_videoInfoList);
						
							}else {
								JSON_ADD_OBJECT(j_list,"tag",tag_json);
								JSON_ADD_OBJECT(j_list,"pid",json_object_new_string(adn[i]->pid)); 
							}

					}

				}
				}			
				//adn[i] = adn[i]->dn_next;
			}
			JSON_ARRAY_ADD_OBJECT(j_array,j_list);
		}
		JSON_ADD_OBJECT(j_reply,"status",json_object_new_int(1));
		JSON_ADD_OBJECT(j_reply,"data",j_array);
		//JSON_ADD_OBJECT(j_reply,"list",j_array));
		//p_debug("j_reply=%s",JSON_TO_STRING(j_reply));
	}
#if 0
	for(i = 0;/*count - detected via !dn below*/;i++)
	{
		if(!dn)
			break;

		p_debug("i = %d,dn->pid = %s,dn->info = %s",i,dn->pid,dn->info);
		JObj *list_json;
		JObj *pid_json =json_object_new_string(dn->pid);
		JSON_ARRAY_ADD_OBJECT(j_array,pid_json);
		if(!strcmp(needTag,"1"))
			JObj *tag_json =json_object_new_string(dn->tag);


		if(!strcmp(type,"0")){//pid's all vid
			if(!strcmp(needInfo,"1"))
			{	//video info
				JObj *info_json =json_object_new_string(dn->tag);
			}else {
				
			}
		}else if(!strcmp(type,"1")){//pid's all finished vid
			
		}else if(!strcmp(type,"2")){//pid's all unfinished vid
			
		}
		
		JSON_ARRAY_ADD_OBJECT(j_array,tag_json);
		p_debug("j_array=%s",JSON_TO_STRING(j_array));

		dn = dn->dn_next;
	}

	p_debug("j_array=%s",JSON_TO_STRING(j_array));

	JSON_ADD_OBJECT(j_data,"list",j_array);
	p_debug("j_data=%s",JSON_TO_STRING(j_data));

	JSON_ADD_OBJECT(j_reply,"data",j_data);
	p_debug("j_reply=%s",JSON_TO_STRING(j_reply));
	JSON_ADD_OBJECT(j_reply,"status",json_object_new_int(1));
//	JSON_ADD_OBJECT(tmp_obj,"vid",json_object_new_string(dn->vid));
#endif
	
	//p_debug("j_reply=%s",JSON_TO_STRING(j_reply));
	//memset(c->loc.io.buf,0,sizeof(c->loc.io.buf));
	c->loc.io.buf=JSON_TO_STRING(j_reply);
	//p_debug("c->loc.io.buf=%s,strlen=%d",c->loc.io.buf,strlen(c->loc.io.buf));

	//JSON_PUT_OBJECT(j_reply);
EXIT:	
//dfree_file_dnode(struct album_node * * dnp)(bdn);
	//destory_album_list(adn[i]);
	//ret=1;
	free(adn);
	pthread_rwlock_unlock(&album_list_lock);
	return ret;

}



int file_parse_header_json(struct conn *c)
{
	#if 0
	c->r_json = JSON_PARSE(c->rem.io.buf);
	if(c->r_json == NULL)
	{
		p_debug("access NULL");
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	if(is_error(c->r_json))
	{
		p_debug("### error:post data is not a json string");
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	
	JObj *header_json = JSON_GET_OBJECT(c->r_json,"header");
	if(header_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *cmd_json = JSON_GET_OBJECT(header_json,"cmd");
	if(cmd_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	c->cmd = JSON_GET_OBJECT_VALUE(cmd_json,int);
	JObj *session_json = JSON_GET_OBJECT(header_json,"session");
	JObj *seq_json = JSON_GET_OBJECT(header_json,"seq");

	if(seq_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	c->seq = JSON_GET_OBJECT_VALUE(seq_json,int);
	if(session_json != NULL)
		strcpy(c->session,JSON_GET_OBJECT_VALUE(session_json,string));
	p_debug("c->cmd = %d",c->cmd);
EXIT:
	#endif
	return 0;
}

int file_parse_process(struct conn *c)
{
	//TODO 需添加解析json文件的出错机制
	/*
    uint8_t i = 0;
    uint8_t switch_flag = 0;
    int ret = 0;
	file_parse_header_json(c);
    for(i = 0; i< FILE_TAGHANDLE_NUM; i++)
    {
        if(c->cmd == all_file_handle[i].tag)
        {
            ret = all_file_handle[i].parsefun(c);
            switch_flag = 1;
        }
    }
    if(switch_flag == 0)
    {
        c->error = INVALIDE_COMMAND;//命令无法识别
		p_debug("cmd not found");
    }else if(ret < 0)
    {
		c->error = REQUEST_FORMAT_ERROR;//命令的格式错误
	}	
    return ret;
    */
	p_debug("access file_parse_process");

	int i = 0;
	char cmd_str[32];
	uint8_t switch_flag = 0;
	int ret = 0;
	JObj *cmd_json = NULL;
	c->r_json = JSON_PARSE(c->rem.io.buf);
	//p_debug("c->rem.io.buf = %s", c->rem.io.buf);
	if(c->r_json == NULL){
		p_debug("access NULL");
		c->error = REQUEST_FORMAT_ERROR;
		c->errorCode=4;
		strcpy(c->error_msg,"Json null");

		ret = -1;
		goto EXIT;
	}
	if(is_error(c->r_json)){
		p_debug("### error:post data is not a json string");
		c->error = REQUEST_FORMAT_ERROR;
		c->errorCode=4;
		strcpy(c->error_msg,"post data is not a json string");
		ret = -2;
		goto EXIT;
	}


	for(i = 0; i < FILE_TAGHANDLE_NUM; i++){
		//p_debug("all_file_handle[%d].tag = %s", i, all_file_handle[i].tag);
		cmd_json = JSON_GET_OBJECT(c->r_json, all_file_handle[i].tag);
		if(cmd_json != NULL){
			p_debug("c--->cmd=%s",all_file_handle[i].tag);
			strcpy(c->cmd, all_file_handle[i].tag);
			ret = all_file_handle[i].parsefun(c);
			//p_debug("rrrrrrrrrrrr=%d",ret);
			switch_flag = 1;
			break;
		}
		if(i==(FILE_TAGHANDLE_NUM-1))
			{
			ret= -5;
			c->errorCode=4;
			strcpy(c->error_msg,"Json format error");

			goto EXIT;
		}
	}
	if(switch_flag == 0){
        c->error = INVALIDE_COMMAND;//命令无法识别
		p_debug("cmd not found");
		ret = -3;
		c->errorCode=4;
		strcpy(c->error_msg,"cmd not found");

		goto EXIT;
    }
	else if(ret < 0){
		c->error = REQUEST_FORMAT_ERROR;//命令的格式错误
		//c->errorCode=4;
		//strcpy(c->error_msg,"cmd not found");
		p_debug("cmd format error");
		//c->errorCode=4;
		//strcpy(c->error_msg,"cmd format error");		
		ret = -4;
		//errcode=4,
		//strcpy(errmsg,"cmd format error");
		goto EXIT;
	}
	else{//执行写入命令后备份
		system("cp -f /tmp/mnt/USB-disk-1/hack/.tasklist /tmp/mnt/USB-disk-1/hack/.tasklist.bak");
		usleep(100000);		
	}
	if(c->r_json != NULL)
        JSON_PUT_OBJECT(c->r_json);
	return ret;
EXIT:
	sprintf(c->loc.io.buf,"{\"status\":0,\"data\":{},\"errorCode\":%d,\"errorMessage\":\"%s\"}",c->errorCode,c->error_msg);
	p_debug(c->loc.io.buf);
	c->loc.io.size = strlen(c->loc.io.buf);
	c->loc.io.head = c->loc.io.size;
	c->loc.io.tail = 0;
	c->loc.io.total = c->loc.io.size;
	if(c->r_json != NULL)
        JSON_PUT_OBJECT(c->r_json);
    return ret;
}

