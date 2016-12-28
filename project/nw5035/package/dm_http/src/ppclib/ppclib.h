/*
 * =============================================================================
 *
 *       Filename:  ppclib.h
 *
 *    Description:  file cache interface
 *
 *        Version:  1.0
 *        Created:  2016/10/14 14:28
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 515296288jf@163.com 18503096207
 *   Organization: longsys 
 *
 * =============================================================================
 */

#ifndef _PPCLIB_H_
#define _PPCLIB_H_

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <resolv.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <dirent.h>
#include <stdint.h>
#include "base.h"
#ifdef __cplusplus
extern "C"{
#endif

typedef enum ERROR_CODE_PPC
{
	/**
	 * ≥…π?
	 */
      SUCCESS=0,
      /**
       * ??∞?
       */
      FAIL,

      /**
       * √a”–????(√a”–μ??o)
       */
      NOT_AUTHORIZATION = 10000,
      /**
       * ???ˉ∑√??(”√a?√a”–????∑√??)
       */
      PERMISSION_DENIED = 10001,
      /**
       * ”√a?√?≤a￥ê‘?
       */
      USERNAME_NOT_FOUND= 10002,
      /**
       * √???￥ì??
       */
      PASSWORD_ERROR	= 10003
} ERROR_CODE_PPC;


typedef struct _PFILE{
	char	mode[8];
	int 	fd;
	const char 	*srcPath;
	size_t 	offset;
	size_t 	length;
    char    *token;
}PFILE;
    
struct ppc_dirent
{
    long d_ino; /* inode number 索引节点号 */
    off_t d_off; /* offset to this dirent 在目录文件中的偏移 */
    unsigned short d_reclen; /* length of this d_name 文件名长 */
    unsigned char d_type; /* the type of d_name 文件类型 */
    char d_name [NAME_MAX+1]; /* file name (null-terminated) 文件名，最长255字符 */
};
    
typedef struct ppc_dirstream
{
    void *__fd;
    char *__data;
    int __entry_data;
    char *__ptr;
    int __entry_ptr;
    size_t __allocation;
    size_t __size;
    struct ppc_dirent **p_data;
    off_t  d_off;
}PPC_DIR;

    
struct ppc_stat {
    dev_t	 	st_dev;		/* [XSI] ID of device containing file */
    ino_t	  	st_ino;		/* [XSI] File serial number */
    mode_t	 	st_mode;	/* [XSI] Mode of file (see below) */
    nlink_t		st_nlink;	/* [XSI] Number of hard links */
    uid_t		st_uid;		/* [XSI] User ID of the file */
    gid_t		st_gid;		/* [XSI] Group ID of the file */
    dev_t		st_rdev;	/* [XSI] Device ID */
#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
    struct	timespec st_atimespec;	/* time of last access */
    struct	timespec st_mtimespec;	/* time of last data modification */
    struct	timespec st_ctimespec;	/* time of last status change */
#else
    time_t		st_atime_t;	/* [XSI] Time of last access */
    long		st_atimensec;	/* nsec of last access */
    time_t		st_mtime_t;	/* [XSI] Last data modification time */
    long		st_mtimensec;	/* last data modification nsec */
    time_t		st_ctime_t;	/* [XSI] Time of last status change */
    long		st_ctimensec;	/* nsec of last status change */
#endif
    off_t		st_size;	/* [XSI] file size, in bytes */
	long 		st_blksize; /*best I/O block size */
	long 		st_blocks; /*number of 512-byte blocks allocated*/
    //blkcnt_t	st_blocks;	/* [XSI] blocks allocated for file */
    //blksize_t	st_blksize;	/* [XSI] optimal blocksize for I/O */
    __uint32_t	st_flags;	/* user defined flags for file */
    __uint32_t	st_gen;		/* file generation number */
    __int32_t	st_lspare;	/* RESERVED: DO NOT USE! */
    __int64_t	st_qspare[2];	/* RESERVED: DO NOT USE! */
};
    
PPC_DIR *ppc_opendir(const char *path,char *token);

struct ppc_dirent *ppc_readdir(PPC_DIR *dp,char *token);

void ppc_rewinddir(PPC_DIR *dp,char *token);

int ppc_closedir(PPC_DIR *dp,char *token);

long ppc_telldir(PPC_DIR *dp,char *token);

void ppc_seekdir(PPC_DIR *dp,long loc,char *token);

int ppc_stat(const char *file_name, struct ppc_stat *buf,char *token);
    
/**
 * function:’à∫≈?￠≤·
 * param:
 * username ”√a?√?
 * password √???
 * return:0 sucess,!0 failed
 */
ERROR_CODE_PPC ppc_register(char* username,char* password);


/**
 * function:’à∫≈?￠≤·￡¨÷?≥÷?¨∏?”√a????àμ??o
 * param:
 * username ”√a?√?
 * password √???
 *  return:0 sucess,!0 failed
 */
ERROR_CODE_PPC ppc_login(char* username,char* password,char **token);
/**
 * function:”√a??à≥?
 * param:
 * token ”√a?±í??
 * return:0 sucess,!0 failed
 */
ERROR_CODE_PPC ppc_logout(char *token);

/*
 * token￡∫μ??o?±∑μa?μ?token
 */
PFILE* ppc_fopen(const char * path,const char * mode,char *token);

/**
 * ??o????°∑Ω∑?
 */
size_t ppc_fread( void *buffer, size_t size, size_t count, PFILE *fp);

/**
 * ??o?–￥∑Ω∑?
 */
size_t ppc_fwrite(const void* buffer, size_t size, size_t count, PFILE* fp);

/**
 * ??o?÷∏’??′“?
 * fromwhere￡??′“????o?a÷√￡∫??o??∑0(SEEK_SET)￡¨μ±?∞?a÷√1(SEEK_CUR)￡¨??o??≤2(SEEK_END)￡???a??o￡¨?′“?offset￡?÷∏’??′“???￡?∏??÷Ω?μ??a÷√°￡
 */
off_t ppc_fseek(PFILE *fp, off_t offset, int fromwhere);


/**
 * π?±’??o?÷∏’?
 */
void ppc_fclose( PFILE *fp);




#ifdef __cplusplus
}
#endif


#endif

