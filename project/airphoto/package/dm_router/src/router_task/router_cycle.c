/*
 * =============================================================================
 *
 *       Filename:  router_inotify.c
 *
 *    Description:  monitor the router hardware parameter changed
 *
 *        Version:  1.0
 *        Created:  2015/08/20 14:33
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver, 
 *   Organization:  
 *
 * =============================================================================*/
#include <stdio.h>//printf  
#include <string.h> //strcmp  
#include <unistd.h>//close  

#include "router_cycle.h"
#include "msg_server.h"
extern int exit_flag;

int dm_change_power_inotify()
{
	FILE *p;
	char power_path[64];
	memset(power_path, 0, 64);
	sprintf(power_path, "%s/%s", INOTIFY_DIR_PATH, INOTIFY_POWER);
	DMCLOG_D("power_path = %s", power_path);
	if(access(power_path,F_OK)==0)
	{
		remove(power_path);/*  如果存在就删除*/
	}
	else
	{
		p=fopen(power_path,"w");/* 创建*/
		fclose(p);
	}
	return 0;
}

int dm_cycle_get_power(power_info_t *p_power_info)
{
	unsigned char be = 0;/* power percent */
	unsigned char stat = 0;/* 0:正常，1:充电，2:放电，3:低电*/
	unsigned char tmpStatus = 0;
	int fh = NULL;
	int count = 0;
	fh=open(rtl_encryp_control, O_RDWR);
	if(fh==NULL)
	{
		DMCLOG_D("open MCU proc error");
		return -1; 
	}
	usleep(100000);
	//get power percent
	while(exit_flag == 0)
	{
		usleep(100000);
		if(ioctl(fh, get_power_level_num, &be) < 0)
		{
			//DMCLOG_D("get power percent from MCU error");
			close(fh);
			return -1; 							
		}
		if(be == 0 || be == 255)
			continue ;
		else
			break;
		count++;
		if(count == 5)
			return -1;
	}
	usleep(100000);
	//get power status
	if(ioctl(fh,get_Firmware_Edition,&tmpStatus)<0)
	{
		DMCLOG_D("get power status from MUC error");
		close(fh);
		return -1;
	}
	stat = (tmpStatus&0x0f);
	if(stat==3)
	{
		stat = 1;
	}
	if(be<=10)
	{
		stat = 3;
	}

	p_power_info->power = be;
	p_power_info->power_status = stat;
	close(fh);
	return 0;
}

int dm_router_cycle(void)
{
	ENTER_FUNC();
	int ret = 0;
	power_info_t power_info_old;
	power_info_t power_info_now;
	int power_times = ROUTER_CYCLE_POWER_TIMES;
	memset(&power_info_old, 0, sizeof(power_info_t));
	while(exit_flag == 0)
	{
		CamD_check_hotplug();
		if(power_times >= ROUTER_CYCLE_POWER_TIMES)
		{
			memset(&power_info_now, 0, sizeof(power_info_t));
			ret = dm_cycle_get_power(&power_info_now);
			if(ret == 0)
			{
				if((power_info_old.power != power_info_now.power) || \
					(power_info_old.power_status != power_info_now.power_status))
				{
					//system("rm /tmp/notify/power || touch /tmp/notify/power");
					dm_change_power_inotify();
					power_info_old.power = power_info_now.power;
					power_info_old.power_status = power_info_now.power_status;
					DMCLOG_D("***********************have change***********************");
				}	
			}
			power_times = 0;
		}
		else
		{
			power_times++;
			//DMCLOG_D("power_times = %d", power_times);
		}
		usleep(ROUTER_CYCLE_TIME_MIN);
	}
	EXIT_FUNC();
}
