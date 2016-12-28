#ifndef __ROUTER_DEFS_H__
#define	__ROUTER_DEFS_H__

/*
define the software platform
*/
#define SUPPORT_OPENWRT_PLATFORM
//#define SUPPORT_LINUX_PLATFORM

/*
define the hardware platform
*/
#if defined(SUPPORT_OPENWRT_PLATFORM)
#undef OPENWRT_MT7628
#define OPENWRT_X1000
#elif defined(SUPPORT_LINUX_PLATFORM)
#define LINUX_X1000
#endif

#if defined(OPENWRT_X1000)
#undef WIFI_DRIVER_WEXT
#define WIFI_DRIVER_NL80211
#endif


/*
define the version info
*/
//Maximum 13 bytes
//#define PRODUCT_MODEL  "nw5025F_ac"
#define FW_FILE "/tmp/fwupgrade"
#define FW_LEN 8060928

//#define FW_1 "1.0.01"
//#define FW_2 "1.0.01.6"

#define MOUNT_PATH  "/tmp/mnt"

#define DM_CYCLE_POWER
#undef DM_CYCLE_DISK

/*
define the communication protocol with mcu
*/
#define MCU_COMMUNICATE_SUPPORT

//MCU communicate with uart or double line argeement
#define MCU_UART			1
#define MCU_DOUBLE_LINE		2

#ifdef MCU_COMMUNICATE_SUPPORT
#define MCU_COMMUNICATE_AGREEMENT	MCU_UART //MCU_DOUBLE_LINE//MCU_NOT
#endif


#ifdef SUPPORT_OPENWRT_PLATFORM
/*
wifi frequnce
*/
#define WIFI_2_4G				"24G"
#define WIFI_5G 				"5G"
#define WIFI_2_4G_5G 			"24G5G"
#define WIFI_DEVICE_24G			"ra0"
#define WIFI_DEVICE_5G			"wlan0"
#define AEMOTE_AP_DEVICE_24G  	"apcli0"

#elif defined(SUPPORT_LINUX_PLATFORM)
/*
ini config file
*/
#define NETCONFIGPATH 			"/etc/config/wireless.ini"
#define NETRADIOSECTION 		"radio0"
#define NETAPSECTION 			"AP"
#define NETCLIENTSECTION 		"CLIENT"

#define USERCONFIGPATH "/factory/userconfig"
#define USERSECTION "factory"
#endif

#define TEMP_BUFFER_SIZE	128

/*
define the router error code
*/
#define ROUTER_OK				  	 0
#define ROUTER_ERRORS_UNKNOW	  	-1
#define ROUTER_ERRORS_UCI         	-2
#define ROUTER_ERRORS_CMD_DATA	    -3
#define ROUTER_ERRORS_SHELL_HANDLE 	-4
#define ROUTER_ERRORS_MCU_IOCTL		-5
#define ROUTER_ERRORS_SOCKET_IOCTL  -6 
#define ROUTER_ERROR_FILE_NOT_EXIST -7
#define ROUTER_ERRORS_INI			-8
#define ROUTER_ERRORS_ADD_NETWORK	-9

#endif
