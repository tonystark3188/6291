#!/bin/sh

. /lib/netifd/wpa_setup.sh
mode=$1

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
	ifconfig wlan0 down
	sleep 3
	ifconfig wlan0 up
	if [ "$wifi_mode" = "5g" ]; then
		ap_ssid=${ap_ssid}_5G
	fi

	if [ "$ap_encrypt" = "none" ]; then
		if [ "$wifi_mode" = "2g" ]; then
			dhd_helper ssid $ap_ssid hidden n bgnmode bgn chan $ch2g amode open emode none
		else
			dhd_helper ssid $ap_ssid hidden n chan $ch5g amode open emode none
		fi
		
	else
		if [ "$wifi_mode" = "2g" ]; then
			dhd_helper ssid $ap_ssid bgnmode bgn chan $ch2g amode wpa2psk emode aes key $ap_key
		else
			dhd_helper ssid $ap_ssid chan $ch5g amode wpa2psk emode aes key $ap_key
		fi
	fi
	if [ "$wifi_mode" = "5g" ]; then
		wl down
		wl chanspec $ch5g/80
		wl up
	fi

	ifconfig wl0.1 $lan_ip up
}

ip route del default dev wlan0
local scan_status
client_sta=$(uci get wireless.@wifi-iface[1].disabled)
dnsmasq_port=$(uci get dhcp.@dnsmasq[0].port)
if [ "$client_sta" = "0" ]; then
	if [ "$mode" = "auto" ]; then
		scan_status=$(auto_connect)
		if [ "$scan_status" != "ok" ]; then
			scan_status=$(auto_connect)
		fi
		if [ "$scan_status" = "ok" ]; then
			if [ "$dnsmasq_port" = "53" ]; then
				killall wpa_supplicant
				start_wpa_supplicant
				killall udhcpc
				udhcpc -b -t 0 -i wlan0 -s /etc/udhcpc.script &
			elif [ "$dnsmasq_port" = "0" ]; then 
				uci set dhcp.@dnsmasq[0].port=53
				uci commit
				killall dnsmasq
				dnsmasq -C /etc/dnsmasq/dnsmasq.conf -p 53 -k &
				wifi_start
				killall wpa_supplicant
				start_wpa_supplicant
				killall udhcpc
				udhcpc -b -t 0 -i wlan0 -s /etc/udhcpc.script &
			fi
		elif [ "$scan_status" = "fail" ]; then
			if [ "$dnsmasq_port" = "53" ]; then
				uci set dhcp.@dnsmasq[0].port=0
				uci commit
				killall dnsmasq
				dnsmasq -C /etc/dnsmasq/dnsmasq.conf -p 0 -k &
				wifi_start
			fi
			killall wpa_supplicant
			killall udhcpc
		fi
	else
		if [ "$dnsmasq_port" = "53" ]; then
			killall wpa_supplicant
			start_wpa_supplicant
			killall udhcpc
			udhcpc -b -t 0 -i wlan0 -s /etc/udhcpc.script &
		elif [ "$dnsmasq_port" = "0" ]; then 
			uci set dhcp.@dnsmasq[0].port=53
			uci commit
			killall dnsmasq
			dnsmasq -C /etc/dnsmasq/dnsmasq.conf -p 53 -k &
			wifi_start
			killall wpa_supplicant
			start_wpa_supplicant
			killall udhcpc
			udhcpc -b -t 0 -i wlan0 -s /etc/udhcpc.script &
		fi
	fi
elif [ "$client_sta" = "1" ]; then
	if [ "$dnsmasq_port" = "53" ]; then
		uci set dhcp.@dnsmasq[0].port=0
		uci commit
		killall dnsmasq
		dnsmasq -C /etc/dnsmasq/dnsmasq.conf -p 0 -k &
		wifi_start
	fi
	killall wpa_supplicant
	killall udhcpc
fi

control_newshair.sh start
