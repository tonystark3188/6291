#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


#include "camera.h"
#include "handle_request.h"
#include "debuglog.h"
#include "free_args.h"
#include "gp_buffer.h"
#include "ap_error.h"
#include "utils.h"

handle_release_args_func  handle_release_args[RCMD_COUNT] = {
	[RCMD_CAMINFO] = handle_relarg_caminfo,
	[RCMD_LISTFILE] = handle_relarg_listfile,
	[RCMD_LISTFOLDER] = handle_relarg_listfile,
	[RCMD_GETEXIFINFO] = handle_relarg_getexifinfo,
//	[RCMD_GETFILE] = handle_relarg_getfile,
	[RCMD_GETTHUMB] = handle_relarg_getthumb,
	[RCMD_GETFILEINFO] = handle_relarg_getfileinfo,
	[RCMD_READFILE] = handle_relarg_readfile,
	[RCMD_DELETE] = handle_relarg_delete,
	[RCMD_PING] = handle_relarg_ping,
};

int handle_relarg_caminfo(void *args)
{
	return 0;
}

int handle_relarg_listfile(void *_args)
{
	//DEBG (1, "[log][releasing argument structure for listfile request...]\n");
	struct listfile_args* args =(struct listfile_args*)_args;
	my_free((void**)&args->folder);
	return 0;
}

int handle_relarg_getexifinfo(void* _pargs)
{
	//DEBG (1, "[log][releasing argument structure for get_exif_info request...]\n");
	struct common_args* args =(struct common_args*)_pargs;
	my_free((void**)&args->p_path);
	return 0;
}
int handle_relarg_getfile(void* pargs);
int handle_relarg_getthumb(void* _pargs)
{
	//DEBG (1, "[log][releasing argument structure for thumb request...]\n");
	struct common_args* args =(struct common_args*)_pargs;
	my_free((void**)&args->p_path);
	return 0;
}
int handle_relarg_getfileinfo(void* _pargs)
{
	//DEBG (1, "[log][releasing argument structure for get_file_info request...]\n");
	struct common_args* pargs =(struct common_args*)_pargs;
	my_free((void**)&pargs->p_path);
	return 0;
}
int handle_relarg_readfile(void* _pargs)
{
	//DEBG (1, "[log][releasing argument structure for read_file request...]\n");
	struct readfile_args* pargs =(struct readfile_args*)_pargs;
	my_free((void**)&pargs->p_file_path);
	return 0;
}
int handle_relarg_delete(void* pargs)
{
	return 0;
}
int handle_relarg_ping(void* pargs)
{
	return 0;
}




