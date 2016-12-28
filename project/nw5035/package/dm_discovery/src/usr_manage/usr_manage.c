/*
 * =============================================================================
 *
 *       Filename:  usr_manage.c
 *
 *    Description:  user infomation process for dm init module
 *
 *        Version:  1.0
 *        Created:  2015/08/21 10:41
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver, 
 *   Organization:  
 *
 * =============================================================================*/
#include "usr_manage.h"
#include "server.h"
#include "my_debug.h"

struct usr_dnode *usr_dn;



int update_usr_count(struct usr_dnode **dn,char *ip)
{
	//ENTER_FUNC();
	time_t timep;
	struct tm *p;
	 struct usr_dnode *node1 = *dn;
	 if (*dn == NULL)
	 {
		 //DMCLOG_D("access NULL");
		 return 0;
	 }
	 else
	 {
		 if(!strcmp(node1->ip,ip))
		 {
			 //DMCLOG_D("node1->ip = %s",node1->ip);
			 time(&timep);
			 p = localtime(&timep);
			 timep = mktime(p);
			 //DMCLOG_D("time()->localtime()->mktime():%d",timep);
			 node1->start_time = timep;
			 //node1->count++;
			 //DMCLOG_D("update dnode success");
			 //EXIT_FUNC();
			 return 0;
		 }
		 else
		 {
			 while (node1 != NULL)
			 {
				 if (!strcmp(node1->ip,ip))
				 {
					 //DMCLOG_D("node1->end = %ip",ip);
					 time(&timep);
					 p = localtime(&timep);
					 timep = mktime(p);
					 //DMCLOG_D("time()->localtime()->mktime():%d",timep);
					 node1->start_time = timep;
					 //node1->count++;
					 //DMCLOG_D("update succ");
					 break;  
				 }
				 node1=node1->dn_next;	 
			 }
			 //EXIT_FUNC();
			 return 0;	 
		 }	 
	 }	 
}

int update_usr_for_ip(struct usr_dnode **dn,char *ip,uint32_t start_time)
{
	 //ENTER_FUNC();
	 struct usr_dnode *cur = NULL;
	 struct usr_dnode *node1 = *dn;
	 if (*dn == NULL)
	 {
		 DMCLOG_D("access NULL");
		 goto ADD_USR;
	 }
	 else
	 {
		 if(!strcmp(node1->ip,ip))
		 {
			 //DMCLOG_D("node1->ip = %s",node1->ip);
			 node1->start_time = start_time;
			 //node1->count = 0;
			 //DMCLOG_D("update dnode success");
			 goto FINISH;
		 }
		 else
		 {
			 while (node1 != NULL)
			 {
				 if (!strcmp(node1->ip,ip))
				 {
					 //DMCLOG_D("node1->end = %ip",ip);
					 node1->start_time = start_time;
					 //node1->count = 0;
					 //DMCLOG_D("update succ");
					 break;  
				 }
				 node1=node1->dn_next;	 
			 }
			 goto FINISH; 
		 }	 
	 }	 
ADD_USR:
	
	cur = xzalloc(sizeof(*cur));
	if(!cur)
	{
		DMCLOG_D("add usr error");
		return -1;
	}
	strcpy(cur->ip,ip);
	cur->start_time = start_time;
	cur->dn_next = *dn;
	//cur->count = 0;
	*dn = cur;
FINISH:
	return 0;
}

/*
*	add client usr info to static list
*/
int add_usr_to_list(struct usr_dnode **dn,char *ip,uint32_t cur_time)
{
	
	struct usr_dnode *cur = NULL;
	cur = xzalloc(sizeof(*cur));
	if(!cur)
	{
		DMCLOG_D("add usr error");
		return -1;
	}
	strcpy(cur->ip,ip);
	cur->cur_time = cur_time;
	cur->dn_next = *dn;
	*dn = cur;
	return 0;
}
/*
*	delete client usr info from static list accorrding to auto descovery module 
*/
int del_usr_from_list_for_ip(struct usr_dnode **head,char *ip)  
{  
    struct usr_dnode *node1=*head;  
    struct usr_dnode *node2=NULL;  
    if (*head==NULL)  
    {  
        return -1;  
    }   
    else  
    {  
        if (!strcmp(node1->ip,ip))
        {  
            *head=(*head)->dn_next;  
            free(node1);  
            return 0;  
        }   
        else  
        {  
            while (node1!=NULL)  
            {  
                node2=node1;  
                node2=node2->dn_next; 
				if(node2 == NULL)
				{
					DMCLOG_D("no comfort ip");
				}
                if (!strcmp(node2->ip,ip))  
                {  
                    node1->dn_next=node2->dn_next;  
                    free(node2);  
                    break;  
                }  
				
                node1=node1->dn_next;  
            }  
			DMCLOG_D("del succ");
            return 0;  
        }  
    }  
}



int destory_usr_list(struct usr_dnode *dn)
{
	unsigned i = 0;
	struct usr_dnode *head;
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
	DMCLOG_D("free succ");
	return 0;
}

void display_usr_dnode(struct usr_dnode *dn)
{
	unsigned i = 0;
	for(i = 0;/*count - detected via !dn below*/;i++)
	{
		if(!dn)
			break;
		DMCLOG_D("i = %d,dn->ip = %s",i,dn->ip);
		dn = dn->dn_next;
	}
}



