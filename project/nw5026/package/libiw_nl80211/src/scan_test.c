/*
 * nl80211 userspace tool
 *
 * Copyright 2007, 2008	Johannes Berg <johannes@sipsolutions.net>
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "nl80211.h"
#include "iw.h"
#include "iw_nl80211.h"


int main(int argc, char **argv)
{
	printf("start iw_main\n");
	int i;
	ap_list_info_t dm_ap_list;
	int ret = 0;
	char ifname[10];
	if(argc < 2){
		printf("argument error\n");
		return -1;
	}
	memset(ifname, 0, sizeof(ifname));
	strcpy(ifname, argv[1]);
	
	ret = get_scan_list_nl80211(ifname, &dm_ap_list);
	if(ret){
		printf("get_scan_list_nl80211 error\n");
		return -1;
	}

	printf("scan list: \n");
	for(i = 0; i < dm_ap_list.count; i++){
		printf("ssid: %s\t", dm_ap_list.ap_info[i].ssid);
		printf("mac: %s\t", dm_ap_list.ap_info[i].mac);
		printf("channel: %d\t", dm_ap_list.ap_info[i].channel);
		printf("signal: %d\t", dm_ap_list.ap_info[i].wifi_signal);
		printf("encrypt: %s\t", dm_ap_list.ap_info[i].encrypt);
		printf("tkip_aes: %s\t", dm_ap_list.ap_info[i].tkip_aes);
		printf("\n");
	}
			
	return 0;
}

