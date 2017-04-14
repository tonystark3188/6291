/*
 * =============================================================================
 *
 *       Filename:  pwd_manage.h
 *
 *    Description: password management
 *
 *        Version:  1.0
 *        Created:  2016/8/10 15:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _PASSWORD_MANAGE_H_
#define _PASSWORD_MANAGE_H_

#include "base.h"
#include <stdbool.h>


#ifdef __cplusplus
extern "C"{
#endif

/*
 * function:
 *	get the password from cfg
 * para: 
 *	pwd:root password (output)
 * return:
 *	0:succ
 *	-1:failed
*/
int dm_get_password(_Out_ char *pwd);

/*
 * function:
 *	set the password to cfg
 * para: 
 *	pwd:root password (input)
 * return:
 *	0:succ
 *	-1:failed
*/
int dm_set_password(_In_ char *pwd);

/*
 * function:
 *	reset the password to cfg
 * para: 
 *	old_pwd:old password (input)
 *	new_pwd:new password (input)
 * return:
 *	0:succ
 *	-1:failed
*/
int dm_reset_password(_In_ char *old_pwd,_In_ char *new_pwd);

/*
* function:
*  match the password
* para: 
*  old_pwd:old password (input)
* return:
*  0:not match
*  1:match
*  -1:
*/
int dm_match_password(_In_ char *pwd);


bool dm_password_exist();

#ifdef __cplusplus
}
#endif

#endif

