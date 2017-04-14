/*
 * =============================================================================
 *
 *       Filename:  router_task.c
 *
 *    Description:  query the router infomation and sending info to dev according to dev quest cmd
 *
 *        Version:  1.0
 *        Created:  2015/08/20 14:33
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver, 
 *   Organization:  
 *
 * =============================================================================*/
#include "router_task.h"
#include "../http_client/http_client.h"

#include "base.h"
#include "util.h"
//#include <io.h>
#include <pthread.h>
#if 0
int backupData(){
	FILE *fTaskList;
	FILE *fTaskListBak;
	FILE *fAlbumList;
	if(access(path,R_OK)!=0){// 不存在
		
	}
	if(fTaskListBak=fopen())
}

int restoreData(){
	FILE *fTaskList;
	FILE *fTaskListBak;
	FILE *fAlbumList;
	if(access(path,R_OK)!=0){// 不存在
		
	}
	if(fTaskListBak=fopen())
}
#endif

int update_album_to_list(struct album_node **dn,struct album_node*cur)
{
	p_debug("access update_album_to_list");
	int i;
	int ret=0;
	struct album_node *pdn;
	pdn=album_dn;
	display_album_dnode(album_dn);
	p_debug("update pid=%s to albumlist",cur->pid);
	for(i=0;;i++){
		if(!pdn)break;
		if(!strcmp(pdn->pid,cur->pid)){
			//if((pdn->download_status != DONE) ||(pdn->download_status != DOWNLOADING)) pdn->download_status=WAITING;
			pdn->status=1;
			//strcpy(pdn->album_img_path,cur->album_img_path);
			strcpy(pdn->coverImgUrl,cur->coverImgUrl);
			strcpy(pdn->info,cur->info);
			//strcpy(pdn->ext,cur->ext);
			//strcpy(pdn->tag,cur->tag);			
			p_debug("already exist pid=%s",pdn->pid);
			write_list_to_file(ALBUM);		
			//safe_free(cur);
			return 1;
		}
		p_debug("pdn->pid=%s",pdn->pid);
		pdn=(pdn)->dn_next;
	}
	
	ret=write_list_to_file(ALBUM);
	if(ret<0){
		p_debug("pid update failed");	
	}

	p_debug("leave update_album_to_list");
	return ret;
}

int add_album_to_list(struct album_node **dn,struct album_node*cur)
{
	p_debug("access add_album_to_list");
	int i;
	int ret=0;
	struct album_node *pdn;
	pdn=album_dn;
	display_album_dnode(album_dn);
	p_debug("add pid=%s to albumlist",cur->pid);
	for(i=0;;i++){
		if(!pdn)break;
		if(!strcmp(pdn->pid,cur->pid)){
			//if((pdn->download_status != DONE) ||(pdn->download_status != DOWNLOADING)) pdn->download_status=WAITING;
			pdn->status=1;
			strcpy(pdn->album_img_path,cur->album_img_path);
			strcpy(pdn->coverImgUrl,cur->coverImgUrl);
			strcpy(pdn->info,cur->info);
			strcpy(pdn->ext,cur->ext);
			strcpy(pdn->tag,cur->tag);		
			p_debug("already exist pid=%s",pdn->pid);
			//system("cp -f /tmp/mnt/USB-disk-1/letv/.tasklist /tmp/mnt/USB-disk-1/letv/.tasklist.bak");
			//usleep(100000);
			write_list_to_file(ALBUM);		
			//safe_free(cur);
			return 1;
		}
		p_debug("pdn->pid=%s",pdn->pid);
		pdn=(pdn)->dn_next;
	}
	cur->dn_next = *dn;
	*dn = cur;
	//(*dn)->dn_next=NULL;
	ret=write_list_to_file(ALBUM);
	if(ret<0){
		p_debug("pid add failed");	
	}
	//system("cp -f /tmp/mnt/USB-disk-1/letv/.tasklist /tmp/mnt/USB-disk-1/letv/.tasklist.bak");
	//usleep(100000);

	p_debug("leave add_album_to_list");
	return ret;
}

int del_album_from_list(struct album_node **head,char *pid)  
{  
	p_debug("access del_album_from_list");
	int i=0;
    struct album_node *node1 = *head;  
    struct album_node *node2 = NULL;  
    if (*head==NULL)  
    {  
        return -1;  
    }   
    else  
    {  
        if (!strcmp(node1->pid,pid))
        {  
            *head=(*head)->dn_next;  
			//album_dn=album_dn->dn_next;
			p_debug("del node[%d].pid=%s,pid=%s",i,node1->pid,pid);
            safe_free(node1);  
			write_list_to_file(ALBUM);
			struct task_dnode *pdn=task_dn;
			for(i=0;;i++){
				if(!pdn)break;
				if(!strcmp(pdn->pid,pid))
					{
					pdn->isAutoAdd=0;
//					pdn->isDeleted=;
					saveOneTaskToFile(pdn);
				}
				pdn = pdn->dn_next;
			}
			//system("cp -f /tmp/mnt/USB-disk-1/letv/.tasklist /tmp/mnt/USB-disk-1/letv/.tasklist.bak");
			//usleep(100000);			
			p_debug("leave del_album_from_list");
            return 0;  
        }   
        else  
        {  
            while (node1!=NULL)  
            {  
				p_debug("node[%d].pid=%s,pid=%s",i,node1->pid,pid);
                node2=node1;  
                node2=node2->dn_next;  


                if (!strcmp(node2->pid,pid))  
                {  
					p_debug("del node2[%d].pid=%s,pid=%s",i,node2->pid,pid);

                    node1->dn_next=node2->dn_next;  
                    safe_free(node2);  
                    break;  
                }  


                node1=node1->dn_next;  
				i++;
            }  
			//p_debug("del album succ");

			write_list_to_file(ALBUM);
			//system("cp -f /tmp/mnt/USB-disk-1/letv/.tasklist /tmp/mnt/USB-disk-1/letv/.tasklist.bak");
			//usleep(100000);

			p_debug("leave del_album_from_list");
			return 0;  
        }  
    }  
}


int update_album_status_to_list(
	char *pid,
	int status)
{
	pthread_rwlock_trywrlock(&album_list_lock); 
	p_debug("access update_album_status_to_list");
	struct album_node *dn;
	dn=album_dn;

	unsigned i = 0;
	for(i = 0;/*!dn*/;i++)
	{
		if(!dn)
			break;
		p_debug("PID=%s",(dn)->pid);

			if(!strcmp((dn)->pid,pid))
			{
				(dn)->status = status;
				p_debug("Find PID=%s, update successfully.",dn->pid);
				write_list_to_file(ALBUM);
				
				pthread_rwlock_unlock(&album_list_lock);
				p_debug("leave update_album_status_to_list");
				return 0;
			}
				
		dn = (dn)->dn_next;
	}
	pthread_rwlock_unlock(&album_list_lock);

	p_debug("Can't find the matched PID=%s, update failed.",pid);
	return -1;
}


void display_album_dnode(struct album_node *dn)
{
	unsigned i = 0;
	struct album_node *tmp_dn=dn;
	for(i = 0;/*count - detected via !dn below*/;i++)
	{
		if(!tmp_dn){
			p_debug("album list is empty");
			break;
			
		}			
		//p_debug("album list is empty3");
		p_debug("i=%d,p=%s,info = %s",i,tmp_dn->pid,tmp_dn->info);
		tmp_dn = tmp_dn->dn_next;
		//p_debug("album list is empty2");
	}
}


void display_task_dnode(struct task_dnode *dn)
{
	unsigned i = 0;
	struct task_dnode *tmp_dn=dn;
	for(i = 0;/*count - detected via !dn below*/;i++)
	{
		if(!tmp_dn)
			break;
		p_debug("i=%d,p=%s,v=%s,loaded=%lld,total=%lld,stat=%d",i,tmp_dn->pid,tmp_dn->vid,tmp_dn->downloaded_size,tmp_dn->total_size,tmp_dn->download_status);
		//p_debug("i = %d,dowloadstatus= %d,vid_url = %s",i,dn->download_status,dn->vid_url);		
		//p_debug("i = %d,add_task_time= %d,update_task_time = %d",i,dn->add_task_time,dn->update_task_time);	
		tmp_dn = tmp_dn->dn_next;
	}
	//p_debug("leave display_task_dnode");
}
int add_task_to_list(struct task_dnode **dn,struct task_dnode *cur,int type)
{
	p_debug("access add_task_to_list");

	int i;
	struct tm *newtime;
    char tmpbuf[128];
	struct task_dnode *pdn;
	pdn=task_dn;
	int write_ret;
	p_debug("cur->ext=%s,cur->info=%s",cur->ext,cur->info);
	for(i=0;;i++){
		if(!pdn)break;
		if((!strcmp((pdn)->vid,cur->vid))&&(!strcmp((pdn)->pid,cur->pid))){
		//	if(type==1){//更新自动下载的地址
				

		//	}
			if(type==2)//追剧时添加的(可能自动添加也可能手动添加的)
			{
			
				if(pdn->isAutoAdd=1&&pdn->isDeleted==1)
					p_debug("already deleted PID=%s,VID=%s, auto add stop.",pdn->pid,pdn->vid);
				else{
					pdn->isAutoAdd=1;
					pdn->isDeleted=0;
					memset(pdn->vid_url,0,sizeof(pdn->vid_url));
					strcpy(pdn->vid_url,cur->vid_url);
					p_debug("auto add vid_url=%s",pdn->vid_url);
				}
				//break;
			} 
			else if(type==3){//add from scan
				p_debug("already exist PID=%s,VID=%s, scan add stop.",pdn->pid,pdn->vid);
			}
			else//if(pdn->isDeleted==0) 手动添加
			{
				pdn->isAutoAdd=0;
				strcpy(pdn->vid_url,cur->vid_url);
				strcpy(pdn->img_url,cur->img_url);
				strcpy(pdn->ext,cur->ext);
				if(pdn->isDeleted==1){
					pdn->isDeleted=0;
					pdn->download_status=WAITING;
				}
				//pdn->isAutoAdd=0;//修改为手动添加

				if((pdn->download_status != DONE)&&(pdn->download_status != DOWNLOADING)&&(pdn->download_status != PAUSE)){ 
					pdn->download_status=WAITING;
				//	p_debug("vid=%s,pid=%s",(pdn)->vid,pdn->pid);
				//	write_list_to_file(VIDEO);	

				}
				write_list_to_file(VIDEO);
				saveOneTaskToFile(dn);
				p_debug("already exist PID=%s,VID=%s",pdn->pid,pdn->vid);
				//system("cp -f /tmp/mnt/USB-disk-1/letv/.tasklist /tmp/mnt/USB-disk-1/letv/.tasklist.bak");
				//usleep(100000);
			}
			//safe_free(cur);
			p_debug("leave add_task_to_list");
			return 1;
		}
		pdn=(pdn)->dn_next;
	}

	if(type==2)
		cur->isAutoAdd=1;
	else if(type==1)
		cur->isAutoAdd=0;
	
	cur->dn_next = *dn;
	*dn = cur;
	//(*dn)->dn_next=NULL;
	if((*dn)->add_task_time == 0){
		(*dn)->download_status=WAITING;
		(*dn)->downloaded_size=0;
		(*dn)->total_size=0;
		(*dn)->add_task_time=time(NULL);
		(*dn)->update_task_time=time(NULL);
	}
	//p_debug("dn[%s]->info=[%s]",(*dn)->vid,(*dn)->info);
	//p_debug("add task time = %d", (*dn)->add_task_time);
	newtime=localtime(&(*dn)->add_task_time);
	strftime( tmpbuf, 128, "Today is %A, day %d of %B in the year %Y.\n", newtime);
	//download_img_first=1;
    //p_debug(tmpbuf);
	write_list_to_file(VIDEO);
	//system("cp -f /tmp/mnt/USB-disk-1/letv/.tasklist /tmp/mnt/USB-disk-1/letv/.tasklist.bak");
	//usleep(100000);	
	p_debug("leave add_task_to_list");
	return 0;
}
int update_task_to_list(struct task_dnode *cur)
{
	//p_debug("access update_task_to_list");
	//struct task_dnode *dn=task_dn;
	//unsigned i = 0;

/*	for(i = 0;;i++)
	{
		if(!dn)
			break;
		if(!strcmp(dn->vid,cur->vid))
		{
			dn->total_size = cur->total_size;
			dn->downloaded_size = cur->downloaded_size;
			dn->download_status = cur->download_status;
			dn->update_task_time = time(NULL);
			strcpy(dn->img_path,cur->img_path);
			strcpy(dn->img_url,cur->img_url);
			p_debug("img path=%s",cur->img_path);
			p_debug("img_url=%s",cur->img_url);
			p_debug("vid_url=%s",cur->vid_url);
			p_debug("vid_path=%s",cur->vid_path);
			strcpy(dn->vid_path,cur->vid_path);
			strcpy(dn->img_path,cur->img_path);
			//display_task_dnode(task_dn);

	*/		write_list_to_file(VIDEO);
			saveOneTaskToFile(cur);
			//display_task_dnode(task_dn);

			//p_debug("leave update_task_to_list");		
			return 0;
	/*	}
		dn = dn->dn_next;
	}

	p_debug("leave update_task_to_list");		
	return -1;
	*/
}

int update_task_status_to_list(
	char *pid,
	char *vid,
	int download_status)
{
	pthread_rwlock_trywrlock(&task_list_lock); 
	p_debug("access update_task_status_to_list");
	struct task_dnode *dn=task_dn;

	unsigned i = 0;
	for(i = 0;/*!dn*/;i++)
	{
		if(!dn)
			break;
		//p_debug("VID=%s",(dn)->vid);
		//p_debug("VID=%s",vid);
		
		if(pid==NULL){
			if(!strcmp((dn)->vid,vid))
			{
				if((dn)->download_status==DOWNLOADING){
					updateSysVal("vid","0");
					//system("mcu_control -s 1");
					system("pwm_control 1 1 0;pwm_control 1 0 0");					
					updateSysVal("led_status","1");
					
					updateSysVal("isDownloading","false");

				//p_debug("..........................find the downloading one................");
				}
				(dn)->download_status = download_status;
				(dn)->update_task_time = time(NULL);
				//p_debug("Find PID/VID=%s/%s,status=%d, update successfully1.",dn->pid,dn->vid,(dn)->download_status );
				write_list_to_file(VIDEO);
				saveOneTaskToFile(dn);
				//display_task_dnode(task_dn);
				pthread_rwlock_unlock(&task_list_lock);
				//p_debug("leave update_task_status_to_list");
				return 0;
			}
		}
		else{

			if((!strcmp(dn->vid,vid))&&(!strcmp(dn->pid,pid)))
			{
				(dn)->download_status = download_status;
				(dn)->update_task_time = time(NULL);
				p_debug("Find PID/VID=%s/%s,status=%d, update successfully2.",dn->pid,dn->vid,(dn)->download_status);
				write_list_to_file(VIDEO);
				saveOneTaskToFile(dn);
				//display_task_dnode(task_dn);
				pthread_rwlock_unlock(&task_list_lock);
				p_debug("leave update_task_status_to_list");
				return 0;
			}

		}
		
		dn = (dn)->dn_next;
	}
	pthread_rwlock_unlock(&task_list_lock);

	p_debug("Can't find the matched VID=%s, update failed.",vid);
	return -1;
}

int del_task_from_list(struct task_dnode **head,char *pid,char *vid)  
{  
	p_debug("access del_task_from_list");
    struct task_dnode *dn = *head; 
	int i;
	for(i = 0;/*!dn*/;i++)
	{
		if(!dn)
			break;
		if(!strcmp(dn->vid,vid)){
			//if(dn->isAutoAdd==1){
			//if(dn->pid == NULL){
			if(pid == NULL){
				//if(dn->isAutoAdd ==1)
				dn->isDeleted=1;//标记自动添加的已删除
				dn->total_size=0;
				dn->downloaded_size=0;
				dn->percent=0;
				write_list_to_file(VIDEO);
				saveOneTaskToFile(dn);
				return 0;
			}else{ // 自动追剧中已删除过的视频处理，而且已取消追剧。
				dn->isAutoAdd=0;//取消自动追剧
				dn->isDeleted=0;//标记自动添加的已删除
				dn->total_size=0;
				dn->downloaded_size=0;
				dn->percent=0;
				write_list_to_file(VIDEO);
				//saveOneTaskToFile(dn);
				char del_file_path[128]="\0";
				sprintf(del_file_path,"rm -rf /tmp/mnt/USB-disk-1/hack/%s.vid",dn->vid);		 
				system(del_file_path); 	 
				break;
				//return 0;

			}
		}
		dn=dn->dn_next;
	}
	

    struct task_dnode *node1 = *head; 
    struct task_dnode *node2 = NULL;  
    if (*head==NULL)  
    {  
        return -1;  
    }   
    else  
    {  
        if (!strcmp(node1->vid,vid))
        {  
        	if(pid==NULL){

	            *head=(*head)->dn_next;  
				//task_dn=task_dn->dn_next;
				//p_debug("del PID/VID=%s/%s",node1->pid,node1->vid);
	            safe_free(node1); 
				p_debug("del succ");
				write_list_to_file(VIDEO);					
	            return 0;  
			}else if(!strcmp(node1->pid,pid)){
				*head=(*head)->dn_next;  
				//task_dn=task_dn->dn_next;
				//p_debug("del PID/VID=%s/%s",node1->pid,node1->vid);
	            safe_free(node1); 
				p_debug("del succ");
				write_list_to_file(VIDEO);
	            return 0;  
			}
        }   
        else  
        {  
            while (node1!=NULL)  
            {  
                node2=node1;  
                node2=node2->dn_next; 
				if(pid==NULL){
	                if (!strcmp(node2->vid,vid))  
	                {  
	                    node1->dn_next=node2->dn_next;  
						p_debug("del PID/VID=%s/%s",node2->pid,node2->vid);
	                    safe_free(node2); 
	                    break;  
	                } 
				}else {
				    if ((!strcmp(node2->vid,vid))&&(!strcmp(node2->pid,pid)))  
                	{ 
						node1->dn_next=node2->dn_next;	
						p_debug("del PID/VID=%s/%s",node2->pid,node2->vid);
						safe_free(node2); 
						break;	
				   }
				} 
                node1=node1->dn_next;  
            }  
			p_debug("del succ2");
			write_list_to_file(VIDEO);
			//system("cp -f /tmp/mnt/USB-disk-1/letv/.tasklist /tmp/mnt/USB-disk-1/letv/.tasklist.bak");
			//usleep(100000);			
            return 0;  
        }  
    }  
}

int destory_album_list(struct album_node *dn)
{
	unsigned i = 0;
	struct album_node *head;
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

int write_list_to_file(int type)//写出数据
{
	char path[128]="\0";
	//p_debug("access write_list_to_file");	
	struct album_node *adn=album_dn;
	struct task_dnode *dn=task_dn;

	unsigned i = 0;
	int enRet = 0;
	FILE *record_fd=NULL;

	if(type == ALBUM)
	{
		pthread_rwlock_trywrlock(&album_list_lock);
		memset(path,"0",128);
		strcpy(path,LETV_ALBUMLIST_FILE_PATH);	
	}
	else if(type == VIDEO)
	{
		pthread_rwlock_trywrlock(&task_list_lock); 
		memset(path,"0",128);
		strcpy(path,LETV_TASKLIST_FILE_PATH);
	}

	//display_task_dnode(dn);
	if((record_fd = fopen(path,"w+"))== NULL)//改动：路径
	{
		p_debug("open file error[errno = %d]",errno);
		return -1;
	}

	for(i = 0;/*count - detected via !dn below*/;i++)
	{
		if(type == VIDEO){

			if(!dn)
			{
				//p_debug("task link is NULL");
				break;
			}
			//p_debug("save tasklist pid/vid=%s/%s",dn->pid,dn->vid);
		}else if(type == ALBUM){

			if(!adn)
			{
				p_debug("album link is NULL");
				break;
			}
			p_debug("save albumlist pid/info=%s/%s",adn->pid,adn->info);
		}
		//p_debug("i = %d,dn->vid = %s",i,dn->vid);
		//p_debug("i = %d,dn->vid_path = %s",i,dn->vid_path);
		
		if(type == VIDEO){ 

			//usleep(100000);
			enRet = fwrite(dn,sizeof(struct task_dnode),1,record_fd);
			if(enRet>0){		
				//system("cp /tmp/mnt/USB-disk-1/letv/.tasklist /tmp/mnt/USB-disk-1/letv/.tasklist.bak");
				//usleep(100000);
			}
		}
		else if(type == ALBUM){
			enRet = fwrite(adn,sizeof(struct album_node),1,record_fd);
			
			if(enRet <= 0)
			{
				p_debug("enRet = %d,errno = %d",enRet,errno);
			}else
			{	
				//p_debug("write task node ok......");
			}

		}
		if(type == VIDEO) 		dn = dn->dn_next;
		else if(type == ALBUM) adn = adn->dn_next;

	}
	fclose(record_fd);
	if(type == VIDEO) 		pthread_rwlock_unlock(&task_list_lock);
	else if(type == ALBUM)	pthread_rwlock_unlock(&album_list_lock);
	//p_debug("leave write_list_to_file");	
	//system("cp -f /tmp/mnt/USB-disk-1/letv/.tasklist /tmp/mnt/USB-disk-1/letv/.tasklist.bak");
	//p_debug("cp -f /tmp/mnt/USB-disk-1/letv/.tasklist");	
	//usleep(100000);
	//system("sync");

}

int IsInt(char* str)
{
    p_debug("str=%s",str);
   int len;
   len = strlen(str);
   int i=0;
   if(len==0) return 0;
   if(str[0]=='-') i=1;
   for(i;i<len;i++)
   {
	   if(!(isdigit(str[i])))
		   return 0;
   }
   return 1;
}

 int read_list_from_file(const char *path,struct task_dnode **dn)//读入数据
 {	 
 p_debug("access read_list_from_file");
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
		 p_debug("task list file does not exist error2[errno:%d]",errno);
		 return -1;
	  }
	  
	  while(!feof(record_fd))
	  {
		 cur = xzalloc(sizeof(*cur));
		 cur->dn_next=NULL;
		 enRet = fread(cur,sizeof(struct task_dnode),1,record_fd);
		 if(enRet <= 0)
	     {	
	     	if(cur!=NULL)free(cur);
			p_debug("fread error,enRet = %d,errno = %d",enRet,errno);
			break;
		 }
		
		
		if(!cur)
		{
			p_debug("malloc task_dn error");
			return -2;
		} 
		if(IsInt(cur->vid)){//invalid vid,read next.
			p_debug("i:%d,pid = %s,vid=%s,download_status = %d",i,cur->pid,cur->vid,cur->download_status);
			cur->dn_next = *dn;
			*dn = cur;
				
			i++;
		}else{
			free(cur);
			cur=NULL;
			continue;
		}
	  }
	  fclose(record_fd);
	  p_debug("leave read_list_from_file");

	  return ret;
 }


 int read_album_list_from_file(const char *path,struct album_node**dn)//读入数据
 {	 
 p_debug("access read_album_list_from_file");
	  FILE *record_fd=NULL;
	  int enRet;
	  struct album_node*cur=NULL;
	  int ret = 0;
	  int i = 0;
	  int j = 0;

	  
	  if(access(path,R_OK)!=0){// 不存在
		  if((record_fd = fopen(path,"wb+"))==NULL)//不存在则创建
		  {
			 p_debug("album list file does not exist error1[errno:%d]",errno);
			 return -1;
		  }
	  }else  if((record_fd = fopen(path,"r"))==NULL)//改动：路径
	  {
		 p_debug("album list file does not exist error2[errno:%d]",errno);
		 return -1;
	  }
	  
	  while(!feof(record_fd))
	  {
		 cur = xzalloc(sizeof(*cur));
		 cur->dn_next=NULL;
		if(!cur)
		{
			p_debug("malloc album_node error");
			return -2;
		}		 
		 enRet = fread(cur,sizeof(struct album_node),1,record_fd);
		 if(enRet <= 0)
	     {	
	     	if(cur!=NULL)safe_free(cur);
			p_debug("fread error,enRet = %d,errno = %d",enRet,errno);
			break;
		 }
		 //p_debug("i:%d,pid = %s,status=%d,\next=%s,\ninfo=%s,\ncoverImgUrl=%s,\nalbum_img_path=%s\ntag=%s\n",i,cur->pid,cur->status,cur->ext,cur->info,cur->coverImgUrl,cur->album_img_path,cur->tag);
	
		if((cur->status==REMOVE)||(!IsInt(cur->pid))){// is removed or error
			i++;
			safe_free(cur);
			continue;
		}else {
			cur->dn_next = *dn;
			*dn = cur;
			//(*dn)->dn_next=NULL;
			i++;
			j++;
		}
	  }
	  fclose(record_fd);
	  p_debug("leave read_list_from_file");

	  return j;
 }


