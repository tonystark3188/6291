/*
 * =============================================================================
 *
 *       Filename:  str_opr.h
 *
 *    Description:  string wrapper module.
 *
 *        Version:  1.0
 *        Created:  2014/12/5 11:56:51
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ye Wenhao (), wenhaoye@126.com
 *   Organization:  
 *
 * =============================================================================
 */
#ifndef _STRING_WRAPPER_H_
#define _STRING_WRAPPER_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <assert.h>

#ifdef DEBUG

#define S_STRNCPY(dest, src, n) do{\
    if((dest) == NULL || (src) == NULL || (n) == 0){\
        printf("%s():%d ERROR!\n", __FUNCTION__, __LINE__);\
        assert(0);\
    }\
    strncpy(dest, src, n);\
    (*(((char *)(dest)) + (n) - 1)) = 0;\
    }while(0)

#define S_SNPRINTF(str, size, fmt, args...) do{\
    if((str) == NULL || (size) == 0){\
        printf("%s():%d ERROR!\n", __FUNCTION__, __LINE__);\
        assert(0);\
    }\
    snprintf(str, size, fmt, ##args);\
    (*(((char *)(str)) + (size) - 1)) = 0;\
    }while(0)

#else

#define S_STRNCPY(dest, src, n) do{\
    strncpy(dest, src, n);\
    (*(((char *)(dest)) + (n) - 1)) = 0;\
    }while(0)

#define S_SNPRINTF(str, size, fmt, args...) do{\
    snprintf(str, size, fmt, ##args);\
    (*(((char *)(str)) + (size) - 1)) = 0;\
    }while(0)


#endif


#ifdef __cplusplus
}
#endif


#endif
