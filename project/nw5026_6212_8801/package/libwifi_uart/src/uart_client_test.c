#include <stdio.h>
#include <string.h>
#include "socket_uart.h"
extern char *optarg;
extern int optind, opterr, optopt;

void usage()
{
	printf("usage   : uart_client_test <option> <cmd>\n");
	printf("<option -g:get cmd>\n");
	printf("cmd 1   : get firmware\n");
	printf("cmd 2   : get power percent\n");
	printf("cmd 3   : get button\n");
	printf("cmd 4   : get charg\n");
	printf("cmd 5   : get storage power\n");
	printf("cmd 6   : get pc detect\n");
	printf("cmd 7   : get storage direction\n");
	printf("cmd 8   : get power status\n");
	printf("cmd 9   : get host power status\n");
	printf("<option -s:set cmd>\n");
	printf("cmd 1   : upload stm8\n");
	printf("cmd 2   : set reset\n");
	printf("cmd 3   : set poweroff\n");
	printf("cmd 4   : set storage power off\n");
	printf("cmd 5   : set storage power on\n");
	printf("cmd 6   : set storage direction to pc\n");
	printf("cmd 7   : set storage direction to board\n");
	printf("cmd 8   : turn off power led just for chunhong project\n");
	printf("<option -c:input code>\n");
}

int main(int argc, char **argv) 
{ 
	int ret = 0;
	int ch;
	socket_uart_cmd_t g_uart_cmd;
	if(argc <= 1)
	{
		usage();
		return -1;
	}
	
	bzero(&g_uart_cmd, sizeof(socket_uart_cmd_t));
	while((ch = getopt(argc,argv,"g:s:c"))!= -1)
	{
		switch(ch)
		{
			case 'g':
				if(!strcmp(optarg,"1"))  
				{
					printf("get firmware version\n");
					g_uart_cmd.mode = UART_R;
					g_uart_cmd.regaddr = SOCKET_UART_GET_FIMRWARE_VERSION;
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("data = %d\n", g_uart_cmd.data & 0xFF);	
				}
				else if(!strcmp(optarg,"2")) 
				{
					printf("get power percent\n");
					g_uart_cmd.mode = UART_R;
					g_uart_cmd.regaddr = SOCKET_UART_GET_POWER_PERCENT;
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("data = %d\n", g_uart_cmd.data & 0xFF);	
				}else if(!strcmp(optarg,"3"))  
				{
					printf("get button status\n");
					g_uart_cmd.mode = UART_R;
					g_uart_cmd.regaddr = SOCKET_UART_GET_BOTTON_STATUS;
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("data = %d\n", g_uart_cmd.data);	
				}else if(!strcmp(optarg,"4"))   
				{
					printf("get charge status\n");
					g_uart_cmd.mode = UART_R;
					g_uart_cmd.regaddr = SOCKET_UART_GET_CHARGE_STATUS;
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("data = %d\n", g_uart_cmd.data & 0xFF);	
				}else if(!strcmp(optarg,"5"))  
				{
					printf("get storage power\n");
					g_uart_cmd.mode = UART_R;
					g_uart_cmd.regaddr = SOCKET_UART_GET_STORAGE_POWER;
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("data = %d\n", g_uart_cmd.data);	
				}else if(!strcmp(optarg,"6"))  
				{
					printf("get pc detect\n");
					g_uart_cmd.mode = UART_R;
					g_uart_cmd.regaddr = SOCKET_UART_GET_PC_DETEC;
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("data = %d\n", g_uart_cmd.data);	
				}else if(!strcmp(optarg,"7"))   
				{
					printf("get storage dir\n");
					g_uart_cmd.mode = UART_R;
					g_uart_cmd.regaddr = SOCKET_UART_GET_STORAGE_DIRECTION;
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("data = %d\n", g_uart_cmd.data);	
				}else if(!strcmp(optarg,"8"))
				{
					printf("get power status\n");
					g_uart_cmd.mode=UART_R;
					g_uart_cmd.regaddr = SOCKET_UART_GET_POWER_STATUS;
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("data = %d\n", g_uart_cmd.data);
				}else if(!strcmp(optarg,"9"))
				{
					printf("get host power status\n");
					g_uart_cmd.mode=UART_R;
					g_uart_cmd.regaddr = SOCKET_UART_GET_IF_POWEROFF;
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("data = %d\n", g_uart_cmd.data);
				}
				
				else
				{
					printf("invalid argument\n");
					usage();
				}
				break;
				
			case 's':
				if(!strcmp(optarg,"1")) 
				{
					printf("stm8 upload\n");
					g_uart_cmd.regaddr = SOCKET_UART_STM8_UPLOAD;
					ret = SocketUartClientStart(&g_uart_cmd);
				}else if(!strcmp(optarg,"2"))  
				{
					printf("set system reset \n");
					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_RESET;
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set system reset success");
				}else if(!strcmp(optarg,"3")) 
				{
					printf("set system pweroff");
					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_POWEROFF;
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set system poweroff success");
				}else if(!strcmp(optarg,"4")) 
				{	
					printf("set storage power off\n");
					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_STORAGE_POWER;
					g_uart_cmd.data = (unsigned short)STORAGE_POWER_OFF;
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set storage power off success\n");
				}else if(!strcmp(optarg,"5")) 
				{	
					printf("set storage power on\n");
					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_STORAGE_POWER;
					g_uart_cmd.data = (unsigned short)STORAGE_POWER_ON;
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set storage power on success\n");
				}else if(!strcmp(optarg,"6"))
				{	
					printf("set storage dirtion to pc\n");
					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_STORAGE_DIRECTION;
					g_uart_cmd.data = (unsigned short)STORAGE_PC;
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set storage dirtion to pc success\n");
				}else if(!strcmp(optarg,"7"))
				{	
					printf("set storage dirtion to board\n");
					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_STORAGE_DIRECTION;
					g_uart_cmd.data = (unsigned short)STORAGE_BOARD;
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set storage dirtion to pc success\n");
				}else if(!strcmp(optarg,"8"))
				{	
					printf("turn off power led for chunhong project\n");
					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_OFF_LED_FOR_CH;
					g_uart_cmd.data = 0x01;
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("turn off power led success\n");
				}else
				{
					printf("invalid argument\n");
					usage();
				}				
				break;
				
			case 'c':
				if(argc == 3)
				{
					
				}
				break;
				
			default:
				printf("invalid argument\n");
				usage();
				break;
		}
	}
	
	if(ret < 0)
		printf("error_code = %d\n", ret);
	
	return ret;	 
}




