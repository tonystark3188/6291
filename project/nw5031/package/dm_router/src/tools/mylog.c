/*
 * =============================================================================
 *
 *       Filename:  mylog.c
 *
 *    Description:  implementate log tool for linux.
 *
 *        Version:  1.0
 *        Created:  2015/03/19 11:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver, 
 *   Organization:  
 *
 * =============================================================================
 */

#include "mylog.h"
#include "file_opr.h"
#include "time_opr.h"
#include "base.h"
#include "my_debug.h"


#define MAX_LOG_BUF_SIZE (16*1024) // 16K

#ifdef DEBUG
#define SUPPORT_STD_OUTPUT
#else
#undef SUPPORT_STD_OUTPUT
#endif


struct MyLogger
{
	char file_name[256];
	int log_fd;
	MyLogLevel ll;
	char *log_buf;
	size_t buf_size;
	size_t buf_len;
#ifdef SUPPORT_MUTIL_PTHREAD
	pthread_mutex_t lock;
#endif

};

struct MyLogLevelDesc{
	MyLogLevel l;
	char *desc;
};


static struct MyLogLevelDesc mll_desc[MLL_ERROR] = {
	{.l = MLL_DEBUG, 	.desc = "DEBUG"},
	{.l = MLL_TRACE, 	.desc = "TRACE"},
	{.l = MLL_NOTICE, 	.desc = "NOTICE"},
	{.l = MLL_WARNING, 	.desc = "WARNING"},
	{.l = MLL_ERROR, 	.desc = "ERROR"},
};

static struct MyLogger *my_log = NULL;


static int generate_log_file_name(const char *file, char *log_file_name, size_t size)
{
	struct tm tv;
	get_time_tm(&tv);
	snprintf(log_file_name, size, "%s-%04d%02d%02d.log", file, tv.tm_year, tv.tm_mon, tv.tm_mday);
	return 0;
}

static int is_log_file_exist(const char *log_file)
{
	if(access(log_file, F_OK) == 0)
	{
		return 1;
	}

	return 0;
}

static int remove_old_log_file(const char *file)
{
	DIR *p_dir = NULL;
	struct dirent *dir_item = NULL;
	char dir_name[128] = {0};
	char short_file_name[128] = {0};
	char tmp_short_file_name[128] = {0};
	size_t len = strlen(file)-1;
	struct tm tv;
	char cur_time[16] = {0}; // yyyy-mm-dd
	char old_log_files[4][128];
	char old_log_file_full_path[128] = {0};
	size_t count = 0;
	int i = 0;
	
	get_time_tm(&tv);
	snprintf(cur_time, sizeof(cur_time), "%04d%02d%02d", tv.tm_year, tv.tm_mon, tv.tm_mday);

	while(len > 0)
	{
		if(file[len] == '/')
			break;
		--len;
	}

    if(file[len] == '/')
    {
	    strncpy(short_file_name, file+len+1, sizeof(short_file_name));
	    memcpy(dir_name, file, len+1);
    }
    else
    {
        strncpy(short_file_name, file, sizeof(short_file_name));
	    //memcpy(dir_name, file, len);
        dir_name[0] = '.';
    }
	//DMCLOG_D("%s():dir_name = %s, file_name= %s\n", __FUNCTION__, dir_name, short_file_name);

	p_dir = opendir(dir_name);
	if(p_dir == NULL)
	{
		DMCLOG_D("%s(): open dir failed!\n", __FUNCTION__);
		return -1;
	}

	while((dir_item = readdir(p_dir)) != NULL)
	{
		if(dir_item->d_type & DT_DIR)
			continue;

		DMCLOG_D("dir_item name = %s\n", dir_item->d_name);	
		if(strncmp(dir_item->d_name, short_file_name, strlen(short_file_name)) == 0)
		{
			snprintf(tmp_short_file_name, sizeof(tmp_short_file_name), 
                    "%s-%s.log", short_file_name, cur_time);
			if(strcmp(tmp_short_file_name, dir_item->d_name))
			{
				strncpy(old_log_files[count], dir_item->d_name, 128);
				++count;
				if(count >= 4)
					break;
			}
		}
	}	
	closedir(p_dir);

	if(count > 0)
	{
		for(i = 0; i < count; ++i)
		{
			DMCLOG_D("%s\n", old_log_files[i]);
			snprintf(old_log_file_full_path, sizeof(old_log_file_full_path), 
                    "%s/%s", dir_name, old_log_files[i]);
			remove(old_log_file_full_path);
		}
	}

	return 0;
}

/* 
 *Desc: initialization Logger  
 *
 *file: input, full path
 *mll: input, the lowest log level
 *size: input, inside buffer size. If we set it 0, we will use default size.
 *Return: success return 0, else return negate value.
 */
static int mylog_init(const char *file, MyLogLevel mll, size_t size)
{
	size_t buf_size = size;
	char log_file_name[256] = {0};

	generate_log_file_name(file, log_file_name, sizeof(log_file_name));
	if(is_log_file_exist(log_file_name))
	{
		remove_old_log_file(file);
	}


	my_log = (struct MyLogger *)malloc(sizeof(struct MyLogger));
	if(my_log == NULL)
	{
		//DMCLOG_D("%s(): malloc failed!\n", __FUNCTION__);
		return -1;	
	}
	memset(my_log, 0, sizeof(struct MyLogger));

	if(size == 0)
	{
		buf_size = MAX_LOG_BUF_SIZE;
	}
    else 
    {
        buf_size = (size << 10);
    }
	
	my_log->log_buf = (char *)malloc(buf_size);
	if(my_log->log_buf == NULL)
	{
		//DMCLOG_D("%s(): malloc failed 2!\n", __FUNCTION__);
		safe_free(my_log);
		return -2;	
	}
	my_log->buf_size = buf_size;
	my_log->buf_len = 0;

	
	strncpy(my_log->file_name, log_file_name, sizeof(my_log->file_name));
	my_log->ll = mll;
	
	my_log->log_fd = open(log_file_name, O_RDWR | O_CREAT | O_APPEND, 0666);
	if(my_log->log_fd < 0)
	{
		//DMCLOG_D("%s(): open %s failed!\n", __FUNCTION__, file);
		safe_free(my_log->log_buf);
		safe_free(my_log);	
		return -3;	
	}

	memset(my_log->log_buf, 0, buf_size);
	return 0;
}

#ifdef SUPPORT_MUTIL_PTHREAD
static int mylog_init_pthread_safe(const char *file, MyLogLevel mll, size_t size)
{
	if(mylog_init(file, mll, size) < 0)
		return -1;
	
	if(pthread_mutex_init(&my_log->lock, NULL) != 0)
	{
		//DMCLOG_D("%s(): init mutex failed\n", __FUNCTION__);
		return -2;	
	}

	return 0;
}
#endif
/* 
 *Desc: exit my log tool.
 *
 *Return: always return 0. 
 */
static int mylog_exit(void)
{
	if((my_log != NULL) && (my_log->log_fd > 0))
	{
		if(my_log->buf_len > 0)
		{
			writen(my_log->log_fd, my_log->log_buf, my_log->buf_len);				
		}
		safe_close(my_log->log_fd);
		safe_free(my_log->log_buf);
		safe_free(my_log);
	}

	return 0;
}

#ifdef SUPPORT_MUTIL_PTHREAD
static int mylog_exit_pthread_safe(void)
{
	if(my_log == NULL)
		return 0;
	
	pthread_mutex_lock(&my_log->lock);
	mylog_exit();
	pthread_mutex_unlock(&my_log->lock);

	pthread_mutex_destroy(&my_log->lock);
	//my_log->lock = NULL;

	return 0;
}
#endif


static const char *mll2str(MyLogLevel l)
{
	return mll_desc[l-1].desc;
}



/*******************************************************************************
 * ****************   public   function     ************************************
 * *****************************************************************************
*/
int log_init(const char *file, MyLogLevel mll, size_t size)
{
#ifdef SUPPORT_MUTIL_PTHREAD
	return mylog_init_pthread_safe(file, mll, size);
#else
	return mylog_init(file, mll, size);
#endif
}

int log_exit(void)
{
#ifdef SUPPORT_MUTIL_PTHREAD
	return mylog_exit_pthread_safe();
#else
	return mylog_exit();
#endif
}


/* 
 *Desc: log msg to buffer.
 */
int log_msg(MyLogLevel l, char *logfmt, ...)
{
	char cur_time[32] = {0};//xxxx-xx-xx xx:xx:xx
	int _size = 0;
    size_t threshold = 0;

	if((my_log == NULL) || (my_log->log_fd <= 0))
		return -1;
	
	if(l < my_log->ll)
		return 0;

#ifdef SUPPORT_MUTIL_PTHREAD
	pthread_mutex_lock(&my_log->lock);
#endif

    threshold = (my_log->buf_size >> 1);
    threshold += (my_log->buf_size >> 2);
	if(my_log->buf_len > threshold)
	{
		size_t write_bytes = 0;
		write_bytes = writen(my_log->log_fd, my_log->log_buf, my_log->buf_len);	
		if(write_bytes <= my_log->buf_len)
			my_log->buf_len -= write_bytes;
		else
			my_log->buf_len = 0;
	}
	

#ifdef SUPPORT_STD_OUTPUT
	size_t offset = my_log->buf_len;
#endif

	get_time_str(cur_time, sizeof(cur_time));
	_size = snprintf(my_log->log_buf + my_log->buf_len, 
             my_log->buf_size-my_log->buf_len, "%s [%s]", mll2str(l), cur_time);
	my_log->buf_len += _size;

	_size = 0;
	va_list args;
	va_start(args, logfmt);
	_size = vsnprintf(my_log->log_buf + my_log->buf_len, 
                        my_log->buf_size-my_log->buf_len, logfmt, args);
	va_end(args);
	my_log->buf_len += _size;

#ifdef SUPPORT_STD_OUTPUT
	DMCLOG_D("%s", my_log->log_buf+offset);
#endif
	
#ifdef SUPPORT_MUTIL_PTHREAD
	pthread_mutex_unlock(&my_log->lock);
#endif

	return 0;	
} 

int log_sync(void)
{
	if((my_log == NULL) || (my_log->log_fd <= 0))
		return -1;

#ifdef SUPPORT_MUTIL_PTHREAD
	pthread_mutex_lock(&my_log->lock);
#endif
	
	if(my_log->buf_len > 0)
	{
		size_t write_bytes = 0;
		write_bytes = writen(my_log->log_fd, my_log->log_buf, my_log->buf_len);	
		if(write_bytes <= my_log->buf_len)
			my_log->buf_len -= write_bytes;
		else
			my_log->buf_len = 0;
	}

#ifdef SUPPORT_MUTIL_PTHREAD
	pthread_mutex_unlock(&my_log->lock);
#endif

	return 0;	
}


