
#include "rfsvfs.h"

static pthread_mutex_t mutex;
static char *dir_path = NULL;
static char *param_file_path = NULL;


/**
 * 流水号
 */
static u_int64_t serial_number = -2;

static char separator = '/';
static char separator_str[2]={0};

/**
 * 全路径默认最大长度
 */
#define RFSVFS_ALL_PATH_LENGTH_DEFAULT 128

/**
 * 模块初始化主目录路径或磁盘路径
 */
int rfsvfs_init(char *path)
{
    if(path != NULL)
    {

    	/**
    	 * 释放原有的内存
    	 */
    	rfsvfs_destroy();
    	/**
    	 * 分配新的内存
    	 */
    	//检查路径是否正确
#ifdef   _WIN32
    	separator = '\\';
#endif
    	separator_str[0] = separator;
    	int len = strlen(path);
    	//添加末尾的分隔符
    	if(path[len - 1] != separator)
    	{
        	dir_path = (char*)malloc(len + 1);
        	memset(dir_path,0,len + 1);
        	strcpy(dir_path,path);
    	}
    	else
    	{
        	dir_path = (char*)malloc(len );
        	memset(dir_path,0,len );
        	strncpy(dir_path,path,len - 1);
    	}

    	len = strlen(dir_path);
    	int file_len = strlen(RFSVFS_PARAM_FILENAME_DEFAULT) + len;
    	param_file_path = (char*)malloc(file_len + 2);
    	memset(param_file_path,0,file_len + 2);
    	strcpy(param_file_path,dir_path);
    	strcat(param_file_path,separator_str);
    	strcat(param_file_path,RFSVFS_PARAM_FILENAME_DEFAULT);

    	printf("dir_path = %s \n",dir_path);
    	printf("file_path = %s \n",param_file_path);
    	/**
    	 * 获取当前参数
    	 */
    	FILE *fp = fopen(param_file_path,"r");
    	if(fp)
    	{
    		size_t ret =  fread(&serial_number, sizeof(serial_number), 1, fp);

    		if(ret)
    		{
    			printf("serial_number = %ld \n",serial_number);
    		}
    		fclose(fp);
    	}
    	else
    	{
    		//没有初始化文件，需要支持初始化
    		serial_number = -1;
    		printf("Param file is not found ! serial_number = %ld . \n",serial_number);
    	}
    }
	return 0;
}

void rfsvfs_destroy()
{
	if(dir_path != NULL)
	{
		free(dir_path);
		dir_path = NULL;
	}
	if(param_file_path != NULL)
	{
		free(param_file_path);
		param_file_path = NULL;
	}
}


int check_env()
{
	if((dir_path == NULL) || (param_file_path == NULL) || (serial_number < 0))
	{
		/**
		 * 当前模块没有初始化或初始化失败
		 */
		return -1;
	}
	return 0;
}

/**
 * 保存当前变量到文件中
 */
int save_serial_number_to_file(u_int64_t tmp_number)
{
	printf("param_file_path = %s\n",param_file_path);
	FILE *fp = fopen(param_file_path,"w");
	if(fp)
	{
		size_t ret =  fwrite(&tmp_number, sizeof(tmp_number), 1, fp);
		fclose(fp);
		if(ret)
		{
//   			printf("tmp_number = %ld \n",tmp_number);
			return 0;
		}
	}
	else
	{
		//没有初始化文件，需要支持初始化
		printf("Cannot file write![error] \n");
	}
	return -1;
}

/**
 * 获取一个新的路径
 */
int get_new_path(char pathBuf[RFSVFS_PATH_LENGTH_DEFAULT])
{
	int ret = 0;
	u_int64_t tmpNum = 0;

	//这里需要线程同步
	//todo


	pthread_mutex_lock(&mutex);
	{
		tmpNum = ++serial_number;
		if(save_serial_number_to_file(tmpNum))
		{
			ret = -2;
		}

	}
	pthread_mutex_unlock(&mutex);

	if(!ret)
	{
		//根据num算出对应的路径
		int major_num = ( tmpNum / RFSVFS_MAJOR_DIV ) % RFSVFS_MAJOR_DIR_SIZE;
		int minor_num = ( tmpNum / RFSVFS_MINOR_DIV ) % RFSVFS_MINOR_DIR_SIZE;
		int file_num = tmpNum % RFSVFS_FILE_DIR_SIZE;


		memset(pathBuf,0,RFSVFS_PATH_LENGTH_DEFAULT);
		sprintf(pathBuf,"%c%d%c%d%c%d",separator,major_num,separator,minor_num,separator,file_num);
	}
	return ret;
}

/**
 * 获取一个未使用的文件路径
 */
int rfsvfs_get_new_file_path(char pathBuf[RFSVFS_PATH_LENGTH_DEFAULT])
{
	//判断当前模块是否正常初始化
	if(check_env())
	{
		return -1;
	}

	/**
	 * 路径不能为空
	 */
	if(pathBuf == NULL)
		return -2;

	//获取当前新路径
	return get_new_path(pathBuf);
}

int get_parent_dir_path(char parent_dir_path[RFSVFS_ALL_PATH_LENGTH_DEFAULT],char* path)
{
	strcpy(parent_dir_path,path);
	char *p =  strrchr(parent_dir_path, separator);
	if(p)
	{
		p[0] = '\0';
//		printf("parent_dir_path = %s.\n",parent_dir_path);
		return 0;
	}
	return -1;
}
/**
 * 这里会循环遍历创建
 */
int create_dir(char *dir_path)
{
	int ret =  mkdir(dir_path,S_IRWXU|S_IRWXG|S_IRWXO);
	if(ret)
	{
		char parent_dir[RFSVFS_ALL_PATH_LENGTH_DEFAULT]={0};
		if(!get_parent_dir_path(parent_dir,dir_path))
		{
			if(!create_dir(parent_dir))
			{
				ret =  mkdir(dir_path,S_IRWXU|S_IRWXG|S_IRWXO);
			}
			else
			{
				return -2;
			}
		}
		else
		{
			ret = -1;
		}
	}

	return ret;
}

int get_all_file_path(char all_file_path[RFSVFS_ALL_PATH_LENGTH_DEFAULT],const char sub_file_path[RFSVFS_PATH_LENGTH_DEFAULT])
{
	memset(all_file_path,0,RFSVFS_ALL_PATH_LENGTH_DEFAULT);
	strcpy(all_file_path,dir_path);
	strcat(all_file_path,sub_file_path);
	return 0;
}

FILE* rfsvfs_fopen(const char * path,const char * mode)
{
	char all_path[RFSVFS_ALL_PATH_LENGTH_DEFAULT]={0};
	get_all_file_path(all_path,path);
	printf("file_path is %s.\n",all_path);
	FILE *fp = fopen(all_path,mode);
	if(fp == NULL)
	{
		char parent_dir[RFSVFS_ALL_PATH_LENGTH_DEFAULT]={0};
		if(!get_parent_dir_path(parent_dir,all_path))
		{
			if(!create_dir(parent_dir))
			{
				return rfsvfs_fopen(path,mode);
			}
			else
			{
				printf("create_dir failed![error]\n");
			}
		}
		else
		{
			printf("get_parent_dir_path failed![error]\n");
		}

		printf("fopen failed![error]\n");
	}

	return fp;
}

/**
 * 文件读取方法
 */
size_t rfsvfs_fread( void *buffer, size_t size, size_t count, FILE *fp)
{
	return fread(buffer,size,count,fp);
}

/**
 * 文件写方法
 */
size_t rfsvfs_fwrite(const void* buffer, size_t size, size_t count, FILE* fp)
{
	return fwrite(buffer,size,count,fp);
}

/**
 * 文件指针偏移
 * fromwhere（偏移起始位置：文件头0(SEEK_SET)，当前位置1(SEEK_CUR)，文件尾2(SEEK_END)）为基准，偏移offset（指针偏移量）个字节的位置。
 */
int rfsvfs_fseek(FILE *fp, long offset, int fromwhere)
{
	return fseek(fp, offset,fromwhere);
}


/**
 * 关闭文件指针
 */
int rfsvfs_fclose(FILE *fp)
{
	return fclose(fp);
}

/**
 * ??????????
 */
int rfsvfs_fstat(const char * path,struct stat *stp)
{
	char all_path[RFSVFS_ALL_PATH_LENGTH_DEFAULT]={0};
	get_all_file_path(all_path,path);
	printf("file_path is %s\n",all_path);
	return (stat(all_path, stp));
}


/**
 * 删除文件
 */
int rfsvfs_remove(char* file_path)
{
	char all_path[RFSVFS_ALL_PATH_LENGTH_DEFAULT]={0};
	get_all_file_path(all_path,file_path);
	printf("rfsvfs_remove file_path = %s\n", all_path);
	return remove(all_path);
}

int rfsvfs_open(const char * path,int flags,mode_t mode)
{
	char all_path[RFSVFS_ALL_PATH_LENGTH_DEFAULT]={0};
	printf("path = %s\n",path);
	get_all_file_path(all_path,path);
	printf("all path is %s\n",all_path);
	int fd = open(all_path,flags,mode);
	if(fd <= 0)
	{
		char parent_dir[RFSVFS_ALL_PATH_LENGTH_DEFAULT]={0};
		if(!get_parent_dir_path(parent_dir,all_path))
		{
			if(!create_dir(parent_dir))
			{
				return rfsvfs_open(path,flags,mode);
			}
			else
			{
				printf("create_dir failed![error:%d]\n",errno);
			}
		}
		else
		{
			printf("get_parent_dir_path failed![error:%d]\n",errno);
		}

		printf("fopen failed![error]\n");
	}
	return fd;
}

/**
 * 文件读取方法
 */
ssize_t rfsvfs_read( int fd,void *buf,int nbyte)
{
	return read(fd,buf,nbyte);
}

/**
 * 文件写方法
 */
ssize_t rfsvfs_write(int fd,void *buf,int nbyte)
{
	return write(fd,buf,nbyte);
}

/**
 * 文件指针偏移
 * fromwhere（偏移起始位置：文件头0(SEEK_SET)，当前位置1(SEEK_CUR)，文件尾2(SEEK_END)）为基准，偏移offset（指针偏移量）个字节的位置。
 */
off_t rfsvfs_lseek(int fd, off_t offset, int whence)
{
	return lseek(fd, offset,whence);
}

/**
 * ?ж????????????
 * ???δ????????0
 * ???????????1
 */
int rfsvfs_feof(FILE* fp)
{
	if(fp == NULL)
		return -1;
	return feof(fp);
}



/**
 * 关闭文件指针
 */
int rfsvfs_close(int fd)
{
	return close(fd);
}




