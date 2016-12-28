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
#include "get_file_list.h"
#include "file_opr.h"
#include "disk_manage.h"


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
			if(errno == EINTR||errno == EAGAIN )
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

/*
 * Desc: make dir wrapper. It will check the dirs if exist before create it!
 * 
 * Notice: This function does not check the father dir if exist!
 * dir_path: input,
 * mode: input
 * Return: 0 : means success. So after calling this function, the dir_path will be there.
 *       < 0 : means error.
 */
/*int make_dir_r(const char *dir_path, mode_t mode)
{
	int ret = is_dir_exist(dir_path);
	
	if(ret > 0)
	{
		return 0;
	}
	else if(ret == 0)
	{
		return mkdir(dir_path, mode);
	}
	else
	{
		return EDIR;	
	}
}*/

/*
 * Desc: make dirs. It will create it's father dirs if they do not exist!
 *
 * dir_path: input, 
 * mode: input,
 * Return: success on 0, else negitave value.
 */
/*int make_dirs(const char *dir_path, mode_t mode)
{
	char tmp_path[PATH_MAX] = {0};
	strcpy(tmp_path,dir_path);
    char *tmp = tmp_path;
    tmp++;
    while(*tmp != '\0')
	{
		//DMCLOG_D("tmp_path = %s",tmp);
		if(*tmp == '/')
		{
			*tmp = '\0';
			DMCLOG_D("tmp_path = %s",tmp_path);
			if(make_dir_r((const char *)tmp_path, mode) < 0)
			{
				DMCLOG_D("tmp_path = %s,errno = %d",tmp_path,errno);
				return EMKDIR;	
			}
			*tmp = '/';
		}
		tmp++;
	}
    return make_dir_r(dir_path, mode);
}*/

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

ssize_t cp_file(const char *src_file, const char *dest_file)
{
    char buf[DEFAULT_PAGE_SIZE];
    ssize_t ret;
    ssize_t read_bytes;
    ssize_t write_bytes;
    
    int src_fd = open(src_file, O_RDONLY);
    if(src_fd < 0)
    {
        log_warning("open src(%s) failed", src_file);
        return EOPEN;
    }

    int dest_fd = open(dest_file, O_RDWR | O_TRUNC | O_CREAT, FILE_DEF_MODE);
    if(dest_fd < 0)
    {
        log_warning("open dest(%s) failed", dest_file);
        close(src_fd);
        return EOPEN;
    }

    while(1)
    {
        read_bytes = readn(src_fd, buf, sizeof(buf));
        if(read_bytes < 0)
        {
            ret = EREAD;
            log_warning("readn failed");
            break;
        }
        else if(read_bytes == 0)
        {
            ret = RET_SUCCESS;
            log_debug("EOF!");
            break;
        }

        write_bytes = writen(dest_fd, buf, read_bytes);
        if(write_bytes != read_bytes)
        {
            log_warning("write_byets(%zd) != read_bytes(%zd)", write_bytes, read_bytes);
            ret = EWRITE;
            break;
        }
    }

    safe_close(src_fd);
    safe_close(dest_fd);
    
    return ret;
}


int get_file_size(const char *path, uint64_t *size)
{
	struct stat stat_buf;
	
	*size = 0;
    if(path == NULL)
    {
		return ENULL_POINT;
	}

	if(stat(path, &stat_buf) < 0)
	{
		log_warning("stat error");
		return EFILE_STAT;
	}

	if(S_ISDIR(stat_buf.st_mode))
	{
		*size = 0;
	}
	else
	{
		*size = stat_buf.st_size;
	}

	return RET_SUCCESS;
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
const char* bb_basename(const char *name)
{
	const char *cp = strrchr(name, '/');
	if (cp)
		return cp + 1;
	return name;
}
int dm_getDirInfo(char *path,struct file_list *file_list_t)
{
	struct dirent *entry;
	DIR *dir;
	unsigned nfiles = 0;
	dir = warn_opendir(path);
	if (dir == NULL) {
	 return -1;    /* could not open the dir */
	}
	while ((entry = readdir(dir)) != NULL) {
	 /* are we going to list the file- it may be . or .. or a hidden file */
	 if (entry->d_name[0] == '.') {
	  if ((!entry->d_name[1] || (entry->d_name[1] == '.' && !entry->d_name[2]))) {
	   continue;
	  }
	 }
	 //DMCLOG_D("entry->d_name = %s",entry->d_name);
	 nfiles++;
	}
	closedir(dir);
	DMCLOG_D("end:nfiles = %d",nfiles);
	file_list_t->totalCount = nfiles;
	file_list_t->pageSize = PAGESIZE;
	if(file_list_t->totalCount%PAGESIZE == 0)
		file_list_t->totalPage = file_list_t->totalCount/PAGESIZE;
	else
		file_list_t->totalPage = file_list_t->totalCount/PAGESIZE + 1;

	DMCLOG_D("file_list_t->totalCount = %d,file_list_t->pageSize = %d",file_list_t->totalCount,file_list_t->pageSize);
	DMCLOG_D("file_list_t->totalPage = %d",file_list_t->totalPage);
	return 0;
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

struct file_dnode *dm_get_file_attr(const char *fullname)
{
	struct stat statbuf;
	struct file_dnode *cur;
	cur = xzalloc(sizeof(*cur));
	if (lstat(fullname, &statbuf)) {
		printf("%s",fullname);
		safe_free(cur);
		return NULL;
	}
	//cur->fullname = fullname;
	cur->size = statbuf.st_size;
	cur->date = (unsigned long)statbuf.st_mtime;
	cur->isFolder = S_ISDIR(statbuf.st_mode);
	return cur;
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

unsigned count_dirs(struct file_dnode **dn, int which)
{
	unsigned dirs, all;

	if (!dn)
		return 0;

	dirs = all = 0;
	for (; *dn; dn++) {
		const char *name;

		all++;
		if (!S_ISDIR((*dn)->dn_mode))
			continue;

		name = (*dn)->name;
		if (which != SPLIT_SUBDIR /* if not requested to skip . / .. */
		 /* or if it's not . or .. */
		 || name[0] != '.'
		 || (name[1] && (name[1] != '.' || name[2]))
		) {
			dirs++;
		}
	}
	return which != SPLIT_FILE ? dirs : all - dirs;
}

struct file_dnode **splitdnarray(struct file_dnode **dn, int which)
{
	unsigned dncnt, d;
	struct file_dnode **dnp;

	if (dn == NULL)
		return NULL;

	/* count how many dirs or files there are */
	dncnt = count_dirs(dn, which);

	/* allocate a file array and a dir array */
	dnp = dnalloc(dncnt);

	/* copy the entrys into the file or dir array */
	for (d = 0; *dn; dn++) {
		if (S_ISDIR((*dn)->dn_mode)) {
			const char *name;

			if (which == SPLIT_FILE)
				continue;

			name = (*dn)->name;
			if ((which & SPLIT_DIR) /* any dir... */
			/* ... or not . or .. */
			 || name[0] != '.'
			 || (name[1] && (name[1] != '.' || name[2]))
			) {
				dnp[d++] = *dn;
			}
		} else
		if (which == SPLIT_FILE) {
			dnp[d++] = *dn;
		}
	}
	return dnp;
}

//在指定磁盘创建标记文件(隐藏在磁盘根目录)
int create_mark_file(char *path,char *uuid)
{
	int res = 0;
	char uuid_path[256];
	memset(uuid_path,0,256);
	sprintf(uuid_path,"%s/%s",path,get_sys_disk_uuid_name());
	FILE *fd;
	if(path == NULL||uuid == NULL)
	{
		return -1;
	}
	if((fd = fopen(uuid_path,"w+"))== NULL)//改动：路径
	{
		DMCLOG_D("open file error[errno = %d]",errno);
		return -1;
	}
	DMCLOG_D("create_mark_file uuid = %s",uuid);
	res = fwrite(uuid,sizeof(char),strlen(uuid),fd);
	if(res <= 0)
	{
		DMCLOG_D("enRet = %d,errno = %d",res,errno);
		fclose(fd);
		return -1;
	}
	fclose(fd);
	return 0;
}

int write_mark_file(char *path,char *uuid)
{
	if(path == NULL || uuid == NULL)
		return -1;
	int enRet = 0,i;
	all_disk_t mAll_disk_t;
	memset(&mAll_disk_t,0,sizeof(all_disk_t));
	enRet = dm_get_storage(&mAll_disk_t);
	if(enRet == 0)
	{
		for(i = 0;i < mAll_disk_t.count;i++)
		{
			if(strcmp(mAll_disk_t.disk[i].path,path) == 0)
			{
				update_disk_uuid(mAll_disk_t.disk[i].path,uuid);
				if(!*uuid)
				{
					strcpy(uuid,"12345678");
				}
				enRet = create_mark_file(mAll_disk_t.disk[i].path,uuid);
				break;
			}
		}
	}
	return enRet;
}
int read_mark_file(char *path,char *uuid)
{
	int res = 0;
	char uuid_path[256];
	FILE *fd;
	if(path == NULL||uuid == NULL)
	{
		return -1;
	}
	memset(uuid_path,0,256);
	sprintf(uuid_path,"%s/%s",path,get_sys_disk_uuid_name());
	DMCLOG_D("uuid_path = %s",uuid_path);
	if((fd = fopen(uuid_path,"r"))== NULL)//改动：路径
	{
		DMCLOG_D("open file error[errno = %d]",errno);
		goto EXIT;
	}
	res = fread(uuid,sizeof(char),16,fd);
	if(res <= 0)
	{
		DMCLOG_D("enRet = %d,errno = %d",res,errno);
		fclose(fd);
		goto EXIT;
	}
	fclose(fd);
	if(!*uuid||(*uuid&&strlen(uuid) < 8))
	{
		
		DMCLOG_E("uuid is invalide length : %d,uuid = %s",strlen(uuid),uuid);
		goto EXIT;
	}
	DMCLOG_D("uuid = %s",uuid);
	return 0;
EXIT:
	return write_mark_file(path,uuid);
}
int get_disk_name(char *src_uri,char *disk_name)
{
    char *tmp = NULL;
    if(src_uri == NULL || disk_name == NULL)
    {
        DMCLOG_E("para error");
        return -1;
    }
    if(*src_uri == '/')
    {
        tmp = strchr(src_uri + 1,'/');
        memcpy(disk_name,src_uri + 1,tmp - src_uri);
    }else{
        tmp = strchr(src_uri,'/');
		if(tmp == NULL)
		{
			 strcpy(disk_name,src_uri);
		}else
		{
			memcpy(disk_name,src_uri,tmp - src_uri);
		}
    }
    DMCLOG_D("disk_name = %s",disk_name);
    return 0;
}

int check_dev_path(char *dir_path)
{	
	char dir_path_tmp[32];
	char *p = NULL;
	p = dir_path;
	p++;
	DMCLOG_D("p = %s", p);
	p = strchr(p, '/');
	if(p != NULL){
		DMCLOG_D("p = %s", p);
		p++;
		p = strchr(p, '/');
		if(p != NULL){
			DMCLOG_D("p = %s", p);
			p++;
			p = strchr(p, '/');
			if(p != NULL){
				memset(dir_path_tmp, 0, sizeof(dir_path_tmp));
				memcpy(dir_path_tmp, dir_path, p-dir_path);
				DMCLOG_D("dir_path_tmp = %s", dir_path_tmp);
			}
			else{
				return EMKDIR;
			}
		}
		else{
			return EMKDIR;
		}
	}
	else{
		return EMKDIR;
	}
				
	if(F_OK == access(dir_path_tmp, 0)){
		return 0;
	}
	else{
		return EMKDIR;
	}
}


int check_device_name_file_exist(_In_ char *disk_path,_In_ char *s_device_name, _Out_ char *d_device_name)
{	
	int num_max = 99;
	int i;
	int ret = 0;
	if(disk_path == NULL || s_device_name == NULL)
		return -1;
	struct stat file_stat;
	char device_name_tmp[MAX_USER_DEV_NAME_LEN];
	memset(device_name_tmp, 0, MAX_USER_DEV_NAME_LEN);
	char *p_device_name_path = NULL;
	for(i = 0; i <= num_max; i++){
		if(i == 0){
			strcpy(device_name_tmp, s_device_name);
		}
		else{
			sprintf(device_name_tmp, "%s(%d)", s_device_name, i);
		}
		p_device_name_path = (char *)calloc(1, strlen(disk_path) + strlen(device_name_tmp) + 10);
		if(p_device_name_path == NULL){
			goto FAIL_EXIT;
		}
		sprintf(p_device_name_path, "%s/%s", disk_path, device_name_tmp);
		
		if(stat(p_device_name_path, &file_stat) < 0){
	        if(errno == ENOENT){
				DMCLOG_D("no exist");
				strcpy(d_device_name, device_name_tmp);
				if(NULL != p_device_name_path)
					free(p_device_name_path);
				return 0;
	        }
	    }
		else{
	    	if(S_ISDIR(file_stat.st_mode)){
				DMCLOG_D("is dir");
				strcpy(d_device_name, device_name_tmp);
				if(NULL != p_device_name_path)
					free(p_device_name_path);
				return 0;
			}
		}
		//DMCLOG_D("is file exist");
		if(NULL != p_device_name_path)
			free(p_device_name_path);
	}

FAIL_EXIT:
	strcpy(d_device_name, s_device_name);
	return -1;
}


