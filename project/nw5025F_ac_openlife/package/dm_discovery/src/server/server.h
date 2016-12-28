/*
 * =============================================================================
 *
 *       Filename:  server.h
 *
 *    Description:  hidisk sever module.
 *
 *        Version:  1.0
 *        Created:  2015/3/19 10:56
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 515296288jf@163.com
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _HIDISK_SERVER_H_
#define _HIDISK_SERVER_H_

#include "base.h"

#ifdef __cplusplus
extern "C"{
#endif
#define EXPIRE_TIME 5
#define EXPIRE_COUNT 30
int _handle_client_json_req(ClientTheadInfo *client_info);
int _udp_listen_clients(void);

#ifdef __cplusplus
}
#endif

#endif

