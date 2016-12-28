/*
 * =============================================================================
 *
 *       Filename:  file_opr.h
 *
 *    Description:  file basic operation
 *
 *        Version:  1.0
 *        Created:  2015/3/19 11:29
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _FILE_BASE_OPR_H_
#define _FILE_BASE_OPR_H_

#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "base.h"
#include "cloud_errno.h"
#include "util.h"
#include "my_debug.h"



#ifdef __cplusplus
extern "C"{
#endif

enum {	/* DO NOT CHANGE THESE VALUES!  cp.c, mv.c, install.c depend on them. */
	FILEUTILS_PRESERVE_STATUS = 1 << 0, /* -p */
	FILEUTILS_DEREFERENCE     = 1 << 1, /* !-d */
	FILEUTILS_RECUR           = 1 << 2, /* -R */
	FILEUTILS_FORCE           = 1 << 3, /* -f */
	FILEUTILS_INTERACTIVE     = 1 << 4, /* -i */
	FILEUTILS_MAKE_HARDLINK   = 1 << 5, /* -l */
	FILEUTILS_MAKE_SOFTLINK   = 1 << 6, /* -s */
	FILEUTILS_DEREF_SOFTLINK  = 1 << 7, /* -L */
	FILEUTILS_DEREFERENCE_L0  = 1 << 8, /* -H */
};


enum {
TERMINAL_WIDTH  = 80,           /* use 79 if terminal has linefold bug */

SPLIT_FILE      = 0,
SPLIT_DIR       = 1,
SPLIT_SUBDIR    = 2,

/* Bits in G.all_fmt: */

/* 51306 lrwxrwxrwx  1 root     root         2 May 11 01:43 /bin/view -> vi* */
/* what file information will be listed */
LIST_INO        = 1 << 0,
LIST_BLOCKS     = 1 << 1,
LIST_MODEBITS   = 1 << 2,
LIST_NLINKS     = 1 << 3,
LIST_ID_NAME    = 1 << 4,
LIST_ID_NUMERIC = 1 << 5,
LIST_CONTEXT    = 1 << 6,
LIST_SIZE       = 1 << 7,
LIST_DATE_TIME  = 1 << 8,
LIST_FULLTIME   = 1 << 9,
LIST_SYMLINK    = 1 << 10,
LIST_FILETYPE   = 1 << 11, /* show / suffix for dirs */
LIST_CLASSIFY   = 1 << 12, /* requires LIST_FILETYPE, also show *,|,@,= suffixes */
LIST_MASK       = (LIST_CLASSIFY << 1) - 1,

/* what files will be displayed */
DISP_DIRNAME    = 1 << 13,      /* 2 or more items? label directories */
DISP_HIDDEN     = 1 << 14,      /* show filenames starting with . */
DISP_DOT        = 1 << 15,      /* show . and .. */
DISP_NOLIST     = 1 << 16,      /* show directory as itself, not contents */
DISP_RECURSIVE  = 1 << 17,      /* show directory and everything below it */
DISP_ROWS       = 1 << 18,      /* print across rows */
DISP_MASK       = ((DISP_ROWS << 1) - 1) & ~(DISP_DIRNAME - 1),

/* what is the overall style of the listing */
STYLE_COLUMNAR  = 1 << 19,      /* many records per line */
STYLE_LONG      = 2 << 19,      /* one record per line, extended info */
STYLE_SINGLE    = 3 << 19,      /* one record per line */


/* how will the files be sorted (CONFIG_FEATURE_LS_SORTFILES) */
SORT_REVERSE    = 1 << 23,

SORT_NAME       = 0,            /* sort by file name */
SORT_SIZE       = 1 << 24,      /* sort by file size */
SORT_ATIME      = 2 << 24,      /* sort by last access time */
SORT_CTIME      = 3 << 24,      /* sort by last change time */
SORT_MTIME      = 4 << 24,      /* sort by last modification time */
SORT_VERSION    = 5 << 24,      /* sort by version */
SORT_EXT        = 6 << 24,      /* sort by file name extension */
SORT_DIR        = 7 << 24,      /* sort by file or directory */

LIST_LONG       = LIST_MODEBITS | LIST_NLINKS | LIST_ID_NAME | LIST_SIZE | \
                  LIST_DATE_TIME | LIST_SYMLINK,
};



#define LONE_CHAR(s,c)     ((s)[0] == (c) && !(s)[1])



struct file_dnode
{
	off_t 			size;//文件大小
	unsigned long 	date;//创建时间
	char 			isFolder;//是否是目录文件
	char* 			name;	//文件名
	char* 			fullname;
	mode_t    		dn_mode;
	unsigned 		index;
	bool			attr;
	int 			fname_allocated;
	struct file_dnode *dn_next;
};
#define PAGESIZE 48
#define mode_t unsigned short

char* bb_basename(char *name);
ssize_t writen(int fd, char *buf, size_t count);
ssize_t readn(int fd, char *buf, size_t count);
int set_fd_nonblock(int fd);
int is_file_exist_ext(const char *file_path);
int cp_file(const char *src_file, const char *dest_file);

#define FILE_DEF_MODE (S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH)
#define DEFAULT_PAGE_SIZE (4*1024)

#ifdef __cplusplus
}
#endif

#endif

