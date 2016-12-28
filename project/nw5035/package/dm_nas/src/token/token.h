/*
 * =============================================================================
 *
 *       Filename:  token.h
 *
 *    Description:  token manage.
 *
 *        Version:  1.0
 *        Created:  2016/10/25 17:11
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 515296288jf@163.com
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _TOKEN_H_
#define _TOKEN_H_

#include "defs.h"



#ifdef __cplusplus
extern "C"{
#endif

int dm_token_init(struct shttpd_ctx *ctx);

int dm_token_destroy(struct shttpd_ctx *ctx);



#ifdef __cplusplus
}
#endif

#endif

