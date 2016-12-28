#ifndef __MALLOC_ARGS_H
#define __MALLOC_ARGS_H


#include "gp_buffer.h"


int handle_mallocargs_caminfo(void **pp_args);
int handle_mallocargs_listfile(void **pp_args);
int handle_mallocargs_getexif(void **pp_args);		
int handle_mallocargs_getfile(void **pp_args);
int handle_mallocargs_getthumb(void **pp_args);
int handle_mallocargs_getfileinfo(void **pp_args);
int handle_mallocargs_readfile(void **pp_args);
int handle_mallocargs_delete(void **pp_args);
int handle_mallocargs_ping(void **pp_args);


typedef int (*handle_malloc_args_func)(void** _args);
extern handle_malloc_args_func  handle_malloc_args[];

#endif
