#!/bin/sh

hasConnect=`iwconfig wlan0 | grep Frequency`
wlan1Cnt=`route | grep wlan0 | wc -l`


if [ "$wlan1Cnt" = "2" ] && [ -n "$hasConnect" ]; then
        touch /tmp/client_is_connected
fi
