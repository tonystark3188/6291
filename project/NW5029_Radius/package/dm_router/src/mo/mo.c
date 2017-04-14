/*
 * =============================================================================
 *
 *       Filename:  mo.c
 *
 *    Description:  message object
 *
 *        Version:  1.0
 *        Created:  2014/7/25 19:09:21
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ye Wenhao (), wenhaoye@126.com
 *   Organization:  
 *
 * =============================================================================
 */

#include "mo/mo.h"
#include "sm/sm.h"


#define MM_MSGOBJ_POOL "msgobj_pool"

#ifdef MEM_DEBUG

typedef struct
{
	struct dl_list head;
	uint8_t cnt;
	pthread_mutex_t lock;
}msg_debug_list_t;


static msg_debug_list_t g_msg_debug_list;
static int _init_msg_debug_list(void)
{
	g_msg_debug_list.cnt = 0;
	dl_list_init(&g_msg_debug_list.head);
	lock_init(&g_msg_debug_list.lock);

	return RET_SUCCESS;
}

static int _free_msg_debug_list(void)
{
	lock_destroy(&g_msg_debug_list.lock);

	return RET_SUCCESS;
}

static int _msg_debug_list_add(Message *p)
{
	lock(&g_msg_debug_list.lock);
	dl_list_add(&g_msg_debug_list.head, &p->debug_next);
	g_msg_debug_list.cnt++;
	unlock(&g_msg_debug_list.lock);

	return RET_SUCCESS;
}

static int _msg_debug_list_remove(Message *p)
{
	lock(&g_msg_debug_list.lock);
	dl_list_del(&p->debug_next);
	g_msg_debug_list.cnt--;
	unlock(&g_msg_debug_list.lock);

	return RET_SUCCESS;
}

static int _foreach_debug_list(void)
{
	Message *p_msg;
	time_t t = time(NULL);

	dl_list_for_each(p_msg,&g_msg_debug_list.head, Message, debug_next)
	{
		log_debug("message type:%u", p_msg->msg_header.m_type);
		log_debug("message duration time:%u", t - p_msg->crt_time);
	}

	return RET_SUCCESS;
}

#else
static int _init_msg_debug_list(void)
{
	return RET_SUCCESS;
}

static int _free_msg_debug_list(void)
{
	return RET_SUCCESS;
}

static int _msg_debug_list_add(Message *p)
{
	return RET_SUCCESS;
}

static int _msg_debug_list_remove(Message *p)
{
	return RET_SUCCESS;
}

static int _foreach_debug_list(void)
{
	return RET_SUCCESS;
}
#endif




/*
 * Desc: new a message.
 *
 * no_wait: input,
 * Return: success on valied Message pointer.
 */
static Message *_new_message(uint8_t no_wait)
{
	Message *p_msg =  (Message *)calloc(1,sizeof(Message));
    if(p_msg == NULL)
   	{
		DMCLOG_D("malloc error");
		return NULL;
	}
#ifdef MEM_DEBUG
	_msg_debug_list_add(p_msg);
	p_msg->crt_time = time(NULL);
#endif

    //DMCLOG_D("Exit, m_ptr=0x%x", (unsigned int)(p_msg));
    return p_msg;
}

Message *new_message(void)
{
    return _new_message(0);
}

Message *create_sync_message(void)
{
	Message *p_msg;
	SemObj *p_semobj;
	p_msg = new_message();
	if(p_msg == NULL)
    {
        log_warning("new_message failed!");
		return NULL;
	}

	p_msg->msg_header.ret_flag = MSG_SYN_RETURN;
    p_semobj = get_semobj();
    if(p_semobj == NULL)
    {
	    log_warning("get_semobj failed!");
        free_message(&p_msg);
        return NULL;
    }
    p_msg->msg_header.m_syn = p_semobj;
	return p_msg;
}


int free_sync_message( Message **pp_msg)
{
    int ret;
	
    if((pp_msg == NULL) || ((*pp_msg) == NULL))
    {
        log_warning("NULL POINTER");
        return ENULL_POINT;
    }
	if((ret = free_semobj(&(*pp_msg)->msg_header.m_syn)) != RET_SUCCESS)
	{
		return ret;
	}

	return free_message(pp_msg);
}


Message *new_message_no_wait(void)
{
    return _new_message(1);
}

/*
 * Desc: free a message 
 *
 * pp_msg: input/output. 
 * Return:
 */
int free_message(Message **pp_msg)
{
    Message *p_msg = NULL;
#ifdef MEM_DEBUG
	_msg_debug_list_remove(*pp_msg);
#endif

    p_msg = (*pp_msg);
  	safe_free(p_msg);
    p_msg = NULL;
    return RET_SUCCESS;
}



