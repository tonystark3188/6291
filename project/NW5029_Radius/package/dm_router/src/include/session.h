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

#ifndef _SESSION_H_
#define _SESSION_H_

#include "defs.h"


#ifdef __cplusplus
extern "C"{
#endif

int dm_session_init(struct shttpd_ctx *ctx);

int dm_session_destroy(struct shttpd_ctx *ctx);



#ifdef __cplusplus
}
#endif

#endif

