#ifndef _NET_CONFIG_H
#define _NET_CONFIG_H
//--------------------------------------------------------------------------------
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>

//cfg set
#define DATA_LEN		32
// #define MAC_ADDR		0x1fc00  			//固定的地址，不可改变
#define SSID_ADDR			CFG_BASE_ADDR
#define ENCRYPTION_ADDR		(CFG_BASE_ADDR+DATA_LEN*1)
#define WIFI0_PASSWORD_ADDR	(CFG_BASE_ADDR+DATA_LEN*2)
#define WIFI1_PASSWORD_ADDR	(CFG_BASE_ADDR+DATA_LEN*3)
#define IP_ADDR				(CFG_BASE_ADDR+DATA_LEN*4)
#define DHCP_START_ADDR 	(CFG_BASE_ADDR+DATA_LEN*5)
#define DHCP_END_ADDR		(CFG_BASE_ADDR+DATA_LEN*6)
#define WPA_CIPHER_ADDR		(CFG_BASE_ADDR+DATA_LEN*7)
#define AIRPLAY_ADDR		(CFG_BASE_ADDR+DATA_LEN*8)
#define CU_ADDR         	(CFG_BASE_ADDR+DATA_LEN*9)
#define sn_ADDR        		(CFG_BASE_ADDR+DATA_LEN*10)
#define WIFICNT_ADDR    	(CFG_BASE_ADDR+DATA_LEN*11)
#define R_PASSWORD_ADDR    	(CFG_BASE_ADDR+DATA_LEN*12)

#define sleep_status_get  12		//sleep 6 wakeup 9
#define CLOSE_CPU_POWER	13
#define GET_SUNSHINE_STATUS	14	//fan close 6 open 9

#define SET_PCIE_RESET	16
#define SET_BT_RESET	17
#define SET_HUB_PWR		18
#define R_315_CTR		19
#define G3_PWR_CTR		20
#define SD_PWR_CTR		21
#define G3_RESET		22
#define LED_CTR			23
#define	SATA_CTR		24

#define M315_PWR		25
#define PWM_OUT			26
#define LED_LEVEL		27
#define LED_RED			28
#define LED_YELLO		29
#define LED_BLUE		30
#define BLUE_WAKEUP		31

//mac ssid encryption password ip dhcp_start dhcp_end wpa_cipher
#define PARA_MAC				"mac"
#define PARA_SSID 				"ssid"
#define PARA_ENCRYPTION			"encryption"
#define PARA_W0_PASSWORD		"wifi0_password"
#define PARA_W1_PASSWORD		"wifi1_password"
#define PARA_IP					"ip"
#define PARA_DHCP_START			"dhcp_start"
#define PARA_DHCP_END			"dhcp_end"
#define PARA_WPA_CIPHER			"wpa_cipher"
#define AIRPLAY_NAME 			"imovename"
#define PARA_CU         		"cu"
#define PARA_SN         		"sn"
#define PARA_WIFICNT    		"wificnt"
#define PARA_ROUTE_PASSWORD    "r_password"

//-------------------------------------------------------------------------------

//wifi hot status
#define WIFI_ON	"on"
#define WIFI_OFF "off"
#define SERVICE_START	"start"
#define SERVICE_RESTART	"restart"
#define SERVICE_STOP	"stop"
//-------------------------------------------------------------------------------
//wifi frequnce
#define WIFI_2_4G	"24G"
#define WIFI_5G "5G"
#define WIFI_2_4G_5G "24G5G"

#define WIFI_DEVICE_24G		"ra0"
#define WIFI_DEVICE_5G		"wlan0"

#define AEMOTE_AP_DEVICE_24G  "apcli0"

//-------------------------------------------------------------------------------
//wifi mode
#define TEMP_BUFFER_SIZE	128


//router error
#define ROUTER_OK				  	 0
#define ROUTER_ERRORS_UNKNOW	  	-1
#define ROUTER_ERRORS_UCI         	-2
#define ROUTER_ERRORS_CMD_DATA	    -3
#define ROUTER_ERRORS_SHELL_HANDLE 	-4
#define ROUTER_ERRORS_MCU_IOCTL		-5
#define ROUTER_ERRORS_SOCKET_IOCTL  -6 


//int cfg_get(char *cfg_set);
//int cfg_set(char *cfg_set);

#endif

