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

#endif //_CGIWIRELESS_H
