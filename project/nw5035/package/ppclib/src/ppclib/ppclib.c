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
#include "ppc_list.h"

/**
  * function:获取库版本信息
  * ppc_version: 库版本信息
  * return:0 sucess,!0 failed
  */
int ppc_get_pkg_version(pkg_version *pkg_version_info)
{
	if(pkg_version_info == NULL){
		return -1;
	}

	strcpy(pkg_version_info->pkg_name, PKG_NAME);
	strcpy(pkg_version_info->pkg_version, PKG_VERSION);

	return 0;
}


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
    
	init_ppc_fd_list();
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
	free_ppc_fd_list();
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
	int ret = handle_fopen_task(fp);
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
	 return handle_fread_task(fp,count,buffer);
}

/**
 * 文件写方法
 */
size_t ppc_fwrite(const void* buffer, size_t size, size_t count, PFILE* fp)
{
	return handle_fwrite_task(fp,count,buffer);
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
	handle_fclose_task(fp);
	fp->offset = fromwhere + offset;
    int ret = handle_fopen_task(fp);
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
	handle_fclose_task(fp);
	safe_free(fp);
}

/*
 *打开目录
 */
PPC_DIR *ppc_opendir(const char *path,char *token)
{
    PPC_DIR * p_dir = (PPC_DIR *)calloc(1,sizeof(PPC_DIR));
    int ret = handle_opendir_task(path,p_dir,token);
    if(ret != 0)
    {
    	safe_free(p_dir);
        return NULL;
    }
    return p_dir;
}

/*
 *打开目录
 */
PPC_DIR *ppc_fdopendir(int fd, char *token)
{
	int ret = 0;
	fd_info *open_fd_info = NULL;

	ret = get_info_from_ppc_fd_list(fd, &open_fd_info);
	if(ret || (open_fd_info == NULL) || (open_fd_info->path == NULL)){
		DMCLOG_E("get_info_from_ppc_fd_list fail");
		return NULL;
	}
	DMCLOG_D("open_fd_info->path: %s", open_fd_info->path);

	PPC_DIR * p_dir = (PPC_DIR *)calloc(1,sizeof(PPC_DIR));
	if(p_dir == NULL){
		DMCLOG_E("p_dir calloc,fail");
		return NULL;
	}


	//ret = handle_fdopendir_task(fd, p_dir, token);
	ret = handle_opendir_task(open_fd_info->path, p_dir, token);
	if(ret != 0){
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
    	safe_free(p_dir);
        return NULL;
    }
	safe_free(open_fd_info->path);
	safe_free(open_fd_info);
	return p_dir;
}

/*
 *读取目录
 */
struct ppc_dirent *ppc_readdir(PPC_DIR *p_dir,char *token)
{
    return handle_readdir_task(p_dir);
}

/*
 *重置目录指针
 */
void ppc_rewinddir(PPC_DIR *dp,char *token)
{
	handle_rewinddir_task(dp);
	return ;
}

/*
 *关闭目录
 */
int ppc_closedir(PPC_DIR *dp,char *token)
{
    return handle_closedir_task(dp);
}

/*
 *获取目录位置
 */
off_t ppc_telldir(PPC_DIR *dp,char *token)
{
    return handle_telldir_task(dp);
}

/*
 *跳到指定目录位置
 */
void ppc_seekdir(PPC_DIR *dp,off_t loc,char *token)
{
	handle_seekdir_task(dp, loc);
    return;
}

/*
 *创建目录
 */
int ppc_mkdir(const char *dirname, mode_t mode, char *token)
{
	// TODO: 缺少mode参数
	return handle_mkdir_task(dirname, mode, token);
}

/*
 *删除目录
 */
int ppc_rmdir(const char *pathname, char *token)
{	
	//文件夹内有文件时，rmdir应返回失败
	return handle_rmdir_task(pathname, token);
}

/*
 *获取文件信息
 */
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

/*
 *打开文件
 * pathname:被打开的文件名
 * flags:
 O_RDONLY以只读方式打开文件
 O_WRONLY以只写方式打开文件
 O_RDWR以可读写方式打开文件。
 上述三种标志位是互斥的，也就是不可同时使用，但可与下列的标志位利用OR(|)运算符组合。
 O_CREAT若欲打开的文件不存在则自动建立该文件。
 O_EXCL如果O_CREAT也被设置，此指令会去检查文件是否存在。文件若不存在则建立该文件，否则将导致打开文件错误。此外，若O_CREAT与O_EXCL同时设置，并且欲打开的文件为符号连接，则会打开文件失败。
 O_NOCTTY如果欲打开的文件为终端机设备时，则不会将该终端机当成进程控制终端机。
 O_TRUNC若文件存在并且以可写的方式打开时，此标志位会令文件长度清为0，而原来存于该文件的资料也会消失。
 O_APPEND当读写文件时会从文件尾开始移动，也就是所写入的数据会以附加的方式加入到文件后面。
 O_NONBLOCK以不可阻断的方式打开文件，也就是无论有无数据读取或等待，都会立即返回进程之中。
 O_NDELAY同O_NONBLOCK。
 O_SYNC以同步的方式打开文件。
 O_NOFOLLOW如果参数pathname所指的文件为一符号连接，则会令打开文件失败。
 O_DIRECTORY如果参数pathname所指的文件并非为一目录，则会令打开文件失败。
 * mode:
 S_IRWXU，00700权限，代表该文件所有者具有可读、可写及可执行的权限。
 S_IRUSR或S_IREAD，00400权限，代表该文件所有者具有可读取的权限。
 S_IWUSR或S_IWRITE，00200权限，代表该文件所有者具有可写入的权限。
 S_IXUSR或S_IEXEC，00100权限，代表该文件所有者具有可执行的权限。
 S_IRWXG00070权限，代表该文件用户组具有可读、可写及可执行的权限。
 S_IRGRP00040权限，代表该文件用户组具有可读的权限。
 S_IWGRP00020权限，代表该文件用户组具有可写入的权限。
 S_IXGRP00010权限，代表该文件用户组具有可执行的权限。
 S_IRWXO00007权限，代表其他用户具有可读、可写及可执行的权限。
 S_IROTH00004权限，代表其他用户具有可读的权限
 S_IWOTH00002权限，代表其他用户具有可写入的权限。
 S_IXOTH00001权限，代表其他用户具有可执行的权限。
 */
int ppc_open(const char*pathname, int flags, mode_t mode, char *token)
{
	ENTER_FUNC();
	if(pathname == NULL)
	{
		DMCLOG_E("para is null");
		return NULL;
	}
	FFILE *file_info = (FFILE *)calloc(1,sizeof(FFILE));
	if(file_info == NULL){
		DMCLOG_E("calloc file_info error");
		return -1;
	}

	//assert(file_info != NULL);
	file_info->token = token;
	file_info->offset = 0;
	file_info->srcPath = pathname;
	file_info->flag = flags;
	file_info->mode = mode;
    //file_info->token = (char *)token;
	int ret = handle_open_task(file_info);
	if(ret != 0){
		DMCLOG_E("handle open error");
		safe_free(file_info);
		return -1;
	}

	fd_info *open_fd_info = (fd_info *)calloc(1, sizeof(fd_info));
	if(open_fd_info == NULL)
	{
		DMCLOG_E("malloc open_fd_info fail");
		safe_free(file_info);
		return -1;
	}
	
	open_fd_info->fd = file_info->fd;
	open_fd_info->socket_fd = file_info->fd;
	open_fd_info->path = strdup(pathname);
	ret = add_info_for_ppc_fd_list(open_fd_info);
	if(ret){
		DMCLOG_E("add_info_for_ppc_fd_list fail");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		safe_free(file_info);
		return -1;
	}

	return file_info->fd;
}


/**
 * 关闭文件描述符
 */
int ppc_close(int fd)
{
	if(fd){
		del_info_for_ppc_fd_list(fd);
		return close(fd);
	}
	return 0;
}

/*
 *通过文件描述符读取文件
 */
ssize_t ppc_read(int fd, void *buf, size_t count)
{
	return handle_read_task(fd, count, buf);
}

/*
 *通过文件描述符写文件
 */
ssize_t ppc_write(int fd, void *buf, size_t count)
{
	return handle_write_task(fd, count, buf);
}


