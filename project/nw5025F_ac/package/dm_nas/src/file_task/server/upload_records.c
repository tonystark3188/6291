/*
 * =============================================================================
 *
 *       Filename:  upload_records.c
 *
 *    Description:  利用fwrite将链表中的数据写入到一个文件
 *
 *        Version:  1.0
 *        Created:  2015/9/7 10:54
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 515296288jf@163.com
 *   Organization:  
 *
 * =============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "util.h"
#include "upload_records.h"



/*
*	add client usr info to static list
*/
int add_record_to_list(struct record_dnode **dn,int index,off_t start,off_t end)
{
	struct record_dnode *cur = NULL;
	cur = xzalloc(sizeof(*cur));
	if(!cur)
	{
		DMCLOG_D("add record error");
		return -1;
	}
	cur->index = index;
	cur->start = start;
	cur->end = end;
	cur->dn_next = *dn;
	*dn = cur;
	return 0;
}
/*
*	 delete record info from list accorrding to client logout cmd 
*/
int del_record_from_list_for_index(struct record_dnode **head,off_t end)  
{	
	ENTER_FUNC();
	struct record_dnode *node1 = *head;  
	struct record_dnode *node2 = NULL;  
	if (*head == NULL)  
	{	
		DMCLOG_D("access NULL");
		return -1;  
	}	 
	else  
	{
		if (node1->end== end)
		{	
		 *head=(*head)->dn_next; 
		 DMCLOG_D("node1->start = %lu",node1->start);
		 safe_free(node1);	
		 node1 = NULL;
		 DMCLOG_D("delete dnode success");
		 EXIT_FUNC();
		 return 0;	
		}	 
		else  
		{	
			while (node1!=NULL)  
			{	
				node2=node1;  
				node2=node2->dn_next;	
				if (node2 != NULL&&node2->end == end)  
				{	
					DMCLOG_D("node2->start = %lu",node2->end);
					node1->dn_next=node2->dn_next;  
					safe_free(node2);	
					break;  
				}
				node1=node1->dn_next;	
			}	
		 DMCLOG_D("del succ");
		 EXIT_FUNC();
		 return 0;	
		}	
	}	
}


 int destory_record_list(struct record_dnode *dn)
 {
	 unsigned i = 0;
	 struct record_dnode *head;
	 if(dn == NULL)
		 return -1;
	 for(i = 0;/*!dn*/;i++)
	 {
		 head = dn->dn_next;
		 free(dn);
		 if(!head)
			 break;
		 dn = head;
	 }
	 dn = NULL;
	 DMCLOG_D("free succ");
	 return 0;
 }
 
 void display_record_dnode(struct record_dnode *dn)
 {
	 unsigned i = 0;
	 for(i = 0;/*count - detected via !dn below*/;i++)
	 {
		 if(!dn)
		 {
			break;
		 }
		 DMCLOG_D("i = %d,dn->index = %d,dn->start = %lld,dn->end = %lld",i,dn->index,dn->start,dn->end);
		 dn = dn->dn_next;
	 }
 }


 int write_list_to_stream(VFILE *record_fd,struct record_dnode *dn)//写出数据
 {
	 unsigned i = 0;
	 int n;
	 if(record_fd == NULL)
	 {
	 	DMCLOG_E("para is null");
		return -1;
	 }
	 bfavfs_fseek(record_fd,0L,SEEK_SET);
	 for(i = 0;/*count - detected via !dn below*/;i++)
	 {
		 if(!dn)
		 {
			 DMCLOG_D("link is NULL");
			 break;
		 }
		 DMCLOG_D("i = %d,dn->index = %d,dn->start=%lld",i,dn->index,dn->start);
		 if((n = bfavfs_fwrite(dn,sizeof(struct record_dnode),1,record_fd)) != 1)
		 {
			 DMCLOG_D("fwrite errno = %d",errno);
			 return -1;
		 }
		 //DMCLOG_D("n = %d,errno = %d",n,errno);
		 dn = dn->dn_next;
	 }
	 return 0;
 }
  

 
 int read_list_from_file(const char *path,struct record_dnode **dn,void *token)//读入数据
 {	 
	  VFILE *record_fd;
	  int enRet;
	  struct record_dnode *cur;
	  int ret = 0;
	  int i = 0;
	  BucketObject *sObject = build_bucket_object(path,token);
	  if(sObject == NULL)
	  {
		DMCLOG_E("buile bucket object error");
		return -1;
	  }
	  if((record_fd = _bfavfs_fopen(sObject,"r",token))==NULL)//改动：路径
	  {
		 DMCLOG_D("fopen error[errno:%d]",errno);
		 return -1;
	  }
	  while(!bfavfs_feof(record_fd))
	  {
		 cur = xzalloc(sizeof(*cur));
		 enRet = bfavfs_fread(cur,sizeof(struct record_dnode),1,record_fd);
		 if(enRet <= 0)
	     {
			DMCLOG_D("fread error,enRet = %d,errno = %d",enRet,errno);
			break;
		 }
		 DMCLOG_D("index = %d,start = %lld,end = %lld",cur->index,cur->start,cur->end);
		
		if(!cur)
		{
			DMCLOG_D("add record error");
			safe_free(sObject);
			_bfavfs_fclose(record_fd,token);
			return -1;
		}
		cur->dn_next = *dn;
		*dn = cur;
		 i++;
	  }
	  safe_free(sObject);
	  _bfavfs_fclose(record_fd,token);
	  return ret;
 }
 
 int update_record_for_index(struct record_dnode **head,off_t end,off_t cur)
 {
	 struct record_dnode *node1 = *head;
	 if (*head == NULL)
	 {
		 DMCLOG_D("access NULL");
		 return -1;
	 }
	 else
	 {
		 if (node1->end == end)
		 {
			 node1->start = cur;
			 DMCLOG_D("update dnode success, node1->start = %lld,node1->end = %lld", node1->start,node1->end);
			 return 0;
		 }
		 else
		 {
			 while (node1 != NULL)
			 {
				 if (node1->end == end)
				 {
					 DMCLOG_D("node1->end = %lld",node1->end);
					 node1->start = cur;
					 DMCLOG_D("update succ, node1->start = %lld", node1->start);
					 break;  
				 }
				 node1=node1->dn_next;	 
			 }
			 EXIT_FUNC();
			 return 0;	 
		 }	 
	 }	 
 }

