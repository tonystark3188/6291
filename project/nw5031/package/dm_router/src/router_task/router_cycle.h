/*
 * =============================================================================
 *
 *       Filename:  router_inotify.h
 *
 *    Description:  monitor router parameter changed
 *
 *        Version:  1.0
 *        Created:  2015/8/24 16:00
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _ROUTER_CYCLE_H_
#define _ROUTER_CYCLE_H_

#include "msg.h"
#include "router_inotify.h"
#include "hd_route.h"
#include "router_defs.h"


#ifdef __cplusplus
extern "C"{
#endif

typedef struct inotify_power_info{
	power_info_t power_info;
	pthread_mutex_t mutex;
}inotify_power_info_t;

#define ROUTER_CYCLE_TIME_MIN 500000  /* us */
#define ROUTER_CYCLE_POWER_TIMES 	6
#ifdef DM_CYCLE_DISK
#define ROUTER_CYCLE_DISK_TIMES 	6
#endif
int dm_router_cycle(void);

#ifdef __cplusplus
}
#endif

#endif

