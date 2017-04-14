#!/bin/sh



factory_set() {

	local MAC_ADDR=`cfg get mac | awk -F = '{print $2}'`
	local IP_ADDR=`cfg get ip | awk -F = '{print $2}'`
	local SSID=`cfg get ssid | awk -F = '{print $2}'`
	local ENCRYPT=`cfg get encryption | awk -F = '{print $2}'`
	local PASSWORD=`cfg get password | awk -F = '{print $2}'`
	local WPA_CIPHER=`cfg get wpa_cipher | awk -F = '{print $2}'`
	local DHCP_START=`cfg get dhcp_start | awk -F = '{print $2}'`
	local DHCP_END=`cfg get dhcp_end | awk -F = '{print $2}'`
	local WIFI_MODULE=`cfg get wifi_module | awk -F = '{print $2}'`
	local RADIO_BAND=`cfg get radio_band | awk -F = '{print $2}'`
	local HOSTNAME=`cfg get host_name | awk -F = '{print $2}'`

	local SMB_USR_NAME=`cfg get smb_usr_name | awk -F = '{print $2}'`
	local SMB_USR_PWD=`cfg get smb_usr_pwd | awk -F = '{print $2}'`
	local SMB_GUEST_OK=`cfg get smb_guest_ok | awk -F = '{print $2}'`
	local SMB_ENABLED=`cfg get smb_enabled | awk -F = '{print $2}'`

	local DMS_NAME=`cfg get dms_name | awk -F = '{print $2}'`
	local DMS_ENABLE=`cfg get dms_enable | awk -F = '{print $2}'`

	local HOSTNAME=`cfg get host_name | awk -F = '{print $2}'`

	[ -n "$WIFI_MODULE" ] && uci set wireless2.@wifi[0].wifi_module=$WIFI_MODULE
	[ -n "$RADIO_BAND" ] && uci set wireless2.@wifi[0].radio_band=$RADIO_BAND

	if [ -n "$HOSTNAME" ]; then
		uci set system.@system[0].hostname=$HOSTNAME
	fi

	var_fw_version=`cfg get fw_version`
	if [ "$var_fw_version" != "" ];then
		cfg set $var_fw_version
	fi
	if [ -n "$MAC_ADDR" ]; then
		local macaddr="${MAC_ADDR:0:2}:${MAC_ADDR:2:2}:${MAC_ADDR:4:2}:${MAC_ADDR:6:2}:${MAC_ADDR:8:2}:${MAC_ADDR:10:2}"
		echo $macaddr >/etc/mac.txt
	fi

	if [ -n "$SSID" ]; then
		uci set wireless.@wifi-iface[0].ssid=$SSID
	fi
	if [ -n "$IP_ADDR" ]; then
		uci set network.lan.ipaddr=$IP_ADDR
	fi
	
	if [ -n "$DHCP_START" ] && [ -n "$DHCP_END" ]; then
		uci set dhcp.lan.start=$DHCP_START
		uci set dhcp.lan.limit=`expr $DHCP_END - $DHCP_START`
		
	fi
	
	
	[ -n "$WPA_CIPHER" ] && {
	if [ "$WPA_CIPHER" = "1" ]; then
		WPA_CIPHER="tkip"
		
	elif [ "$WPA_CIPHER" = "2" ]; then
		WPA_CIPHER="ccmp"
		
	elif [ "$WPA_CIPHER" = "3" ]; then
		WPA_CIPHER="tkip+ccmp"
	fi
	}
	
	[ -n "$ENCRYPT" ] && {
	if [ "$ENCRYPT" = "0" ]; then
		ENCRYPT="none"
		uci set wireless.@wifi-iface[0].encryption=$ENCRYPT
	elif [ "$ENCRYPT" = "1" ]; then
		ENCRYPT="wep"
		uci set wireless.@wifi-iface[0].encryption=$ENCRYPT
	elif [ "$ENCRYPT" = "2" ]; then
		ENCRYPT="psk"
		if [ -n "$WPA_CIPHER" ];then
			uci set wireless.@wifi-iface[0].encryption="$ENCRYPT+$WPA_CIPHER"
		else
			uci set wireless.@wifi-iface[0].encryption="$ENCRYPT+ccmp"
		fi
	elif [ "$ENCRYPT" = "4" ]; then
		ENCRYPT="psk2"
		if [ -n "$WPA_CIPHER" ];then
			uci set wireless.@wifi-iface[0].encryption="$ENCRYPT+$WPA_CIPHER"
		else
			uci set wireless.@wifi-iface[0].encryption="$ENCRYPT+ccmp"
		fi
	elif [ "$ENCRYPT" = "6" ]; then
		ENCRYPT="mixed-psk"
		if [ -n "$WPA_CIPHER" ];then
			uci set wireless.@wifi-iface[0].encryption="$ENCRYPT+$WPA_CIPHER"
		else
			uci set wireless.@wifi-iface[0].encryption="$ENCRYPT+ccmp"
		fi
	fi
	
	}
		
	[ "$ENCRYPT" != "none" ] && uci set wireless.@wifi-iface[0].key=$PASSWORD

	[ -n "$SMB_USR_NAME" -a "$SMB_USR_NAME" != "unknow" ] && uci set samba.@samba[0].user=$SMB_USR_NAME
	[ -n "$SMB_USR_PWD" -a "$SMB_USR_PWD" != "unknow" ] && uci set samba.@samba[0].password=$SMB_USR_PWD

	if [ "$SMB_GUEST_OK" = "yes" ]; then
		uci delete samba.@sambashare[0].users
		uci set samba.@sambashare[0].guest_ok=yes
	elif [ "$SMB_GUEST_OK" = "no" ]; then
		uci set samba.@sambashare[0].users=$SMB_USR_NAME
		uci set samba.@sambashare[0].guest_ok=no
	fi
	[ -n "$SMB_ENABLED" -a "$SMB_ENABLED" != "unknow" ] && uci set samba.@samba[0].enabled=$SMB_ENABLED
	
	[ -n "$DMS_NAME" -a "$DMS_NAME" != "unknow" ] && uci set ushare.@ushare[0].servername=$DMS_NAME
	[ -n "$DMS_ENABLE" -a "$DMS_ENABLE" != "unknow" ] && uci set ushare.@ushare[0].enabled=$DMS_ENABLE

	[ -n "$HOSTNAME" -a "$HOSTNAME" != "unknow" ] && uci set system.@system[0].hostname=$HOSTNAME

	uci set system.@mpset[0].status=1

	uci commit
	
}

user_set() {
	local USER_SMB_PASSWORD=`nor get smb_password`
	local USER_SMB_ENABLE=`nor get smb_enable`
	local USER_SMB_ANONYMOUS_EN=`nor get smb_anonymous_en`

	local USER_SSID=`nor get ssid_name`
	local USER_ENCRYPT=`nor get encryption`
	local USER_PASSWORD=`nor get ssid_password`

	local WIFI_MODE=`nor get wifi_mode`
	local CLIENT_DISABLED=`nor get client_disabled`
	

	[ "$USER_SMB_PASSWORD" != "unknow" ] && uci set samba.@samba[0].password=$USER_SMB_PASSWORD
	[ "$USER_SMB_ENABLE" != "unknow" ] && uci set samba.@samba[0].enabled=$USER_SMB_ENABLE
	[ "$USER_SMB_ANONYMOUS_EN" != "unknow" ] && {
		if [ "$USER_SMB_ANONYMOUS_EN" = "1" ]; then
			uci delete samba.@sambashare[0].users
			uci set samba.@sambashare[0].guest_ok=yes
		else
			uci set samba.@sambashare[0].users=airdisk
			uci set samba.@sambashare[0].guest_ok=no
		fi
	}

	[ "$USER_SSID" != "unknow" ] && uci set wireless.@wifi-iface[0].ssid=$USER_SSID
	[ "$USER_ENCRYPT" != "unknow" ] && uci set wireless.@wifi-iface[0].encryption=$USER_ENCRYPT
	[ "$USER_PASSWORD" != "unknow" ] && uci set wireless.@wifi-iface[0].key=$USER_PASSWORD

	[ "$WIFI_MODE" != "unknow" ] && uci set wireless2.@wifi[0].mode=$WIFI_MODE
	[ "$CLIENT_DISABLED" != "unknow" ] && uci set wireless.@wifi-iface[1].disabled=$CLIENT_DISABLED
	

	[ -f /factory/wifilist ] && cp -f /factory/wifilist /etc/config/wifilist


	uci commit
}


factory_user_set() {
	local mp_flag=`cfg get flag | awk -F = '{print $2}'`
	local MPSet=$(uci get system.@mpset[0].status)
	if [ "$mp_flag" != "1" ]; then
		return 1
	fi
	if [ "$MPSet" = "1" ]; then
		return 1
	fi

	factory_set
	user_set
	usleep 10000

}


