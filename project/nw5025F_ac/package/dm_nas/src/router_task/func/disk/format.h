
/* $Id: json_util.h,v 1.4 2006/01/30 23:07:57 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#ifndef _format_h_
#define _format_h_
#include "base.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct disk_info {
	char name[32];					/* disk mount name eg.USB-disk-1 */
	char path[256];					/* disk mount path */
	char dev[32];					/* device node */
	unsigned long long total_size; 		/* total size 1KB */
	unsigned long long free_size;  		/* free size 1KB */
	int is_format;					/* whether format */
	int type; 						/* 1:HD; 2:Sdcard; 3:UDisk; 4:private disk;-1:unknown */
	int rw_flag;					/* 1:read and write;0:only read */
	char fstype[16];				/* file system type */	
	char pid[16];					/* pid */
	char vid[16];					/* vid */
	char uuid[16];					/* uuid */
	int auth;
	char spath[32];
	int is_scanning;
	int mount_status;
}_disk_info_t;

typedef struct all_disk{
	int count;
	int storage_dir;
	_disk_info_t disk[16];
}all_disk_t;
/* utility functions */
/*******************************************************************************
 * Function:
 * int dm_get_storage (all_disk_t *mAll_disk_t);
 * Description:
 * get storage infomation 0x0300
 * Parameters:
 *    mAll_disk_t [struct OUT] storage infomation
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_get_storage (all_disk_t *mAll_disk_t);


#ifdef __cplusplus
}
#endif

#endif