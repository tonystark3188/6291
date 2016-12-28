/*************************************************************/ 
/* Copyright (c) 2014, longsys */ 
/* All rights reserved. */ 
/*Project Name: NW2415*/ 
/*Author:		julien*/ 
/*Date:			2014-12-20*/ 
/*Version:		v1.0 */ 
/*Abstract£º    format hdisk*/ 
/*************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <mntent.h>
#include <sys/vfs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>  
#include <libudev.h>
#include <locale.h>
#include <fcntl.h> 
#include <errno.h>
#include <signal.h>
#include "mcu.h"
#include "router_defs.h"
#include "socket_uart.h"
#include "hd_route.h"

extern int	exit_flag;

int dm_mcu_get_power(power_info_t *p_power_info)
{
	int ret = 0;
	int run_time = 0;
	unsigned short power_percent = 0;
	unsigned short power_status = 0;
#if MCU_COMMUNICATE_AGREEMENT == MCU_DOUBLE_LINE
	int fh = 0;
	fh = open(rtl_encryp_control, O_RDWR);
	if(fh == -1){
		DMCLOG_E("open MCU proc error");
		return -1; 
	}
	usleep(100000);
	for(run_time = 0; (exit_flag == 0) && (run_time < 5); run_time++)
	{
		//usleep(100000);
		//get power percent
		if(ioctl(fh, get_power_level_num, &(unsigned char)power_percent) < 0){
			//DMCLOG_E("get power percent from MCU error");
			usleep(100000);	
			continue;
		}
		if(power_percent == 0 || power_percent == 255){
			continue;
			usleep(100000);
		}

		usleep(100000);
		//get power status
		if(ioctl(fh,get_Firmware_Edition,&(unsigned char)power_status)<0){
			//DMCLOG_E("get power status from MUC error");
			usleep(100000);	
			continue;
		}
		power_status = (power_status&0x0f);
		if(power_status == 3){
			power_status = 1;
		}
		if(power_percent <= 10){
			power_status = 3;
		}
		break;
	}
	close(fh);

#elif MCU_COMMUNICATE_AGREEMENT == MCU_UART
	socket_uart_cmd_t g_uart_cmd;
	for(run_time = 0; (exit_flag == 0) && (run_time < 5); run_time++){
		//get power percent
		memset(&g_uart_cmd, 0, sizeof(socket_uart_cmd_t));
		g_uart_cmd.mode = UART_R;
		g_uart_cmd.regaddr = SOCKET_UART_GET_CHARGE_STATUS;
		ret = SocketUartClientStart(&g_uart_cmd);
		if(ret < 0){
			usleep(100000);
			continue;
		}
		else{
			power_status = g_uart_cmd.data;
		}

		usleep(100000);
		//get power status
		memset(&g_uart_cmd, 0, sizeof(socket_uart_cmd_t));
		g_uart_cmd.mode = UART_R;
		g_uart_cmd.regaddr = SOCKET_UART_GET_POWER_PERCENT;
		ret = SocketUartClientStart(&g_uart_cmd);
		if(ret < 0){
			usleep(100000);
			continue;
		}
		else{
			power_percent = g_uart_cmd.data;
		}
		if((power_status != POWER_CHARGING) && (power_percent <= 10))
			power_status = POWER_LOW;	
		break;
	}
	
#endif
	if(run_time >= 5)
	{
		//DMCLOG_E("dm cycle get power fail");
		return -1;
	}

	p_power_info->power = (int)power_percent;
	p_power_info->power_status = (int)power_status;
	return 0;
}


static int dm_mcu_get_storage_dir(int *storage_dir)
{
	//DMCLOG_D("access dm_mcu_get_storage_dir");
	int ret = -1;
	int run_time = 0;
#if MCU_COMMUNICATE_AGREEMENT == MCU_UART
	socket_uart_cmd_t g_uart_cmd;
	if(NULL == storage_dir)
		return -1;
	
	memset(&g_uart_cmd, 0, sizeof(socket_uart_cmd_t));
	g_uart_cmd.mode = UART_R;
	#if defined(OPENWRT_MT7628)
	g_uart_cmd.regaddr = SOCKET_UART_GET_STORAGE_DIRECTION;
	#elif defined(OPENWRT_X1000)
	g_uart_cmd.regaddr = SOCKET_UART_GET_PC_DETEC;
	#endif
	ret = SocketUartClientStart(&g_uart_cmd);
	if(ret < 0)
	{
		DMCLOG_E("get storage direction error, error code = %d \r\n");
		//ret = -1;	
	}
	else
	{
		#if defined(OPENWRT_MT7628)
		*storage_dir = (int)g_uart_cmd.data;
		#elif defined(OPENWRT_X1000)
		*storage_dir = !(int)g_uart_cmd.data;
		#endif
		//DMCLOG_D("storage dir value = %d\n\r", *storage_dir);
		ret = 0;
	}
#endif
	return ret;
}



