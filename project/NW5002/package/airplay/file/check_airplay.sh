#!/bin/sh
sleep 5
while true
do
if [ -e /tmp/stop_airplay ] ;then
sleep 10
rm -f /tmp/stop_airplay
fi
status=`ps -www | grep airplay | grep -v "grep"`
if [ ! -n "$status" ] ;then
	killall avahi-publish-service
	sleep 1
	/usr/sbin/control_airplay.sh  start&
	sleep 5
fi
sleep 5

done


