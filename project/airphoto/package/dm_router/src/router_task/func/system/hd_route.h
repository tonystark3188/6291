/*
 * =============================================================================
 *
 *       Filename:  hd_route.h
 *
 *    Description:  route interface
 *
 *        Version:  1.0
 *        Created:  2015/04/03 10:51
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver, 
 *   Organization:  
 *
 * =============================================================================*/


#ifndef _HD_ROUTE_H_
#define _HD_ROUTE_H_

#include <unistd.h>
#include <stdio.h>
#include "base.h"

#ifdef __cplusplus
extern "C"{
#endif

#define FW_1 "1.0.00"
#define FW_2 "1.0.00.1"

#define get_power_level_num  1
#define get_Firmware_Edition 5
#define rtl_encryp_control "/proc/rtl_encryp_control"

#define MNT_PATH "/tmp/mnt"

typedef struct power_info{
	int power_status;		/* 0:normal，1:charging，2:discharging，3:low power*/
	int power; 				/* percent of power */
}power_info_t;

typedef struct time_info{
	char time_value[64];   	/* data and time eg. 2013-03-14 20:00:00*/
	char time_zone[64];		/* time zone eg.  GMT+8:00*/
}dm_time_info;

typedef struct ota_info{
	char customCode[32];    /* project code eg. A85(M.03) */
	char versionCode[32];	/* version code eg.  1.0.01 */
	char mac[32];			/* mac */
	int version_flag;		/* 0: release version;1:test version*/
	int time;				/*upgrade time*/
}dm_ota_info;

/*******************************************************************************
 * Function:
 * int dm_get_power(power_info_t *m_power_info);
 * Description:
 * get power info 0x0208
 * Parameters:
 *    m_power_info [OUT] power info
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_get_power(power_info_t *m_power_info);
/*******************************************************************************
 * Function:
 * int dm_upgrade_fw();
 * Description:
 * upgrade from U drive 0x021B
 * Parameters:
 *    NULL
 * Returns:
 *    0:设备根目录无升级文件,
 *    1:升级文件CRC校验失败
 *    2:升级文件正常，开始升级
 *    -1:failed
 *******************************************************************************/
int dm_upgrade_fw();
/*******************************************************************************
 * Function:
 * int dm_sync_time();
 * Description:
 * sync time 0x021D
 * Parameters:
 *    NULL
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_sync_time(dm_time_info *time_info_t);
int dm_get_ota_info(dm_ota_info *p_ota_info);

int dm_get_version_flag(int *version_flag);

int dm_set_version_flag(int version_flag);

int dm_get_fw_version(char *fw_version);
#ifdef __cplusplus
}
#endif

#endif


