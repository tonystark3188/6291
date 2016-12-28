/**
 * 真实文件系统模块。
 * 专门用来存取文件
 */
#ifndef _RFSVFS_H_
#define _RFSVFS_H_

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>


/**
 * 运行的平台，如果是非 windows系统需要屏蔽该项
 */
//#define _WIN32


/**
 * 系统默认的内存路径
 */
#define RFSVFS_DIR_PATH_DEFAULT "/tmp/mnt/SD-disk-1/"

/**
 * 默认的参数文件
 */
#define RFSVFS_PARAM_FILENAME_DEFAULT "rfsvfs"
//#define RFSVFS_PARAM_FILENAME_DEFAULT ".rfsvfs"


/**
 * 路径默认长度
 */
#define RFSVFS_PATH_LENGTH_DEFAULT 32

/**
 * 主目录数量
 */
#define RFSVFS_MAJOR_DIR_SIZE  1000
/**
 * 次级目录数量
 */
#define RFSVFS_MINOR_DIR_SIZE  1000
/**
 * 文件目录数量
 */
#define RFSVFS_FILE_DIR_SIZE  1000


#define RFSVFS_MAJOR_DIV  (RFSVFS_MINOR_DIR_SIZE * RFSVFS_FILE_DIR_SIZE)
#define RFSVFS_MINOR_DIV  RFSVFS_FILE_DIR_SIZE

/**
 * 模块初始化主目录路径或磁盘路径
 */
int rfsvfs_init(char *path);

/**
 * 模块释放资源
 */
void rfsvfs_destroy();


/**
 * 获取一个未使用的文件路径
 */
int rfsvfs_get_new_file_path(char pathBuf[RFSVFS_PATH_LENGTH_DEFAULT]);

/**
 * 以某种方式打开文件
 * param：
 * path：文件路径
 * mode：打开模式
 * “r” 以只读方式打开文件，该文件必须存在。
“r+” 以可读写方式打开文件，该文件必须存在。
”rb+“ 读写打开一个二进制文件，允许读写数据，文件必须存在。
“w” 打开只写文件，若文件存在则文件长度清为0，即该文件内容会消失。若文件不存在则建立该文件。
“w+” 打开可读写文件，若文件存在则文件长度清为零，即该文件内容会消失。若文件不存在则建立该文件。
“a” 以附加的方式打开只写文件。若文件不存在，则会建立该文件，如果文件存在，写入的数据会被加到文件尾，即文件原先的内容会被保留。（EOF符保留）
”a+“ 以附加方式打开可读写的文件。若文件不存在，则会建立该文件，如果文件存在，写入的数据会被加到文件尾后，即文件原先的内容会被保留。 （原来的EOF符不保留）
“wb” 只写打开或新建一个二进制文件；只允许写数据。
“wb+” 读写打开或建立一个二进制文件，允许读和写
“wx” 创建文本文件,只允许写入数据.[C11]
“wbx” 创建一个二进制文件,只允许写入数据.[C11]
”w+x“ 创建一个文本文件,允许读写.[C11]
“wb+x” 创建一个二进制文件,允许读写.[C11]
“w+bx” 和"wb+x"相同[C11]
“rt” 只读打开一个文本文件，只允许读数据
　　“wt” 只写打开或建立一个文本文件，只允许写数据
　　“at” 追加打开一个文本文件，并在文件末尾写数据
　　“rb” 只读打开一个二进制文件，只允许读数据
　　“wb” 只写打开或建立一个二进制文件，只允许写数据
　　“ab” 追加打开一个二进制文件，并在文件末尾写数据
　　“rt+” 读写打开一个文本文件，允许读和写
　　“wt+” 读写打开或建立一个文本文件，允许读写
　　“at+” 读写打开一个文本文件，允许读，或在文件末追加数据
　　“rb+” 读写打开一个二进制文件，允许读和写
　　“ab+” 读写打开一个二进制文件，允许读，或在文件末追加数据
 *
 */
FILE* rfsvfs_fopen(const char * path,const char * mode);

/**
 * 文件读取方法
 */
size_t rfsvfs_fread( void *buffer, size_t size, size_t count, FILE *fp);

/**
 * 文件写方法
 */
size_t rfsvfs_fwrite(const void* buffer, size_t size, size_t count, FILE* fp);

/**
 * 文件指针偏移
 * fromwhere（偏移起始位置：文件头0(SEEK_SET)，当前位置1(SEEK_CUR)，文件尾2(SEEK_END)）为基准，偏移offset（指针偏移量）个字节的位置。
 */
int rfsvfs_fseek(FILE *fp, long offset, int fromwhere);

/**
 * �ж��ļ��Ƿ��Ѷ���
 * �ļ�δ��������0
 * �ļ���������1
 */
int rfsvfs_feof(FILE* fp);

/**
 * 关闭文件指针
 */
int rfsvfs_fclose(FILE *fp);


/**
 * ��ȡ�ļ�����
 */
int rfsvfs_fstat(const char * path,struct stat *stp);


/**
 * 删除文件
 */
int rfsvfs_remove(char* file_path);

int rfsvfs_open(const char * path,int flags,mode_t mode);

/**
 * 文件读取方法
 */
ssize_t rfsvfs_read( int fd,void *buf,int nbyte);

/**
 * 文件写方法
 */
ssize_t rfsvfs_write(int fd,void *buf,int nbyte);

/**
 * 文件指针偏移
 * fromwhere（偏移起始位置：文件头0(SEEK_SET)，当前位置1(SEEK_CUR)，文件尾2(SEEK_END)）为基准，偏移offset（指针偏移量）个字节的位置。
 */
off_t rfsvfs_lseek(int fd, off_t offset, int whence);


/**
 * 关闭文件指针
 */
int rfsvfs_close(int fd);



#endif
