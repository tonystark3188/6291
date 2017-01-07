/**
 * 基础文件操作接口
 */
#ifndef _BFAVFS_H_
#define _BFAVFS_H_
//open，read，write，seek，close等。
//copy，move，stat，rename


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "cloud_errno.h"
#include "db_table.h"
#include "rfsvfs.h"
#include "list.h"

typedef struct _BucketObject{
	char 		bucket_name[MAX_BUCKET_NAME_LEN];
	char 		path[MAX_FILE_PATH_LEN];
	char 		file_uuid[MAX_FILE_UUID_LEN];
	struct dl_list *head;
	int 		file_type;
	long long 	offset;
}BucketObject;


typedef struct _VFILE{
	void	*token;
	char	mode[8];
	FILE 	*fp;
	int 	fd;
	char 	*srcPath;
	char	*realPath;
	size_t 	offset;
	size_t 	length;
	char	uuid[MAX_FILE_UUID_LEN];
	BucketObject *bobject;
}VFILE;

BucketObject *build_bucket_object(char *path,void *token);


/**
 * 文件读取方法
 */
size_t bfavfs_fread( void *buffer, size_t size, size_t count, VFILE *vf);

/**
 * 文件写方法
 */
size_t bfavfs_fwrite(const void* buffer, size_t size, size_t count, VFILE* vf);


/**
 * 文件读取方法
 */
size_t _bfavfs_fread( void *buffer, size_t size, size_t count, VFILE *vf);

/**
 * 文件写方法
 */
size_t _bfavfs_fwrite(const void* buffer, size_t size, size_t count, VFILE* vf);

/**
 * 文件指针偏移
 * fromwhere（偏移起始位置：文件头0(SEEK_SET)，当前位置1(SEEK_CUR)，文件尾2(SEEK_END)）为基准，偏移offset（指针偏移量）个字节的位置。
 */
int _bfavfs_fseek(VFILE *vf, long offset, int fromwhere);



/**
 * 文件指针偏移
 * fromwhere（偏移起始位置：文件头0(SEEK_SET)，当前位置1(SEEK_CUR)，文件尾2(SEEK_END)）为基准，偏移offset（指针偏移量）个字节的位置。
 */
int bfavfs_fseek(VFILE *vf, long offset, int fromwhere);


/**
 * 获取文件属性
 */
int bfavfs_fstat(const char * path,struct stat *st,void *token);


/**
 * 判断文件是否已读完
 * 文件未结束返回0
 * 文件结束返回1
 */
int bfavfs_feof(VFILE* vf);

/**
 * 删除目录下类型文件
 
 */
int bfavfs_remove_type(char* path,int file_type,void * token);


/**
 * 通过UUID 判断文件是否存在
 */
int bfavfs_exist(const char *uuid,void *token);

/**
 *通过UUID list 判断文件是否存在
 */
int bfavfs_list_exist(struct dl_list *head,void *token);

/**
 * 设置文件属性
 */
int bfavfs_fsetattr(const char *path,void *token);


/**
 * 通过文件路径获取当前目录下的文件列表。
 * param:
 * dirPath 目录路径
 * startIndex 其实文件索引值（从0开始）
 * count 需要从startIndex的文件条数
 * category 获取那些类别文件
 * sortType 排序方式
 */
ERROR_CODE_VFS _bfavfs_get_file_list(BucketObject* sObject,v_file_list_t *vlist,void * token);

VFILE* _bfavfs_fopen(BucketObject* sObject,const char * mode,void * token);

int _bfavfs_fclose( VFILE *vf,void* token);

VFILE* bfavfs_fopen(BucketObject* sObject,const char * mode,void * token);

int bfavfs_fclose( VFILE *vf,void* token);


/**
 * 获取文件属性
 */
int _bfavfs_fstat(BucketObject* sObject,struct stat *st,void *token);

/**
 *通过UUID 判断文件是否存在
 */
int _bfavfs_exist(BucketObject* sObject,void *token);

/**
 * 多桶拷贝
 */
int _bfavfs_fcopy(BucketObject* sObject,BucketObject* dObject,void * token);


/**
 * 多桶移动
 */
int _bfavfs_fmove(BucketObject* sObject,BucketObject* dObject,void * token);

/**
 * 文件重命名
 */
int _bfavfs_frename(BucketObject* sObject,BucketObject* dObject,void * token);

/**
 * 删除目录下类型文件
 
 */
int _bfavfs_remove(BucketObject* sObject,void * token);


/**
 * 创建文件夹
 
 */
int _bfavfs_mkdir(BucketObject* sObject,void * token);

/**
 * 设置文件属性
 */
int _bfavfs_fsetattr(BucketObject* sObject,void *token);


/**
 *参数fd是该进程打开来的文件描述符。 函数成功执行时，返回0。失败返回-1，errno被设为以下的某个值
EBADF： 文件描述词无效
EIO ： 读写的过程中发生错误
EROFS, EINVAL：文件所在的文件系统不支持同步
 */
int _bfavfs_fsync( VFILE *vf,void* token);

/**
 *ftruncate()会将参数fd指定的文件大小改为参数length指定的大小。
　　参数fd为已打开的文件描述词，而且必须是以写入模式打开的文件。
　　如果原来的文件大小比参数length大，则超过的部分会被删去。
 *　　返回值:执行成功则返回0，失败返回-1，错误原因存于errno。
 */
int _bfavfs_ftruncate( VFILE *vf,off_t length,void* token);


/**
 *fallocate的功能是为文件预分配物理空间。
 *返回值:执行成功则返回0，失败返回-1，错误原因存于errno。
 */
int _bfavfs_fallocate( VFILE *vf, int mode, off_t offset, off_t len,void* token);

/**
 *link()以参数newpath 指定的名称来建立一个新的连接(硬连接)到参数oldpath 所指定的已存在文件. 如果参数newpath 指定的名称为一已存在的文件则不会建立连接.
 *返回值：成功则返回0, 失败返回-1, 错误原因存于errno.
 */
int _bfavfs_link( BucketObject* oObject, BucketObject* nObject,void* token);

/**
 *readlink()会将参数path 的符号连接内容存到参数buf 所指的内存空间, 返回的内容不是以NULL作字符串结尾, 但会将字符串的字符数返回. 若参数bufsiz 小于符号连接的内容长度, 过长的内容会被截断.
 *返回值：执行成功则传符号连接所指的文件路径字符串, 失败则返回-1, 错误代码存于errno.
 */
int _bfavfs_readlink( BucketObject* rObject,char * buf, size_t bufsiz,void* token);

/**
 *symlink()以参数newpath 指定的名称来建立一个新的连接(符号连接)到参数oldpath 所指定的已存在文件. 参数oldpath 指定的文件不一定要存在, 如果参数newpath 指定的名称为一已存在的文件则不会建立连接.
 *返回值：成功则返回0, 失败返回-1, 错误原因存于errno.
 */
int _bfavfs_symlink( BucketObject* oObject, BucketObject* nObject,void* token);
//函数futimensat则可以通过制定一个文件的绝对路径来修改文件的时间属性，而此时fd参数被忽略。
int _bfavfs_utimensat( BucketObject* rObject,const struct timespec times[2], int flag,void* token);
//获取文件系统信息
int _bfavfs_statvfs( BucketObject* rObject, struct statvfs *buf, void* token);

#endif

