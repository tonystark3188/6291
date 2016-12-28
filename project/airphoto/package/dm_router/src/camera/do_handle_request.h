#ifndef __DO_HANDLE_REQUEST_H
#define __DO_HANDLE_REQUEST_H

#include "gp_buffer.h"



int handle_request_caminfo(struct my_buffer *buffer, void* _args);
int handle_request_listfile(struct my_buffer *buffer, void* _args);
int handle_request_listfolder(struct my_buffer *buffer, void* _args);
int handle_request_getexifinfo(struct my_buffer *buffer, void* _args);
int handle_request_getfile(struct my_buffer *buffer, void* _args);
int handle_request_getthumb(struct my_buffer *buffer, void* _args);
int handle_request_getfileinfo(struct my_buffer *buffer, void* _args);
int handle_request_readfile(struct my_buffer *buffer, void* _args);
int handle_request_delete(struct my_buffer *buffer, void* _args);
int handle_request_ping(struct my_buffer *buffer, void* _args);


typedef int (*handle_reqest_func)(struct my_buffer *buffer, void* _args);
extern handle_reqest_func  handle_request[];


#endif
