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
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>    
#include <sys/stat.h>    
#include <fcntl.h>
#include "ppclib.h"
#include "my_debug.h"
/*############################## Global Variable #############################*/


#define ROOT_PATH ""
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
    DM_Cd = 21,//更改工作目录
    DM_Pwd = 22,//查看工作目录
    DM_GetFileAttr = 81,//获取文件详情
    DM_FCopy = 52,/*文件或者文件夹复制*/
    DM_FMove = 53,/*文件或者文件夹移动*/
    DM_Ftruncate = 54,/* 文件截断 */
    DM_Fallocate = 55,/* 文件空间预分配*/
    DM_Symlink = 56,/* 创建符号链接 */
    DM_Link = 57,/* 创建硬链接*/
    DM_Readlink = 58,/* 获取链接地址 */
    DM_Statvfs = 59,/* 获取文件系统信息 */
	DM_Utimensat = 60,/* 设置文件时间 */

    DM_FileFWrite = 200,//文件上传
    DM_FileFRead =  201,//文件上传
    DM_FileFSeek = 204,
	DM_FileWrite = 202,//文件上传
    DM_FileRead =  203,//文件上传
    DM_FileSeek = 205,
    DM_FilePWrite = 206,//文件上传
    DM_FilePRead =  207,//文件上传
    DM_FileWritev = 208,//文件上传
} DMDiskClientCommand;

int test_fread(char *src_path,char *des_path, _int64_t token)
{
    size_t per_bytes = 0;
    
    size_t total_bytes = 0;
	DMCLOG_M("START test_fread !!!!!!!!!");
	
    int fd = open(des_path,O_WRONLY | O_CREAT,0644);
    FILE *fp = ppc_fopen(src_path,"r",token);
    if(fp == NULL)
    {
        printf("ppc open error\n");
        return -1;
    }
    char buffer[16384] = {0};
    
    do{
        per_bytes = ppc_fread(buffer,1, 16384, fp);
        if(per_bytes <= 0)
        {
            printf("read finish\n");
            break;
        }
        total_bytes += per_bytes;
        DMCLOG_D("haved read %lu , ftell: %d",total_bytes, ppc_ftell(fp, token));
        write(fd,buffer,per_bytes);
    }while(1);
    
    ppc_fclose(fp);
    close(fd);
	DMCLOG_M("END test_fread !!!!!!!!!");
    return 0;
}

int test_fwrite(char *src_path,char *des_path, _int64_t token)
{
    ssize_t per_bytes = 0;
    size_t write_bytes = 0;
    size_t total_bytes = 0;
	
    DMCLOG_M("START test_fwrite !!!!!!!!!");
    int fd = open(src_path,O_RDONLY,0644);
    if(fd <= 0)
    {
        DMCLOG_E("%s open error",src_path);
        return -1;
    }
    FILE *fp = ppc_fopen(des_path,"w",token);
    if(fp == NULL)
    {
        printf("ppc open error\n");
        return -1;
    }
	DMCLOG_D("fopen success");
    char buffer[16384] = {0};
    do{
        per_bytes = read(fd,buffer,16384);
        if(per_bytes <= 0)
        {
            printf("read finish\n");
            break;
        }
        DMCLOG_D("per_bytes = %zu",per_bytes);
        write_bytes = ppc_fwrite(buffer,1,per_bytes,fp);
        if(write_bytes > 0)
        {
            total_bytes += write_bytes;
            DMCLOG_D("haved writed %lu ",total_bytes);
        }
    }while(write_bytes >0);
    
    ppc_fclose(fp);
    close(fd);
	DMCLOG_M("END test_fwrite !!!!!!!!!");
    return 0;
}

int test_fseek_ftell(char *src_path,char *des_path, _int64_t token)
{
    size_t per_bytes = 0;
    off_t seek_offset = 0;
    size_t total_bytes = 0;
	DMCLOG_M("START test_fseek_ftell !!!!!!!!!");
	
    int fd = open(des_path,O_WRONLY | O_CREAT,0644);
    FILE *fp = ppc_fopen(src_path,"r",token);
    if(fp == NULL)
    {
        printf("ppc open error\n");
        return -1;
    }
    char buffer[16384] = {0};
    
    do{
        per_bytes = ppc_fread(buffer,1, 16384, fp);
        if(per_bytes <= 0)
        {
            printf("read finish\n");
            break;
        }
        total_bytes += per_bytes;
        DMCLOG_D("haved read %lu , ftell: %d",total_bytes, ppc_ftell(fp, token));
        write(fd,buffer,per_bytes);

		//seek_offset = ppc_fseek(fp, total_bytes, SEEK_SET, token);
		seek_offset = ppc_fseek(fp, total_bytes - 9207537 , SEEK_END, token);
		//seek_offset = ppc_fseek(fp, 0, SEEK_CUR, token);
		
		DMCLOG_D("seek_offset %d , ftell: %d",total_bytes, ppc_ftell(fp, token));
    }while(1);
    
    ppc_fclose(fp);
    close(fd);
	DMCLOG_M("END test_fseek_ftell !!!!!!!!!");
    return 0;

}


int test_read(char *src_path,char *des_path, _int64_t token)
{
    size_t per_bytes = 0;
    size_t total_bytes = 0;

	DMCLOG_M("START test_read !!!!!!!!!");
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
        DMCLOG_D("haved read %lu ",total_bytes);
        write(des_fd, buffer, per_bytes);
    }while(1);
    
    ppc_close(src_fd);
    close(des_fd);
	DMCLOG_M("END test_read !!!!!!!!!");
    return 0;
}

int test_write(char *src_path,char *des_path, _int64_t token)
{
    ssize_t per_bytes = 0;
    size_t write_bytes = 0;
    size_t total_bytes = 0;
	
    DMCLOG_M("START test_write !!!!!!!!!");
    int src_fd = open(src_path,O_RDONLY,0644);
    if(src_fd < 0){
        DMCLOG_E("%s open error",src_path);
        return -1;
    }
	
    int des_fd = ppc_open(des_path, O_WRONLY, 0, token);
    //int des_fd = ppc_open(des_path, O_CREAT, 0, token);
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
            DMCLOG_D("haved writed %lu ",total_bytes);
        }
    }while(write_bytes >0);
    
    ppc_close(des_fd);
    close(src_fd);
	DMCLOG_M("END test_write !!!!!!!!!");
    return 0;
}

int test_seek_tell(char *src_path,char *des_path, _int64_t token)
{
    size_t per_bytes = 0;
    size_t total_bytes = 0;
	 off_t seek_offset = 0;

	DMCLOG_M("START test_seek_tell !!!!!!!!!");
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
        DMCLOG_D("haved read %lu ",total_bytes);
        write(des_fd, buffer, per_bytes);

		seek_offset = ppc_lseek(src_fd, total_bytes, SEEK_SET, token);
		//seek_offset = ppc_lseek(src_fd, total_bytes - 9207537, SEEK_END, token);
		//seek_offset = ppc_lseek(src_fd, 0, SEEK_CUR, token);
		
		DMCLOG_D("seek_offset %d ",total_bytes);
    }while(1);
    
    ppc_close(src_fd);
    close(des_fd);
	DMCLOG_M("END test_seek_tell !!!!!!!!!");
    return 0;
}

int test_pread(char *src_path,char *des_path, _int64_t token)
{
    size_t per_bytes = 0;
    size_t total_bytes = 0;

	DMCLOG_M("START test_read !!!!!!!!!");
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
        per_bytes = ppc_pread(src_fd, buffer, sizeof(buffer), total_bytes, token);
        if(per_bytes <= 0)
        {
            printf("read finish\n");
            break;
        }
        total_bytes += per_bytes;
        DMCLOG_D("haved read %lu ",total_bytes);
        write(des_fd, buffer, per_bytes);
    }while(1);
    
    ppc_close(src_fd);
    close(des_fd);
	DMCLOG_M("END test_read !!!!!!!!!");
    return 0;
}


int test_pwrite(char *src_path,char *des_path, _int64_t token)
{
    ssize_t per_bytes = 0;
    size_t write_bytes = 0;
    size_t total_bytes = 0;
	
    DMCLOG_M("START test_write !!!!!!!!!");
    int src_fd = open(src_path,O_RDONLY,0644);
    if(src_fd < 0){
        DMCLOG_E("%s open error",src_path);
        return -1;
    }
	
    int des_fd = ppc_open(des_path, O_WRONLY, 0, token);
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
        write_bytes = ppc_pwrite(des_fd, buffer, per_bytes, total_bytes, token);
        if(write_bytes > 0)
        {
            total_bytes += write_bytes;
            DMCLOG_D("haved writed %lu ",total_bytes);
        }
    }while(write_bytes >0);
    
    ppc_close(des_fd);
    close(src_fd);
	DMCLOG_M("END test_write !!!!!!!!!");
    return 0;
}

int test_writev(char *src_path,char *des_path, _int64_t token)
{
    ssize_t per_bytes = 0;
    size_t write_bytes = 0;
    size_t total_bytes = 0;
	
    DMCLOG_M("START test_writev !!!!!!!!!");
    int src_fd = open(src_path,O_RDONLY,0644);
    if(src_fd < 0){
        DMCLOG_E("%s open error",src_path);
        return -1;
    }
	
    int des_fd = ppc_open(des_path, O_WRONLY, 0, token);
    if(des_fd < 0)
    {
        printf("ppc open error\n");
        return -1;
    }
    char buffer1[16384] = {0};
	memset(buffer1, 0, sizeof(buffer1));
	char buffer2[16384] = {0};
	memset(buffer2, 0, sizeof(buffer2));

    struct iovec iov[2];
	int iov_num = 0;
    iov[0].iov_base = buffer1;  
    iov[1].iov_base = buffer2;  

    do{
		iov_num = 0;
		memset(buffer1, 0, sizeof(buffer1));
		memset(buffer2, 0, sizeof(buffer2));

		per_bytes = read(src_fd, buffer1, sizeof(buffer1));
        if(per_bytes <= 0){
            printf("read none\n");
            //break;
        }
		else{
			DMCLOG_D("per_bytes = %zu",per_bytes);
			iov[0].iov_len = per_bytes;
			iov_num++;
		}

		per_bytes = read(src_fd, buffer2, sizeof(buffer2));
        if(per_bytes <= 0){
            printf("read none\n");
            //break;
        }
		else{
			DMCLOG_D("per_bytes = %zu",per_bytes);
			iov[1].iov_len = per_bytes;
			iov_num++;
		}

        if(iov_num > 0){
	        write_bytes = ppc_writev(des_fd, iov, iov_num, token);
	        if(write_bytes > 0){
	            total_bytes += write_bytes;
	            DMCLOG_D("haved writed %lu ",total_bytes);
	        }
        }
		else{
			printf("read finish\n");
            break;
		}
    }while(write_bytes >0);
    
    ppc_close(des_fd);
    close(src_fd);
	DMCLOG_M("END test_writev !!!!!!!!!");
    return 0;
}

typedef enum {
    TOTAL_SIZE,///文件系统的大小
    FREE_SIZE, ///自由空间
    USED_SIZE, ///已用空间
    AVAIL_SIZE ///用户实际可以使用的空间
}VFsize;

//把数字大小转换成字符形式
char *
byte_size_to_string(fsblkcnt_t size)
{
    const double k = 1024;
    const double m = k*1024;
    const double g = m*k;
    static char size_str[128] = {0};
    if(size>=g)
        sprintf(size_str,"%0.2lf GB",size/g);
    else if(size>=m)
        sprintf(size_str,"%0.2lf MB",size/m);
    else if(size>=k)
        sprintf(size_str,"%0.2lf KB",size/k);
    else
        sprintf(size_str,"%0.0lf Byte",size>=0?size:0.0);
    return size_str;
}
///文件系统的各种信息数据的大小
fsblkcnt_t
get_vfs_size(struct statvfs *buf,VFsize flag)
{
    fsblkcnt_t block;
    fsblkcnt_t bsize;
    bsize = buf->f_bsize;
    switch (flag){
    case TOTAL_SIZE:
        block = buf->f_blocks;
    break;
    case FREE_SIZE:
        block = buf->f_bfree;
    break;
    case USED_SIZE:
        block = buf->f_blocks - buf->f_bavail;
    break;
    case AVAIL_SIZE:
        block = buf->f_bavail;
    break;
    default:
        block = 0;
    break;
    }
    return  bsize * block;
}

static ssize_t dm_stream_read(int seq,void *buf,ssize_t readSize)
{
	FILE *read_fd;
    ssize_t len;
    len = fread(buf, 1, readSize, read_fd);
    return len;
}

static ssize_t dm_stream_write(int seq,void *buf,ssize_t writeSize)
{
	FILE *write_fd;
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
    _int64_t utoken = 0;
    int i = 0;
    char *username  = NULL;
    char *password = NULL;
    char cmd_str[1024] = {0};
    char *cmd[4];
    int command = 0;
    char src[1024] = {0};
    char des[1024] = {0};
	char des_2[1024] = {0};
	char tmp_str[1024]; 
    DIR *p_dir = NULL;
	struct statvfs statvfs_buf;
	struct timespec times[2];
	struct stat st;
	pkg_version *ppc_pkg_version = NULL;
	res = ppc_initialise();
	if(res){
		DMCLOG_E("initialise fail");
		return -1;
	}
	else{
		DMCLOG_D("initialise success");
	}
	
    do{
		memset(src, '\0', sizeof(src));
		memset(des, '\0', sizeof(des));
        gets(cmd_str);
        DMCLOG_D("cmd_str = %s",cmd_str);
        i = 0;
        char *arg = strtok(cmd_str," ");
        if(arg == NULL){
            DMCLOG_E("para is null");
			continue;
			//return -1;
        }
        cmd[i++] = arg;
        while(arg != NULL)
        {
            DMCLOG_D("arg = %s",arg);
            arg = strtok(NULL," ");
            cmd[i++] = arg;
        }
		if(!strcmp(cmd[0],"register"))
        {
            command = DM_Register;
            if(cmd[1] != NULL)
            	strcpy(src,cmd[1]);
			if(cmd[2] != NULL)
            	strcpy(des,cmd[2]);
		}else if(!strcmp(cmd[0],"login"))
        {
            command = DM_LoginDevice;
            if(cmd[1] != NULL)
            	strcpy(src,cmd[1]);
			if(cmd[2] != NULL)
            	strcpy(des,cmd[2]);
		}else if(!strcmp(cmd[0],"logout"))
        {
            command = DM_LogoutDevice;
        }else if(!strcmp(cmd[0],"ls"))
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
		}else if(!strcmp(cmd[0],"ppc_fseek"))
        {
            command = DM_FileFSeek;
            sprintf(src,"%s",cmd[1]);
            sprintf(des,"%s/%s",ROOT_PATH,cmd[2]);
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
		}else if(!strcmp(cmd[0],"ppc_seek"))
        {
            command = DM_FileSeek;
            sprintf(src,"%s",cmd[1]);
            sprintf(des,"%s/%s",ROOT_PATH,cmd[2]);
		}else if(!strcmp(cmd[0],"ppc_pread"))
        {
            command = DM_FilePRead;
            sprintf(src,"%s",cmd[1]);
            sprintf(des,"%s/%s",ROOT_PATH,cmd[2]);
		}else if(!strcmp(cmd[0],"ppc_writev"))
        {
            command = DM_FileWritev;
            sprintf(src,"%s/%s",ROOT_PATH,cmd[1]);
            sprintf(des,"%s",cmd[2]);
        }else if(!strcmp(cmd[0],"ppc_pwrite"))
        {
            command = DM_FilePWrite;
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
		}else if(!strcmp(cmd[0], "cd"))
		{
			command = DM_Cd;
            if(cmd[1] != NULL)
            	strcpy(src,cmd[1]);
		}else if(!strcmp(cmd[0], "pwd"))
		{
			command = DM_Pwd;
		}else if(!strcmp(cmd[0], "ftruncate"))
		{
			sprintf(src,"%s",cmd[1]);
			command = DM_Ftruncate;
		}else if(!strcmp(cmd[0], "fallocate"))
		{
			sprintf(src,"%s",cmd[1]);
			command = DM_Fallocate;
		}else if(!strcmp(cmd[0], "symlink"))
		{
			sprintf(src,"%s",cmd[1]);
			sprintf(des,"%s",cmd[2]);
			command = DM_Symlink;
		}else if(!strcmp(cmd[0], "link"))
		{
			sprintf(src,"%s",cmd[1]);
			sprintf(des,"%s",cmd[2]);
			command = DM_Link;
		}else if(!strcmp(cmd[0], "readlink"))
		{
			sprintf(src,"%s",cmd[1]);
			command = DM_Readlink;
		}else if(!strcmp(cmd[0], "statvfs"))
		{
			sprintf(src,"%s",cmd[1]);
			command = DM_Statvfs;	
		}else if(!strcmp(cmd[0], "utimensat"))
		{
			sprintf(src,"%s",cmd[1]);
			command = DM_Utimensat;
		}else if(!strcmp(cmd[0], "version"))
		{
			command = DM_PkgVersion;
		}else{
            command = atoi(cmd[0]);
        }

        switch (command) {
            case DM_Register:
				if(strlen(src) && strlen(des)){
					res = ppc_register(src,des);
				}
				else{
                	username  = "hcy";
                	password = "123456";
                	res = ppc_register(username,password);
				}
				if(res != 0){
                    DMCLOG_E("register error");
                    //return res;
                }
				else{
					DMCLOG_E("register success");
				}
                break;
            case DM_LoginDevice:
				if(strlen(src) && strlen(des)){
					res = ppc_login(src,des,&utoken);
				}
				else{
	                username  = "hcy";
	                password = "123456";
	                res = ppc_login(username,password,&utoken);
				}
                if(res != 0){
                    DMCLOG_E("login error");
                    //return res;
                }
				else{
					DMCLOG_E("login success");
				}
                break;
            case DM_LogoutDevice:
                res = ppc_logout(utoken);
                if(res != 0){
                    DMCLOG_E("logout error");
                    //return res;
                }
				else{
					DMCLOG_E("logout success");
				}
                break;
            case DM_DGetVersion:
                break;
            case DM_GetFileList:
				//while(1){
                p_dir = ppc_opendir(src,utoken);
                char file[FILENAME_MAX];
                const char *slash = "/";
                //struct stat st;
				int rewinddir_flag = 0;
                if(p_dir == NULL)
                {
                    DMCLOG_E("open dir %s error",src);
					break;
					//return -1;
                }
                struct dirent *dp = NULL;
                do{
                    if((dp = ppc_readdir(p_dir,utoken)) == NULL)
                    {
                        DMCLOG_D("read finish");
                        break;
                    }
					DMCLOG_D("dp->d_name: %s, dp->d_type: %d", dp->d_name, (int)dp->d_type);
					if(!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")){
						DMCLOG_D("continue");
						continue;
					}
					
                    if(*(src+strlen(src)-1) == '/'){
                        sprintf(file,"%s%s",src,dp->d_name);
                    }else{
                        sprintf(file,"%s%s%s",src,slash,dp->d_name);
                    }
					DMCLOG_D("src: %s, file: %s", src, file);
			
					memset(&st, '\0', sizeof(struct stat));
                    ppc_stat(file,&st,utoken);
                    DMCLOG_D("ppc_stat: size:%lld, mtime:%ld, isDir:%d, st.st_mode:%d, st.st_nlink:%d, st.st_ino:%d, st.st_dev: %d, st.st_uid:%d, st.st_gid:%d, st.st_rdev:%d, st.st_blksize:%d, st.st_blocks:%d, st.st_atime:%d, st.st_ctime:%d",
						st.st_size, st.st_mtime, S_ISDIR(st.st_mode), st.st_mode, st.st_nlink, st.st_ino, st.st_dev, st.st_uid, st.st_gid, st.st_rdev, st.st_blksize, st.st_blocks, st.st_atime, st.st_ctime);
					#if 0
					memset(&st, '\0', sizeof(struct stat));
	                ppc_lstat(file,&st,utoken);
                    DMCLOG_D("ppc_lstat: size = %lld,mtime = %ld,isDir = %d,st.st_mode = %d",st.st_size,st.st_mtime,S_ISDIR(st.st_mode),st.st_mode);

					int fd = ppc_open(file, O_RDONLY, 0, utoken);
					if(fd > 0){
						memset(&st, '\0', sizeof(struct stat));
						ppc_fstat(fd,&st,utoken);					
                    	DMCLOG_D("ppc_fstat: size = %lld,mtime = %ld,isDir = %d,st.st_mode = %d",st.st_size,st.st_mtime,S_ISDIR(st.st_mode),st.st_mode);
						ppc_close(fd);
					}
					else{
						DMCLOG_D("open %s error", file);
					}
	
					off_t telldir_offset = ppc_telldir(p_dir, utoken);				
					DMCLOG_D("1-ppc_telldir: %lld", telldir_offset);
					off_t seek_num = telldir_offset + 2;
					ppc_seekdir(p_dir, seek_num, utoken);

					telldir_offset = ppc_telldir(p_dir, utoken);	
					DMCLOG_D("2-ppc_telldir: %lld", telldir_offset);
					
					if(!rewinddir_flag){
						rewinddir_flag = 1;
						ppc_rewinddir(p_dir, utoken);
						telldir_offset = ppc_telldir(p_dir, utoken);
						DMCLOG_D("3-ppc_telldir: %lld", telldir_offset);
					}
					#endif
                }while(dp != NULL);
                
                ppc_closedir(p_dir,utoken);
				//DMCLOG_D("test_num: %d\r\n\r\n\r\n", i++);
				//sleep(3);
				//}
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
				res = ppc_rename(src, des, utoken);
				if(!res){
					DMCLOG_D("%s rename to %s success", src, des);
				}
				else{
					DMCLOG_D("%s rename to %s fail", src, des);
				}				
                break;
            case DM_Delete:
				res = ppc_unlink(src, utoken);
				if(!res){
					DMCLOG_D("DELETE %s success", src);
				}
				else{
					DMCLOG_D("DELETE %s fail", src);
				}
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
			case DM_FileFSeek:
            {
                DMCLOG_D("src = %s",src);
                DMCLOG_D("des = %s",des);
                test_fseek_ftell(src, des,utoken);
            }
                break;
			case DM_FileWrite:
            {
                DMCLOG_D("src = %s",src);
                DMCLOG_D("des = %s",des);
				#if 1
				test_write(src, des, utoken);
				#else
				while(1){
					memset(des_2, 0, sizeof(des_2));
					sprintf(des_2, "%s_%d", des, i++);
                	test_write(src, des_2, utoken);
					DMCLOG_D("des_2: %s", des_2);
					sleep(2);
				}
				#endif
            }
                break;
            case DM_FileRead:
            {
                DMCLOG_D("src = %s",src);
                DMCLOG_D("des = %s",des);
                test_read(src, des,utoken);
            }
                break;
			case DM_FilePWrite:
            {
                DMCLOG_D("src = %s",src);
                DMCLOG_D("des = %s",des);
                test_pwrite(src, des, utoken);
            }
                break;
            case DM_FilePRead:
            {
                DMCLOG_D("src = %s",src);
                DMCLOG_D("des = %s",des);
                test_pread(src, des,utoken);
            }
                break;
			case DM_FileWritev:
            {
                DMCLOG_D("src = %s",src);
                DMCLOG_D("des = %s",des);
                test_writev(src, des, utoken);
            }
                break;
			case DM_FileSeek:
            {
                DMCLOG_D("src = %s",src);
                DMCLOG_D("des = %s",des);
                test_seek_tell(src, des,utoken);
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
                //struct stat st;
                if(p_dir == NULL)
                {
                    DMCLOG_E("open dir %s error",src);
					break;
					//return -1;
                }
                struct dirent *dp = NULL;
                do{
                    if((dp = ppc_readdir(p_dir,utoken)) == NULL)
                    {
                        DMCLOG_D("read finish");
                        break;
                    }
					DMCLOG_D("dp->d_name: %s, dp->d_type: %d", dp->d_name, (int)dp->d_type);
					if(!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")){
						DMCLOG_D("continue");
						continue;
					}
					
                   	if(*(src+strlen(src)-1) == '/'){
                        sprintf(file,"%s%s",src,dp->d_name);
                    }else{
                        sprintf(file,"%s%s%s",src,slash,dp->d_name);
                    }
					DMCLOG_D("src: %s, file: %s", src, file);

					memset(&st, 0, sizeof(struct stat));
                    ppc_stat(file,&st,utoken);
                    DMCLOG_D("size = %lld,mtime = %ld,isDir = %d,st.st_mode = %d",st.st_size,st.st_mtime,S_ISDIR(st.st_mode),st.st_mode);

					off_t telldir_offset = ppc_telldir(p_dir, utoken);				
					DMCLOG_D("ppc_telldir: %lld", telldir_offset);
					off_t seek_num = telldir_offset + 2;
					ppc_seekdir(p_dir, seek_num, utoken);
                }while(dp != NULL);
                
                ppc_closedir(p_dir,utoken);
				ppc_close(fd);
			}
				break;
			case DM_Cd:
				res = ppc_chdir(src, utoken);
				if(res){
					DMCLOG_D("change dir(%s) fail", src);
				}
				else{
					DMCLOG_D("change dir(%s) success", src);
				}
				break;
			case DM_Pwd:
				memset(tmp_str, 0, sizeof(tmp_str));
				if(ppc_getwd(tmp_str, utoken) != NULL){
					DMCLOG_D("getwd success: %s", tmp_str);
				}
				else{
					DMCLOG_D("getwd fail", tmp_str);
				}	
				break;
			case DM_Ftruncate:
				res = ppc_ftruncate(src, atoi(des), utoken);
				if(!res){
					DMCLOG_D("ftruncate(%s %s) success", src, des);
				}
				else{
					DMCLOG_D("ftruncate(%s %s) fail", src, des);
				}	
				break;
			case DM_Fallocate:
				res = ppc_fallocate(src, 0, 0, atoi(des), utoken);
				if(!res){
					DMCLOG_D("fallocate(%s %s) success", src, des);
				}
				else{
					DMCLOG_D("fallocate(%s %s) fail", src, des);
				}	
				break;
			case DM_Symlink:
				res = ppc_symlink(src, des, utoken);
				if(!res){
					DMCLOG_D("symlink(%s %s) success", src, des);
				}
				else{
					DMCLOG_D("symlink(%s %s) fail", src, des);
				}	
				break;
			case DM_Link:
				res = ppc_link(src, des, utoken);
				if(!res){
					DMCLOG_D("link(%s %s) success", src, des);
				}
				else{
					DMCLOG_D("link(%s %s) fail", src, des);
				}	
				break;
			case DM_Readlink:
				memset(tmp_str, 0, sizeof(tmp_str));
				res = ppc_readlink(src, tmp_str, sizeof(tmp_str), utoken);
				if(!res){
					DMCLOG_D("readlink(%s %s) success, ", src, tmp_str);
				}
				else{
					DMCLOG_D("link(%s) fail", src);
				}	
				break;
			case DM_Statvfs:
				memset(&statvfs_buf, 0, sizeof(struct statvfs));
				DMCLOG_D("src: %s", src);
				res = ppc_statvfs(src, &statvfs_buf, utoken);
				if(!res){
				    DMCLOG_D("Total:%s", byte_size_to_string(get_vfs_size(&statvfs_buf,TOTAL_SIZE)));
				    DMCLOG_D("Used :%s", byte_size_to_string(get_vfs_size(&statvfs_buf,USED_SIZE)));
				    DMCLOG_D("Avail:%s", byte_size_to_string(get_vfs_size(&statvfs_buf,AVAIL_SIZE)));
				}
				else{
					DMCLOG_D("ppc_statvfs(%s) fail", src);
				}	
				break;
			case DM_Utimensat:
				memset(&times, 0, sizeof(struct timespec)*2);
				times[0].tv_nsec = UTIME_NOW;
				times[1].tv_nsec = UTIME_NOW;
				res = ppc_utimensat(0, src, times, 0, utoken);
				if(!res){
					DMCLOG_D("utimensat(%s) success, ", src);
				}
				else{
					DMCLOG_D("utimensat(%s) fail", src);
				}

				memset(&st, 0, sizeof(struct stat));
				ppc_stat(src,&st,utoken);
				DMCLOG_D("size = %lld,mtime = %ld,isDir = %d,st.st_mode = %d",st.st_size,st.st_mtime,S_ISDIR(st.st_mode),st.st_mode);

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

	res = ppc_uninitialise();
	if(res){
		DMCLOG_E("uninitialise fail");
		return -1;
	}
	else{
		DMCLOG_D("uninitialise success");
	}
	
	return 0;
}

