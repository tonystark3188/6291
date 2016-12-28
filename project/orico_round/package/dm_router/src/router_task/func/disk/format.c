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
#include "format.h"
#include "router_defs.h"
#if 0
typedef struct my_udev{
	char dev_path[32];
	char pid[16];
	char vid[16];
}my_dev_t;

typedef struct all_my_udev{
	int count;
	my_dev_t m_my_dev[32];
}all_my_dev_t;
#endif
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

#if 0
/*******************************************************************************
 * Function:
 *    int dev_is_hd(char *dev)
 * Description:
 *    dev is hd?
 * Parameters:
 *    dev   [IN] device name, sample: /dev/sda1
 * Returns:
 *    0, is hd device
 *    other, no hd
 *******************************************************************************/
int dev_is_hd(char *dev)
{
	int fd, ret = -1;
	char fulldev[256];
	unsigned short id2[256];

	if(dev == NULL) return -1;

	memset(fulldev, 0, sizeof(fulldev));
	strcpy(fulldev, dev);
	fd = open(fulldev, O_RDONLY);
	if (fd < 0) 
	{
		printf("open %s failed.", fulldev);
		return -1;
	}

	if (!ioctl(fd, 0x030d, id2))
		ret = 0;

	close(fd);

	return ret;
}



/*******************************************************************************
 * Function:
 *    int dev_is_sdcard(char *sdev)
 * Description:
 *    dev is sdcard?
 * Parameters:
 *    dev   [IN] device name, sample: /dev/sda1
 * Returns:
 *    0, is sdcard device
 *    other, no sd
 *******************************************************************************/
int dev_is_sdcard(char *sdev)
{
#define DEFAULT_VID	"1307"
#define DEFAULT_PID "0168"
#define DEFAULT_PID1 "0310"
typedef struct{
	char devpath[32];
	// char * mountpoint;
	char pid[10];
	char vid[10];
}myudev;

	struct udev *udev;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;
	struct udev_device *dev;
	int ret = -1, i=0;
	myudev myusb[32], *p;

	if(sdev == NULL)
		
		return -1;

	p=myusb;

	/* Create the udev object */
	udev = udev_new();
	if (!udev) {
		printf("Can't create udev\n");   
		return ret;
	}

	enumerate = udev_enumerate_new(udev);
	udev_enumerate_add_match_subsystem(enumerate, "block");
	udev_enumerate_scan_devices(enumerate);
	devices = udev_enumerate_get_list_entry(enumerate);
	udev_list_entry_foreach(dev_list_entry, devices) 
	{
		const char *path;
		  
		path = udev_list_entry_get_name(dev_list_entry);
		dev = udev_device_new_from_syspath(udev, path);

		strcpy(p[i].devpath,udev_device_get_devnode(dev));
		// printf("%s\n", p[i].devpath);
		dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
		if (!dev) {
			break;
		}

		strcpy(p[i].vid,udev_device_get_sysattr_value(dev,"idVendor"));
		strcpy(p[i].pid,udev_device_get_sysattr_value(dev, "idProduct"));
		if(!strcmp(p[i].vid,DEFAULT_VID) 
			&&(!strcmp(p[i].pid,DEFAULT_PID) || !strcmp(p[i].pid,DEFAULT_PID1)) 
			&&!(strcmp(p[i].devpath, sdev)))
		{
			ret = 1;
		}
		
		//printf("myusb[%d]:dev=%s,vid=%s,pid=%s\n",i, p[i].devpath,p[i].vid,p[i].pid);
		i++;
		udev_device_unref(dev);
	}
	/* Free the enumerator object */
	udev_enumerate_unref(enumerate);
	udev_unref(udev);

	return ret;
}

int get_dev_type(char *sdev)
{
	if(sdev == NULL)
		return -1;

	if(dev_is_hd(sdev) == 0)
		return 1;
	else if(dev_is_sdcard(sdev) == 0)
		return 2;
	else
		return 3;

}

#endif

static unsigned long kscale(unsigned long b, unsigned long bs)
{
	return (b * (unsigned long long) bs + 1024/2) / 1024;
}

#if 0
#define HDISK_NAME "Hdisk"
/*******************************************************************************
 * Function:
 *   int get_all_my_dev_info(all_my_dev_t *m_all_dev);
 * Description:
 *    get dev pid vid path,
 * Parameters:
 *    dev   [IN] device, sample: sda1
 * Returns:
 *    0:success
 *    -1:failed
 *******************************************************************************/

int get_all_my_dev_info(all_my_dev_t *m_all_dev)
{
	struct udev *udev;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices,*dev_list_entry;
	struct udev_device *dev;
	int i = 0;
	udev = udev_new();
	if(!udev)
	{
		DMCLOG_D("Can't create udev");
		return -1;
	}
	enumerate = udev_enumerate_new(udev);
	udev_enumerate_add_match_subsystem(enumerate, "block");
	udev_enumerate_scan_devices(enumerate);
	devices = udev_enumerate_get_list_entry(enumerate);
	udev_list_entry_foreach(dev_list_entry, devices) {
		const char *path;
		path = udev_list_entry_get_name(dev_list_entry);
		dev = udev_device_new_from_syspath(udev, path);
		DMCLOG_D("path2 = %s",path);
		strcpy(m_all_dev->m_my_dev[i].dev_path,udev_device_get_devnode(dev));
		dev = udev_device_get_parent_with_subsystem_devtype(
		     dev,
		     "usb",
		     "usb_device");
		if(dev == NULL)
		{
			DMCLOG_D("dev is null,errno = %d",errno);
		}
		if (dev) {
			strcpy(m_all_dev->m_my_dev[i].vid,udev_device_get_sysattr_value(dev,"idVendor"));
			strcpy(m_all_dev->m_my_dev[i].pid,udev_device_get_sysattr_value(dev, "idProduct"));
			DMCLOG_D("myusb[%d]:dev=%s,vid=%s,pid=%s",i, m_all_dev->m_my_dev[i].dev_path,m_all_dev->m_my_dev[i].vid,m_all_dev->m_my_dev[i].pid);
			i++;
			m_all_dev->count++;
			udev_device_unref(dev);
		}
	}
	udev_enumerate_unref(enumerate);/* Free the enumerator object */
	udev_unref(udev);
	return 0;
}
#endif
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

int get_name_from_dev(char *dev,disk_info_t *disk_info)
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
				//DMCLOG_D("m_all_dev_count = %d",m_all_dev.count);
				/*for(i = 0;i < m_all_dev.count;i++)
				{
					DMCLOG_D("m_all_dev.m_my_dev[%d].dev_path = %s",i,m_all_dev.m_my_dev[i].dev_path);
					if(!strcmp(m_all_dev.m_my_dev[i].dev_path,mtpair->mnt_fsname))
					{
						strcpy(disk_info->pid,m_all_dev.m_my_dev[i].pid);
						strcpy(disk_info->vid,m_all_dev.m_my_dev[i].vid);
					}
				}*/
				ret = 0;
			}
		}
	}
	endmntent(mountTable);
	return ret;
}
#if 0
/*******************************************************************************
 * Function:
 *    char *get_hd_dev(char *driver)
 * Description:
 *    get hdisk device num ;sample /dev/sda1
 * Parameters:
 *    driver   [IN] hd path name, sample:hdisk1
 * Returns:
 *    NULL,   no found device
 *    string, success
 *    
*******************************************************************************/
char *get_hd_dev(char *driver)
{
	static char fulldev[256];
	char fullpath[256];
	struct mntent mtpair[2];
	char getmntent_buf[1024];
	FILE *mountTable;
	int num = 0;

	if(driver == NULL)
		return NULL;

	memset(fulldev, 0, sizeof(fulldev));
	memset(fullpath, 0, sizeof(fullpath));
	sprintf(fullpath, "/tmp/mnt/%s", driver);

	mountTable = setmntent("/proc/mounts", "r");
	if (!mountTable) return NULL;

	while (getmntent_r(mountTable, &mtpair[0], getmntent_buf, 1024))
	{
//		printf("%s on %s type %s (%s)\n", mtpair->mnt_fsname, mtpair->mnt_dir, mtpair->mnt_type, mtpair->mnt_opts);
		if(strcmp(fullpath, mtpair->mnt_dir) == 0)
		{
			strcpy(fulldev, mtpair->mnt_fsname);
			endmntent(mountTable);
			return fulldev;
		}
	}
	endmntent(mountTable);

	return NULL;
}

#endif

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

/*******************************************************************************
 * Function:
 *    dm_get_storage_dir(int* storage_dir)
 * Description:
 *    get storage direction,
 * Parameters:
 *    storage_dir  storage dircetion,
 * Returns:
 *    0, success
 *    other, failed.
 *******************************************************************************/
int dm_get_storage_dir(int* storage_dir)
{
	int ret = 0;
	char *usb_sw_type = get_sys_usb_sw_type();
	if(!strcmp(usb_sw_type, "otg"))
		ret = _dm_mcu_get_storage_dir(storage_dir);
	else
		ret = dm_mcu_get_storage_dir(storage_dir);
	DMCLOG_D("storage_dir = %d", *storage_dir);
	return ret;
}
/*******************************************************************************
 * Function:
 *    int Format_formatdisk(char *drivname, char *drivdev,int type)
 * Description:
 *    format hd
 * Parameters:
 *    drivname   [IN] hd path, sample: hdisk1
 *    drivdev    [IN] hd dev name, sample: sda1
 *    type       [IN] format type, ntfs - 1, ext4 - 2
 * Returns:
 *    0   success
 *    -1  no hd device
 *    -2  device isn't hd
 *    -3  other error
 *******************************************************************************/
int dm_format_disk(char *drivname, char *drivdev, int type)
{
#if 0
static char *format_ok_str = "mkntfs completed successfully. Have a nice day.";

	FILE * fp = NULL;
	int format_status = -1, hd_no_fdisk = 0; 
	char buf[256];
	char strcmd[256], *sdev = NULL, sdevice[64];
	memset(buf, 0, sizeof(buf));
	memset(sdevice, 0, sizeof(sdevice));
	memset(strcmd, 0, sizeof(strcmd));

	sdev = get_hd_dev(drivname);
	if(sdev == NULL)
	{
		if(drivdev == NULL)
			return -1;
		sprintf(sdevice, "/dev/%s", drivdev);
		sdev = sdevice;
	}

	if(dev_is_hd(sdev) != 0)
		return -2;

	if(strlen(sdev) == 8)
	{
		hd_no_fdisk = 1;
	}

	printf("format hd %s\n", drivname);
	//samba stop 
	system("/etc/init.d/samba stop");
	sync();

	if(hd_no_fdisk == 0)
	{
		//fuser 
		sprintf(strcmd, "fuser -km /tmp/mnt/%s", drivname);
		system(strcmd);

		//umount 
		sprintf(strcmd, "umount %s", sdev);
		system(strcmd);
	}
	else //hd_no_fdisk == 1
	{
		sprintf(strcmd, "fdisk %s < /etc/fmt_hdisk.cmd", sdev);
		system(strcmd);
		sleep(1);
	}

	//mkfs.ntfs
	if(hd_no_fdisk == 0)
		sprintf(strcmd, "mkfs.ntfs -f %s", sdev);
	else
		sprintf(strcmd, "mkfs.ntfs -f %s1", sdev);
	if ((fp = popen(strcmd, "r") ) == NULL) 
	{ 
		perror("popen");
		printf("popen error: %s/n", strerror(errno)); 
		return -3; 
	} 
	else
	{
		while(fgets(buf, sizeof(buf), fp)) 
		{
			if(strstr(buf, format_ok_str))
				format_status = 0;
//			printf("%s", buf);
		} 
		pclose(fp);
	}

	if(hd_no_fdisk == 0)
	{
		sprintf(strcmd, "mkdir -p /tmp/mnt/%s", drivname);
		system(strcmd);
		sprintf(strcmd, "ntfs-3g -o noatime,big_writes,nls=utf8 %s /tmp/mnt/%s", sdev, drivname);
		system(strcmd);
	}
	else
	{
		sprintf(strcmd, "mkdir -p /tmp/mnt/%s1", drivname);
		system(strcmd);
		sprintf(strcmd, "ntfs-3g -o noatime,big_writes,nls=utf8 %s1 /tmp/mnt/%s1", sdev, drivname);
		system(strcmd);
	}

	sprintf(sdevice, "%s1", HDISK_NAME);
	printf("sdevice = %s\n", sdevice);
	if(strcmp(drivname, sdevice) == 0)
	{
		sprintf(strcmd, "mkdir -p /tmp/mnt/%s/tmp", drivname);
		system(strcmd);
		printf(strcmd);
	}
	//samba start 
	system("/etc/init.d/samba start");
	return format_status;
	#endif
}
