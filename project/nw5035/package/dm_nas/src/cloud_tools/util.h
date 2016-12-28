#ifndef _UTIL_H_
#define _UTIL_H_

#ifdef __cplusplus
extern "C"{
#endif
#include "file_opr.h"
#include "str_opr.h"

ssize_t  safe_read(int fd, void *buf, size_t count);

ssize_t full_write(int fd, const void *buf, size_t len);

int rm(const char *path);

int is_file_exist(char *src_path);
char* concat_path_file(const char *path, const char *filename);

int touch_file(char *src_path);
    
char* bb_get_last_path_component_strip(char *path);

char* get_last_char(const char *s, int c);
       
void* xzalloc(size_t size);
    
off_t bb_copyfd_eof(int fd1, int fd2);

#ifdef __cplusplus
}
#endif


#endif
