#!/bin/sh

hasConnect=`iwconfig mlan0 | grep Frequency`
wlan1Cnt=`route | grep mlan0 | wc -l`


if [ "$wlan1Cnt" = "2" ] && [ -n "$hasConnect" ]; then
        touch /tmp/client_is_connected
fi
