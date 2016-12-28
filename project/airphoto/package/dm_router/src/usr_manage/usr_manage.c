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
#include "base.h"
#include "router_task.h"
#include "util.h"
struct usr_dnode *usr_dn;



/*
*	add client usr info to static list
*/
int add_usr_to_list(struct usr_dnode **dn,char *session_id,char *ip,char *username,char *password,uint32_t cur_time)
{
	struct usr_dnode *cur = NULL;
	cur = xzalloc(sizeof(*cur));
	if(!cur)
	{
		DMCLOG_D("add usr error");
		return -1;
	}
	strcpy(cur->session,session_id);
	strcpy(cur->ip,ip);
	strcpy(cur->password,password);
	strcpy(cur->username,username);
	cur->cur_time = cur_time;
	cur->dn_next = *dn;
	*dn = cur;
	return 0;
}

/*
*	delete client usr info from static list accorrding to client logout cmd 
*/
int del_usr_from_list_for_session(struct usr_dnode **head,char *session_id)  
{  
	ENTER_FUNC();
    struct usr_dnode *node1 = *head;  
    struct usr_dnode *node2 = NULL;  
	DMCLOG_D("session_id = %s",session_id);
	
	if(session_id == NULL&&strlen(session_id)<16)
	{
		DMCLOG_D("session_id invalid");
		return -1;
	}
    if (*head == NULL)  
    {  
    	DMCLOG_D("access NULL");
        return -1;  
    }   
    else  
    {  
        if (!strcmp(node1->session,session_id))
        {  
            *head=(*head)->dn_next;  
            safe_free(node1);  
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
                if (!strcmp(node2->session,session_id))  
                {  
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

/*
*	delete client usr info from static list accorrding to auto descovery module 
*/
int del_usr_from_list_for_descovery(struct usr_dnode **head,char *ip)  
{ 
	ENTER_FUNC();
    struct usr_dnode *node1=*head;  
    struct usr_dnode *node2=NULL;  
    if (*head==NULL)  
    { 
    	DMCLOG_D("usr dnode null");
        return -1;  
    }   
    else  
    { 
    	DMCLOG_D("node1->ip = %s,ip = %s",node1->ip,ip);
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
					DMCLOG_D("no ip confort");
					break;
				}
                if (!strcmp(node2->ip,ip))  
                {  
                    node1->dn_next=node2->dn_next;  
                    free(node2);  
                    break;  
                }  
                node1=node1->dn_next;  
            }  
            return 0;  
        }  
    }  
	EXIT_FUNC();
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
		DMCLOG_D("i = %d,dn->session = %s,dn->ip = %s",i,dn->session,dn->ip);
		dn = dn->dn_next;
	}
}


