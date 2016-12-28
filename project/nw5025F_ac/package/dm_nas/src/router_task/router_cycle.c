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
#include "mcu.h"
#include "router_defs.h"
#include "defs.h"

inotify_power_info_t power_info_global;
#ifdef DM_CYCLE_DISK
int disk_status;//0:on cp;1:on board
#endif
extern int exit_flag;

int dm_cycle_change_inotify(int status)
{
	FILE *p;
	char inotify_path[64];
	memset(inotify_path, 0, 64);
	if(status == power_changed)
		sprintf(inotify_path, "%s/%s", INOTIFY_DIR_PATH, INOTIFY_POWER);
	else if(status == disk_changed)
		sprintf(inotify_path, "%s/%s", INOTIFY_DIR_PATH, INOTIFY_DISK);
	else if(status == data_base_changed)
		sprintf(inotify_path, "%s/%s", INOTIFY_DIR_PATH, INOTIFY_DATA_BASE);
	else if(status == pwd_changed)
		sprintf(inotify_path, "%s/%s", INOTIFY_DIR_PATH, INOTIFY_PWD);
	else
		return -1;
	DMCLOG_D("inotify_path = %s", inotify_path);
	if(access(inotify_path,F_OK)==0)
	{
		remove(inotify_path);/*  如果存在就删除*/
	}
	else
	{
		p=fopen(inotify_path,"w");/* 创建*/
		fclose(p);
	}
	return 0;
}

int dm_router_cycle(void)
{
	ENTER_FUNC();
	int ret = 0;
	power_info_t power_info_now;
	int power_times = ROUTER_CYCLE_POWER_TIMES;
	#ifdef DM_CYCLE_DISK
	int disk_times = ROUTER_CYCLE_DISK_TIMES;
	int disk_status_now;
	disk_status = -1;
	#endif
	memset(&power_info_global, 0, sizeof(inotify_power_info_t));
	while(exit_flag == 0){
		if(power_times >= ROUTER_CYCLE_POWER_TIMES){
			memset(&power_info_now, 0, sizeof(power_info_t));
			ret = dm_mcu_get_power(&power_info_now);
			if(ret == 0)
			{
				EnterCriticalSection(&power_info_global.mutex);
				if((power_info_global.power_info.power != power_info_now.power) || \
					(power_info_global.power_info.power_status != power_info_now.power_status))
				{
					//system("rm /tmp/notify/power || touch /tmp/notify/power");
					power_info_global.power_info.power = power_info_now.power;
					power_info_global.power_info.power_status = power_info_now.power_status;
					DMCLOG_D("new power:%d,power status:%d", power_info_now.power, power_info_now.power_status);
					dm_cycle_change_inotify(power_changed);
					DMCLOG_D("***********************power have change***********************");
				}
				LeaveCriticalSection(&power_info_global.mutex);
			}
			power_times = 0;
		}
		#ifdef DM_CYCLE_DISK
		else if(disk_times >= ROUTER_CYCLE_DISK_TIMES){
			ret = _dm_mcu_get_storage_dir(&disk_status_now);
			if(ret == 0){
				if(disk_status_now != disk_status){
					disk_status = disk_status_now;	
					DMCLOG_D("new disk_status:%d", disk_status);
					dm_cycle_change_inotify(disk_changed);					
					DMCLOG_D("***********************disk have change***********************");
				}
			}
			disk_times = 0;
		}
		#endif
		else{
			power_times++;
			#ifdef DM_CYCLE_DISK
			disk_times++;
			#endif
			//DMCLOG_D("power_times = %d", power_times);
		}
		usleep(ROUTER_CYCLE_TIME_MIN);
	}
	EXIT_FUNC();
	return 0;
}
