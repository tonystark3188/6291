/************************************************************************
 #
 #  Copyright (c) 2015-2016  longsys(SHENTHEN) Co., Ltd.
 #  All Rights Reserved
 #
 #  author: Oliver
 #  create date: 2015-3-17
 #
 # Unless you and longsys execute a separate written software license
 # agreement governing use of this software, this software is licensed
 # to you under the terms of the GNU General Public License version 2
 # (the "GPL"), with the following added to such license:
 #
 #    As a special exception, the copyright holders of this software give
 #    you permission to link this software with independent modules, and
 #    to copy and distribute the resulting executable under terms of your
 #    choice, provided that you also meet, for each linked independent
 #    module, the terms and conditions of the license of that module.
 #    An independent module is a module which is not derived from this
 #    software.  The special exception does not apply to any modifications
 #    of the software.
 #
 # Not withstanding the above, under no circumstances may you combine
 # this software in any way with any other longsys software provided
 # under a license other than the GPL, without longsys's express prior
 # written consent.
 #
 #
 *************************************************************************/

/*############################## Includes ####################################*/
#include <errno.h>
#include <signal.h>
#include "ppclib.h"
/*############################## Global Variable #############################*/


#define ROOT_PATH "/home/hcy/test_video"
/*!
 @enum
 @abstract 命令类型
 */
typedef enum DMDiskClientCommand {
    DM_Register = 1,//注册账号
    DM_PkgVersion = 2,//获取库版本信息
    DM_LoginDevice = 111,//登陆设备
    DM_LogoutDevice = 112,//登出设备
    /*************基础文件接口***************/
    DM_DGetVersion = 8,//获取当前设备版本号
    DM_GetFileList = 10,//获取文件列表
    DM_Mkdirs = 11,//创建文件夹
    DM_Rename = 12,//重命名
    DM_IsExist = 13,//判断目标路径是否存在
    DM_Download = 14,//文件下载
    DM_StreamDownload = 15,//文件流下载
    DM_Upload = 16,//文件上传
    DM_StreamUpload = 17,//文件流形式上传
    DM_Delete = 18,//删除文件
    DM_FdGetFileList = 19,//根据fdopendir打开目录并获取文件列表
    DM_Rmdir = 20,//删除目录
    DM_GetFileAttr = 81,//获取文件详情
    DM_FCopy = 52,/*文件或者文件夹复制*/
    DM_FMove = 53,/*文件或者文件夹移动*/

    DM_FileFWrite = 200,//文件上传
    DM_FileFRead =  201,//文件上传
	DM_FileWrite = 202,//文件上传
    DM_FileRead =  203,//文件上传
} DMDiskClientCommand;

int test_fread(char *src_path,char *des_path,char *token)
{
    size_t per_bytes = 0;
    
    size_t total_bytes = 0;
    int fd = open(des_path,O_WRONLY | O_CREAT,0644);
    PFILE *pf = ppc_fopen(src_path,"r",token);
    if(pf == NULL)
    {
        printf("ppc open error\n");
        return -1;
    }
    char buffer[16384] = {0};
    
    do{
        per_bytes = ppc_fread(buffer,1, 16384, pf);
        if(per_bytes <= 0)
        {
            printf("read finish\n");
            break;
        }
        total_bytes += per_bytes;
        printf("haved read %lu \n",total_bytes);
        write(fd,buffer,per_bytes);
    }while(1);
    
    ppc_fclose(pf);
    close(fd);
    return 0;
}

int test_fwrite(char *src_path,char *des_path,char *token)
{
    ssize_t per_bytes = 0;
    size_t write_bytes = 0;
    size_t total_bytes = 0;
    DMCLOG_D("src_path = %s",src_path);
    int fd = open(src_path,O_RDONLY,0644);
    if(fd <= 0)
    {
        DMCLOG_E("%s open error",src_path);
        return -1;
    }
    PFILE *pf = ppc_fopen(des_path,"w",token);
    if(pf == NULL)
    {
        printf("ppc open error\n");
        return -1;
    }
    char buffer[16384] = {0};
    do{
        per_bytes = read(fd,buffer,16384);
        if(per_bytes <= 0)
        {
            printf("read finish\n");
            break;
        }
        DMCLOG_D("per_bytes = %zu",per_bytes);
        write_bytes = ppc_fwrite(buffer,1,per_bytes,pf);
        if(write_bytes > 0)
        {
            total_bytes += write_bytes;
            printf("haved writed %lu \n",total_bytes);
        }
    }while(write_bytes >0);
    
    ppc_fclose(pf);
    close(fd);
    return 0;
}

int test_read(char *src_path,char *des_path,char *token)
{
    size_t per_bytes = 0;
    size_t total_bytes = 0;
	
    int des_fd = open(des_path,O_WRONLY | O_CREAT,0644);
	if(-1 == des_fd)
	{
		DMCLOG_E("open error\n");
		return -1;
	}
	
    int src_fd = ppc_open(src_path, O_RDONLY, 0, token);
    if(-1 == src_fd)
    {
        DMCLOG_E("ppc open error\n");
        return -1;
    }
    char buffer[16384] = {0};
	memset(buffer, 0, sizeof(buffer));
    
    do{
        per_bytes = ppc_read(src_fd, buffer, sizeof(buffer));
        if(per_bytes <= 0)
        {
            printf("read finish\n");
            break;
        }
        total_bytes += per_bytes;
        printf("haved read %lu \n",total_bytes);
        write(des_fd, buffer, per_bytes);
    }while(1);
    
    ppc_close(src_fd);
    close(des_fd);
    return 0;
}

int test_write(char *src_path,char *des_path,char *token)
{
    ssize_t per_bytes = 0;
    size_t write_bytes = 0;
    size_t total_bytes = 0;
    DMCLOG_D("src_path = %s",src_path);
    int src_fd = open(src_path,O_RDONLY,0644);
    if(src_fd < 0){
        DMCLOG_E("%s open error",src_path);
        return -1;
    }
	
    int des_fd = ppc_open(src_path, O_WRONLY, 0, token);
    if(des_fd < 0)
    {
        printf("ppc open error\n");
        return -1;
    }
    char buffer[16384] = {0};
	memset(buffer, 0, sizeof(buffer));
	
    do{
        per_bytes = read(src_fd, buffer, sizeof(buffer));
        if(per_bytes <= 0)
        {
            printf("read finish\n");
            break;
        }
        DMCLOG_D("per_bytes = %zu",per_bytes);
        write_bytes = ppc_write(des_fd, buffer, per_bytes);
        if(write_bytes > 0)
        {
            total_bytes += write_bytes;
            printf("haved writed %lu \n",total_bytes);
        }
    }while(write_bytes >0);
    
    ppc_close(des_fd);
    close(src_fd);
    return 0;
}


FILE *read_fd;
FILE *write_fd;

static ssize_t dm_stream_read(int seq,void *buf,ssize_t readSize)
{
    ssize_t len;
    len = fread(buf, 1, readSize, read_fd);
    return len;
}

static ssize_t dm_stream_write(int seq,void *buf,ssize_t writeSize)
{
    ssize_t len;
    len = fwrite(buf, 1, writeSize, write_fd);
    return len;
}

//    DMCLOG_D("int length:%lu",sizeof(int));
//    DMCLOG_D("unsigned length:%lu",sizeof(unsigned));
//    DMCLOG_D("long length:%lu",sizeof(long));
//    DMCLOG_D("unsigned long length:%lu",sizeof(unsigned long));
//        scanf("%d",&cmd);

int main(int argc,char *argv[])
{
    int res = 0;
    char *utoken = NULL;
    int i = 0;
    char *username  = NULL;
    char *password = NULL;
    char cmd_str[1024] = {0};
    char *cmd[4];
    int command = 0;
    char src[1024] = {0};
    char des[1024] = {0};
    PPC_DIR *p_dir = NULL;
	pkg_version *ppc_pkg_version = NULL;
	
    do{
        gets(cmd_str);
        DMCLOG_D("cmd_str = %s",cmd_str);
        i = 0;
        char *token = strtok(cmd_str," ");
        if(token == NULL)
        {
            DMCLOG_E("para is null");
            return -1;
        }
        cmd[i++] = token;
        while(token != NULL)
        {
            DMCLOG_D("token = %s",token);
            token = strtok(NULL," ");
            cmd[i++] = token;
        }
        if(!strcmp(cmd[0],"ls"))
        {
            command = DM_GetFileList;
            if(cmd[1] != NULL)
            strcpy(src,cmd[1]);
        }else if(!strcmp(cmd[0],"mkdir"))
        {
            command = DM_Mkdirs;
            strcpy(src,cmd[1]);
        }else if(!strcmp(cmd[0],"rename"))
        {
            command = DM_Rename;
            strcpy(src,cmd[1]);
            strcpy(des,cmd[2]);
        }else if(!strcmp(cmd[0],"delete"))
        {
            command = DM_Delete;
            strcpy(src,cmd[1]);
        }else if(!strcmp(cmd[0],"cp"))
        {
            command = DM_FCopy;
            strcpy(src,cmd[1]);
            strcpy(des,cmd[2]);
        }else if(!strcmp(cmd[0],"mv"))
        {
            command = DM_FMove;
            strcpy(src,cmd[1]);
            strcpy(des,cmd[2]);
        }else if(!strcmp(cmd[0],"rm"))
        {
            command = DM_Delete;
            strcpy(src,cmd[1]);
        }else if(!strcmp(cmd[0],"stat"))
        {
            command = DM_GetFileAttr;
            strcpy(src,cmd[1]);
        }else if(!strcmp(cmd[0],"download"))
        {
            command = DM_Download;
            sprintf(src,"%s",cmd[1]);
            sprintf(des,"%s/%s",ROOT_PATH,cmd[2]);
        }else if(!strcmp(cmd[0],"upload"))
        {
            command = DM_Upload;
            sprintf(src,"%s/%s",ROOT_PATH,cmd[1]);
            sprintf(des,"%s",cmd[2]);
        }else if(!strcmp(cmd[0],"ppc_fread"))
        {
            command = DM_FileFRead;
            sprintf(src,"%s",cmd[1]);
            sprintf(des,"%s/%s",ROOT_PATH,cmd[2]);
        }else if(!strcmp(cmd[0],"ppc_fwrite"))
        {
            command = DM_FileFWrite;
            sprintf(src,"%s/%s",ROOT_PATH,cmd[1]);
            sprintf(des,"%s",cmd[2]);
		}else if(!strcmp(cmd[0],"ppc_read"))
        {
            command = DM_FileRead;
            sprintf(src,"%s",cmd[1]);
            sprintf(des,"%s/%s",ROOT_PATH,cmd[2]);
        }else if(!strcmp(cmd[0],"ppc_write"))
        {
            command = DM_FileWrite;
            sprintf(src,"%s/%s",ROOT_PATH,cmd[1]);
            sprintf(des,"%s",cmd[2]);
        }else if(!strcmp(cmd[0],"fd_ls"))
		{
			command = DM_FdGetFileList;
            if(cmd[1] != NULL)
            	strcpy(src,cmd[1]);
		}else if(!strcmp(cmd[0], "rmdir"))
		{
			command = DM_Rmdir;
            if(cmd[1] != NULL)
            	strcpy(src,cmd[1]);
		}
		else if(!strcmp(cmd[0], "version"))
		{
			command = DM_PkgVersion;
		}	
		else{
            command = atoi(cmd[0]);
        }

        switch (command) {
            case DM_Register:
                username  = "jack4";
                password = "22222222";
                res = ppc_register(username,password);
                if(res != 0)
                {
                    DMCLOG_E("register error");
                    return res;
                }
                break;
            case DM_LoginDevice:
                username  = "jack4";
                password = "22222222";
                res = ppc_login(username,password,&utoken);
                if(res != 0)
                {
                    DMCLOG_E("login error");
                    return res;
                }
                break;
            case DM_LogoutDevice:
                res = ppc_logout(utoken);
                if(res != 0)
                {
                    DMCLOG_E("logout error");
                    return res;
                }
                break;
            case DM_DGetVersion:
                break;
            case DM_GetFileList:
                p_dir = ppc_opendir(src,utoken);
                char file[FILENAME_MAX];
                const char *slash = "/";
                struct ppc_stat st;
                if(p_dir == NULL)
                {
                    DMCLOG_E("open dir %s error",src);
                    return -1;
                }
                struct ppc_dirent *dp = NULL;
                do{
                    if((dp = ppc_readdir(p_dir,utoken)) == NULL)
                    {
                        DMCLOG_D("read finish");
                        break;
                    }
                    if(!strcmp(src,slash))
                    {
                        sprintf(file,"%s%s",src,dp->d_name);
                    }else{
                        sprintf(file,"%s%s%s",src,slash,dp->d_name);
                    }

                    ppc_stat(file,&st,utoken);
                    DMCLOG_D("size = %lld,mtime_t = %ld,isDir = %d,st.st_mode = %d",st.st_size,st.st_mtime_t,S_ISDIR(st.st_mode),st.st_mode);

					off_t telldir_offset = ppc_telldir(p_dir, utoken);				
					DMCLOG_D("ppc_telldir: %llu", telldir_offset);
					off_t seek_num = p_dir->d_off + 1;
					ppc_seekdir(p_dir, seek_num, utoken);
                }while(dp != NULL);
                
                ppc_closedir(p_dir,utoken);
                break;
            case DM_Mkdirs:
				if(!ppc_mkdir(src, 0, utoken)){
					DMCLOG_D("mkdir %s success", src);
				}
				else{
					DMCLOG_D("mkdir %s fail", src);
				}
                break;
			case DM_Rmdir:
				if(!ppc_rmdir(src, utoken)){
					DMCLOG_D("rmdir %s success", src);
				}
				else{
					DMCLOG_D("rmdir %s fail", src);
				}
                break;
            case DM_Rename:
                break;
            case DM_IsExist:
                break;
            case DM_FCopy:
                break;
            case DM_FMove:
                break;
            case DM_Delete:
                break;
            case DM_GetFileAttr:
                break;
            case DM_Download:
                break;
            case DM_Upload:
                break;
            case DM_FileFWrite:
            {
                DMCLOG_D("src = %s",src);
                DMCLOG_D("des = %s",des);
                test_fwrite(src, des, utoken);
            }
                break;
            case DM_FileFRead:
            {
                DMCLOG_D("src = %s",src);
                DMCLOG_D("des = %s",des);
                test_fread(src, des,utoken);
            }
                break;
			case DM_FileWrite:
            {
                DMCLOG_D("src = %s",src);
                DMCLOG_D("des = %s",des);
                test_write(src, des, utoken);
            }
                break;
            case DM_FileRead:
            {
                DMCLOG_D("src = %s",src);
                DMCLOG_D("des = %s",des);
                test_read(src, des,utoken);
            }
                break;			
			case DM_FdGetFileList:
			{
				DMCLOG_D("access ppc_open %s", src);
				int fd = ppc_open(src, O_RDONLY, 0, utoken);
				if(!fd){
					DMCLOG_E("ppc_open fail");
					break;
				}
				p_dir = ppc_fdopendir(fd,utoken);
				char file[FILENAME_MAX];
                const char *slash = "/";
                struct ppc_stat st;
                if(p_dir == NULL)
                {
                    DMCLOG_E("open dir %s error",src);
                    return -1;
                }
                struct ppc_dirent *dp = NULL;
                do{
                    if((dp = ppc_readdir(p_dir,utoken)) == NULL)
                    {
                        DMCLOG_D("read finish");
                        break;
                    }
                    if(!strcmp(src,slash))
                    {
                        sprintf(file,"%s%s",src,dp->d_name);
                    }else{
                        sprintf(file,"%s%s%s",src,slash,dp->d_name);
                    }

                    ppc_stat(file,&st,utoken);
                    DMCLOG_D("size = %lld,mtime_t = %ld,isDir = %d,st.st_mode = %d",st.st_size,st.st_mtime_t,S_ISDIR(st.st_mode),st.st_mode);

					off_t telldir_offset = ppc_telldir(p_dir, utoken);				
					DMCLOG_D("ppc_telldir: %llu", telldir_offset);
					off_t seek_num = p_dir->d_off + 1;
					ppc_seekdir(p_dir, seek_num, utoken);
                }while(dp != NULL);
                
                ppc_closedir(p_dir,utoken);
				ppc_close(fd);
			}
				break;
			case DM_PkgVersion:
				if((ppc_pkg_version = (pkg_version *)calloc(1, sizeof(pkg_version))) == NULL){
					DMCLOG_E("malloc for ppc_pkg_version fail");
				}
				else{				
					res = ppc_get_pkg_version(ppc_pkg_version);
					if(!res){
						DMCLOG_D("PKG NAME: %s, PKG VERSION: %s", ppc_pkg_version->pkg_name, ppc_pkg_version->pkg_version);
					}
					else{
						DMCLOG_E("get pkg version fail");
					}
					free(ppc_pkg_version);
				}
				break;
            default:
                break;
        }
    }while(1);
	return 0;
}

