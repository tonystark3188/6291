#!/bin/sh

hasConnect=$(iwconfig wlan0 | grep "Access Point: Not-Associated")
wlan1Cnt=`ip route | grep wlan0 | wc -l`


if [ "$wlan1Cnt" = "2" ] && [ -z "$hasConnect" ]; then
        touch /tmp/client_is_connected
fi
