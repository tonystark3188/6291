/**************************************************************
* nor将用户设置的参数保存到flash上，并可以读出，以恢复用户设置
* nor set xxx=xxx
* nor get xxx
* 得到的参数会打印出来
* Author: liuxiaolong
* time: 2012-12-13
**************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "nor.h"



extern char *g_nor_parameter[];

static void usage(void)
{
    int i = 0;
	printf("usage: user config tool\n");
	printf("config parameters are: ssid_name encryption ssid_password network_mode wan_mode\n");
	printf("                       stc_ip stc_mask stc_gw stc_dns1 stc_dns2\n");
	printf("                       smb_password smb_enable smb_anonymous_en\n");
	printf("                       ftp_password ftp_enable ftp_anonymous_en dms_enable dms_name\n");
	printf("	set 		: nor set parameter=value \n");
	printf("	get 		: nor get parameter \n");
	printf("	reset 		: clear user config data \n");		
	printf("	show 		: print all user config data \n");
	printf("support parameter:\n");//g_nor_parameter

	while(g_nor_parameter[i])
	{
        printf("%s  ",g_nor_parameter[i]);
		if((i)&&(i%4 == 0))
			printf("\n");
		i++;
	}
	printf("\n");
	exit(1);
}


int main(int argc,char *argv[])
{
	if(argc < 2)
	{
		usage();
		exit(0);
	}
	
	if(!strcmp(argv[1],"get"))
	{
		cfg_nor_get(argv[2]);
	}
	else if(!strcmp(argv[1],"set"))
	{
		cfg_nor_set(argv[2]);
	}
	else if(!strcmp(argv[1],"reset"))
	{
		reset_user_config();
	}
	else if(!strcmp(argv[1],"show"))
	{
		print_all_config();
	}

	else
	{
		usage();
	}
	
	return 0;
}


