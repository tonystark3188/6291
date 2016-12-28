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
#include "fatattr.h"
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



/*
* func:set file or dir to hide that cannot access in windows
*
* para: the full path
*
* return : 0 succ, -1 failed
*/
int dm_set_attr_hide(char *path,bool attr,bool album)
{
	int ret = 0;

	if(path == NULL||access(path,F_OK) != 0)
	{
		DMCLOG_E("path is null or is not exist");
		return -1;
	}

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


