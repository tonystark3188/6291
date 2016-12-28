/**************************************************************
* cfg用来设置出厂配置，并对参数进行加密
* cfg set mac=845dd7001122
* cfg set ssid=airdisk
* cfg set encryption=none(wep,wpa,wpa2,mixed-wpa)
* cfg set password=00000000
* cfg set ip=192.168.222.254
* cfg set dhcp_start=xxx
* cfg set dhcp_end=xxx
* cfg set wpa_cipher=xxx(tkip,aes,tkip/aes)
* 同时，cfg还可以读取配置信息，
* 读取的格式为cfg get xxx
* 得到的参数会打印出来
* Author: liuxiaolong
* time: 2012-12-6
**************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "uci_for_cgi.h"

#define FACTORYCONFIGPATH "/factory/"
#define DATA_LEN		256
#define CFGFLAG "flag"
#define QQSN     "qqsn"
#define QQLICENCE "license"
#define VERSION_FLAG  "version_flag"
#define FW_VERSION "fw_version"



char *g_cfg_parameter[] = {
    [0] = "mac",                [1] = "ssid",
	[2] = "encryption",         [3] = "password", 
	[4] = "ip",	                [5] = "dhcp_start", 
	[6] = "dhcp_end",           [7] = "wpa_cipher",      
	[8] = "airplay_name",        [9] = "dlna_name",
	[10] = "fw_version",        [11] = "host_name",     
	[12] = "model_name",        [13] = "model_number",    
	[14] = "model_description",	[15] = "manufacturer",   
	[16] = "url_manufacturer",  [17] = "smb_usr_name",     
	[18] = "smb_usr_pwd",       [19] = "smb_guest_ok",
	[20] = "smb_enabled",       [21] = "dms_name",     
	[22] = "dms_enable",        [23] = "flag",
	[24] = "qqsn",              [25] = "license", 
	[26] = "version_flag",		[27] = "wifi_module",
	[28] = "radio_band",		[29] = NULL,
	
};	

static void usage(void)
{
    int i = 0;
	printf("usage: pisen config tool\n");
	printf("config parameters are: mac ssid encryption password ip dhcp_start dhcp_end wpa_cipher\n");
	printf("	set 		: cfg set parameter=value \n");
	printf("	get 		: cfg get parameter\n");
	printf("	list 	: show all cfg parameter and value\n");
	printf("	checkflag 	: check whether config parameters has been set\n");
	printf("	setflag 	: set the flag to 1\n");
	printf("	clearflag 	: set the flag to 0\n");

	while(g_cfg_parameter[i])
	{
        printf("%s  ",g_cfg_parameter[i]);
		if((i)&&(i%4 == 0))
			printf("\n");
		i++;
	}
	printf("\n");

	exit(1);
}

//检查标志位，如果标志位为pisen，说明配置参数已经被设置，否则
//则没有被设置
static int check_cfg_flag(void)
{
    cfg_get(CFGFLAG);
}
//将标志位设置为pisen，如果参数被写入，则必须将标志位设为pisen
static set_cfg_flag(void)
{
	cfg_set("flag=1");
}
//清除标志位
static clear_cfg_flag(void)
{
    cfg_set("flag=0");
}
//检查mac地址是否合法
static int check_mac(char *mac_addr)
{
	int i=0;
	if(strlen(mac_addr)!=12)
	{
		return 1;
	}
	for(i=0;i<12;i++)
	{
		if(!isxdigit(mac_addr[i]))
		{
			return 1;
		}
	}
	return 0;
}

//获取参数
int cfg_get(char *argv)
{
	unsigned char arg_str[DATA_LEN]={0};
	char uci_option_str[DATA_LEN]={0};
    int i = 0;

	if(argv==NULL)
		return 1;
    ctx=uci_alloc_context();
	if(NULL == ctx)
		return 1;
	uci_set_confdir(ctx,FACTORYCONFIGPATH);

	while(g_cfg_parameter[i])
	{
        if(!strcmp(argv,g_cfg_parameter[i]))
        {
			snprintf(uci_option_str,sizeof(uci_option_str),"factoryconfig.@save[0].%s",argv);  //cur_mode
			uci_get_option_value(uci_option_str,arg_str);
			if(strlen(arg_str))
			    printf("%s=%s\n",argv,arg_str);
			break;
		}
		i++;
	}
	uci_free_context(ctx);
	return 0;
}


//设置参数
int cfg_set(char *argv)
{
	unsigned char str_sp[DATA_LEN]={0};
    char uci_option_str[DATA_LEN]={0};
	char temp_mac[32]={0};
	int ret = 0;
    int i = 0,j = 0,k = 0;
	
	if(argv==NULL)
		return ret;

    ctx=uci_alloc_context();
	if(NULL == ctx)
		return ret;
	uci_set_confdir(ctx,FACTORYCONFIGPATH);
	while(g_cfg_parameter[i])
	{
        if(!strncmp(g_cfg_parameter[i],argv,strlen(g_cfg_parameter[i])))
        {  
            if(i==0)//mac
            {
				unsigned char mac_temp[16]={0};
				strncpy(mac_temp,argv+strlen(g_cfg_parameter[0])+1,sizeof(mac_temp));//ignore"mac="
				if(check_mac(mac_temp))
				{
					exit(1);
				}

				for(j = 0; j < 18; j+=3)
				{
                    temp_mac[j] = *(mac_temp+(k++));
					temp_mac[j+1] = *(mac_temp+(k++));
					if(j<15)
					    temp_mac[j+2] = ':';
				}
				sprintf(str_sp,"echo \'%s\' >/etc/mac.txt",temp_mac);	
				system(str_sp);
				system("sync");
			}
		
			snprintf(uci_option_str,sizeof(uci_option_str),"factoryconfig.@save[0].%s",argv);  //cur_mode
			uci_set_option_value(uci_option_str);
			system("uci set system.@mpset[0].status=0");
			system("uci commit system");
			
			if(!strncmp(argv,QQSN,strlen(QQSN)))
			{
			    system("mkdir -p /etc/qqconfig/");
				sprintf(str_sp,"echo \'%s\' >/etc/qqconfig/guid.txt",argv+strlen(QQSN)+1);	
				system(str_sp);
			}
			else if(!strncmp(argv,QQLICENCE,strlen(QQLICENCE)))
			{
			    system("mkdir -p /etc/qqconfig/");
				sprintf(str_sp,"echo \'%s\' >/etc/qqconfig/licence.txt",argv+strlen(QQLICENCE)+1);	
				system(str_sp);
			}
			else if(!strncmp(argv,FW_VERSION,strlen(FW_VERSION)))
			{ 
				char set_nrender_str[128]="\0";
				memset(set_nrender_str,0,sizeof(set_nrender_str));
				sprintf(set_nrender_str,"sed -e \'/^%s=.*/d\' -i /etc/nrender.conf",FW_VERSION);
				system(set_nrender_str);
				memset(set_nrender_str,0,sizeof(set_nrender_str));
				sprintf(set_nrender_str,"echo \'%s\' >> /etc/nrender.conf",argv);
				system(set_nrender_str);
			}
			

			if(!strncmp(argv,CFGFLAG,strlen(CFGFLAG)))
			{
			    ret = 0;
			}
			else
				ret = 1;
			break;
		}
		i++;
	}
	uci_free_context(ctx);
	system("uci commit factoryconfig -c /factory/");
	return ret;
}
void cfg_erase(void)
{
}

void cfg_list(void)
{
	int i=0;

	while(g_cfg_parameter[i])
	{
        cfg_get(g_cfg_parameter[i]);
		//printf("\n");
		i++;
	}
}

int main(int argc,char *argv[])
{
	if(argc < 2 || !strcmp(argv[1],"-h"))
	{
		usage();
		exit(0);
	}
	if(!strcmp(argv[1],"get"))
	{
		cfg_get(argv[2]);
	}
	else if(!strcmp(argv[1],"set"))
	{
		if(cfg_set(argv[2]))
	        set_cfg_flag();
	}
	else if(!strcmp(argv[1],"checkflag"))
	{
		check_cfg_flag();
	}
	else if(!strcmp(argv[1],"setflag"))
	{
		set_cfg_flag();
	}
	else if(!strcmp(argv[1],"clearflag"))
	{
		clear_cfg_flag();
	}
	else if(!strcmp(argv[1],"list"))
	{
		cfg_list();
	}
	/*else if(!strcmp(argv[1],"erase"))
	{
		cfg_erase();
	}*/
	else
	{
		usage();
	}
	return 0;
}
