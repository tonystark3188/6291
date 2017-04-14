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


static int _network_monitor_handler(void *arg, struct inotify_event *event)
{
    NetworkMonitorArg *net_arg = (NetworkMonitorArg *)(arg);

    if(event == NULL)
    {
        // exit, we do not need to anything for now.
        log_notice("exit watch");
        return RET_SUCCESS;
    }

    if(!(event->mask & IN_MODIFY))
    {
        log_warning("event mask(0x%x) != IN_MODIFY(0x%x)", event->mask, IN_MODIFY);
        return 1;
    }

    char buf[2] = {0};
    int fd = open(net_arg->path, O_RDONLY);
    if(fd < 0)
    {
        log_error("open(%s) failed", net_arg->path);
        return EOPEN;
    }

    if(read(fd, buf, sizeof(buf)) <= 0)
    {
        log_error("read(%s) failed", net_arg->path);
        safe_close(fd);
        return EREAD;
    }
    safe_close(fd);

    if(buf[0] == '1')
    {
        // ELINK UP
        log_notice("ETH LINK UP");
        //get_inf_by_name(get_eth_wan_inf());
    }
    else if(buf[0] == '0')
    {
        // ELINK DOWN
        log_notice("ETH LINK DOWN");
        //set_inf_status(get_eth_wan_inf(), INF_DOWN);
    }
    else
    {
        log_error("ETH UNKNOW STATUS");
    }

    return RET_SUCCESS;
}

int init_network_monitor(const char *path)
{
    if(strlen(path) >= NETWORK_PATH_MAX_LEN)
    {
        log_error("invalid arg");
        ASSERT(0);
        return EINVAL_ARG;
    }

    if(is_file_exist_ext(path) <= 0)
    {
        log_warning("file(%s) not exist, but we do not care it!", path);
        ASSERT(0);
        return EINVAL_ARG;
    }
    
    memset(&g_net_monitor_arg, 0, sizeof(NetworkMonitorArg));
    g_net_monitor_arg.mask = IN_MODIFY;
    S_STRNCPY(g_net_monitor_arg.path, path, NETWORK_PATH_MAX_LEN);
    
    return add_file_watch(path, g_net_monitor_arg.mask, 
                        _network_monitor_handler, (void *)&g_net_monitor_arg);
}


