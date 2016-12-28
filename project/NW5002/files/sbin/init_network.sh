#!/bin/sh

. /lib/netifd/wpa_setup.sh

lan_ip=$(uci get network.lan.ipaddr)
radio_channel=$(uci get wireless.radio0.channel)
ap_ssid=$(uci get wireless.@wifi-iface[0].ssid)
ap_encrypt=$(uci get wireless.@wifi-iface[0].encryption)
channel=$(uci get wireless.radio0.channel)
if [ "$channel" -ge 1 -a "$channel" -le 13 ]; then
	uaputl sys_cfg_channel $channel
fi
mac=`cfg get mac | awk -F = '{print $2}'`
if [ -n $mac ];then
	mac_addr="${mac:0:2}:${mac:2:2}:${mac:4:2}:${mac:6:2}:${mac:8:2}:${mac:10:2}"
else
	mac_addr="84:5d:d7:aa:bb:cc"
fi
uaputl sys_cfg_ap_mac_address $mac_addr
if [ "$ap_encrypt" != "none" ]; then
	ap_key=$(uci get wireless.@wifi-iface[0].key)
fi

if [ -n "$ap_encrypt" -a "$ap_encrypt" != "none" ]; then
	uaputl sys_cfg_protocol 32
	uaputl sys_cfg_wpa_passphrase $ap_key
	uaputl sys_cfg_cipher 8 8
fi
uaputl sys_cfg_ssid $ap_ssid

iwpriv uap0 bssstart
ifconfig uap0 $lan_ip up

killall dnsmasq
dnsmasq -C /etc/dnsmasq/dnsmasq.conf -k &
local IP_HEADER=$(uci get network.lan.ipaddr| awk -F "." '{printf"%d.%d.%d",$1,$2,$3}')
if [ -n "$IP_HEADER" ]; then
	iptables -t nat -A POSTROUTING -s $IP_HEADER.0/24 -o mlan0 -j MASQUERADE
else
	iptables -t nat -A POSTROUTING -s 192.168.222.0/24 -o mlan0 -j MASQUERADE
fi
iptables -A FORWARD -s 0/0 -d 0/0 -j ACCEPT

#this function is in /lib/netifd/wpa_setup.sh
start_wpa_supplicant

udhcpc -b -t 0 -i mlan0 -s /etc/udhcpc.script &


