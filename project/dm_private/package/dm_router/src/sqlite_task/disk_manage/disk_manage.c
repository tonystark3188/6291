#include "disk_manage.h"
#include "util.h"
struct disk_node *disk_dn;



/*
*	add client usr info to static list
*/
int add_disk_to_disk_list(struct disk_node **dn,struct disk_node *disk_info)
{
	struct disk_node *cur = NULL;
	cur = xzalloc(sizeof(*cur));
	if(!cur)
	{
		DMCLOG_D("add usr error");
		return -1;
	}
	strcpy(cur->uuid,disk_info->uuid);
	strcpy(cur->path,disk_info->path);
	DMCLOG_D("cur->path = %s,cur->uuid = %s",cur->path,cur->uuid);
	cur->dn_next = *dn;
	*dn = cur;
	return 0;
}



/*
*	delete client usr info from static list accorrding to client logout cmd 
*/
int del_disk_from_list_for_uuid(struct disk_node **head,char *uuid)  
{  
	ENTER_FUNC();
    struct disk_node *node1 = *head;  
    struct disk_node *node2 = NULL;  
	DMCLOG_D("session_id = %s",uuid);
	
	if(uuid == NULL&&strlen(uuid)<16)
	{
		DMCLOG_D("session_id invalid");
		return -1;
	}
    if (*head == NULL)  
    {  
        return -1;  
    }   
    else  
    {  
        if (!strcmp(node1->uuid,uuid))
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
                if (!strcmp(node2->uuid,uuid))  
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


int destory_disk_list(struct disk_node *dn)
{
	unsigned i = 0;
	struct disk_node *head;
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

void display_disk_dnode(struct disk_node *dn)
{
	unsigned i = 0;
	for(i = 0;/*count - detected via !dn below*/;i++)
	{
		if(!dn)
			break;
		DMCLOG_D("i = %d,dn->uuid = %s",i,dn->uuid);
		dn = dn->dn_next;
	}
}

int get_disk_dnode_count(struct disk_node *dn)
{
	unsigned i = 0;
	for(i = 0;/*count - detected via !dn below*/;i++)
	{
		if(!dn)
			break;
		dn = dn->dn_next;
	}
	return i;
}
struct disk_node *get_disk_node(char *uuid)
{
	struct disk_node *dn = disk_dn;
	unsigned i = 0;
	for(i = 0;/*count - detected via !dn below*/;i++)
	{
		if(!dn)
			break;
		DMCLOG_D("dn->uuid = %s,uuid = %s",dn->uuid,uuid);
		if(!strcmp(dn->uuid,uuid))
			return dn;
		dn = dn->dn_next;
	}
	return NULL;
}



