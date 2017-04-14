#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <mntent.h>
#include <sys/vfs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>     /*Unix 标准函数定义*/
#include <sys/types.h>  
#include <libudev.h>
#include <locale.h>
#include "uci_for_cgi.h"


#define TRUE 0
#define FALSE -1

FILE* fp_error;
FILE* fp_info;

#define ERROR_FILE		"/tmp/gps_error"
#define INFO_FILE		"/tmp/gps_info"

static const unsigned long long G = 1024*1024*1024ull;
static const unsigned long long M = 1024*1024;
static const unsigned long long K = 1024;
static char str[20];


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
		{ 0, "UNKNOWN" }
	};

	int i;

	for (i = 0; humantypes[i].type; ++i)
		if (humantypes[i].type == f_type)
			break;
	return humantypes[i].fs;
}

char* kscale(unsigned long b, unsigned long bs)
{
	unsigned long long size = b * (unsigned long long)bs;
	if (size > G)
	{
		sprintf(str, "%0.1fGB", size/(G*1.0));
		return str;
	}
	else if (size > M)
	{
		sprintf(str, "%0.1fMB", size/(1.0*M));
		return str;
	}
	else if (size > K)
	{
		sprintf(str, "%0.1fK", size/(1.0*K));
		return str;
	}
	else
	{
		sprintf(str, "%0.1fB", size*1.0);
		return str;
	}
}



int main(int argc, char const *argv[])
{

	FILE* mount_table;
	struct mntent *mount_entry;
	struct statfs fs;
	unsigned long blocks_used;
	unsigned blocks_percent_used;
	char tmpi=0;
	char tmpstr[128]="\0";
	struct udev *udev;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;
	struct udev_device *dev;
	int i=0;
	int j=0;
	typedef struct myudevstruct{
	char devpath[20];
	// char * mountpoint;
	char pid[10];
	char vid[10];
	}myudev;
	myudev myusb[20];
	myudev *p=myusb;

	/* Create the udev object */
	udev = udev_new();
	if (!udev) {
		printf("Can't create udev\n");       
		exit(1);
	}

	enumerate = udev_enumerate_new(udev);
	udev_enumerate_add_match_subsystem(enumerate, "block");
	udev_enumerate_scan_devices(enumerate);
	devices = udev_enumerate_get_list_entry(enumerate);
	udev_list_entry_foreach(dev_list_entry, devices) {
		const char *path;
		  
		path = udev_list_entry_get_name(dev_list_entry);
		dev = udev_device_new_from_syspath(udev, path);

		strcpy(p[i].devpath,udev_device_get_devnode(dev));
		// printf("%s\n", p[i].devpath);
		dev = udev_device_get_parent_with_subsystem_devtype(
		dev,
		"usb",
		"usb_device");
		if (!dev) {
			break;
			// printf("Unable to find parent usb device.");
			// exit(1);
		}
		// p=myusb malloc(sizeof(myusb));
		// myusb[i].pid=malloc(8*sizeof(char));
		strcpy(p[i].vid,udev_device_get_sysattr_value(dev,"idVendor"));
		strcpy(p[i].pid,udev_device_get_sysattr_value(dev, "idProduct"));
		if(!strcmp(p[i].vid,"1307"))
		{
			if(strlen(p[i].devpath)>8)
				printf("%s\n", p[i].devpath+5);
		}
		
		//printf("myusb[%d]:dev=%s,vid=%s,pid=%s\n",i, p[i].devpath,p[i].vid,p[i].pid);
		i++;
		udev_device_unref(dev);
	}
	/* Free the enumerator object */
	udev_enumerate_unref(enumerate);

	udev_unref(udev);


	
		
	return 0;
}
