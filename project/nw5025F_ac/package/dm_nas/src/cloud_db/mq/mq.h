/*
 * =============================================================================
 *
 *       Filename:  mq.h
 *
 *    Description:  message queue
 *
 *        Version:  1.0
 *        Created:  2014/7/25 13:13:41
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ye Wenhao (), wenhaoye@126.com
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _MESSAGE_QUEUE_H_
#define _MESSAGE_QUEUE_H_

#include "base.h"
#include "mo.h"
#include "list.h"


#ifdef __cplusplus
extern "C"{
#endif


typedef struct _MQObj
{
	struct dl_list mq_head;
	char mq_name[32];
	uint16_t msg_nums;
    uint8_t is_valid;
	pthread_mutex_t mq_lock;
	pthread_cond_t mq_cond;

}MQObj;


int create_mq(MQObj *p_mq, const char *name);
int release_mq(MQObj *p_mq);
int register_mq(MQObj *p_mq, MSG_TYPE msg_type);
int unregister_mq(MSG_TYPE msg_type);

int send_message(Message *p_msg);
int send_message_sync(Message *p_msg);
Message *recv_message(MQObj *p_mq);
Message *recv_message_no_wait(MQObj *p_mq);



#ifdef __cplusplus
}
#endif


#endif

