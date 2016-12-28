#ifndef _UTIL_H_
#define _UTIL_H_

#ifdef __cplusplus
extern "C"{
#endif
#include <stdio.h>
int cp(char *source_file, const char *dest_file);
int mv(const char *source, const char *dest);
int rm(const char *path);
int dm_rename(char *source, char *dest);
char* concat_path_file(const char *path, const char *filename);
const char* bb_basename(const char *name);
const char* bb_basepath(const char *name);
ssize_t readn(int fd, char *buf, size_t count);


void* xzalloc(size_t size);
//int make_dir_r(const char *dir_path,mode_t mode);
int make_directory(char *path);
char* last_char_is(const char *s, int c);

#ifdef __cplusplus
}
#endif


#endif
