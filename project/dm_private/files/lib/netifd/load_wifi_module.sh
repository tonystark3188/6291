#!/bin/sh



load_wifi_module() {

	local WIFI_MODULE=$(uci get wireless2.@wifi[0].wifi_module)

	if [ "$WIFI_MODULE" = "AP6255" ]; then
		insmod /lib/modules/3.0.8/bcm/bcmdhd_ap6255.ko
	elif [ "$WIFI_MODULE" = "AP6181" ]; then
		insmod /lib/modules/3.0.8/bcm/bcmdhd_ap6181.ko
	elif [ "$WIFI_MODULE" = "AP6212" ]; then
		insmod /lib/modules/3.0.8/bcm/bcmdhd_ap6212.ko
	fi

}




