#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>        
#include <linux/wireless.h>
#include <dirent.h>
#include <signal.h>
#include <sys/sysinfo.h>

#include "socket_uart.h"
#include "notify_server.h"
#include "uci_for_cgi.h"
#include "net_speed.h"

#define DISK_P4_DEV		"/dev/mmcblk0p1"

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

enum wifi_mode
{
	M_2G=2,
	M_5G=5
};

enum g_file_storage_module{
	G_FILE_STORAGE_SET,
	G_FILE_STORAGE_CLEAR,
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
	system("block mount");
}

static int notify_server_unrelease(void)
{
	int ret = 0;
	int status = 0;
	ret = notify_server_release_disk(UNRELEASE_DISK, &status);
	if(ret == 0 && status == 0){
		printf("notify server unrelease disk success\n");
	}
	else{
		printf("notify server unrelease disk fail,ret=%d,status=%d\n", ret, status);
	}
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
		//printf("reset value = %d\n\r", *reset_status);
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
		//printf("reset value = %d\n\r", *power_status);
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
		//printf("reset value = %d\n\r", *host_power);
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
		//printf("detect value = %d\n\r", *detect_status);
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


int get_wifi_connect_count()
{
	char buffer[512]; 
	FILE *read_fp; 
	int chars_read; 
	int ret=0; 
	int ap_cn = 0, sta_cn = 0;

	memset( buffer, 0, 512 ); 
	read_fp = popen("wl -i wlan0 assoclist | wc | awk '{print $1}'", "r");
	if(read_fp!=NULL)
	{
		chars_read = fread(buffer, sizeof(char), 512-1, read_fp); 
		if (chars_read > 0) 
		{ 
			//printf("wlan0 connect count: %s",buffer);
			ap_cn = atoi(buffer);
		}
		pclose(read_fp); 
	}
	else{
		ret = -1;
		goto EXIT;
	}
	
	memset( buffer, 0, 512 );
	read_fp = popen("wl -i wl0.1 assoclist | wc | awk '{print $1}'", "r");
	if(read_fp!=NULL)
	{
		chars_read = fread(buffer, sizeof(char), 512-1, read_fp); 
		if (chars_read > 0) 	
		{ 
			//printf("wl0.1 connect count: %s",buffer);
			sta_cn = atoi(buffer); 
		} 
		pclose(read_fp); 
	}
	else{
		ret = -1;
		goto EXIT;
	}

	ret = ap_cn + sta_cn;
EXIT:
	return ret;
}


void system_reset(void)
{
	//system("sh /lib/netifd/reset2factory.sh");
	//system("block umount");
	system("jffs2reset -y");
	system("/bin/restore_factory_from_cfg");
	system("sync");
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


#define DISK_ST_PATH "/etc/disk_st.txt"
int get_disk_st(int *disk_st)
{
	int ret = 0;
	char *disk_st_buf = NULL;
	FILE *fd = NULL;
	char *p = NULL;
	char disk_st_c;
	int len = 0;
	
	if((fd = fopen(DISK_ST_PATH, "r")) == NULL){
		printf("open %s error!\r\n", DISK_ST_PATH);
		ret = -1;
		goto EXIT;
	}

	fseek(fd, 0, SEEK_END);
	len = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	disk_st_buf = calloc(1, len+1);
	ret = fgets(disk_st_buf, len, fd);
	if(ret <= 0){
		ret = -1;
		goto EXIT;
	}

//	printf("disk_st_buf:%s\r\n", disk_st_buf);
	p = strstr(disk_st_buf, "pc_disable");
	if(p == NULL){
		ret = -1;
		goto EXIT;
	}

	p = strchr(disk_st_buf, ':');
	if(p == NULL){
		ret = -1;
		goto EXIT;
	}
	
	p++;
	if(p == NULL){
		ret = -1;
		goto EXIT;
	}

	disk_st_c = *p;

	if(disk_st_c == '1'){
		*disk_st = 1;	
	}
	else{
		*disk_st = 0;
	}
	//printf("disk_st:%d\r\n", *disk_st);

	if(fd != NULL)
		fclose(fd);
	if(disk_st_buf != NULL)
		free(disk_st_buf);
	return 0;
EXIT:
	if(fd != NULL)
		fclose(fd);
	if(disk_st_buf != NULL)
		free(disk_st_buf);
	return ret;
}




#define SLEEP_TIME_MIN 			200000//us
#define CHECK_RESET_TIMES		5
#define CHECK_PC_DET_TIMES		5
#define CHECK_HOST_POWER_TIMES	5
#define CHECK_DEV_FLOW_TIMES	15

//wl -i wl0.1 assoclist
int main() 
{
	int reset_status = 0;
	int ret;
	int detect_status_now = 0, detect_status_old = 0;
	int connect_count_now = 0, connect_count_old = 0;
	int pc_disable_now = 0, pc_disable_old = 0, pc_disable = 0;
	int host_power_sta_new=0, host_power_sta_old=0, power_sta=0;
	int storage_dir = 0;
	int idle_cnt = 0;
	int cur_reset_times = 0, cur_pc_det_times = 0, cur_dev_flow_times = 0, cur_host_power_status = 0;
	int cur_time = 0 , last_time = 0, delta_time = 0;
	char cmd[128];
	struct sysinfo info; 
	struct net_stat wlan0_net_stat;	
	memset(&wlan0_net_stat, 0, sizeof(struct net_stat));
	strcpy(wlan0_net_stat.dev, "wlan0");

	struct net_stat wl0_1_net_stat;
	memset(&wl0_1_net_stat, 0, sizeof(struct net_stat));
	strcpy(wl0_1_net_stat.dev, "wl0.1");

	sysinfo(&info); 
	last_time = info.uptime;

	get_pc_detect_status(&detect_status_old);
#if 0
	if(detect_status_old == DETECT_BOARD)
	{
		set_storage_direction(STORAGE_BOARD);
		sleep(1);
	}
	
#endif

	ret = get_disk_st(&pc_disable);
	printf("ret: %d, pc_disable: %d\r\n", ret, pc_disable);
	if(ret == 0 && pc_disable == 1){
		printf("no allow mount pc\r\n");
		pc_disable_old = 1;
		system("echo 0 >/sys/devices/platform/jz-dwc2/dwc2/gadget/gadget-lun0/file && sync");
	}
	else{
		printf("allow mount pc\r\n");
		pc_disable_old = 0;	
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "echo %s > /sys/devices/platform/jz-dwc2/dwc2/gadget/gadget-lun0/file && sync", DISK_P4_DEV);
		printf("cmd: %s\n", cmd);
		system(cmd);
		if(detect_status_old == DETECT_PC){
			fuser_mount_point();
		}
	}

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
		//mount_point();
		sleep(3);
		system("/etc/init.d/ushare start");
	}
	else
	{
		system("echo 0 > /sys/class/leds/longsys\:blue\:led/brightness");
		system("echo 0 > /sys/class/leds/longsys\:green\:led/brightness");
		system("echo none > /sys/class/leds/longsys\:blue\:led/trigger");
		if(detect_status_old == DETECT_PC)
			fuser_mount_point();
	}
	ret=get_host_power(&host_power_sta_old);
	printf("host_power_sta_old = %d\n", host_power_sta_old);

	while(1){
		cur_reset_times++;
		cur_pc_det_times++;
		cur_dev_flow_times++;

		if(cur_host_power_status >= CHECK_HOST_POWER_TIMES){
			ret= get_host_power(&host_power_sta_new);
			if(ret == 0){
				printf("host_power_sta_old = %d, host_power_sta_new = %d\n", host_power_sta_old,host_power_sta_new);
				if( (host_power_sta_new == HOST_POWER_ON) && (host_power_sta_old == HOST_POWER_OFF) )
				{ 	//turn on the wifi
					if(get_wifi_mode()==M_2G)
					{
						system("echo 0 > /sys/class/leds/longsys\:green\:led/brightness");
						system("echo 1 > /sys/class/leds/longsys\:blue\:led/brightness");
						system("echo netdev > /sys/class/leds/longsys\:blue\:led/trigger");
						system("echo wl0.1 > /sys/class/leds/longsys\:blue\:led/device_name");
						system("echo link tx rx > /sys/class/leds/longsys\:blue\:led/mode");

					}
					else
					{
						system("echo 0 > /sys/class/leds/longsys\:blue\:led/brightness");
						system("echo 1 > /sys/class/leds/longsys\:green\:led/brightness");
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
				host_power_sta_old = host_power_sta_new;
			}
			cur_host_power_status = 0;
		}

		if(cur_pc_det_times >= CHECK_PC_DET_TIMES){
			ret = get_pc_detect_status(&detect_status_now);
			if(ret == 0){
				printf("detect_status_now = %d, detect_status_old = %d\n", detect_status_now, detect_status_old);
				ret = get_disk_st(&pc_disable);
				//printf("ret: %d, pc_disable: %d\r\n", ret, pc_disable);
				if(ret == 0 && pc_disable == 1){
					//printf("no allow mount pc\r\n");
					pc_disable_now = 1;
				}
				else{
					//printf("allow mount pc\r\n");
					pc_disable_now = 0;
				}

				if((pc_disable_old == 1) && (pc_disable_now == 0)){
					printf("set allow mount pc\r\n");
					memset(cmd, 0, sizeof(cmd));
					sprintf(cmd, "echo %s > /sys/devices/platform/jz-dwc2/dwc2/gadget/gadget-lun0/file && sync", DISK_P4_DEV);
					printf("cmd: %s\n", cmd);
					system(cmd);
				}
				else if((pc_disable_old == 0) && (pc_disable_now == 1)){			
					printf("set no allow mount pc\r\n");
					system("echo 0 >/sys/devices/platform/jz-dwc2/dwc2/gadget/gadget-lun0/file && sync");
				}

				if((detect_status_old == DETECT_BOARD) && (detect_status_now == DETECT_PC)){
					if(pc_disable_now == 1){
						printf("no allow mount pc\r\n");
					}
					else{
						fuser_mount_point();
						usleep(500000);
						dm_cycle_change_inotify(disk_changed);
					}
					detect_status_old = detect_status_now;
				}
				else if((detect_status_old == DETECT_PC) && (detect_status_now == DETECT_BOARD)){
					mount_point();
					usleep(500000);
					
					if(pc_disable_now == 1){
						printf("no allow mount pc\r\n");
					}
					else{
						notify_server_unrelease();
						dm_cycle_change_inotify(disk_changed);
					}
				}
				else if(detect_status_now == DETECT_PC){
					if((pc_disable_old == 1) && (pc_disable_now == 0)){
						printf("umount umount\r\n");
						fuser_mount_point();
						usleep(500000);
						dm_cycle_change_inotify(disk_changed);
					}
					else if((pc_disable_old == 0) && (pc_disable_now == 1)){			
						printf("mount mount\r\n");
						mount_point();
						usleep(500000);
						dm_cycle_change_inotify(disk_changed);
					}
				}
				detect_status_old = detect_status_now;
				pc_disable_old = pc_disable_now;
			}
			cur_pc_det_times = 0;
		}

		if(cur_reset_times >= CHECK_RESET_TIMES){
			ret = get_button_reset_status(&reset_status);
			if(ret == 0){	
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
						}
					}
				}
				else if(reset_status == BUTTON_DOUBLE){
					system("sh /sbin/2g_5g_switch.sh &");
				}
			}
			cur_reset_times = 0;
		}

		if(cur_dev_flow_times >= CHECK_DEV_FLOW_TIMES){
			sysinfo(&info); 
			cur_time = info.uptime;
			delta_time = cur_time - last_time;
			if(delta_time >= 5){
				//get net connection
				connect_count_now = get_wifi_connect_count();
				printf("connect_count_now: %d\n", connect_count_now);
				if(connect_count_now >= 0){
					if(connect_count_now > connect_count_old){//add new connect
						idle_cnt=0;
					}
					connect_count_old = connect_count_now;
				}
				
				//get net speed
				printf("enough, cur_time: %d, last_time: %d, delta_time: %d\n", cur_time, last_time, delta_time);
				last_time = cur_time;
				ret = get_net_stat_for_dev(&wlan0_net_stat, delta_time);
				if(!ret){
		        	printf("%s: Recv Speed: %f KB/s\n",wlan0_net_stat.dev, wlan0_net_stat.recv_speed/1024);
		        	printf("%s: Send Speed: %f KB/s\n",wlan0_net_stat.dev, wlan0_net_stat.trans_speed/1024);
				}
				else{
					printf("Get net stat for %s error!!!!!!\n", wlan0_net_stat.dev);
				}

				ret = get_net_stat_for_dev(&wl0_1_net_stat, delta_time);
				if(!ret){
		        	printf("%s: Recv Speed: %f KB/s\n",wl0_1_net_stat.dev, wl0_1_net_stat.recv_speed/1024);
		        	printf("%s: Send Speed: %f KB/s\n",wl0_1_net_stat.dev, wl0_1_net_stat.trans_speed/1024);
				}
				else{
					printf("Get net stat for %s error!!!!!!\n", wl0_1_net_stat.dev);
				}

				if(wlan0_net_stat.recv_speed < 10240 && wlan0_net_stat.trans_speed < 10240 && \
				wl0_1_net_stat.recv_speed < 10240 && wl0_1_net_stat.trans_speed < 10240){
					idle_cnt += delta_time;
				}
				else{
					idle_cnt=0;
				}
			}
			else{
				//printf("not enough, cur_time: %d, last_time: %d, delta_time: %d\n", cur_time, last_time, delta_time);
			}

			printf("idle_cnt: %d\n", idle_cnt);
			if(idle_cnt >= 900){
				while(1){
					ret = set_system_poweroff();	
					if(ret < 0){
						printf("set system poweroff fail!\n\r");
					}
					else{
						printf("set system poweroff success!\n\r");
					}
					sleep(1);
				}
			}
			
			cur_dev_flow_times = 0;
		}			

		usleep(SLEEP_TIME_MIN);
	}
	//close(socket_id);
}

