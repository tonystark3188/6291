/*
 * =============================================================================
 *
 *       Filename:  task_base.c
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

#include "task_base.h"
extern int exit_flag;
/*
 * Desc: task entry function.
 *
 * self: input,
 */
static void *_task_entry_func(void *self)
{
	ENTER_FUNC();
	TaskObj *task = (TaskObj *)self;	
    Message *p_msg = NULL;
	int ret = -1;
	while(exit_flag == 0)
	{
		// 1. recv msg from message queue;
		if((p_msg = recv_message(&task->task_mq)) == NULL)
		{
			DMCLOG_E("recv_msg failed!");
			exit_flag = 1;
			continue;
		}
		// 2. handle msg depend on msg_type;
		ret = (*task->msg_cb)(task, p_msg);	
		if(ret < 0)
		{
			DMCLOG_D("handle msg failed(0x%x)", ret);
		}
	}
    if(task->exit_cb != NULL)
    {
        (*(task->exit_cb))(task);
    }

    unregister_mq(task->msg_type);
	release_mq(&task->task_mq);
	EXIT_FUNC();
	return 0;
}

/*
 * Desc: create base task on task object.
 *
 * task: input, point to BaseTask object.
 * Return: success on RET_SUCCESS,
 */
int create_base_task(BaseTask *task)
{
	if(task == NULL || task->task_func == NULL)
	{
		DMCLOG_E("NULL point");
		return ENULL_POINT;
	}

	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	
	if(pthread_create(&tid, &attr, task->task_func, task->task_arg) != 0)
	{
		DMCLOG_D("pthread_create failed!");
		pthread_attr_destroy(&attr);
		return ETHREAD;
	}
	
	PTHREAD_JOIN(tid,NULL);
	task->task_id = tid;
	pthread_attr_destroy(&attr);

	return RET_SUCCESS;
}
/*
 * Desc: create base task on task object.
 *
 * task: input, point to BaseTask object.
 * Return: success on RET_SUCCESS,
 */
int create_detach_task(BaseTask *task)
{
    if(task == NULL || task->task_func == NULL)
    {
        DMCLOG_E("NULL point");
        return ENULL_POINT;
    }
    
    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    
    if(pthread_create(&tid, &attr, task->task_func, task->task_arg) != 0)
    {
        DMCLOG_D("pthread_create failed!");
        pthread_attr_destroy(&attr);
        return ETHREAD;
    }
    
    PTHREAD_DETACH(tid);
    task->task_id = tid;
    pthread_attr_destroy(&attr);
    
    return RET_SUCCESS;
}
/*
 * Desc: create task which has a message queue.
 *
 * Notices: task must init task_name member.
 * task: input/output, 
 */
int create_task(TaskObj *task, const char *name, MSG_TYPE type)
{
	//ENTER_FUNC();
	pthread_t task_id;
	pthread_attr_t task_attr;
    task_loop_func loop_func = NULL;
	int ret;

	if(task == NULL || name == NULL)
	{
		DMCLOG_E("NULL pointer!");
		return ENULL_POINT;
	}

	S_STRNCPY(task->task_name, name, sizeof(task->task_name));
    task->msg_type = type;

    //DMCLOG_D("start(%s)", task->task_name);
	if((ret = create_mq(&task->task_mq, task->task_name)) != RET_SUCCESS)
	{
		DMCLOG_D("init_mq failed(%d)", ret);		
		return ret;
	}

    if((ret = register_mq(&task->task_mq, type)) != RET_SUCCESS)
    {
        DMCLOG_D("register_mq failed, ret(%x)", ret);
        release_mq(&task->task_mq);
        return ret;
    }
	pthread_attr_init(&task_attr);
	pthread_attr_setdetachstate(&task_attr, PTHREAD_CREATE_DETACHED);

    if(task->task_func != NULL)
    {
        loop_func = task->task_func;    
    }
    else
    {
        loop_func = _task_entry_func;
    }

    task->exit_flag = 0;
    if(pthread_create(&task_id, &task_attr, loop_func, task) != 0)
    {
        DMCLOG_D("pthread_create failed!");
        unregister_mq(type);
        release_mq(&task->task_mq);
		ret = ETHREAD;
    }
    else
	{
		//DMCLOG_D("pthread_create success, tid(0x%08x)", task_id);
        ret = RET_SUCCESS;
        task->task_id = task_id;
	}
	
	pthread_attr_destroy(&task_attr);
	return ret;
}


