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
	printf("cmd 8   : set reset led\n");
	printf("cmd 9	: set RTC time for 2016-12-25-7(week)08:00:00\n");
	printf("cmd 10	: set ALARM time for 2016-12-25-7(week)08:05:00 on\n");
	printf("cmd 11	: set ALARM time off\n");
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
					printf("set reset led\n");
					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_RESET_LED;
					g_uart_cmd.data = (unsigned short)RESET_LED_BLINK;
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set storage dirtion to pc success\n");
				}else if(!strcmp(optarg,"9"))
				{	
					printf("set RTC time for 2016-12-25-7(week)08:00:00\n");
					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_RTC;
					g_uart_cmd.data = (unsigned short)(16<<8)|SET_YEAR;	//SET YEAR 2016 MEANS 16
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set rtc TIME YEAR success\n");

					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_RTC;
					g_uart_cmd.data = (unsigned short)(12<<8)|SET_MONTH;	//SET MONTH  12
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set rtc TIME MONTH success\n");

					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_RTC;
					g_uart_cmd.data = (unsigned short)(25<<8)|SET_DAY;	//SET DAY 25
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set rtc TIME DAY success\n");

					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_RTC;
					g_uart_cmd.data = (unsigned short)(7<<8)|SET_WEEK;	//SET WEEKDAY 7
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set rtc TIME WEEKDAY success\n");

					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_RTC;
					g_uart_cmd.data = (unsigned short)(8<<8)|SET_HOUR;	//SET hour 8
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set rtc TIME hour success\n");

					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_RTC;
					g_uart_cmd.data = (unsigned short)(SET_AM<<8)|SET_AMPM;	//SET AMPM always 24H
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set rtc TIME 24H success\n");

					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_RTC;
					g_uart_cmd.data = (unsigned short)(0<<8)|SET_MINUTE;	//SET minute 0
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set rtc TIME minute success\n");

					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_RTC;
					g_uart_cmd.data = (unsigned short)(0<<8)|SET_SECORD;	//SET second 0
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set rtc TIME second success\n");

					
					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_RTC;
					g_uart_cmd.data = (unsigned short)SET_COMPLETE;	//SET  RTC complete
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set rtc TIME success\n");
					
				}else if(!strcmp(optarg,"10"))
				{	
					printf("set ALARM time for 2016-12-25-7(week)08:05:00 \n");

					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_ALARM;
					g_uart_cmd.data = (unsigned short)(7<<8)|SET_WEEK;	//SET WEEKDAY 7
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set ALARM TIME WEEKDAY success\n");

					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_ALARM;
					g_uart_cmd.data = (unsigned short)(8<<8)|SET_HOUR;	//SET hour 8
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set ALARM TIME hour success\n");

					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_ALARM;
					g_uart_cmd.data = (unsigned short)(SET_AM<<8)|SET_AMPM;	//SET AMPM always 24H
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set ALARM TIME 24H success\n");

					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_ALARM;
					g_uart_cmd.data = (unsigned short)(5<<8)|SET_MINUTE;	//SET minute 5
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set ALARM TIME minute success\n");

					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_ALARM;
					g_uart_cmd.data = (unsigned short)(0<<8)|SET_SECORD;	//SET second 0
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set ALARM TIME second success\n");

					
					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_ALARM;
					g_uart_cmd.data = (unsigned short)SET_COMPLETE;	//SET  RTC complete
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set ALARM TIME success\n");

				}else if(!strcmp(optarg,"11"))
				{	
					printf("set ALARM time off \n");
					g_uart_cmd.mode = UART_W;
					g_uart_cmd.regaddr = SOCKET_UART_SET_ALARM;
					g_uart_cmd.data = (unsigned short)ALARM_CANCEL;	//SET  RTC complete
					ret = SocketUartClientStart(&g_uart_cmd);
					if(ret == 0)
						printf("set ALARM TIME off success\n");;
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




