/*
 * =============================================================================
 *
 *       Filename:  task_base.h
 *
 *    Description:  task base template
 *
 *        Version:  1.0
 *        Created:  2014/7/26 22:59:25
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ye Wenhao (), wenhaoye@126.com
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _TASK_BASEK_H_
#define _TASK_BASEK_H_

#include "mq.h"
#include "base.h"


#ifdef __cplusplus
extern "C"{
#endif

typedef struct
{
    int seq;
	char task_name[16];
	pthread_t task_id;
	ThreadFunc task_func;
	void *task_arg;
}BaseTask;

int create_base_task(BaseTask *task);
int create_detach_task(BaseTask *task);


typedef int (*task_msg_cb)(void *self, void *msg);
typedef void *(*task_loop_func)(void *self);
typedef void (*task_exit_cb)(void *self);

typedef struct _TaskObj
{
    //private member.
	char task_name[32];
	pthread_t task_id;
	MQObj task_mq;
    MSG_TYPE msg_type;

    //public member.
	uint8_t exit_flag;
    task_loop_func task_func;
	task_msg_cb msg_cb;
    task_exit_cb exit_cb;
	void *arg;
}TaskObj;

int create_task(TaskObj *task, const char *name, MSG_TYPE type);


#ifdef __cplusplus
}
#endif


#endif
