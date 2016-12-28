/*
 * =============================================================================
 *
 *       Filename:  config.c
 *
 *    Description:  AirNas config module
 *
 *        Version:  1.0
 *        Created:  2014/9/5 13:50:00
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ye Wenhao (), wenhaoye@126.com
 *   Organization:  
 *
 * =============================================================================
 */

#include "config.h"

SystemConfigInfo * g_p_sys_conf_info = NULL;




/* **********************************************************************
 * call by other module for get system config info. 
 *
 * **********************************************************************/

// get db version
const char * get_sys_db_version(void)
{
    return g_p_sys_conf_info->db_ver;
}

const char *get_sys_nas_data_path(void)
{
    return g_p_sys_conf_info->nas_data_path;
}

const char * get_sys_dm_db_path(void)
{
	sprintf(g_p_sys_conf_info->nas_root_path, AIRDISK_ROOT_PATH);
	DMCLOG_D("g_p_sys_conf_info->nas_root_path = %s",g_p_sys_conf_info->nas_root_path);
	sprintf(g_p_sys_conf_info->nas_db_path, "%s/%s", g_p_sys_conf_info->nas_root_path, NAS_DB_DIR_NAME);
	mkdir(g_p_sys_conf_info->nas_db_path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	DMCLOG_D("g_p_sys_conf_info->nas_db_path = %s",g_p_sys_conf_info->nas_db_path);
    return g_p_sys_conf_info->nas_db_path;
}


//"1.0.11" --> major=1,minor=0,revision=11
void version_to_digit(const char *version, digit_version_t *dv)
{
    int i,j,k;
    char tmp[32];
	int d[3];
	
	for(i=0,j=0,k=0; version[i]!=0; i++)
	{
	    if(version[i] != '.')
	    {
			tmp[j++] = version[i];
		}
		else
		{
			tmp[j] = 0;
			j=0;
			d[k++] = atoi(tmp);
		}
	}

	tmp[j] = 0;
	d[k] = atoi(tmp);

	dv->major = d[0];
	dv->minor = d[1];
	dv->revision = d[2];
}


