#include "hidisk_file_client.h"
#include "file_process.h"
#include "ppclib.h"

int handle_opendir_task(const char *path,PPC_DIR *p_dir,char *token)
{
    ClientTheadInfo p_client_info;
    memset(&p_client_info,0,sizeof(ClientTheadInfo));
    p_client_info.cmd = FN_FILE_GET_LIST;
    p_client_info.path = path;
    p_client_info.p_dir = p_dir;
    p_client_info.token = (char *)token;
    return dm_file_list_prcs_task(&p_client_info);
}

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

struct ppc_dirent *handle_readdir_task(PPC_DIR *p_dir)
{
    struct ppc_dirent *dp = p_dir->p_data[p_dir->d_off++];
    return dp;
}

struct ppc_dirent *handle_seekdir_task(PPC_DIR *p_dir, long loc)
{
	p_dir = p_dir->p_data[loc];
}

struct ppc_dirent *handle_telldir_task(PPC_DIR *p_dir)
{
	return p_dir->d_off;
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
int handle_login_task(char *username,char *password,char **token)
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
    //DMCLOG_D("p_client_info.token = %d", (int)p_client_info.token);
	*token = p_client_info.token;
    return ret;
}

int handle_logout_task(char *token)
{
    ClientTheadInfo p_client_info;
    memset(&p_client_info,0,sizeof(ClientTheadInfo));
    p_client_info.cmd = 0x0003;
    p_client_info.token = token;
    return dm_check_prcs_task(&p_client_info);
}



int handle_open_task(void* arg)
{
	ENTER_FUNC();
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
    //DMCLOG_D("path = %s",fp->srcPath);
    p_client_info.token = fp->token;
    p_client_info.offset = fp->offset;
    ret = dm_open_prcs_task(&p_client_info);
    if(ret != 0)
    {
        safe_free(p_client_info.path);
        return -1;
    }
    fp->fd = p_client_info.client_fd;
    fp->length = p_client_info.content_len;
	EXIT_FUNC();
    return 0;
}


/*!
 @method
 @abstract 文件下载任务
 @param ?
 @param ?
 @result Null
 */
size_t handle_read_task(void* arg,size_t length,char *buf)
{
    PFILE* fp = (PFILE*)arg;
    ssize_t ret = -1;
    size_t read_bytes = 0;
    while((ret = read(fp->fd, buf+read_bytes, length-read_bytes)) != 0)
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
    return read_bytes;
}

/*
 * 按照offset,length上传文件
 */
size_t handle_write_task(void *arg,size_t length,const char *buf)
{
    PFILE* fp = (PFILE*)arg;
    size_t bytes_write = 0;
    size_t ret = 0;
    //DMCLOG_E("fd = %d,length = %zu",fp->fd,length);
    while((bytes_write = send(fp->fd, buf, length, 0))!=0)
    {
        DMCLOG_D("access");
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
    return ret;
}

void handle_close_task(void *arg)
{
    PFILE* fp = (PFILE*)arg;
    DM_DomainClientDeinit(fp->fd);
}

int handle_stat_task(const char* src_path,struct ppc_stat *st,char *token)
{
    ClientTheadInfo p_client_info;
    memset(&p_client_info,0,sizeof(ClientTheadInfo));
    p_client_info.cmd = FN_FILE_GET_ATTR;
    p_client_info.srcPath = src_path;
    p_client_info.token = (char *)token;
    p_client_info.st = st;
    return dm_check_prcs_task(&p_client_info);
}
