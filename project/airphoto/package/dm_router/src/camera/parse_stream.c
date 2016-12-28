#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


#include "camera.h"
#include "handle_request.h"
#include "debuglog.h"
#include "parse_stream.h"
#include "gp_buffer.h"
#include "ap_error.h"

handle_request_buffer_func  handle_request_buffer[RCMD_COUNT] = {
	[RCMD_CAMINFO] = handle_request_buffer_caminfo,
	[RCMD_LISTFILE] = handle_request_buffer_listfile,
	[RCMD_LISTFOLDER] = handle_request_buffer_listfile,
	[RCMD_GETEXIFINFO] = handle_request_buffer_getexifinfo,
//	[RCMD_GETFILE] = handle_request_buffer_getfile,
	[RCMD_GETTHUMB] = handle_request_buffer_getthumb,
	[RCMD_GETFILEINFO] = handle_request_buffer_getfileinfo,
	[RCMD_READFILE] = handle_request_buffer_readfile,
	[RCMD_DELETE] = handle_request_buffer_delete,
	[RCMD_PING] = handle_request_buffer_ping,
};


int handle_request_buffer_caminfo(struct my_buffer* buffer, void *_args)
{
	DEBG (1, "[log][handling request stream for caminfo request...]\n");
	return 0;
}

int handle_request_buffer_listfile(struct my_buffer* buffer, void *_args)
{
	DEBG (1, "[log][handle request stream for listfile request...]\n");
	//after reading recv-cmd field, current buffer offset should 2.
	struct listfile_args* args = _args;
	int ret = 0;
	int path_len = 0;
	CK00 (ret = get_buffer_16(buffer ,(unsigned short*)&path_len), err_no);
	DEBG (1, "[log][folder pathlen=%d]\n", path_len);
	CK20 (args->folder = (char*)malloc(path_len+10), err_cmal_no);
	memset(args->folder, 0, path_len+10);
	CK00 (ret = get_buffer_stream(buffer ,(unsigned char*)args->folder, path_len), err_no);
	DEBG (1, "[log][folder path=%s]\n", args->folder);
	CK00 (ret = get_buffer_8(buffer ,(unsigned char*)&args->recursive), err_no);
	DEBG (1, "[log][recursive:%s]\n", args->recursive == 0?"no":"yes");
	return 0;
	
err_cmal_no:
	DEBG (0, "[error][malloc error][%s]\n", strerror(errno) );
	ret = GP_ERROR_NO_MEMORY;
err_no:
	return ret;
}

int handle_request_buffer_getexifinfo(struct my_buffer *buffer, void* pargs)
{
	DEBG (1, "[log][handle request stream for request:listfile ...]\n");
	//after reading recv-cmd field, current buffer offset should 2.
	struct common_args* args = pargs;
	int ret = 0;
	CK00 (ret = get_buffer_stream_2(buffer, (unsigned char**)&args->p_path, NULL), err_no);
	DEBG (1, "[log][file path=%s]\n", args->p_path);
	return 0;
	
err_no:
	return ret;
	
}
int handle_request_buffer_getthumb(struct my_buffer *buffer, void* pargs)
{
	DEBG (1, "[log][handle request stream for request:get_thumb ...]\n");
	//after reading recv-cmd field, current buffer offset should 2.
	struct common_args* args = pargs;
	int ret = 0;
	CK00 (ret = get_buffer_stream_2(buffer, (unsigned char**)&args->p_path, NULL), err_no);
	DEBG (1, "[log][file path=%s]\n", args->p_path);
	return 0;
	
err_no:
	return ret;
}
int handle_request_buffer_getfileinfo(struct my_buffer *buffer, void* _pargs)
{
	DEBG (1, "[log][handle request stream for request:get_file_info ...]\n");
	//after reading recv-cmd field, current buffer offset should 2.
	struct common_args* p_args = _pargs;
	int ret = 0;
	CK00 (ret = get_buffer_stream_2(buffer, (unsigned char**)&p_args->p_path, NULL), err_no);
	DEBG (1, "[log][file path=%s]\n", p_args->p_path);
	return 0;
	
err_no:
	return ret;
	return 0;
}
int handle_request_buffer_readfile(struct my_buffer *buffer, void* _pargs)
{
	DEBG (1, "[log][handle request stream for request:readfile ...]\n");
	//after reading recv-cmd field, current buffer offset should 2.
	struct readfile_args* p_args = _pargs;
	int ret = 0;
	CK00 (ret = get_buffer_stream_2(buffer, (unsigned char**)&p_args->p_file_path, NULL), err_no);
	DEBG (1, "[log][file path=%s]\n", p_args->p_file_path);
	CK00 (ret = get_buffer_64(buffer, &p_args->offset), err_no);
	DEBG (1, "[log][file offset=llu]\n", p_args->offset);
	CK00 (ret = get_buffer_64(buffer, &p_args->size), err_no);
	DEBG (1, "[log][file size=%llu]\n", p_args->size);
	return 0;
	
err_no:
	return ret;
}
int handle_request_buffer_delete(struct my_buffer *buffer, void* _args)
{
	return 0;
}

int handle_request_buffer_ping(struct my_buffer *buffer, void* _args)
{
	return 0;
}




