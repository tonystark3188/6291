/*
 * =============================================================================
 *
 *       Filename:  hidisk_tcp_monitor.h
 *
 *    Description:  tcp monitor for Hidisk
 *
 *        Version:  1.0
 *        Created:  2016/1/20 13:05:49
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver
 *   Organization:  
 *
 * =============================================================================
 */
#ifndef _HIDISK_TCP_MONITOR_H_
#define _HIDISK_TCP_MONITOR_H_


#ifdef __cplusplus
extern "C"{
#endif
#include "base.h"
int _handle_client_json_req(ClientTheadInfo *client_info);

#ifdef __cplusplus
}
#endif


#endif
