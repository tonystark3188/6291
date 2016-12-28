/*
 * =============================================================================
 *
 *       Filename:  sq.c
 *
 *    Description:  sem queue
 *
 *        Version:  1.0
 *        Created:  2014/7/28 16:09:21
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ye Wenhao (), wenhaoye@126.com
 *   Organization:  
 *
 * =============================================================================
 */

#include "sm.h"


typedef struct _SemQueue
{
	SemObj sem_q[MAX_SEM_NUMS];	
	pthread_mutex_t sq_lock;
    uint8_t is_valid;
}SemQueue;

static SemQueue sem_queue;

/*
 * Desc: create sem queue.
 *
 * Return: success on RET_SUCCESS,
 */
int create_sem_queue(void)
{
	uint16_t i = 0;
	uint16_t j = 0;
    int ret;

	MEMSET(sem_queue);
	for(i = 0; i < MAX_SEM_NUMS; ++i)
	{
		if(sem_init(&sem_queue.sem_q[i].sm_sem, 0, 0) < 0)
		{
			DMCLOG_E("sem_init failed!");
            ret = ESEM;
			goto _FAILED;
		}
	}	

	if(pthread_mutex_init(&sem_queue.sq_lock, NULL) < 0)
	{
		DMCLOG_E("init mutex failed!");
        ret = EMUTEX;
		goto _FAILED;
	}

    sem_queue.is_valid = 1;
	return RET_SUCCESS;

_FAILED:
	for(j = 0; j < i; ++j)
	{
		sem_destroy(&sem_queue.sem_q[i].sm_sem);
	}

	return ret;
}

/*
 * Desc: release sem queue.
 *
 * Return: success on RET_SUCCESS,
 */
int release_sem_queue(void)
{
	uint16_t i = 0;

    if(!sem_queue.is_valid)
    {
        DMCLOG_E("sem queue is not vaild!");
        return ESEM_QUEUE;
    }
    sem_queue.is_valid = 0;
    
	pthread_mutex_lock(&sem_queue.sq_lock);
	for(i = 0; i < MAX_SEM_NUMS; ++i)
	{
		sem_destroy(&sem_queue.sem_q[i].sm_sem);
	}	
	pthread_mutex_unlock(&sem_queue.sq_lock);

	pthread_mutex_destroy(&sem_queue.sq_lock);
    
	return RET_SUCCESS;
}

/*
 * Desc; get sem object form sem queue.
 *
 * Return: success on valid pointer at sem object, else on NULL.
 */
SemObj  *get_semobj(void)
{
	SemObj *p_sem_obj = NULL;
	uint16_t i = 0;

    if(!sem_queue.is_valid)
    {
        DMCLOG_D("sem queue is not vaild!");
        return NULL;
    }
	pthread_mutex_lock(&sem_queue.sq_lock);
	for(i = 0; i < MAX_SEM_NUMS; ++i)
	{
		if(sem_queue.sem_q[i].sm_used == 0)
			break;
	}

	if(i < MAX_SEM_NUMS)
	{
		sem_queue.sem_q[i].sm_used = 1;
		p_sem_obj = &sem_queue.sem_q[i];	
	}

	pthread_mutex_unlock(&sem_queue.sq_lock);
	return p_sem_obj;
}

/*
 * Desc: free sem object back to sem queue.
 *
 * pp_sem_obj: input/output, 
 * Return: success on RET_SUCCESS, 
 */
int free_semobj(SemObj **pp_sem_obj)
{
	SemObj *p_sem_obj = NULL;

	if((pp_sem_obj == NULL) || ((*pp_sem_obj) == NULL))
	{
		DMCLOG_E("NULL pointer!");
		return ENULL_POINT;
	}

	p_sem_obj = (*pp_sem_obj);
	p_sem_obj->sm_used = 0;	
    p_sem_obj = NULL;
	return RET_SUCCESS;
}


