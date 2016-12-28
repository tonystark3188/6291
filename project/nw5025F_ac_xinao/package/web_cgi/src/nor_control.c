#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "nor.h"
#include "uci_for_cgi.h"

char *g_nor_parameter[] = {
	[0] = "ssid_name",         [1] = "encryption",
	[2] = "ssid_password",     [3] = "wwanssid", 
	[4] = "wwanpwd",	       [5] = "wwanencry", 
	[6] = "wwanchan",          [7] = "airplay_name",      
	[8] = "dlna_name",          [9] = "network_mode",
	[10] = "wan_mode",         [11] = "stc_ip",     
	[12] = "stc_mask",         [13] = "stc_gw",    
	[14] = "stc_dns1",	       [15] = "stc_dns2",   
	[16] = "smb_password",     [17] = "smb_enable",     
	[18] = "smb_anonymous_en", [19] = "ftp_password",
	[20] = "ftp_enable",       [21] = "dms_enable",     
	[22] = "dms_name",         [23] = "wifi_mode", 
	[24] = "client_disabled",   		[25] = NULL,  
	 
};

static void debugwrite(char *str)
{
	int fw_fp;
	int f_size;
	
	if( (fw_fp=fopen("/tmp/feng.txt","ab+"))==NULL)    // write and read,binary
	{
		exit(1);
	}		
	
	f_size=fwrite(str,1,strlen(str),fw_fp);
	fwrite("\r\n", 1, 2, fw_fp);
	
	fclose(fw_fp);
}

void reset_user_config(void)
{
	int i=0;
	char option_str[DATA_LEN] = {0};

	while(g_nor_parameter[i])
	{
	    memset(option_str,0x0,sizeof(option_str));
	    snprintf(option_str,sizeof(option_str),"%s=unknow",g_nor_parameter[i]);
        cfg_nor_set(option_str);
		i++;
	}
}

void print_all_config(void)
{
	int i=0;

	while(g_nor_parameter[i])
	{
	    printf("%s=",g_nor_parameter[i]);
        cfg_nor_get(g_nor_parameter[i]);
		//printf("\n");
		i++;
	}
}

int cfg_nor_get(char *argv)
{
	unsigned char arg_str[DATA_LEN]={0};
	char uci_option_str[DATA_LEN]={0};
    int i = 0;

	if(argv==NULL)
		return 1;
    ctx=uci_alloc_context();
	if(NULL == ctx)
		return 1;
	uci_set_confdir(ctx,USERCONFIGPATH);

	while(g_nor_parameter[i])
	{
        if(!strcmp(argv,g_nor_parameter[i]))
        {
			snprintf(uci_option_str,sizeof(uci_option_str),"userconfig.@save[0].%s",argv);  //cur_mode
			uci_get_option_value(uci_option_str,arg_str);
			printf("%s\n",arg_str);
			break;
		}
		i++;
	}
	uci_free_context(ctx);
	return 0;
}

int cfg_nor_set(char *argv)
{
//	unsigned char arg_str[DATA_LEN]={0};
    char uci_option_str[DATA_LEN]={0};
    int i = 0;
	
	if(argv==NULL)
		return 1;

    ctx=uci_alloc_context();
	if(NULL == ctx)
		return 1;
	uci_set_confdir(ctx,USERCONFIGPATH);
	while(g_nor_parameter[i])
	{
        if(!strncmp(g_nor_parameter[i],argv,strlen(g_nor_parameter[i])))
        {
			snprintf(uci_option_str,sizeof(uci_option_str),"userconfig.@save[0].%s",argv);  //cur_mode
			uci_set_option_value(uci_option_str);
			break;
		}
		i++;
	}
	uci_free_context(ctx);
	system("uci commit userconfig -c /factory/");
	return 0;
}


int nor_set(char *str_name,unsigned char *value)
{
	unsigned char argv[256]={0};

	sprintf(argv,"%s=%s",str_name,value);
//	debugwrite(argv);
	return cfg_nor_set(argv);
}




