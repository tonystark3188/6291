#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "uci_for_cgi.h"
#include "my_def.h"
#include "auto_connect.h"

#define WIFI_NUM_MAX	15

wifilist_t wifi_param;

int get_wifi_mode()
{
	char uci_option_str[64]="\0";
	char mode[16]="\0";
	ctx=uci_alloc_context();
	strcpy(uci_option_str,"wireless2.@wifi[0].mode"); 
	uci_get_option_value(uci_option_str,mode);
	memset(uci_option_str,'\0',64);
	d_printf("wifimode=%s\n", mode);

	uci_free_context(ctx);

	if(strcmp(mode,"2g")==0)
		return M_2G;
	else
		return M_5G;
}

int get_wifi_param()
{
	char uci_option_str[64]="\0";
	ctx=uci_alloc_context();
	strcpy(uci_option_str,"wireless.@wifi-iface[1].ssid"); 
	uci_get_option_value(uci_option_str,wifi_param.ssid);
	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"wireless.@wifi-iface[1].bssid"); 
	uci_get_option_value(uci_option_str,wifi_param.mac);
	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"wireless.@wifi-iface[1].encryption"); 
	uci_get_option_value(uci_option_str,wifi_param.encryption);
	memset(uci_option_str,'\0',64);
	if(strcmp(wifi_param.encryption,"none")!=0)
	{
		strcpy(uci_option_str,"wireless.@wifi-iface[1].key"); 
		uci_get_option_value(uci_option_str,wifi_param.key);
		memset(uci_option_str,'\0',64);
	}
	uci_free_context(ctx);
	d_printf("%s %s %s %s\n",wifi_param.ssid,wifi_param.mac,wifi_param.encryption,wifi_param.key);
	return 0;
}
#if 0
int check_connected_sta()
{
	FILE *fp_client=NULL;
	int status=0;
	int i;
	int ret=FALSE;
	for(i=0;i<15;i++)
	{
		system("/usr/mips/cgi-bin/script/ClientStatus.sh");
		if( (fp_client=fopen("/tmp/client_is_connected","r")) != NULL )
		{
			ret=TRUE;
			fclose(fp_client);
			break;
		}
		sleep(1);
	}
	return ret;
}
#endif

int check_connected_sta()
{
	char buffer[512]; 
	FILE *read_fp; 
	int chars_read; 
	int ret=FALSE; 
	memset( buffer, 0, 512 ); 
	read_fp = popen("wl -i wlan0 assoclist", "r");
	if(read_fp!=NULL)
	{
		chars_read = fread(buffer, sizeof(char), 512-1, read_fp); 
		if (chars_read > 0)   	//connect !
		{ 
			
			ret = TRUE; 
		} 
		else 					//disconnect !
		{ 
			ret = FALSE; 
		} 
		pclose(read_fp); 
	}
	
	return ret;
}

void formatMac(char *destMac, char *sourMac)
{
	int i = 0;
	char *p_dest = destMac,*p_sour = sourMac;
	for(i = 1; i<=17; i++){
		if(i%3 == 0){
			*p_dest = ':';
			p_dest++;
		}
		else{
			*p_dest = *p_sour;
			p_dest++;
			p_sour++;
		}
	}
	return ;
}


static wifilist_t *get_wifi_list(int wifimode,int *number)
{
	char uci_option_str[64]="\0";
	char wifinum[16]="\0";
	int i_wifinum=0;
	int i=0;
	int j;
	ctx=uci_alloc_context();

	if(wifimode==M_2G)
	{
		strcpy(uci_option_str,"wifilist.number2g.num"); 
		uci_get_option_value(uci_option_str,wifinum);
		memset(uci_option_str,'\0',64);
	}
	else
	{
		strcpy(uci_option_str,"wifilist.number5g.num"); 
		uci_get_option_value(uci_option_str,wifinum);
		memset(uci_option_str,'\0',64);
	}
	i_wifinum=atoi(wifinum);
	*number=i_wifinum;

	if(i_wifinum==0)
		return NULL;



	wifilist_t *wifi_list=(wifilist_t *)malloc(sizeof(wifilist_t)*i_wifinum);
	memset(wifi_list,0,sizeof(wifilist_t)*i_wifinum);
	for(i=0;i<i_wifinum;i++)
	{
		if(wifimode==M_2G)
		{
			sprintf(uci_option_str,"wifilist.@wifi2g[%d].ssid",i);
			uci_get_option_value(uci_option_str,(wifi_list+i)->ssid);
			memset(uci_option_str,'\0',64);
			sprintf(uci_option_str,"wifilist.@wifi2g[%d].encryption",i);
			uci_get_option_value(uci_option_str,(wifi_list+i)->encryption);
			memset(uci_option_str,'\0',64);
			if(strcmp((wifi_list+i)->encryption,"none")!=0)
			{
				sprintf(uci_option_str,"wifilist.@wifi2g[%d].key",i);
				uci_get_option_value(uci_option_str,(wifi_list+i)->key);
				memset(uci_option_str,'\0',64);
			}
		}
		else
		{
			sprintf(uci_option_str,"wifilist.@wifi5g[%d].ssid",i);
			uci_get_option_value(uci_option_str,(wifi_list+i)->ssid);
			memset(uci_option_str,'\0',64);
			sprintf(uci_option_str,"wifilist.@wifi5g[%d].encryption",i);
			uci_get_option_value(uci_option_str,(wifi_list+i)->encryption);
			memset(uci_option_str,'\0',64);
			if(strcmp((wifi_list+i)->encryption,"none")!=0)
			{
				sprintf(uci_option_str,"wifilist.@wifi5g[%d].key",i);
				uci_get_option_value(uci_option_str,(wifi_list+i)->key);
				memset(uci_option_str,'\0',64);
			}
		}
		//i++;
	}

	uci_free_context(ctx);
	for(i=0;i<i_wifinum;i++)
		d_printf("%s %s %s\n", (wifi_list+i)->ssid,(wifi_list+i)->encryption,(wifi_list+i)->key);
		
	return wifi_list;
}


int main()
{
	wifilist_t *wifilist;
	int i;
	char str_sp[64]="\0";

	sleep(15);
	int client_sta=check_connected_sta();
	if(client_sta==FALSE)
	{
		d_printf("client is not connected.\n");
		return 1;
	}
	d_printf("client is connected.\n");

	int i_wifimode,iwifimode;
	i_wifimode=get_wifi_mode();
	if(i_wifimode==M_2G)
		iwifimode=2;
	else
		iwifimode=5;

	get_wifi_param();

	
	int wifinumber=0;
	wifilist=get_wifi_list(i_wifimode,&wifinumber);

	
	int flag=0;
	for(i=0;i<wifinumber;i++)
	{
		if( !strcmp( (wifilist+i)->ssid, wifi_param.ssid) )
		{
			d_printf("match success.\n");
			flag=1;
			break;
		}
	}

	if(flag)
	{
		sprintf(str_sp,"uci set wifilist.@wifi%dg[%d].ssid='%s'",iwifimode,i,wifi_param.ssid);
		//uci_set_option_value(str_sp);
		system(str_sp);
		memset(str_sp,0,64);

		
		sprintf(str_sp,"uci set wifilist.@wifi%dg[%d].encryption=%s",iwifimode,i,wifi_param.encryption);
		system(str_sp);
		memset(str_sp,0,64);
		if(memcmp(wifi_param.encryption,"none",4)!=0)
		{
			sprintf(str_sp,"uci set wifilist.@wifi%dg[%d].key='%s'",iwifimode,i,wifi_param.key);
		}
		//uci_set_option_value(str_sp);
		system(str_sp);
		memset(str_sp,0,64);
		sprintf(str_sp,"uci set wifilist.@wifi%dg[%d].bssid=%s",iwifimode,i,wifi_param.mac);
		//uci_set_option_value(str_sp);
		system(str_sp);
		memset(str_sp,0,64);
		system("uci commit wifilist");

	
	}
	else
	{			
		sprintf(str_sp,"uci add wifilist wifi%dg",iwifimode);
		system(str_sp);
		memset(str_sp,0,64);
		
		sprintf(str_sp,"uci set wifilist.@wifi%dg[-1].ssid='%s'",iwifimode,wifi_param.ssid);
		system(str_sp);
		memset(str_sp,0,64);
		
		sprintf(str_sp,"uci set wifilist.@wifi%dg[-1].bssid='%s'",iwifimode,wifi_param.mac);
		system(str_sp);
		memset(str_sp,0,64);
		
		sprintf(str_sp,"uci set wifilist.@wifi%dg[-1].encryption=%s",iwifimode,wifi_param.encryption);
		system(str_sp);
		memset(str_sp,0,64);
		
		if(strcmp(wifi_param.encryption,"none")!=0)
		{
			sprintf(str_sp,"uci set wifilist.@wifi%dg[-1].key='%s'",iwifimode,wifi_param.key);
		}
		system(str_sp);
		memset(str_sp,0,64);
		
		if(wifinumber >= WIFI_NUM_MAX){
			sprintf(str_sp,"uci del wifilist.@wifi%dg[0]",iwifimode);
			system(str_sp);
			memset(str_sp,0,64);
			sprintf(str_sp,"uci set wifilist.number%dg.num=%d", iwifimode, wifinumber);
			system(str_sp);
			memset(str_sp,0,64);
		}
		else{
			sprintf(str_sp,"uci set wifilist.number%dg.num=%d", iwifimode, wifinumber+1);
			system(str_sp);
			memset(str_sp,0,64);
		}
		
		system("uci commit wifilist");
	}
	system("cp -f /etc/config/wifilist /factory/wifilist");
	free(wifilist);

	return 0;
}
