/*
 * =============================================================================
 *
 *       Filename:  file_opr.c
 *
 *    Description:  file base operation
 *    
 *
 *        Version:  1.0
 *        Created:  2015/3/19 11:30
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver
 *   Organization:  
 *
 * =============================================================================
 */
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include "file_opr.h"


/*
 *Desc: write count bytes data to file
 *
 *fd: input, 
 *buf: input,
 *count: data len
 *Return: the bytes we have written. 
 */
ssize_t writen(int fd, char *buf, size_t count)
{
	ssize_t ret = -1;
	ssize_t write_bytes = 0;

	if(fd < 0)
		return -1;
	
	while(write_bytes < count)
	{
		ret = write(fd, buf+write_bytes, count-write_bytes); 
		if(ret < 0)
		{
			if(errno == EINTR)
				continue;
			return write_bytes; 	
		}

		write_bytes += ret;
	}

	return write_bytes;
}

/*
 *Desc: read specific count bytes .
 *
 *fd: input, 
 *buf: output,
 *count: input, the number of bytes we want to read.
 *Return: return the number of bytes we has readed.
 */
ssize_t readn(int fd, char *buf, size_t count)
{
	ssize_t ret = -1;
	size_t read_bytes = 0;

	while((ret = read(fd, buf+read_bytes, count-read_bytes)) != 0)
	{
		if(ret < 0)
		{
			if(errno == EINTR)
				continue;
			else
				break;
		}

		read_bytes += ret;
	}

	if(ret == 0)
		return read_bytes;
	else if(ret < 0)
		return -1;

	return read_bytes;
}

/*
 * Desc: set fd nonblock
 *
 * fd: input
 * Return: success on 0.
 */
int set_fd_nonblock(int fd)
{
	if(fd < 0)
		return -1;

	int flags = fcntl(fd, F_GETFL, 0);
	if(fcntl(fd, F_SETFL, (flags | O_NONBLOCK)) == -1)
	{
        return -1;
    }
    
	return 0;
}

/*
 * Desc: is file exist.
 * 
 * dir_path: input,
 * Return: -1: means the file_path exist, but it is not dir, maybe it is regular file.
 *          0: means file not exist.
 *          1: means file exist.
 */
int is_file_exist_ext(const char *file_path)
{
    struct stat file_stat;

    if(stat(file_path, &file_stat) < 0)
    {
        if(errno == ENOENT)
            return 0;
        
        return -1;
    }

    if(S_ISREG(file_stat.st_mode) || S_ISLNK(file_stat.st_mode))
        return 1;

    return -1;
}

char* bb_basename(char *name)
{
	char *cp = strrchr(name, '/');
	if (cp)
		return (cp + 1);
	return name;
}

int cp_file(const char *src_file, const char *dest_file)
{
    char buf[DEFAULT_PAGE_SIZE];
    ssize_t ret;
    ssize_t read_bytes;
    ssize_t write_bytes;
    
    int src_fd = open(src_file, O_RDONLY);
    if(src_fd < 0)
    {
        DMCLOG_E("open src(%s) failed", src_file);
        return EOPEN;
    }

    int dest_fd = open(dest_file, O_RDWR | O_TRUNC | O_CREAT, FILE_DEF_MODE);
    if(dest_fd < 0)
    {
        DMCLOG_E("open dest(%s) failed", dest_file);
        close(src_fd);
        return EOPEN;
    }

    while(1)
    {
        read_bytes = readn(src_fd, buf, sizeof(buf));
        if(read_bytes < 0)
        {
            ret = EREAD;
            DMCLOG_E("readn failed");
            break;
        }
        else if(read_bytes == 0)
        {
            ret = RET_SUCCESS;
            DMCLOG_E("EOF!");
            break;
        }

        write_bytes = writen(dest_fd, buf, read_bytes);
        if(write_bytes != read_bytes)
        {
            DMCLOG_E("write_byets(%d) != read_bytes(%d)", write_bytes, read_bytes);
            ret = EWRITE;
            break;
        }
    }

    safe_close(src_fd);
    safe_close(dest_fd);
    
    return ret;
}

