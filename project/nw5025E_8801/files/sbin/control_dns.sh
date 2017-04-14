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
		iwpriv uap0 stop
		sleep 3
		if [ "$ap_encrypt" = "none" ]; then
			iwpriv uap0 apcfg "ASCII_CMD=AP_CFG,SSID=$ap_ssid,SEC=open"
		else
			iwpriv uap0 apcfg "ASCII_CMD=AP_CFG,SSID=$ap_ssid,SEC=WPA2-PSK,KEY=$ap_key"
			
		fi
		ifconfig uap0 $lan_ip up
		#uaputl sys_cfg_11n 1 0x113C
		iwpriv uap0 start
		iwpriv uap0 bssstart
		mlanutl mlan0 countrycode CN
}

ip route del default dev mlan0
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
				udhcpc -b -t 0 -i mlan0 -s /etc/udhcpc.script &
			elif [ "$dnsmasq_port" = "0" ]; then 
				uci set dhcp.@dnsmasq[0].port=53
				uci commit
				killall dnsmasq
				dnsmasq -C /etc/dnsmasq/dnsmasq.conf -p 53 -k &
				wifi_start
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
				wifi_start
			fi
			killall wpa_supplicant
			killall udhcpc
		fi
	else
		if [ "$dnsmasq_port" = "53" ]; then
#			wifi_start
			killall wpa_supplicant
			killall udhcpc
			start_wpa_supplicant
			udhcpc -b -t 0 -i mlan0 -s /etc/udhcpc.script &
		elif [ "$dnsmasq_port" = "0" ]; then 
			uci set dhcp.@dnsmasq[0].port=53
			uci commit
			killall dnsmasq
			dnsmasq -C /etc/dnsmasq/dnsmasq.conf -p 53 -k &
			wifi_start
			killall wpa_supplicant
			killall udhcpc
			start_wpa_supplicant
			udhcpc -b -t 0 -i mlan0 -s /etc/udhcpc.script &
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


ifconfig mlan0 up


control_newshair.sh start
