
/*
 * =============================================================================
 *
 *       Filename:  hd_3g.c
 *
 *    Description:  set and get 3g access settings
 *
 *        Version:  1.0
 *        Created:  2015/04/08 11:01
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
 * int dm_get_3g_access_info(dm_3g_access_info *m_3g_info);
 * Description:
 * get 3g access infomation 0x0217
 * Parameters:
 *    m_3g_info [struct OUT] 3g access settings
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_get_3g_access_info(dm_3g_access_info *m_3g_info)
{
	DMCLOG_D("access dm_get_3g_access_info");
	int ret = -1;
	char cmd_buf[TEMP_BUFFER_SIZE];
    char temp_buf[TEMP_BUFFER_SIZE];
#ifdef SUPPORT_OPENWRT_PLATFORM
	memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
	strcpy(cmd_buf, "3g.@3g[0].apnmode");
	memset(temp_buf, 0, TEMP_BUFFER_SIZE);
	ret = uci_get_option_value(cmd_buf, &temp_buf);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	else
	{
		if(strcmp(temp_buf, "NULL"))	
			m_3g_info->apnmode = atoi(temp_buf);
	}
		
	memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
	strcpy(cmd_buf,"3g.@3g[0].apn");
	memset(temp_buf, 0, TEMP_BUFFER_SIZE);
	ret = uci_get_option_value(cmd_buf, &temp_buf);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	else
	{
		if(strcmp(temp_buf, "NULL"))
			strcpy(m_3g_info->apn, temp_buf);
	}

	memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
	strcpy(cmd_buf, "3g.@3g[0].username"); 	
	memset(temp_buf, 0, TEMP_BUFFER_SIZE);
	ret = uci_get_option_value(cmd_buf, &temp_buf);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	else
	{
		if(strcmp(temp_buf, "NULL"))
			strcpy(m_3g_info->username, temp_buf);
	}

	memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
	strcpy(cmd_buf, "3g.@3g[0].password");	
	memset(temp_buf, 0, TEMP_BUFFER_SIZE);
	ret = uci_get_option_value(cmd_buf, &temp_buf);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	else
	{
		if(strcmp(temp_buf, "NULL"))
			strcpy(m_3g_info->password, temp_buf);
	}

	memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
	strcpy(cmd_buf, "3g.@3g[0].dialnumber"); 
	memset(temp_buf, 0, TEMP_BUFFER_SIZE);
	ret = uci_get_option_value(cmd_buf, &temp_buf);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	else
	{
		if(strcmp(temp_buf, "NULL"))
			strcpy(m_3g_info->dialnumber, temp_buf);
	}

	
	memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
	strcpy(cmd_buf, "3g.@3g[0].operator"); 
	memset(temp_buf, 0, TEMP_BUFFER_SIZE);
	ret = uci_get_option_value(cmd_buf, &temp_buf);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	else
	{
		if(strcmp(temp_buf, "NULL"))
			strcpy(m_3g_info->operator, temp_buf);
	}


	memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
	strcpy(cmd_buf, "3g.@3g[0].status"); 
	memset(temp_buf, 0, TEMP_BUFFER_SIZE);
	ret = uci_get_option_value(cmd_buf, &temp_buf);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	else
	{
		if(strcmp(temp_buf, "NULL"))
			m_3g_info->status = atoi(temp_buf);
	}


	memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
	strcpy(cmd_buf, "3g.@3g[0].multista"); 
	memset(temp_buf, 0, TEMP_BUFFER_SIZE);
	ret = uci_get_option_value(cmd_buf, &temp_buf);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	else
	{
		if(strcmp(temp_buf, "NULL"))
			m_3g_info->status2 = atoi(temp_buf);
	}

	if(m_3g_info->status == 2)
	{

		memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
		strcpy(cmd_buf, "3g.@3g[0].ipaddr"); 
		memset(temp_buf, 0, TEMP_BUFFER_SIZE);
		ret = uci_get_option_value(cmd_buf, &temp_buf);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
		else
		{
			if(strcmp(temp_buf, "NULL"))
				strcpy(m_3g_info->ipaddr, temp_buf);
		}

		memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
		strcpy(cmd_buf, "3g.@3g[0].gateway"); 
		memset(temp_buf, 0, TEMP_BUFFER_SIZE);
		ret = uci_get_option_value(cmd_buf, &temp_buf);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
		else
		{
			if(strcmp(temp_buf, "NULL"))
				strcpy(m_3g_info->gateway, temp_buf);
		}

		memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
		strcpy(cmd_buf, "3g.@3g[0].netmask"); 
		memset(temp_buf, 0, TEMP_BUFFER_SIZE);
		ret = uci_get_option_value(cmd_buf, &temp_buf);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
		else
		{
			if(strcmp(temp_buf, "NULL"))
				strcpy(m_3g_info->netmask, temp_buf);
		}

		memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
		strcpy(cmd_buf, "3g.@3g[0].dns"); 
		memset(temp_buf, 0, TEMP_BUFFER_SIZE);
		ret = uci_get_option_value(cmd_buf, &temp_buf);
		if(ret < 0)
		{
			return ROUTER_ERRORS_UCI;
		}
		else
		{
			if(strcmp(temp_buf, "NULL"))
				strcpy(m_3g_info->dns, temp_buf);
		}
	}
#endif
#ifdef SUPPORT_LINUX_PLATFORM
#endif
	return ROUTER_OK;
}
/*******************************************************************************
 * Function:
 * int dm_set_3g_access_info(dm_3g_access_info *m_3g_info);
 * Description:
 * get 3g access infomation 0x0217
 * Parameters:
 *    m_3g_info [struct IN] 3g access settings
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_set_3g_access_info(dm_3g_access_info *m_3g_info)
{
	DMCLOG_D("access dm_set_3g_access_info");
	int ret= -1;
	char cmd_buf[TEMP_BUFFER_SIZE];
    char temp_buf[TEMP_BUFFER_SIZE];
#ifdef SUPPORT_OPENWRT_PLATFORM
	memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
	strcpy(cmd_buf, "wireless.@wifi-iface[1].disabled=1");
	ret = uci_set_option_value(cmd_buf);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	//system("uci commit");

	memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
	sprintf(cmd_buf, "3g.@3g[0].apnmode=%s", m_3g_info->apnmode);
	ret = uci_set_option_value(cmd_buf);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}

	if(!strcmp(m_3g_info->apnmode, "auto"))
	{
		if(strlen(m_3g_info->apn)!=0)
		{	
			memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
			sprintf(cmd_buf, "3g.@3g[0].apn=%s", m_3g_info->apn);
			ret = uci_set_option_value(cmd_buf);
			if(ret < 0)
			{
				return ROUTER_ERRORS_UCI;
			}
		}
		else
		{
			memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
			sprintf(cmd_buf, "3g.@3g[0].apn=NULL");
			ret = uci_set_option_value(cmd_buf);
			if(ret < 0)
			{
				return ROUTER_ERRORS_UCI;
			}
		}
		
		if(strlen(m_3g_info->username)!=0)
		{
			memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
			sprintf(cmd_buf, "3g.@3g[0].username=%s",m_3g_info->username);
			ret = uci_set_option_value(cmd_buf);
			if(ret < 0)
			{
				return ROUTER_ERRORS_UCI;
			}
		}
		else
		{
			memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
			sprintf(cmd_buf, "3g.@3g[0].username=NULL");
			ret = uci_set_option_value(cmd_buf);
			if(ret < 0)
			{
				return ROUTER_ERRORS_UCI;
			}
		}
		
		if(strlen(m_3g_info->password)!=0)
		{
			memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
			sprintf(cmd_buf, "3g.@3g[0].password=%s", m_3g_info->password);
			ret = uci_set_option_value(cmd_buf);
			if(ret < 0)
			{
				return ROUTER_ERRORS_UCI;
			}
		}
		else
		{
			memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
			sprintf(cmd_buf, "3g.@3g[0].password=NULL");
			ret = uci_set_option_value(cmd_buf);
			if(ret < 0)
			{
				return ROUTER_ERRORS_UCI;
			}
		}
		
		if(strlen(m_3g_info->dialnumber)!=0)
		{
			memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
			sprintf(cmd_buf, "3g.@3g[0].dialnumber=%s", m_3g_info->dialnumber);
			ret= uci_set_option_value(cmd_buf);
			if(ret < 0)
			{
				return ROUTER_ERRORS_UCI;
			}
		}
		else
		{			
			memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
			sprintf(cmd_buf, "3g.@3g[0].dialnumber=NULL");
			ret = uci_set_option_value(cmd_buf);
			if(ret < 0)
			{
				return ROUTER_ERRORS_UCI;
			}
		}
		
	}

	memset(cmd_buf, 0, TEMP_BUFFER_SIZE);
	sprintf(cmd_buf, "network.wan.workmode=2");
	ret = uci_set_option_value(cmd_buf);
	if(ret < 0)
	{
		return ROUTER_ERRORS_UCI;
	}
	//system("uci commit");

	system("killall -9 pppd");
	system("touch /tmp/3g_reconnect");
#endif
#ifdef SUPPORT_LINUX_PLATFORM
#endif
	return ROUTER_OK;
}
#endif
