#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>

#include "camera.h"
#include "handle_request.h"
#include "debuglog.h"
#include "do_handle_request.h"
#include "gp_buffer.h"
#include "ap_error.h"
#include "commucation.h"
#include "utils.h"
//#include "cJSON.h"
handle_reqest_func  handle_request[RCMD_COUNT] = {
	[RCMD_CAMINFO] = handle_request_caminfo,
	[RCMD_LISTFILE] = handle_request_listfile,	
	[RCMD_LISTFOLDER] = handle_request_listfolder,	
	[RCMD_GETEXIFINFO] = handle_request_getexifinfo,
	//	[RCMD_GETFILE] = handle_request_getfile,	
	[RCMD_GETTHUMB] = handle_request_getthumb,	
	[RCMD_GETFILEINFO] = handle_request_getfileinfo,	
	[RCMD_READFILE] = handle_request_readfile,	
	[RCMD_DELETE] = handle_request_delete,	
	[RCMD_PING] = handle_request_ping,
};



int handle_request_caminfo(struct my_buffer *buffer, void* _args)
{
	return 0;
}

int handle_request_listentries(struct my_buffer *buffer, void* pargs, enum EntityType et)
{
	
	return 0;
}

int handle_request_listfile(struct my_buffer *buffer, void* pargs)
{
	return handle_request_listentries(buffer, pargs, ET_FILE);
}
int handle_request_listfolder(struct my_buffer *buffer, void* pargs)
{
	return handle_request_listentries(buffer, pargs, ET_FOLDER);
}

int handle_request_getexifinfo(struct my_buffer *buffer, void* _args)
{
	return 0;
}
//int handle_request_getfile(struct my_buffer *buffer, void* _args);

int handle_request_getthumb(struct my_buffer *buffer, void* _args)
{
	return 0;
}

int handle_request_getfileinfo(struct my_buffer *buffer, void* _args)
{
	return 0;
}
int handle_request_readfile(struct my_buffer *buffer, void* _args)
{
	return 0;
}
int handle_request_delete(struct my_buffer *buffer, void* _args)
{
	return 0;
}


int handle_request_ping(struct my_buffer *buffer, void* _args)
{
	int ret = 0;
	CK00 (ret = put_buffer_16(buffer, RCMD_PING), err_no);//cmd
	CK00 (ret = put_buffer_16(buffer, 0), err_no);//errcode succeed
	int count = CamD_get_camera_count();
	CK00 (ret = put_buffer_16(buffer, count), err_no);//cam count
//	CK01 (ret = send_camera_detect());
	return 0;
err_no:
	return ret;
}



