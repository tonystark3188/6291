#!/bin/sh

. /lib/netifd/wpa_setup.sh
mode=$1

WIFI_MODULE=$(uci get wireless2.@wifi[0].wifi_module)
client_if=wlan0

if [ "$WIFI_MODULE" = "AP6212" ]; then
	client_if=wlan0
elif [ "$WIFI_MODULE" = "MRVL8801" ]; then	
	client_if=mlan0
fi
	

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
	
	
	if [ "$WIFI_MODULE" = "AP6212" ]; then
		ifconfig wlan0 down
		sleep 3
		ifconfig wlan0 up
		if [ "$ap_encrypt" = "none" ]; then
			dhd_helper ssid "$ap_ssid" hidden n bgnmode bgn chan $ch2g amode open emode none
		else
			dhd_helper ssid "$ap_ssid" bgnmode bgn chan $ch2g amode wpa2psk emode aes key "$ap_key"
		fi
		
		ifconfig wl0.1 $lan_ip up
		
		
	
	elif [ "$WIFI_MODULE" = "MRVL8801" ]; then
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
		
	fi
	
	
	
}

ip route del default dev $client_if

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
				udhcpc -b -t 0 -i $client_if -s /etc/udhcpc.script &
			elif [ "$dnsmasq_port" = "0" ]; then 
				uci set dhcp.@dnsmasq[0].port=53
				uci commit
				killall dnsmasq
				dnsmasq -C /etc/dnsmasq/dnsmasq.conf -p 53 -k &
				wifi_start
				killall wpa_supplicant
				start_wpa_supplicant
				killall udhcpc
				udhcpc -b -t 0 -i $client_if -s /etc/udhcpc.script &
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
			udhcpc -b -t 0 -i $client_if -s /etc/udhcpc.script &
		elif [ "$dnsmasq_port" = "0" ]; then 
			uci set dhcp.@dnsmasq[0].port=53
			uci commit
			killall dnsmasq
			dnsmasq -C /etc/dnsmasq/dnsmasq.conf -p 53 -k &
			wifi_start
			killall wpa_supplicant
			killall udhcpc
			start_wpa_supplicant
			udhcpc -b -t 0 -i $client_if -s /etc/udhcpc.script &
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


ifconfig $client_if up


control_newshair.sh start
