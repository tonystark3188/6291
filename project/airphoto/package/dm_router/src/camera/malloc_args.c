#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


#include "camera.h"
#include "handle_request.h"
#include "debuglog.h"
#include "malloc_args.h"
#include "gp_buffer.h"
#include "ap_error.h"


handle_malloc_args_func  handle_malloc_args[RCMD_COUNT] = {
	[RCMD_CAMINFO] = handle_mallocargs_caminfo,
	[RCMD_LISTFILE] = handle_mallocargs_listfile,
	[RCMD_LISTFOLDER] = handle_mallocargs_listfile,		//the same with listfile
	[RCMD_GETEXIFINFO] = handle_mallocargs_getexif,		
//	[RCMD_GETFILE] = handle_mallocargs_getfile,
	[RCMD_GETTHUMB] = handle_mallocargs_getthumb,
	[RCMD_GETFILEINFO] = handle_mallocargs_getfileinfo,
	[RCMD_READFILE] = handle_mallocargs_readfile,
	[RCMD_DELETE] = handle_mallocargs_delete,
	[RCMD_PING] = handle_mallocargs_ping,
};

int handle_mallocargs_caminfo(void **_args)
{
	DEBG (1, "[log][mallocing argument structure for caminfo request...]\n");
	return 0;	
}

int handle_mallocargs_listfile(void **_args)
{
	DEBG (1, "[log][mallocing argument structure for listfile request...]\n");
	struct listfile_args* args = NULL;

	CK20 (args = (struct listfile_args*)malloc(sizeof(struct listfile_args)), err_cmalloc_no);
	memset(args, 0, sizeof(struct listfile_args));
	*_args = args;
	return 0;	
err_cmalloc_no:
	DEBG (0, "[error][malloc error][%s]\n", strerror(errno));
	return GP_ERROR_NO_MEMORY;
}

int handle_mallocargs_getexif(void **ppargs)
{
	DEBG (1, "[log][mallocing argument structure for request:getexif ...]\n");
	CK20 (*ppargs = (struct common_args*)malloc(sizeof(struct common_args)), err_cmalloc_no);
	memset(*ppargs, 0, sizeof(struct common_args));
	return 0;	
err_cmalloc_no:
	DEBG (0, "[error][malloc error][%s]\n", strerror(errno));
	return GP_ERROR_NO_MEMORY;
}
	
int handle_mallocargs_getthumb(void **ppargs)
{
	DEBG (1, "[log][mallocing argument structure for request:get_thumb ...]\n");
	CK20 (*ppargs = (struct common_args*)malloc(sizeof(struct common_args)), err_cmalloc_no);
	memset(*ppargs, 0, sizeof(struct common_args));
	return 0;	
err_cmalloc_no:
	DEBG (0, "[error_msg][malloc error][%s]\n", strerror(errno));
	return GP_ERROR_NO_MEMORY;
}
int handle_mallocargs_getfileinfo(void **pp_args)
{
	DEBG (1, "[log][mallocing argument structure for request:get_file_info ...]\n");
	CK20 (*pp_args = (struct common_args*)malloc(sizeof(struct common_args)), err_cmalloc_no);
	memset(*pp_args, 0, sizeof(struct common_args));
	return 0;	
err_cmalloc_no:
	DEBG (0, "[error_msg][malloc error][%s]\n", strerror(errno));
	return GP_ERROR_NO_MEMORY;
}
int handle_mallocargs_readfile(void **pp_args)
{
	DEBG (1, "[log][mallocing argument structure for request: read_info ...]\n");
	CK20 (*pp_args = (struct readfile_args*)malloc(sizeof(struct readfile_args)), err_cmalloc_no);
	memset(*pp_args, 0, sizeof(struct readfile_args));
	return 0;	
err_cmalloc_no:
	DEBG (0, "[error_msg][malloc error][%s]\n", strerror(errno));
	return GP_ERROR_NO_MEMORY;
}
int handle_mallocargs_delete(void **pp_args)
{
	return 0;
}

int handle_mallocargs_ping(void **pp_args)
{
	return 0;
}



