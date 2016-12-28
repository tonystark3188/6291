/*
 * =============================================================================
 *
 *       Filename:  time_opr.h
 *
 *    Description:  time basic operation
 *
 *        Version:  1.0
 *        Created:  2015/3/19 11:32
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _TIME_BASE_OPERAITON_H_
#define _TIME_BASE_OPERAITON_H_

#include <time.h>

#ifdef __cplusplus
extern "C"{
#endif

#define MAX_TIME_STR_LEN 16

int get_time_tm(struct tm *tv);
int get_time_str(char *time_s, size_t size);
int get_time_str_for_db(char *time_s, size_t size);
int get_gmt_time_tm(struct tm *tv);


#ifdef __cplusplus
}
#endif


#endif

