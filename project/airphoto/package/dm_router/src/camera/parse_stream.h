#ifndef __PARSE_STREAM_H
#define __PARSE_STREAM_H

#include "gp_buffer.h"
	

int handle_request_buffer_caminfo(struct my_buffer *buffer, void* _args);
int handle_request_buffer_listfile(struct my_buffer *buffer, void* _args);
int handle_request_buffer_getexifinfo(struct my_buffer *buffer, void* _args);
int handle_request_buffer_getfile(struct my_buffer *buffer, void* _args);
int handle_request_buffer_getthumb(struct my_buffer *buffer, void* _args);
int handle_request_buffer_getfileinfo(struct my_buffer *buffer, void* _args);
int handle_request_buffer_readfile(struct my_buffer *buffer, void* _args);
int handle_request_buffer_delete(struct my_buffer *buffer, void* _args);
int handle_request_buffer_ping(struct my_buffer *buffer, void* _args);


typedef int (*handle_request_buffer_func)(struct my_buffer *buffer, void* _args);
extern handle_request_buffer_func  handle_request_buffer[];



#endif
