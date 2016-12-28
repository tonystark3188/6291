#ifndef __IW_NL80211_H
#define __IW_NL80211_H

int get_scan_list_nl80211_add_network(char *ifname, char *ssid, ap_list_info_t *p_ap_list);

int get_scan_list_nl80211(char *ifname, ap_list_info_t *p_ap_list);

#endif 
