/*
 * =============================================================================
 *
 *       Filename:  raid.c
 *
 *    Description:  raid basic operation
 *
 *        Version:  1.0
 *        Created:  2016/12/19
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver
 *   Organization:  
 *
 * =============================================================================
 */
 #include "raid.h"

/*******************************************************************************
* Description:
*    get raid info;
* Parameters:
*  current_mode: [OUT] current mode;
*  disk_loc:[OUT] disk location;
*    disk_info:[OUT] total_size,free_size;
* Returns:
*   <0:error,0:succ
*******************************************************************************/
int raid_get_disk_info(_Out_ p_raid_info *raid_info)
{
	//TODO
	return 0;
}

/*******************************************************************************
* Description:
*    检测指定盘位磁盘是否正确安装;
* Parameters:
*  disk_loc:[IN] disk location;
* Returns:
*   <0:error,0:succ
*******************************************************************************/
int raid_check_disk_status(_In_ int disk_loc,raidType type)
{
	//TODO
	return 0;
}

/*******************************************************************************
* Description:
*    init new disk,include raid create and format;
* Parameters:
*  disk_loc:[IN] disk location;
*    onProgressChange:[OUT] progress,total;
* Returns:
*   <0:error,0:succ
*******************************************************************************/
int raid_new_disk_init(_In_ int disk_loc,raidType type,void *conn,_Out_ void (*onProgressChange)(int progress, int total,void *conn))
{
	//TODO
	return 0;
}


/*******************************************************************************
* Description:
*    disk data rebuild;
* Parameters:
*  disk_loc:[IN] disk location;
*    onProgressChange:[OUT] progress,total;
* Returns:
*   <0:error,0:succ
*******************************************************************************/
int raid_disk_rebuild(_In_ int disk_loc,raidType type,void *conn,_Out_ void (*onProgressChange)(int progress, int total,void *conn))
{
	//TODO
	return 0;
}




