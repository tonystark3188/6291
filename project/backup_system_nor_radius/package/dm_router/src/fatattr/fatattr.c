#ident "$Id$"
/* ----------------------------------------------------------------------- *
 *   
 *   Copyright 2005 H. Peter Anvin - All Rights Reserved
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 53 Temple Place Ste 330,
 *   Boston MA 02111-1307, USA; either version 2 of the License, or
 *   (at your option) any later version; incorporated herein by reference.
 *
 * ----------------------------------------------------------------------- */

/*
 * fatattr.c
 *
 * Display or change attributes on a FAT filesystem, similar to the
 * DOS attrib command.
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sysexits.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/msdos_fs.h>
#include <ctype.h>
#include <mntent.h>
#include <sys/vfs.h>
#include <dirent.h>
#include <sys/types.h>  
#include <libudev.h>
#include <locale.h>
#include <errno.h>
#include <signal.h>


#include "fatattr.h"
#include "router_defs.h"

#ifndef FAT_IOCTL_GET_ATTRIBUTES
# define FAT_IOCTL_GET_ATTRIBUTES        _IOR('r', 0x10, __u32)
#endif
#ifndef FAT_IOCTL_SET_ATTRIBUTES
# define FAT_IOCTL_SET_ATTRIBUTES        _IOW('r', 0x11, __u32)
#endif
/* Currently supports only the FAT flags, not the NTFS ones */
const char bit_to_char[] = "rhsvda67";

static int
chattr(const char *file, uint32_t s_attr, uint32_t c_attr)
{
  int fd = -1;
  uint32_t attr, nattr;
  int e;

  fd = open(file, O_RDONLY);
  if ( fd < 0 )
    goto err;

  if ( ioctl(fd, FAT_IOCTL_GET_ATTRIBUTES, &attr) )
    goto err;

  nattr = (attr & ~c_attr) | s_attr;

  if ( nattr != attr ) {
    if ( ioctl(fd, FAT_IOCTL_SET_ATTRIBUTES, &nattr) )
      goto err;
  }

  close(fd);
  return 0;

 err:
  e = errno;
  if ( fd >= 0 )
    close(fd);

  errno = e;
  return -1;
}

static int
lsattr(const char *file)
{
  int fd = -1;
  uint32_t attr;
  int i, e;

  fd = open(file, O_RDONLY);
  if ( fd < 0 )
    goto err;

  if ( ioctl(fd, FAT_IOCTL_GET_ATTRIBUTES, &attr) )
    goto err;

  close(fd);
  attr >>= 1;

  if(attr & 1)
  {
 		DMCLOG_D("the %s is hided",file);
		return 1;	
  }
  return 0;

 err:
  e = errno;
  if ( fd >= 0 )
    close(fd);
  errno = e;
  return -1;
}

static uint32_t
parse_attr(const char *attrs)
{
  uint32_t attr = 0;
  const char *p;

  while ( *attrs ) {
    p = strchr(bit_to_char, *attrs);
    if ( !p ) {
      fprintf(stderr, " unknown attribute: '%c'\n", *attrs);
      exit(EX_USAGE);
    }

    attr |= (uint32_t)1 << (p-bit_to_char);
    attrs++;
  }

  return attr;
}

int attrib(int argc, char **argv)
{
	uint32_t s_attr = 0;		/* Attributes to set */
	uint32_t c_attr = 0;		/* Attributes to clear */
	int i;
	int files = 0;		/* Files processed */
	int e;

	for ( i = 1 ; i < argc ; i++ )
	{
		if ( argv[i][0] == '-' )
			c_attr |= parse_attr(argv[i]+1);
		else if ( argv[i][0] == '+' )
			s_attr |= parse_attr(argv[i]+1);
		else {
			e = 0;
			if ( c_attr | s_attr )
				e = chattr(argv[i], s_attr, c_attr);
			else
				e = lsattr(argv[i]);

			if (e < 0 )
			{
				perror(argv[i]);
				EXIT_FUNC();
				return -1;
			}
			files++;
		}
	}
	return e;
}

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


static int dm_check_attr_support(char *path)
{
	int ret = 0;
	struct mntent mtpair[2];
	char getmntent_buf[1024];
	FILE *mountTable = NULL;
	struct statfs fs;

	mountTable = setmntent("/proc/mounts", "r");
	if (!mountTable){
		DMCLOG_E("setmntent error");
		return -1;
	}
	
	while (getmntent_r(mountTable, &mtpair[0], getmntent_buf, 1024)){
		//DMCLOG_D("path: %s, mtpair->mnt_dir: %s", path, mtpair->mnt_dir);
		if(!strncmp(mtpair->mnt_dir, MOUNT_PATH, strlen(MOUNT_PATH)) && !strncmp(path, mtpair->mnt_dir, strlen(mtpair->mnt_dir))){
			if (statfs(mtpair->mnt_dir, &fs) == 0){
				//DMCLOG_D("human_fstype(fs.f_type): %s", human_fstype(fs.f_type));
				if(!strcmp("msdos", human_fstype(fs.f_type))){
					ret = 0;
				}
				else{
					ret = 1;
				}
			}
			break;		
		}
	}
	endmntent(mountTable);
	return ret;
}


/*
* func:set file or dir to hide that cannot access in windows
*
* para: the full path
*
* return : 0 succ, -1 failed
*/
static int _dm_set_attr_hide(char *path,bool attr,bool album)
{
	int ret = 0;

	struct stat stp;
	stat(path,&stp);
	if(S_ISDIR(stp.st_mode))
	{
		DIR *dp;
        struct dirent *d;
		dp = opendir(path);
		if(dp == NULL)
			return -1;
		while((d = readdir(dp)) != NULL)
		{
			if(strcmp(d->d_name, ".") == 0
               || strcmp(d->d_name, "..") == 0)
            {
                continue;
            }
            
            char *fullname = concat_path_file(path, d->d_name);
            if(fullname == NULL)
            {
                continue;
            }
			if(album == true)
			{
				if(db_get_mime_type(fullname,strlen(fullname)) == 3)
				{
					if(attr == true)
					{
						char *attr_argv1[] = {"attrib","+r","+s","+h",fullname};
						ret = attrib(5,attr_argv1);
					}else{
						char *attr_argv2[] = {"attrib","-r","-s","-h",fullname};
						ret = attrib(5,attr_argv2);
					}
				}
			}else{
				if(attr == true)
				{
					char *attr_argv1[] = {"attrib","+r","+s","+h",fullname};
					ret = attrib(5,attr_argv1);
				}else{
					char *attr_argv2[] = {"attrib","-r","-s","-h",fullname};
					ret = attrib(5,attr_argv2);
				}
			}
			
			safe_free(fullname);
		}
		closedir(dp);
	}

	if(album == false)
	{
		if(attr == true)
		{
			char *attr_argv3[] = {"attrib","+r","+s","+h",path};
			ret = attrib(5,attr_argv3);
		}else{
			char *attr_argv4[] = {"attrib","-r","-s","-h",path};
			ret = attrib(5,attr_argv4);
		}
	}
	return ret;
}

/*
* func:check hide_setting support and set file or dir to hide that cannot access in windows
*
* para: the full path
*
* return : 0 succ, -1 failed, 1 not support
*/
int dm_set_attr_hide(char *path,bool attr,bool album)
{
	int ret = 0;
	if(path == NULL||access(path,F_OK) != 0){
		DMCLOG_E("path is null or is not exist");
		return -1;
	}

	ret = dm_check_attr_support(path);
	if(ret == 1){
		DMCLOG_D("file system nonsupport");
		return 1;
	}
	else if(ret == -1){
		return -1;
	}

	return _dm_set_attr_hide(path, attr, album);
}


/*
* func:get file or dir hide attr  in windows
*
* para: the full path
*
* return : true: hided,false:not hide
*/
bool dm_get_attr_hide(char *path)
{
	char *attr_argv[] = {"attrib",path};
	if(attrib(2,attr_argv) == 1)
		return true;
	return false;
}


