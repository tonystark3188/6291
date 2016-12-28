#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>        
#include <linux/wireless.h>
#include "socket_uart.h"
#include "notify_server.h"
#include "uci_for_cgi.h"


#define DISK_P4_DEV		"/dev/mmcblk0p4"

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

#define ARRAY_SIZE(x)		((sizeof(x))/(sizeof(x[0])))

#define DBR_HEAD_1		0xEB
#define DBR_HEAD_3		0x90

#define DBR_TAIL_1		0x55
#define DBR_TAIL_2		0xAA

#define P1_FS			0x1C2
#define P1_OFFSET		0x1C6
#define P1_SIZE			0x1Ca

#define P2_FS			0x1D2
#define P2_OFFSET		0x1D6
#define P2_SIZE			0x1Da

#define P3_FS			0x1E2
#define P3_OFFSET		0x1E6
#define P3_SIZE			0x1Ea

#define P4_FS			0x1F2
#define P4_OFFSET		0x1F6
#define P4_SIZE			0x1Fa

struct mbr_tab_item {
	unsigned int	offset;
	unsigned int	size;
	unsigned char	type;
};

struct mbr_head {
	unsigned char head1;
	unsigned char head2;
	unsigned char head3;
};

struct mbr_tail {
	unsigned char tail1;
	unsigned char tail2;
};

enum wifi_mode
{
	M_2G=2,
	M_5G=5
};


//#define IDLE_WIFI_TIME	(750) 
#define IDLE_WIFI_TIME	(1125)
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


#define INOTIFY_DIR_PATH "/tmp/notify"
#define INOTIFY_POWER "power"
#define INOTIFY_DISK  "disk"
#define INOTIFY_SSID  "ssid"

#define power_changed			0x01
#define disk_changed			0x02

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
	int status = 0;
	if(!fp)
		return -1;

	ret = notify_server_release_disk(RELEASE_DISK, &status);
	if(ret == 0 && status == 0){
		printf("notify server release disk success\n");
	}
	else{
		printf("notify server release disk fail,ret=%d,status=%d\n", ret, status);
	}
		
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

static int mount_point(void){
	int ret = 0;
	int status = 0;
	ret = notify_server_release_disk(UNRELEASE_DISK, &status);
	if(ret == 0 && status == 0){
		printf("notify server unrelease disk success\n");
	}
	else{
		printf("notify server unrelease disk fail,ret=%d,status=%d\n", ret, status);
	}
	system("block mount");
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

int get_host_power(int *host_power)
{
	int ret = 0;
	socket_uart_cmd_t g_uart_cmd;
	if(NULL == host_power)
		return -1;

	memset(&g_uart_cmd, 0, sizeof(socket_uart_cmd_t));
	g_uart_cmd.mode = UART_R;
	g_uart_cmd.regaddr = SOCKET_UART_GET_IF_POWEROFF;
	ret = SocketUartClientStart(&g_uart_cmd);
	if(ret < 0)
	{
		printf("get button status error, error code = %d \r\n");
		ret = -1;	
	}
	else
	{
		*host_power = (int)g_uart_cmd.data;
		printf("reset value = %d\n\r", *host_power);
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



int get_wifi_connect_info()
{
	char buffer[512]; 
	FILE *read_fp; 
	int chars_read; 
	int ret=0; 
	memset( buffer, 0, 512 ); 
	read_fp = popen("ifconfig | grep wlan0", "r");
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
	if(ret==0)
		return 1;

	memset( buffer, 0, 512 ); 
	read_fp = popen("wl -i wlan0 assoclist", "r");
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
	if(ret==1)
		return ret;
	memset( buffer, 0, 512 );
	read_fp = popen("wl -i wl0.1 assoclist", "r");
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
	system("echo none > /sys/class/leds/longsys\:green\:led/trigger");
	system("echo timer > /sys/class/leds/longsys\:blue\:led/trigger ");
	system("echo 100 > /sys/class/leds/longsys\:blue\:led/delay_on ");
	system("echo 100 > /sys/class/leds/longsys\:blue\:led/delay_off ");

	
}

static int get_wifi_mode()
{
	char uci_option_str[64]="\0";
	char mode[16]="\0";
	ctx=uci_alloc_context();
	strcpy(uci_option_str,"wireless2.@wifi[0].mode"); 
	uci_get_option_value(uci_option_str,mode);
	memset(uci_option_str,'\0',64);
	// d_printf("wifimode=%s\n", mode);

	uci_free_context(ctx);

	if(strcmp(mode,"2g")==0)
		return M_2G;
	else
		return M_5G;
}

int dm_cycle_change_inotify(int status)
{
	FILE *p;
	char inotify_path[64];
	memset(inotify_path, 0, 64);
	if(status == power_changed)
		sprintf(inotify_path, "%s/%s", INOTIFY_DIR_PATH, INOTIFY_POWER);
	else if(status == disk_changed)
		sprintf(inotify_path, "%s/%s", INOTIFY_DIR_PATH, INOTIFY_DISK);
	else
		return -1;
	if(access(inotify_path,F_OK)==0)
	{
		remove(inotify_path);/*  ????????????*/
	}
	else
	{
		p=fopen(inotify_path,"w");/* ????*/
		fclose(p);
	}
	return 0;
}

//return value:-1:error; 1: no need fix; 0: need fix
int check_mbr()
{
	int fd = -1;
	unsigned char block_mbr2[512];
	unsigned char block_dbr[512];
	struct mbr_tab_item	tab_item_mbr2[4];
	struct mbr_head dm_head_mbr2;
	struct mbr_tail dm_tail_mbr2;
	struct mbr_head dm_head_dbr;
	struct mbr_tail dm_tail_dbr;
	memset(tab_item_mbr2, 0, ARRAY_SIZE(tab_item_mbr2));
	memset(&dm_head_mbr2, 0, sizeof(struct mbr_head));
	memset(&dm_tail_mbr2, 0, sizeof(struct mbr_tail));
	memset(&dm_head_dbr, 0, sizeof(struct mbr_head));
	memset(&dm_tail_dbr, 0, sizeof(struct mbr_tail));
	memset(block_mbr2, 0, ARRAY_SIZE(block_mbr2));
	memset(block_dbr, 0, ARRAY_SIZE(block_dbr));

	fd = open(DISK_P4_DEV,O_RDONLY);
	if(fd < 0)
	{
		printf("open %s failed.\n",DISK_P4_DEV);
		return -1;
	}
	
	if(read(fd, block_mbr2, 512) != 512)
	{
		printf("read %s failed.\n",DISK_P4_DEV);
		close(fd);
		return -1;
	}

	memcpy(&tab_item_mbr2[0].offset,block_mbr2+P1_OFFSET,sizeof(unsigned int));	
	memcpy(&tab_item_mbr2[0].size,block_mbr2+P1_SIZE,sizeof(unsigned int));
	memcpy(&tab_item_mbr2[0].type,block_mbr2+P1_FS,sizeof(unsigned char));

	memcpy(&dm_head_mbr2.head1,block_mbr2+0,sizeof(unsigned char));
	memcpy(&dm_head_mbr2.head2,block_mbr2+1,sizeof(unsigned char));
	memcpy(&dm_head_mbr2.head3,block_mbr2+2,sizeof(unsigned char));

	memcpy(&dm_tail_mbr2.tail1,block_mbr2+0x1FE,sizeof(unsigned char));
	memcpy(&dm_tail_mbr2.tail2,block_mbr2+0x1FF,sizeof(unsigned char));

	//判断第四个分区首512字节是否是MBR
	if((dm_head_mbr2.head1 == DBR_HEAD_1) && (dm_head_mbr2.head3 == DBR_HEAD_3) && 
		(dm_tail_mbr2.tail1 == DBR_TAIL_1) && (dm_tail_mbr2.tail2 == DBR_TAIL_2)){
		printf("no seconf mbr\n");
		close(fd);
		return 1;
	}
	else if((dm_tail_mbr2.tail1 != DBR_TAIL_1) && (dm_tail_mbr2.tail2 != DBR_TAIL_2)){
		printf("no seconf mbr or dbr\n");
		close(fd);
		return 1;
	}else{
		printf("has second mbr\n");
	}

	//如果为MBR，判断分区偏移地址是否为DBR
	unsigned long long dbr_offset = tab_item_mbr2[0].offset << 9;
	lseek64(fd, dbr_offset, SEEK_SET);
	
	if(read(fd, block_dbr, 512) != 512)
	{
		printf("read %s failed.\n",DISK_P4_DEV);
		close(fd);
		return -1;
	}

	memcpy(&dm_head_dbr.head1,block_dbr+0,sizeof(unsigned char));
	memcpy(&dm_head_dbr.head2,block_dbr+1,sizeof(unsigned char));
	memcpy(&dm_head_dbr.head3,block_dbr+2,sizeof(unsigned char));

	memcpy(&dm_tail_dbr.tail1,block_dbr+0x1FE,sizeof(unsigned char));
	memcpy(&dm_tail_dbr.tail2,block_dbr+0x1FF,sizeof(unsigned char));

	//判断是否为DBR
	if((dm_head_dbr.head1 == DBR_HEAD_1) && (dm_head_dbr.head3 == DBR_HEAD_3) && 
		(dm_tail_dbr.tail1 == DBR_TAIL_1) && (dm_tail_dbr.tail2 == DBR_TAIL_2)){
		printf("has dbr\n");
	}
	else{
		printf("no dbr\n");
		close(fd);
		return 1;
	}

	close(fd);
	return 0;
}

//wl -i wl0.1 assoclist
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
	int power_sta=0;
	int host_power_sta_new=0;
	int host_power_sta_old=0;
	
	int    ret;
	unsigned int *pdata = NULL;
	
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_id < 0){
		printf("error::Open socket error!\n\n");
		return -1;
	}

	get_pc_detect_status(&detect_status_old);
#if 0
	if(detect_status_old == DETECT_BOARD)
	{
		set_storage_direction(STORAGE_BOARD);
		sleep(1);
	}
	
#endif
	get_power_status(&power_sta);
	if(power_sta == NORMAL_POWER )
	{
		//turn on wifi led;
		if(get_wifi_mode()==M_2G)
		{
			system("echo 0 > /sys/class/leds/longsys\:green\:led/brightness");
			system("echo none > /sys/class/leds/longsys\:green\:led/trigger");
			system("echo netdev > /sys/class/leds/longsys\:blue\:led/trigger");
			system("echo wl0.1 > /sys/class/leds/longsys\:blue\:led/device_name");
			system("echo link tx rx > /sys/class/leds/longsys\:blue\:led/mode");
		}
		else
		{
			system("echo 0 > /sys/class/leds/longsys\:blue\:led/brightness");
			system("echo none > /sys/class/leds/longsys\:blue\:led/trigger");
			system("echo netdev > /sys/class/leds/longsys\:green\:led/trigger");
			system("echo wl0.1 > /sys/class/leds/longsys\:green\:led/device_name");
			system("echo link tx rx > /sys/class/leds/longsys\:green\:led/mode");
		}
		system("init_network.sh &");
		sleep(3);
		system("/etc/init.d/ushare start");
	}
	else
	{
		system("echo 0 > /sys/class/leds/longsys\:blue\:led/brightness");
		system("echo 0 > /sys/class/leds/longsys\:green\:led/brightness");
		system("echo none > /sys/class/leds/longsys\:blue\:led/trigger");
	}
	ret=get_host_power(&host_power_sta_old);
	printf("host_power_sta_old = %d\n", host_power_sta_old);
	while(1){	
		usleep(800000);
		
		ret = get_button_reset_status(&reset_status);
		if(ret < 0){
			continue;
		}
		ret= get_host_power(&host_power_sta_new);
		if(ret<0)
			continue;
#if 1
		printf("host_power_sta_old = %d, host_power_sta_new = %d\n", host_power_sta_old,host_power_sta_new);

		if( (host_power_sta_new == HOST_POWER_ON) && (host_power_sta_old == HOST_POWER_OFF) )
		{ //turn on the wifi
			if(get_wifi_mode()==M_2G)
			{
				system("echo 1 > /sys/class/leds/longsys\:blue\:led/brightness");
				system("echo 0 > /sys/class/leds/longsys\:green\:led/brightness");
				system("echo netdev > /sys/class/leds/longsys\:blue\:led/trigger");
				system("echo wl0.1 > /sys/class/leds/longsys\:blue\:led/device_name");
				system("echo link tx rx > /sys/class/leds/longsys\:blue\:led/mode");

			}
			else
			{
				system("echo 1 > /sys/class/leds/longsys\:green\:led/brightness");
				system("echo 0 > /sys/class/leds/longsys\:blue\:led/brightness");
				system("echo netdev > /sys/class/leds/longsys\:green\:led/trigger");
				system("echo wl0.1 > /sys/class/leds/longsys\:green\:led/device_name");
				system("echo link tx rx > /sys/class/leds/longsys\:green\:led/mode");
				
			}
			system("init_network.sh &");
			system("killall ushare");
			sleep(3);
			system("/etc/init.d/ushare start");
		}
		else if( (host_power_sta_new == HOST_POWER_OFF) && (host_power_sta_old == HOST_POWER_ON) )
		{ //turn off wifi
			system("echo 0 > /sys/class/leds/longsys\:blue\:led/brightness");
			system("echo 0 > /sys/class/leds/longsys\:green\:led/brightness");
			system("killall udhcpc");
			system("killall dnsmasq");
			system("killall wpa_supplicant");
			system("ifconfig wlan0 down");
			int status;
			ret = notify_server_release_disk(RELEASE_DISK, &status);
			if(ret == 0 && status == 0){
				printf("notify server release disk success\n");
			}
			else{
				printf("notify server release disk fail,ret=%d,status=%d\n", ret, status);
			}
		}
		host_power_sta_old=host_power_sta_new;
#endif	
		//usleep(200000);
		ret = get_pc_detect_status(&detect_status_now);
		if(ret < 0){
			continue;
		}

		printf("detect_status_now = %d, detect_status_old = %d\n", detect_status_now, detect_status_old);
		if((detect_status_old == DETECT_BOARD) && (detect_status_now == DETECT_PC)){
			fuser_mount_point();
			usleep(500000);
			dm_cycle_change_inotify(disk_changed);
			detect_status_old = detect_status_now;
			#if 0
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
			#endif
		}
		else if((detect_status_old == DETECT_PC) && (detect_status_now == DETECT_BOARD)){
			if(0 == check_mbr()){
				system("sync");
				system("reboot -f");
			}
			mount_point();
			usleep(500000);
			dm_cycle_change_inotify(disk_changed);
			detect_status_old = detect_status_now;
			#if 0
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
			#endif
		}
					
		if(detect_status_now != DETECT_PC && reset_status == BUTTON_DOWN_LED_OFF){	
			fuser_mount_point();
			system("echo none > /sys/class/leds/longsys\:blue\:led/trigger");
			system("echo none > /sys/class/leds/longsys\:green\:led/trigger");
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
					system("reboot -f");
					// ret = set_reset_mcu();
					// if(ret < 0){
					// 	printf("set reset mcu fail!\n\r");
					// }
					// else{
					// 	printf("set reset mcu success!\n\r");

					// }	
				}
			}
		}
		else if(reset_status == BUTTON_DOUBLE){
			system("sh /sbin/2g_5g_switch.sh &");
		}
		
		ret = get_wifi_connect_info();
		
		
		if( ret==0 ){
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

	}
	close(socket_id);
}

