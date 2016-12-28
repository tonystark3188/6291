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
const char * get_sys_dm_router_version(void)
{
    return g_p_sys_conf_info->router_ver;
}

const char * get_sys_fw_version(void)
{
    return g_p_sys_conf_info->fw_ver;
}

const char * get_sys_product_model(void)
{
    return g_p_sys_conf_info->product_model;
}

const char *get_sys_nas_data_path(void)
{
    return g_p_sys_conf_info->nas_data_path;
}

const char *get_sys_disk_uuid_name(void)
{
    return g_p_sys_conf_info->uuid_name;
}

const char *get_sys_db_name(void)
{
    return g_p_sys_conf_info->db_name;
}

int get_sys_file_port(void)
{
    return g_p_sys_conf_info->airdisk_file_port;
}

int get_sys_router_port(void)
{
    return g_p_sys_conf_info->airdisk_router_port;
}

int get_sys_init_port(void)
{
    return g_p_sys_conf_info->airdisk_init_port;
}

int get_func_list_flag(void)
{
	return g_p_sys_conf_info->fun_list_flag;
}

int get_database_sign(void)
{
	return g_p_sys_conf_info->database_sign;
}


int set_database_sign(uint16_t sign)
{
	g_p_sys_conf_info->database_sign = sign;
}

int get_token_watch_time(void)
{
	return g_p_sys_conf_info->session_watch_time;
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


