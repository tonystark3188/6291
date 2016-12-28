#ifndef _UTIL_H_
#define _UTIL_H_

#ifdef __cplusplus
extern "C"{
#endif
#include "file_opr.h"
#include "str_opr.h"
int cp(char *source_file, const char *dest_file);
int mv(const char *source, const char *dest);
int rm(const char *path);
int dm_rename(char *source, char *dest);
int is_file_exist(char *src_path);
char* concat_path_file(const char *path, const char *filename);

int touch_file(char *src_path);



void* xzalloc(size_t size);

#ifdef __cplusplus
}
#endif


#endif
