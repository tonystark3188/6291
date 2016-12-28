#!/bin/sh

#. /lib/netifd/wpa_setup.sh

lan_ip=$(uci get network.lan.ipaddr)
radio_channel=$(uci get wireless.radio0.channel)
ap_ssid=$(uci get wireless.@wifi-iface[0].ssid)
ap_encrypt=$(uci get wireless.@wifi-iface[0].encryption)
if [ "$ap_encrypt" != "none" ]; then
	ap_key=$(uci get wireless.@wifi-iface[0].key)
fi
#ifconfig wlan0 up
uaputl bss_stop
if [ -n "$ap_encrypt" -a "$ap_encrypt" != "none" ]; then
	uaputl sys_cfg_protocol 32
	uaputl sys_cfg_wpa_passphrase $ap_key
	uaputl sys_cfg_cipher 8 8
fi
uaputl sys_cfg_ssid $ap_ssid
uaputl bss_start
sleep 2
uaputl bss_stop
uaputl bss_start



