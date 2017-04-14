#!/bin/sh
echo $1
if [ "$1" = "ap"  ];then
	killall wpa_supplicant
	killall udhcpc
	ifconfig wlan0 down;ifconfig wlan0 up
	#sleep 5
	#init_network.sh
	2g_5g_switch.sh
	uci set system.@system[0].wifimode=ap
	uci commit
        echo "none" > /sys/class/leds/led\:wifi\:blue/trigger
        echo "default-on" > /sys/class/leds/led\:wifi\:red/trigger

elif [ "$1" = "sta" ];then
	SSID=`cfg get ssid |awk -F "=" '{print $2}'`
	if [ "$SSID" = "" ];then
		echo "no ssid,error."
		exit;
	fi
	PASSWORD=`cfg get password |awk -F "=" '{print $2}'`
	if [ "$PASSWORD" = "" ];then
		echo "no password,error."
		exit;
	fi
	mac=`cfg get mac |awk -F "=" '{print $2}'`
        MAC=$(echo $mac | tr '[a-z]' '[A-Z]')
        #MAC=${mac:0:2}${mac:3:2}${mac:6:2}${mac:9:2}${mac:12:2}${mac:15:2}
        mac1=${MAC:6:6}
	wifimode_btn=`get_wifimode_btn`
	ifconfig wl0.1 down
	ifconfig wlan0 down;
	ifconfig wlan0 up
        if [ "$wifimode_btn" = "2g" ];then
		SSID=LEHE$mac1-2.4G
        else
		ch5g=$(uci get wireless2.@wifi[0].ch5g)
		SSID=LEHE$mac1-5G
		wl down                                                                                
                wl chanspec $ch5g/80                                                                   
                wl up 
        fi	

        sn=`cfg get qqsn |awk -F "=" '{print $2}'`
        SN=$(echo $sn | tr '[a-z]' '[A-Z]')
        sed -e '/sn/d' -i /tmp/state/status
        echo sn=$SN >> /tmp/state/status
        PASSWORD=${SN:8:8}

	killall wpa_supplicant
	sleep 1
	wpa_supplicant -B -ddd -f /tmp/w.txt -P /var/run/wifi-wlan0.pid -D nl80211 -i wlan0 -c /etc/wpa.conf  -N -Dnl80211 -i p2p0 -c /etc/p2p.conf -puse_p2p_group_interface=1
	#./wpa_supplicant -i wlan0 -Dnl80211 -c wpa.conf  -N -i p2p0 -Dnl80211 -c p2p.conf -puse_p2p_group_interface=1
	#./wpa_supplicant -B -dddddd  -P /var/run/wifi-wlan0.pid -D nl80211 -i wlan0 -c wpa.conf  -N -Dnl80211 -i p2p0 -c p2p.conf -puse_p2p_group_interface=1
	#./wpa_cli -ip2p0 p2p_group_add ssid=AP6255 ssid_len=6 passphrase=11111111 passphrase_len=8 

	#./wpa_cli log_level debug
	#wpa_cli -i p2p0 set p2p_ssid_postfix ingenic
	wpa_cli -i p2p0 set device_name DIRECT-$SSID
	#./wpa_cli p2p_group_add

	
	SSID_LEN=`echo $SSID |wc -L`
	PASSWORD_LEN=`echo $PASSWORD |wc -L`
	if [ "$wifimode_btn" = "2g" ];then 	
		wpa_cli -ip2p0 p2p_group_add ssid=$SSID ssid_len=$SSID_LEN passphrase=$PASSWORD passphrase_len=$PASSWORD_LEN freq=2

	else
		wpa_cli -ip2p0 p2p_group_add ssid=$SSID ssid_len=$SSID_LEN passphrase=$PASSWORD passphrase_len=$PASSWORD_LEN freq=5 

	fi
	wpa_cli -ip2p-p2p0-0 wps_pbc 
	#./wpa_cli -iwlan0 scan

	ifconfig p2p-p2p0-0 10.10.10.254

	killall dnsmasq
	mkdir -p /tmp/dnsmasq.d/
	dnsmasq -C /etc/dnsmasq/dnsmasq.conf
	#./wpa_cli -ip2p0 p2p_find
	uci set system.@system[0].wifimode=sta
	uci commit
	echo "default-on" > /sys/class/leds/led\:wifi\:blue/trigger
        echo "none" > /sys/class/leds/led\:wifi\:red/trigger

fi

