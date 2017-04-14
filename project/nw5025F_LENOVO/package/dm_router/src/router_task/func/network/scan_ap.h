#ifndef __SCAN_AP_H__
#define __SCAN_AP_H__
#include "router_defs.h"

#if defined(LINUX_X1000) || defined(OPENWRT_X1000) 

#define IW_SCAN_HACK		0x8000

#define WPA2_FLAG 0x01
#define WPA_FLAG 0x02
#define WPA1X_FLAG 0x04

#define WPA_MIX 0x03

#define ENCRYPT_TKIP 0x01
#define ENCRYPT_CCMP 0x02
#define ENCRYPT_WEP_40 0x04
#define ENCRYPT_WEP_104 0x08
#define ENCRYPT_WRAP 0x10


/****************************** TYPES ******************************/

/*
 * Scan state and meta-information, used to decode events...
 */
typedef struct iwscan_state
{
  /* State */
  int			ap_num;		/* Access Point number 1->N */
  int			val_index;	/* Value in table 0->(N-1) */
} iwscan_state;


int cgi_get_channel(char *ifname, char *chstr);

#if defined(WIFI_DRIVER_NL80211)
int cgi_get_scan_nl80211(char *ifname, ap_list_info_t *p_ap_list);

int cgi_get_scan_nl80211_add_network(char *ifname, char *ssid, ap_info_t *p_ap_info);

#endif

#endif

#endif

