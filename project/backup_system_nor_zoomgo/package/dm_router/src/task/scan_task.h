/*
 * =============================================================================
 *
 *       Filename:  scan_task.h
 *
 *    Description:  handle scanning disk.
 *
 *        Version:  1.0
 *        Created:  2015/9/24 15:08:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver ()
 *   Organization:  
 *
 * =============================================================================
 */
#ifndef _SIGNAL_TASK_H_
#define _SIGNAL_TASK_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <pthread.h>
#include "base.h"

int create_scan_task(void *disk_info);



#ifdef __cplusplus
}
#endif


#endif
