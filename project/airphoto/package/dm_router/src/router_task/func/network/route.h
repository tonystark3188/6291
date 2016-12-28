#ifndef _ROUTER_SET
#define _ROUTER_SET
#include "hd_wifi.h"



int set_wifi_settings(hd_wifi_info *m_wifi_info);
int get_wifi_settings(hd_wifi_info *m_wifi_info);
int set_remote_ap(hd_remoteap_info *m_remote_info);
int set_to_repeart(hd_remoteap_info *m_remote_info);
int get_repeart_status(hd_remoteap_info *m_remote_info);
int get_remote_ap(hd_remoteap_info *m_remote_info);

#endif

