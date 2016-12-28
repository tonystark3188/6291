/*
 * =====================================================================================
 *
 *       Filename:  db_mm_pool.c
 *
 *    Description:  database memory pool module
 *
 *        Version:  1.0
 *        Created:  2015/10/6 14:38:13
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver ()
 *   Organization:  
 *
 * =====================================================================================
 */

#include "db/file_table.h"
#include "base.h"


static file_info_t * _new_db_fd(uint8_t no_wait)
{
	file_info_t *p_db_fd = NULL;
	p_db_fd = (file_info_t *)calloc(1,sizeof(file_info_t));
	if(p_db_fd == NULL)
	{
		DMCLOG_D("request_mm_unit failed!");
		return NULL;
	}
	return p_db_fd;
}	

file_info_t * new_db_fd(void)
{
	return _new_db_fd(0);
}

file_info_t * new_db_fd_no_wait(void)
{
	return _new_db_fd(1);
}

int free_db_fd(file_info_t **pp_db_fd)
{
	file_info_t *p_db_fd = NULL;

	if(pp_db_fd == NULL)
	{
		log_warning("NULL pointer!");
		return -1;	
	}

	p_db_fd = (*pp_db_fd);
	safe_free(p_db_fd->path);
	safe_free(p_db_fd->name);
	safe_free(p_db_fd);
	p_db_fd = NULL;

	return 0;
}

