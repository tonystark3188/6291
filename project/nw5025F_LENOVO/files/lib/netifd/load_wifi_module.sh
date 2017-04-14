#!/bin/sh



load_wifi_module() {

	local WIFI_MODULE=$(uci get wireless2.@wifi[0].wifi_module)

	if [ "$WIFI_MODULE" = "AP6255" ]; then
		insmod /lib/modules/3.0.8/bcm/bcmdhd_ap6255.ko
	elif [ "$WIFI_MODULE" = "AP6181" ]; then
		cp /lib/firmware/BCM43362_A0.cal /system/etc/firmware/nvram.txt
		insmod /lib/modules/3.0.8/bcm/bcmdhd_ap6255.ko
	elif [ "$WIFI_MODULE" = "AP6212" ]; then
		insmod /lib/modules/3.0.8/bcm/bcmdhd_ap6212.ko
	elif [ "$WIFI_MODULE" = "MRVL8801" ]; then
		insmod /lib/modules/3.0.8/mrvl/8801/mlan.ko
		insmod /lib/modules/3.0.8/mrvl/8801/sd8801.ko auto_ds=2 ps_mode=2 mac_addr=$(cat /etc/mac.txt)

	fi

}




