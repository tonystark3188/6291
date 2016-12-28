/*
 * =============================================================================
 *
 *       Filename:  time_opr.c
 *
 *    Description:  time basic operation
 *
 *        Version:  1.0
 *        Created:  2015/3/19 11:34
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver
 *   Organization:  
 *
 * =============================================================================
 */

#include "base.h"

int get_time_tm(struct tm *tv)
{
	time_t t;
    struct tm tmp_time;

	time(&t);
	localtime_r(&t, &tmp_time);
    
	memcpy(tv, &tmp_time, sizeof(struct tm));
    tv->tm_year += 1900;
    tv->tm_mon  += 1;
    
	return 0;
}

int get_time_str_for_db(char *time_s, size_t size)
{
    struct tm tv;

	memset(time_s, 0, size);
	get_time_tm(&tv);
	return snprintf(time_s, size, "%02d%02d-%02d%02d%2d",
            tv.tm_mon, tv.tm_mday, tv.tm_hour, tv.tm_min, tv.tm_sec);
}

int get_time_str(char *time_s, size_t size)
{
	struct tm tv;

	memset(time_s, 0, size);
	get_time_tm(&tv);
	snprintf(time_s, size, "%04d-%02d-%02d %02d:%02d:%02d", tv.tm_year, \
            tv.tm_mon, tv.tm_mday, tv.tm_hour, tv.tm_min, tv.tm_sec);
	return 0;	
}

int get_gmt_time_tm(struct tm *tv)
{
    time_t t;
    struct tm tmp_time;

    time(&t);
    gmtime_r(&t, &tmp_time);

    memcpy(tv, &tmp_time, sizeof(struct tm));
    tv->tm_year += 1900;
    tv->tm_mon  += 1;
    
	return 0;
}



