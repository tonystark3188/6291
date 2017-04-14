/*
 * =============================================================================
 *
 *       Filename:  hd_dms.c
 *
 *    Description:  set and get dms settings
 *
 *        Version:  1.0
 *        Created:  2015/04/08 10:25
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver, 
 *   Organization:  
 *
 * =============================================================================*/



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hd_net.h"
#include "router_defs.h"

#if 0
/*******************************************************************************
 * Function:
 * int dm_get_dms_settings(dm_net_info *m_net_info);
 * Description:
 * get dms infomation 0x020F
 * Parameters:
 *    m_net_info [struct OUT] dms settings
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_get_dms_settings(dm_net_info *m_net_info)
{
	DMCLOG_D("access dm_get_dms_settings");
	int ret = -1;
	char name[64]="\0";
	char status[5]="\0";
	char path[32]={0};
	char uci_option_str[64]="\0";
	char *pencode_name = NULL;
#ifdef SUPPORT_OPENWRT_PLATFORM
	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"ushare.@ushare[0].servername");  //name
	ret = uci_get_option_value(uci_option_str,name);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	
	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"ushare.@ushare[0].enabled");  //status
	ret = uci_get_option_value(uci_option_str,status);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	
	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"network.lan.ipaddr");  //ip
	ret = uci_get_option_value(uci_option_str,path);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}

	if(status[0]=='1')
		m_net_info->status = 1;
	else
		m_net_info->status = 0;

	strcpy(m_net_info->name, name);
	strcpy(m_net_info->path, path);
	m_net_info->enable = 1;
#endif
#ifdef SUPPORT_LINUX_PLATFORM
#endif
	return 0;
}

/*******************************************************************************
 * Function:
 * int dm_set_dms_settings(dm_net_info *m_net_info);
 * Description:
 * get dms infomation 0x0210
 * Parameters:
 *    m_net_info [struct INT] dms settings
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_set_dms_settings(dm_net_info *m_net_info)
{
	DMCLOG_D("access dm_set_dms_settings");
	int ret = -1;
	int enable;
	char name[64]="\0";
	char enable_config[3]="\0";
	char str_sp[64]="\0";
#ifdef SUPPORT_OPENWRT_PLATFORM
	enable = m_net_info->status;
	strcpy(name, m_net_info->name);

	if((0 == enable) || (1 == enable))
	{
		memset(str_sp, '\0', 64);
		sprintf(str_sp,"ushare.@ushare[0].enabled=%d",enable);
		ret = uci_set_option_value(str_sp);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
	}

	if(NULL != name)
	{
		memset(str_sp, '\0', 64);
		sprintf(str_sp,"ushare.@ushare[0].servername=%s",name);
		ret = uci_set_option_value(str_sp);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
	}

	//system("uci commit");
	if(1 == enable)
	{
		system("/etc/init.d/ushare stop >/dev/null 2>&1");
		sleep(2);
		system("/etc/init.d/ushare start >/dev/null 2>&1");
	}else if(0 == enable)
	{
		system("/etc/init.d/ushare stop >/dev/null 2>&1");
	}
#endif
#ifdef SUPPORT_LINUX_PLATFORM
#endif
	return 0;
}
#endif
