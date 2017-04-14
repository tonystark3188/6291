#!/bin/sh
. /lib/network/networkmode.sh

switch_network_mode $1

[ "$1" = "1" ] && {
#uci set wireless.@wifi-iface[1].disabled=0
uci set ushare.@ushare[0].interface=br-lan
uci set wireless.@wifi-iface[1].network=wan
}
[ "$1" = "2" ] && {
#uci set wireless.@wifi-iface[1].disabled=1
uci set ushare.@ushare[0].interface=wlan0
uci delete wireless.@wifi-iface[1].network
}
uci commit
#reboot
