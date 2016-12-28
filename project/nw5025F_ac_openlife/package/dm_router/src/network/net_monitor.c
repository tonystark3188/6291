/*
 * =============================================================================
 *
 *       Filename:  net_monitor.c
 *
 *    Description:  monitor network if ok
 *
 *        Version:  1.0
 *        Created:  2015/1/5 10:09:15
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ye Wenhao (), wenhaoye@126.com
 *   Organization:  
 *
 * =============================================================================
 */
#include "event/inotify_base.h"
#include "network/net_util.h"
#include "config.h"
#include "file_opr.h"

#define NETWORK_PATH_MAX_LEN    64

typedef struct
{
    uint32_t mask;
    char path[NETWORK_PATH_MAX_LEN];
}NetworkMonitorArg;

static NetworkMonitorArg g_net_monitor_arg;

