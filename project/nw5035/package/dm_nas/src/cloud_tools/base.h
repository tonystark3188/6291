/*
 * =============================================================================
 *
 *       Filename:  base.h
 *
 *    Description:  base define and include
 *
 *        Version:  1.0
 *        Created:  2016/10/20
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 515296288jf@163.com
 *   Organization: longsys 
 *
 * =============================================================================
 */

#ifndef _BASE_NC_H_
#define _BASE_NC_H_

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
#include <sys/socket.h>
#include <errno.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <dirent.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "my_debug.h"
#include "list.h"
#include "my_json.h"
#include "cloud_thread.h"
#include "cloud_errno.h"
#include "msg.h"


#ifdef __cplusplus
extern "C"{
#endif

#ifndef _Out_
#define _Out_
#endif

#ifndef _In_
#define _In_
#endif

#ifndef _Inout_
#define _Inout_
#endif

/* Some macros in common use. */
#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif
typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
// typedef char int8_t;
typedef unsigned char uint8_t;


#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) >= (b)) ? (a) : (b))
#endif

#ifndef BIT
#define BIT(val, i) ((val) & (0x1 << (i)))
#endif


#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#define FIELD_SIZEOF(t, f) (sizeof(((t *)0)->f))
#if (defined DEBUG) && (defined PLATFORM_X86)
#define ASSERT(n) assert(n)
#else
#define ASSERT(n) 
#endif
#define EXIT(n) do{\
    ASSERT(0);\
    exit((n));\
    }while(0)


#define ENTER_FUNC() do{\
    DMCLOG_D("Enter %s()", __FUNCTION__);\
    }while(0)

#define EXIT_FUNC() do{\
    DMCLOG_D("Exit %s()", __FUNCTION__);\
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

#define S_STRNCPY(dest, src, n) do{\
		strncpy(dest, src, n);\
		(*(((char *)(dest)) + (n) - 1)) = 0;\
		}while(0)
	
#define S_SNPRINTF(str, size, fmt, args...) do{\
		snprintf(str, size, fmt, ##args);\
		(*(((char *)(str)) + (size) - 1)) = 0;\
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

#define LOCAL_ADDR				"127.0.0.1"
//Comm Port
#define LISTEN_PORT            	27212
#define SEND_PORT              	27213
#define TCP_INIT_PORT 			13111

//Time out
#define UDP_RECV_TIMEOUT       	1000//ms
#define DEV_DEL_TIMEOUT 		6//6s no receive udp package,delete the device

//Cmd num
#define FN_GET_ROUTE_IP    		1024
#define FM_GET_ROUTE_DETAIL 	1025
#define FM_GET_ROUTE_BEAT   	1027
#define FN_NOTIFY_DEL_IP 		5

#define MAX_USER_DEV_NAME_LEN 	512
#define MAX_BUCKET_NAME_LEN	32



typedef int (*CallBackFunc) (void *self, void *arg);
typedef void *(*ThreadFunc) (void *self);



#ifdef __cplusplus
}
#endif


#endif

