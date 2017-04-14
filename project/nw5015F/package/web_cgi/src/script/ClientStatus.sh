#!/bin/sh

hasConnect=$(iwconfig mlan0 | grep "Access Point: Not-Associated")
wlan1Cnt=`route -n | grep mlan0 | wc -l`


if [ "$wlan1Cnt" = "2" ] && [ -z "$hasConnect" ]; then
        touch /tmp/client_is_connected
fi
