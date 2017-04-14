/*
 * =============================================================================
 *
 *       Filename:  hd_net.h
 *
 *    Description: network software settings
 *
 *        Version:  1.0
 *        Created:  2015/04/08 9:27
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver, 
 *   Organization:  
 *
 * =============================================================================*/


#ifndef _HD_NET_H_
#define _HD_NET_H_

#include <unistd.h>
#include <stdio.h>
#include "base.h"
#ifdef __cplusplus
extern "C"{
#endif

#if 0
typedef struct net_info{
	char user[64];
	char password[64];
	int port;
	char path[256];
	int status; //0:close;1:open
	int anonymous_en; //0:不允许匿名登陆，1:允许匿名登陆
	int enable;
	char name[64];//DMS name
}dm_net_info;

typedef struct ddns_settings{
	char name[64]; //DDNS服务提供商，例：oray（花生壳）
	char domain[64];//DDNS服务提供商域名www.oray.com
	char user[64];
	char password[64];
	int status;
}dm_ddns_settings;
typedef struct ddns_list{
	int enable;
	int count;
	dm_ddns_settings m_ddns_settings[10];
}dm_ddns_list;

typedef struct g_access_info
{
	int apnmode; //拨号方式
	char apn[32];
	char dialnumber[32];
	char username[32];
	char operator[32];
	char password[32];
	char ipaddr[32];
	char gateway[32];
	char netmask[32];
	char dns[32];
	int status;
	int status2;
	int enable;
}dm_3g_access_info;



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
int dm_get_ftp_settings(dm_net_info *m_net_info);

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
int dm_set_ftp_settings(dm_net_info *m_net_info);

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
int dm_get_smb_settings(dm_net_info *m_net_info);

/*******************************************************************************
 * Function:
 * int dm_set_smb_settings(dm_net_info *m_net_info);
 * Description:
 * get samba infomation 0x020E
 * Parameters:
 *    m_net_info [struct INT] ftp settings
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_set_smb_settings(dm_net_info *m_net_info);

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
int dm_get_dms_settings(dm_net_info *m_net_info);

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
int dm_set_dms_settings(dm_net_info *m_net_info);

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
int dm_get_ddns_settings(dm_ddns_list *m_ddns_list);

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
int dm_set_ddns_settings(dm_ddns_list *m_ddns_list);

/*******************************************************************************
 * Function:
 * int dm_get_webdav_settings(dm_net_info *m_net_info);
 * Description:
 * get webdav infomation 0x0213
 * Parameters:
 *    m_net_info [struct OUT] webdav settings
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_get_webdav_settings(dm_net_info *m_net_info);

/*******************************************************************************
 * Function:
 * int dm_set_webdav_settings(dm_net_info *m_net_info);
 * Description:
 * get webdav infomation 0x0214
 * Parameters:
 *    m_net_info [struct INT] webdav settings
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_set_webdav_settings(dm_net_info *m_net_info);

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
int dm_get_3g_access_info(dm_3g_access_info *m_3g_info);
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
int dm_set_3g_access_info(dm_3g_access_info *m_3g_info);
#endif
#ifdef __cplusplus
}
#endif



#endif

