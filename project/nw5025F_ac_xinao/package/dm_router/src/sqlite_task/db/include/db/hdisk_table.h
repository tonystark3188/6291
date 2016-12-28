/*
 * =============================================================================
 *
 *       Filename: hdisk_table.h
 *
 *    Description: hdisk table related data structure definition.
 *
 *        Version:  1.0
 *        Created:  2014/09/17 
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Tao Sheng (), vanbreaker90@gmail.com
 *   Organization:  
 *
 * =============================================================================
 */


#ifndef HDISK_TABLE_H
#define HDISK_TABLE_H
#include "hidisk_errno.h"

#define INVALID_HDISK_ID 0xFFFFFFFF


typedef struct HdiskInfoTable
{
	int id;
	char uuid[16];					/* uuid */
	char pid[16];					/* pid */
	char vid[16];					/* vid */
	uint64_t total_capacity;
	uint64_t free_capacity;
	uint64_t recycle_size;
	uint64_t video_size;
	uint64_t audio_size;
	uint64_t photo_size;
	int mount_status;
}hdisk_info_t;

//INSERT INTO hdisk_table(UUID,PID,VID,FREE_SIZE,TOTAL_SIZE,ISMOUNTED) 
error_t hdisk_table_init(void);
void register_hdisk_table_ops(void);


#endif
