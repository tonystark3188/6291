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
 * Desc: is dir exist.
 * 
 * dir_path: input,
 * Return: -1: means the dir_path exist, but it is not dir, maybe it is regular file.
 *          0: means dir not exist.
 *          1: means dir exist.
 */
int is_dir_exist(const char *dir_path)
{
    struct stat file_stat;

    if(stat(dir_path, &file_stat) < 0)
    {
        if(errno == ENOENT)
            return 0;

        return -1;
    }

    if(S_ISDIR(file_stat.st_mode))
        return 1;

    return -1;
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


// Print a warning message if opendir() fails, but don't die.
DIR* warn_opendir(const char *path)
{
	DIR *dp;
	dp = opendir(path);
	if (!dp)
		printf("can't warn_opendir '%s',errno = %d", path,errno);
	return dp;
}
char* bb_basename(char *name)
{
	char *cp = strrchr(name, '/');
	if (cp)
		return (cp + 1);
	return name;
}

void display_files(struct file_dnode *dn)
{
    unsigned i = 0;
    for (i = 0; /* i < nfiles - detected via !dn below */; i++) {
		 if (!dn)
            break;
        DMCLOG_D("i = %d,filePath = %s,utime = %lu",i,dn->name,dn->date);
        dn = dn->dn_next;
    }

}

void dfree(struct file_dnode **dnp)
{
	unsigned i;

	if (dnp == NULL)
		return;

	for (i = 0; dnp[i]; i++) {
		struct file_dnode *cur = dnp[i];
		if (cur->fname_allocated)
			free((char*)cur->fullname);
		free(cur);
	}
	free(dnp);
}

struct file_dnode **dnalloc(unsigned num)
{
	if (num < 1)
		return NULL;

	num++; /* so that we have terminating NULL */
	return xzalloc(num * sizeof(struct file_dnode *));
}
