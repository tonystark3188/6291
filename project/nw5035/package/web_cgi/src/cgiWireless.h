#ifndef _CGIWIRELESS_H
#define _CGIWIRELESS_H

#include "iwlib.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h> 

#undef WIFI_DRIVER_WEXT
#define WIFI_DRIVER_NL80211

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


#define is_ascii  0x11
#define is_gb2312 0x22
#define is_utf8   0x33

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


#define encrypt_wpa_tkip "WPA/TKIP"
#define encrypt_wpa_aes "WPA/AES"
#define encrypt_wpa_tkip_aes "WPA/TKIPAES"
#define encrypt_wpa2_aes "WPA2/AES"
#define encrypt_wpa2_tkip "WPA2/TKIP"
#define encrypt_wpa2_tkip_aes "WPA2/TKIPAES"
#define encrypt_wpa2_wpa1_aes "WPA1PSKWPA2/AES"
#define encrypt_wpa2_wpa1_tkip "WPA1PSKWPA2/TKIP"
#define encrypt_wpa2_wpa1_tkip_aes "WPA1PSKWPA2/TKIPAES"


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

extern int cgi_get_channel(char *ifname, char *chstr);
extern  char* xmlEncode(char *string);
extern int is_UTF8_or_gb2312(const char* str,long length);
extern void del_space(unsigned char *tmp_buf,int leng);

#endif //_CGIWIRELESS_H
