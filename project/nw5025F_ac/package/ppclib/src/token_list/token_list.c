#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>
#include <errno.h>
#include "token_list.h"

token_list g_token_list;
#if TEST_MAIN
int main(void)  
{  
	int choice; 
	int ret = 0;
	ret = InitLinklist();
	if(ret)
	{
		l_log_err("InitLinklist failed, ret = %d", ret);
		goto MYEXIT;
	}
	CreateListFront();
	ret = DeleteNodeByVuid(1); 
	l_log_info("del by vuid ret = %d", ret);
	ret = DeleteNodeByVuid(3); 
	l_log_info("del by vuid ret = %d", ret);
	ret = DeleteNodeByVuid(4); 
	l_log_info("del by vuid ret = %d", ret);

	token_t mytoken = 0;
	ret = FindTokenByVuid(2, &mytoken);	
	l_log_info("token by vuid ret = %d", ret);	
	ret = FindTokenByVuid(3, &mytoken);	
	l_log_info("token by vuid ret = %d", ret);
	
	ShowLinklist();  
	
MYEXIT:
	ReleaseLinklist();
	return 0;
}

void CreateListFront()  
{  
	l_log("entry");
	token_t i = 0;
	char str[STR_LEN];
	
	while(1)
	{ 
		l_log_info(" ( nodes = %04lld ) please input:", i);
		gets(str);
		if(strcmp(str, "end") == 0)
		{
			break;
		}
		myattr_type tmpdata;
		tmpdata.token = ++i;
		tmpdata.vuid  = i;
		strncpy(tmpdata.info, str, STR_LEN-1);
		if(0 != AddLinkNode(&tmpdata))
		{
			break;
		}
	}
	return ; 
}  
#endif


int init_lock()
{
	int ret = 0;
#if LINKLIST_LOCK_DM
	ret = pthread_mutex_init(&g_token_list.mutex, NULL);
#endif
	return ret;
}

int get_lock()
{
	int ret = 0;
#if LINKLIST_LOCK_DM
	ret = pthread_mutex_lock(&g_token_list.mutex);
#endif
	return ret;
}

int put_lock()
{
	int ret = 0;
#if LINKLIST_LOCK_DM
	ret = pthread_mutex_unlock(&g_token_list.mutex);
#endif
	return ret;
}

int deinint_lock()
{
	int ret = 0;
#if LINKLIST_LOCK_DM
	ret = pthread_mutex_destroy(&g_token_list.mutex);
	if(ret)
	{
		l_log_err("pthread_mutex_destroy failed, errno = %d", errno);
	}
#endif
	return ret;
}

int CheckLinklist()  
{
	if(LINKLIST_INIT_OK == g_token_list.init_flag)
	{
		return 0;
	}else
	{
		return -1;
	}
}  

int InitLinklist()  
{
	l_log("entry");
	int ret = 0;
	if(CheckLinklist())
	{
		ret = init_lock();
		if(!ret)
		{
			memset(&g_token_list, 0, sizeof(token_list));
			g_token_list.init_flag = LINKLIST_INIT_OK;
		}
	}
	return ret;
}  


void ShowLinklist()  
{  
	l_log_info("entry");
	if(CheckLinklist())
	{
		l_log_err("init linklist failed");
		return ;
	}
	get_lock();

	linklist **h = &(g_token_list.head.next);
	linklist *p;  
	p = *h;  
	while(p != NULL)  
	{  
		l_log_info("token = %04lld, str = %s", p->myvalue.token, p->myvalue.info);  
		p = p->next;  
	}  
	
	put_lock();
}  

/*
 * if h == head, head must be NULL;
 */
int AddLinkNode(myattr_type *data)
{
	int ret = 0;
	
	if(CheckLinklist())
	{
		l_log_err("init linklist failed");
		return -1;
	}
	
	get_lock();
	l_log("entry");
	linklist **h = &(g_token_list.head.next);
	if(NULL == data)
	{
		l_log_err("data is NULL");
		ret = -1;
		goto MYEXIT;
	}
	
	linklist *p = (linklist*)malloc(sizeof(linklist));
	if(!p)
	{
		l_log_err("malloc failed");
		ret = -1;
		goto MYEXIT;
	}

	memcpy(&(p->myvalue), data, sizeof(myattr_type));
	p->next = *h;
	*h = p;	
	
MYEXIT:
	put_lock();
	return ret;
}

int DeleteNodeByValue(void* value)
{	
	int ret = -1;
	if(CheckLinklist())
	{
		return ret;
	}
	l_log("entry");
	linklist *h = &(g_token_list.head);
	linklist *p = h->next, *q = h;
	if(!p)
		return ret;

	get_lock();
	
	
	while(p)
	{
		//l_log("%s-%lld", p->myvalue.info, p->myvalue.token);
		if(p->myvalue.value == value)
		{
			l_log("free node by value = %p, token = %lld", value, p->myvalue.token);
			q->next = p->next;
			free(p);
			return 0;
		}
		q = p;
		p = p->next;
		
	}
	put_lock();

	return ret;
} 

int FindTokenByValue(void* value, token_t *token)
{	
	int ret = -1;
	if(CheckLinklist())
	{
		return ret;
	}
	//l_log("entry");
	linklist *h = &(g_token_list.head);
	linklist *p = h->next, *q = h;
	if(!p)
		return ret;

	get_lock();
	
	
	while(p)
	{
		//l_log("%s-%lld", p->myvalue.info, p->myvalue.token);
		if(p->myvalue.value == value)
		{
			*token = p->myvalue.token;
			//l_log("find token = %lld by value = %p", p->myvalue.token, value);
			return 0;
		}
		q = p;
		p = p->next;
		
	}
	put_lock();

	return ret;
} 

void DeleteLastNode()
{	
	if(CheckLinklist())
	{
		return ;
	}
	//l_log("entry");
	linklist **h = &(g_token_list.head.next);
	if(!(*h))
		return;

	get_lock();
	
	linklist *p = *h;
	*h = p->next;
	l_log("%s", p->myvalue.info);
	free(p);
	
	put_lock();
} 

void ReleaseLinklist()  
{
	if(CheckLinklist())
	{
		return ;
	}
	
	get_lock();
	linklist **h = &(g_token_list.head.next);
	l_log("ReleaseLinklist");
	while(NULL != *h)
	{
		DeleteLastNode(h);
	}  

	put_lock();
	
	deinint_lock();
	g_token_list.init_flag = LINKLIST_UNINIT;
	memset(&g_token_list, 0, sizeof(token_list));
}
