#ifndef __FREE_ARGS_H
#define __FREE_ARGS_H

#include "gp_buffer.h"
	

int handle_relarg_caminfo(void* _args);
int handle_relarg_listfile(void* _args);
int handle_relarg_getexifinfo(void* pargs);
int handle_relarg_getfile(void* pargs);
int handle_relarg_getthumb(void* pargs);
int handle_relarg_getfileinfo(void* pargs);
int handle_relarg_readfile(void* pargs);
int handle_relarg_delete(void* pargs);
int handle_relarg_ping(void* pargs);

typedef int (*handle_release_args_func)(void* _args);
extern handle_release_args_func  handle_release_args[];


#endif
