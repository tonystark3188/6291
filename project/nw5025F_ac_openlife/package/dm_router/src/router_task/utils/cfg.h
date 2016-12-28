#ifndef _CFG_H
#define _CFG_H


#define Debug_cfg 0
#define ENCODE 0

//if nor flash, #it
#ifdef CONFIG_MTD_NAND
#define NAND_BOOT 1
#endif

#ifdef NAND_BOOT
#define CFG_DEV 		"/dev/mtdblock5"  	//uboot
#define CFG_FLAG_ADDR	0x17ff0				//cfg flag addr
#define CFG_FLAG		"airdisk"
#define CFG_BASE_ADDR	0x18000  			//32KB#define DATA_LEN		64
#else
#define CFG_DEV 		"/dev/mtdblock0"  	//uboot
#define CFG_FLAG_ADDR	0x17ff0				//cfg flag addr
#define CFG_FLAG		"airdisk"
#define CFG_BASE_ADDR	0x18000  			//32KB#define DATA_LEN		64
#endif
#define DATA_LEN 64


#define PARA_SN	"sn"
#define PARA_RECOVERY	"recovery"

//router init status
#define PARA_INIT_STATUS 	"init_status"

//wan connect type
#define PARA_WAN_TYPE		"wan" //return pppoe, dhcp, static
//pppoe mode
#define PARA_PPPOE_USER		"pppoe_user"
#define PARA_PPPOE_PSW      "pppoe_psw"
#define PARA_WAN_MAC      	"wan_mac"
//static ip
#define PARA_STATIC_IP		"static_ip"
#define PARA_STATIC_MASK	"static_mask"
#define PARA_STATIC_GATEWAY	"static_gateway"
//dns
#define PARA_DNS			"dns"
#define PARA_WAN_MOD		"wan_mode"
//router ip
#define PARA_IP				"ip"
#define PARA_IP_MASK		"ip_mask"
#define PARA_DHCP			"dhcp_status"
//ip range
#define PARA_DHCP_START		"dhcp_start"
#define PARA_DHCP_END		"dhcp_end"
#define PARA_fwupgrade		"fwupgrade"

#define PARA_MAC   			"mac"	//eth0    mac 845dd7000001
							//eth1    mac 845dd8000001
							//wlan0   mac 845dd9000001
                            //wlan0-1 mac 845dda000001
                            //wlan0-2 mac 845ddb000001
							//wlan0   mac 845ddc000001
                            //wlan0-1 mac 845ddd000001
                            //wlan0-2 mac 845dde000001
//ssid  main 2.4G, 5G, guest 2.4G 5G
#define PARA_SSID_H_24G		"hssid1"
#define PARA_SSID_H_5G		"hssid2"
#define PARA_SSID_C_24G		"cssid1"
#define PARA_SSID_G_24G		"gssid1"
#define PARA_SSID_G_5G		"gssid2"

//password main 2.4G, 5G, guest 2.4G 5G
#define PARA_PSW_H_24G		"hpsw1"
#define PARA_PSW_H_5G		"hpsw2"
#define PARA_PSW_C_24G		"cpsw1"
#define PARA_PSW_G_24G		"gpsw1"
#define PARA_PSW_G_5G		"gpsw2"


//encryption main 2.4G, 5G, guest 2.4G 5G
#define PARA_ENC_H_24G		"henc1"
#define PARA_ENC_H_5G		"henc2"
#define PARA_ENC_C_24G		"cenc1"
#define PARA_ENC_G_24G		"genc1"
#define PARA_ENC_G_5G		"genc2"

//cipher main 2.4G, 5G, guest 2.4G 5G
#define PARA_CHIPER_H_24G	"hchiper1"
#define PARA_CHIPER_H_5G	"hchiper2"
#define PARA_CHIPER_c_24G	"cchiper2"
#define PARA_CHIPER_G_24G	"gchiper1"
#define PARA_CHIPER_G_5G	"gchiper2"

//  reserve 8*32
//enable or disable, main 2.4G, 5G, guest 2.4G 5G
#define PARA_STATUS_H_24G		"hstatus1"
#define PARA_STATUS_H_5G		"hstatus2"
#define PARA_STATUS_C_24G		"cstatus1"
#define PARA_STATUS_G_24G		"gstatus1"
#define PARA_STATUS_G_5G		"gstatus2"
#define PARA_STATUS_24G			"g24"
#define PARA_STATUS_5G			"g5"

//hidden ssid, main 2.4G, 5G, guest 2.4G 5G
#define PARA_HIDDEN_H_24G	"hhidden1"
#define PARA_HIDDEN_H_5G	"hhidden2"
#define PARA_HIDDEN_C_24G	"chidden1"
#define PARA_HIDDEN_G_24G	"ghidden1"
#define PARA_HIDDEN_G_5G	"ghidden2"

#define PARA_ROUTER_IP			"routerip"
#define PARA_ROUTER_MASK		"routermask"
#define PARA_ROUTER_24G_CHAN	"channel24g"
#define PARA_ROUTER_5G_CHAN		"channel5g"

#define PARA_MAIN_AP_SSID		"mainssid"
#define PARA_MAIN_AP_ENCRY		"mainencry"
#define PARA_MAIN_AP_KEY		"mainkey"
#define PARA_REPEATER_MODE		"repeatermode"
#define PARA_ROUTER_NAME		"routername"
#define PARA_HD_SLEEP			"hdsleep"
#define PARA_BACK_LIST_VALID		"backlist_valid"

#define ADDR_SN				(CFG_BASE_ADDR+DATA_LEN*1)
#define ADDR_RECOVERY		(CFG_BASE_ADDR+DATA_LEN*2)
#define ADDR_INIT_STATUS	(CFG_BASE_ADDR+DATA_LEN*3)
#define ADDR_WAN_TYPE		(CFG_BASE_ADDR+DATA_LEN*4)
#define ADDR_PPPOE_USER		(CFG_BASE_ADDR+DATA_LEN*5)
#define ADDR_PPPOE_PSW		(CFG_BASE_ADDR+DATA_LEN*6)
#define ADDR_STATIC_IP		(CFG_BASE_ADDR+DATA_LEN*7)
#define ADDR_STATIC_MASK	(CFG_BASE_ADDR+DATA_LEN*8)
#define ADDR_DNS			(CFG_BASE_ADDR+DATA_LEN*9)
#define ADDR_IP				(CFG_BASE_ADDR+DATA_LEN*10)
#define ADDR_DHCP			(CFG_BASE_ADDR+DATA_LEN*11)
#define ADDR_DHCP_START		(CFG_BASE_ADDR+DATA_LEN*12)
#define ADDR_DHCP_END		(CFG_BASE_ADDR+DATA_LEN*13)
#define ADDR_MAC			(CFG_BASE_ADDR+DATA_LEN*14)

//reserve 2
#define ADDR_SSID_H_24G		(CFG_BASE_ADDR+DATA_LEN*17)
#define ADDR_SSID_H_5G		(CFG_BASE_ADDR+DATA_LEN*18)
#define ADDR_SSID_G_24G		(CFG_BASE_ADDR+DATA_LEN*19)
#define ADDR_SSID_G_5G		(CFG_BASE_ADDR+DATA_LEN*20)



#define ADDR_PSW_H_24G		(CFG_BASE_ADDR+DATA_LEN*21)
#define ADDR_PSW_H_5G		(CFG_BASE_ADDR+DATA_LEN*22)
#define ADDR_PSW_G_24G		(CFG_BASE_ADDR+DATA_LEN*23)
#define ADDR_PSW_G_5G		(CFG_BASE_ADDR+DATA_LEN*24)

#define ADDR_ENC_H_24G		(CFG_BASE_ADDR+DATA_LEN*25)
#define ADDR_ENC_H_5G		(CFG_BASE_ADDR+DATA_LEN*26)
#define ADDR_ENC_G_24G		(CFG_BASE_ADDR+DATA_LEN*27)
#define ADDR_ENC_G_5G		(CFG_BASE_ADDR+DATA_LEN*28)

#define ADDR_CHIPER_H_24G	(CFG_BASE_ADDR+DATA_LEN*29)
#define ADDR_CHIPER_H_5G	(CFG_BASE_ADDR+DATA_LEN*30)
#define ADDR_CHIPER_G_24G	(CFG_BASE_ADDR+DATA_LEN*31)
#define ADDR_CHIPER_G_5G	(CFG_BASE_ADDR+DATA_LEN*32)

#define ADDR_STATUS_H_24G	(CFG_BASE_ADDR+DATA_LEN*33)
#define ADDR_STATUS_H_5G	(CFG_BASE_ADDR+DATA_LEN*34)
#define ADDR_STATUS_G_24G	(CFG_BASE_ADDR+DATA_LEN*35)
#define ADDR_STATUS_G_5G	(CFG_BASE_ADDR+DATA_LEN*36)

#define ADDR_HIDDEN_H_24G	(CFG_BASE_ADDR+DATA_LEN*37)
#define ADDR_HIDDEN_H_5G	(CFG_BASE_ADDR+DATA_LEN*38)
#define ADDR_HIDDEN_G_24G	(CFG_BASE_ADDR+DATA_LEN*39)
#define ADDR_HIDDEN_G_5G	(CFG_BASE_ADDR+DATA_LEN*40)

#define ADDR_ROUTER_IP			(CFG_BASE_ADDR+DATA_LEN*41)
#define ADDR_ROUTER_MASK		(CFG_BASE_ADDR+DATA_LEN*42)
#define ADDR_ROUTER_24G_CHAN	(CFG_BASE_ADDR+DATA_LEN*43)
#define ADDR_ROUTER_5G_CHAN		(CFG_BASE_ADDR+DATA_LEN*44)

#define ADDR_MAIN_AP_SSID		(CFG_BASE_ADDR+DATA_LEN*45)
#define ADDR_MAIN_AP_ENCRY		(CFG_BASE_ADDR+DATA_LEN*46)
#define ADDR_MAIN_AP_KEY		(CFG_BASE_ADDR+DATA_LEN*47)
#define ADDR_REPEATER_MODE		(CFG_BASE_ADDR+DATA_LEN*48)

#define ADDR_STATUS_24G			(CFG_BASE_ADDR+DATA_LEN*49)
#define ADDR_STATUS_5G			(CFG_BASE_ADDR+DATA_LEN*50)
#define ADDR_STATIC_GATEWAY		(CFG_BASE_ADDR+DATA_LEN*51)
#define ADDR_WAN_MOD			(CFG_BASE_ADDR+DATA_LEN*52)
#define ADDR_fwupgrade			(CFG_BASE_ADDR+DATA_LEN*53)
#define ADDR_SSID_C_24G			(CFG_BASE_ADDR+DATA_LEN*53)
#define ADDR_PSW_C_24G			(CFG_BASE_ADDR+DATA_LEN*54)
#define ADDR_ENC_C_24G			(CFG_BASE_ADDR+DATA_LEN*55)
#define ADDR_STATUS_C_24G		(CFG_BASE_ADDR+DATA_LEN*56)
#define ADDR_HIDDEN_C_24G		(CFG_BASE_ADDR+DATA_LEN*57)
#define ADDR_ROUTER_NAME		(CFG_BASE_ADDR+DATA_LEN*58)
#define ADDR_IP_MASK			(CFG_BASE_ADDR+DATA_LEN*59)
#define ADDR_WAN_MAC			(CFG_BASE_ADDR+DATA_LEN*60)
#define ADDR_HD_SLEEP			(CFG_BASE_ADDR+DATA_LEN*61)
#define ADDR_BACK_LIST_VALID	(CFG_BASE_ADDR+DATA_LEN*62)

typedef struct tconfig{
	char *item;
	unsigned int addr;
	int valid;
}Config;



int config_get(char *item, char *sbuf);
int config_set(char *item, char *sbuf);


#endif

