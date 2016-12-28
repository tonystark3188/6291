/*************************************************************/ 
/* Copyright (c) 2014, longsys */ 
/* All rights reserved. */ 
/*Project Name: NW2415*/ 
/*Author:		solider*/ 
/*Date:			2014-12-15*/ 
/*Version:		v1.0 */ 
/*Abstract£º    set config parameters*/ 
/*************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "cfg.h"

#define _dprintf printf


Config config[] = {
	{PARA_SN, ADDR_SN, 1},
	{PARA_RECOVERY, ADDR_RECOVERY, 1},
	{PARA_INIT_STATUS, ADDR_INIT_STATUS, 1},
	{PARA_WAN_TYPE, ADDR_WAN_TYPE, 1},
	{PARA_PPPOE_USER, ADDR_PPPOE_USER, 1},
	{PARA_PPPOE_PSW, ADDR_PPPOE_PSW, 1},
	{PARA_STATIC_IP, ADDR_STATIC_IP, 1},
	{PARA_STATIC_MASK, ADDR_STATIC_MASK, 1},
	{PARA_STATIC_GATEWAY, ADDR_STATIC_GATEWAY, 1},
	{PARA_WAN_MOD, ADDR_WAN_MOD, 1},	
	{PARA_DNS, ADDR_DNS, 1},
	{PARA_IP, ADDR_IP, 1},
	{PARA_DHCP, ADDR_DHCP, 1},
	{PARA_DHCP_START, ADDR_DHCP_START, 1},
	{PARA_DHCP_END, ADDR_DHCP_END, 1},
	{PARA_MAC, ADDR_MAC, 1},
	{PARA_SSID_H_24G, ADDR_SSID_H_24G, 1},
	{PARA_SSID_H_5G, ADDR_SSID_H_5G, 1},
	{PARA_SSID_C_24G, ADDR_SSID_C_24G, 1},
	{PARA_SSID_G_24G, ADDR_SSID_G_24G, 1},
	{PARA_SSID_G_5G, ADDR_SSID_G_5G, 1},
	{PARA_PSW_H_24G, ADDR_PSW_H_24G, 1},
	{PARA_PSW_H_5G, ADDR_PSW_H_5G, 1},
	{PARA_PSW_C_24G, ADDR_PSW_C_24G, 1},
	{PARA_PSW_G_24G, ADDR_PSW_G_24G, 1},
	{PARA_PSW_G_5G, ADDR_PSW_G_5G, 1},
	{PARA_ENC_H_24G, ADDR_ENC_H_24G, 1},
	{PARA_ENC_H_5G, ADDR_ENC_H_5G, 1},
	{PARA_ENC_C_24G, ADDR_ENC_C_24G, 1},
	{PARA_ENC_G_24G, ADDR_ENC_G_24G, 1},
	{PARA_ENC_G_5G, ADDR_ENC_G_5G, 1},
	{PARA_CHIPER_H_24G, ADDR_CHIPER_H_24G, 1},
	{PARA_CHIPER_H_5G, ADDR_CHIPER_H_5G, 1},
	{PARA_CHIPER_G_24G, ADDR_CHIPER_G_24G, 1},
	{PARA_CHIPER_G_5G, ADDR_CHIPER_G_5G, 1},
	{PARA_STATUS_H_24G, ADDR_STATUS_H_24G, 1},
	{PARA_STATUS_H_5G, ADDR_STATUS_H_5G, 1},
	{PARA_STATUS_C_24G, ADDR_STATUS_C_24G, 1},
	{PARA_STATUS_G_24G, ADDR_STATUS_G_24G, 1},
	{PARA_STATUS_G_5G, ADDR_STATUS_G_5G, 1},
	{PARA_HIDDEN_H_24G, ADDR_HIDDEN_H_24G, 1},
	{PARA_HIDDEN_H_5G, ADDR_HIDDEN_H_5G, 1},
	{PARA_HIDDEN_C_24G, ADDR_HIDDEN_C_24G, 1},
	{PARA_HIDDEN_G_24G, ADDR_HIDDEN_G_24G, 1},
	{PARA_HIDDEN_G_5G, ADDR_HIDDEN_G_5G, 1},
	{PARA_ROUTER_IP, ADDR_ROUTER_IP, 1},
	{PARA_ROUTER_MASK, ADDR_ROUTER_MASK, 1},
	{PARA_ROUTER_24G_CHAN, ADDR_ROUTER_24G_CHAN, 1},
	{PARA_ROUTER_5G_CHAN, ADDR_ROUTER_5G_CHAN, 1},
	{PARA_MAIN_AP_SSID, ADDR_MAIN_AP_SSID, 1},
	{PARA_MAIN_AP_ENCRY, ADDR_MAIN_AP_ENCRY, 1},
	{PARA_MAIN_AP_KEY, ADDR_MAIN_AP_KEY, 1},
	{PARA_REPEATER_MODE, ADDR_REPEATER_MODE, 1},	
	{PARA_STATUS_24G, ADDR_STATUS_24G, 1},
	{PARA_STATUS_5G, ADDR_STATUS_5G, 1},
	{PARA_fwupgrade, ADDR_fwupgrade, 1},
	{PARA_ROUTER_NAME, ADDR_ROUTER_NAME, 1},
	{PARA_IP_MASK, ADDR_IP_MASK, 1},
	{PARA_HD_SLEEP, ADDR_HD_SLEEP, 1},
	{PARA_BACK_LIST_VALID, ADDR_BACK_LIST_VALID, 1},	
	NULL
};

#define ALL_CONFIG sizeof(config)/sizeof(Config)

/*******************************************************************************
 * Function:
 *    static void encode_string(unsigned char *org_str)
 * Description:
 *    
 * Parameters:
 *    org_str   [IN]
 * Returns:
 *    no 
 *    
 *******************************************************************************/
static void encode_string(unsigned char *org_str)
{
#if ENCODE
	int org_str_len=strlen(org_str);
	int i=0;
	for(i=0;i<org_str_len;i++)
	{
		org_str[i]=org_str[i]^0xfe;
	}
#else
	;
#endif
}

/*******************************************************************************
 * Function:
 *    static void decode_string(unsigned char *en_str)
 * Description:
 *    
 * Parameters:
 *    en_str   [IN]
 * Returns:
 *    no 
 *    
 *******************************************************************************/
static void decode_string(unsigned char *en_str)
{
#if ENCODE
	int en_str_len=strlen(en_str);
	int i=0;
	for(i=0;i<en_str_len;i++)
	{
		en_str[i]=en_str[i]^0xfe;
	}
#else
	;
#endif
}
/*******************************************************************************
 * Function:
 *    static void read_mtd(unsigned char *buffer,FILE *fp,unsigned long offset,unsigned long len)
 * Description:
 *    read setting from mtd
 * Parameters:
 *    buffer   [IN/OUT] buffer for read
 *    fp	   [IN] handle
 *    offset   [IN] address offset
 *    len	   [IN] length to read
 * Returns:
 *    no 
 *    
 *******************************************************************************/
static void read_mtd(unsigned char *buffer,FILE *fp,unsigned long offset,unsigned long len)
{
	fseek(fp,offset,SEEK_SET);
	fread(buffer,len,1,fp);
}

/*******************************************************************************
 * Function:
 *    static void write_mtd(unsigned char *buffer,FILE *fp,unsigned long offset,unsigned long len)
 * Description:
 *    write setting to mtd
 * Parameters:
 *    buffer   [IN] buffer for read
 *    fp	   [IN] handle
 *    offset   [IN] address offset
 *    len	   [IN] length to read
 * Returns:
 *    no 
 *    
 *******************************************************************************/
static void write_mtd(unsigned char *buffer,FILE *fp,unsigned long offset,unsigned long len)
{
	fseek(fp,offset,SEEK_SET);
	fwrite(buffer,len,1,fp);
}

/*******************************************************************************
 * Function:
 *    int check_cfg_flag(int ifprint)
 * Description:
 *    check "airdisk" flag
 * Parameters:
 *    ifprint   [IN] print flag 
 * Returns:
 *    if get flag, return 0, else return -1.  
 *    
 *******************************************************************************/
int check_cfg_flag(int ifprint)
{
	unsigned char cfg_flag[16];
	FILE *fp_mtd;
	fp_mtd=fopen(CFG_DEV,"rb");
	if(fp_mtd==NULL)
	{
		#if Debug_cfg
		printf("open /dev/mtdblock0 failed\n");
		#endif
		return -1;
	}
	memset(cfg_flag,0,sizeof(cfg_flag));
	read_mtd(cfg_flag,fp_mtd,CFG_FLAG_ADDR,strlen(CFG_FLAG));
	fclose(fp_mtd);
	if(!strcmp((char *)cfg_flag,CFG_FLAG))
	{
		if(ifprint!=1)
			printf(CFG_FLAG);
		return 0;

	}
	else
		return -1;

}
/*******************************************************************************
 * Function:
 *    int set_cfg_flag
 * Description:
 *    set "airdisk" flag
 * Parameters:
 *    no
 * Returns:
 *    if set flag, return 0, else return -1.  
 *    
 *******************************************************************************/
int set_cfg_flag()
{
	
	FILE *fp_mtd;
	fp_mtd=fopen(CFG_DEV,"rb+");
	if(fp_mtd==NULL)
	{
		#if Debug_cfg
		printf("open /dev/mtdblock0 failed\n");
		#endif
		return -1;
	}
	write_mtd(CFG_FLAG,fp_mtd,CFG_FLAG_ADDR,strlen(CFG_FLAG));
	fclose(fp_mtd);
	return 0;
}
/*******************************************************************************
 * Function:
 *    int clear_cfg_flag
 * Description:
 *    clear "airdisk" flag
 * Parameters:
 *    no
 * Returns:
 *    if set flag, return 0, else return -1.  
 *    
 *******************************************************************************/
int clear_cfg_flag()
{
	FILE *fp_mtd;
	unsigned char clean_flag[strlen(CFG_FLAG)];
	memset(clean_flag,0xff,strlen(CFG_FLAG));
	fp_mtd=fopen(CFG_DEV,"rb+");
	if(fp_mtd==NULL)
	{
		#if Debug_cfg
		printf("open /dev/mtdblock0 failed\n");
		#endif
		return -1;
	}
	write_mtd(clean_flag,fp_mtd,CFG_FLAG_ADDR,strlen(CFG_FLAG));
	fclose(fp_mtd);
	return 0;
}

/*******************************************************************************
 * Function:
 *    int check_valid_mac(char *mac_addr)
 * Description:
 *    check mac is valid
 * Parameters:
 *    mac_addr	[IN] mac string, example xx:xx:xx:xx:xx:xx
 * Returns:
 *    valid mac return 0;
 *	  error mac return -1.  
 *******************************************************************************/
int check_valid_mac(char *mac_addr)
{
	int i=0, j=0;

	if(mac_addr == NULL)
	{
		printf("check_mac mac_addr == NULL\n");
		return -1;
	}

	if(strlen(mac_addr) != 17)
	{
		printf("the mac address must be 17 charactors, example:xx:xx:xx:xx:xx:xx\n");
		return -1;
	}

	for(i=0;i<17;i+=3)
	{
		if(!isxdigit(mac_addr[i]) || !isxdigit(mac_addr[i+1]))
		{
			printf("wrong charactor %c %c\n", mac_addr[i], mac_addr[i+1]);
			return -1;
		}

		if(i<15 && mac_addr[i+2] != ':')
		{
			printf("wrong charactor : \n");
			return -1;
		}

	}
	return 0;
}

/*******************************************************************************
 * Function:
 *    int check_mac(char *mac_addr, char *new_mac)
 * Description:
 *    check mac is valid
 * Parameters:
 *    mac_addr	[IN] old mac string, example xx:xx:xx:xx:xx:xx
 *	  new_mac	[IN] new mac string, example xxxxxxxxxxxx
 * Returns:
 *    valid mac return 0;
 *	  error mac return -1.  
 *******************************************************************************/
int check_mac(char *mac_addr, char *new_mac)
{
	int i=0, j=0;

	if(mac_addr == NULL || mac_addr == NULL)
	{
		printf("check_mac mac_addr == NULL or mac_addr == NULL\n");
		return -1;
	}

	if(strlen(mac_addr) != 17)
	{
		printf("the mac address must be 17 charactors, example:xx:xx:xx:xx:xx:xx\n");
		return -1;
	}

	for(i=0;i<17;i+=3)
	{
		if(!isxdigit(mac_addr[i]) || !isxdigit(mac_addr[i+1]))
		{
			printf("wrong charactor %c %c\n", mac_addr[i], mac_addr[i+1]);
			return -1;
		}
		new_mac[j++] = mac_addr[i];
		new_mac[j++] = mac_addr[i+1];

		if(i<15 && mac_addr[i+2] != ':')
		{
			printf("wrong charactor : \n");
			return -1;
		}

	}
	return 0;
}
/*******************************************************************************
 * Function:
 *    int config_get(char *item, char *sbuf)
 * Description:
 *    get all config value.
 * Parameters:
 *    item	[IN] all config item.
 *	  sbuf	[OUT] config value.
 * Returns:
 *    0  success
 *	  -1 open mtd error
 *	  -2 param is null
 *******************************************************************************/
int config_get(char *item, char *sbuf)
{
	int i = 0, ret = 0;
	FILE *fp_mtd = NULL;
	unsigned char data_str[DATA_LEN+1]={0};
	unsigned char cfg_flag[DATA_LEN+1];

	if(item == NULL || sbuf == NULL)
	{
		printf("cfg set item == NULL or sbuf == NULL\n");
		ret = -2;
		goto out;
	}

	fp_mtd=fopen(CFG_DEV,"rb+");
	if(fp_mtd == NULL)
	{
		printf("open /dev/mtdblock0 failed\n");
		ret = -1;
		goto out;
	}

	memset(cfg_flag,0,sizeof(cfg_flag));
	read_mtd(cfg_flag, fp_mtd, CFG_FLAG_ADDR, strlen(CFG_FLAG));
	if(strcmp((char *)cfg_flag, CFG_FLAG) != 0)
	{
		printf("cfg_get >> set default setting.\n");
		write_mtd(CFG_FLAG, fp_mtd, CFG_FLAG_ADDR, strlen(CFG_FLAG));
		for(i=0; i<ALL_CONFIG; i++)
		{
			if(config[i].valid > 0)
				write_mtd(data_str, fp_mtd, config[i].addr, sizeof(data_str));
		}
	}

	for(i=0; i<ALL_CONFIG; i++)
	{
		if(!strcmp(item, PARA_MAC))
		{//for mac
			unsigned char mac[6];
			memset(mac, 0, sizeof(mac));
			read_mtd(mac, fp_mtd, ADDR_MAC, sizeof(mac));
			sprintf(sbuf, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
			break;
		}
		else if((config[i].valid > 0) && !strcmp(item, config[i].item))
		{
			read_mtd(data_str, fp_mtd, config[i].addr, DATA_LEN);
			decode_string(data_str);
			strcpy(sbuf, data_str);
			break;
		}
	}

	fclose(fp_mtd);
out:
	return ret;
}

static int string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !_is_hex(tmpBuf[0]) || !_is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}


/*******************************************************************************
 * Function:
 *    int config_set(char *item, char *sbuf)
 * Description:
 *    set all config value.
 * Parameters:
 *    item	[IN] all config item.
 *	  sbuf	[IN] config value.
 * Returns:
 *    0  success
 *	  -1 open mtd error
 *	  -2 param is null
 *	  -3 mac is error
 *******************************************************************************/
int config_set(char *item, char *sbuf)
{
	int ret = 0, i = 0, j, jj = 0;
	FILE *fp_mtd;
	unsigned char data_str[DATA_LEN+1]={0};
	unsigned char cfg_flag[DATA_LEN+1];

	if(item == NULL || sbuf == NULL)
	{
		printf("cfg set item == NULL or sbuf == NULL\n");
		ret = -2;
		goto out;
	}

	fp_mtd=fopen(CFG_DEV,"rb+");
	if(fp_mtd == NULL)
	{
		printf("open /dev/mtdblock0 failed\n");
		ret = -1;
		goto out;
	}

	memset(cfg_flag,0,sizeof(cfg_flag));
	read_mtd(cfg_flag, fp_mtd, CFG_FLAG_ADDR, strlen(CFG_FLAG));
	if(strcmp((char *)cfg_flag, CFG_FLAG) != 0)
	{
		printf("cfg_set >> set default setting.\n");
		write_mtd(CFG_FLAG, fp_mtd, CFG_FLAG_ADDR, strlen(CFG_FLAG));
		for(i=0; i<ALL_CONFIG; i++)
		{
			if(config[i].valid > 0)
				write_mtd(data_str, fp_mtd, config[i].addr, sizeof(data_str));
		}
	}

	for(i=0; i<ALL_CONFIG; i++)
	{
		if(!strcmp(item, PARA_MAC))
		{//for mac
			char smac[32], tmpBuf[4];
			unsigned char mac_temp[12];
			unsigned char mac[6];
			j=0;

			memset(smac, 0, sizeof(smac));
			memset(tmpBuf, 0, sizeof(tmpBuf));
			memset(mac, 0, sizeof(mac));
			if(check_mac(sbuf, smac) == -1)
			{
				ret = -3;
				break;
			}
			jj = 0;
			for(j=0;j<12;j+=2)
			{
				tmpBuf[0] = smac[j];
				tmpBuf[1] = smac[j+1];
				tmpBuf[2] = 0;
				mac[jj++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
			}
			
			write_mtd(mac, fp_mtd, ADDR_MAC, sizeof(mac));

#ifdef CONFIG_MTD_NAND
			if(access("/hw_setting",0) != 0)
			{
				system("mkdir /hw_setting;chmod 777 /hw_setting");
			}
//			system("mount -t jffs2 /dev/mtdblock1 /hw_setting");
			system("mount -t yaffs2 -o tags-ecc-off /dev/mtdblock1 /hw_setting");
#endif
			//eth1 
			sprintf(data_str, "flash set HW_NIC1_ADDR %02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			_dprintf("data_str = %s\n", data_str);
			system(data_str);

			//eth0
			mac[3] += 0x10;
			sprintf(data_str, "flash set HW_NIC0_ADDR %02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			_dprintf("data_str = %s\n", data_str);
			system(data_str);

			//wlan0
			mac[3] += 0x10;
			sprintf(data_str, "flash set HW_WLAN0_WLAN_ADDR %02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			_dprintf("data_str = %s\n", data_str);
			system(data_str);

			//wlan1
			mac[3] += 0x10;
			sprintf(data_str, "flash set HW_WLAN1_WLAN_ADDR %02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			_dprintf("data_str = %s\n", data_str);
			system(data_str);

			//wlan0-1
			mac[3] += 0x10;
			sprintf(data_str, "flash set HW_WLAN0_WLAN_ADDR1 %02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			_dprintf("data_str = %s\n", data_str);
			system(data_str);

			//wlan0-2
			mac[3] += 0x10;
			sprintf(data_str, "flash set HW_WLAN0_WLAN_ADDR2 %02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			_dprintf("data_str = %s\n", data_str);
			system(data_str);

			//wlan1-1
			mac[3] += 0x10;
			sprintf(data_str, "flash set HW_WLAN1_WLAN_ADDR1 %02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			_dprintf("data_str = %s\n", data_str);
			system(data_str);

			//wlan1-2
			mac[3] += 0x10;
			sprintf(data_str, "flash set HW_WLAN1_WLAN_ADDR2 %02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			_dprintf("data_str = %s\n", data_str);
			system(data_str);

#ifdef CONFIG_MTD_NAND
			system("umount /hw_setting");
			system("rm -rf /hw_setting");
#endif
			break;
		}
		else if((config[i].valid > 0) && !strcmp(item, config[i].item))
		{
			strcpy(data_str, sbuf);
			encode_string(data_str);
			write_mtd(data_str, fp_mtd, config[i].addr, strlen(data_str)+1);
			_dprintf("%s = %s\n", item, sbuf);
			break;
		}
	}

	fclose(fp_mtd);
out:
	return ret;
}
