/*
 * =============================================================================
 *
 *       Filename:  mylog.h
 *
 *    Description:  log tool on linux
 *
 *        Version:  1.0
 *        Created:  2014/06/24 11:28:19
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Wenhao Ye (wenhao), 
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _MY_LOG_H_
#define _MY_LOG_H_

#include <unistd.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"{
#endif

typedef enum MyLogLevel{
	MLL_DEBUG = 1,
	MLL_TRACE = 2,
	MLL_NOTICE = 3,
	MLL_WARNING = 4,
	MLL_ERROR = 5,	
}MyLogLevel;


int log_init(const char *file, MyLogLevel mll, size_t size);
int log_exit(void);
int log_msg(MyLogLevel l, char *logfmt, ...); 
int log_sync(void);
#define log_error(log_fmt, log_arg...) do{\
	printf( "[%s:%d][%s()] " log_fmt "\n", \
			__FILE__, __LINE__, __FUNCTION__, ##log_arg);\
	}while(0)


#define log_warning(log_fmt, log_arg...) do{\
	printf("[%s():%d] " log_fmt "\n", \
			__FUNCTION__, __LINE__, ##log_arg);\
	}while(0)

#define log_notice(log_fmt, log_arg...) do{\
	printf("[%s():%d] " log_fmt "\n", \
			__FUNCTION__, __LINE__, ##log_arg);\
	}while(0)

#define log_trace(log_fmt, log_arg...) do{\
	printf("[%s():%d] " log_fmt "\n", \
			__FUNCTION__, __LINE__, ##log_arg);\
	}while(0)

#define log_debug(log_fmt, log_arg...) do{\
	printf( "[%s():%d] " log_fmt "\n", \
			__FUNCTION__, __LINE__, ##log_arg);\
	}while(0)



#ifdef __cplusplus
}
#endif



#endif

