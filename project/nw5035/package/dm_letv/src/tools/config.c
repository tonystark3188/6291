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


// save with json format!
#ifdef PLATFORM_X86
#define AIRNAS_SERVER_CONFIG_FILE "airnas_server_conf"
#else
#define AIRNAS_SERVER_CONFIG_FILE "/usr/sbin/airnas_server_conf"
#endif

static uint8_t g_nas_root_path_len = 0;
static uint8_t g_nas_data_path_len = 0;

SystemConfigInfo * g_p_sys_conf_info = NULL;




/* **********************************************************************
 * call by other module for get system config info. 
 *
 * **********************************************************************/

// get the port of nas device discover service.
uint16_t get_nas_dev_disc_port(void)
{
    return g_p_sys_conf_info->nas_dev_disc_port;
}

// get the listen port of nas server.
uint16_t get_nas_serv_port(void)
{
    return g_p_sys_conf_info->nas_server_port;
}

// get the publish server port
uint16_t get_nas_pub_serv_port(void)
{
    return g_p_sys_conf_info->publish_server_port;
}

// get nas fw version.
const char * get_nas_fw_ver(void)
{
    return g_p_sys_conf_info->fw_ver;
}

// get nas fw version.
void get_nas_fw_ver_ext(char *ver_str, size_t size)
{
    S_STRNCPY(ver_str, g_p_sys_conf_info->fw_ver, size);
    return;
}

// get db version
const char * get_sys_db_version(void)
{
    return g_p_sys_conf_info->db_ver;
}

// get nas fw code. Used inside.
int get_nas_fw_code(void)
{
    return g_p_sys_conf_info->fw_code;
}

// get nas api version code
uint16_t get_nas_api_ver_code(void)
{
    return g_p_sys_conf_info->api_ver_code;
}

// get nas force update flag
uint16_t get_nas_force_update_flag(void)
{
    return g_p_sys_conf_info->force_update_flag;
}

// get system platfrom type.
PlatformType get_sys_platform_type(void)
{
    return g_p_sys_conf_info->type;
}

uint8_t get_nas_idle_threshold(void)
{
    return g_p_sys_conf_info->idle_threshold;
}

const char *get_sys_nas_data_path(void)
{
    return g_p_sys_conf_info->nas_data_path;
}

const char *get_sys_nas_system_path(void)
{
    return g_p_sys_conf_info->nas_system_path;
}

const char * get_sys_nas_db_path(void)
{
	ENTER_FUNC();
	sprintf(g_p_sys_conf_info->nas_root_path, AIRDISK_ROOT_PATH);
	p_debug("g_p_sys_conf_info->nas_root_path = %s",g_p_sys_conf_info->nas_root_path);
	sprintf(g_p_sys_conf_info->nas_db_path, "%s/%s", 
            g_p_sys_conf_info->nas_root_path, NAS_DB_DIR_NAME);
	p_debug("g_p_sys_conf_info->nas_db_path = %s",g_p_sys_conf_info->nas_db_path);
	EXIT_FUNC();
    return g_p_sys_conf_info->nas_db_path;
}

const char * get_sys_nas_root_path(void)
{
    return g_p_sys_conf_info->nas_root_path;
}

const char * get_sys_nas_share_path(void)
{
    return g_p_sys_conf_info->nas_share_path;
}

const char * get_sys_nas_backup_path(void)
{
    return g_p_sys_conf_info->nas_backup_path;  
}

const char * get_sys_nas_log_path(void)
{
    return g_p_sys_conf_info->nas_log_path;
}

const char * get_sys_nas_log_name(void)
{
    return g_p_sys_conf_info->sw_conf_info.log_name;
}



// get network interface
const char *get_eth_wan_inf(void)
{
    return (g_p_sys_conf_info->eth_wan_inf);
}

const char *get_wifi_ap_inf(void)
{
    return (g_p_sys_conf_info->wifi_ap_inf);
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


