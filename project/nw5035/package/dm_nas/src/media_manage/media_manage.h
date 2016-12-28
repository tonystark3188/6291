/*
 * =============================================================================
 *
 *       Filename:  media_manage.h
 *
 *    Description: media infomation process for dm init module
 *
 *        Version:  1.0
 *        Created:  2016/11/22 10:13
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _MEDIA_MANAGE_H_
#define _MEDIA_MANAGE_H_

#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "base.h"
#include "pqueue.h"

typedef int (*ON_NEW_FILE_HANDLE)(void *arg);

#ifdef __cplusplus
extern "C"{
#endif
typedef struct
{
	char *token;
	char *path;
	int media_type;
	pqueue_pri_t pri;
	size_t pos;
	ON_NEW_FILE_HANDLE on_new_file_handle;
}media_dnode_t;

typedef struct media_list{
	pqueue_t *pqueue;
	pthread_mutex_t mutex;
}media_list_t;

static __inline void media_dnode_free(media_dnode_t *p)
{
	if(p)
	{
		safe_free(p->path);
	}
	safe_free(p);
}


int media_list_init();
int media_list_destroy();
int add_media_to_list(char *path,int media_type,ON_NEW_FILE_HANDLE *on_new_file_handle,unsigned int pri);


#ifdef __cplusplus
}
#endif

#endif

