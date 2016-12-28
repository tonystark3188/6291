/*
 * =============================================================================
 *
 *       Filename:  sm.h
 *
 *    Description:  sem queue
 *
 *        Version:  1.0
 *        Created:  2014/7/28 15:13:41
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ye Wenhao (), wenhaoye@126.com
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _SEM_QUEUE_H_
#define _SEM_QUEUE_H_

#include "base.h"

#ifdef __cplusplus
extern "C"{
#endif
#define MAX_SEM_NUMS            128 //6

typedef struct _SemObj
{
	sem_t sm_sem;
	uint16_t sm_used;
}SemObj;


int create_sem_queue(void);
int release_sem_queue(void);
SemObj  *get_semobj(void);
int free_semobj(SemObj **pp_sem_obj);


#ifdef __cplusplus
}
#endif


#endif

