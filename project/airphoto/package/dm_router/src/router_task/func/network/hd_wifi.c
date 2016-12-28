/*
 * =============================================================================
 *
 *       Filename:  hd_wifi.c
 *
 *    Description:  set and get wifi settings,set and get network settings
 *
 *        Version:  1.0
 *        Created:  2015/04/03 14:33
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

#include "hd_wifi.h"
#include "route.h"

int router_get_wifi_band(void)
{
	DMCLOG_D("router_get_wifi_band");
    return _router_get_wifi_band();
}



/*******************************************************************************
 * Function:
 * int dm_get_wifi_settings(hd_wifi_info *m_wifi_info);
 * Description:
 * get hidisk wifi information
 * Parameters:
 *    m_wifi_info   [OUT] wifi para
 * Returns:
 *    0:success,-1:others,-2:uci error,-3:cmd data error,-4:shell handle error
 *******************************************************************************/
int dm_get_wifi_settings(hd_wifi_info *m_wifi_info)
{
	DMCLOG_D("access dm_get_wifi_settings");
    return get_wifi_settings(m_wifi_info);
}


/*******************************************************************************
 * Function:
 * int dm_set_wifi_settings(hd_wifi_info *m_wifi_info);
 * Description:
 * set wifi information
 * Parameters:
 *    m_wifi_info   [IN] wifi para
 * Returns:
 *    0:success,-1:others,-2:uci error,-3:cmd data error
 *******************************************************************************/
int dm_set_wifi_settings(hd_wifi_info *m_wifi_info)
{
	DMCLOG_D("access dm_set_wifi_settings");
    return set_wifi_settings(m_wifi_info);
}


/*******************************************************************************
 * Function:
 * int dm_get_remote_ap_info(hd_wifi_info *m_wifi_info);
 * Description:
 * get remote ap information
 * Parameters:
 *    m_remote_info   [OUT] remote ap info
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_get_remote_ap_info(hd_remoteap_info *m_remote_info)
{
	DMCLOG_D("access dm_get_remote_ap_info");
    return get_remote_ap(m_remote_info);
}


/*******************************************************************************
 * Function:
 * int dm_set_remote_ap_info(hd_wifi_info *m_wifi_info);
 * Description:
 * set remote ap
 * Parameters:
 *    m_remote_info   [IN] remote ap info
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_set_remote_ap_info(hd_remoteap_info *m_remote_info)
{
	DMCLOG_D("access dm_set_remote_ap_info");
    return set_remote_ap(m_remote_info);
}

/*******************************************************************************
 * Function:
 * int dm_get_wlan_con_mode();
 * Description:
 * get wlan connection mode 0x0204
 * Parameters:
 *    NULL
 * Returns:
 *    0:wired;  1:wireless;  2:3G;  -1:failed
 *******************************************************************************/
int dm_get_wlan_con_mode()
{
	DMCLOG_D("access dm_get_wlan_con_mode");
	return _get_wlan_con_mode();
}

/*******************************************************************************
 * Function:
 * int dm_get_wired_con_mode(wired_con_mode_array *m_wired_con_array);
 * Description:
 * get wired 0x0206
 * Parameters:
 *    m_wired_con_array [OUT] wired connection array
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_get_wired_con_mode(wired_con_mode_array *m_wired_con_array)
{
	DMCLOG_D("access dm_get_wired_con_mode");
	return _get_wired_con_mode(m_wired_con_array);
}


/*******************************************************************************
 * Function:
 * int dm_set_wired_con_mode(wired_con_mode_array *m_wired_con_array);
 * Description:
 * set wired  0x0205
 * Parameters:
 *    m_wired_con_array [INT] wired connection array
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_set_wired_con_mode(wired_con_mode_array *m_wired_con_array)
{
	DMCLOG_D("access dm_set_wired_con_mode");
	return _set_wired_con_mode(m_wired_con_array);
}

/*******************************************************************************
 * Function:
 * int dm_get_wlan_list(ap_list_info_t *m_ap_list);
 * Description:
 * get ap list 0x0207
 * Parameters:
 *    m_ap_list [OUT] ap list info
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_get_wlan_list(ap_list_info_t *m_ap_list)
{
	DMCLOG_D("access dm_get_wlan_list");
	return dm_wlan_scan(m_ap_list);
}


/*******************************************************************************
 * Function:
 * int dm_get_client_status();
 * Description:
 * get client status 0x0219
 * Parameters:
 *    NULL
 * Returns:
 *    0:closed,1:open,-1:error
 *******************************************************************************/
int dm_get_client_status()
{
	DMCLOG_D("access dm_get_client_status");
	return _get_client_status();
}

/*******************************************************************************
 * Function:
 * int dm_set_client_status(int status);
 * Description:
 * set client status 0x021A
 * Parameters:
 *    status [IN] client status
 * Returns:
 *    0:success,-1:error
 *******************************************************************************/
int dm_set_client_status(int status)
{
	DMCLOG_D("access dm_set_client_status");
	return _set_client_status(status);
}

/*******************************************************************************
 * Function:
 * int dm_get_internet_status();
 * Description:
 * get the status of whether or net connect internet
 * Parameters:
 *    NULL
 * Returns:
 *    0:not connected,1:connected,-1:error
 *******************************************************************************/
int dm_get_internet_status()
{
	DMCLOG_D("access dm_get_internet_status");
	return 0;
}



