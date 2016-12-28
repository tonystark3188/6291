#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <stdarg.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/wait.h>
//#include <netinet/in.h>
//#include <netinet/tcp.h>
//#include <arpa/inet.h>
#include <time.h>

#include "msg.h"
#include "wifi_uart.h"
#include "socket_uart.h"

int SocketUartServerHandle(int uart_fd, socket_uart_cmd_t *p_socket_uart_cmd)
{
	int ret = -1;

	if(p_socket_uart_cmd->regaddr == SOCKET_UART_STM8_UPLOAD)
	{
		ret = stm8_upload(uart_fd);	
		if(ret < 0)
		{
			return -1;
		}
	}
	
	else if((p_socket_uart_cmd->mode == UART_R))
	{
		if((p_socket_uart_cmd->regaddr >= SOCKET_UART_GET_START) && \
			(p_socket_uart_cmd->regaddr <= SOCKET_UART_GET_END))
		{
			ret = stm8_read(uart_fd, p_socket_uart_cmd->regaddr, &p_socket_uart_cmd->data);
			if(ret <= 0)
			{
				return -1;
			}
			debug("regaddr:%d,data:%d\n", p_socket_uart_cmd->regaddr, p_socket_uart_cmd->data);
		}
		else
		{	
			debug("unsupport regaddr:0x%02x\n",p_socket_uart_cmd->regaddr);
			return -1;
		}
	}

	else if((p_socket_uart_cmd->mode == UART_W))
	{
		if((p_socket_uart_cmd->regaddr >= SOCKET_UART_SET_START) && \
		(p_socket_uart_cmd->regaddr < SOCKET_UART_SET_END))
		{
			ret = stm8_write(uart_fd, p_socket_uart_cmd->regaddr, p_socket_uart_cmd->data);
			if(ret <= 0)
			{
				return -1;
			}
		}
		else
		{	
			debug("unsupport regaddr:0x%02x\n",p_socket_uart_cmd->regaddr);
			return -1;
		}
	}
	return 0;
}

int SocketUartCompRequest(char **buf, socket_uart_cmd_t *p_socket_uart_cmd)
{
	//char cmd_buf[256];
	char *cmd_buf = (char *)calloc(1, 256); 
	if(cmd_buf == NULL)
	{
		return -1;
	}
	sprintf(cmd_buf, "{\"regaddr\": \"%d\",\"data\": \"%d\",\"mode\": \"%d\",\"error\": \"0\"}",\
			p_socket_uart_cmd->regaddr, p_socket_uart_cmd->data, p_socket_uart_cmd->mode);
	*buf = cmd_buf;
	return 0;
}

int SocketUartCompResponse(char **buf, socket_uart_cmd_t *p_socket_uart_cmd, int error_code)
{
	char *cmd_buf = (char *)calloc(1, 256); 
	if(cmd_buf == NULL)
	{
		return -1;
	}
	sprintf(cmd_buf, "{\"regaddr\": \"%d\",\"data\": \"%d\",\"mode\": \"%d\",\"error\": \"%d\"}",\
			p_socket_uart_cmd->regaddr, p_socket_uart_cmd->data, p_socket_uart_cmd->mode, error_code);
	*buf = cmd_buf;
	return 0;
}

unsigned int JsonGetProp(char *buf, char *tag)
{
	char cmd_buf[256];
	char nstr[32];
	char *ptmp = NULL, *pend = NULL, *pstart = NULL;
	if((NULL == buf) || (NULL == tag))
	{
		return -1;
	}
	bzero(cmd_buf, 256);
	memcpy(cmd_buf, buf, 256);

	ptmp = strstr(cmd_buf, tag);
	if(NULL == ptmp)
	{
		return -1;
	}

	ptmp = strchr(ptmp, ':');	
	if(NULL == ptmp)
	{
		return -1;
	}

	ptmp = strchr(ptmp, '\"');	
	if(NULL == ptmp)
	{
		return -1;
	}

	pstart = ptmp+1;
	if(NULL == pstart)
	{
		return -1;
	}

	pend = strchr(pstart, '\"');
	if(NULL == pend)
	{
		return -1;
	}
	
	bzero(nstr, 32);
	memcpy(nstr, pstart, pend-pstart);
	return atoi(nstr);
}

int SocketUartParseJson(char *buf, socket_uart_cmd_t *p_socket_uart_cmd)
{
	unsigned int ret = 0;

	ret = JsonGetProp(buf, "regaddr");
	if(ret < 0)
	{
		return -1;
	}
	p_socket_uart_cmd->regaddr = (unsigned char)ret;

	ret = JsonGetProp(buf, "data");
	if(ret < 0)
	{
		return -1;
	}
	p_socket_uart_cmd->data = (unsigned short)ret;

	ret = JsonGetProp(buf, "mode");
	if(ret < 0)
	{
		return -1;
	}
	p_socket_uart_cmd->mode = (unsigned char)ret;

	ret = JsonGetProp(buf, "error");
	return ret;
}

int SocketUartServerStart()
{
	int listen_fd = -1;
	int accept_fd = -1;
	int ret = -1;
	int enRet = 0;
	int error_code = 0;
	socket_uart_cmd_t uart_cmd;
	char *rev_buf = NULL;
	char *send_buf = NULL;
	int send_len = 0;
	int uart_fd = 0;

	uart_fd = init_uart(BAUD_RATE);
	if(uart_fd < 0)
	{
		debug("uart init error\n");
		enRet = -1;
		goto EXIT1;
	}
	
	listen_fd = DM_UART_InetServerInit(AF_INET, SOCKET_UART_PORT, SOCK_STREAM, SOCKET_UART_WAIT_COUNT);
	if(listen_fd < 0)
	{
		debug("server init error\n");
		enRet = -1;
		goto EXIT1;
	}

	while(1)
	{
		bzero(&uart_cmd, sizeof(uart_cmd));
		error_code = 0;
		accept_fd = DM_UART_ServerAcceptClient(listen_fd);
		if(accept_fd < 0)
		{
			debug("accept client error\n");
			continue;
		}

		ret = DM_UART_MsgReceive(accept_fd, &rev_buf, SOCKET_UART_TIMEOUT);
		if(ret < 0)
		{
			DM_UART_ServerAcceptClientClose(accept_fd);
			if(ret = MSGRET_TIMED_OUT)
				debug("receive client timeout\n");
			else
				debug("receive client error\n");
			continue;
		}

		debug("rev_buf = %s\n", rev_buf);
		ret = SocketUartParseJson(rev_buf, &uart_cmd);
		if(ret < 0)
		{
			debug("parse json error\n");
			//DM_ServerAcceptClientClose(accept_fd);
			//if(rev_buf != NULL)
				//free(rev_buf);
			error_code = ERROR_SERVER_PARSE;
		}else
		{
			ret = SocketUartServerHandle(uart_fd, &uart_cmd);
			if(ret < 0)
			{
				debug("uart handle error\n");
				//DM_ServerAcceptClientClose(accept_fd);
				//if(rev_buf != NULL)
					//free(rev_buf);
				error_code = ERROR_SERVER_PARSE;
			}
		}
			
		ret = SocketUartCompResponse(&send_buf, &uart_cmd, error_code);
		if(ret < 0)
		{
			debug("compose response error\n");
			DM_UART_ServerAcceptClientClose(accept_fd);
			if(rev_buf != NULL)
				free(rev_buf);
			continue;
		}

		debug("send_buf = %s\n", send_buf);
		send_len = strlen(send_buf);
		ret = DM_UART_MsgSend(accept_fd, send_buf, send_len);
		if(ret < 0)
		{
			debug("send buf error\n");
			DM_UART_ServerAcceptClientClose(accept_fd);
			if(rev_buf != NULL)
				free(rev_buf);
			if(send_buf != NULL)
				free(send_buf);
			continue;
		}
		
		DM_UART_ServerAcceptClientClose(accept_fd);
		if(rev_buf != NULL)
			free(rev_buf);
		if(send_buf != NULL)
			free(send_buf);
	}

EXIT1:
	DM_UART_InetServerDeinit(listen_fd);
	close_uart(uart_fd);
	return enRet;
}


int SocketUartClientStart(socket_uart_cmd_t *p_socket_uart_cmd)
{
	int ret = 0;
	int ret_code = 0;
	int client_fd = -1;
	int error_code = 0;
	char *rev_buf = NULL;
	char *send_buf = NULL;
	char local_ip[32] = "\0";
	int send_len = 0;
	socket_uart_cmd_t rev_uart_cmd;
	int time_out = 0;

	bzero(&local_ip, 32);
	strcpy(local_ip, "127.0.0.1");
	if((p_socket_uart_cmd != NULL) && (p_socket_uart_cmd->regaddr != SOCKET_UART_STM8_UPLOAD))
	{
		time_out = SOCKET_UART_TIMEOUT;
	}

	ret = SocketUartCompRequest(&send_buf, p_socket_uart_cmd);
	if(ret < 0)
	{
		ret_code = ERROR_CLIENT_DATA;
		goto EXIT1;
	}

	//debug("send_buf = %s\n", send_buf);
	client_fd = DM_UART_InetClientInit(AF_INET, SOCKET_UART_PORT, SOCK_STREAM, local_ip);
	if(client_fd < 0)
	{
		ret_code = ERROR_CLIENT_INIT;
		goto EXIT1;
	}

	send_len = strlen(send_buf);	
	ret = DM_UART_MsgSend(client_fd, send_buf, send_len);	
	if(ret < 0)
	{
		ret_code = ERROR_CLIENT_SEND;
		goto EXIT1;
	}

	ret = DM_UART_MsgReceive(client_fd, &rev_buf, time_out);
	if(ret < 0)
	{
		if(ret = MSGRET_TIMED_OUT)
		{
			debug("receive server timeout\n");
			ret_code = ERROR_CLIENT_TIMEOUT;
		}
		else
		{
			debug("receive server error\n");
			ret_code = ERROR_CLIENT_RECV;
		}
		goto EXIT1;
	}

	//debug("rev_buf = %s\n", rev_buf);
	ret = SocketUartParseJson(rev_buf, p_socket_uart_cmd);
	if(ret < 0)
	{
		if(ret == -1)
		{
			debug("receive server data error\n");
			ret_code = ERROR_SERVER_DATA;
		}
		else
		{
			//debug("server error\n");
			ret_code = ret;
		}
		goto EXIT1;
	}

EXIT1:
	DM_UART_InetClientDeinit(client_fd);
	if(rev_buf != NULL)
		free(rev_buf);
	if(send_buf != NULL)
		free(send_buf);
	return ret_code;
}




