#!/bin/sh

. /lib/netifd/wpa_setup.sh

wifi_mode=$(uci get wireless2.@wifi[0].mode)
ch2g=$(uci get wireless2.@wifi[0].ch2g)
ch5g=$(uci get wireless2.@wifi[0].ch5g)
lan_ip=$(uci get network.lan.ipaddr)
ap_ssid=$(uci get wireless.@wifi-iface[0].ssid)
ap_encrypt=$(uci get wireless.@wifi-iface[0].encryption)
if [ "$ap_encrypt" != "none" ]; then
	ap_key=$(uci get wireless.@wifi-iface[0].key)
fi

#update mac add by jjjhhh
if [ "$wifi_mode" = "2g" ]; then
        wan_bssid=$(iwconfig wlan0 | grep 'Access Point'|awk -F ' '  '{print $6}')
        lan_bssid=$(ifconfig wl0.1 | grep 'Link encap' | awk \ '{print $5}')
        if [ -n $wan_bssid ] && [ "$wan_bssid" != "$lan_bssid" ]; then
                uci set wireless.@wifi-iface[1].bssid=$wan_bssid
                uci commit wireless
        fi
fi
ifconfig wlan0 down
ifconfig wlan0 up
if [ "$wifi_mode" = "5g" ]; then
	ap_ssid=${ap_ssid}_5G
fi

if [ "$ap_encrypt" = "none" ]; then
	if [ "$wifi_mode" = "2g" ]; then
		dhd_helper ssid "$ap_ssid" hidden n bgnmode bgn chan $ch2g amode open emode none
	else
		dhd_helper ssid "$ap_ssid" hidden n chan $ch5g amode open emode none
	fi
	
else
	if [ "$wifi_mode" = "2g" ]; then
		dhd_helper ssid "$ap_ssid" bgnmode bgn chan $ch2g amode wpa2psk emode aes key "$ap_key"
	else
		dhd_helper ssid "$ap_ssid" chan $ch5g amode wpa2psk emode aes key "$ap_key"
	fi
fi

if [ "$wifi_mode" = "5g" ]; then
	wl down
	wl chanspec $ch5g/80
	wl up
fi

ifconfig wl0.1 $lan_ip up

killall dnsmasq
dnsmasq -C /etc/dnsmasq/dnsmasq.conf -k &

killall wpa_supplicant
start_wpa_supplicant
killall udhcpc
udhcpc -b -t 0 -i wlan0 -x hostname:"$ap_ssid" -O rootpath -s /etc/udhcpc.script &


