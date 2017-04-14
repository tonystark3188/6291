#ifndef _AUTO_CONNECT_H
#define _AUTO_CONNECT_H

#define MAX_AP_NUM 50


// #define DEBUG
#ifdef DEBUG
#define d_printf(format, ...) \
	do { \
		printf(format,##__VA_ARGS__ ); \
		} while (0)
#else
#define d_printf(format, ...) 
#endif


typedef struct _scan_list
{
	int rssi;
	int channel;
	char ssid[64];
	char mac[32];
}scan_list_t, *pscan_list_t;

typedef struct _wifilist
{
	char ssid[64];
	char encryption[32];
	char key[32];
	char mac[32];
}wifilist_t, *pwifilist_t;


enum wifi_mode
{
	M_2G,
	M_5G
};




#endif   //_AUTO_CONNECT_H