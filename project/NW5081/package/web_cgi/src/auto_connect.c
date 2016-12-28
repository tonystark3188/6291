#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "uci_for_cgi.h"
#include "cgiWireless.h"
#include "my_def.h"
#include "auto_connect.h"



extern int cgi_scan2(char *ifname, char *outstr);




int get_wifi_mode()
{
	char uci_option_str[64]="\0";
	char mode[16]="\0";
	// ctx=uci_alloc_context();
	strcpy(uci_option_str,"wireless2.@wifi[0].mode"); 
	uci_get_option_value(uci_option_str,mode);
	memset(uci_option_str,'\0',64);
	d_printf("wifimode=%s\n", mode);

	// uci_free_context(ctx);

	if(strcmp(mode,"2g")==0)
		return M_2G;
	else
		return M_5G;
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


scan_list_t *read_scanlist_to_buff(char *scan_list_str,int *scan_num,int wifimode)
{
	int list_len;
	char *scan_list_tmp=scan_list_str;
	int apnum=0;
	
	if( (list_len=strlen(scan_list_str))==0)
		return -1;
	// scan_list_t *pscan_list=NULL;
	while( *scan_list_tmp !='\0')
	{
		if( *scan_list_tmp == '\n')
			apnum++;
		scan_list_tmp++;
	}

	if(apnum==0)
		return NULL;

	

	scan_list_t *scan_list=(scan_list_t *)malloc(sizeof(scan_list_t)*apnum);
	memset(scan_list,0,sizeof(scan_list_t)*apnum);

	scan_list_tmp=scan_list_str;
	char *p1=scan_list_str;
	char *p2=scan_list_str;
	int i=0,j=0;
	char singlelist[128]="\0";
	char *psinglelist;
	char *p_stok_ssid;
	char *p_stok_mac;
	char *p_stok_channel;
	char *p_stok_rssi;

	while(*scan_list_tmp != '\0')
	{
		if( *scan_list_tmp == '\n')
		{
			p2=scan_list_tmp;
			memcpy(singlelist,p1,p2-p1);
			psinglelist=singlelist;
			p_stok_ssid = strtok(psinglelist, ",");
			p_stok_mac = strtok(NULL, ",");
			//strcpy( (scan_list+i)->ssid,p_stok);
			p_stok_channel = strtok(NULL, ","); 
			//(scan_list+i)->channel=atoi(p_stok);
			p_stok_rssi = strtok(NULL, ",");
			//(scan_list+i)->rssi=atoi(p_stok);
			if(wifimode==M_2G)
			{
				if( atoi(p_stok_channel) <36 )  //2.4G
				{
					strcpy( (scan_list+i)->ssid,p_stok_ssid);
					formatMac((scan_list+i)->mac, p_stok_mac);
					(scan_list+i)->channel=atoi(p_stok_channel);
					(scan_list+i)->rssi=atoi(p_stok_rssi);
					i++;
				}
				else
				{
					apnum--;
				}
			}
			else
			{
				if( atoi(p_stok_channel) >14 )  //5G
				{	
					strcpy( (scan_list+i)->ssid,p_stok_ssid);
					(scan_list+i)->channel=atoi(p_stok_channel);
					(scan_list+i)->rssi=atoi(p_stok_rssi);
					i++;
				}
				else
				{
					apnum--;
				}
			}

			p1=p2+1;
			// i++;
			memset(singlelist,0,sizeof(singlelist));
		}
		scan_list_tmp++;
	}

	for(i=0;i<apnum;i++)
		d_printf("%s %s %d %d\n", (scan_list+i)->ssid, (scan_list+i)->mac,(scan_list+i)->channel,(scan_list+i)->rssi);

	*scan_num=apnum;
	
	return scan_list;

}


int scan_list_sequence(scan_list_t *scan_list,int num)
{
	scan_list_t swap_list;
	// memset(&swap_list,0,sizeof(scan_list_t));
	int i,j;
	int m=num;
	
	for(j=0;j<num-1;j++)
	{
		m--;
		for(i=0;i<m;i++)
		{
			if( (scan_list+i)->rssi < (scan_list+i+1)->rssi)
			{
				memset(&swap_list,0,sizeof(scan_list_t));
				memcpy(swap_list.ssid,(scan_list+i)->ssid,sizeof(swap_list.ssid));
				memcpy(swap_list.mac,(scan_list+i)->mac,sizeof(swap_list.mac));
				swap_list.channel=(scan_list+i)->channel;
				swap_list.rssi=(scan_list+i)->rssi;

				memset((scan_list+i)->ssid,0,sizeof(swap_list.ssid));
				memcpy((scan_list+i)->ssid,(scan_list+i+1)->ssid,sizeof(swap_list.ssid));
				memcpy((scan_list+i)->mac,(scan_list+i+1)->mac,sizeof(swap_list.mac));
				(scan_list+i)->channel=(scan_list+i+1)->channel;
				(scan_list+i)->rssi=(scan_list+i+1)->rssi;

				memset((scan_list+i+1)->ssid,0,sizeof(swap_list.ssid));
				memcpy((scan_list+i+1)->ssid,swap_list.ssid,sizeof(swap_list.ssid));
				memcpy((scan_list+i+1)->mac,swap_list.mac,sizeof(swap_list.mac));
				(scan_list+i+1)->channel=swap_list.channel;
				(scan_list+i+1)->rssi=swap_list.rssi;

			}
		}
	}
	
	for(i=0;i<num;i++)
		d_printf("%s %d %d\n", (scan_list+i)->ssid,(scan_list+i)->channel,(scan_list+i)->rssi);
	// free(swap_list);
	return 0 ;
}

wifilist_t *get_wifi_list(int wifimode,int *number)
{
	char uci_option_str[64]="\0";
	char wifinum[16]="\0";
	int i_wifinum=0;
	int i=0;
	int j;
	// ctx=uci_alloc_context();

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
	for(j=i_wifinum-1,i=0;j>=0;j--,i++)
	{
		if(wifimode==M_2G)
		{
			sprintf(uci_option_str,"wifilist.@wifi2g[%d].ssid",j);
			uci_get_option_value(uci_option_str,(wifi_list+i)->ssid);
			memset(uci_option_str,'\0',64);
			sprintf(uci_option_str,"wifilist.@wifi2g[%d].bssid",j);
			uci_get_option_value(uci_option_str,(wifi_list+i)->mac);
			memset(uci_option_str,'\0',64);
			sprintf(uci_option_str,"wifilist.@wifi2g[%d].encryption",j);
			uci_get_option_value(uci_option_str,(wifi_list+i)->encryption);
			memset(uci_option_str,'\0',64);
			if(strcmp((wifi_list+i)->encryption,"none")!=0)
			{
				sprintf(uci_option_str,"wifilist.@wifi2g[%d].key",j);
				uci_get_option_value(uci_option_str,(wifi_list+i)->key);
				memset(uci_option_str,'\0',64);
			}
		}
		else
		{
			sprintf(uci_option_str,"wifilist.@wifi5g[%d].ssid",j);
			uci_get_option_value(uci_option_str,(wifi_list+i)->ssid);
			memset(uci_option_str,'\0',64);
			sprintf(uci_option_str,"wifilist.@wifi5g[%d].bssid",j);
			uci_get_option_value(uci_option_str,(wifi_list+i)->mac);
			memset(uci_option_str,'\0',64);
			sprintf(uci_option_str,"wifilist.@wifi5g[%d].encryption",j);
			uci_get_option_value(uci_option_str,(wifi_list+i)->encryption);
			memset(uci_option_str,'\0',64);
			if(strcmp((wifi_list+i)->encryption,"none")!=0)
			{
				sprintf(uci_option_str,"wifilist.@wifi5g[%d].key",j);
				uci_get_option_value(uci_option_str,(wifi_list+i)->key);
				memset(uci_option_str,'\0',64);
			}
		}
		//i++;
	}

	// uci_free_context(ctx);
	for(i=0;i<i_wifinum;i++)
		d_printf("%s %s %s\n", (wifi_list+i)->ssid,(wifi_list+i)->encryption,(wifi_list+i)->key);
	return wifi_list;
}



int main()
{
	char scan_list[8192]="\0";
	int wifimode;
	int scan_ap_num=0;
	int wifi_num=0;
	int i,j;
	char uci_set_str[128]="\0";
	int flag=0;

	scan_list_t *s_scan_list;
	wifilist_t *s_wifi_list;


	memset(scan_list,0,sizeof(scan_list));
	memset(uci_set_str,0,sizeof(uci_set_str));

	ctx=uci_alloc_context();
	
	wifimode=get_wifi_mode();
	
	cgi_scan2("wlan0", scan_list);
	if(strlen(scan_list)==0)
		return -1;
	d_printf("%s\n", scan_list);
	s_scan_list=read_scanlist_to_buff(scan_list,&scan_ap_num,wifimode);
	if(s_scan_list==NULL)
		goto error_exit;
	d_printf("\n");

	scan_list_sequence(s_scan_list,scan_ap_num);
	d_printf("\n");

	s_wifi_list=get_wifi_list(wifimode,&wifi_num);
	if(s_wifi_list==NULL)
	{
		free(s_scan_list);
		goto error_exit;
	}

	uci_free_context(ctx);

	for(i=0;i<wifi_num;i++)
	{
		for(j=0;j<scan_ap_num;j++)
		{
			if( !strcmp( (s_wifi_list+i)->ssid, (s_scan_list+j)->ssid ))
			{
				sprintf(uci_set_str,"uci set wireless.@wifi-iface[1].ssid='%s'",(s_wifi_list+i)->ssid);
				system(uci_set_str);
				memset(uci_set_str,0,sizeof(uci_set_str));
				//sprintf(uci_set_str,"uci set wireless.@wifi-iface[1].bssid='%s'",(s_wifi_list+i)->mac);
				//some ssid have diffrent mac
				sprintf(uci_set_str,"uci set wireless.@wifi-iface[1].bssid='%s'",(s_scan_list+j)->mac);
				system(uci_set_str);
				memset(uci_set_str,0,sizeof(uci_set_str));
				sprintf(uci_set_str,"uci set wireless.@wifi-iface[1].encryption='%s'",(s_wifi_list+i)->encryption);
				system(uci_set_str);
				memset(uci_set_str,0,sizeof(uci_set_str));
				if(strcmp((s_wifi_list+i)->encryption,"none")!=0)
				{
					sprintf(uci_set_str,"uci set wireless.@wifi-iface[1].key='%s'",(s_wifi_list+i)->key);
					system(uci_set_str);
					memset(uci_set_str,0,sizeof(uci_set_str));
				}
				system("uci commit");
				
				flag=1;
				break;
			}
		}
		if(flag==1)
			break;
	}
	if(flag==1)
	{
		printf("ok");
	}
	else
	{
		printf("fail");
	}
	usleep(10000);

	free(s_scan_list);
	free(s_wifi_list);
	return 0;

error_exit:
	printf("fail");
	uci_free_context(ctx);
	return -1;
}
