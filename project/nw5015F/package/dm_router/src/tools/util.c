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



static off_t bb_full_fd_action(int src_fd, int dst_fd, off_t size)
{
    int status = -1;
    off_t total = 0;
    bool continue_on_write_error;
#if CONFIG_FEATURE_COPYBUF_KB <= 4
    char buffer[CONFIG_FEATURE_COPYBUF_KB * 1024];
    enum { buffer_size = sizeof(buffer) };
#else
    char *buffer;
    int buffer_size;
#endif

    if (size < 0)
    {
        size = -size;
	continue_on_write_error = 1;
    }

#if CONFIG_FEATURE_COPYBUF_KB > 4
    if (size > 0 && size <= 4 * 1024)
        goto use_small_buf;
	/* We want page-aligned buffer, just in case kernel is clever
	 * and can do page-aligned io more efficiently */
    buffer = mmap(NULL, CONFIG_FEATURE_COPYBUF_KB * 1024,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANON,
                     /* ignored: */ -1, 0);
    buffer_size = CONFIG_FEATURE_COPYBUF_KB * 1024;
    if (buffer == MAP_FAILED)
    {
 use_small_buf:
        buffer = alloca(4 * 1024);
        buffer_size = 4 * 1024;
    }
#endif  

    if (src_fd < 0)
        goto out;

    if (!size)
    {
        size = buffer_size;
        status = 1; /* copy until eof */
    }

    while (1)
    {
        ssize_t rd;

        rd = safe_read(src_fd, buffer, size > buffer_size ? buffer_size : size);

        if (!rd) 
        { /* eof - all done */
            status = 0;
            break;
        }
        if (rd < 0)
        {
            break;
        }
        /* dst_fd == -1 is a fake, else... */
        if (dst_fd >= 0) 
        {
            ssize_t wr = full_write(dst_fd, buffer, rd);
            if (wr < rd)
            {
                printf("write error\n");
                break;
            }
	}
        total += rd;
        if (status < 0) 
        { /* if we aren't copying till EOF... */
            size -= rd;
	    if (!size)
            {
	    /* 'size' bytes copied - all done */
                status = 0;
                break;
            }
        }
    }  

out:
#if CONFIG_FEATURE_COPYBUF_KB > 4
    if (buffer_size != 4 * 1024)
        munmap(buffer, buffer_size);
#endif
    return status ? -1 : total;
    
}



off_t bb_copyfd_eof(int fd1, int fd2)
{
	return bb_full_fd_action(fd1, fd2, 0);
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

int copy_file(const char *source, const char *dest)
{
    struct stat source_stat;
    struct stat dest_stat;
    char dest_exist = 0;
    int retval = 1;
    
    if(stat(source, &source_stat) < 0)
    {
        return 0;
    }
    if(stat(dest, &dest_stat) < 0)
    {
        if(errno != ENOENT)
        {
            return 0;
        }
    }
    else
    {
        if (source_stat.st_dev == dest_stat.st_dev
	    && source_stat.st_ino == dest_stat.st_ino)
        {
	    printf("%s and %s are the same file\n", source, dest);
            return 0;
        }
        dest_exist = 1;
    }

    //copy dir
    if(S_ISDIR(source_stat.st_mode))
    {
        DIR *dp;
        struct dirent *d;
        mode_t saved_umask = 0;
        printf("dest_exist = %d\n",dest_exist);
        if(dest_exist)
        {
            if(!S_ISDIR(dest_stat.st_mode))
            {
                printf("target %s is not a directory\n", dest);
            }
        }
        else //create dest dir
        {
            mode_t mode;
            saved_umask = umask(0);
            
            mode = source_stat.st_mode;
            mode |= S_IRWXU;
            if (mkdir(dest, mode) < 0)
            {
                umask(saved_umask);
                return 0;
            }
            umask(saved_umask);
        }

        dp = opendir(source);
        if(dp == NULL)
        {
            return 0;
        }

        while((d = readdir(dp)) != NULL)
        {
            char *new_source, *new_dest;
    
            if(strcmp(d->d_name, ".") == 0 
               || strcmp(d->d_name, "..") == 0)
            {
                continue;
            }
           
            new_source = concat_path_file(source, d->d_name);
            if(new_source == NULL)
            {
                continue;
            }
            new_dest = concat_path_file(dest, d->d_name);
            if(new_dest == NULL)
            {
                continue;
            }
            copy_file(new_source, new_dest);
            if(copy_file(new_source, new_dest) == 0)
            {
                printf("copy %s to %s failed\n", new_source, new_dest);
                free(new_source);
                free(new_dest);
                closedir(dp);
                return 0;               
            }
            free(new_source);
            free(new_dest);
        }
        closedir(dp);
    }

    //copy regular file
    if(S_ISREG(source_stat.st_mode))
    {
        int src_fd;
        int dst_fd;
        mode_t new_mode;

        src_fd = open(source, O_RDONLY, 0666);
        if(src_fd < 0)
        {
            printf("cannot open source file:%s\n", source);
            return 0;
        }
        new_mode = source_stat.st_mode;
        if (!S_ISREG(source_stat.st_mode))
        {
            new_mode = 0666;
        }
        dst_fd = open(dest, O_WRONLY|O_CREAT, new_mode);
        if(dst_fd < 0)
        {
            printf("cannot open dest file:%s,errno = %d\n", dest,errno);
            return 0;
        }
        
        if(bb_copyfd_eof(src_fd, dst_fd) == -1)
        {
            retval = 0;
        }
        if(close(dst_fd) < 0)
        {
            retval = 0;
        }
        close(src_fd);
        if (!S_ISREG(source_stat.st_mode))
            return retval;
    }
    
    return retval;
    
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

int cp_mv_stat(const char *fn, struct stat *fn_stat)
{
    if(stat(fn, fn_stat) < 0)
    {
        if(errno != ENOENT)
        {
            return -1;
        }
        return 0;
    }    
    if(S_ISDIR(fn_stat->st_mode))
    {
        return 3;
    }

    return 1;
}




int cp(char *source_file, const char *dest_file)
{
    struct stat source_stat;
    struct stat dest_stat;
    const char *last;
    const char *dest;
    //char source[80];
    int s_flag;
    int d_flag;
	int malloc=1;

    last = dest_file;
    s_flag = cp_mv_stat(source_file, &source_stat);
    if(s_flag <= 0)
    {
        return 0;
    }
    d_flag = cp_mv_stat(dest_file, &dest_stat);
    if(d_flag < 0)
    {
        return 0;
    }

    if( !((s_flag | d_flag)&2)   
      || ((s_flag & 2) && !d_flag))
    {
        dest = dest_file;
		malloc = 0;
        goto do_copy;
    }

    //strcpy(source,source_file);
    dest = concat_path_file(last, bb_get_last_path_component_strip(source_file));
do_copy:
    if(copy_file((const char*)source_file, dest) == 0)
    {
        if(malloc)
        {
            free((void*)dest);
        }
        return 0;
    }
    
    if(malloc)
    {
        free((void*)dest);
    }
    return 1;
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

int del_file_type(const char *path,int type)
{
    struct stat path_stat;
	int file_type;
    if(stat(path,&path_stat) < 0)
    {
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
			file_type = db_get_mime_type(new_path,strlen(new_path));//TODO Oliver
			if(file_type != type)
			{
				printf("type:%d\n",file_type);
				continue;
			}
            if(unlink(new_path) < 0)
            {
                printf("unlink file %s failed,errno = %d\n", new_path,errno);
                free(new_path);
                closedir(dp);
                return -1;
            }
            free(new_path);
        } 
        closedir(dp);
    }
    return 0;
}


int copy_compute(const char *path,off_t *total_size,unsigned *nfiles)
{
    struct stat path_stat;
    
    if(stat(path,&path_stat) < 0)
    {
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
            if(copy_compute(new_path,total_size,nfiles) < 0)
            {
                printf("remove file %s failed,errno = %d\n", new_path,errno);
                free(new_path);
                closedir(dp);
                return -1;
            }
            free(new_path);
        }
        closedir(dp);
		(*nfiles)++;
		printf("path = %s,nfiles = %d\n",path,*nfiles);
        return 0;
    }
    //regular file
    (*nfiles)++;
	printf("path = %s,nfiles = %d\n",path,*nfiles);
    *total_size += path_stat.st_size;
    return 0;
}

int mv(char *source, const char *dest)
{
    if(!cp(source, dest))
    {
        printf("cp error\n");
        return -1;
    }
    if(!rm(source))
    {
        printf("remove file error\n");
        return -1;
    }

    return 0;
}


int dm_rename(char *source, char *dest)
{
	int ret = 0;
	if(source == NULL || dest == NULL)
		return -1;
	if (rename(source, dest) < 0) {
		printf("dm_rename error:%d\n",errno);
		return -1;
		/*ret = mv(source,dest);	
		if(ret < 0)
			return -1;*/
	}
	return 0;
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





	
