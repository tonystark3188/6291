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
#include "ppc_fd_list.h"
#include "ppc_token_list.h"
#include "ppc_deal_path.h"


/**
  * function:获取库版本信息
  * ppc_version: 库版本信息
  * return:0 sucess,!0 failed
  */
int ppc_get_pkg_version(pkg_version *pkg_version_info)
{
	if(pkg_version_info == NULL){
		return FAIL;
	}

	strcpy(pkg_version_info->pkg_name, PKG_NAME);
	strcpy(pkg_version_info->pkg_version, PKG_VERSION);

	return SUCCESS;
}


/**
  * function:ppc lib初始化
  * return:0 sucess,!0 failed
  */
int ppc_initialise()
{
	int res = 0;
	res = init_ppc_fd_list();
	if(0 != res){
		return FAIL;
	}
	
	res = init_ppc_token_list();
	if(0 != res){
		return FAIL;
	}

	res = InitLinklist();
	if(res)
	{
		DMCLOG_E("InitLinklist failed...");
		return FAIL;
	}
	
	return SUCCESS;
}


/**
  * function:ppc lib反初始化
  * return:0 sucess,!0 failed
  */
int ppc_uninitialise()
{
	int res = 0;
	res = free_ppc_fd_list();
	if(0 != res){
		return FAIL;
	}	

	res = free_ppc_token_list();
	if(0 != res){
		return FAIL;
	}

	ReleaseLinklist();
	
	return SUCCESS;
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
ERROR_CODE_PPC ppc_login(char* username, char* password, _int64_t *token)
{
    int res = 0;
	res = handle_login_task(username,password,token);
    if(res != 0){
        return USERNAME_NOT_FOUND;
    }

	token_info *login_token_info = (token_info *)calloc(1, sizeof(token_info));
	if(login_token_info == NULL){
		return FAIL;
	}

	login_token_info->token = *token;
	login_token_info->work_dir = strdup("/");
	res = add_info_for_ppc_token_list(login_token_info);
	if(0 != res){
		return FAIL;
	}	

	return SUCCESS;
}

/**
 * function:用户退出
 * param:
 * token 用户标识
 * return:0 sucess,!0 failed
 */
ERROR_CODE_PPC ppc_logout(_int64_t token)
{
    int res = 0;
	res = handle_logout_task(token);
    if(res != 0){
        return USERNAME_NOT_FOUND;
    }
	
	res = del_info_for_ppc_token_list(token);
	if(0 != res){
		return FAIL;
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
FILE *ppc_fopen(const char *path, const char *mode, _int64_t token)
{
	int ret = 0;
	char *full_path = NULL;
	
	if(path == NULL || mode == NULL){
		DMCLOG_E("para is null");
		return NULL;
	}

	ret = create_new_full_path(path, &full_path, token);
	if(ret != 0 || full_path == NULL){
		DMCLOG_D("create full path fail");
		return NULL;
	}
	
	PFILE *file_info = (PFILE *)calloc(1,sizeof(PFILE));
	if(file_info == NULL){
		DMCLOG_E("malloc error");
		safe_free(full_path);
		return NULL;
	}
	
    file_info->token = token;
	strcpy(file_info->mode,mode);
	file_info->offset = 0;
	file_info->srcPath = full_path;
	ret = handle_fopen_task(file_info);
	if(ret != 0){
		DMCLOG_E("handle fopen error");
		safe_free(file_info->srcPath);
		safe_free(file_info);
		return NULL;
	}

	fd_info *open_fd_info = (fd_info *)calloc(1, sizeof(fd_info));
	if(open_fd_info == NULL){
		DMCLOG_E("malloc open_fd_info fail");
		safe_free(file_info->srcPath);
		safe_free(file_info);
		return NULL;
	}

	open_fd_info->socket_fd = file_info->fd;
	open_fd_info->path = strdup(file_info->srcPath);
	open_fd_info->offset = file_info->offset;
	open_fd_info->file_len = file_info->length;
	open_fd_info->type = fd_type_fopen;
	memcpy(open_fd_info->f_mode, file_info->mode, strlen(file_info->mode));
	ret = add_info_for_ppc_fd_list(open_fd_info);
	if(ret){
		DMCLOG_E("add_info_for_ppc_fd_list fail");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		safe_free(file_info->srcPath);
		safe_free(file_info);
		return NULL;
	}

	int v_fd = open_fd_info->v_fd;
	//FILE *fp = (FILE *)calloc(1, sizeof(FILE));
	FILE *fp = (FILE *)v_fd;

	if(fp == NULL){
		del_info_for_ppc_fd_list(v_fd);
		safe_free(file_info->srcPath);
		safe_free(file_info);
	}

	//fp->_fileno = open_fd_info->v_fd;
	safe_free(file_info->srcPath);
	safe_free(file_info);

	return fp;
}

/**
 * 文件读取方法
 */
size_t ppc_fread(void *buffer, size_t size, size_t count, FILE *fp)
{
	int ret = 0;
	int v_fd = (int)fp;
	int socket_fd = get_socket_fd_from_ppc_fd_list(v_fd);
	if(socket_fd < 0){
		return FAIL;
	}

	ssize_t read_len = handle_fread_task(socket_fd, count, buffer);
	if(read_len > 0){
		ret = inc_offset_for_ppc_fd_list(v_fd, (off_t)read_len);
		if(ret){
			return FAIL;
		}
	}

	return read_len;
}

/**
 * 文件写方法
 */
size_t ppc_fwrite(const void* buffer, size_t size, size_t count, FILE *fp)
{
	int ret = 0;
	int v_fd = (int)fp;
	int socket_fd = get_socket_fd_from_ppc_fd_list(v_fd);
	if(socket_fd < 0){
		return FAIL;
	}
	
	ssize_t write_len = handle_fwrite_task(socket_fd,count,buffer);
	if(write_len > 0){
		ret = inc_offset_for_ppc_fd_list(v_fd, (off_t)write_len);
		if(ret){
			return FAIL;
		}
	}

	return write_len;
}

/**
 * 文件指针偏移
 * fromwhere（偏移起始位置：文件头0(SEEK_SET)，当前位置1(SEEK_CUR)，文件尾2(SEEK_END)）为基准，偏移offset（指针偏移量）个字节的位置。
 */
off_t ppc_fseek(FILE *fp, off_t offset, int fromwhere, _int64_t token)
{
	DMCLOG_D("start ppc_fseek");
	int ret = 0;
	int v_fd = 0;
	off_t seek_offset = 0;
	fd_info *open_fd_info = NULL;
	
	if(fp == NULL){
		DMCLOG_E("para is null");
		return FAIL;
	}

	v_fd = (int)fp;
	
	ret = get_info_from_ppc_fd_list(v_fd, &open_fd_info);
	if(ret || (open_fd_info == NULL) || (open_fd_info->path == NULL)){
		DMCLOG_E("get_info_from_ppc_fd_list fail");
		return FAIL;
	}
	DMCLOG_D("open_fd_info->path: %s", open_fd_info->path);
	
	PFILE *file_info = (PFILE *)calloc(1,sizeof(PFILE));
	if(file_info == NULL){
		DMCLOG_E("calloc file_info error");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		return FAIL;
	}

	if(fromwhere == SEEK_SET){
		seek_offset = offset;
	}
	else if(fromwhere == SEEK_CUR){
		seek_offset = open_fd_info->offset + offset;
	}
	else if(fromwhere == SEEK_END){
		seek_offset = open_fd_info->file_len + offset;
	}
	else{
		DMCLOG_E("argument error");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		safe_free(file_info);
		return FAIL;
	}

	if(seek_offset > open_fd_info->file_len){
		DMCLOG_E("seek_offset(%lld) is longer than file length(%lld)!!!", seek_offset, open_fd_info->file_len);
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		safe_free(file_info);
		return FAIL;
	}

	file_info->offset = seek_offset;
	file_info->srcPath = open_fd_info->path;
	memcpy(file_info->mode, open_fd_info->f_mode, strlen(open_fd_info->f_mode));
    file_info->token = token;
	ret = handle_fopen_task(file_info);
	if(ret != 0){
		DMCLOG_E("handle open error");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		safe_free(file_info);
		return FAIL;
	}

	//保存旧的fd
	int socket_fd_old = open_fd_info->socket_fd;
	
	ret = ch_socket_fd_for_ppc_fd_list(v_fd, file_info->fd);
	if(ret){
		DMCLOG_E("change socket fail");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		safe_free(file_info);
		return FAIL;
	}
	//不能调用ppc_close
	close(socket_fd_old);

	ret = set_offset_for_ppc_fd_list(v_fd, file_info->offset);
	if(ret){
		DMCLOG_E("set_offset_for_ppc_fd_list fail");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		safe_free(file_info);
		return FAIL;
	}	

	safe_free(open_fd_info->path);
	safe_free(open_fd_info);
	safe_free(file_info);

	return seek_offset;	
}

off_t ppc_ftell(FILE *fp, _int64_t token)
{
	int ret = 0;
	int v_fd = 0;
	off_t offset = 0;
	fd_info *open_fd_info = NULL;
	
	if(fp == NULL){
		DMCLOG_E("para is null");
		return FAIL;
	}

	v_fd = (int)fp;
	
	ret = get_info_from_ppc_fd_list(v_fd, &open_fd_info);
	if(ret || (open_fd_info == NULL) || (open_fd_info->path == NULL)){
		DMCLOG_E("get_info_from_ppc_fd_list fail");
		return FAIL;
	}
	DMCLOG_D("open_fd_info->path: %s", open_fd_info->path);

	offset = open_fd_info->offset;

	safe_free(open_fd_info->path);
	safe_free(open_fd_info);

	return offset;
}

/**
 * 关闭文件指针
 */
int ppc_fclose(FILE *fp)
{
	if(fp == NULL){
		DMCLOG_E("para is null");
		return FAIL;
	}
	int v_fd = (int)fp;
	if(v_fd > 0){
		int socket_fd = get_socket_fd_from_ppc_fd_list(v_fd);
		if(socket_fd > 0){
			close(socket_fd);
			if(del_info_for_ppc_fd_list(v_fd) < 0){
				return FAIL;
			}
			else{
				//safe_free(fp);
				return SUCCESS;
			}	
		}
		else{
			return FAIL;
		}
	}
	else{
		return FAIL;
	}
}

/*
 *打开目录
 */
DIR *ppc_opendir(const char *path, _int64_t token)
{
	int ret = 0;
	struct dirent **p_data;
	char *full_path = NULL;
	int file_count = 0;
	
	if(path == NULL){
		DMCLOG_E("para is null");
		return NULL;
	}

	ret = create_new_full_path(path, &full_path, token);
	if(ret != 0 || full_path == NULL){
		DMCLOG_D("create full path fail");
		return NULL;
	}

    file_count = handle_opendir_task(full_path, &p_data, token);
	if(file_count < 0 || p_data == NULL){
		safe_free(full_path);
        return NULL;
    }	

	fd_info *open_dir_info = (fd_info *)calloc(1, sizeof(fd_info));
	if(open_dir_info == NULL){
		safe_free(full_path);
		DMCLOG_E("malloc open_fd_info fail");
		return NULL;
	}

	open_dir_info->path = strdup(full_path);
	open_dir_info->p_data = p_data;
	open_dir_info->offset = 0;
	open_dir_info->type = fd_type_opendir;
	open_dir_info->file_count = file_count;
	ret = add_info_for_ppc_fd_list(open_dir_info);
	if(ret){
		DMCLOG_E("add_info_for_ppc_fd_list fail");
		safe_free(full_path);
		safe_free(open_dir_info->path);
		safe_free(open_dir_info);
		return NULL;
	}	

	//PPC_DIR *p_dir = (PPC_DIR *)calloc(1, sizeof(PPC_DIR));
	int v_fd = open_dir_info->v_fd;
	DIR *p_dir = (DIR *)v_fd;
	if(p_dir == NULL){
		safe_free(full_path);
		del_info_for_ppc_fd_list(v_fd);
	}

	//p_dir->__entry_ptr = v_fd;	
	safe_free(full_path);
    return p_dir;
}

/*
 *根据文件描述打开目录
 */
DIR *ppc_fdopendir(int fd, _int64_t token)
{
	int ret = 0;
	char *full_path = NULL;
	fd_info *open_fd_info = NULL;
	struct dirent **p_data = NULL;
	int file_count = 0;

	ret = get_info_from_ppc_fd_list(fd, &open_fd_info);
	if(ret || (open_fd_info == NULL) || (open_fd_info->path == NULL)){
		DMCLOG_E("get_info_from_ppc_fd_list fail");
		return NULL;
	}
	DMCLOG_D("open_fd_info->path: %s", open_fd_info->path);
	full_path = strdup(open_fd_info->path);
	safe_free(open_fd_info->path);
	safe_free(open_fd_info);

	file_count = handle_opendir_task(full_path, &p_data, token);
    if(file_count < 0 || p_data == NULL){
		safe_free(full_path);
        return NULL;
    }

	fd_info *open_dir_info = (fd_info *)calloc(1, sizeof(fd_info));
	if(open_dir_info == NULL){
		safe_free(full_path);
		DMCLOG_E("malloc open_fd_info fail");
		return NULL;
	}

	open_dir_info->path = strdup(full_path);
	open_dir_info->p_data = p_data;
	open_dir_info->offset = 0;
	open_dir_info->type = fd_type_opendir;
	open_dir_info->file_count = file_count;
	ret = add_info_for_ppc_fd_list(open_dir_info);
	if(ret){
		DMCLOG_E("add_info_for_ppc_fd_list fail");
		safe_free(full_path);
		safe_free(open_dir_info->path);
		safe_free(open_dir_info);
		return NULL;
	}	

	//PPC_DIR *p_dir = (PPC_DIR *)calloc(1,sizeof(PPC_DIR));
	int v_fd = open_dir_info->v_fd;
	DIR *p_dir = (DIR *)v_fd;
	if(p_dir == NULL){
		safe_free(full_path);
		del_info_for_ppc_fd_list(v_fd);
	}

	//p_dir->__entry_ptr = v_fd;	
	safe_free(full_path);
    return p_dir;
}

/*
 *读取目录
 */
struct dirent *ppc_readdir(DIR *dp, _int64_t token)
{
	int v_fd = 0;
	int ret = 0;
	fd_info *open_fd_info = NULL;

	if(dp == NULL){
		return NULL;
	}

	//PPC_DIR *p_dir = (PPC_DIR *)dp;
	v_fd = (int)dp;

	ret = get_info_from_ppc_fd_list(v_fd, &open_fd_info);
	if(ret || (open_fd_info == NULL) || (open_fd_info->p_data == NULL)){
		DMCLOG_E("get_info_from_ppc_fd_list fail(ret %d)", ret);
		return NULL;
	}

	DMCLOG_D("open_fd_info->offset: %lld, open_fd_info->file_count: %d", open_fd_info->offset, open_fd_info->file_count);
	if(open_fd_info->offset >= open_fd_info->file_count){
		DMCLOG_D("read end");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		return NULL;	
	}

	struct dirent *dirp = NULL;
    dirp = handle_readdir_task(open_fd_info->p_data, open_fd_info->offset);
	if(dirp == NULL){
		DMCLOG_D("handle_readdir_task fail");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		return NULL;
	}

	ret = inc_offset_for_ppc_fd_list(v_fd, 1);
	if(ret){
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		return NULL;
	}

	safe_free(open_fd_info->path);
	safe_free(open_fd_info);
	
	return dirp;	
}

/*
 *重置目录指针
 */
void ppc_rewinddir(DIR *dp, _int64_t token)
{
	int v_fd = 0;
	int ret = 0;

	if(dp == NULL){
		return ;
	}

	//PPC_DIR *p_dir = (PPC_DIR *)dp;
	v_fd = (int)dp;

	ret = set_offset_for_ppc_fd_list(v_fd, 0);
	if(ret){
		DMCLOG_E("set_offset_for_ppc_fd_list fail");
		return ;
	}

	DMCLOG_E("set_offset_for_ppc_fd_list success");
	return ;
}

/*
 *关闭目录
 */
int ppc_closedir(DIR *dp, _int64_t token)
{
	int v_fd = 0;
	int ret = 0;

	if(dp == NULL){
		return FAIL;
	}

	//PPC_DIR *p_dir = (PPC_DIR *)dp;
	v_fd = (int)dp;

	del_info_for_ppc_fd_list(v_fd);
	//safe_free(dp);
	
	return SUCCESS;
}

/*
 *获取目录位置
 */
off_t ppc_telldir(DIR *dp, _int64_t token)
{
	int v_fd = 0;
	int ret = 0;

	if(dp == NULL){
		return FAIL;
	}

	//PPC_DIR *p_dir = (PPC_DIR *)dp;
	v_fd = (int)dp;

	return get_offset_fd_from_ppc_fd_list(v_fd);
}

/*
 *跳到指定目录位置
 */
void ppc_seekdir(DIR *dp, off_t loc, _int64_t token)
{
	int v_fd = 0;
	int ret = 0;

	if(dp == NULL){
		return ;
	}

	//PPC_DIR *p_dir = (PPC_DIR *)dp;
	v_fd = (int)dp;

	ret = set_offset_for_ppc_fd_list(v_fd, loc);
	if(ret){
		DMCLOG_E("set_offset_for_ppc_fd_list fail");
		return ;
	}

	DMCLOG_E("set_offset_for_ppc_fd_list success");
	return ;
}

/*
 *创建目录
 */
int ppc_mkdir(const char *dirname, mode_t mode, _int64_t token)
{
	int ret = 0;
	char *full_path = NULL;
	
	if(dirname == NULL){
		DMCLOG_E("para is null");
		return FAIL;
	}

	ret = create_new_full_path(dirname, &full_path, token);
	if(ret != 0 || full_path == NULL){
		DMCLOG_D("create full path fail");
		return FAIL;
	}	

	// TODO: 缺少mode参数
	ret = handle_mkdir_task(full_path, mode, token);
	if(ret != 0){
		DMCLOG_D("handle_mkdir_task fail");
		safe_free(full_path);
		return FAIL;
	}

	safe_free(full_path);
	return SUCCESS;
}

/*
 *删除目录
 */
int ppc_rmdir(const char *pathname, _int64_t token)
{
	int ret = 0;
	char *full_path = NULL;
	
	if(pathname == NULL){
		DMCLOG_E("para is null");
		return FAIL;
	}

	ret = create_new_full_path(pathname, &full_path, token);
	if(ret != 0 || full_path == NULL){
		DMCLOG_D("create full path fail");
		return FAIL;
	}	
	
	//文件夹内有文件时，rmdir应返回失败
	ret = handle_rmdir_task(full_path, token);
	if(ret != 0){
		DMCLOG_D("handle_rmdir_task fail");
		safe_free(full_path);
		return FAIL;
	}

	safe_free(full_path);
	return SUCCESS;
}

/*
 *通过文件路径获取文件信息
 */
int ppc_stat(const char *file_name, struct stat *buf, _int64_t token)
{
	int ret = 0;
	char *full_path = NULL;
	
	if(file_name == NULL){
		DMCLOG_E("para is null");
		return FAIL;
	}

	ret = create_new_full_path(file_name, &full_path, token);
	if(ret != 0 || full_path == NULL){
		DMCLOG_D("create full path fail");
		return FAIL;
	}

    ret = handle_stat_task(full_path, buf, token);
    if(ret != SUCCESS){
        DMCLOG_E("stat %s error", full_path);
		safe_free(full_path);
        return FAIL;
    }
	safe_free(full_path);
    return SUCCESS;
}


/*
  *通过文件描述符获取文件信息
  */
int ppc_fstat(int fildes, struct stat *buf, _int64_t token)
{
	int ret = 0;
	fd_info *open_fd_info = NULL;
	ret = get_info_from_ppc_fd_list(fildes, &open_fd_info);
	if(ret || (open_fd_info == NULL) || (open_fd_info->path == NULL)){
		DMCLOG_E("get_info_from_ppc_fd_list fail");
		return FAIL;
	}
	DMCLOG_D("open_fd_info->path: %s", open_fd_info->path);

	ret = handle_stat_task(open_fd_info->path, buf, token);
    if(ret != SUCCESS){
		DMCLOG_E("fstat %s(%d) error", open_fd_info->path, fildes);
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
        return FAIL;
    }

	safe_free(open_fd_info->path);
	safe_free(open_fd_info);
    return SUCCESS;
}

/*
  *通过文件路径获取文件信息(包括符号链接文件)
  */
int ppc_lstat(const char *path, struct stat *buf, _int64_t token)
{
	int ret = 0;
	char *full_path = NULL;
	
	if(path == NULL){
		DMCLOG_E("para is null");
		return FAIL;
	}

	ret = create_new_full_path(path, &full_path, token);
	if(ret != 0 || full_path == NULL){
		DMCLOG_D("create full path fail");
		return FAIL;
	}
	
	ret = handle_stat_task(full_path, buf, token);
    if(ret != SUCCESS){
        DMCLOG_E("stat %s error", full_path);
		safe_free(full_path);
        return FAIL;
    }

	safe_free(full_path);
    return SUCCESS;
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
int ppc_open(const char *pathname, int flags, mode_t mode, _int64_t token)
{
	int ret = 0;
	char *full_path = NULL;
	
	if(pathname == NULL){
		DMCLOG_E("para is null");
		return FAIL;
	}

	ret = create_new_full_path(pathname, &full_path, token);
	if(ret != 0 || full_path == NULL){
		DMCLOG_D("create full path fail");
		return FAIL;
	}
	
	FFILE *file_info = (FFILE *)calloc(1,sizeof(FFILE));
	if(file_info == NULL){
		DMCLOG_E("calloc file_info error");
		return FAIL;
	}

	//assert(file_info != NULL);
	file_info->token = token;
	file_info->offset = 0;
	file_info->srcPath = full_path;
	file_info->flag = flags;
	file_info->mode = mode;
	ret = handle_open_task(file_info);
	if(ret != 0){
		DMCLOG_E("handle open error");
		safe_free(file_info->srcPath);
		safe_free(file_info);
		return FAIL;
	}

	fd_info *open_fd_info = (fd_info *)calloc(1, sizeof(fd_info));
	if(open_fd_info == NULL){
		DMCLOG_E("malloc open_fd_info fail");
		safe_free(file_info->srcPath);
		safe_free(file_info);
		return FAIL;
	}

	open_fd_info->socket_fd = file_info->fd;
	open_fd_info->path = strdup(full_path);
	open_fd_info->offset = file_info->offset;
	open_fd_info->file_len = file_info->length;
	open_fd_info->mode = file_info->mode;
	open_fd_info->flag = file_info->flag;
	open_fd_info->type = fd_type_open;
	ret = add_info_for_ppc_fd_list(open_fd_info);
	if(ret){
		DMCLOG_E("add_info_for_ppc_fd_list fail");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		safe_free(file_info->srcPath);
		safe_free(file_info);
		return FAIL;
	}

	safe_free(file_info->srcPath);
	safe_free(file_info);
	return open_fd_info->v_fd;
}


/**
 * 关闭文件描述符
 */
int ppc_close(int fd)
{
	if(fd > 0){
		int socket_fd = get_socket_fd_from_ppc_fd_list(fd);
		if(socket_fd > 0){
			close(socket_fd);
			if(del_info_for_ppc_fd_list(fd) < 0)
				return FAIL;
			else
				return SUCCESS;
		}
		else{
			return FAIL;
		}
	}
	else{
		return FAIL;
	}
}

/*
 *通过文件描述符读取文件
 */
ssize_t ppc_read(int fd, void *buf, size_t count)
{
	int ret = 0;
	int socket_fd = get_socket_fd_from_ppc_fd_list(fd);
	if(socket_fd < 0){
		return FAIL;
	}

	ssize_t read_len = handle_read_task(socket_fd, count, buf);
	if(read_len > 0){
		ret = inc_offset_for_ppc_fd_list(fd, (off_t)read_len);
		if(ret){
			return FAIL;
		}
	}

	return read_len;
}

/*
 *通过文件描述符写文件
 */
ssize_t ppc_write(int fd, void *buf, size_t count)
{
	int ret = 0;
	int socket_fd = get_socket_fd_from_ppc_fd_list(fd);
	if(socket_fd < 0){
		return FAIL;
	}
	
	ssize_t write_len = handle_write_task(socket_fd, count, buf);
	if(write_len > 0){
		ret = inc_offset_for_ppc_fd_list(fd, (off_t)write_len);
		if(ret){
			return FAIL;
		}
	}

	return write_len;
}

/*
 *跳转到指定文件长度位置
 */
off_t ppc_lseek(int fd, off_t offset ,int whence, _int64_t token)
{
	int ret = 0;
	off_t seek_offset = 0;
	fd_info *open_fd_info = NULL;
	ret = get_info_from_ppc_fd_list(fd, &open_fd_info);
	if(ret || (open_fd_info == NULL) || (open_fd_info->path == NULL)){
		DMCLOG_E("get_info_from_ppc_fd_list fail");
		return FAIL;
	}
	DMCLOG_D("open_fd_info->path: %s", open_fd_info->path);
	
	FFILE *file_info = (FFILE *)calloc(1,sizeof(FFILE));
	if(file_info == NULL){
		DMCLOG_E("calloc file_info error");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		return FAIL;
	}

	if(whence == SEEK_SET){
		seek_offset = offset;
	}
	else if(whence == SEEK_CUR){
		seek_offset = open_fd_info->offset + offset;
	}
	else if(whence == SEEK_END){
		seek_offset = open_fd_info->file_len + offset;
	}
	else{
		DMCLOG_E("argument error");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		safe_free(file_info);
		return FAIL;
	}

	// TODO: open为write时，file_len为0
	if(!(open_fd_info->flag & (O_WRONLY | O_CREAT)) && (seek_offset > open_fd_info->file_len)){
		DMCLOG_E("seek_offset(%lld) is longer than file length(%lld)!!!", seek_offset, open_fd_info->file_len);
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		safe_free(file_info);
		return FAIL;
	}

	file_info->offset = seek_offset;
	file_info->srcPath = open_fd_info->path;
	file_info->flag = open_fd_info->flag;
	file_info->mode = open_fd_info->mode;
    file_info->token = token;
	ret = handle_open_task(file_info);
	if(ret != 0){
		DMCLOG_E("handle open error");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		safe_free(file_info);
		return FAIL;
	}

	//保存旧的fd
	int socket_fd_old = open_fd_info->socket_fd;
	
	ret = ch_socket_fd_for_ppc_fd_list(fd, file_info->fd);
	if(ret){
		DMCLOG_E("change socket fail");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		safe_free(file_info);
		return FAIL;
	}
	//不能调用ppc_close
	close(socket_fd_old);

	ret = set_offset_for_ppc_fd_list(fd, file_info->offset);
	if(ret){
		DMCLOG_E("set_offset_for_ppc_fd_list fail");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		safe_free(file_info);
		return FAIL;
	}

	safe_free(open_fd_info->path);
	safe_free(open_fd_info);
	safe_free(file_info);

	return seek_offset;
}

/*
 *通过带偏移量地原子读取文件
 */
ssize_t ppc_pread(int fd, void *buf, size_t count, off_t offset, _int64_t token)
{
	int ret = 0;
	fd_info *open_fd_info = NULL;
	ret = get_info_from_ppc_fd_list(fd, &open_fd_info);
	if(ret || (open_fd_info == NULL) || (open_fd_info->path == NULL)){
		DMCLOG_E("get_info_from_ppc_fd_list fail");
		return FAIL;
	}
	DMCLOG_D("open_fd_info->path: %s", open_fd_info->path);
	
	FFILE *file_info = (FFILE *)calloc(1,sizeof(FFILE));
	if(file_info == NULL){
		DMCLOG_E("calloc file_info error");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		return FAIL;
	}

	if(offset > open_fd_info->file_len){
		DMCLOG_E("offset(%lld) is longer than file length(%lld)!!!", offset, open_fd_info->file_len);
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		safe_free(file_info);
		return FAIL;
	}

	file_info->offset = offset;
	file_info->srcPath = open_fd_info->path;
	file_info->flag = open_fd_info->flag;
	file_info->mode = open_fd_info->mode;
    file_info->token = token;
	ret = handle_open_task(file_info);
	if(ret != 0){
		DMCLOG_E("handle open error");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		safe_free(file_info);
		return FAIL;
	}

	//保存旧的fd
	int socket_fd_old = open_fd_info->socket_fd;
	
	ret = ch_socket_fd_for_ppc_fd_list(fd, file_info->fd);
	if(ret){
		DMCLOG_E("change socket fail");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		safe_free(file_info);
		return FAIL;
	}

	//不能调用ppc_close
	close(socket_fd_old);

	ssize_t read_len = handle_read_task(file_info->fd, count, buf);
	if(read_len > 0){
		set_offset_for_ppc_fd_list(fd, read_len + offset);
	}

	safe_free(open_fd_info->path);
	safe_free(open_fd_info);
	safe_free(file_info);
	
	return read_len;
}

/*
 *通过带偏移量地原子写文件
 */
ssize_t ppc_pwrite(int fd, void *buf, size_t count, off_t offset, _int64_t token)
{
	int ret = 0;
	fd_info *open_fd_info = NULL;
	ret = get_info_from_ppc_fd_list(fd, &open_fd_info);
	if(ret || (open_fd_info == NULL) || (open_fd_info->path == NULL)){
		DMCLOG_E("get_info_from_ppc_fd_list fail");
		return FAIL;
	}
	DMCLOG_D("open_fd_info->path: %s", open_fd_info->path);
	
	FFILE *file_info = (FFILE *)calloc(1,sizeof(FFILE));
	if(file_info == NULL){
		DMCLOG_E("calloc file_info error");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		return FAIL;
	}

	#if 0
	if(offset > open_fd_info->file_len){
		DMCLOG_E("offset(%lld) is longer than file length(%lld)!!!", offset, open_fd_info->file_len);
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		safe_free(file_info);
		return FAIL;
	}
	#endif

	file_info->offset = offset;
	file_info->srcPath = open_fd_info->path;
	file_info->flag = open_fd_info->flag;
	file_info->mode = open_fd_info->mode;
    file_info->token = token;
	ret = handle_open_task(file_info);
	if(ret != 0){
		DMCLOG_E("handle open error");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		safe_free(file_info);
		return FAIL;
	}

	//保存旧的fd
	int socket_fd_old = open_fd_info->socket_fd;
	
	ret = ch_socket_fd_for_ppc_fd_list(fd, file_info->fd);
	if(ret){
		DMCLOG_E("change socket fail");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		safe_free(file_info);
		return FAIL;
	}

	//不能调用ppc_close
	close(socket_fd_old);
	
	ssize_t write_len = handle_write_task(file_info->fd, count, buf);
	if(write_len > 0){
		set_offset_for_ppc_fd_list(fd, (off_t)write_len + offset);
	}

	safe_free(open_fd_info->path);
	safe_free(open_fd_info);
	safe_free(file_info);
	return write_len;
}

/*
  *聚集写,即收集内存中分散的若干缓冲区中的数据写至文件的连续区域中
  */
ssize_t ppc_writev(int fd, const struct iovec *iov, int cnt, _int64_t token)
{
	int ret = 0;
	int num = 0;
	ssize_t write_len = 0;
	ssize_t write_all = 0;
	if(iov == NULL || cnt < 0){
		DMCLOG_D("invalid argument");
		return FAIL;
	}

	int socket_fd = get_socket_fd_from_ppc_fd_list(fd);
	if(socket_fd < 0){
		return FAIL;
	}
	
	for(num = 0; num < cnt; num++){	
		write_len = handle_write_task(socket_fd, iov[num].iov_len, (char *)(iov[num].iov_base));
		if(write_len > 0){
			ret = inc_offset_for_ppc_fd_list(fd, (off_t)write_len);
			if(ret){
				return FAIL;
			}
		}
		else{
			DMCLOG_E("handle_write_task fail");
			return FAIL;
		}
		write_all += write_len;
	}

	return write_all;
}

/*
  *重命名操作
  */
int ppc_rename(const char *oldname, const char *newname, _int64_t token)
{
	int ret = 0;
	char *old_full_path = NULL;
	char *new_full_path = NULL;
	
	if(oldname == NULL || newname == NULL){
		DMCLOG_E("para is null");
		return FAIL;
	}

	ret = create_new_full_path(oldname, &old_full_path, token);
	if(ret != 0 || old_full_path == NULL){
		DMCLOG_D("create full path fail");
		return FAIL;
	}

	ret = create_new_full_path(newname, &new_full_path, token);
	if(ret != 0 || new_full_path == NULL){
		DMCLOG_D("create full path fail");
		safe_free(old_full_path);
		return FAIL;
	}
	
	ret = handle_rename_task(old_full_path, new_full_path, token);
	if(ret != 0){
		DMCLOG_E("handle_rename_task fail");
		safe_free(old_full_path);
		safe_free(new_full_path);
		return FAIL;
	}

	safe_free(old_full_path);
	safe_free(new_full_path);
	return SUCCESS;
}

/*
  *删除文件链接及文件
  */
int ppc_unlink(const char *pathname, _int64_t token)
{
	int ret = 0;
	char *full_path = NULL;

	if(pathname == NULL){
		DMCLOG_E("para is null");
		return FAIL;
	}

	ret = create_new_full_path(pathname, &full_path, token);
	if(ret != 0 || full_path == NULL){
		DMCLOG_D("create full path fail");
		return FAIL;
	}

	ret = handle_unlink_task(full_path, token);
	if(ret != 0){
		DMCLOG_E("handle_unlink_task fail");
		safe_free(full_path);
		return FAIL;
	}

	safe_free(full_path);
	return SUCCESS;
}

/*
  function: 修改文件访问时间和修改时间的时间戳
  dirfd pathname:
  1)如果pathname是相对路径，则是相对于dirfd的路径，不是相对于当前进程工作空间;
  2)如果pathname是相对路径，且dirfd为AT_FDCWD，则是相对于当前进程工作空间;
  3)如果pathname是绝对路径，则dirfd会被忽略;
  flags:
  1)AT_SYMLINK_NOFOLLOW: 如果path指向一个符号链接，则返回该链接信息;
  */
int ppc_utimensat(int dirfd, const char *pathname, const struct timespec times[2], int flags, _int64_t token)
{
	char *real_path = NULL;
	struct timespec real_times[2];
	int ret = 0;
	fd_info *open_fd_info = NULL;

	if(pathname != NULL && *pathname != '/'){
		if(dirfd == AT_FDCWD){
			ret = create_new_full_path(pathname, &real_path, token);
			if(ret != 0 || real_path == NULL){
				DMCLOG_D("create real path fail");
				ret = FAIL;
				goto EXIT2;
			}	
		}
		else{
			ret = get_info_from_ppc_fd_list(dirfd, &open_fd_info);
			if(ret || (open_fd_info == NULL) || (open_fd_info->path == NULL)){
				DMCLOG_E("get_info_from_ppc_fd_list fail");
				ret = FAIL;
				goto EXIT1;
			}
			DMCLOG_D("open_fd_info->path: %s", open_fd_info->path);
			real_path = (char *)calloc(1, strlen(open_fd_info->path) + strlen(pathname) + 1);
			if(real_path == NULL){
				DMCLOG_E("mclloc fail");
				ret = FAIL;
				goto EXIT2;
			}
			memcpy(real_path, open_fd_info->path, strlen(open_fd_info->path));
			strcat(real_path, pathname);
		}
	}
	else if(pathname != NULL && *pathname == '/'){
		real_path = (char *)calloc(1, strlen(pathname) + 1);
		if(real_path == NULL){
			DMCLOG_E("mclloc fail");
			ret = FAIL;
			goto EXIT2;
		}
		memcpy(real_path, pathname, strlen(pathname));
	}
	else{
		DMCLOG_E("invalid argument");
		ret = FAIL;
		goto EXIT1;
	}
	DMCLOG_D("real_path: %s", real_path);

	if(times == NULL){
		struct timespec ts_now;
        ret = clock_gettime(CLOCK_REALTIME, &ts_now);
		DMCLOG_D("ret: %d, CLOCK_REALTIME: %lu, %lu\n", ret, ts_now.tv_sec, ts_now.tv_nsec);
		memcpy(&real_times[0], &ts_now, sizeof(struct timespec));
		memcpy(&real_times[1], &ts_now, sizeof(struct timespec));
	}
	else{
		if(times[0].tv_nsec == UTIME_NOW){
			struct timespec ts_now;
	        clock_gettime(CLOCK_REALTIME, &ts_now);
			memcpy(&real_times[0], &ts_now, sizeof(struct timespec));
		}
		#if 0
		else if(times[0].tv_nsec == UTIME_OMIT){
			real_times[0].tv_nsec = 0;
			real_times[0].tv_sec = 0;
		}
		#endif
		else{
			memcpy(&real_times[0], &times[0], sizeof(struct timespec));
		}

		if(times[1].tv_nsec == UTIME_NOW){
			struct timespec ts_now;
	        clock_gettime(CLOCK_REALTIME, &ts_now);
			memcpy(&real_times[1], &ts_now, sizeof(struct timespec));
		}
		#if 0
		else if(times[1].tv_nsec == UTIME_OMIT){
			real_times[1].tv_nsec = 0;
			real_times[1].tv_sec = 0;
		}
		#endif
		else{
			memcpy(&real_times[1], &times[1], sizeof(struct timespec));
		}
	}
	
	ret = handle_utimensat_task(real_path, real_times, flags, token);
	if(ret){
		DMCLOG_E("handle_utimensat_task fail");
		ret = FAIL;
		goto EXIT3;
	}
	ret = SUCCESS;

EXIT3:
	safe_free(real_path);
EXIT2:
	if(open_fd_info != NULL){
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
	}
EXIT1:
	return ret;	
}


/*
  *更改当前工作目录
  */
int ppc_chdir(const char *path, _int64_t token)
{
	int ret = 0;
	struct stat st;
	char *full_path = NULL;
	if(path == NULL){
		return FAIL;
	}				

	ret = create_new_full_path(path, &full_path, token);
	if(ret < 0){
		return FAIL;
	}
	else if(1 == ret){
		DMCLOG_D("no need change");
		return SUCCESS;
	}

	DMCLOG_D("full_path: %s", full_path);
	ret = handle_stat_task(full_path, &st, token);
	if(ret == 0 && st.st_mode == S_IFDIR){
		DMCLOG_D("check stat success");
	}
	else{
		DMCLOG_E("fial (ret %s, st.st_mode %d)", full_path, st.st_mode);
		safe_free(full_path);
		return FAIL;	
	}

	ret = update_work_dir_for_ppc_token_list(full_path, token);
	if(ret){
		DMCLOG_E("update work dir fail");
		safe_free(full_path);
		return FAIL;	
	}

	safe_free(full_path);
	return SUCCESS;	
}

/*
  *获取当前工作目录
  */
char *ppc_getwd(char *buf, _int64_t token)
{
	char *work_dir = NULL;
	if(buf == NULL){
		return NULL;
	}

	work_dir = get_work_dir_from_ppc_token_list(token);
	if(work_dir == NULL){
		return NULL;
	}
	else{
		memcpy(buf, work_dir, strlen(work_dir));
		safe_free(work_dir);
		return buf;
	}
}

/*
  *改变文件属性
  */
int ppc_chmod(const char *pathname, mode_t mode, _int64_t token)
{
	return SUCCESS;
}

/*
  *改变文件属性
  */
int ppc_fchmod(int filedes, mode_t mode, _int64_t token)
{
	return SUCCESS;
}

/*
  *改变文件用户组
  */
int ppc_chown(const char *path, uid_t owner, gid_t group, _int64_t token)
{
	return SUCCESS;
}

/*
  *改变文件用户组
  */
int ppc_fchown(int fd, uid_t owner, gid_t group, _int64_t token)
{
	return SUCCESS;
}

/*
  *改变文件用户组
  */
int ppc_lchown(const char *path, uid_t owner, gid_t group, _int64_t token)
{
	return SUCCESS;
}

/*
  *将fd指定的文件大小改为参数length指定的大小
  */
int ppc_ftruncate(int fd, off_t length, _int64_t token)
{
	int ret = 0;
	fd_info *open_fd_info = NULL;
	ret = get_info_from_ppc_fd_list(fd, &open_fd_info);
	if(ret || (open_fd_info == NULL) || (open_fd_info->path == NULL)){
		DMCLOG_E("get_info_from_ppc_fd_list fail");
		return FAIL;
	}
	DMCLOG_D("open_fd_info->path: %s", open_fd_info->path);

	ret = handle_ftruncate_task(open_fd_info->path, length, token);
	if(ret){
		DMCLOG_E("handle_ftruncate_task fail");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		return FAIL;
	}
	else{
		DMCLOG_D("handle_ftruncate_task success");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		return SUCCESS;
	}
}

/*
  *为文件预分配物理空间
  */
int ppc_fallocate(int fd, int mode, off_t offset, off_t len, _int64_t token)
{
	int ret = 0;
	fd_info *open_fd_info = NULL;
	ret = get_info_from_ppc_fd_list(fd, &open_fd_info);
	if(ret || (open_fd_info == NULL) || (open_fd_info->path == NULL)){
		DMCLOG_E("get_info_from_ppc_fd_list fail");
		return FAIL;
	}
	DMCLOG_D("open_fd_info->path: %s", open_fd_info->path);

	ret = handle_fallocate_task(open_fd_info->path, mode, offset, len, token);
	if(ret){
		DMCLOG_E("handle_fallocate_task fail");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		return FAIL;
	}
	else{
		DMCLOG_D("handle_fallocate_task success");
		safe_free(open_fd_info->path);
		safe_free(open_fd_info);
		return SUCCESS;
	}
}

/*
  *实现零拷贝
  */
ssize_t ppc_sendfile(int out_fd, int in_fd, off_t *offset, size_t count, _int64_t token)
{
	return SUCCESS;
}

/*
  *实现零拷贝
  */
ssize_t ppc_splice(int fd_in,loff_t* off_t,int fd_out,loff_t* off_out,size_t len,unsigned int flags, _int64_t token)
{
	return SUCCESS;
}

/*
  *文件同步
  */
int ppc_fsync(int fd, _int64_t token)
{
	return SUCCESS;
}

/*
  *建立符号链接
  */
int ppc_symlink(const char *oldpath, const char *newpath, _int64_t token)
{
	int ret = 0;
	char *old_full_path = NULL;
	char *new_full_path = NULL;
	
	if(oldpath == NULL || newpath == NULL){
		DMCLOG_E("para is null");
		return FAIL;
	}

	ret = create_new_full_path(oldpath, &old_full_path, token);
	if(ret != 0 || old_full_path == NULL){
		DMCLOG_D("create full path fail");
		return FAIL;
	}

	ret = create_new_full_path(newpath, &new_full_path, token);
	if(ret != 0 || new_full_path == NULL){
		DMCLOG_D("create full path fail");
		safe_free(old_full_path);
		return FAIL;
	}
	
	ret = handle_symlink_task(old_full_path, new_full_path, token);
	if(ret != 0){
		DMCLOG_E("ppc_symlink fail");
		safe_free(old_full_path);
		safe_free(new_full_path);
		return FAIL;
	}

	safe_free(old_full_path);
	safe_free(new_full_path);
	return SUCCESS;
}

/*
  *取得符号连接所指的文件
  */
ssize_t ppc_readlink(const char *path, char *buf, size_t bufsiz, _int64_t token)
{
	int ret = 0;
	char *link_buf = NULL;
	char *full_path = NULL;
	ssize_t link_len = 0;

	if(path == NULL || buf == NULL){
		DMCLOG_E("para is null");
		return FAIL;
	}

	ret = create_new_full_path(path, &full_path, token);
	if(ret != 0 || full_path == NULL){
		DMCLOG_D("create full path fail");
		return FAIL;
	}
	
	ret = handle_readlink_task(full_path, &link_buf, token);
	if(ret != 0 || link_buf == NULL){
		DMCLOG_E("ppc_readlink fail");
		safe_free(full_path);
		return FAIL;
	}

	link_len = strlen(link_buf);
	if(link_len > bufsiz){
		DMCLOG_E("link_buf() is longer than bufsiz(%d)", strlen(link_buf), bufsiz);
		safe_free(full_path);
		safe_free(link_buf);
	}
	
	memcpy(buf, link_buf, link_len);

	safe_free(full_path);
	safe_free(link_buf);
	
	return link_len;
}


/*
  *创建硬链接
  */
int ppc_link (const char * oldpath, const char *newpath, _int64_t token)
{
	int ret = 0;
	char *old_full_path = NULL;
	char *new_full_path = NULL;
	
	if(oldpath == NULL || newpath == NULL){
		DMCLOG_E("para is null");
		return FAIL;
	}

	ret = create_new_full_path(oldpath, &old_full_path, token);
	if(ret != 0 || old_full_path == NULL){
		DMCLOG_D("create full path fail");
		return FAIL;
	}

	ret = create_new_full_path(newpath, &new_full_path, token);
	if(ret != 0 || new_full_path == NULL){
		DMCLOG_D("create full path fail");
		safe_free(old_full_path);
		return FAIL;
	}
	
	ret = handle_link_task(old_full_path, new_full_path, token);
	if(ret != 0){
		DMCLOG_E("handle_link_task fail");
		safe_free(old_full_path);
		safe_free(new_full_path);
		return FAIL;
	}

	safe_free(old_full_path);
	safe_free(new_full_path);
	return SUCCESS;
}

/*
  *创建文件
  */
int ppc_mknod(const char *path, mode_t mode, dev_t dev, _int64_t token)
{
	return SUCCESS;
}

/*
  *获取文件绝对路径
  */
char *ppc_realpath(const char *path, char *resolved_path, _int64_t token)
{
	int ret = 0;
	char *full_path = NULL;
	
	if(path == NULL || resolved_path == NULL){
		DMCLOG_E("para is null");
		return NULL;
	}

	ret = create_new_full_path(path, &full_path, token);
	if(ret != 0 || full_path == NULL){
		DMCLOG_D("create full path fail");
		return NULL;
	}

	memcpy(resolved_path, full_path, strlen(full_path));
	safe_free(full_path);
	return resolved_path;
}

/*
  *读取文件系统信息
  */
int ppc_statvfs(const char *path, struct statvfs *buf, _int64_t token)
{
	int ret = 0;
	char *full_path = NULL;
	
	if(path == NULL || buf == NULL){
		DMCLOG_E("para is null");
		return FAIL;
	}

	ret = create_new_full_path(path, &full_path, token);
	if(ret != 0 || full_path == NULL){
		DMCLOG_D("create full path fail");
		return FAIL;
	}
	DMCLOG_D("full_path: %s", full_path);

	ret = handle_statvfs_task(full_path, buf, token);
	if(ret != 0){
		DMCLOG_E("handle_link_task fail");
		safe_free(full_path);
		return FAIL;
	}
	
	return SUCCESS;
}

