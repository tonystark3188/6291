#!/bin/sh
if [ "$1" == "stop" ] ;then
touch /tmp/stop_airplay
killall airplay
killall avahi-publish-service
fi

if [ "$1" == "start" ] ;then
killall airplay
killall avahi-publish-service
sleep 1
mac=`cfg get mac`
ssid=`uci get wireless.@wifi-iface[0].ssid`
local PORT=$(uci get shair.@shairname[0].port)
#port=`netstat -apn | awk '/dm_server/ {print}' | awk '{print $4}' | awk -F':' '{print $2}' | sed -n '1p'`
airplay -m external-avahi -a airdisk -S "$ssid" -M "$mac" -p "5002" -d -O "$PORT" &
fi



