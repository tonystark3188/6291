/*
 * =============================================================================
 *
 *       Filename:  session.c
 *
 *    Description:  time basic operation
 *
 *        Version:  1.0
 *        Created:  2015/8/12 17:21
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver
 *   Organization:  
 *
 * =============================================================================
 */
#include "session.h"
#include "usr_manage.h"
#include "get_file_list.h"
extern struct usr_dnode *usr_dn;

int dm_usr_logout(struct conn *c)
{
	ENTER_FUNC();
	int res = 0;
	#if 0
	EnterCriticalSection(&c->ctx->mutex);
	display_usr_dnode(usr_dn);
	res = del_usr_from_list_for_session(&usr_dn,c->session);
	LeaveCriticalSection(&c->ctx->mutex);
	if(res < 0)
	{
		return -1;
	}
	#else
	DMCLOG_D("c->session = %s",c->session);
	res = handle_db_logout(c->session);
	if(res < 0)
	{
		return -1;
	}
	#endif
	
	EXIT_FUNC();
	return 0;
}
int dm_usr_login(struct conn *c)
{
	int res = 0;
	_dm_gen_session(c->session, c->username, c->password, c->cur_time);
	if(c->session == NULL||!*c->session)
	{
		DMCLOG_D("get session error");
		return -1;
	}
	DMCLOG_D("access register client ip");
	#if 0
	EnterCriticalSection(&c->ctx->mutex);
	res = add_usr_to_list(&usr_dn,c->session,c->client_ip,c->username,c->password,c->cur_time);
	display_usr_dnode(usr_dn);
	LeaveCriticalSection(&c->ctx->mutex);	
	if(res < 0)
	{
		DMCLOG_D("add_usr_to_list error");
		return -1;
	}
	display_usr_dnode(usr_dn);
	#else
	DMCLOG_D("c->ip = %s",c->client_ip);
	EnterCriticalSection(&c->ctx->mutex);
	res = handle_db_del_usr_for_ip(c->client_ip);
	LeaveCriticalSection(&c->ctx->mutex);
	if(res < 0)
	{
		DMCLOG_D("handle_db_del_usr_for_ip error");
		return -1;
	}
	DMCLOG_D("c->session = %s",c->session);
	EnterCriticalSection(&c->ctx->mutex);
	res = handle_db_login(c->session,c->deviceUuid,c->deviceName,c->client_ip,c->username,c->password);
	LeaveCriticalSection(&c->ctx->mutex);
	if(res < 0)
	{
		return -1;;
	}
	#endif
	return 0;
}

