
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

#define VIDEO_BUF_LEN 4194304// 1MB 1048576  4MB 4194304

void display_album_dnode(struct album_node *dn)
{
	unsigned i = 0;
	struct album_node *tmp_dn=dn;
	for(i = 0;/*count - detected via !dn below*/;i++)
	{
		if(!tmp_dn){
			break;
			p_debug("album list is empty");
		}
		p_debug("i=%d,p=%s\n,info = %s\n,ext=%s\n",i,tmp_dn->pid,tmp_dn->info,tmp_dn->ext);
		tmp_dn = tmp_dn->dn_next;
	}
}

int read_album_list_from_file(const char *path,struct album_node**dn)//读入数据
{	
printf("access read_album_list_from_file\n");
	 FILE *record_fd=NULL;
	 int enRet;
	 struct album_node*cur=NULL;
	 int ret = 0;
	 int i = 0;
	 int j = 0;

	 
	 if(access(path,R_OK)!=0){// 不存在
		 if((record_fd = fopen(path,"wb+"))==NULL)//不存在则创建
		 {
			printf("album list file does not exist error1[errno:%d]",errno);
			return -1;
		 }
	 }else	if((record_fd = fopen(path,"r"))==NULL)//改动：路径
	 {
		printf("album list file does not exist error2[errno:%d]",errno);
		return -1;
	 }
	 
	 while(!feof(record_fd))
	 {
		cur = (struct album_node *)malloc(sizeof(*cur));
		memset(cur,0,sizeof(*cur));
	   if(!cur)
	   {
		   printf("malloc album_node error");
		   return -2;
	   }		
	   cur->dn_next=NULL;
		enRet = fread(cur,sizeof(struct album_node),1,record_fd);
		if(enRet <= 0)
		{  
		   if(cur!=NULL)free(cur);
		   printf("fread error,enRet = %d,errno = %d",enRet,errno);
		   break;
		}
		printf("\ni:%d,pid = %s,status=%d,\next=%s,\ninfo=%s,\ncoverImgUrl=%s,\nalbum_img_path=%s\n,tag=%s\n,tag2=%s\n,isEnd=%d",\
			i,cur->pid,cur->status,cur->ext,cur->info,cur->coverImgUrl,cur->album_img_path,cur->tag,cur->tag2,cur->isEnd);
   
	   if(cur->status==REMOVE){// is removed
		   i++;
		   free(cur);
		   continue;
	   }else {
		   cur->dn_next = *dn;
		   *dn = cur;
		   i++;
		   j++;
	   }
	 }
	 fclose(record_fd);
	 printf("leave read_list_from_file");

	 return j;
}

int destory_album_list(struct album_node *dn)
{
	unsigned i = 0;
	struct album_node  *head;
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
	printf("free succ\n");
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
	//printf("free succ\n");
	return 0;
}

int read_list_from_file(const char *path,struct task_dnode **dn)//读入数据
 {	 
 //printf("access read_list_from_file\n");
	  FILE *record_fd;
	  int enRet;
	  struct task_dnode *cur=NULL;
	  int ret = 0;
	  int i = 0;
	  if(access(path,R_OK)!=0){// 不存在
		  if((record_fd = fopen(path,"wb+"))==NULL)//改动：路径
		  {
			 printf("task list file does not exist error1[errno:%d]\n",errno);
			 return -1;
		  }
	  }else  if((record_fd = fopen(path,"r"))==NULL)//改动：路径
	  {
		 printf("task list file does not exist error2[errno:%d]\n",errno);
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
			printf("fread error,enRet = %d,errno = %d\n",enRet,errno);
			break;
		 }
		 printf("\nvid[%d]=%s,pid[%d]=%s,\nstatus=%d,\next=%s,\ntag=%s,\nimg_url=%s,\nvid_url=%s,\nvid_re_url=%s\n",i,cur->vid,i,cur->pid,cur->download_status,cur->ext,cur->tag,cur->img_url,cur->vid_url,cur->vid_re_url);
		 printf("info=%s,\ntotal_size=%lld,downloaded_size=%lld,\nvid_path=%s,img_path=%s,error_msg=%s,errorCode=%d\n",cur->info,cur->total_size,cur->downloaded_size,cur->vid_path,cur->img_path,cur->error_msg,cur->errorCode);		
		 printf("add_task_time=%d,update_task_time=%d\n",cur->add_task_time,cur->update_task_time);
 		 printf("total_img_size=%lld,download_img_size=%lld\n",cur->total_img_size,cur->download_img_size);
 		 printf("isDeleted=%d,isAutoAdd=%d\n",cur->isDeleted,cur->isAutoAdd);
		if(!cur)
		{
			printf("malloc task_dn error\n");
			return -2;
		}
		cur->dn_next = *dn;
		*dn = cur;
		 i++;
	  }
	  fclose(record_fd);
	  //printf("leave read_list_from_file\n");

	  return ret;
 }
int readOneTask(char *vid){
	FILE *record_fd;

	char path[256]={0};
	sprintf(path,"%s/%s.vid",LETV_DOWNLOAD_DIR_PATH,vid);
	
	if((record_fd = fopen(path,"r"))==NULL)//改动：路径
	{
		 printf("task file does not exist error2[errno:%d]\n",errno);
		 return -1;
	}

	struct task_dnode *cur;
	cur = (struct task_dnode *)malloc(sizeof(*cur));
	memset(cur,0,sizeof(*cur));
	cur->dn_next=NULL;
	int enRet = fread(cur,sizeof(struct task_dnode),1,record_fd);
	if(enRet <= 0)
	{	
		if(cur!=NULL)free(cur);
		printf("fread error,enRet = %d,errno = %d\n",enRet,errno);
		return;//break;
	}
	printf("\nvid=%s,pid=%s,\nstatus=%d,\next=%s,\ntag=%s,\nimg_url=%s,\nvid_url=%s,\nvid_re_url=%s\n",cur->vid,cur->pid,cur->download_status,cur->ext,cur->tag,cur->img_url,cur->vid_url,cur->vid_re_url);
	printf("info=%s,\ntotal_size=%lld,downloaded_size=%lld,\nvid_path=%s,img_path=%s,error_msg=%s,errorCode=%d\n",cur->info,cur->total_size,cur->downloaded_size,cur->vid_path,cur->img_path,cur->error_msg,cur->errorCode);		
	printf("add_task_time=%d,update_task_time=%d\n",cur->add_task_time,cur->update_task_time);
	printf("total_img_size=%lld,download_img_size=%lld\n",cur->total_img_size,cur->download_img_size);
	printf("total_size=%lld,downloaded_size=%lld\n",cur->total_size,cur->downloaded_size);
	printf("isDeleted=%d,isAutoAdd=%d\n",cur->isDeleted,cur->isAutoAdd);

	free(cur);
	return;
}
void main(int argc ,char **argv)
{
		printf("vid=%s\n",argv[1]);
	
		if(argv[1]!=NULL)
			readOneTask(argv[1]);
		else 
			{
#if 1

		struct task_dnode *task_dn=NULL;
		struct album_node *album_dn=NULL;

		read_list_from_file(LETV_TASKLIST_FILE_PATH,&task_dn);

		destory_task_list(task_dn);
		read_album_list_from_file(LETV_ALBUMLIST_FILE_PATH,&album_dn);
		destory_album_list(album_dn);
#endif
		}
		return ;
}

