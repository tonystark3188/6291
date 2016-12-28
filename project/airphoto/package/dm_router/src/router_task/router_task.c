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
#include "base.h"
#include "usr_manage.h"
#include "util.h"
#include "get_file_list.h"

struct hd_dnode *router_dn;
void display_hd_dnode(struct hd_dnode *dn)
{
	unsigned i = 0;
	for(i = 0;/*count - detected via !dn below*/;i++)
	{
		if(!dn)
			break;
		DMCLOG_D("i = %d,dn->session = %s,dn->port = %d",i,dn->session,dn->port);
		dn = dn->dn_next;
	}
}
extern struct usr_dnode *usr_dn;
static int get_ip_from_usr_list(_In_ const char *session,_Out_ char *ip)
{
	unsigned i = 0;
	int res = -1;
	if(usr_dn == NULL)
		return -1;
	for(i = 0;/*i < counr detected via !dn below*/;i++)
	{
		DMCLOG_D("i = %d,session = %s,ip = %s",i,usr_dn->session,usr_dn->ip);
		if(!strcmp(session,usr_dn->session))
		{
			strcpy(ip,usr_dn->ip);
			res = 0;
			goto exit;
		}
		usr_dn = usr_dn->dn_next;
		if(!usr_dn)
			break;
	}
exit:
	return res;
}

int add_dev_to_list(struct hd_dnode **dn,char *session_id,uint16_t port,uint8_t request_type)
{
	ENTER_FUNC();
	int res = 0;
	char ip[32];
	unsigned i = 0;
	#if 0
	res = get_ip_from_usr_list(session_id,ip);
	#else
	memset(ip,0,32);
	DMCLOG_D("session_id = %s",session_id);
	res = get_ip_from_usr_table(session_id,ip);
	#endif
	DMCLOG_D("res = %d",res);
	if(res < 0)
	{
		DMCLOG_D("get ip error");
		return -1;
	}
	struct hd_dnode *head = *dn;
	for(i = 0;/*!dn*/;i++)
	{
		if(!*dn)
			break;
		if(!strcmp((*dn)->ip,ip))
		{
			DMCLOG_D("ip = %s,session_id = %s",ip,session_id);
			strcpy((*dn)->session,session_id);
			(*dn)->port = port;
			(*dn)->request_type = request_type;
			return 0;
		}
		*dn = (*dn)->dn_next;
	}
	DMCLOG_D("ip = %d",ip);
	*dn = head;
	struct hd_dnode *cur;
	cur = xzalloc(sizeof(*cur));
	if(!cur)
	{
		DMCLOG_D("add dev error");
		return -1;
	}
	strcpy(cur->session,session_id);
	strcpy(cur->ip,ip);
	DMCLOG_D("port = %d",port);
	cur->port = port;
	cur->request_type = request_type;
	cur->dn_next = *dn;
	*dn = cur;
	return 0;
}
int del_dev_from_list(struct hd_dnode **head,char *session_id)  
{  
    struct hd_dnode *node1 = *head;  
    struct hd_dnode *node2 = NULL;  
    if (*head==NULL)  
    {  
        return NULL;  
    }   
    else  
    {  
        if (!strcmp(node1->session,session_id))
        {  
            *head=(*head)->dn_next;  
            safe_free(node1);  
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
            return 0;  
        }  
    }  
}

int del_dev_from_list_for_ip(struct hd_dnode **head,char *ip)  
{  
    struct hd_dnode *node1 = *head;  
    struct hd_dnode *node2 = NULL;  
    if (*head==NULL)  
    {  
        return NULL;  
    }   
    else  
    {  
        if (!strcmp(node1->ip,ip))
        {  
            *head=(*head)->dn_next;  
            safe_free(node1);  
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
					DMCLOG_D("no confort ip");
					break;
				}
                if (!strcmp(node2->ip,ip))  
                {  
                    node1->dn_next=node2->dn_next;  
                    safe_free(node2);  
                    break;  
                }  
                node1=node1->dn_next;  
            }  
			DMCLOG_D("del succ");
            return 0;  
        }  
    }  
}



int destory_router_list(struct hd_dnode *dn)
{
	unsigned i = 0;
	struct hd_dnode *head;
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
	DMCLOG_D("free succ");
	return 0;
}

int dm_get_status_changed()
{
	int statusFlag = 0x000111;
	return statusFlag;
}

