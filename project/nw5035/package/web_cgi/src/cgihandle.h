#ifndef CGIHANDLE_H
#define CGIHANDLE_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <mntent.h>
#include <sys/vfs.h>
#include <dirent.h>
//#define XML2
#include <sys/stat.h>
#include <libudev.h>
#include <locale.h>
#ifdef XML2
#include <libxml/xmlmemory.h> 
#include <libxml/parser.h>
#else
typedef char* xmlNodePtr;
typedef char xmlChar;
#endif

#include "uci_for_cgi.h"

extern int error_num;
extern char error_info[1024];

#define GETSYSSTR "getSysInfo"
#define SETSYSSTR "setSysInfo"


#define ERROR_SSID "ssid error! "
#define ERROR_PASSWORD "password error! "
#define PARAMETER_ERROR	"parameter error! "

#define FW_1 "1.0.1"
#define FW_2 "1.2.04"
#define FW_3 "1.2.04.16"


//function str

//get function
#define FN_GET_SSID "SSID"
	//
	#define SSID_GET_name "name"
	#define SSID_GET_encrypt "encrypt"
	#define SSID_GET_channel "channel"
	#define SSID_GET_password "password"
	#define SSID_GET_encrypt_len "encrypt_len"
	#define SSID_GET_format "format"
	#define SSID_GET_mac "mac"
	
#define FN_GET_RemoteAP "RemoteAP"
	#define RemoteAP_GET_name "name"
	#define RemoteAP_GET_encrypt "encrypt"
	#define RemoteAP_GET_channel "channel"
	#define RemoteAP_GET_password "password"
	#define RemoteAP_GET_status "status"

	
#define FN_GET_WorkMode "WorkMode"
	#define WorkMode_GET_value "value"
	
#define FN_GET_APList "APList"
	#define APList_GET_AP "AP"

	#define APList_GET_AP_name "name"
	#define APList_GET_AP_encrypt "encrypt"
	#define APList_GET_AP_channel "channel"
	#define APList_GET_AP_rssi "rssi"
	#define APList_GET_AP_tkip_aes "tkip_aes"
	#define APList_GET_AP_mac "mac"

#define FN_GET_Power "Power"
	#define Power_GET_percent "percent"
	#define Power_GET_status "status"
	
#define FN_GET_Storage "Storage"
	#define Storage_GET_Section "Section"
	
	#define Storage_GET_Section_total "total"
	#define Storage_GET_Section_used  "used"
	#define Storage_GET_Section_free  "free"
	#define Storage_GET_Section_volume "volume"
	#define Storage_GET_Section_fstype "fstype"
	
#define FN_GET_FTP "FTP"
	#define FTP_GET_user "user"
	#define FTP_GET_password "password"
	#define FTP_GET_port "port"
	#define FTP_GET_path "path"
	#define FTP_GET_status "status"
	#define FTP_GET_anonymous_en "anonymous_en"
	
#define FN_GET_SAMBA "SAMBA"

	#define SAMBA_GET_user "user"
	#define SAMBA_GET_password "password"
	#define SAMBA_GET_port "port"
	#define SAMBA_GET_path "path"
	#define SAMBA_GET_status "status"


#define FN_GET_DMS "DMS"
	#define DMS_GET_name "name"
	#define DMS_GET_path "path"
	#define DMS_GET_enable "enable"
	
#define FN_GET_DDNS "DDNS"
	#define DDNS_GET_name "name"
	#define DDNS_GET_domain "domain"
	#define DDNS_GET_user "user"


#define FN_GET_WebDAV "WebDAV"
	#define WebDAV_GET_user "user"
	#define WebDAV_GET_password "password"
	#define WebDAV_GET_port "port"
	#define WebDAV_GET_path "path"
	#define WebDAV_GET_enable "enable"

#define FN_GET_JoinWired "JoinWired"

	
#define FN_GET_Version "Version"

#define FN_GET_Client_Status "Client"
	#define Client_Status_enable "enable"
	
#define FN_GET_3G "G3"

#define FN_GET_BTN_RST "BtnReset"

#define FN_GET_AIRPLAY_NAME "airplay"

//set function

#define FN_SET_SSID "SSID"
	#define SSID_SET_name "name"
	#define SSID_SET_encrypt "encrypt"
	#define SSID_SET_tkip_aes "tkip_aes"
	#define SSID_SET_channel "channel"
	#define SSID_SET_password "password"
	#define SSID_SET_format "format"
	#define SSID_SET_encrypt_len "encrypt_len"
	
#define FN_SET_WorkMode "WorkMode"
	#define WorkMode_SET_value "value"
	
#define FN_SET_JoinWireless "JoinWireless"
	#define JoinWireless_SET_AP "AP"
	#define JoinWireless_SET_AP_name "name"
	#define JoinWireless_SET_AP_encrypt "encrypt"
	#define JoinWireless_SET_AP_channel "channel"
	#define JoinWireless_SET_AP_tkip_aes "tkip_aes"
	#define JoinWireless_SET_AP_password "password"
	#define JoinWireless_SET_AP_mac "mac"
	
#define FN_SET_JoinWired "JoinWired"
	#define JoinWired_SET_PPPOE "PPPOE"
	#define JoinWired_SET_PPPOE_user "user"
	#define JoinWired_SET_PPPOE_password "password"
	#define JoinWired_SET_DHCP "DHCP"
	#define JoinWired_SET_StaticIP "StaticIP"
	#define JoinWired_SET_StaticIP_ip "ip"
	#define JoinWired_SET_StaticIP_mask "mask"
	#define JoinWired_SET_StaticIP_gateway "gateway"
	#define JoinWired_SET_StaticIP_dns_mode "dns_mode"
	#define JoinWired_SET_StaticIP_dns1 "dns1"
	#define JoinWired_SET_StaticIP_dns2 "dns2"
	#define JoinWired_SET_StaticIP_dns3 "dns3"
	
#define FN_SET_FTP "FTP"
	#define FTP_SET_user "user"
	#define FTP_SET_password "password"
	#define FTP_SET_port "port"
	#define FTP_SET_path "path"
	#define FTP_SET_anonymous_en "anonymous_en"
	#define FTP_SET_enable "enable"
	
#define FN_SET_SAMBA "SAMBA"
	#define SAMBA_SET_user "user"
	#define SAMBA_SET_password "password"
	#define SAMBA_SET_port "port"
	#define SAMBA_SET_path "path"
	#define SAMBA_SET_enable "enable"
	#define SAMBA_SET_anonymous_en "anonymous_en"
	
#define FN_SET_DMS "DMS"
	#define DMS_SET_name "name"
	#define DMS_SET_path "path"
	#define DMS_SET_enable "enable"
	
#define FN_SET_DDNS "DDNS"
	#define DDNS_SET_name "name"
	#define DDNS_SET_domain "domain"
	#define DDNS_SET_user "user"
	#define DDNS_SET_password "password"

#define FN_SET_WebDAV "WebDAV"
	#define WebDAV_SET_user "user"
	#define WebDAV_SET_password "password"
	#define WebDAV_SET_port "port"
	#define WebDAV_SET_path "path"
	#define WebDAV_SET_enable "enable"

#define FN_Upgrade "Upgrade"

#define FN_HALT "Halt"

#define FN_Time_Sync "Time"
	#define Time_Sync_value "value"
	#define Time_Sync_zone "zone"
	
#define FN_SET_Client "Client"
	#define SET_Client_enable "enable"

#define FN_SET_3G "G3"
	#define SET_3G_apnmode "apnmode"
	#define SET_3G_apn "apn"
	#define SET_3G_username "username"
	#define SET_3G_password "password"
	#define SET_3G_dialnumber "dialnumber"

#define FN_SET_AIRPLAY_NAME "airplay"
	#define AIRPLAY_SET_name "name"

#define FN_SET_iperf "iperf"
	#define SET_iperf_enable "enable"
	

typedef int (*ROOTFUNC)(xmlNodePtr root);
typedef int (*TAGFUNC)(xmlNodePtr tag, char *retstr);


typedef struct _tag_cgiHandle
{
	char tag[20];
	ROOTFUNC rootfun;
}cgiHandle;

typedef struct _tag_cgitagHandle
{
	char tag[20];
	TAGFUNC tagfun;
}tagHandle;

#endif //CGIHANDLE_H
