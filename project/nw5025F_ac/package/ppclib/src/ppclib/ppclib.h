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
//#include <fcntl.h>
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
#include <sys/statvfs.h>


//#include "base.h"
#ifdef __cplusplus
extern "C"{
#endif

#define PKG_NAME 			"libppc"
#define PKG_VERSION 		"1.01.06"

#define PUBLIC_USER_NAME	"public"
#define PUBLIC_PASSWORD		"13141314"

typedef struct _pkg_version{
	char pkg_name[32];				//软件包名称
	char pkg_version[32];			//软件包版本
}pkg_version;

typedef enum ERROR_CODE_PPC
{
      SUCCESS=0,		//返回成功
      FAIL = -1,		//返回错误
      NOT_AUTHORIZATION = 10000,	//未认证
      PERMISSION_DENIED = 10001,	//权限不允许
      USERNAME_NOT_FOUND= 10002,	//用户名未找到
      PASSWORD_ERROR	= 10003		//密码错误
} ERROR_CODE_PPC;

//typedef unsigned long long uint64_t;
typedef long long _int64_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
// typedef char int8_t;
typedef unsigned char uint8_t;

typedef struct _PFILE{
	char	mode[8];
	int 	fd;
	char 	*srcPath;
	size_t 	offset;
	size_t 	length;
    _int64_t 	token;
}PFILE;

typedef struct _FFILE{
	int 	fd;
	char 	*srcPath;
	size_t 	offset;
	size_t 	length;
	int 	flag;
	mode_t 	mode;
    _int64_t 	token;
}FFILE;

#if 0    
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
	#if 0
    void *__fd;
    char *__data;
    int __entry_data;
    char *__ptr;
    int __entry_ptr;
    size_t __allocation;
    size_t __size;
    struct ppc_dirent **p_data;
    off_t  d_off;
	#endif
	void *__fd;    
    char *__data;    
    int __entry_data;    
    char *__ptr;    
    int __entry_ptr;    
    size_t __allocation;    
    size_t __size;    
    //__libc_lock_define (, __lock)
}PPC_DIR;

typedef struct _ppc_dir_info
{
	struct dirent **p_data;
	off_t  d_off;
}ppc_dir_info;
   
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
#endif


/*************************************************
Function:  	ppc_get_pkg_version
Description:  	获取库版本信息
Input: 
Output:  		pkg_version_info  库版本信息结构体
Return:  		0 sucess,!0 failed
Others: 
*************************************************/
int ppc_get_pkg_version(pkg_version *pkg_version_info);	


/*************************************************
Function: 	ppc_initialise
Description:  	ppc lib初始化
Input: 		
Output: 
Return: 	 	0 sucess,!0 failed
Others: 
*************************************************/
int ppc_initialise();


/*************************************************
Function: 	ppc_initialise
Description:  	ppc lib反初始化
Input: 		
Output: 
Return: 	 	0 sucess,!0 failed
Others: 
*************************************************/
int ppc_uninitialise();


/*************************************************
Function: 	ppc_register
Description:  	账号注册
Input: 		username 用户名
			password 密码
Output: 
Return: 	 	0 sucess,!0 failed
Others: 
*************************************************/
ERROR_CODE_PPC ppc_register(char* username,char* password);


/*************************************************
Function: 	ppc_login
Description:  	账号注册，支持同个用户多端登录
Input: 		username 用户名
			password 密码
Output: 		token token值
Return: 	 	0 sucess,!0 failed
Others: 
*************************************************/
ERROR_CODE_PPC ppc_login(char* username,char* password, _int64_t *token);


/*************************************************
Function: 	ppc_logout
Description:  	用户退出
Input: 		token token值
Output: 
Return: 	 	0 sucess,!0 failed
Others: 
*************************************************/
ERROR_CODE_PPC ppc_logout(_int64_t token);


/*************************************************
Function: 	ppc_opendir
Description:  	打开目录
Input: 		path 打开的目录文件
			token token值
Output: 
Return: 	 	目录指针
Others: 
*************************************************/
DIR *ppc_opendir(const char *path, _int64_t token);


/*************************************************
Function: 	ppc_fdopendir
Description:  	根据文件描述打开目录
Input: 		fd 打开的目录文件描述符
			token token值
Output: 
Return: 	 	目录指针
Others: 
*************************************************/
DIR *ppc_fdopendir(int fd, _int64_t token);


/*************************************************
Function: 	ppc_readdir
Description:  	读取目录
Input: 		dp 目录指针
			token token值
Output: 
Return: 	 	文件信息指针
Others: 
*************************************************/
struct dirent *ppc_readdir(DIR *dp, _int64_t token);


/*************************************************
Function: 	ppc_rewinddir
Description:  	重置目录指针
Input: 		dp 目录指针
			token token值
Output: 
Return: 	 
Others: 
*************************************************/
void ppc_rewinddir(DIR *dp, _int64_t token);


/*************************************************
Function: 	ppc_closedir
Description:  	关闭目录
Input: 		dp 目录指针
			token token值
Output: 
Return: 	 	0 sucess,!0 failed
Others: 
*************************************************/
int ppc_closedir(DIR *dp, _int64_t token);


/*************************************************
Function: 	ppc_telldir
Description:  	获取目录位置
Input: 		dp 目录指针
			token token值
Output: 
Return: 	 	小于0: fail;大于0: 目录位置
Others: 
*************************************************/
off_t ppc_telldir(DIR *dp, _int64_t token);


/*************************************************
Function: 	ppc_seekdir
Description:  	跳到指定目录位置
Input: 		dp 目录指针
			loc 目录位置
			token token值
Output: 
Return: 	 	
Others: 
*************************************************/
void ppc_seekdir(DIR *dp,off_t loc, _int64_t token);


/*************************************************
Function: 	ppc_mkdir
Description:  	创建目录
Input: 		dirname 创建的目录路径
			mode 创建的目录权限
			token token值
Output: 
Return: 	 	0 sucess,!0 failed
Others: 
*************************************************/
int ppc_mkdir(const char *dirname, mode_t mode, _int64_t token);


/*************************************************
Function: 	ppc_rmdir
Description:  	删除目录
Input: 		path 要删除的目录文件
			token token值
Output: 
Return: 	 	0 sucess,!0 failed
Others: 
*************************************************/
int ppc_rmdir(const char *pathname, _int64_t token);


/*************************************************
Function: 	ppc_stat
Description:  	打开目录
Input: 		file_name 操作的文件路径
			token token值
Output: 		buf: 返回的信息结构体
Return: 	 	0 sucess,!0 failed
Others: 
*************************************************/
int ppc_stat(const char *file_name, struct stat *buf, _int64_t token);


/*************************************************
Function: 	ppc_opendir
Description:  	通过文件描述符获取文件信息
Input: 		fildes 操作的文件描述符
			token token值
Output: 		buf: 返回的信息结构体
Return: 	 	0 sucess,!0 failed
Others: 
*************************************************/
int ppc_fstat(int fildes, struct stat *buf, _int64_t token);


/*************************************************
Function: 	ppc_opendir
Description:  	通过文件路径获取文件信息(包括符号链接文件)
Input: 		path 操作的文件路径
			token token值
Output: 		buf: 返回的信息结构体
Return: 	 	0 sucess,!0 failed
Others: 
*************************************************/
int ppc_lstat(const char *path, struct stat *buf, _int64_t token);


/*************************************************
Function: 	ppc_open
Description:  	打开文件
Input: 		pathname 打开的文件路径
			flags 打开文件的方式
			mode 仅当创建新文件时才使用，用于指定文件的访问权限位（access permission bits）
			token token值
Output: 		
Return: 	 	0 sucess,!0 failed
Others: 
*************************************************/
int ppc_open(const char *pathname, int flags, mode_t mode, _int64_t token);


/*************************************************
Function: 	ppc_read
Description:  	通过文件描述符读取文件
Input: 		fd 文件描述符
			count 读取的数据长度
Output: 		buf 读取的数据
Return: 	 	小于0: 读取失败；大于0: 读取的数据长度
Others: 
*************************************************/
ssize_t ppc_read(int fd, void *buf, size_t count);


/*************************************************
Function: 	ppc_write
Description:  	通过文件描述符写文件
Input: 		fd 文件描述符
			count 写入的文件长度
Output: 		buf 写入的数据缓存
Return: 	 	小于0: 读取失败；大于0: 写入的数据长度
Others: 
*************************************************/
ssize_t ppc_write(int fd, void *buf, size_t count);


/*************************************************
Function: 	ppc_close
Description:  	关闭文件描述符
Input: 		fd 被关闭的文件描述符
Output: 		
Return: 	 
Others: 
*************************************************/
int ppc_close(int fd);


/*************************************************
Function: 	ppc_lseek
Description:  	跳转到指定文件长度位置
Input: 		fd 操作的文件描述符
			offset 偏移位置
			whence相对位置
			token token值
Output: 		
Return: 	 	小于0: 失败；大于0: 跳转的位置
Others: 
*************************************************/
off_t ppc_lseek(int fd, off_t offset ,int whence, _int64_t token);


/*************************************************
Function: 	ppc_pread
Description:  	通过带偏移量地原子读取文件
Input: 		fd 操作的文件描述符
			count 读取的数据长度
			offset 读取的文件位置
			token token值
Output: 		buf 读取的数据缓存
Return: 	 	小于0: 读取失败；大于0: 读取数据长度
Others: 
*************************************************/
ssize_t ppc_pread(int fd, void *buf, size_t count, off_t offset, _int64_t token);


/*************************************************
Function: 	ppc_pwrite
Description:  	通过带偏移量地原子写文件
Input: 		fd 操作的文件描述符
			buf 写入的数据缓存
			count 写入的数据长度
			offset 写入的文件位置
			token token值
Output: 		
Return: 	 	0 sucess,!0 failed
Others: 
*************************************************/
ssize_t ppc_pwrite(int fd, void *buf, size_t count, off_t offset, _int64_t token);


/*************************************************
Function: 	ppc_writev
Description:  	聚集写,即收集内存中分散的若干缓冲区中的数据写至文件的连续区域中
Input: 		iov 数据结构体数组
			token token值
Output: 		
Return: 	 	写数据长度
Others: 
*************************************************/
ssize_t ppc_writev( int fd, const struct iovec *iov, int cnt, _int64_t token);


/*************************************************
Function: 	ppc_rename
Description:  	重命名操作
Input: 		oldname 原文件名
			newname 新文件名
			token token值
Output: 		
Return: 	 	0 sucess,!0 failed
Others: 
*************************************************/
int ppc_rename(const char *oldname, const char *newname, _int64_t token);


/*************************************************
Function: 	ppc_unlink
Description:  	删除文件链接及文件
Input: 		pathname 删除的文件路径
			token token值
Output: 		
Return: 	 	0 sucess,!0 failed
Others: 
*************************************************/
int ppc_unlink(const char *pathname, _int64_t token);

int ppc_chmod(const char *pathname, mode_t mode, _int64_t token);

int ppc_fchmod(int filedes, mode_t mode, _int64_t token);

int ppc_chown(const char *path, uid_t owner, gid_t group, _int64_t token);

int ppc_fchown(int fd, uid_t owner, gid_t group, _int64_t token);

int ppc_lchown(const char *path, uid_t owner, gid_t group, _int64_t token);


/*************************************************
Function: 	ppc_utimensat
Description:  	修改文件访问时间和修改时间的时间戳
Input: 		dirfd 相对路径或绝对路径
			pathname 路径名
			times 时间参数
			flags 标志
			token token值
Output: 		
Return: 	 	0 sucess,!0 failed
Others: 
*************************************************/
int ppc_utimensat(int dirfd, const char *pathname, 
		const struct timespec times[2], int flags, _int64_t token);


/*************************************************
Function: 	ppc_chdir
Description:  	更改当前工作目录
Input: 		path 新的工作目录路径
			token token值
Output: 		
Return: 	 	0 sucess,!0 failed
Others: 
*************************************************/
int ppc_chdir(const char *path, _int64_t token);


/*************************************************
Function: 	ppc_getwd
Description:  	获取当前工作目录
Input: 		token token值
Output: 		buf 工作目录指针
Return: 	 	NULL 获取失败；!NULL 指向文件路径的指针
Others: 
*************************************************/
char *ppc_getwd(char *buf, _int64_t token);


/*************************************************
Function: 	ppc_fopen
Description:  	以流的方式打开文件
Input: 		path 文件路径
			mode 打开的方式
			token token值
Output: 		
Return: 	 	0 sucess,!0 failed
Others: 
*************************************************/
FILE *ppc_fopen(const char *path, const char *mode, _int64_t token);


/*************************************************
Function: 	ppc_fread
Description:  	以流的方式读取文件
Input: 		size 每次读取的大小
			count 读取的长度
			fp 指向被读取文件的文件指针
			token token值
Output: 		buffer 读取的数据缓存
Return: 	 	小于0 : 失败；大于0 : 读取的数据长度
Others: 
*************************************************/
size_t ppc_fread(void *buffer, size_t size, size_t count, FILE *fp);


/*************************************************
Function: 	ppc_fwrite
Description:  	以流的方式写入文件
Input: 		buffer 写入的数据缓存
			size 每次读取的大小
			count 读取的长度
			fp 指向被读取文件的文件指针
			token token值
Output: 		
Return: 	 	小于0 : 失败；大于0 : 读取的数据长度
Others: 
*************************************************/
size_t ppc_fwrite(const void* buffer, size_t size, size_t count, FILE *fp);


/*************************************************
Function: 	ppc_fseek
Description:  	文件指针偏移
Input: 		fp 文件指针
			offset 偏移位置
			fromwhere 偏移起始位置：文件头0(SEEK_SET)，当前位置1(SEEK_CUR)，文件尾2(SEEK_END)）为基准，偏移offset（指针偏移量）个字节的位置。
			token token值
Output: 		
Return: 	 	小于0失败，大于0  : 偏移位置
Others: 
*************************************************/
off_t ppc_fseek(FILE *fp, off_t offset, int fromwhere, _int64_t token);


/*************************************************
Function: 	ppc_ftell
Description:  	获取当前文件指针位置
Input: 		fp 文件指针
			token token值
Output: 		
Return: 	 	小于0: 失败；大于0 : 当前文件指针位置
Others: 
*************************************************/
off_t ppc_ftell(FILE *fp, _int64_t token);


/*************************************************
Function: 	ppc_fclose
Description:  	关闭文件指针
Input: 		fp 文件指针
			token token值
Output: 		
Return: 	 	
Others: 
*************************************************/
int ppc_fclose(FILE *fp);

/*
  *将fd指定的文件大小改为参数length指定的大小
  */
int ppc_ftruncate(int fd,off_t length, _int64_t token);

/*
  *为文件预分配物理空间
  */
int ppc_fallocate(int fd, int mode, off_t offset, off_t len, _int64_t token);

/*
  *实现零拷贝
  */
ssize_t ppc_sendfile(int out_fd, int in_fd, off_t *offset, size_t count, _int64_t token);

/*
  *实现零拷贝
  */
ssize_t ppc_splice(int fd_in,loff_t* off_t,int fd_out,loff_t* off_out,size_t len,unsigned int flags, _int64_t token);

/*
  *文件同步
  */
int ppc_fsync(int fd, _int64_t token);

/*
  *建立符号链接
  */
int ppc_symlink(const char * oldpath, const char * newpath, _int64_t token);

/*
  *取得符号连接所指的文件
  */
ssize_t ppc_readlink(const char *path, char *buf, size_t bufsiz, _int64_t token);

/*
  *创建硬链接
  */
int ppc_link (const char * oldpath, const char *newpath, _int64_t token);

/*
  *创建文件
  */
int ppc_mknod(const char *path, mode_t mode, dev_t dev, _int64_t token);


/*************************************************
Function: 	ppc_realpath
Description:  	获取文件绝对路径
Input: 		path 文件相对路径
			token token值
Output: 		resolved_path 文件绝对路径
Return: 	 	指向文件相对路径的指针
Others: 
*************************************************/
char *ppc_realpath(const char *path, char *resolved_path, _int64_t token);

/*
  *读取文件系统信息
  */
int ppc_statvfs(const char *path, struct statvfs *buf, _int64_t token);

#ifdef __cplusplus
}
#endif


#endif

