/*
 * =============================================================================
 *
 *       Filename:  hd_wifi.h
 *
 *    Description:  private disk process
 *
 *        Version:  1.0
 *        Created:  2015/04/03 10:51
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver, 
 *   Organization:  
 *
 * =============================================================================*/


#ifndef _HD_WIFI_H_
#define _HD_WIFI_H_

#include <unistd.h>
#include <stdio.h>
#include "base.h"
#ifdef __cplusplus
extern "C"{
#endif

#define PPPOE_MODE  1
#define DHCP_MODE   2
#define STATIC_MODE 3
#define encrypt_none "NONE"
#define encrypt_wep "WEP"
#define encrypt_wpapsk_tkip "WPAPSK/TKIP"
#define encrypt_wpapsk_aes "WPAPSK/AES"
#define encrypt_wpapsk_tkip_aes "WPAPSK/TKIPAES"
#define encrypt_wpa2psk_aes "WPA2PSK/AES"
#define encrypt_wpa2psk_tkip "WPA2PSK/TKIP"
#define encrypt_wpa2psk_tkip_aes "WPA2PSK/TKIPAES"
#define encrypt_wpa2_wpa1_psk_aes "WPA1PSKWPA2PSK/AES"
#define encrypt_wpa2_wpa1_psk_tkip "WPA1PSKWPA2PSK/TKIP"
#define encrypt_wpa2_wpa1_psk_tkip_aes "WPA1PSKWPA2PSK/TKIPAES"

typedef struct wirelessInfo
{
	int wifi_type;				/* 1:2.4G,2:5G */
	int disabled; 				/* 0:open 1:close */
	char ssid[64];				/* ssid name */
	char encrypt[32];			/* encryption method */
	char tkip_aes[16];			/* encryption algorithm */
	char wifi_password[64]; 	/* wifi password */
	int channel;				/* channel */	
	int encrypt_len;			/* 64 bit or 128 bit for wep*/
	char format[32];			/* HEX or ASCII format for wep */
	char mac[32];				/* mac */
}hd_wifi_info;

typedef struct remoteAP
{
	char ssid[64];				/* ssid name */
	char mac[32];				/* mac */
	char password[64];			/* remote ap password */
	int channel;				/* channel */
	char encrypt[32];			/* encryption method */
	char tkip_aes[16];			/* encryption algorithm */
	int is_connect;				/* 0:no connect;1:connect success */	
}hd_remoteap_info;

typedef struct wired_mode{    
	int con_type; //连接方式
	int enable;// 0:禁止,1:允许
	int is_connect; //是否连接
	char adsl_name[64];//broadband account
	char adsl_password[64];//broadband password
	char ip[32];
	char dns1_ip[32];
	char dns2_ip[32];
	char netmask[32];//subnet mask
	char gateway[32];
}wired_con_mode;
typedef struct wired_connection{    
	int status;
	int enable;
	wired_con_mode m_wired_mode[3];
}wired_con_mode_array; 


typedef struct _scan_AP{
	int has_encrypt;
	int encrypt;
	int rssidbm;
	char mac_addr[18];
	char ssid[64];
	int channel;
} scan_ap_struct;

typedef struct ap_info{
	char ssid[32];				/* ssid name */
	int ssid_len;				/* ssid length */
	char mac[32];				/* mac */
	int channel;				/* channel */
	char encrypt[32];			/* encryption method */
	char tkip_aes[16];			/* encryption algorithm */
	int wifi_signal;			/* ap signal */
	int record;					/* 0:no record;1:has record */
	char password[64];			/* record ap password */
}ap_info_t;

typedef struct ap_list_info{    
	int count;					/* the count of ap */
	char fre[8];				/* 2.4G or 5G */
	ap_info_t ap_info[100];		/* ap list */
}ap_list_info_t;

typedef struct forget_wifi_info
{
	char ssid[64];				/* ssid name */
	char mac[32];				/* mac */
}forget_wifi_info_t;

/*******************************************************************************
 * Function:
 * int dm_get_wifi_settings(hd_wifi_info *m_wifi_info);
 * Description:
 * get hidisk wifi information
 * Parameters:
 *    m_wifi_info   [OUT] wifi para
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_get_wifi_settings(hd_wifi_info *m_wifi_info);


/*******************************************************************************
 * Function:
 * int dm_set_wifi_settings(hd_wifi_info *m_wifi_info);
 * Description:
 * set wifi information
 * Parameters:
 *    m_wifi_info   [IN] wifi para
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_set_wifi_settings(hd_wifi_info *m_wifi_info);


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
int dm_get_remote_ap_info(hd_remoteap_info *m_remote_info);


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
int dm_set_remote_ap_info(hd_remoteap_info *m_remote_info);

/*******************************************************************************
 * Function:
 * int dm_get_wlan_con_mode();
 * Description:
 * get wlan connection mode 0x0204
 * Parameters:
 *    NULL
 * Returns:
 *    0:success,-1:failed
 *******************************************************************************/
int dm_get_wlan_con_mode();
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
int dm_set_wired_con_mode(wired_con_mode_array *m_wired_con_array);
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
int dm_get_wired_con_mode(wired_con_mode_array *m_wired_con_array);
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
int dm_get_wlan_list(ap_list_info_t *m_ap_list);


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
int dm_get_client_status();

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
int dm_set_client_status(int status);

int dm_get_wifi_type(int *p_wifi_type);

int dm_set_wifi_type(int wifi_type);

#ifdef __cplusplus
}
#endif
#endif

