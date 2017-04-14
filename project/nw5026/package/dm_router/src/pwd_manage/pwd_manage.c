/*
 * =============================================================================
 *
 *       Filename:  pwd_manage.c
 *
 *    Description:  password management
 *
 *        Version:  1.0
 *        Created:  2016/08/10 15:28
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver, 
 *   Organization:  
 *
 * =============================================================================*/
 #include "pwd_manage.h"
/*
* function:
*  get the password from cfg
* para: 
*  pwd:root password (output)
* return:
*  0:succ
*  -1:failed
*/
int dm_get_password(_Out_ char *pwd)
{
	if(pwd == NULL)
	{
		return -1;
	}
	//TODO
	return dm_get_root_pwd(pwd);
}

/*
* function:
*  set the password to cfg
* para: 
*  pwd:root password (input)
* return:
*  0:succ
*  -1:failed
*/
int dm_set_password(_In_ char *pwd)
{
	char root_pwd[32] = {0};
	if(pwd == NULL)
	{
		return -1;
	}
	if(dm_get_password(root_pwd) == 0) 
	{
		DMCLOG_E("the root password is exist");
		return -1;
	}
	//TODO
	return dm_set_root_pwd(pwd);
}

/*
* function:
*  match the password
* para: 
*  old_pwd:old password (input)
* return:
*  0:not match
*  1:match
*  -1:error
*/
int dm_match_password(_In_ char *pwd)
{
	char root_pwd[64] = {0};
	int ret = -1;
	if(pwd == NULL)
	{
		return -1;
	}
	//TODO
	ret = dm_get_password(root_pwd);
	if(ret != 0)
	{
		DMCLOG_E("dm get pwd error");
		return -1;
	}
	DMCLOG_D("pwd = %s,root_pwd = %s,len:%d,len:%d",pwd,root_pwd,strlen(pwd),strlen(root_pwd));
	if(strcmp(pwd,root_pwd) == 0)
	{
		DMCLOG_D("password match");
		return 1;
	}
	return 0;
}


/*
* function:
*  reset the password to cfg
* para: 
*  old_pwd:old password (input)
*  new_pwd:new password (input)
* return:
*  0:succ
*  -1:failed
*/
int dm_reset_password(_In_ char *old_pwd,_In_ char *new_pwd)
{
	if(old_pwd == NULL||new_pwd == NULL)
	{
		return -1;
	}
	//TODO
	if(dm_match_password(old_pwd) == 1)
	{
		return dm_set_root_pwd(new_pwd);
	}
	return -1;
}

/*
* function:
*  the password is or not exist
* para: 
* return:
*  0:not exist
*  1:exist
*/
bool dm_password_exist()
{
	return dm_root_pwd_exist();
}



