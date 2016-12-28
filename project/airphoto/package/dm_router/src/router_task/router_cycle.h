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


#ifdef __cplusplus
extern "C"{
#endif

#define ROUTER_CYCLE_TIME_MIN 500000  /* us */
#define ROUTER_CYCLE_POWER_TIMES 6

int dm_router_cycle(void);

#ifdef __cplusplus
}
#endif

#endif

