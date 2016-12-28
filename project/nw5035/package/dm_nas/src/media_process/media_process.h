#ifndef _MEDIA_PROCESS_H_
#define _MEDIA_PROCESS_H_

#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "base.h"

#ifdef __cplusplus
extern "C"{
#endif


#define  MAX_MEDIA_PROCESS_THREAD_NUM   (5)

enum{
	TYPE_VIDEO = 1,
	TYPE_AUDIO = 2,
	TYPE_IMAGE = 3,
};


//api
int media_prc_thpool_init(void);
void media_prc_thpool_destroy(void);
int media_prc_thpool_add_work(int (*function_p)(void*), void* arg_p);
int media_prc_thpool_cb(void* arg_p);



#ifdef __cplusplus
}
#endif
#endif

