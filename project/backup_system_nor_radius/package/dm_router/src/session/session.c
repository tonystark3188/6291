/*
 * =============================================================================
 *
 *       Filename:  session.c
 *
 *    Description:  session basic operation
 *
 *        Version:  1.0
 *        Created:  2016/8/23 
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver
 *   Organization:  
 *
 * =============================================================================
 */
#include "session.h"
#include "dm_encrypt.h"

static unsigned long acc_count = 0;

static int dm_session_process(struct conn *c)
{
	time_t timep;
	struct tm *p;
	int ret = 0;
	if(!*c->session)
	{
		//allocate session
		DMCLOG_D("session is null");
		time(&timep);
		p = localtime(&timep);
		timep = mktime(p);
		DMCLOG_D("time()->localtime()->mktime():%d",timep);
		acc_count++;
		c->cur_time = timep + acc_count;
		_dm_gen_session(c->session, c->username, c->password, c->cur_time);
	}
	add_session_to_list(c->session,&c->ctx->p_session_list);
	 
	if(is_session_login(c->session,&c->ctx->p_session_list) == TRUE)
	{
		if(c->cmd == 66||c->cmd == 67)
		{
			c->ctx->session_reset = reset_session_list;
		}
	}else{
		if(c->cmd != 2)
		{
			DMCLOG_E("the cmd : %d have no permit to process",c->cmd);
			return -1;
		}
		if(dm_match_password(c->password) != 1)
		{
			DMCLOG_E("password match error");
			return -1;
		}
		set_session_login(c->session,TRUE,&c->ctx->p_session_list);
	}
	
	return 0;
}

int dm_session_init(struct shttpd_ctx *ctx)
{
	session_list_init(&ctx->p_session_list);
	ctx->session_process = dm_session_process;
}

int dm_session_destroy(struct shttpd_ctx *ctx)
{
	session_list_destroy(&ctx->p_session_list);
}


