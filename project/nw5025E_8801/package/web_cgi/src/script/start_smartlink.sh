#!/bin/sh
local status=`ps | grep smartlink | grep -v grep | grep -v start_smartlink`
if [ -z "$status" ]; then
    echo "start smartlink"  >/dev/console
    smartlink &
    exit
fi
echo "smartlink is running"  >/dev/console
exit
