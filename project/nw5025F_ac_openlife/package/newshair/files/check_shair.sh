#!/bin/sh
sleep 5
while true
do
if [ -e /tmp/stop_newshair ] ;then
sleep 10
rm -f /tmp/stop_newshair
fi
status=`ps -www | grep newshair | grep -v "grep"`
if [ ! -n "$status" ] ;then
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

local wanip=$(ifconfig wlan0 | grep 'inet addr' | awk '{print $2}'|awk -F : '{print $2}')
local lanip=192.168.222.254
local dlnastate=no
if [ "$wanip" != "" ]; then
	statusupnp=`ps -www | grep deviceAdvertised | grep -v "grep" | grep $wanip`
	if [ ! -n "$statusupnp" ] ;then
		adverrestart
		continue
	fi
	statusupnp=`ps -www | grep deviceAdvertised | grep -v "grep" |  wc -l`
	if [ "$statusupnp" = "2" ]; then
		dlnastate=yes
	fi
else
	statusupnp=`ps -www | grep deviceAdvertised | grep -v "grep" |  wc -l`
	if [ "$statusupnp" = "1" ]; then
		dlnastate=yes
	fi
fi
if [ "$dlnastate" = "no" ]; then
	adverrestart
fi

sleep 5

done


