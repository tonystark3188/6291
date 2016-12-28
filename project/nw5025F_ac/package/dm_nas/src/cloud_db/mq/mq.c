/*
 * =============================================================================
 *
 *       Filename:  mq.c
 *
 *    Description:  message queue
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

#include "mq.h"

#define MSG_TYPE_INDEX_OFFSET   8


#define MAX_MQ_INDEX ((MSG_TYPE_TEST >> MSG_TYPE_INDEX_OFFSET)+1)

static MQObj * g_mqs[MAX_MQ_INDEX] = {0};

#ifdef SUPPORT_MUTIL_PTHREAD
#define mq_lock_init(lock) pthread_mutex_init((lock), NULL)
#define mq_lock(lock) pthread_mutex_lock((lock))
#define mq_unlock(lock) pthread_mutex_unlock((lock))
#define mq_lock_destroy(lock) pthread_mutex_destroy((lock))

#define mq_cond_init(cond) pthread_cond_init((cond), NULL)
#define mq_cond_wait(cond, lock) pthread_cond_wait((cond), (lock))
#define mq_cond_signal(cond) pthread_cond_signal((cond))
#define mq_cond_destroy(cond) pthread_cond_destroy((cond))

#else
#define mq_lock_init(lock) 
#define mq_lock(lock) 
#define mq_unlock(lock)
#define mq_lock_destroy(lock)

#define mq_cond_init(cond) 
#define mq_cond_wait(cond, lock) 
#define mq_cond_signal(cond) 
#define mq_cond_destroy(cond) 

#endif

extern int exit_flag;
/*
 * Desc: create message queue.
 *
 * p_mq: input/output, point to MQObj.
 * name: input, the name of message queue.
 * Return: success on RET_SUCCESS,
 */
int create_mq(MQObj *p_mq, const char *name)
{
	if(p_mq == NULL || name == NULL)
	{
		DMCLOG_E("NULL pointer!");
		return ENULL_POINT;
	}

	p_mq->is_valid = 0;
	p_mq->msg_nums = 0;
	S_STRNCPY(p_mq->mq_name, name, sizeof(p_mq->mq_name));

	//DMCLOG_D("start(%s)", p_mq->mq_name);
	dl_list_init(&p_mq->mq_head);

	mq_lock_init(&p_mq->mq_lock);
	mq_cond_init(&p_mq->mq_cond);

	//DMCLOG_D("end(%s)", p_mq->mq_name);
	p_mq->is_valid = 1;
    
	return RET_SUCCESS;
}

/*
 * Desc: release message queue. 
 *
 * p_mq: input, point to MQObj.
 */
int release_mq(MQObj *p_mq)
{
	if(p_mq == NULL)
	{
		DMCLOG_E("NULL pointer!");
		return ENULL_POINT;
	}

    if(!p_mq->is_valid)
    {
        DMCLOG_E("%s is not valid!", p_mq->mq_name);
        return EINVAL_OBJ;
    }
    p_mq->is_valid = 0;

	DMCLOG_E("start(%s)", p_mq->mq_name);
	mq_lock(&p_mq->mq_lock);
	p_mq->msg_nums = 0;
	dl_list_init(&p_mq->mq_head);
	mq_unlock(&p_mq->mq_lock);

	mq_lock_destroy(&p_mq->mq_lock);
	mq_cond_destroy(&p_mq->mq_cond);

	DMCLOG_E("end(%s)", p_mq->mq_name);
	return RET_SUCCESS;
}

/*
 * Desc: register message queue into g_mqs.
 *
 * p_mq: input, 
 * msg_type: input,
 * Return: success on RET_SUCCESS,
 */
int register_mq(MQObj *p_mq, MSG_TYPE msg_type)
{
    //DMCLOG_D("Enter");
    
    if(p_mq == NULL)
    {
        DMCLOG_E("NULL point");
        return ENULL_POINT;
    }
    //DMCLOG_D("msg_type = %d",msg_type);
    int index = (msg_type >> MSG_TYPE_INDEX_OFFSET);
	//DMCLOG_D("index = %d",index);
    if(index > MAX_MQ_INDEX)
    {
        DMCLOG_D("mq_index(%d) too big!", index);
        return EOUTBOUND;
    }

    if(g_mqs[index] != NULL)
    {
        DMCLOG_D("mq_index(%d) has used!", index);
        return EEXISTE;
    }

    g_mqs[index] = p_mq;
    //DMCLOG_D("Exit");
    return RET_SUCCESS;
}

/*
 * Desc: unregister message queue from g_mqs.
 *
 * msg_type: input,
 * Return: success on RET_SUCCESS,
 */
int unregister_mq(MSG_TYPE msg_type)
{
    int index = (msg_type >> MSG_TYPE_INDEX_OFFSET);

    if(index > MAX_MQ_INDEX)
    {
        DMCLOG_E("mq_index(%d) too big!", index);
        return EOUTBOUND;
    }
    g_mqs[index] = NULL;
    return RET_SUCCESS;
}

/*
 * Desc: find message queue by message type.
 *
 * msg_type: input,
 * Return: error on NULL, 
 */
static MQObj *_find_mq_by_msg_type(MSG_TYPE msg_type)
{
    int index = (msg_type >> MSG_TYPE_INDEX_OFFSET);
    MQObj *p_mq = NULL;

    if(index > MAX_MQ_INDEX)
    {
        DMCLOG_E("index(%d) too big!", index);
        return NULL;
    }

    if(g_mqs[index] == NULL)
    {
        DMCLOG_E("index(%d) do not used!", index);
        return NULL;
    }

    p_mq = g_mqs[index];
    return p_mq;
}

/*
 * Desc: recv message from queue.
 *
 * p_mq: input/output, point to MQObj.
 * no_wait: input, It signs that whether we should wait until we get a message.
 * Return: success on valid pointer of MsgObj, else on NULL.
 */
static MsgObj * _recv_msg(MQObj *p_mq, uint8_t no_wait)
{
	MsgObj *p_msg = NULL;
	struct dl_list *tmp = NULL;

    if(!p_mq->is_valid)
    {
        DMCLOG_E("%s is not valid!", p_mq->mq_name);
        return NULL;
    }

	mq_lock(&p_mq->mq_lock);

    if(no_wait == 0)
    {
    	while(p_mq->msg_nums == 0&&exit_flag == 0)
    	{
    		mq_cond_wait(&p_mq->mq_cond, &p_mq->mq_lock);
			usleep(100000);//wait 0.1 sec
    	}
    }
	if(p_mq->msg_nums > 0)
	{
		tmp = p_mq->mq_head.next;
        dl_list_del(tmp);
	    p_mq->msg_nums -= 1;
        p_msg = dl_list_entry(tmp, MsgObj, head);
	}
	mq_unlock(&p_mq->mq_lock);
	return p_msg;
}

/*
 * Desc: recv message from specific queue.
 *
 * p_mq: input, point to the queue which we recv message from.
 * Return: success on valid pointer of Message, else on NULL.
 */
Message *recv_message(MQObj *p_mq)
{
    MsgObj *p_msg_header = NULL;
    Message *p_msg = NULL;

    if(p_mq == NULL)
    {
        DMCLOG_E("NULL pointer!");
		return NULL;
    }

    p_msg_header = _recv_msg(p_mq, 0);
    if(p_msg_header == NULL)
    {
        DMCLOG_E("_recv_msg failed!");
        return NULL;
    }

    p_msg = (Message *)(p_msg_header);
    return p_msg;
}

/*
 * Desc: recv message from specific queue. 
 *      If the queue is empty, it will not wait and return directly.
 *
 * p_mq: input, point to the queue which we want to recv message from.
 * Return: error on NULL,
 */
Message *recv_message_no_wait(MQObj *p_mq)
{
    MsgObj *p_msg_header = NULL;
    Message *p_msg = NULL;

    if(p_mq == NULL)
    {
        DMCLOG_E("NULL pointer!");
		return NULL;
    }

    p_msg_header = _recv_msg(p_mq, 1);
    if(p_msg_header == NULL)
    {
        // log_warning("_recv_msg failed!");
        return NULL;
    }

    p_msg = (Message *)(p_msg_header);

    return p_msg;
}

/*
 * Desc: send message to queue.
 *
 * p_mq: input/output, point to the message queue which we send message to.
 * p_msg: input, the message we want to send.
 * Return: success on RET_SUCCESS, 
 */
static int _send_msg(MQObj *p_mq, MsgObj *p_msg)
{
    if(!p_mq->is_valid)
    {
        DMCLOG_D("%s is not valid!", p_mq->mq_name);
        return EINVAL_OBJ;
    }

	DMCLOG_D("start(%s)", p_mq->mq_name);

	mq_lock(&p_mq->mq_lock);
	dl_list_add_tail(&p_mq->mq_head, &p_msg->head);
	p_mq->msg_nums += 1;
	//DMCLOG_D("p_mq->msg_nums = %d",p_mq->msg_nums);
	mq_unlock(&p_mq->mq_lock);
	mq_cond_signal(&p_mq->mq_cond);
    
	DMCLOG_D("end(%s)", p_mq->mq_name);
	return RET_SUCCESS;
}

/*
 * Desc: send message into queue until we get the response.
 * 
 * Notices: We should set m_syn member of the p_msg.
 *
 * p_mq: input/output, point to the message queue which we will send message to.
 * p_msg: input, point to the message we want to send.
 * Return: success on RET_SUCCESS,
 */
static int _send_msg_sync(MQObj *p_mq, MsgObj *p_msg)
{
    if(!p_mq->is_valid)
    {
        DMCLOG_D("%s is not valid!", p_mq->mq_name);
        return EINVAL_OBJ;
    }

	if(p_msg->m_syn == NULL) 
	{
		DMCLOG_E("NULL pointer 2!");
		return ENULL_POINT;
	}

	_send_msg(p_mq, p_msg);
	DMCLOG_E("send msg finish, wait response!");
	//sem_wait(p_msg->m_syn);
	sem_wait(&p_msg->m_syn->sm_sem);
	DMCLOG_E("get response!");

	return RET_SUCCESS;
}

/*
 * Desc: send message
 *
 * p_msg: input, the message we want to send.
 * is_syn: input, It flags us whether we should wait the response.
 * Return: success on RET_SUCCESS,
 */
static int _send_message(Message *p_msg, uint8_t is_syn)
{
	MQObj *p_mq = NULL;

    if(p_msg == NULL)
    {
        DMCLOG_E("NULL pointer!");
		return ENULL_POINT;
    }
    DMCLOG_D("p_msg->msg_header.m_type = %d",p_msg->msg_header.m_type);
	if(p_msg->msg_header.m_type & 0x01) // msg request
	{
		p_mq = _find_mq_by_msg_type(p_msg->msg_header.m_type);
		if(p_mq == NULL)
		{
			DMCLOG_E("find mq by msg_type(%d) failed!", p_msg->msg_header.m_type);
			return -2;
		}	

		if(is_syn)
		{
			return _send_msg_sync(p_mq, &p_msg->msg_header);
		}
		else
		{
			return _send_msg(p_mq, &p_msg->msg_header);
		}
	}
	else // msg response
	{
	    p_mq = (MQObj *)(p_msg->msg_header.m_from);
        if(p_mq == NULL)
		{
			DMCLOG_E("no m_from!");
			return -3;
		}

        return _send_msg(p_mq, &p_msg->msg_header);
	}

	return RET_SUCCESS;
}


int send_message(Message *p_msg)
{
    return _send_message(p_msg, 0);
}

int send_message_sync(Message *p_msg)
{
    return _send_message(p_msg, 1);
}


