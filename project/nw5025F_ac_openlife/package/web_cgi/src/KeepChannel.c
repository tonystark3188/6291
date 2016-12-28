/********************************************************
* 1.保持ap和client的通道号一致
* 2.当远端无线路由掉电或者pisen远离它时，关闭wpa_supplicant,重启hostapd，
*   防止手机连接不上pisen
*
*********************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define FALSE -1
#define TRUE 0

//检测wlan1的状态
int iwconfig_wlan1()
{
	char buffer[512]; 
	FILE *read_fp; 
	int chars_read; 
	int ret=0; 
	memset( buffer, 0, 512 ); 
	read_fp = popen("iwconfig wlan1 | grep Frequency", "r");
	if(read_fp!=NULL)
	{
		chars_read = fread(buffer, sizeof(char), 512-1, read_fp); 
		if (chars_read > 0)   	//connect !
		{ 
			//printf("%s",buffer);
			ret = 1; 
		} 
		else 					//disconnect !
		{ 
			ret = 0; 
		} 
		pclose(read_fp); 
	}
	return ret;
}

//检测是否获得ip
int route_wlan1()
{
	char buffer[512]; 
	FILE *read_fp; 
	int chars_read; 
	int ret=0; 
	memset( buffer, 0, 512 ); 
	read_fp = popen("route | grep wlan1", "r");
	if(read_fp!=NULL)
	{
		chars_read = fread(buffer, sizeof(char), 512-1, read_fp); 
		if (chars_read > 0)   	//got ip !
		{ 
			//printf("%s",buffer);
			ret = 1; 
		} 
		else 					//no ip !
		{ 
			ret = 0; 
		} 
		pclose(read_fp); 
	}
	return ret;
}

int check_wpa_supplicant()
{
	char buffer[512]; 
	FILE *read_fp; 
	int chars_read; 
	int ret=0; 
	memset( buffer, 0, 512 ); 
	read_fp = popen("ps | grep wpa_supplicant | grep -v grep", "r");
	if(read_fp!=NULL)
	{
		chars_read = fread(buffer, sizeof(char), 512-1, read_fp); 
		if (chars_read > 0)   	//got ip !
		{ 
			//printf("%s",buffer);
			ret = 1; 
		} 
		else 					//no ip !
		{ 
			ret = 0; 
		} 
		pclose(read_fp); 
	}
	return ret;
}

int get_wlan0_channel()
{
	int wlan0_channel=0;
	char channel0[16]="\0";
	char buffer[128]; 
	FILE *read_fp; 
	int chars_read; 
	int ret=0; 
	char *tmp;
	char i;
	char ch[10];
	memset( buffer, 0, sizeof(buffer) ); 
	read_fp = popen("iw wlan0 info | grep \"channel\"", "r");
	if(read_fp!=NULL)
	{
		chars_read = fread(buffer, sizeof(char), sizeof(buffer)-1, read_fp); 
		if (chars_read > 0)   	
		{ 

			tmp = strstr(buffer, "channel");
			
			tmp = tmp+strlen("channel ");
			
			printf("%s",buffer);
			
			i = 0;
			while(1)
			{
				if(tmp[i] == ' ')
					break;
				else
					ch[i]=tmp[i];
			}
			
			ch[i]=0;
			printf("ch %s\n", ch);
			wlan0_channel=atoi(ch);
			printf("%d\n",wlan0_channel);
			ret = wlan0_channel; 
		} 
		else 					
		{ 
			ret = 0; 
		} 
		pclose(read_fp); 
	}
	return ret;
}

int get_wlan1_channel()
{
	int wlan1_channel=0;
	char channel1[16]="\0";
	char buffer[128]; 
	FILE *read_fp; 
	int chars_read; 
	int ret=0; 
	memset( buffer, 0, sizeof(buffer) ); 
	read_fp = popen("iwlist wlan1 channel | grep \"Current Frequency:\" | awk '{print $5}' | awk -F')' '{print $1}'", "r");
	if(read_fp!=NULL)
	{
		chars_read = fread(buffer, sizeof(char), sizeof(buffer)-1, read_fp); 
		if (chars_read > 0)   	
		{ 
			//printf("%s",buffer);
			wlan1_channel=atoi(buffer);
			//printf("%d\n",wlan1_channel);
			ret = wlan1_channel; 
		} 
		else 					
		{ 
			ret = 0; 
		} 
		pclose(read_fp); 
	}
	return ret;
}

int get_wifi_client_status()
{
	int iwconfig_wlan1_status=0;
	int route_wlan1_status=0;
	int ret=0;
	
	iwconfig_wlan1_status=iwconfig_wlan1();
	route_wlan1_status=route_wlan1();
	if(iwconfig_wlan1_status && route_wlan1_status)
	{
		ret=1;
	}
	else if(iwconfig_wlan1_status==0 || route_wlan1_status==0)
	{
		ret=0;
	}
	
	return ret;
	
	
}

int if_wifi_client_is_connecting()
{
	FILE *fp;
	int ret=0;
	if( (fp=fopen("/tmp/wifi_client_is_connecting","r")) != NULL )
	{
		ret=1;
		fclose(fp);
	}
	else
		ret=0;
	return ret;
}

int if_manul_shutdown_client()
{
	FILE *fp;
	int ret=0;
	if( (fp=fopen("/tmp/manul_shutdown_client","r")) != NULL )
	{
		ret=1;
		fclose(fp);
		system("rm -f /tmp/manul_shutdown_client");
	}
	else
		ret=0;
	return ret;
}

int main(int argc,char *argv[])
{
	int wlan0_channel=0;
	int wlan1_channel=0;
	
	int cur_wifi_client_status=0;
	int old_wifi_client_status=0;
	
	int connect_flag=0;
	
	char system_str[128]="\0";
	memset(system_str,0,sizeof(system_str));
#if 0	
	sleep(10);
	if(get_wifi_client_status()==0)
	{
		system("killall wpa_supplicant >/dev/null 2>&1");
		system("killall hostapd ");
		//sleep(1);
		system("sed -e 's/HT20/HT40+/g' -i /var/run/hostapd-phy0.conf");
		system("hostapd -P /var/run/wifi-phy0.pid -B /var/run/hostapd-phy0.conf");
		sleep(1);
		system("ifconfig wlan1 up");
	}
#endif
	while(1)
	{
#if 0	
		cur_wifi_client_status=get_wifi_client_status();
		if(cur_wifi_client_status==0)
		{
			if(old_wifi_client_status==1)  //之前连接成功过，现在断开
			{
				printf("\nwifi client is disconnected\n");
				if(if_wifi_client_is_connecting()==0)
				{
					system("killall wpa_supplicant >/dev/null 2>&1");
					system("killall hostapd ");
					//sleep(1);
					system("sed -e 's/HT20/HT40+/g' -i /var/run/hostapd-phy0.conf");
					system("hostapd -P /var/run/wifi-phy0.pid -B /var/run/hostapd-phy0.conf");
					sleep(1);
					if(if_manul_shutdown_client()==1)  //手动关闭wifi client
						system("ifconfig wlan1 down");
					else
						system("ifconfig wlan1 up");
					connect_flag=0;
					//system("iw wlan0 set channel 1");
				}
				else   //用户正在连接
				{
					sleep(10);   //等待10秒后，再次检测连接是否成功
					if( get_wifi_client_status()==0)
					{
						system("killall wpa_supplicant >/dev/null 2>&1");
						system("killall hostapd ");
						//sleep(1);
						system("sed -e 's/HT20/HT40+/g' -i /var/run/hostapd-phy0.conf");
						system("hostapd -P /var/run/wifi-phy0.pid -B /var/run/hostapd-phy0.conf");
						sleep(1);
						system("ifconfig wlan1 up");
						connect_flag=0;
						//system("iw wlan0 set channel 1");
					}
				}
			}
			else if(old_wifi_client_status==0 && connect_flag==0)  //没有连接成功过
			{
				if(check_wpa_supplicant()==1)
				{
					if(if_wifi_client_is_connecting()==0)  //
					{
						system("killall wpa_supplicant >/dev/null 2>&1");
						system("killall hostapd ");
						//sleep(1);
						system("sed -e 's/HT20/HT40+/g' -i /var/run/hostapd-phy0.conf");
						system("hostapd -P /var/run/wifi-phy0.pid -B /var/run/hostapd-phy0.conf");
						sleep(1);
						system("ifconfig wlan1 up");
						connect_flag=0;
						//system("iw wlan0 set channel 1");
					}
					else   //用户正在连接
					{
						sleep(10);   //等待10秒后，再次检测连接是否成功
						if( get_wifi_client_status()==0)
						{
							system("killall wpa_supplicant >/dev/null 2>&1");
							system("killall hostapd ");
							//sleep(1);
							system("sed -e 's/HT20/HT40+/g' -i /var/run/hostapd-phy0.conf");
							system("hostapd -P /var/run/wifi-phy0.pid -B /var/run/hostapd-phy0.conf");
							sleep(1);
							system("ifconfig wlan1 up");
							connect_flag=0;
							//system("iw wlan0 set channel 1");
						}
					}
					
				}
			}
		}
#endif
		if((wlan0_channel=get_wlan0_channel())!=0 && (wlan1_channel=get_wlan1_channel())!=0 && get_wifi_client_status())
		{
			connect_flag=1;
			if(wlan0_channel!=wlan1_channel)
			{
				
				printf("wlan0_channel=%d,wlan1_channel=%d\n",wlan0_channel,wlan1_channel);
				sprintf(system_str,"iw wlan0 set channel %d",wlan1_channel);
				system(system_str);
				printf("%s\n",system_str);
				memset(system_str,0,sizeof(system_str));
			}
		}

		
		old_wifi_client_status=cur_wifi_client_status;
		sleep(60);
	}
	return 0;
}





