#include "hidisk_file_client.h"
#include "file_process.h"
#include "ppclib.h"

int handle_opendir_task(const char *path, struct dirent ***p_data, _int64_t token)
{
	int ret = 0;
    ClientTheadInfo p_client_info;
    memset(&p_client_info,0,sizeof(ClientTheadInfo));
    p_client_info.cmd = FN_FILE_GET_LIST;
    p_client_info.path = path;
    p_client_info.token = token;
    ret = dm_file_list_prcs_task(&p_client_info);
	if(ret){
		return -1;
	}

	*p_data = p_client_info.p_data;
	return p_client_info.count;
}

#if 0
int handle_closedir_task(PPC_DIR *p_dir)
{
    int i = 0;
    if(p_dir != NULL)
    {
        for(i = 0;;i++)
        {
            if(p_dir->p_data[i] == NULL)
                break;
            safe_free(p_dir->p_data[i]);
        }
        safe_free(p_dir->p_data);
        safe_free(p_dir);
    }
    return 0;
}
#endif

struct dirent *handle_readdir_task(struct dirent **p_data, off_t offset)
{
    struct dirent *dp = p_data[offset];
	if(dp != NULL){
		//DMCLOG_D("p_data[%d]: %s", offset, p_data[offset]->d_name);
		return dp;
	}
	else{
		return NULL;
	}
}

#if 0
void handle_rewinddir_task(PPC_DIR *p_dir)
{
	p_dir->d_off = 0;
	return;
}


void handle_seekdir_task(PPC_DIR *p_dir, off_t loc)
{
	p_dir->d_off = loc;
	return;
}

off_t handle_telldir_task(PPC_DIR *p_dir)
{
	return p_dir->d_off - 1;
}
#endif

int handle_mkdir_task(const char *dirname, mode_t mode, _int64_t token)
{
	ClientTheadInfo p_client_info;
	memset(&p_client_info,0,sizeof(ClientTheadInfo));
    p_client_info.cmd = FN_FILE_MKDIR;
    p_client_info.path = dirname;
    p_client_info.token = token;
    return dm_check_prcs_task(&p_client_info);
}


int handle_rmdir_task(const char *pathname, _int64_t token)
{
	ClientTheadInfo p_client_info;
	memset(&p_client_info,0,sizeof(ClientTheadInfo));
    p_client_info.cmd = FN_FILE_DELETE;// TODO: 可否用DELET
    p_client_info.path = pathname;
    p_client_info.token = token;
    return dm_check_prcs_task(&p_client_info);
}

int handle_register_task(char *username,char *password)
{
    int ret = 0;
    ClientTheadInfo p_client_info;
    memset(&p_client_info,0,sizeof(ClientTheadInfo));
    p_client_info.cmd = 0x0008;
    p_client_info.username = username;
    p_client_info.password = password;
    ret = dm_check_prcs_task(&p_client_info);
    if(ret != 0)
    {
        DMCLOG_E("register error");
    }
    return ret;
}

//登陆操作
int handle_login_task(char *username, char *password, _int64_t *token)
{
    int ret = 0;
    ClientTheadInfo p_client_info;
    memset(&p_client_info,0,sizeof(ClientTheadInfo));
    p_client_info.cmd = 0x0002;
    
    p_client_info.username = username;
    p_client_info.password = password;
//    p_client_info.token = (char *)token;
    ret = dm_check_prcs_task(&p_client_info);
    if(ret != 0)
    {
        DMCLOG_E("login error");
        return ret;
    }
    DMCLOG_D("p_client_info.token = %lld", p_client_info.token);
    *token = p_client_info.token;
    return ret;
}

int handle_logout_task(_int64_t token)
{
    ClientTheadInfo p_client_info;
    memset(&p_client_info,0,sizeof(ClientTheadInfo));
    p_client_info.cmd = 0x0003;
    p_client_info.token = token;
    return dm_check_prcs_task(&p_client_info);
}

int handle_fopen_task(void* arg)
{
    int ret = 0;
    PFILE* fp = (PFILE*)arg;
    if(fp == NULL)
    {
        DMCLOG_E("para is null");
        return -1;
    }
    ClientTheadInfo p_client_info;
    memset(&p_client_info,0,sizeof(ClientTheadInfo));
    if(!strcmp(fp->mode,"rb")||!strcmp(fp->mode,"r"))
    {
        p_client_info.cmd = FN_FILE_DOWNLOAD;
        p_client_info.srcPath = fp->srcPath;
    }else if(!strcmp(fp->mode,"wb")||!strcmp(fp->mode,"w"))
    {
        p_client_info.cmd = FN_FILE_UPLOAD;
        p_client_info.desPath = fp->srcPath;
    }

    p_client_info.token = fp->token;
    p_client_info.offset = fp->offset;
    ret = dm_fopen_prcs_task(&p_client_info);
    if(ret != 0)
    {
        //safe_free(p_client_info.path);
        return -1;
    }
	
    fp->fd = p_client_info.client_fd;
    fp->length = p_client_info.content_len;
    return 0;
}


/*!
 @method
 @abstract 文件下载任务
 @param ?
 @param ?
 @result Null
 */
size_t handle_fread_task(int fd,size_t length,char *buf)
{
    //PFILE* fp = (PFILE*)arg;
    ssize_t ret = -1;
    size_t read_bytes = 0;
    while((ret = read(fd, buf+read_bytes, length-read_bytes)) != 0)
    {
        if(ret < 0)
        {
            if(errno == EINTR)
                continue;
            else
                break;
        }
        read_bytes += ret;
        if(read_bytes == length)
        {
            break;
        }
    }
	//fp->offset += read_bytes;
    return read_bytes;
}

/*
 * 按照offset,length上传文件
 */
size_t handle_fwrite_task(int fd,size_t length,const char *buf)
{
    //PFILE* fp = (PFILE*)arg;
    size_t bytes_write = 0;
    size_t ret = 0;
    DMCLOG_E("fd = %d,length = %zu", fd,length);
    while((bytes_write = send(fd, buf, length, 0))!=0)
    {
        if(bytes_write == -1)
        {
            if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
            {
                continue;
            }else{
                ret = 0;
                DMCLOG_D("server disconnected positive errno = %d",errno);
                break;
            }
        }
        if(bytes_write == length)
       	{
			ret += bytes_write;
			break;
		}
        else if(bytes_write > 0)
        {
        	ret += bytes_write;
            buf += bytes_write;
            length -= bytes_write;
        }
	}
	//fp->offset += ret;
    return ret;
}

void handle_fclose_task(void *arg)
{
    PFILE* fp = (PFILE*)arg;
    DM_DomainClientDeinit(fp->fd);
}

int handle_stat_task(const char *src_path,struct stat *st,_int64_t token)
{
    ClientTheadInfo p_client_info;
    memset(&p_client_info,0,sizeof(ClientTheadInfo));
    p_client_info.cmd = FN_FILE_GET_ATTR;
    p_client_info.srcPath = src_path;
    p_client_info.token = token;
    p_client_info.st = st;
    return dm_check_prcs_task(&p_client_info);
}

int handle_open_task(void* arg)
{
    int ret = 0;
    FFILE* file_info = (FFILE*)arg;
    if(file_info == NULL)
    {
        DMCLOG_E("para is null");
        return -1;
    }
    ClientTheadInfo p_client_info;
    memset(&p_client_info,0,sizeof(ClientTheadInfo));
	// TODO: 其他flag属性和mode怎么处理
	DMCLOG_D("file_info->flag: %d, O_RDONLY: %d, O_WRONLY: %d", file_info->flag, O_RDONLY, O_WRONLY);

	if(file_info->flag & (O_WRONLY | O_CREAT))
    {
        p_client_info.cmd = FN_FILE_WRITE;
        p_client_info.desPath = file_info->srcPath;
    }
	else{
		p_client_info.cmd = FN_FILE_DOWNLOAD;
        p_client_info.srcPath = file_info->srcPath;
	}

#if 0
	//if(file_info->flag == O_RDONLY)
	if(file_info->flag & O_RDONLY)
	{
        p_client_info.cmd = FN_FILE_DOWNLOAD;
        p_client_info.srcPath = file_info->srcPath;
    //}else if(file_info->flag == O_WRONLY)
    }else if(file_info->flag & O_WRONLY)
    {
        p_client_info.cmd = FN_FILE_UPLOAD;
        p_client_info.desPath = file_info->srcPath;
    }
	else{
		DMCLOG_E("unknow flag(%d)", file_info->flag);
		return -1;
	}
#endif
    DMCLOG_D("path = %s",file_info->srcPath);
    p_client_info.token = file_info->token;
    p_client_info.offset = file_info->offset;
    ret = dm_open_prcs_task(&p_client_info);
    if(ret != 0)
    {
        //safe_free(p_client_info.path);
        return -1;
    }
    file_info->fd = p_client_info.client_fd;
    file_info->length = p_client_info.content_len;
    return 0;
}

size_t handle_read_task(int fd,size_t length,char *buf)
{
    ssize_t ret = 0;
    size_t read_bytes = 0;
    while((ret = read(fd, buf+read_bytes, length-read_bytes)) != 0)
    {
        if(ret < 0)
        {
            if(errno == EINTR)
                continue;
            else
                break;
        }
        read_bytes += ret;
        if(read_bytes == length)
        {
            break;
        }
    }
	if(ret < 0)
		return ret;
	else
	    return read_bytes;
}

size_t handle_write_task(int fd,size_t length,const char *buf)
{
    size_t bytes_write = 0;
    size_t ret = 0;
    DMCLOG_D("fd = %d,length = %zu",fd,length);
    while((bytes_write = send(fd, buf, length, 0))!=0)
    {
        if(bytes_write == -1)
        {
            if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
            {
                continue;
            }else{
                ret = -1;
                DMCLOG_E("server disconnected positive errno = %d",errno);
                break;
            }
        }
        if(bytes_write == length)
       	{
			ret += bytes_write;
			break;
		}
        else if(bytes_write > 0)
        {
        	ret += bytes_write;
            buf += bytes_write;
            length -= bytes_write;
        }
	}
    return ret;
}


int handle_rename_task(const char *oldname, const char *newname, _int64_t token)
{
	ClientTheadInfo p_client_info;
	memset(&p_client_info,0,sizeof(ClientTheadInfo));
    p_client_info.cmd = FN_FILE_RENAME;
    p_client_info.srcPath = oldname;
	p_client_info.desPath = newname;
    p_client_info.token = token;
    return dm_check_prcs_task(&p_client_info);
}

int handle_unlink_task(const char *pathname, _int64_t token)
{
	ClientTheadInfo p_client_info;
	memset(&p_client_info,0,sizeof(ClientTheadInfo));
    p_client_info.cmd = FN_FILE_DELETE;
    p_client_info.path = pathname;
    p_client_info.token = token;
    return dm_check_prcs_task(&p_client_info);
}

int handle_utimensat_task(const char *pathname, const struct timespec times[2], int flags, _int64_t token)
{
	ClientTheadInfo p_client_info;
	memset(&p_client_info,0,sizeof(ClientTheadInfo));
	p_client_info.cmd = FN_FILE_UTIMENSAT;
	p_client_info.path = pathname;
	p_client_info.status = flags;
	memcpy(&p_client_info.a_time, &times[0], sizeof(struct timespec));
	memcpy(&p_client_info.m_time, &times[1], sizeof(struct timespec));
	p_client_info.token = token;
	return dm_check_prcs_task(&p_client_info);
}

int handle_ftruncate_task(const char *pathname, off_t length, _int64_t token)
{
	ClientTheadInfo p_client_info;
	memset(&p_client_info,0,sizeof(ClientTheadInfo));
	p_client_info.cmd = FN_FILE_FTRUNCATE;
	p_client_info.path = pathname;
	p_client_info.length = length;
	p_client_info.token = token;
	return dm_check_prcs_task(&p_client_info);
}

int handle_fallocate_task(const char *pathname, int mode, off_t offset, off_t len, _int64_t token)
{
	ClientTheadInfo p_client_info;
	memset(&p_client_info,0,sizeof(ClientTheadInfo));
	p_client_info.cmd = FN_FILE_FALLOCATE;
	p_client_info.path = pathname;
	p_client_info.status = mode;
	p_client_info.offset = offset;
	p_client_info.length = len;
	p_client_info.token = token;
	return dm_check_prcs_task(&p_client_info);
}

int handle_symlink_task(const char *oldname, const char *newname, _int64_t token)
{
	ClientTheadInfo p_client_info;
	memset(&p_client_info,0,sizeof(ClientTheadInfo));
    p_client_info.cmd = FN_FILE_SYMLINK;
    p_client_info.srcPath = oldname;
	p_client_info.desPath = newname;
    p_client_info.token = token;
    return dm_check_prcs_task(&p_client_info);
}

int handle_link_task(const char *oldname, const char *newname, _int64_t token)
{
	ClientTheadInfo p_client_info;
	memset(&p_client_info,0,sizeof(ClientTheadInfo));
    p_client_info.cmd = FN_FILE_LINK;
    p_client_info.srcPath = oldname;
	p_client_info.desPath = newname;
    p_client_info.token = token;
    return dm_check_prcs_task(&p_client_info);
}

int handle_readlink_task(const char *pathname, char **link_buf, _int64_t token)
{
	int ret = 0;
	ClientTheadInfo p_client_info;
	memset(&p_client_info,0,sizeof(ClientTheadInfo));
    p_client_info.cmd = FN_FILE_LINK;
    p_client_info.srcPath = pathname;
    p_client_info.token = token;
    ret = dm_check_prcs_task(&p_client_info);
	if(!ret && p_client_info.path != NULL){
		*link_buf = p_client_info.path; 
	}

	return ret;
}

int handle_statvfs_task(const char *pathname, struct statvfs *buf, _int64_t token)
{
	ClientTheadInfo p_client_info;
	memset(&p_client_info,0,sizeof(ClientTheadInfo));
    p_client_info.cmd = FN_FILE_STATVFS;
    p_client_info.path = pathname;
	p_client_info.statvfs_buf = buf;
    p_client_info.token = token;
    return dm_check_prcs_task(&p_client_info);
}


