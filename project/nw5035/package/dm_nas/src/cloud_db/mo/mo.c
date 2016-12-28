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

#include "mo.h"


#define MM_MSGOBJ_POOL "msgobj_pool"

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
        DMCLOG_E("new_message failed!");
		return NULL;
	}

	p_msg->msg_header.ret_flag = MSG_SYN_RETURN;
    p_semobj = get_semobj();
    if(p_semobj == NULL)
    {
	    DMCLOG_E("get_semobj failed!");
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
        DMCLOG_E("NULL POINTER");
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
    p_msg = (*pp_msg);
  	safe_free(p_msg);
    p_msg = NULL;
    return RET_SUCCESS;
}



