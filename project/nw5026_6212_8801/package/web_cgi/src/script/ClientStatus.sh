#!/bin/sh

WIFI_MODULE=$(uci get wireless2.@wifi[0].wifi_module)

if [ "$WIFI_MODULE" = "AP6212" ]; then
	
	hasConnect=$(iwconfig wlan0 | grep "Access Point: Not-Associated")
	wlan1Cnt=`ip route | grep wlan0 | wc -l`
	
elif [ "$WIFI_MODULE" = "MRVL8801" ]; then
	
	hasConnect=$(iwconfig mlan0 | grep "Access Point: Not-Associated")
	wlan1Cnt=`ip route | grep mlan0 | wc -l`
fi


if [ "$wlan1Cnt" = "2" ] && [ -z "$hasConnect" ]; then
        touch /tmp/client_is_connected
fi
