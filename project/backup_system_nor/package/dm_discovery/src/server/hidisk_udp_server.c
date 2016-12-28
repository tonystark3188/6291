/*
 * =============================================================================
 *
 *       Filename:  hidisk_udp_server.c
 *
 *    Description:  udp server for HiDisk
 *
 *        Version:  1.0
 *        Created:  2015/6/9 17:43
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver
 *   Organization:  
 *
 * =============================================================================
 */
#include "msg.h"
#include "hidisk_udp_server.h"
#include "my_debug.h"


/*
 * Desc: init udp server.
 *
 * port: the listen port of udp server.
 * Return: success on RET_SUCCESS, 
 */
int init_udp_server(int port)
{
	int listen_fd = -1;
	listen_fd = DM_UdpServerInit(AF_INET,port,SOCK_DGRAM,0);
	return listen_fd;
}

/*
 * Desc: stop tcp listen task.
 *
 */
void stop_udp_listen_task(int client_fd)
{
	g_udp_listen_thread_loop_flag = 0;
	DM_exit_udp_server(client_fd);
}

/*
 * Desc: recv tcp request from client.
 *
 * client_info: input/output, 
 * Return: success on RET_SUCCESS,
 */
int recv_req_from_udp_client(ClientTheadInfo *p_client_info)
{
	int ret = -1;
	ret = DM_UdpReceive(p_client_info->client_fd, &p_client_info->recv_buf,&p_client_info->time_out,&p_client_info->clientAddr);
	//DMCLOG_D("ret = %d,p_msg = %s",ret,p_client_info->recv_buf);
	
	if(ret == MSGRET_TIMED_OUT)
	{
		return 1;
	}
	else if(ret >= 0)
	{
		strcpy(p_client_info->ip, inet_ntoa(p_client_info->clientAddr.sin_addr));//反向获取客户端IP
		return 0;
	}
	else
	{
    	return -1;
	}
}

/*
 * Desc: send handle result to tcp client.
 *
 * client_info: input/output, 
 * Return: success on RET_SUCCESS, 
 */
int send_result_to_udp_client(ClientTheadInfo *p_client_info)
{
	int ret = -1;
	ret = DM_UdpSend(p_client_info->client_fd_send, p_client_info->retstr, strlen(p_client_info->retstr),&p_client_info->clientAddrSend);
	if (ret != 0)
	{
		DMCLOG_D("DM send failed with %d,acc_fd = %d",ret,p_client_info->client_fd_send);
		return -1;
	}
	return 0;
}
