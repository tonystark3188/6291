/*
 * =====================================================================================
 *
 *       Filename:  db_user_manage.h
 *
 *    Description:  manage user info
 *
 *        Version:  1.0
 *        Created:  2014/8/8 16:56:25
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ye Wenhao (), wenhaoye@126.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef _DB_USER_MANAGE_H_
#define _DB_USER_MANAGE_H_

#include "tools/base.h"


typedef struct 
{
	uint8_t user_index;
	uint8_t is_super;
	char user_name[16];
	char user_passwd[16];
}UserInfo;


#endif

