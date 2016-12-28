#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>        
#include <linux/wireless.h>
#include "socket_uart.h"


#define reset_status_set  13
#define reset_status_get  14
#define key_status_set  15
#define key_status_get  16
#define shutdown_set  17
#define CMD  18

#define SHUTDOWN  6
#define STATUS_WIFILED_OFF	3
#define STATUS_RESTORE		6
#define STATUS_RESTORE_ACK	9
#define STATUS_KEY_PRESS	7
#define STATUS_KEYPRESS_ACK	4



#define IDLE_WIFI_TIME	(750) 
#define __DEBUG__  

#if WIRELESS_EXT <= 11
#ifndef SIOCDEVPRIVATE
#define SIOCDEVPRIVATE      0x8BE0
#endif
#define SIOCIWFIRSTPRIV      SIOCDEVPRIVATE
#endif

#define RT_PRIV_IOCTL    (SIOCIWFIRSTPRIV + 0x01)
#define RTPRIV_IOCTL_SET       (SIOCIWFIRSTPRIV + 0x02)
#define RTPRIV_IOCTL_BBP   (SIOCIWFIRSTPRIV + 0x03)
#define RTPRIV_IOCTL_MAC    (SIOCIWFIRSTPRIV + 0x05)
#define RTPRIV_IOCTL_E2P    (SIOCIWFIRSTPRIV + 0x07)
#define RTPRIV_IOCTL_STATISTICS    (SIOCIWFIRSTPRIV + 0x09)
#define RTPRIV_IOCTL_ADD_PMKID_CACHE    (SIOCIWFIRSTPRIV + 0x0A)
#define RTPRIV_IOCTL_RADIUS_DATA     (SIOCIWFIRSTPRIV + 0x0C)
#define RTPRIV_IOCTL_GSITESURVEY    (SIOCIWFIRSTPRIV + 0x0D)
#define RTPRIV_IOCTL_ADD_WPA_KEY    (SIOCIWFIRSTPRIV + 0x0E)
#define RTPRIV_IOCTL_GET_MAC_TABLE    (SIOCIWFIRSTPRIV + 0x0F)
#define RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT	(SIOCIWFIRSTPRIV + 0x1F)
#define RTPRIV_IOCTL_SHOW_CONNSTATUS	(SIOCIWFIRSTPRIV + 0x1B)
#define RTPRIV_IOCTL_GET_APCLI_RSSI	    (SIOCIWFIRSTPRIV + 0x1C)

#define MAC_ADDR_LEN        6
#define ETH_LENGTH_OF_ADDRESS      6
#define MAX_LEN_OF_MAC_TABLE      64



typedef union _MACHTTRANSMIT_SETTING {
	struct {
		unsigned short MCS:7;	/* MCS */
		unsigned short BW:1;	/*channel bandwidth 20MHz or 40 MHz */
		unsigned short ShortGI:1;
		unsigned short STBC:2;	/*SPACE */
		unsigned short rsv:3;
		unsigned short MODE:2;	/* Use definition MODE_xxx. */
	} field;
	unsigned short word;
} MACHTTRANSMIT_SETTING, *PMACHTTRANSMIT_SETTING;

typedef struct _RT_802_11_MAC_ENTRY {
	unsigned char ApIdx;
	unsigned char Addr[MAC_ADDR_LEN];
	unsigned char Aid;
	unsigned char Psm;		/* 0:PWR_ACTIVE, 1:PWR_SAVE */
	unsigned char MimoPs;		/* 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled */
	signed char AvgRssi0;
	signed char AvgRssi1;
	signed char AvgRssi2;
	unsigned int ConnectedTime;
	MACHTTRANSMIT_SETTING TxRate;
	unsigned int LastRxRate;
	signed short StreamSnr[3];				/* BF SNR from RXWI. Units=0.25 dB. 22 dB offset removed */
	signed short SoundingRespSnr[3];			/* SNR from Sounding Response. Units=0.25 dB. 22 dB offset removed */
/*	signed short TxPER;	*/					/* TX PER over the last second. Percent */
/*	signed short reserved;*/

} RT_802_11_MAC_ENTRY, *PRT_802_11_MAC_ENTRY;

typedef struct _RT_802_11_MAC_TABLE {
	unsigned long Num;
	RT_802_11_MAC_ENTRY Entry[MAX_LEN_OF_MAC_TABLE];
} RT_802_11_MAC_TABLE, *PRT_802_11_MAC_TABLE;

#ifdef __DEBUG__  
//#define DEBUG(format,...) printf("File: "__FILE__", Line: %05d: "format"/n", __LINE__, ##__VA_ARGS__)  
#define DEBUG(format,...) printf(format, ##__VA_ARGS__) 
#else  
#define DEBUG(format,...)  
#endif 


struct cmd_info{
	unsigned char regaddr;
	unsigned char data;
	unsigned char mode;  // 0 --read  1--write
};
enum led_status
{	
	ON,
	OFF
};




static void poll_online(void)
{
	system("iwpriv ra0 show stainfo");
	system("iwpriv apcli0 show connStatus");
}
static void suspend_usb(void)
{
	system("sync");
	system("reg s 0xb01c0000");
	system("reg w 54 0x1084");
}
static void resume_usb(void)
{
	system("reg s 0xb01c0000");
	system("reg w 54 0x1100");
	system("reg w 54 0x1004");
}

static void suspend_wifi(void)
{
	system("/etc/init.d/network stop");
	system("echo timer > /sys/class/leds/longsys\:wifi\:led/trigger ");
	system("echo 100 > /sys/class/leds/longsys\:wifi\:led/delay_on ");
	system("echo 1000 > /sys/class/leds/longsys\:wifi\:led/delay_off ");
}
static void resume_wifi(void)
{
	system("/etc/init.d/network start");
	system("/etc/init.d/led restart");
}

static void suspend_service(void)
{
	system("/etc/init.d/apache stop");
	system("/etc/init.d/samba stop");
	system("/etc/init.d/ushare stop");
	system("/etc/init.d/telnet stop");
	system("/etc/init.d/dnsmasq stop");
	system("killall auto_connect.sh");
	system("killall config_dns.sh");
	system("killall check_shair.sh");
	system("/usr/mips/cgi-bin/script/control_newshair.sh  stop");
	system("killall dm_monitor");
	system("killall dm_server");
	
}
static void resume_service(void)
{
	system("/etc/init.d/apache start");
	system("/etc/init.d/samba start");
	system("/etc/init.d/ushare start");
	system("/etc/init.d/telnet start");
	system("/etc/init.d/dnsmasq start");
	system("/usr/mips/cgi-bin/script/auto_connect.sh  &");
	system("/usr/mips/cgi-bin/script/config_dns.sh  &");
	system("/usr/mips/cgi-bin/script/control_newshair.sh  start");
	system("/usr/mips/cgi-bin/script/check_shair.sh &");
	system("dm_server &");
	system("dm_monitor &");
}

//return value : -1 error,0 no storage,1 have storage
static int fuser_mount_point(void)
{
	char disk_type[3][24]={"USB-disk-","SD-disk-","Private-disk-"};
	FILE *fp = fopen("/proc/mounts", "r");
	char line[256];
	int count = 0;
	char mount_point[32];
	char cmd[128];
	int i = 0;
	char *p1 = NULL;
	char *p2 = NULL;
	int ret = 0;
	if(!fp)
		return -1;

	while (fgets(line, sizeof(line), fp))
	{
		if(strstr(line,disk_type[0]) || strstr(line,disk_type[1]) || strstr(line,disk_type[2]))
		{
			ret = 1;
			p1 = strstr(line," ");
			p2 = strstr(p1+1," ");
			*p2 = '\0';
			memset(cmd,0,sizeof(cmd));
			memset(mount_point,0,sizeof(mount_point));
			strcpy(mount_point,p1+1);
			printf("==>mount point:%s\n",mount_point);
			sprintf(cmd,"fuser -km %s",mount_point);
			system(cmd);
			system("sync");
		}
	}
	system("block umount");
	fclose(fp);
	return ret;
	
}


int get_button_reset_status(int *reset_status)
{
	int ret = 0;
	socket_uart_cmd_t g_uart_cmd;
	if(NULL == reset_status)
		return -1;

	memset(&g_uart_cmd, 0, sizeof(socket_uart_cmd_t));
	g_uart_cmd.mode = UART_R;
	g_uart_cmd.regaddr = SOCKET_UART_GET_BOTTON_STATUS;
	ret = SocketUartClientStart(&g_uart_cmd);
	if(ret < 0)
	{
		printf("get button status error, error code = %d \r\n");
		ret = -1;	
	}
	else
	{
		*reset_status = (int)g_uart_cmd.data;
		printf("reset value = %d\n\r", *reset_status);
	}
	return ret;
}

int get_power_status(int *power_status)
{
	int ret = 0;
	socket_uart_cmd_t g_uart_cmd;
	if(NULL == power_status)
		return -1;

	memset(&g_uart_cmd, 0, sizeof(socket_uart_cmd_t));
	g_uart_cmd.mode = UART_R;
	g_uart_cmd.regaddr = SOCKET_UART_GET_POWER_STATUS;
	ret = SocketUartClientStart(&g_uart_cmd);
	if(ret < 0)
	{
		printf("get button status error, error code = %d \r\n");
		ret = -1;	
	}
	else
	{
		*power_status = (int)g_uart_cmd.data;
		printf("reset value = %d\n\r", *power_status);
	}
	return ret;
}

int get_pc_detect_status(int *detect_status)
{
	int ret = 0;
	socket_uart_cmd_t g_uart_cmd;
	if(NULL == detect_status)
		return -1;

	memset(&g_uart_cmd, 0, sizeof(socket_uart_cmd_t));
	g_uart_cmd.mode = UART_R;
	g_uart_cmd.regaddr = SOCKET_UART_GET_PC_DETEC;
	ret = SocketUartClientStart(&g_uart_cmd);
	if(ret < 0)
	{
		printf("get pc detect status error, error code = %d \r\n");
		ret = -1;	
	}
	else
	{
		*detect_status = (int)g_uart_cmd.data;
		printf("detect value = %d\n\r", *detect_status);
	}
	return ret;
}


int set_storage_direction(int storage_dir)
{
	int ret = 0;
	socket_uart_cmd_t g_uart_cmd;
	
	g_uart_cmd.mode = UART_W;
	g_uart_cmd.regaddr = SOCKET_UART_SET_STORAGE_DIRECTION;
	g_uart_cmd.data = (unsigned short)storage_dir;
	ret = SocketUartClientStart(&g_uart_cmd);
	if(ret < 0)
	{
		printf("set storage direction error, error code = %d \r\n");
		ret = -1;	
	}
	else
	{
		printf("set storage direction success\n");
	}
	return ret;
}


int get_storage_direction(int *storage_dir)
{
	int ret = 0;
	socket_uart_cmd_t g_uart_cmd;
	if(NULL == storage_dir)
		return -1;

	memset(&g_uart_cmd, 0, sizeof(socket_uart_cmd_t));
	g_uart_cmd.mode = UART_R;
	g_uart_cmd.regaddr = SOCKET_UART_GET_STORAGE_DIRECTION;
	ret = SocketUartClientStart(&g_uart_cmd);
	if(ret < 0)
	{
		printf("get storage direction error, error code = %d \r\n");
		ret = -1;	
	}
	else
	{
		*storage_dir = (int)g_uart_cmd.data;
		printf("storage dir value = %d\n\r", *storage_dir);
	}
	return ret;
}


int set_reset_mcu(void)
{
	int ret = 0;
	socket_uart_cmd_t g_uart_cmd;
	
	g_uart_cmd.mode = UART_W;
	g_uart_cmd.regaddr = SOCKET_UART_SET_RESET;
	ret = SocketUartClientStart(&g_uart_cmd);
	if(ret < 0)
	{
		printf("set reset mcu error, error code = %d \r\n");
		ret = -1;	
	}
	else
	{
		printf("set mcu reset success\n");
	}
	return ret;
}

int set_system_poweroff(void)
{
	int ret = 0;
	socket_uart_cmd_t g_uart_cmd;
	
	g_uart_cmd.mode = UART_W;
	g_uart_cmd.regaddr = SOCKET_UART_SET_POWEROFF;
	ret = SocketUartClientStart(&g_uart_cmd);
	if(ret < 0)
	{
		printf("set system poweroff error, error code = %d \r\n");
		ret = -1;	
	}
	else
	{
		printf("set system poweroff success\n");
	}
	return ret;
}



int get_wifi_connect_info(int socket_id, int *current_sta_cnt, int *client_status)
{
	struct   iwreq wrq;
	char data[2048];
	int    ret;
	RT_802_11_MAC_TABLE    *mp;
	int    i;
	memset(data, 0x00, 1024);
	strcpy(wrq.ifr_name, "ra0");
	wrq.u.data.length = 1024;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;
	ret = ioctl(socket_id, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &wrq);
	if(ret != 0){
		printf("error::get mac table\n\n");
		return -1;
	}
	mp = (RT_802_11_MAC_TABLE *)wrq.u.data.pointer;
	*current_sta_cnt = mp->Num;

	memset(data, 0x00, 128);
	strcpy(wrq.ifr_name, "apcli0");
	wrq.u.data.length = 128;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;
	ret = ioctl(socket_id, RTPRIV_IOCTL_SHOW_CONNSTATUS, &wrq);
	if(ret != 0){
		printf("error::get apcli0 connstatus error\n\n");
		return -1;
	}
	*client_status = *(unsigned int *)wrq.u.data.pointer;

	return 0;
}

void system_reset(void)
{
#if 0
	system("jffs2reset -y");
	//system("/bin/restore_factory_from_cfg");
	system("block umount");
	usleep(200000);
	system("stm8_control 6");  //logout private disk
	system("echo timer > /sys/class/leds/longsys\:wifi\:led/trigger ");
	system("echo 100 > /sys/class/leds/longsys\:wifi\:led/delay_on ");
	system("echo 100 > /sys/class/leds/longsys\:wifi\:led/delay_off ");
#endif
	system("sh /lib/netifd/reset2factory.sh");
	system("block umount");
	usleep(200000);
	//system("stm8_control 6");  //logout private disk
	system("echo timer > /sys/class/leds/longsys\:wifi\:led/trigger ");
	system("echo 100 > /sys/class/leds/longsys\:wifi\:led/delay_on ");
	system("echo 100 > /sys/class/leds/longsys\:wifi\:led/delay_off ");

	
}

int main() 
{
	int fh=NULL;
	int fp_online=NULL;
	int reset_status = 0;
	int detect_status_now = 0;
	int detect_status_old = 0;
	int storage_dir = 0;
	int current_sta_cnt = 0; 
	int client_status = 0;    //  0 -- client disconnect    1 -- client connect
	int idle_cnt=0;
	int counter = 0;
	struct cmd_info cmd;
	int i = 0;
	int    socket_id;
	int power_status_old=0;
	int power_status_now=0;
	
	int    ret;
	unsigned int *pdata = NULL;
	
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_id < 0){
		printf("error::Open socket error!\n\n");
		return -1;
	}
#if 0
	get_pc_detect_status(&detect_status_old);
	if(detect_status_old == DETECT_BOARD)
	{
		set_storage_direction(STORAGE_BOARD);
		sleep(1);
	}
#endif	
	get_power_status(&power_status_old);
	if(power_status_old == NORMAL_POWER )
	{
		//turn on wifi led;
		system("echo netdev > /sys/class/leds/longsys\:wifi\:led/trigger");
		system("echo wl0.1 > /sys/class/leds/longsys\:wifi\:led/device_name");
		system("echo link tx rx > /sys/class/leds/longsys\:wifi\:led/mode");
		system("init_network.sh &");
	}
	while(1){	
		usleep(800000);
		
		ret = get_button_reset_status(&reset_status);
		if(ret < 0){
			continue;
		}
		ret= get_power_status(&power_status_now);
		if(ret<0)
			continue;
		if( (power_status_old == NOT_NORMAL_POWER) && (power_status_now == NORMAL_POWER) )
		{
		system("echo netdev > /sys/class/leds/longsys\:wifi\:led/trigger");
		system("echo wl0.1 > /sys/class/leds/longsys\:wifi\:led/device_name");
		system("echo link tx rx > /sys/class/leds/longsys\:wifi\:led/mode");
		system("init_network.sh &");
		}
		power_status_old=power_status_now;
#if 0			
		usleep(200000);

		ret = get_pc_detect_status(&detect_status_now);
		if(ret < 0){
			continue;
		}

		printf("detect_status_now = %d, detect_status_old = %d\n", detect_status_now, detect_status_old);
		if((detect_status_old == DETECT_BOARD) && (detect_status_now == DETECT_PC)){
			fuser_mount_point();
			for(i = 0; i < 5; i++){
				usleep(500000);
				ret = set_storage_direction(STORAGE_PC);
				if(ret < 0){
					continue;
				}
				sleep(1);
				ret = get_storage_direction(&storage_dir);
				if(ret < 0){
					continue;
				}
				else if(storage_dir != STORAGE_PC){
					continue;
				}
				else{
					printf("set direction to pc success!\n\r");
					break;
				}
			}
			if(i == 5)
				printf("set direction to pc fail!\n\r");
			
		}
		else if((detect_status_old == DETECT_PC) && (detect_status_now == DETECT_BOARD)){
			for(i = 0; i < 5; i++){
				usleep(500000);
				ret = set_storage_direction(STORAGE_BOARD);
				if(ret < 0){
					continue;
				}
				sleep(1);
				ret = get_storage_direction(&storage_dir);
				if(ret < 0){
					continue;
				}
				else if(storage_dir != STORAGE_BOARD){
					continue;
				}
				else {
					printf("set direction to board success!\n\r");
					break;
				}
			}
			if(i == 5)
				printf("set direction to board fail!\n\r");
		}
		detect_status_old = detect_status_now;
#endif					
		if(reset_status == BUTTON_DOWN_LED_OFF){	
		//	fuser_mount_point();
		//	system("echo none > /sys/class/leds/longsys\:wifi\:led/trigger ");
		}
		else if(reset_status == BUTTON_DOWN_RESET){
			system_reset();
			while(1){
				//usleep(200000);
				sleep(1);
				ret = get_button_reset_status(&reset_status);
				if(ret < 0){
					continue;
				}

				printf("reset_status = %d\n", reset_status);
				if(reset_status == BUTTON_UP)
				{
					ret = set_reset_mcu();
					if(ret < 0){
						printf("set reset mcu fail!\n\r");
					}
					else{
						printf("set reset mcu success!\n\r");
					}	
				}
			}
		}
		else if(reset_status == BUTTON_DOUBLE){
			system("sh /sbin/2g_5g_switch.sh &");
		}
#if 0		
		ret = get_wifi_connect_info(socket_id, &current_sta_cnt, &client_status);
		if(ret < 0)
		{
			printf("get wifi connect info fail \n\r");
			continue;
		}
		printf("current_sta_cnt = %d \n",current_sta_cnt);
		printf("client status   = %d \n",client_status);
		printf("idle_cnt        = %d \n",idle_cnt);
		if(
(current_sta_cnt == 0) && (client_status == 0)){
			idle_cnt++;
		}
		else{
			idle_cnt=0;
		}
		if(idle_cnt >= IDLE_WIFI_TIME){
			while(1){	
				usleep(200000);
				ret = set_system_poweroff();	
				if(ret < 0){
					printf("set system poweroff fail!\n\r");
				}
				else{
					printf("set system poweroff success!\n\r");
				}
			}
		}
#endif
	}
	close(socket_id);
}

