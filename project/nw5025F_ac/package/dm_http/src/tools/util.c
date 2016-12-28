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

enum {	/* DO NOT CHANGE THESE VALUES!  cp.c, mv.c, install.c depend on them. */
    FILEUTILS_PRESERVE_STATUS = 1 << 0, /* -p */
    FILEUTILS_DEREFERENCE     = 1 << 1, /* !-d */
    FILEUTILS_RECUR           = 1 << 2, /* -R */
    FILEUTILS_FORCE           = 1 << 3, /* -f */
    FILEUTILS_INTERACTIVE     = 1 << 4, /* -i */
    FILEUTILS_MAKE_HARDLINK   = 1 << 5, /* -l */
    FILEUTILS_MAKE_SOFTLINK   = 1 << 6, /* -s */
    FILEUTILS_DEREF_SOFTLINK  = 1 << 7, /* -L */
    FILEUTILS_DEREFERENCE_L0  = 1 << 8, /* -H */
};


enum {
    TERMINAL_WIDTH  = 80,           /* use 79 if terminal has linefold bug */
    
    SPLIT_FILE      = 0,
    SPLIT_DIR       = 1,
    SPLIT_SUBDIR    = 2,
    
    /* Bits in G.all_fmt: */
    
    /* 51306 lrwxrwxrwx  1 root     root         2 May 11 01:43 /bin/view -> vi* */
    /* what file information will be listed */
    LIST_INO        = 1 << 0,
    LIST_BLOCKS     = 1 << 1,
    LIST_MODEBITS   = 1 << 2,
    LIST_NLINKS     = 1 << 3,
    LIST_ID_NAME    = 1 << 4,
    LIST_ID_NUMERIC = 1 << 5,
    LIST_CONTEXT    = 1 << 6,
    LIST_SIZE       = 1 << 7,
    LIST_DATE_TIME  = 1 << 8,
    LIST_FULLTIME   = 1 << 9,
    LIST_SYMLINK    = 1 << 10,
    LIST_FILETYPE   = 1 << 11, /* show / suffix for dirs */
    LIST_CLASSIFY   = 1 << 12, /* requires LIST_FILETYPE, also show *,|,@,= suffixes */
    LIST_MASK       = (LIST_CLASSIFY << 1) - 1,
    
    /* what files will be displayed */
    DISP_DIRNAME    = 1 << 13,      /* 2 or more items? label directories */
    DISP_HIDDEN     = 1 << 14,      /* show filenames starting with . */
    DISP_DOT        = 1 << 15,      /* show . and .. */
    DISP_NOLIST     = 1 << 16,      /* show directory as itself, not contents */
    DISP_RECURSIVE  = 1 << 17,      /* show directory and everything below it */
    DISP_ROWS       = 1 << 18,      /* print across rows */
    DISP_MASK       = ((DISP_ROWS << 1) - 1) & ~(DISP_DIRNAME - 1),
    
    /* what is the overall style of the listing */
    STYLE_COLUMNAR  = 1 << 19,      /* many records per line */
    STYLE_LONG      = 2 << 19,      /* one record per line, extended info */
    STYLE_SINGLE    = 3 << 19,      /* one record per line */
    
    
    /* how will the files be sorted (CONFIG_FEATURE_LS_SORTFILES) */
    SORT_REVERSE    = 1 << 23,
    
    SORT_NAME       = 0,            /* sort by file name */
    SORT_SIZE       = 1 << 24,      /* sort by file size */
    SORT_ATIME      = 2 << 24,      /* sort by last access time */
    SORT_CTIME      = 3 << 24,      /* sort by last change time */
    SORT_MTIME      = 4 << 24,      /* sort by last modification time */
    SORT_VERSION    = 5 << 24,      /* sort by version */
    SORT_EXT        = 6 << 24,      /* sort by file name extension */
    SORT_DIR        = 7 << 24,      /* sort by file or directory */
    
    LIST_LONG       = LIST_MODEBITS | LIST_NLINKS | LIST_ID_NAME | LIST_SIZE | \
    LIST_DATE_TIME | LIST_SYMLINK,
};

#define LONE_CHAR(s,c)     ((s)[0] == (c) && !(s)[1])
#define CONFIG_FEATURE_COPYBUF_KB 4

static ssize_t  safe_read(int fd, void *buf, size_t count)
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



static ssize_t full_write(int fd, const void *buf, size_t len)
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



static off_t bb_copyfd_eof(int fd1, int fd2)
{
	return bb_full_fd_action(fd1, fd2, 0);
}

const char* bb_basename(const char *name)
{
    const char *cp = strrchr(name, '/');
    if (cp)
        return cp + 1;
    return name;
}

const char* bb_basepath(const char *name)
{
    const char *cp = strrchr(name, '/');
    
    if (cp)
    {
        char *des_path = (char *)malloc(strlen(name) - strlen(cp) + 1);
        if(des_path == NULL)
        {
            printf("malloc NULL");
            return NULL;
        }
        memcpy(des_path,name,strlen(name) - strlen(cp));
        return des_path;
    }
    return name;
}
char* last_char_is(const char *s, int c)
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
    p = (char *)malloc(strlen(path)+strlen(filename)+2);
    sprintf(p,"%s%s%s", path, (lc==NULL ? "/" : ""), filename);

    return p;
}

static int copy_file(const char *source, const char *dest)
{
    struct stat source_stat;
    struct stat dest_stat;
    bool dest_exist;
    int retval=1;

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
            printf("cannot open dest file:%s\n", dest);
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
static char* bb_get_last_path_component_strip(char *path)
{
    char *slash = last_char_is(path, '/');

    if (slash)
        while (*slash == '/' && slash != path)
            *slash-- = '\0';

    return bb_get_last_path_component_nostrip(path);
}



static int cp_mv_stat(const char *fn, struct stat *fn_stat)
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
        return 0;
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
    	    return 0;
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
            if(rm(new_path) == 0)
            {
                printf("remove file %s failed\n", new_path);
                free(new_path);
                closedir(dp);
                return 0;
            }
            free(new_path);
        } 
        closedir(dp);

        if (rmdir(path) < 0)
        {
	    return 0;
        }

        return 1; 
    }
    //regular file
    if (unlink(path) < 0) 
    {
        printf("remove file %s failed \n", path);
        return 0;
    }
 
    return 1;
}

int mv(char *source, const char *dest)
{
    if(!cp(source, dest))
    {
        printf("cp error = %d\n",errno);
        return 0;
    }
    if(!rm(source))
    {
        printf("remove file error\n");
        return 0;
    }

    return 1;
}


int dm_rename(char *source, char *dest)
{
	if(source == NULL || dest == NULL)
		return -1;
	if (rename(source, dest) < 0) {
		mv(source,dest);
	}
	return 0;
}




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
        if(read_bytes == count)
        {
            break;
        }
        
    }
    
    if(ret == 0)
        return 0;
    else if(ret < 0)
        return -1;
    
   return read_bytes;
}

int is_dir_exist(const char *dir_path)
{
    struct stat file_stat;
    if(stat(dir_path,&file_stat) < 0)
    {
        if(errno == ENOENT)
            return 0;
        return -1;
    }
    if(S_ISDIR(file_stat.st_mode))
        return 1;
    return -1;
}
#if 0
int make_dir_r(const char *dir_path,mode_t mode)
{
    int ret = is_dir_exist(dir_path);
    if(ret > 0)
    {
        return 0;
    }else if(ret == 0)
    {
        printf("dir_path = %s,mode = %u\n",dir_path,mode);
        return mkdir(dir_path,mode);
    }else{
        return -1;
    }
}
#endif
int bb_make_directory(char *path, long mode, int flags)
{
    mode_t cur_mask;
    mode_t org_mask;
    const char *fail_msg;
    char *s;
    char c;
    struct stat st;
    
    /* Happens on bb_make_directory(dirname("no_slashes"),...) */
    if (LONE_CHAR(path, '.'))
        return 0;
    
    org_mask = cur_mask = (mode_t)-1L;
    s = path;
    while (1) {
        c = '\0';
        
        if (flags & FILEUTILS_RECUR) {  /* Get the parent */
            /* Bypass leading non-'/'s and then subsequent '/'s */
            while (*s) {
                if (*s == '/') {
                    do {
                        ++s;
                    } while (*s == '/');
                    c = *s; /* Save the current char */
                    *s = '\0'; /* and replace it with nul */
                    break;
                }
                ++s;
            }
        }
        
        if (c != '\0') {
            /* Intermediate dirs: must have wx for user */
            if (cur_mask == (mode_t)-1L) { /* wasn't done yet? */
                mode_t new_mask;
                org_mask = umask(0);
                cur_mask = 0;
                /* Clear u=wx in umask - this ensures
                 * they won't be cleared on mkdir */
                new_mask = (org_mask & ~(mode_t)0300);
                //bb_error_msg("org_mask:%o cur_mask:%o", org_mask, new_mask);
                if (new_mask != cur_mask) {
                    cur_mask = new_mask;
                    umask(new_mask);
                }
            }
        } else {
            /* Last component: uses original umask */
            //bb_error_msg("1 org_mask:%o", org_mask);
            if (org_mask != cur_mask) {
                cur_mask = org_mask;
                umask(org_mask);
            }
        }
        
        if (mkdir(path, 0777) < 0) {
            /* If we failed for any other reason than the directory
             * already exists, output a diagnostic and return -1 */
            if (!(flags & FILEUTILS_RECUR)
                || ((stat(path, &st) < 0) || !S_ISDIR(st.st_mode))
                ) {
                fail_msg = "create";
                break;
            }
            /* Since the directory exists, don't attempt to change
             * permissions if it was the full target.  Note that
             * this is not an error condition. */
            if (!c) {
                goto ret0;
            }
        }
        
        if (!c) {
            /* Done.  If necessary, update perms on the newly
             * created directory.  Failure to update here _is_
             * an error. */
            if ((mode != -1) && (chmod(path, mode) < 0)) {
                fail_msg = "set permissions of";
                break;
            }
            goto ret0;
        }
        
        /* Remove any inserted nul from the path (recursive mode) */
        *s = c;
    } /* while (1) */
    
    printf("can't %s directory '%s',errno = %d\n", fail_msg, path,errno);
    flags = -1;
    goto ret;
ret0:
    flags = 0;
ret:
    //bb_error_msg("2 org_mask:%o", org_mask);
    if (org_mask != cur_mask)
        umask(org_mask);
    return flags;
}

int make_directory(char *path)
{
    int ret = 0;
    int flags;
    flags |= FILEUTILS_RECUR;
    if (bb_make_directory(path, 0777, flags)) {
        //status = EXIT_FAILURE;
        ret = -1;
    }
    return ret;
}
