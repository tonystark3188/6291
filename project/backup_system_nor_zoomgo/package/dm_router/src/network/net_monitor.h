/*
 * =============================================================================
 *
 *       Filename:  net_monitor.h
 *
 *    Description:  monitor network if ok
 *
 *        Version:  1.0
 *        Created:  2015/1/5 10:09:38
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ye Wenhao (), wenhaoye@126.com
 *   Organization:  
 *
 * =============================================================================
 */
#ifndef _NETWORK_MONITOR_H_
#define _NETWORK_MONITOR_H_

#ifdef __cplusplus
extern "C"{
#endif


#define ETH_NETWORK_STATUS_FILE "/tmp/linkup"
int init_network_monitor(const char *path);


#ifdef __cplusplus
}
#endif

#endif
