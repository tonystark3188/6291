#!/bin/sh

. /lib/netifd/wpa_setup.sh

WIFI_MODULE=$(uci get wireless2.@wifi[0].wifi_module)

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
	ifconfig wlan0 up
	if [ "$ap_encrypt" = "none" ]; then
		dhd_helper ssid "$ap_ssid" hidden n bgnmode bgn chan $ch2g amode open emode none
	else
		dhd_helper ssid "$ap_ssid" bgnmode bgn chan $ch2g amode wpa2psk emode aes key "$ap_key"
	fi
	
	ifconfig wl0.1 $lan_ip up
	client_if=wlan0
	
	
elif [ "$WIFI_MODULE" = "MRVL8801" ]; then
	if [ "$ap_encrypt" = "none" ]; then
		iwpriv uap0 apcfg "ASCII_CMD=AP_CFG,SSID=$ap_ssid,SEC=open"
	else
		iwpriv uap0 apcfg "ASCII_CMD=AP_CFG,SSID=$ap_ssid,SEC=WPA2-PSK,KEY=$ap_key"
		
	fi
	iwpriv mlan0 regioncode 0xff
	ifconfig uap0 $lan_ip up
	#uaputl sys_cfg_11n 1 0x113C
	iwpriv uap0 start
	iwpriv uap0 bssstart
	client_if=mlan0
fi

echo 1 > /sys/class/leds/wifi\:led/brightness



#iptables -F -t nat
iptables -t nat -A POSTROUTING -s 192.168.222.0/24 -o $client_if -j MASQUERADE
iptables -A FORWARD -s 0/0 -d 0/0 -j ACCEPT

if [ ! -d "/tmp/dnsmasq.d" ]; then
	mkdir /tmp/dnsmasq.d
fi

#check if have the connected hostpot
local scan_status
client_sta=$(uci get wireless.@wifi-iface[1].disabled)
dnsmasq_port=$(uci get dhcp.@dnsmasq[0].port)
if [ "$client_sta" = "0" ]; then
	scan_status=$(auto_connect)
	if [ "$scan_status" != "ok" ]; then
		scan_status=$(auto_connect)
	fi
	if [ "$scan_status" = "ok" ]; then
		if [ "$dnsmasq_port" = "0" ]; then 
			uci set dhcp.@dnsmasq[0].port=53
			uci commit
		fi
		dnsmasq -C /etc/dnsmasq/dnsmasq.conf -p 53 -k &
		start_wpa_supplicant
		udhcpc -b -t 0 -i $client_if -s /etc/udhcpc.script &
	elif [ "$scan_status" = "fail" ]; then
		dnsmasq -C /etc/dnsmasq/dnsmasq.conf -p 53 -k &
		start_wpa_supplicant
		udhcpc -b -t 0 -i $client_if -s /etc/udhcpc.script &
		sleep 10
		hasConnect=$(iwconfig $client_if | grep "Access Point: Not-Associated")
		wlan1Cnt=`ip route | grep $client_if | wc -l`

		if [ "$wlan1Cnt" = "2" ] && [ -z "$hasConnect" ]; then
			echo "client is connected"
		else
			if [ "$dnsmasq_port" = "53" ]; then
				uci set dhcp.@dnsmasq[0].port=0
				uci commit
			fi
			killall dnsmasq
			dnsmasq -C /etc/dnsmasq/dnsmasq.conf -p 0 -k &
			killall wpa_supplicant
			killall udhcpc
		fi
	else 
		dnsmasq -C /etc/dnsmasq/dnsmasq.conf -p 53 -k &
		start_wpa_supplicant
		udhcpc -b -t 0 -i $client_if -s /etc/udhcpc.script &	
	fi
elif [ "$client_sta" = "1" ]; then
	if [ "$dnsmasq_port" = "53" ]; then
		uci set dhcp.@dnsmasq[0].port=0
		uci commit
	fi
	dnsmasq -C /etc/dnsmasq/dnsmasq.conf -p 0 -k &
	#killall wpa_supplicant
	#killall udhcpc
fi

