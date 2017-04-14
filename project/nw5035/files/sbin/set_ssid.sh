#!/bin/sh

. /lib/netifd/wpa_setup.sh

#wifi_mode=$(uci get wireless2.@wifi[0].mode)
wifi_mode=`/usr/sbin/get_wifimode_btn`
uci set wireless2.@wifi[0].mode=$wifi_mode
uci commit
ch2g=$(uci get wireless2.@wifi[0].ch2g)
ch5g=$(uci get wireless2.@wifi[0].ch5g)
lan_ip=$(uci get network.lan.ipaddr)
ap_ssid=$(uci get wireless.@wifi-iface[0].ssid)
ap_encrypt=$(uci get wireless.@wifi-iface[0].encryption)
if [ "$ap_encrypt" != "none" ]; then
	ap_key=$(uci get wireless.@wifi-iface[0].key)
fi

ifconfig wlan0 down
ifconfig wlan0 up
if [ "$wifi_mode" = "5g" ]; then
        mac=`cfg get mac |awk -F "=" '{print $2}'`
        MAC=$(echo $mac | tr '[a-z]' '[A-Z]')
        #MAC=${mac:0:2}${mac:3:2}${mac:6:2}${mac:9:2}${mac:12:2}${mac:15:2}
        mac1=${MAC:6:6}
        ap_ssid=LEHE$mac1-5G
else
	mac=`cfg get mac |awk -F "=" '{print $2}'`
        MAC=$(echo $mac | tr '[a-z]' '[A-Z]')
        #MAC=${mac:0:2}${mac:3:2}${mac:6:2}${mac:9:2}${mac:12:2}${mac:15:2}
        mac1=${MAC:6:6}
        ap_ssid=LEHE$mac1-2.4G
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

#killall dnsmasq
#dnsmasq -C /etc/dnsmasq/dnsmasq.conf -k &

killall wpa_supplicant
start_wpa_supplicant
killall udhcpc
udhcpc -b -t 0 -i wlan0 -s /etc/udhcpc.script &


