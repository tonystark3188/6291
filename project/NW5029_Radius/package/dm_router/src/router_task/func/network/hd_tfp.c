/*
 * =============================================================================
 *
 *       Filename:  hd_ftp.c
 *
 *    Description:  set and get ftp settings
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
 * int dm_get_ftp_settings(dm_net_info *m_net_info);
 * Description:
 * get ftp infomation 0x020B
 * Parameters:
 *    m_net_info [struct OUT] ftp settings
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_get_ftp_settings(dm_net_info *m_net_info)
{
	DMCLOG_D("dm_get_ftp_settings");
}

/*******************************************************************************
 * Function:
 * int dm_set_ftp_settings(dm_net_info *m_net_info);
 * Description:
 * get ftp infomation 0x020C
 * Parameters:
 *    m_net_info [struct INT] ftp settings
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_set_ftp_settings(dm_net_info *m_net_info)
{
	DMCLOG_D("dm_set_ftp_settings");
	return 0;
}

 #endif

