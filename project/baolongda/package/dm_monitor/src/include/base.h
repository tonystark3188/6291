/*
 * =============================================================================
 *
 *       Filename:  base.h
 *
 *    Description:  base define and include
 *
 *        Version:  1.0
 *        Created:  2015/3/19 11:16
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 515296288jf@163.com
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _BASE_H_
#define _BASE_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <resolv.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <dirent.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C"{
#endif

#include "msg.h"
#include "my_json.h"
#include "my_debug.h"
//#include "my_json.h"
#include "dmclog.h"
//#include "hidisk_thread.h"
#define BROADCAST_TIME 3
#define RECV_TIMEOUT 3000
#define UDP_CLIENT_FLAG 1
#define READ_DEV_FLAG 1
#define HEART_BEAT_PORT 27213
    /*############################## Macros ######################################*/
#define UDP_DEV_PATH "/tmp/udp_dev_info"
typedef struct dev_info {
    char ip[32];
    char type[4];
    char dev_name[32];
    char pri_port[32];
    int detail_flag; //0:not exist,1:exist
}dev_info_t;

typedef struct dev_group{
    dev_info_t g_dev[16];
    int count;
}dev_group_t;

typedef struct _ClientInfo
{
    struct sockaddr_in clientAddr;
    UINT32 client_fd;
    SINT32 des_port;
    SINT32 time_out;
    char *rcv_buf;
    char *send_buf;
    char ip[32];
    char type[4];
    char dev_name[32];
    char pri_port[32];
    SINT8 detail_flag; //0:not exist,1:exist
    SINT8 status;
    SINT32 cmd;
    SINT8 heat_count;
}ClientInfo;

typedef struct _ClientThreadInfo
{
    void *client_arg; // for future extend!
    int client_fd;  // for tcp or cgi_forward udp
    // for send response
    JObj *r_json;
    JObj *s_json;
    unsigned cmd;
    unsigned seq;
    char session[64]; // for session
    int error;
    int acc_fd;
    char *retstr;
    unsigned time_out;
    //recv para
    char ver[32];
    const char *username;
    const char *password;
    int client_port;
    int tcp_server_port;
    char ip[32];
	dev_group_t p_dev_group;
    struct sockaddr_in clientAddr;
    char *rcv_buf;
    char *send_buf;
    SINT8 pos;
}ClientTheadInfo;


#define ENTER_FUNC() do{\
    p_debug("\nEnter %s()\n", __FUNCTION__);\
    }while(0)

#define EXIT_FUNC() do{\
    p_debug("Exit %s()\n", __FUNCTION__);\
    }while(0)



#define safe_free(p) do{\
	if((p) != NULL)\
	{\
		free((p));\
		(p) = NULL;\
	}\
	}while(0)

#define safe_fclose(f) do{\
	if((f) != NULL)\
	{\
		fclose((f));\
		(f) = NULL;\
	}\
	}while(0)

#define safe_close(fd) do{\
	if((fd) > 0)\
	{\
		close((fd));\
		(fd) = -1;\
	}\
	}while(0)

#define MEMSET(o) memset(&(o), 0, sizeof(o))
#define MEMSET_P(p,s) memset(p, 0, s)



#ifdef SUPPORT_MUTIL_PTHREAD
#define lock_init(lock) pthread_mutex_init((lock), NULL)
#define lock(lock) pthread_mutex_lock((lock))
#define unlock(lock) pthread_mutex_unlock((lock))
#define lock_destroy(lock) pthread_mutex_destroy((lock))

#define cond_init(cond) pthread_cond_init((cond), NULL)
#define cond_wait(cond, lock) pthread_cond_wait((cond), (lock))
#define cond_signal(cond) pthread_cond_signal((cond))
#define cond_destroy(cond) pthread_cond_destroy((cond))

#else
#define lock_init(lock) 
#define lock(lock) 
#define unlock(lock)
#define lock_destroy(lock)

#define cond_init(cond) 
#define cond_wait(cond, lock) 
#define cond_signal(cond) 
#define cond_destroy(cond) 

#endif

typedef int (*CallBackFunc) (void *self, void *arg);
typedef void *(*ThreadFunc) (void *self);

#ifdef __cplusplus
}
#endif


#endif

