#!/bin/sh
if [ "$1" == "stop" ] ;then
touch /tmp/stop_newshair
killall newshair
killall avahi-publish-service
fi

if [ "$1" == "start" ] ;then
killall newshair
killall avahi-publish-service
sleep 1
wifi_mode=$(uci get wireless2.@wifi[0].mode)
mac=$(cat /etc/mac.txt | awk -F':' '{print $1$2$3$4$5$6}' | tr [a-z] [A-Z])
ssid=`uci get wireless.@wifi-iface[0].ssid`
if [ "$wifi_mode" = "5g" ]; then
	ssid=${ssid}_5G
fi
#port=`netstat -apn | awk '/dm_server/ {print}' | awk '{print $4}' | awk -F':' '{print $2}' | sed -n '1p'`
newshair -m external-avahi -a airdisk -S "$ssid" -M "$mac" -p "5002" -d &
fi



