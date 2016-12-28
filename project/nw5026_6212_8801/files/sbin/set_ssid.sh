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


if [ "$wifi_mode" = "5g" ]; then
	ap_ssid=${ap_ssid}_5G
fi

if [ "$WIFI_MODULE" = "AP6212" ]; then
	ifconfig wlan0 down
	ifconfig wlan0 up
	if [ "$ap_encrypt" = "none" ]; then
		dhd_helper ssid "$ap_ssid" hidden n bgnmode bgn chan $ch2g amode open emode none
	else
		dhd_helper ssid "$ap_ssid" bgnmode bgn chan $ch2g amode wpa2psk emode aes key "$ap_key"
	fi
	
	ifconfig wl0.1 $lan_ip up
	client_if=wlan0
	
	
elif [ "$WIFI_MODULE" = "MRVL8801" ]; then
	iwpriv uap0 stop
	if [ "$ap_encrypt" = "none" ]; then
		iwpriv uap0 apcfg "ASCII_CMD=AP_CFG,SSID=$ap_ssid,SEC=open"
	else
		iwpriv uap0 apcfg "ASCII_CMD=AP_CFG,SSID=$ap_ssid,SEC=WPA2-PSK,KEY=$ap_key"
		
	fi
	ifconfig uap0 $lan_ip up
	#uaputl sys_cfg_11n 1 0x113C
	iwpriv uap0 start
	iwpriv uap0 bssstart
	client_if=mlan0
fi

killall dnsmasq
dnsmasq -C /etc/dnsmasq/dnsmasq.conf -k &

killall wpa_supplicant
start_wpa_supplicant
killall udhcpc
udhcpc -b -t 0 -i $client_if -s /etc/udhcpc.script &


