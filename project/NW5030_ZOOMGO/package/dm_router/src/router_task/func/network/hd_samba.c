/*
 * =============================================================================
 *
 *       Filename:  hd_samba.c
 *
 *    Description:  set and get samba settings
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
 * int dm_get_smb_settings(dm_net_info *m_net_info);
 * Description:
 * get samba infomation 0x020D
 * Parameters:
 *    m_net_info [struct OUT] samba settings
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_get_smb_settings(dm_net_info *m_net_info)
{
	DMCLOG_D("dm_get_smb_settings");
	int ret = -1;
	char user[64] = "\0";
	char password[64] = "\0";
	char anonymous_en[5] = "\0";
	char path[64] = {0};
	char status[5] = "\0";
	char uci_option_str[64]="\0";
	char *pencode_password = NULL;
#ifdef SUPPORT_OPENWRT_PLATFORM
	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"system.@system[0].hostname");
	ret = uci_get_option_value(uci_option_str,path);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	
	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"samba.@samba[0].user");  //user
	ret = uci_get_option_value(uci_option_str,user);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	
	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"samba.@samba[0].password");  //password
	ret = uci_get_option_value(uci_option_str,password);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	
	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"samba.@sambashare[0].guest_ok");  //anonymous_en
	ret = uci_get_option_value(uci_option_str,anonymous_en);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	
	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"samba.@samba[0].enabled");  //status
	ret = uci_get_option_value(uci_option_str,status);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}

	if(!strcmp(anonymous_en,"yes"))
		m_net_info->anonymous_en = 1;
	else
		m_net_info->anonymous_en = 0;

	if(status[0]=='1')
		m_net_info->status = 1;
	else
		m_net_info->status = 0;

	strcpy(m_net_info->path, path);
	strcpy(m_net_info->user, user);
	strcpy(m_net_info->password, password);
	m_net_info->port = 137;
	m_net_info->enable = 1;
#endif
#ifdef SUPPORT_LINUX_PLATFORM
#endif
	return ROUTER_OK;
}

/*******************************************************************************
 * Function:
 * int dm_set_smb_settings(dm_net_info *m_net_info);
 * Description:
 * set samba infomation 0x020E
 * Parameters:
 *    m_net_info [struct INT] samba settings
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_set_smb_settings(dm_net_info *m_net_info)
{
	DMCLOG_D("dm_set_smb_settings");
	int ret =-1;
	char user[64]="\0";
	int enable;
	char password[64]="\0";
	int anonymous_en;
	char anonymous_en_config[8]="\0";
	char smb_usr_name[64]="\0";
	char str_sp[64]="\0";
	char uci_option_str[64]="\0";
#ifdef SUPPORT_OPENWRT_PLATFORM
	enable = m_net_info->status;
	anonymous_en = m_net_info->anonymous_en;
	strcpy(password, m_net_info->password);

	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"samba.@samba[0].user");  
	ret = uci_get_option_value(uci_option_str,smb_usr_name);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	
	if((0 == enable) || (1 == enable))
	{
		memset(str_sp, '\0', 64);
		sprintf(str_sp,"samba.@samba[0].enabled=%d",enable);
		ret = uci_set_option_value(str_sp);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
	}

	if((0 == anonymous_en) || (1 == anonymous_en))
	{
		if(1 == anonymous_en)
		{
			strcpy(anonymous_en_config,"yes");
			system("uci delete samba.@sambashare[0].users");
		}
		else
		{
			memset(str_sp, '\0', 64);
			strcpy(anonymous_en_config,"no");
			sprintf(str_sp,"samba.@sambashare[0].users=%s",smb_usr_name);
			ret = uci_set_option_value(str_sp);
			if(ret < 0)
			{
				return ROUTER_ERRORS_UCI;
			}
		}
		memset(str_sp, '\0', 64);
		sprintf(str_sp,"samba.@sambashare[0].guest_ok=%s",anonymous_en_config);
		ret = uci_set_option_value(str_sp);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
	}

	if(NULL != password)
	{
		memset(str_sp, '\0', 64);
		sprintf(str_sp,"samba.@samba[0].password=%s",password);
		ret = uci_set_option_value(str_sp);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
	}

	//system("uci commit");
	if(1 == enable)
	{
		system("/etc/init.d/samba stop >/dev/null 2>&1");
		system("/etc/init.d/samba start >/dev/null 2>&1");	
	}
	else if(0 == enable)
	{
		system("/etc/init.d/samba stop >/dev/null 2>&1");
	}
#endif
	return ROUTER_OK;
}
#endif

