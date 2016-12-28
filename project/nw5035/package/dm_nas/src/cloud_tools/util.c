#include<unistd.h>
#include<fcntl.h>
#include<stdio.h>
#include<dirent.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<stdbool.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>

#define CONFIG_FEATURE_COPYBUF_KB 4

void* xmalloc(size_t size)
{
    void *ptr = malloc(size);
    if(ptr == NULL && size != 0)
        printf("bb_msg_memory_exhausted\n");
    return ptr;
}

void* xzalloc(size_t size)
{
    void *ptr = xmalloc(size);
    memset(ptr,0,size);
    return ptr;
}


ssize_t  safe_read(int fd, void *buf, size_t count)
{
    ssize_t n;

    do {
        n = read(fd, buf, count);
    } while (n < 0 && errno == EINTR);

    return n;
}


static ssize_t safe_write(int fd, const void *buf, size_t count)
{
    ssize_t n;

    do {
        n = write(fd, buf, count);
    } while (n < 0 && errno == EINTR);

    return n;
}



ssize_t full_write(int fd, const void *buf, size_t len)
{
    ssize_t cc;
    ssize_t total;

    total = 0;

    while (len) {
        cc = safe_write(fd, buf, len);

        if (cc < 0) {
            if (total) {
	    /* we already wrote some! */
	    /* user can do another write to know the error code */
                return total;
            }
            return cc;  /* write() returns -1 on failure. */
        }
        total += cc;
        buf = ((const char *)buf) + cc;
        len -= cc;
    }

    return total;
}


static char* last_char_is(const char *s, int c)
{
    if (s && *s) {
        size_t sz = strlen(s) - 1;
        s += sz;
        if ( (unsigned char)*s == c)
            return (char*)s;
    }
    return NULL;
}


char* concat_path_file(const char *path, const char *filename)
{
    char *lc;
    char *p;

    if (!path)
        path = "";
    lc = last_char_is(path, '/');
    while (*filename == '/')
                filename++;
    p = (char *)malloc(strlen(path)+strlen(filename)+ 0x10);
	if(p == NULL)
		return NULL;
    sprintf(p,"%s%s%s", path, (lc==NULL ? "/" : ""), filename);

    return p;
}


static char* bb_get_last_path_component_nostrip(const char *path)
{
    char *slash = strrchr(path, '/');

    if (!slash || (slash == path && !slash[1]))
        return (char*)path;

    return slash + 1;
}

/*
 * "/"        -> "/"
 * "abc"      -> "abc"
 * "abc/def"  -> "def"
 * "abc/def/" -> "def" !!
 */
char* bb_get_last_path_component_strip(char *path)
{
    char *slash = last_char_is(path, '/');

    if (slash)
        while (*slash == '/' && slash != path)
            *slash-- = '\0';

    return bb_get_last_path_component_nostrip(path);
}

char* get_last_char(const char *s, int c)
{
	return last_char_is(s, c);
}

int is_file_exist(char *src_path)
{
	if(access(src_path,F_OK)!=0)
		return -1;
	return 0;
}

int touch_file(char *src_path)
{
	int fd = 0;
	fd = open(src_path, O_RDWR | O_CREAT, 0666);
	if (fd >= 0) {
		printf("create src_path = %s success\n",src_path);
		close(fd);
		return 0;
	}
	return -1;
}


int rm(const char *path)
{
    struct stat path_stat;

    if(stat(path,&path_stat) < 0)
    {
    	printf("remove file %s failed,errno = %d\n", path,errno);
        return -1;
    }

    //directory
    if(S_ISDIR(path_stat.st_mode))
    {
        DIR *dp;
        struct dirent *d;
        //int status = 0;

        dp = opendir(path);
        if (dp == NULL)
        {
        	printf("remove file %s failed,errno = %d\n", path,errno);
    	    return -1;
        }

        while((d = readdir(dp)) != NULL)
        {
            char *new_path;

            if(strcmp(d->d_name, ".") == 0
              || strcmp(d->d_name, "..") == 0)
            {
                continue;
            }
 
            new_path = concat_path_file(path, d->d_name);
            if(rm(new_path) < 0)
            {
                printf("remove file %s failed,errno = %d\n", new_path,errno);
                free(new_path);
                closedir(dp);
                return -1;
            }
            free(new_path);
        } 
        closedir(dp);

        if (rmdir(path) < 0)
        {
        	printf("rmdir file %s failed,errno = %d\n", path,errno);
	    	return -1;
        }
        return 0; 
    }
    //regular file
    if (unlink(path) < 0) 
    {
        printf("unlink file %s failed,errno = %d\n", path,errno);
        return -1;
    }
    return 0;
}





	
