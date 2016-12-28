#!/bin/sh

. /lib/netifd/wpa_setup.sh

lan_ip=$(uci get network.lan.ipaddr)
radio_channel=$(uci get wireless.radio0.channel)
ap_ssid=$(uci get wireless.@wifi-iface[0].ssid)
ap_encrypt=$(uci get wireless.@wifi-iface[0].encryption)
if [ "$ap_encrypt" != "none" ]; then
	ap_key=$(uci get wireless.@wifi-iface[0].key)
fi
#ifconfig wlan0 up

#if [ "$ap_encrypt" = "none" ]; then
#	dhd_helper ssid $ap_ssid hidden n bgnmode bgn chan $radio_channel amode open emode none
#else
#	dhd_helper ssid $ap_ssid bgnmode bgn chan $radio_channel amode wpawpa2psk emode tkipaes key $ap_key
#fi
#ifconfig wl0.1 $lan_ip up

#killall dnsmasq
#dnsmasq -C /etc/dnsmasq/dnsmasq.conf -k &

#iptables -t nat -A POSTROUTING -s 192.168.222.0/24 -o wlan0 -j MASQUERADE
#iptables -A FORWARD -s 0/0 -d 0/0 -j ACCEPT

#this function is in /lib/netifd/wpa_setup.sh
killall wpa_supplicant
killall udhcpc
start_wpa_supplicant

udhcpc -b -t 0 -i wlan0 -s /etc/udhcpc.script &


