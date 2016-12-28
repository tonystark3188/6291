#!/bin/sh
. /lib/netifd/netifd-wireless.sh
. /lib/netifd/hostapd.sh

ifname=wlan0

wpa_supplicant_prepare_conf() {
	local ifname="$1"
	_w_driver="$2"

	_wpa_supplicant_common "$1"

	wpa_supplicant_teardown_interface "$ifname"

}

check_encryption() {
	local encryption=$(uci get wireless.@wifi-iface[1].encryption)
	auth_type1=none
	case "$encryption" in
		*psk*)
			auth_type1=psk
		;;
		*wpa*|*8021x*)
			auth_type1=eap
		;;
		
	esac
}

wpa_supplicant_generate_conf() {
	local ifname="$1"
	_wpa_supplicant_common "$1"
	local ssid=$(uci get wireless.@wifi-iface[1].ssid)
	local key=$(uci get wireless.@wifi-iface[1].key)
	local bssid=$(uci get wireless.@wifi-iface[1].bssid)

	local key_mgmt='NONE'
	local enc_str=
	local network_data=
	local T="	"

	local wpa_key_mgmt="WPA-PSK"
	local scan_ssid="1"
	local freq

	check_encryption

	case "$auth_type1" in
		none) ;;
		wep)
			local wep_keyidx=0
			hostapd_append_wep_key network_data
			append network_data "wep_tx_keyidx=$wep_keyidx" "$N$T"
		;;
		psk)
			local passphrase

			key_mgmt="$wpa_key_mgmt"
			if [ ${#key} -eq 64 ]; then
				passphrase="psk=${key}"
			else
				passphrase="psk=\"${key}\""
			fi
			append network_data "$passphrase" "$N$T"
		;;
	esac

	cat >> "$_config" <<EOF
network={
	scan_ssid=$scan_ssid
	ssid="$ssid"
	bssid=$bssid
	key_mgmt=$key_mgmt
	$network_data
}
EOF
	return 0



}



start_wpa_supplicant() {
	wpa_supplicant_prepare_conf "$ifname" nl80211
	wpa_supplicant_generate_conf "$ifname"
	wpa_supplicant_run "$ifname" ${hostapd_ctrl:+-H $hostapd_ctrl}
}


