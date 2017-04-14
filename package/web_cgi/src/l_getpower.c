#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include "socket_uart.h" 

#include "uci_for_cgi.h"
// /etc/config/system sid
#define SID "sid"
#define rtl_encryp_control "/proc/rtl_encryp_control"
#define get_power_level_num  1
#define get_Firmware_Edition 5



int get_conf_str(char *dest,char *var)
{
	FILE *fp=fopen("/tmp/state/status","r");
	if(NULL == fp)
	{
		//printf("open /etc/config/nrender.conf failed \n");
		return 0;
	}
	char tmp[128];
	char *ret_str;
	bzero(tmp,128);
	while(fgets(tmp,128,fp)!=NULL)
	{
		if('\n'==tmp[strlen(tmp)-1])
		{
			tmp[strlen(tmp)-1]=0;
		}
		//printf("get string from /etc/config/nrender.conf:%s\n",tmp);
		if(!strncmp(var,tmp,strlen(var)))
		{
			ret_str = malloc(strlen(tmp)-strlen(var));
			if(!ret_str)
			{
				fclose(fp);
				return 0;
			}
			bzero(ret_str,strlen(tmp)-strlen(var));
			strcpy(ret_str,tmp+strlen(var)+1);
			
			//printf("ret string:%s\n",ret_str);
			fclose(fp);
			strcpy(dest,ret_str);
			free(ret_str);
			return 0;
		}
		
	}
	fclose(fp);
	return 0;
}


int power(char **retstr)
{
	char bat[8]={0};
	system("letv_gpio 1 & >/dev/null");
	get_conf_str(bat,"power");

#if 0
	unsigned char percent=0;
	unsigned char stat=0;
	unsigned char tmpStatus=0;
	int bat;
	unsigned char chargflag = 0;
	int count = 0;
	int fh=NULL;
	int ret=0;
	socket_uart_cmd_t g_uart_cmd;
	bzero(&g_uart_cmd, sizeof(socket_uart_cmd_t));

	//p_debug("ret===sssssssss");
	g_uart_cmd.mode = UART_R;
	g_uart_cmd.regaddr =SOCKET_UART_GET_POWER_PERCENT;

	ret=SocketUartClientStart(&g_uart_cmd);
	//p_debug("ret===%d",ret);

	if(ret==0)
		bat = g_uart_cmd.data & 0xFF;
	else{
		//error_num++;
		sprintf(retstr,"{\"status\":0,\"errorCode\":3,\"errorMessage\":\"get power error\",\"data\":{}}");
		return 0;
	}
	//p_debug("bat=%d",bat);
	
	if(bat<=5)percent=0;
	else if(bat>5&&bat<=10)percent=10;
	else if(bat>10&&bat<=25)percent=25;
	else if(bat>25&&bat<=50)percent=50;
	else if(bat>50&&bat<=75)percent=75;
	else if(bat>75&&bat<=100)percent=100;
#endif	
	sprintf(retstr,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"power\":%s}}",bat);


	
}
void main()
{
		char ret_buf[2048];
		printf("Content-type:text/plain\r\n\r\n");
		power(&ret_buf);
		printf("%s",ret_buf);
		fflush(stdout);
		return;
}
