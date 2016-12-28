/*
 * =============================================================================
 *
 *       Filename:  session.h
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
#include "dm_encrypt.h"

#ifdef __cplusplus
extern "C"{
#endif
int dm_usr_logout(struct conn *c);
int dm_usr_login(struct conn *c);



#ifdef __cplusplus
}
#endif

#endif

