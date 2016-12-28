/*
 * =============================================================================
 *
 *       Filename:  ppclib.c
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
#include "ppclib.h"
#include "ppc_file_client.h"
#include "my_debug.h"
#include "assert.h"


/**
 * function:账号注册
 * param:
 * username 用户名
 * password 密码
 * return:0 sucess,!0 failed
 */
ERROR_CODE_PPC ppc_register(char* username,char* password)
{
    int res = handle_register_task(username,password);
    if(res != 0)
    {
        return FAIL;
    }
	return SUCCESS;
}


/**
 * function:账号注册，支持同个用户多端登录
 * param:
 * username 用户名
 * password 密码
 *  return:0 sucess,!0 failed
 */
ERROR_CODE_PPC ppc_login(char* username,char* password,char **token)
{
    int res = handle_login_task(username,password,token);
    if(res != 0)
    {
        return USERNAME_NOT_FOUND;
    }
    
	return SUCCESS;
}
/**
 * function:用户退出
 * param:
 * token 用户标识
 * return:0 sucess,!0 failed
 */
ERROR_CODE_PPC ppc_logout(char *token)
{
    int res = handle_logout_task(token);
    if(res != 0)
    {
        return USERNAME_NOT_FOUND;
    }
	return SUCCESS;
}

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
 * token：登录时返回的token
 */
PFILE* ppc_fopen(const char *path,const char *mode,char *token)
{
	if(path == NULL)
	{
		DMCLOG_E("para is null");
		return NULL;
	}
	PFILE *fp = (PFILE *)calloc(1,sizeof(PFILE));
	assert(fp != NULL);
    fp->token = token;
	strcpy(fp->mode,mode);
	fp->offset = 0;
	fp->srcPath = path;
    //fp->token = (char *)token;
	int ret = handle_open_task(fp);
	if(ret != 0)
	{
		DMCLOG_E("handle open error");
		safe_free(fp);
		return NULL;
	}
	return fp;
}

/**
 * 文件读取方法
 */
size_t ppc_fread( void *buffer, size_t size, size_t count, PFILE *fp)
{
	 return handle_read_task(fp,count,buffer);
}

/**
 * 文件写方法
 */
size_t ppc_fwrite(const void* buffer, size_t size, size_t count, PFILE* fp)
{
	return handle_write_task(fp,count,buffer);
}

/**
 * 文件指针偏移
 * fromwhere（偏移起始位置：文件头0(SEEK_SET)，当前位置1(SEEK_CUR)，文件尾2(SEEK_END)）为基准，偏移offset（指针偏移量）个字节的位置。
 */
off_t ppc_fseek( PFILE* fp, off_t offset, int fromwhere)
{
	if(fp == NULL||fp->srcPath == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	handle_close_task(fp);
	fp->offset = fromwhere + offset;
    int ret = handle_open_task(fp);
    if(ret != 0)
    {
        DMCLOG_E("handle open error");
        safe_free(fp);
        return -1;
    }
	return fp->offset;
}


/**
 * 关闭文件指针
 */
void ppc_fclose( PFILE *fp)
{
	handle_close_task(fp);
	safe_free(fp);
}

PPC_DIR *ppc_opendir(const char *path,char *token)
{
    PPC_DIR * p_dir = (PPC_DIR *)calloc(1,sizeof(PPC_DIR));
    int ret = handle_opendir_task(path,p_dir,token);
    if(ret != 0)
    {
        return NULL;
    }
    return p_dir;
}

struct ppc_dirent *ppc_readdir(PPC_DIR *p_dir,char *token)
{
    return handle_readdir_task(p_dir);
}

void ppc_rewinddir(PPC_DIR *dp,char *token)
{

}

int ppc_closedir(PPC_DIR *dp,char *token)
{
    return handle_closedir_task(dp);
}

long ppc_telldir(PPC_DIR *dp,char *token)
{
    return handle_telldir_task(dp);
}

void ppc_seekdir(PPC_DIR *dp,long loc,char *token)
{
    handle_seekdir_task(dp, loc);
}

int ppc_stat(const char *file_name, struct ppc_stat *buf,char *token)
{
    int ret = handle_stat_task(file_name,buf,token);
    if(ret != 0)
    {
        DMCLOG_E("stat %s error",file_name);
        return ret;
    }
    return ret;
}



