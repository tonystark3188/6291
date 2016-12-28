/*
 * =============================================================================
 *
 *       Filename:  hd_ddns.c
 *
 *    Description:  set and get ddns settings
 *
 *        Version:  1.0
 *        Created:  2015/04/08 10:35
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
 * int dm_get_ddns_settings(dm_net_info *m_net_info);
 * Description:
 * get ddns infomation 0x0211
 * Parameters:
 *    m_net_info [struct OUT] ddns settings
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_get_ddns_settings(dm_ddns_list *m_ddns_list)
{
	DMCLOG_D("access dm_get_ddns_settings");
	return 0;
}
/*******************************************************************************
 * Function:
 * int dm_set_ddns_settings(dm_net_info *m_net_info);
 * Description:
 * get ddns infomation 0x0212
 * Parameters:
 *    m_net_info [struct INT] ddns settings
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_set_ddns_settings(dm_ddns_list *m_ddns_list)
{
	DMCLOG_D("dm_set_ddns_settings");
	return 0;
}
#endif
