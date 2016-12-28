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
	/usr/sbin/control_newshair.sh  start&
	sleep 5
fi
sleep 5

done


