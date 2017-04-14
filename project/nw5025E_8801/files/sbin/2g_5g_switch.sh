#!/bin/sh

. /lib/netifd/wpa_setup.sh

radio_band=$(uci get wireless2.@wifi[0].radio_band)
[ "$radio_band" != "double"] && exit
wifi_start() {
	wifi_mode=$(uci get wireless2.@wifi[0].mode)
	ch2g=$(uci get wireless2.@wifi[0].ch2g)
	ch5g=$(uci get wireless2.@wifi[0].ch5g)
	lan_ip=$(uci get network.lan.ipaddr)
	ap_ssid=$(uci get wireless.@wifi-iface[0].ssid)
	ap_encrypt=$(uci get wireless.@wifi-iface[0].encryption)
	if [ "$ap_encrypt" != "none" ]; then
		ap_key=$(uci get wireless.@wifi-iface[0].key)
	fi
	ifconfig mlan0 down
	ifconfig mlan0 up
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
}

wifi_mode=$(uci get wireless2.@wifi[0].mode)

if [ "$wifi_mode" = "2g" ]; then
	uci set wireless2.@wifi[0].mode=5g
else
	uci set wireless2.@wifi[0].mode=2g
fi

uci set wireless.@wifi-iface[1].ssid=disable
uci set wireless.@wifi-iface[1].bssid=00:11:22:33:44:55
uci set wireless.@wifi-iface[1].encryption=none

uci commit
sync
usleep 10000

wifi_start
wifi_mode=$(uci get wireless2.@wifi[0].mode)
nor set wifi_mode=$wifi_mode
set_wifimode_to_boot $wifi_mode

if [ "$wifi_mode" = "2g" ]; then
	echo none > /sys/class/leds/longsys\:green\:led/trigger
	echo netdev > /sys/class/leds/longsys\:blue\:led/trigger
	echo wl0.1 > /sys/class/leds/longsys\:blue\:led/device_name
	echo link tx rx > /sys/class/leds/longsys\:blue\:led/mode

else
	
	echo none > /sys/class/leds/longsys\:blue\:led/trigger
	echo netdev > /sys/class/leds/longsys\:green\:led/trigger
	echo wl0.1 > /sys/class/leds/longsys\:green\:led/device_name
	echo link tx rx > /sys/class/leds/longsys\:green\:led/mode
fi

local scan_status
client_sta=$(uci get wireless.@wifi-iface[1].disabled)
dnsmasq_port=$(uci get dhcp.@dnsmasq[0].port)
if [ "$client_sta" = "0" ]; then
	scan_status=$(auto_connect)
	if [ "$scan_status" != "ok" ]; then
		scan_status=$(auto_connect)
	fi
	if [ "$scan_status" != "ok" ]; then
		scan_status=$(auto_connect)
	fi
	if [ "$scan_status" = "ok" ]; then
		if [ "$dnsmasq_port" = "53" ]; then
			killall wpa_supplicant
			start_wpa_supplicant
			killall udhcpc
			udhcpc -b -t 0 -i mlan0 -s /etc/udhcpc.script &
		elif [ "$dnsmasq_port" = "0" ]; then 
			uci set dhcp.@dnsmasq[0].port=53
			uci commit
			killall dnsmasq
			dnsmasq -C /etc/dnsmasq/dnsmasq.conf -p 53 -k &
			# wifi_start
			killall wpa_supplicant
			start_wpa_supplicant
			killall udhcpc
			udhcpc -b -t 0 -i mlan0 -s /etc/udhcpc.script &
		fi
	elif [ "$scan_status" = "fail" ]; then
		if [ "$dnsmasq_port" = "53" ]; then
			uci set dhcp.@dnsmasq[0].port=0
			uci commit
			killall dnsmasq
			dnsmasq -C /etc/dnsmasq/dnsmasq.conf -p 0 -k &
			# wifi_start
		fi
		killall wpa_supplicant
		killall udhcpc
	fi
elif [ "$client_sta" = "1" ]; then
	if [ "$dnsmasq_port" = "53" ]; then
		uci set dhcp.@dnsmasq[0].port=0
		uci commit
		killall dnsmasq
		dnsmasq -C /etc/dnsmasq/dnsmasq.conf -p 0 -k &
		# wifi_start
	fi
	killall wpa_supplicant
	killall udhcpc
fi

control_newshair.sh start
