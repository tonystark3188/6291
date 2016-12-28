#ifndef __HANDLE_REQUEST_H
#define __HANDLE_REQUEST_H

#include "gp_buffer.h"
	
//#include "cJSON.h"
#include "camera.h"
/*
struct session_data {
	struct my_buffer recv_buffer;
	struct my_buffer resp_buffer;
	int entity_count;
	enum EntityType et;
	char* folder;
	char recursive;
};*/

enum REQUESTCMD {
	RCMD_CAMINFO = 0,
	RCMD_LISTFILE = 1,
	RCMD_LISTFOLDER = 2,
	RCMD_GETEXIFINFO = 3,
	RCMD_GETFILE = 4,
	RCMD_GETTHUMB = 5,
	RCMD_GETFILEINFO = 6,
	RCMD_READFILE = 7,
	RCMD_DELETE = 8,
	RCMD_PING = 9,
	RCMD_CAMERACONN = 10,
	RCMD_COUNT,
};

struct listfile_args {
	char* folder;		//need free
	int entity_count;
	//enum EntityType et;
	char recursive;
	struct my_buffer* resp_buffer;//just to improve speed, need not free this argument
	//cJSON* root;
};
struct readfile_args {
	int sockfd;
	char* p_file_path;		//need free
	unsigned long long offset;
	unsigned long long size;
};


struct common_args {//for those command has a path argument
	char* p_path;
};
int a_request_session(int sockfd);




#endif
