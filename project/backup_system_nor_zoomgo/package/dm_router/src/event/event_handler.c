/*
 * =============================================================================
 *
 *       Filename:  event_handler.c
 *
 *    Description:  event handler module
 *
 *        Version:  1.0
 *        Created:  2014/8/27 17:05:30
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ye Wenhao (), wenhaoye@126.com
 *   Organization:  
 *
 * =============================================================================
 */

#include "my_event_base.h"
#include <sys/types.h>
#include <sys/socket.h>

typedef struct
{
	int eb_fd;
	//event_cb init_client_sock_obj;
}ClientSockInterface;


int tcp_listen_sock_event_handler(EventObj *event)
{
	/*if(event->fd < 0)
		return -1;

	int client_fd;
	struct sockaddr_in client_addr;
	socklen_t socklen = sizeof(client_addr);

	if((client_fd = accept(event->fd, (struct sockaddr *)&client_addr, &socklen) ) < 0)
	{
		log_error("accept failed!");
		return -2;	
	}

	EventObj new_event;
	memset(&new_event, 0, sizeof(new_event));
	new_event.fd = client_fd;
	
	ClientSockInterface *p_interface = (ClientSockInterface *)(event->arg);
	if(p_interface->init_client_sock_obj != NULL)
	{
		(*p_interface->init_client_sock_obj)(&new_event);
		my_event_base_add(p_interface->eb_fd, &new_event);
	}*/
	
	return 0;
}

#define CLIENT_BUF_SIZE 2048

typedef struct
{
	char read_buf[CLIENT_BUF_SIZE];
	char send_buf[CLIENT_BUF_SIZE];
	size_t read_buf_start;
	size_t read_buf_end;
	size_t read_buf_size;
	size_t send_buf_start;
	size_t send_buf_end;
	// size_t send_buf_size;
}ClientSockArg;

int tcp_client_sock_read_event_handler(EventObj *event)
{
	/*if(event->fd < 0)
		return -1;

	ssize_t read_bytes;
	ClientSockArg *p_arg = (ClientSockArg *)(event->arg);
	ssize_t len = p_arg->read_buf_end - p_arg->read_buf_start;
	if(len)
	{
		read_bytes = readn(event->fd, p_arg->read_buf+len, \
						  (p_arg->read_buf_size - len));
		if(read_bytes > 0)
		{
			p_arg->read_buf_len += read_bytes;
		}
		else if(read_bytes == 0)
		{
			close(event->fd);
			event->fd = -1;
			log_debug("close client socket!");
			return -1;
		}
	}*/

	// parse client requeset and handle it.

	return 0;
}


int tcp_client_sock_write_event_handler(EventObj *event)
{
	if(event->fd < 0)
		return -1;

	ssize_t write_bytes;
	ClientSockArg *p_arg = (ClientSockArg *)(event->arg);
	ssize_t len = p_arg->send_buf_end - p_arg->send_buf_start;
	if(len > 0)
	{
		write_bytes = writen(event->fd, p_arg->send_buf+p_arg->send_buf_start, len);
		if(write_bytes > 0)
		{
			p_arg->send_buf_start += write_bytes;
		}
	}


	return 0;
}


