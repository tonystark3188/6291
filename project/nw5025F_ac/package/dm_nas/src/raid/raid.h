/*
 * =============================================================================
 *
 *       Filename: raid.h
 *
 *    Description:  raid manage.
 *
 *        Version:  1.0
 *        Created:  2016/12/19 17:11
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 515296288jf@163.com
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _RAID_H_
#define _RAID_H_
#include "bash.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef enum{
	GET_CUR_RAID_MODE_INFO = 0,//check if or not in raid,if it is not in raid,set single mode or double mode ,if it is in raid mode ,return raid info,include total size ,free size and mode
	SET_SINGLE_MODE_TO_RAID,//set single mode for raid if it is not in raid mode
	SET_DOUBLE_MODE_TO_RAID,//set double mode for raid if it is not in raid mode
	REPLACE_DISK_FOR_SINGLE,//restore single mode and replace new disk when it is in single mode
	ADD_ONE_DISK_TO_DOUBLE_MODE_FOR_SINGLE,//add new disk to double mode when it is in single mode
	ADD_DOUBLE_DISK_TO_DOUBLE_MODE_FOR_SINGLE,//delete old disk and add double disk to double mode when it is in single mode
	SWITCH_DOUBLE_TO_SINGLE_MODE_FOR_DOUBLE,//switch double mode to single mode when it is in double mode
	REPLACE_NEW_DISK_FOR_DOUBLE,//replace new disk for double mode when it is in double mode
	REPLACE_DOUBLE_DISK_FOR_DOUBLE,//delete two old disk and replace two new disk when it is in double mode
}raidType;

void (*onProgressChange)(int progress, int total,void *con);

typedef struct raid_info{
	int current_mode;//current mode: 1 single mode,2 double mode
	int disk_loc;//disk location :1 disk1,2 disk2 
	long long total_size;
	long long free_size;
}p_raid_info;
/*******************************************************************************
 * Description:
 *    get raid info;
 * Parameters:
 * 	current_mode: [OUT] current mode;
 *	disk_loc:[OUT] disk location;
 *    disk_info:[OUT] total_size,free_size;
 * Returns:
 *   <0:error,0:succ
 *******************************************************************************/
int raid_get_disk_info(_Out_ p_raid_info *raid_info);

/*******************************************************************************
 * Description:
 *    检测指定盘位磁盘是否正确安装;
 * Parameters:
 *	disk_loc:[IN] disk location;
 * Returns:
 *   <0:error,0:succ
 *******************************************************************************/
int raid_check_disk_status(_In_ int disk_loc,raidType type);

/*******************************************************************************
 * Description:
 *    init new disk,include raid create and format;
 * Parameters:
 *	disk_loc:[IN] disk location;
 *    onProgressChange:[OUT] progress,total;
 * Returns:
 *   <0:error,0:succ
 *******************************************************************************/
int raid_new_disk_init(_In_ int disk_loc,raidType type,void *conn,_Out_ void (*onProgressChange)(int progress, int total,void *conn));


/*******************************************************************************
 * Description:
 *    disk data rebuild;
 * Parameters:
 *	disk_loc:[IN] disk location;
 *    onProgressChange:[OUT] progress,total;
 * Returns:
 *   <0:error,0:succ
 *******************************************************************************/
int raid_disk_rebuild(_In_ int disk_loc,raidType type,void *conn,_Out_ void (*onProgressChange)(int progress, int total,void *conn));


#ifdef __cplusplus
}
#endif

#endif


