/*
 * =============================================================================
 *
 *       Filename:  get_service_list.h
 *
 *    Description:  get hidisk running moudules info.
 *
 *        Version:  1.0
 *        Created:  2015/8/12 17:11
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 515296288jf@163.com
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _GET_SERVICE_LIST_H_
#define _GET_SERVICE_LIST_H_

#include "defs.h"
#ifdef __cplusplus
extern "C"{
#endif
#define SERVICE_CONT 3
typedef struct service_info {
	char name[32];
	uint32_t port;
}service_info_t;

typedef struct service_list{
	service_info_t service_info[8];
	uint8_t count;
}service_list_t;

int get_service_list(service_list_t *service_list);


#ifdef __cplusplus
}
#endif

#endif

