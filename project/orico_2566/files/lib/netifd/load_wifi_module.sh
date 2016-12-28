#!/bin/sh



load_wifi_module() {

	local WIFI_MODULE=$(uci get wireless2.@wifi[0].wifi_module)
	insmod /lib/modules/3.0.8/bcm/bcmdhd_ap6255.ko
	return

	if [ "$WIFI_MODULE" = "AP6255" ]; then
		insmod /lib/modules/3.0.8/bcm/bcmdhd_ap6255.ko
	elif [ "$WIFI_MODULE" = "AP6212" ]; then
		insmod /lib/modules/3.0.8/bcm/bcmdhd_ap6255.ko
	fi

}




