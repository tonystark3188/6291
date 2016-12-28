/*
 * =============================================================================
 *
 *       Filename:  hidisk_UDP_server.h
 *
 *    Description:  udp server for Hidisk
 *
 *        Version:  1.0
 *        Created:  2015/6/18 13:05:49
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver
 *   Organization:  
 *
 * =============================================================================
 */
#ifndef _HIDISK_UDP_SERVER_H_
#define _HIDISK_UDP_SERVER_H_

#include "base.h"

#ifdef __cplusplus
extern "C"{
#endif
static uint8_t g_udp_listen_thread_loop_flag = 1;

/*
 * Desc: init udp server.
 *
 * port: the listen port of udp server.
 * Return: success on RET_SUCCESS, 
 */
int init_udp_server(int port);

/*
 * Desc: stop tcp listen task.
 *
 */
void stop_udp_listen_task(int client_fd);

/*
 * Desc: recv tcp request from client.
 *
 * client_info: input/output, 
 * Return: success on RET_SUCCESS,
 */
int recv_req_from_udp_client(ClientTheadInfo *p_client_info);

/*
 * Desc: send handle result to tcp client.
 *
 * client_info: input/output, 
 * Return: success on RET_SUCCESS, 
 */
int send_result_to_udp_client(ClientTheadInfo *p_client_info);


#ifdef __cplusplus
}
#endif


#endif

