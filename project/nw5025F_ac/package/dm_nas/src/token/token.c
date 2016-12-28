/*
 * =============================================================================
 *
 *       Filename:  token.c
 *
 *    Description:  token basic operation
 *
 *        Version:  1.0
 *        Created:  2016/10/25 
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver
 *   Organization:  
 *
 * =============================================================================
 */
#include "token.h"
#include "file_json.h"

static int public_user_process(struct conn *c)
{
	int user_id = 0;
	int bucket_id = 0;
	int authority = 7;
	int autority_id = 0;
	int ret = handle_user_table_query(c->username,c->password,&user_id);
	if(ret != 0)
	{
		DMCLOG_E("public user is not exist");
		DMCLOG_D("c->username = %s,c->password = %s",c->username,c->password);
		ret = handle_user_table_insert(c->username,c->password,&user_id);
		if(ret != RET_SUCCESS)
		{
			DMCLOG_E("user table insert error");
			return -1;
		}

		ret = handle_bucket_table_insert(PUBLIC_BUCKET_NAME,user_id,&bucket_id);
		if(ret != RET_SUCCESS)
		{
			DMCLOG_E("buncket table insert error");
			return -1;
		}
		
		ret = handle_authority_table_insert(bucket_id,user_id,authority,&autority_id);
		if(ret != RET_SUCCESS)
		{
			DMCLOG_E("authority table insert error");
			return -1;
		}
		
		v_file_insert_t v_file_insert;
		memset(&v_file_insert,0,sizeof(v_file_insert_t));
		v_file_insert.cmd = V_FILE_TABLE_INSERT_INFO;
		v_file_insert.v_file_info.isDir = 1;
		S_STRNCPY(v_file_insert.bucket_name,PUBLIC_BUCKET_NAME,MAX_BUCKET_NAME_LEN);
		sprintf(v_file_insert.v_file_info.path,"/%s",PUBLIC_BUCKET_NAME);
		ret = _handle_v_file_table_insert(&v_file_insert);
		if(ret != 0)
		{
			DMCLOG_E("mkdir root file table error");
			return -1;
		}
	}else{
		DMCLOG_E("public user is exist");
		if(strcmp(c->password,PUBLIC_PASSWORD))
		{
			c->error = PASSWORD_ERROR;
			DMCLOG_E("password error");
			return -1;
		}
		char bucket_name[MAX_BUCKET_NAME_LEN] = {0};
		ret = handle_bucket_table_query(bucket_name,user_id,&bucket_id);
		if(ret != 0)
		{
			DMCLOG_E("bucket is not exist");
			c->error = FAIL;
			return -1;
		}
		DMCLOG_D("bucket_id = %d",bucket_id);
		ret = handle_authority_table_query(bucket_id,user_id,&authority,&autority_id);
		if(ret != 0)
		{
			DMCLOG_E("authority is not exist");
			c->error = FAIL;
			return -1;
		}
		
		DMCLOG_D("autority_id = %d",autority_id);
		v_file_insert_t v_file_insert;
		memset(&v_file_insert,0,sizeof(v_file_insert_t));
		v_file_insert.cmd = V_FILE_TABLE_INSERT_INFO;
		v_file_insert.v_file_info.isDir = 1;
		S_STRNCPY(v_file_insert.bucket_name,PUBLIC_BUCKET_NAME,MAX_BUCKET_NAME_LEN);
		sprintf(v_file_insert.v_file_info.path,"/%s",PUBLIC_BUCKET_NAME);
		ret = _handle_v_file_table_insert(&v_file_insert);
		if(ret != 0)
		{
			DMCLOG_E("mkdir root file table error");
			return -1;
		}
	}
	c->token = add_token_to_list(user_id,PUBLIC_BUCKET_NAME,authority,true,&c->ctx->p_token_list);
	return 0;
}

static int normal_user_process(struct conn *c)
{
	int user_id = 0;
	char password[MAX_PASSWORD_LEN] = {0};
	int ret = handle_user_table_query(c->username,password,&user_id);
	if(ret != 0)
	{
		DMCLOG_E("user is not exist");
		c->error = USERNAME_NOT_FOUND;
		return -1;
	}
	if(strcmp(password,c->password))
	{
		c->error = PASSWORD_ERROR;
		DMCLOG_E("password error");
		return -1;
	}
	char bucket_name[MAX_BUCKET_NAME_LEN] = {0};
	int bucket_id;
	ret = handle_bucket_table_query(bucket_name,user_id,&bucket_id);
	if(ret != 0)
	{
		DMCLOG_E("bucket is not exist");
		c->error = FAIL;
		return -1;
	}
	int authority;
	int autority_id;
	ret = handle_authority_table_query(bucket_id,user_id,&authority,&autority_id);
	if(ret != 0)
	{
		DMCLOG_E("authority is not exist");
		c->error = FAIL;
		return -1;
	}
	c->token = add_token_to_list(user_id,bucket_name,authority,false,&c->ctx->p_token_list);
	return 0;
}

static int dm_token_process(struct conn *c)
{
	ENTER_FUNC();
	int ret = 0;
	if(c->cmd == FN_DM_REGISTER)
	{
		DMCLOG_D("new user register");
	}else if(c->cmd == FN_DM_LOGIN)
	{
		DMCLOG_D("public name = %s",PUBLIC_USER_NAME);
		if(!strcmp(c->username,PUBLIC_USER_NAME))
		{
			ret = public_user_process(c);
			if(ret != 0)
			{
				DMCLOG_E("public user process error");
				c->error = FAIL;
				return -1;
			}
		}else{
			ret = normal_user_process(c);
			if(ret != 0)
			{
				DMCLOG_E("normal user process error");
				c->error = FAIL;
				return -1;
			}
		}
	}else if(c->cmd == FN_DM_LOGOUT)
	{
		if(c->token == 0)
		{
			DMCLOG_E("token is invalid");
			c->error = NOT_AUTHORIZATION;
			return -1;
		}
		ret = del_token_from_list(c->token,&c->ctx->p_token_list);	
		if(ret != 0)
		{
			DMCLOG_E("del token error");
			c->error = FAIL;
			return -1;
		}
	}else{
		if(c->token == 0)
		{
			DMCLOG_E("token is invalid");
			c->error = NOT_AUTHORIZATION;
			return -1;
		}
		if(is_token_login(c->token,&c->ctx->p_token_list) == false)
		{
			DMCLOG_E("token is not exist");
			c->error = NOT_AUTHORIZATION;
			return -1;
		}
		
	}
	return 0;
}



static int dm_file_token_process(struct conn *c)
{
	token_dnode_t *token_dnode = get_token_info(c->token,&c->ctx->p_token_list);
	if(token_dnode == NULL)
	{
		DMCLOG_E("token is not exist");
		c->error = NOT_AUTHORIZATION;
		return -1;
	}

	if(token_dnode->authority == 0)
	{
		DMCLOG_E("user have not PERMISSION");
		c->error = PERMISSION_DENIED;
		return -1;
	}
	return 0;
}


int dm_token_init(struct shttpd_ctx *ctx)
{
	token_list_init(&ctx->p_token_list);
	ctx->token_process = dm_token_process;
	ctx->file_token_process = dm_file_token_process;
}

int dm_token_destroy(struct shttpd_ctx *ctx)
{
	token_list_destroy(&ctx->p_token_list);
}


