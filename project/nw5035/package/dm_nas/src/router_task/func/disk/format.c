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
#include "router_defs.h"
#include "format.h"
static const char *human_fstype(long f_type)
{
	static const struct types {
		long type;
		const char *const fs;
	} humantypes[] = {
		{ 0xADFF,     "affs" },
		{ 0x1Cd1,     "devpts" },
		{ 0x137D,     "ext" },
		{ 0xEF51,     "ext2" },
		{ 0xEF53,     "ext2/ext3" },
		{ 0x3153464a, "jfs" },
		{ 0x58465342, "xfs" },
		{ 0xF995E849, "hpfs" },
		{ 0x9660,     "isofs" },
		{ 0x4000,     "isofs" },
		{ 0x4004,     "isofs" },
		{ 0x137F,     "minix" },
		{ 0x138F,     "minix (30 char.)" },
		{ 0x2468,     "minix v2" },
		{ 0x2478,     "minix v2 (30 char.)" },
		{ 0x4d44,     "msdos" },
		{ 0x4006,     "fat" },
		{ 0x564c,     "novell" },
		{ 0x6969,     "nfs" },
		{ 0x9fa0,     "proc" },
		{ 0x517B,     "smb" },
		{ 0x012FF7B4, "xenix" },
		{ 0x012FF7B5, "sysv4" },
		{ 0x012FF7B6, "sysv2" },
		{ 0x012FF7B7, "coh" },
		{ 0x00011954, "ufs" },
		{ 0x012FD16D, "xia" },
		{ 0x5346544e, "ntfs" },
		{ 0x1021994,  "tmpfs" },
		{ 0x52654973, "reiserfs" },
		{ 0x28cd3d45, "cramfs" },
		{ 0x7275,     "romfs" },
		{ 0x858458f6, "romfs" },
		{ 0x73717368, "squashfs" },
		{ 0x62656572, "sysfs" },
		{ 0x2011bab0, "exfat" },
		{ 0, "UNKNOWN" }
	};

	int i;
	DMCLOG_D("f_type: %d", f_type);
	for (i = 0; humantypes[i].type; ++i){
		if (humantypes[i].type == f_type)
			break;
	}
	return humantypes[i].fs;
}

static unsigned long kscale(unsigned long b, unsigned long bs)
{
	return (b * (unsigned long long) bs + 1024/2) / 1024;
}

/*******************************************************************************
 * Function:
 *    char *get_name_from_dev(char *dev)
 * Description:
 *    get dev name,
 * Parameters:
 *    dev   [IN] device, sample: sda1
 * Returns:
 *    NULL, not found match
 *    device name string
 *******************************************************************************/

int get_name_from_dev(char *dev,_disk_info_t *disk_info)
{
	char fulldev[64] = {0};
	struct mntent mtpair[2];
	char getmntent_buf[1024];
	FILE *mountTable = NULL;
	struct statfs fs;
	//all_my_dev_t m_all_dev;
	int dev_ret = -1;
	int ret = -1;
	int i = 0;
	if(dev == NULL)	
	{
		DMCLOG_E("para is null");
		return ret;
	}
	sprintf(fulldev, "/dev/%s", dev);
	mountTable = setmntent("/proc/mounts", "r");
	if (!mountTable)
	{
		DMCLOG_E("setmntent error");
		return ret;
	}
	//memset(&m_all_dev,0,sizeof(all_my_dev_t));
	//dev_ret = get_all_my_dev_info(&m_all_dev);
	while (getmntent_r(mountTable, &mtpair[0], getmntent_buf, 1024))
	{
		//DMCLOG_D("%s on %s type %s (%s)", mtpair->mnt_fsname, mtpair->mnt_dir, mtpair->mnt_type, mtpair->mnt_opts);
		if(strncmp(fulldev, mtpair->mnt_fsname, strlen(mtpair->mnt_fsname)) == 0)
		{ 
			DMCLOG_D("mtpair->mnt_dir=%s, MOUNT_PATH=%s", mtpair->mnt_dir, MOUNT_PATH);
			if(0 == memcmp(mtpair->mnt_dir, MOUNT_PATH, strlen(MOUNT_PATH)))
			{
				strcpy(disk_info->spath, mtpair->mnt_dir);
				if(mtpair->mnt_opts != NULL)
				{
					if(strstr(mtpair->mnt_opts,"rw"))
					{
						disk_info->rw_flag = 1;
					}else{
						disk_info->rw_flag = 0;
					}
				}
				if (disk_info->spath != NULL&&statfs(disk_info->spath, &fs) == 0)
				{
					disk_info->total_size = kscale(fs.f_blocks, fs.f_bsize);
					disk_info->free_size = kscale(fs.f_bavail, fs.f_bsize);
					strcpy(disk_info->fstype,human_fstype(fs.f_type));
				}
				ret = 0;
			}
		}
	}
	endmntent(mountTable);
	return ret;
}

/*******************************************************************************
 * Function:
 *    Format_get_storage (all_disk_t *alldisk)
 * Description:
 *    get storage info list,
 * Parameters:
 *    alldisk   [IN/OUT] disk info struct,
 * Returns:
 *    0, success
 *    other, failed.
 *******************************************************************************/
int dm_get_storage (all_disk_t *alldisk)
{
	DIR * dir;
	struct dirent *ptr = NULL;
	struct statfs fs;
	char fulldev[64], *spath = NULL;
	int ret = -1;

	fulldev[0] = 0;
	alldisk->count = 0;
	dir =opendir("/dev");
	while((ptr = readdir(dir))!=NULL)
	{
		if(((ptr->d_name[0]=='s')&&(ptr->d_name[1]=='d') && strlen(ptr->d_name) >= 3) || \
			((ptr->d_name[0]=='m')&&(ptr->d_name[1]=='m')&&(ptr->d_name[2]=='c')&&(ptr->d_name[3]=='b') && strlen(ptr->d_name) >= 4))
		{
			ret = get_name_from_dev(ptr->d_name,&alldisk->disk[alldisk->count]);
			if(ret < 0)
			{
				//DMCLOG_D("get %s storage info error",ptr->d_name);
				continue;
			}
			if(alldisk->disk[alldisk->count].spath)
			{
				//DMCLOG_D("alldisk->disk[alldisk->count].spath: %s", alldisk->disk[alldisk->count].spath);
				strcpy(alldisk->disk[alldisk->count].path, alldisk->disk[alldisk->count].spath);
				strcpy(alldisk->disk[alldisk->count].name, alldisk->disk[alldisk->count].spath+9);///tmp/mnt/
				strcpy(alldisk->disk[alldisk->count].dev, ptr->d_name);
				alldisk->disk[alldisk->count].is_format = 1;
				sprintf(fulldev, "/dev/%s", ptr->d_name);
				alldisk->count++;
			}
		}
	}
	closedir(dir);
	return 0;
}
