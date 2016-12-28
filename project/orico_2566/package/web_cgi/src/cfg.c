/**************************************************************
* cfg用来设置出厂配置，并对参数进行加密
* cfg set mac=845dd7001122
* cfg set ssid=airdisk
* cfg set encryption=none(wep,wpa,wpa2,mixed-wpa)
* cfg set password=00000000
* cfg set ip=192.168.222.254
* cfg set dhcp_start=xxx
* cfg set dhcp_end=xxx
* cfg set wpa_cipher=xxx(tkip,aes,tkip/aes)
* 同时，cfg还可以读取配置信息，
* 读取的格式为cfg get xxx
* 得到的参数会打印出来
* Author: liuxiaolong
* time: 2012-12-6
**************************************************************/
#define Debug_cfg 0
#define DEBUG(x) do{;}while(0)
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>
#include "uci_for_cgi.h"

//#define CFG_FACTORY 		"/dev/mtdblock3"  	//factory所在的分区
//#define FACTORY_MAC_ADDR		0x00004  			//固定的地址，不可改变
//#define FACTORY_ETH_MAC_ADDR		0x00028

#define ENCODE 0

#define CFG_DEV 		"/dev/mtdblock2"  	//config分区
#define CFG_FLAG_ADDR	0				//标志位的地址
#define CFG_FLAG		"airdisk"
#define CFG_BASE_ADDR	32  			//32KB的数据空间可用
#define DATA_LEN		32

#define SSID_ADDR				CFG_BASE_ADDR	
#define ENCRYPTION_ADDR			(CFG_BASE_ADDR+DATA_LEN*1)
#define PASSWORD_ADDR			(CFG_BASE_ADDR+DATA_LEN*2)
#define IP_ADDR					(CFG_BASE_ADDR+DATA_LEN*3)
#define DHCP_START_ADDR 		(CFG_BASE_ADDR+DATA_LEN*4)
#define DHCP_END_ADDR			(CFG_BASE_ADDR+DATA_LEN*5)
#define WPA_CIPHER_ADDR			(CFG_BASE_ADDR+DATA_LEN*6)
#define AIRPLAY_ADDR			(CFG_BASE_ADDR+DATA_LEN*7)
#define MAC_ADDR				(CFG_BASE_ADDR+DATA_LEN*100)
//--------------------add by chengpan-----------------------------
#define FW_VERSION_ADDR  		(CFG_BASE_ADDR+DATA_LEN*8)

#define MODEL_NAME_ADDR  		(CFG_BASE_ADDR+DATA_LEN*9)
#define MODEL_NUMBER_ADDR  		(CFG_BASE_ADDR+DATA_LEN*10)
#define MODEL_DESCRIPTION_ADDR1	(CFG_BASE_ADDR+DATA_LEN*11)
#define MODEL_DESCRIPTION_ADDR2	(CFG_BASE_ADDR+DATA_LEN*12)

#define MANUFACTURER_ADDR	  	(CFG_BASE_ADDR+DATA_LEN*13)
#define MANUFACTURER_URL_ADDR1  (CFG_BASE_ADDR+DATA_LEN*14)
#define MANUFACTURER_URL_ADDR2	(CFG_BASE_ADDR+DATA_LEN*15)

#define SMB_USR_NAMR_ADDR	  	(CFG_BASE_ADDR+DATA_LEN*16)
#define SMB_USR_PWD_ADDR	  	(CFG_BASE_ADDR+DATA_LEN*17)
#define SMB_GUEST_OK_ADDR	  	(CFG_BASE_ADDR+DATA_LEN*18)
#define SMB_ENABLED_ADDR	  	(CFG_BASE_ADDR+DATA_LEN*19)


#define DMS_NAME_ADDR	  		(CFG_BASE_ADDR+DATA_LEN*20)
#define DMS_ENABLE_ADDR	 		(CFG_BASE_ADDR+DATA_LEN*21)
#define HOST_NAME_ADDR     		(CFG_BASE_ADDR+DATA_LEN*22)

#define DLNA_ADDR				(CFG_BASE_ADDR+DATA_LEN*23)

#define VERSION_ADDR			(CFG_BASE_ADDR+DATA_LEN*24)

#define REPAIR_AUTO_ADDR		(CFG_BASE_ADDR+DATA_LEN*25)
//length=16
#define SN_ADDR					(CFG_BASE_ADDR+DATA_LEN*26)
#define SN_LEN					16

//lenght<256
#define LICENSE_ADDR			(CFG_BASE_ADDR+DATA_LEN*27)
#define LICENSE_LEN				256  //32*8

#define ROOT_PWD_ADDR			(CFG_BASE_ADDR+DATA_LEN*35)

//----------------------add end-----------------------------------

//mac ssid encryption password ip dhcp_start dhcp_end wpa_cipher
#define PARA_MAC		"mac"
#define PARA_SSID 		"ssid"
#define PARA_ENCRYPTION	"encryption"
#define PARA_PASSWORD	"password"
#define PARA_IP			"ip"
#define PARA_DHCP_START	"dhcp_start"
#define PARA_DHCP_END	"dhcp_end"
#define PARA_WPA_CIPHER	"wpa_cipher"
#define AIRPLAY_NAME "airplay_name"
#define DLNA_NAME "dlna_name"
//--------------------add by chengpan -------------------------
//----------------------nrender and ushare----------------------
#define FW_VERSION "fw_version"

#define MODEL_NAME "model_name"
#define MODEL_NUMBER "model_number"
#define MODEL_DESCRIPTION "model_description"

#define MANUFACTURER "manufacturer"
#define MANUFACTURER_URL "url_manufacturer"
//-----------------------------samba-----------------------------
#define SMB_USR_NAMR "smb_usr_name"
#define SMB_USR_PWD "smb_usr_pwd"
#define SMB_GUEST_OK "smb_guest_ok"
#define SMB_ENABLED "smb_enabled"

#define DMS_NAME "dms_name"
#define DMS_ENABLE "dms_enable"

#define HOST_NAME "host_name"

#define VERSION_FLAG  "version_flag"

#define REPAIR_AUTO  "repair_auto"
#define SN "qqsn"
#define LICENSE "license"

#define ROOT_PWD	"root_pwd"

static void usage(void)
{
	printf("usage:airdisk config tool\n");
	printf("warning:don't use strings that are too long!!!\n");
	printf("config parameters are:\n\n");
	printf("	mac ssid encryption password ip dhcp_start qqsn license\n");
	printf("	dhcp_end wpa_cipher airplay_name dlna_name fw_version model_name model_number\n");
	printf("	model_description manufacturer url_manufacturer dms_name dms_enable(1/0)\n");
	printf("	smb_usr_name smb_usr_pwd smb_guest_ok(yes/no) smb_enabled(1/0)\n");
	printf("	host_name version_flag repair_auto qqsn license root_pwd\n\n");
	printf("	set 		: cfg set parameter=value \n");
	printf("	get 		: cfg get parameter \n");	
	printf("	****debug version****added by BlackAvengers****\n");	
	exit(1);
}
//对字符串进行加密，每个字符与0xfe进行异或
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

//解密，同样每个字符与0xfe进行异或
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
static void read_mtd(unsigned char *buffer,FILE *fp,unsigned long offset,unsigned long len)
{
	fseek(fp,offset,SEEK_SET);
	fread(buffer,len,1,fp);
}

static void write_mtd(unsigned char *buffer,FILE *fp,unsigned long offset,unsigned long len)
{
	fseek(fp,offset,SEEK_SET);
	fwrite(buffer,len,1,fp);
}

//检查标志位，如果标志位为airdisk，说明配置参数已经被设置，否则
//则没有被设置
static int check_cfg_flag(int ifprint)
{
	unsigned char cfg_flag[16];
	FILE *fp_mtd;
	fp_mtd=fopen(CFG_DEV,"rb");
	if(fp_mtd==NULL)
	{
		exit(1);
	}
	memset(cfg_flag,0,sizeof(cfg_flag));
	read_mtd(cfg_flag,fp_mtd,CFG_FLAG_ADDR,strlen(CFG_FLAG));
	fclose(fp_mtd);
	if(!strcmp((char *)cfg_flag,CFG_FLAG))
	{
		if(ifprint!=1)
			printf(CFG_FLAG);
		return 1;

	}
	else
		return 0;

}
//将标志位设置为airdisk，如果参数被写入，则必须将标志位设为airdisk
static set_cfg_flag()
{
	
	FILE *fp_mtd;
	fp_mtd=fopen(CFG_DEV,"rb+");
	if(fp_mtd==NULL)
	{
		exit(1);
	}
	write_mtd(CFG_FLAG,fp_mtd,CFG_FLAG_ADDR,strlen(CFG_FLAG));
	fclose(fp_mtd);
}
//清除标志位
static clear_cfg_flag()
{
	FILE *fp_mtd;
	unsigned char clean_flag[strlen(CFG_FLAG)];
	memset(clean_flag,0xff,strlen(CFG_FLAG));
	fp_mtd=fopen(CFG_DEV,"rb+");
	if(fp_mtd==NULL)
	{
		exit(1);
	}
	write_mtd(clean_flag,fp_mtd,CFG_FLAG_ADDR,strlen(CFG_FLAG));
	fclose(fp_mtd);
}
//检查mac地址是否合法
static int check_mac(char *mac_addr)
{
	int i=0;
	if(strlen(mac_addr)!=12)
	{
		#if Debug_cfg
		printf("the mac address must be 12 charactors\n");
		#endif
		return 1;
	}
	for(i=0;i<12;i++)
	{
		if(!isxdigit(mac_addr[i]))
		{
			#if Debug_cfg
			printf("wrong charactor\n");
			#endif
			return 1;
		}

	}
	return 0;
}

//获取参数
int cfg_get(char *argv[])
{
	FILE *fp_mtd;
	fp_mtd=fopen(CFG_DEV,"rb");
	if(fp_mtd==NULL)
	{
		#if Debug_cfg
		printf("open /dev/mtdblock0 failed\n");
		#endif
		exit(1);
	}
	if(!strcmp(argv[2],PARA_MAC))
	{
		unsigned char mac[DATA_LEN];  //必须为unsigned char,不能定义为char，mac地址的范围为00-ff
		memset(mac,0,sizeof(mac));
		read_mtd(mac,fp_mtd,MAC_ADDR,6);
		//decode_string(mac);
		if(mac[0] == 0xff && mac[1] == 0xff && mac[2] == 0xff && mac[3] == 0xff && mac[4] == 0xff && mac[5] == 0xff)
		{
			return;
		}
		printf("%s=%02x%02x%02x%02x%02x%02x",PARA_MAC,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
		//printf("%s",(char *)mac);
		printf("\n");
		
	}
	else if(!strcmp(argv[2],PARA_SSID))
	{
		unsigned char ssid[DATA_LEN];
		memset(ssid,0,sizeof(ssid));
		read_mtd(ssid,fp_mtd,SSID_ADDR,DATA_LEN);
		decode_string(ssid);
		if(strstr(ssid,PARA_SSID)!=NULL)
		{
			printf("%s",(char *)ssid);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],PARA_ENCRYPTION))
	{
		unsigned char encryption[DATA_LEN];
		memset(encryption,0,sizeof(encryption));
		read_mtd(encryption,fp_mtd,ENCRYPTION_ADDR,DATA_LEN);
		decode_string(encryption);
		if(strstr(encryption,PARA_ENCRYPTION)!=NULL)
		{
			printf("%s",(char *)encryption);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],PARA_PASSWORD))
	{
		unsigned char password[DATA_LEN];
		memset(password,0,sizeof(password));
		read_mtd(password,fp_mtd,PASSWORD_ADDR,DATA_LEN);
		decode_string(password);
		if(strstr(password,PARA_PASSWORD)!=NULL)
		{
			printf("%s",(char *)password);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],PARA_IP))
	{
		unsigned char ip[DATA_LEN];
		memset(ip,0,sizeof(ip));
		read_mtd(ip,fp_mtd,IP_ADDR,DATA_LEN);
		decode_string(ip);
		if(strstr(ip,PARA_IP)!=NULL)
		{
			printf("%s",(char *)ip);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],PARA_DHCP_START))
	{
		unsigned char dhcp_start[DATA_LEN];
		memset(dhcp_start,0,sizeof(dhcp_start));
		read_mtd(dhcp_start,fp_mtd,DHCP_START_ADDR,DATA_LEN);
		decode_string(dhcp_start);
		if(strstr(dhcp_start,PARA_DHCP_START)!=NULL)
		{
			printf("%s",(char *)dhcp_start);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],PARA_DHCP_END))
	{
		unsigned char dhcp_end[DATA_LEN];
		memset(dhcp_end,0,sizeof(dhcp_end));
		read_mtd(dhcp_end,fp_mtd,DHCP_END_ADDR,DATA_LEN);
		decode_string(dhcp_end);
		if(strstr(dhcp_end,PARA_DHCP_END)!=NULL)
		{
			printf("%s",(char *)dhcp_end);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],PARA_WPA_CIPHER))
	{
		unsigned char wpa_cipher[DATA_LEN];
		memset(wpa_cipher,0,sizeof(wpa_cipher));
		read_mtd(wpa_cipher,fp_mtd,WPA_CIPHER_ADDR,DATA_LEN);
		decode_string(wpa_cipher);
		if(strstr(wpa_cipher,PARA_WPA_CIPHER)!=NULL)
		{
			printf("%s",(char *)wpa_cipher);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],AIRPLAY_NAME))
	{
		unsigned char airplay[DATA_LEN];
		memset(airplay,0,sizeof(airplay));
		read_mtd(airplay,fp_mtd,AIRPLAY_ADDR,DATA_LEN);
		decode_string(airplay);
		if(strstr(airplay,AIRPLAY_NAME)!=NULL)
		{
			printf("%s",(char *)airplay);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],DLNA_NAME))
	{
		unsigned char dlna[DATA_LEN];
		memset(dlna,0,sizeof(dlna));
		read_mtd(dlna,fp_mtd,DLNA_ADDR,DATA_LEN);
		if(strstr(dlna,DLNA_NAME)!=NULL)
		{
			printf("%s",(char *)dlna);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],DMS_NAME))
	{
		unsigned char tmp[DATA_LEN];
		memset(tmp,0,sizeof(tmp));
		read_mtd(tmp,fp_mtd,DMS_NAME_ADDR,DATA_LEN);
		
		if(strstr(tmp,DMS_NAME)!=NULL)
		{
			printf("%s",(char *)tmp);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],DMS_ENABLE))
	{
		unsigned char tmp[DATA_LEN];
		memset(tmp,0,sizeof(tmp));
		read_mtd(tmp,fp_mtd,DMS_ENABLE_ADDR,DATA_LEN);
		
		if(strstr(tmp,DMS_ENABLE)!=NULL)
		{
			printf("%s",(char *)tmp);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],FW_VERSION))
	{
		unsigned char tmp[DATA_LEN];
		memset(tmp,0,sizeof(tmp));
		read_mtd(tmp,fp_mtd,FW_VERSION_ADDR,DATA_LEN);
		DEBUG(printf("FW_VERSION %s\n",(char *)tmp));
		if(strstr(tmp,FW_VERSION)!=NULL)
		{
			printf("%s",(char *)tmp);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],MODEL_NAME))
	{
		unsigned char tmp[DATA_LEN];
		memset(tmp,0,sizeof(tmp));
		read_mtd(tmp,fp_mtd,MODEL_NAME_ADDR,DATA_LEN);
		DEBUG(printf("MODEL_NAME %s\n",(char *)tmp));
		if(strstr(tmp,MODEL_NAME)!=NULL)
		{
			printf("%s",(char *)tmp);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],MODEL_NUMBER))
	{
		unsigned char tmp[DATA_LEN];
		memset(tmp,0,sizeof(tmp));
		read_mtd(tmp,fp_mtd,MODEL_NUMBER_ADDR,DATA_LEN);
		DEBUG(printf("MODEL_NUMBER %s\n",(char *)tmp));
		if(strstr(tmp,MODEL_NUMBER)!=NULL)
		{
			printf("%s",(char *)tmp);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],MODEL_DESCRIPTION))
	{
		unsigned char tmp[64];
		memset(tmp,0,sizeof(tmp));
		read_mtd(tmp,fp_mtd,MODEL_DESCRIPTION_ADDR1,64);
		DEBUG(printf("MODEL_DESCRIPTION %s\n",(char *)tmp));
		if(strstr(tmp,MODEL_DESCRIPTION)!=NULL)
		{
			printf("%s",(char *)tmp);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],MANUFACTURER))
	{
		unsigned char tmp[DATA_LEN];
		memset(tmp,0,sizeof(tmp));
		read_mtd(tmp,fp_mtd,MANUFACTURER_ADDR,DATA_LEN);
		DEBUG(printf("MANUFACTURER %s\n",(char *)tmp));
		if(strstr(tmp,MANUFACTURER)!=NULL)
		{
			printf("%s",(char *)tmp);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],MANUFACTURER_URL))
	{
		unsigned char tmp[64];
		memset(tmp,0,sizeof(tmp));
		read_mtd(tmp,fp_mtd,MANUFACTURER_URL_ADDR1,64);
		if(strstr(tmp,MANUFACTURER_URL)!=NULL)
		{
			printf("%s",(char *)tmp);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],SMB_USR_NAMR))
	{
		unsigned char tmp[32];
		memset(tmp,0,sizeof(tmp));
		read_mtd(tmp,fp_mtd,SMB_USR_NAMR_ADDR,32);
		DEBUG(printf("get %s\n",(char *)tmp));
		if(strstr(tmp,SMB_USR_NAMR)!=NULL)
		{
			printf("%s",(char *)tmp);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],SMB_USR_PWD))
	{
		unsigned char tmp[32];
		memset(tmp,0,sizeof(tmp));
		read_mtd(tmp,fp_mtd,SMB_USR_PWD_ADDR,32);
		DEBUG(printf("get %s\n",(char *)tmp));
		if(strstr(tmp,SMB_USR_PWD)!=NULL)
		{
			printf("%s",(char *)tmp);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],SMB_GUEST_OK))
	{
		unsigned char tmp[32];
		memset(tmp,0,sizeof(tmp));
		read_mtd(tmp,fp_mtd,SMB_GUEST_OK_ADDR,32);
		if(strstr(tmp,SMB_GUEST_OK)!=NULL)
		{
			printf("%s",(char *)tmp);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],SMB_ENABLED))
	{
		unsigned char tmp[32];
		memset(tmp,0,sizeof(tmp));
		read_mtd(tmp,fp_mtd,SMB_ENABLED_ADDR,32);
		DEBUG(printf("get %s\n",(char *)tmp));
		if(strstr(tmp,SMB_ENABLED)!=NULL)
		{
			printf("%s",(char *)tmp);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],HOST_NAME))
	{
		unsigned char tmp[32];
		memset(tmp,0,sizeof(tmp));
		read_mtd(tmp,fp_mtd,HOST_NAME_ADDR,32);
		if(strstr(tmp,HOST_NAME)!=NULL)
		{
			printf("%s",(char *)tmp);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],VERSION_FLAG))
	{
		unsigned char tmp[32];
		memset(tmp,0,sizeof(tmp));
		read_mtd(tmp,fp_mtd,VERSION_ADDR,32);
		if(strstr(tmp,VERSION_FLAG)!=NULL)
		{
			printf("%s",(char *)tmp);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],REPAIR_AUTO))
	{
		unsigned char tmp[32];
		memset(tmp,0,sizeof(tmp));
		read_mtd(tmp,fp_mtd,REPAIR_AUTO_ADDR,32);
		if(strstr(tmp,REPAIR_AUTO)!=NULL)
		{
			printf("%s",(char *)tmp);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],SN))
	{
		unsigned char tmp[32];
		memset(tmp,0,sizeof(tmp));
		read_mtd(tmp,fp_mtd,SN_ADDR,32);
		if(strstr(tmp,SN)!=NULL)
		{
			printf("%s",(char *)tmp);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],LICENSE))
	{
		unsigned char tmp[LICENSE_LEN+8];
		memset(tmp,0,sizeof(tmp));
		read_mtd(tmp,fp_mtd,LICENSE_ADDR,LICENSE_LEN+8);
		if(strstr(tmp,LICENSE)!=NULL)
		{
			printf("%s",(char *)tmp);
			printf("\n");
		}
	}
	else if(!strcmp(argv[2],ROOT_PWD))
	{
		unsigned char tmp[32];
		memset(tmp,0,sizeof(tmp));
		read_mtd(tmp,fp_mtd,ROOT_PWD_ADDR,32);
		if(strstr(tmp,ROOT_PWD)!=NULL)
		{
			printf("%s",(char *)tmp);
			printf("\n");
		}
	}
	else
	{
		#if Debug_cfg
		printf("Invalid parameter!\r\n");
		usage();
		#endif
	}
	
	fclose(fp_mtd);
}

//设置参数
int cfg_set(char *argv[])
{
	FILE *fp_mtd,*fp_factory;
	unsigned char data_str[32]={0};
	char str_sp[64]="\0";
	char uci_option_str[64]="\0";
	memset(data_str,0,sizeof(data_str));
	char set_nrender_str[128]={0};
	char set_smb_str[128]={0};
	char set_nor_str[128]={0};
	char license_str[512]={0};
	fp_mtd=fopen(CFG_DEV,"rb+");
	if(fp_mtd==NULL)
	{
		exit(1);
	}
//----------------------------------------------add by chengpan------check string length-----------------------
	if(strlen(argv[2])>=DATA_LEN && strncmp(MODEL_DESCRIPTION,argv[2],strlen(MODEL_DESCRIPTION)) && strncmp(MANUFACTURER_URL,argv[2],strlen(MANUFACTURER_URL)) && strncmp(LICENSE,argv[2],strlen(LICENSE)))
	{
		#if 1
		printf("string too long %d>31!!!\n",strlen(argv[2]));
		#endif
		exit(1);
	}
	if(strlen(argv[2])>=(DATA_LEN*2)&& strncmp(LICENSE,argv[2],strlen(LICENSE)))
	{
		#if 1
		printf("string too long %d>63!!!\n",strlen(argv[2]));
		#endif
		exit(1);
	}
	if(strlen(argv[2])>(LICENSE_LEN+7))
	{
		if(!strncmp(LICENSE,argv[2],strlen(LICENSE)))
		{	
			printf("license string too long %d>256!!!\n",strlen(argv[2]));
			exit(1);
		}
	}
//------------------------------------------------add end------------------------------------
	if(!strncmp(PARA_MAC,argv[2],strlen(PARA_MAC)))
	{
		int i=0,j=0;
		char mac_input[32]="\0";
		unsigned char mac_temp[12];
		unsigned char mac[10];
		unsigned char eth_mac[20];
		memset(mac,0,sizeof(mac));
		memset(eth_mac,0,sizeof(eth_mac));
		strcpy(mac_input,argv[2]+strlen(PARA_MAC)+1);
		if(check_mac(mac_input))
		{
			fclose(fp_mtd);
			exit(1);
		}
		for(i=0;i<12;i++)
		{
			if(mac_input[i]>='0' && mac_input[i]<='9')
			{
				mac_temp[i]=mac_input[i]-48;
			}
			else if(mac_input[i]>='a' && mac_input[i]<='f')
			{
				mac_temp[i]=mac_input[i]-97+10;
			}
			else if(mac_input[i]>='A' && mac_input[i]<='F')
			{
				mac_temp[i]=mac_input[i]-65+10;
			}
		}
		for(i=0;i<6;i++)
		{
			mac[i]=mac_temp[2*i]*16 + mac_temp[2*i+1];
		}

		for(i=0,j=0;i<12;i++,j++)
		{
			eth_mac[j] = mac_input[i];
			if((i+1)%2 == 0)
			{
				if(i != 11)
				{
					j++;
					eth_mac[j] = ':';
				}
				
			}
		}
		//strcpy(data_str,PARA_MAC);
		//strcat(data_str,"=");
		//strcat(data_str,mac);
		//encode_string(data_str);
		write_mtd(mac,fp_mtd,MAC_ADDR,6);

		memset(str_sp,0,64);
		sprintf(str_sp,"echo \'%s\' >/etc/mac.txt",eth_mac);	
		system(str_sp);
		system("sync");
	//	sprintf(str_sp,"wireless.radio0.macaddr=%02x:%02x:%02x:%02x:%02x:%02x",
	//					mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	//	uci_set_option_value(str_sp);
		memset(str_sp,0,64);
		#if 0
		fp_factory=fopen(CFG_FACTORY,"rb+");
		if(fp_factory==NULL)
		{
			exit(1);
		}
		write_mtd(mac,fp_factory,FACTORY_MAC_ADDR,6);
		write_mtd(mac,fp_factory,FACTORY_ETH_MAC_ADDR,6);
		sprintf(str_sp,"/usr/mips/cgi-bin/script/set_lanwan_mac.sh %s",eth_mac);
		system(str_sp);
		memset(str_sp,0,64);
		fclose(fp_factory);
		#endif
	}
	else if(!strncmp(PARA_SSID,argv[2],strlen(PARA_SSID))) 
	{
		strcpy(data_str,argv[2]);
		encode_string(data_str);
		write_mtd(data_str,fp_mtd,SSID_ADDR,strlen(data_str)+1);
		memset(data_str,0,sizeof(data_str));
		sprintf(str_sp,"wireless.@wifi-iface[0].ssid=%s",argv[2]+strlen(PARA_SSID)+1);
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);
	}
	else if(!strncmp(PARA_ENCRYPTION,argv[2],strlen(PARA_ENCRYPTION)))  
	{
		char str_encryption[32]="\0";
		strcpy(data_str,argv[2]);
		encode_string(data_str);
		write_mtd(data_str,fp_mtd,ENCRYPTION_ADDR,strlen(data_str)+1);
		memset(data_str,0,sizeof(data_str));
		
		if(!strcmp(argv[2]+strlen(PARA_ENCRYPTION)+1,"0"))
			strcpy(str_encryption,"none");
		else if(!strcmp(argv[2]+strlen(PARA_ENCRYPTION)+1,"1"))
			strcpy(str_encryption,"wep-auto");
		else if(!strcmp(argv[2]+strlen(PARA_ENCRYPTION)+1,"2"))
			strcpy(str_encryption,"psk");
		else if(!strcmp(argv[2]+strlen(PARA_ENCRYPTION)+1,"4"))
			strcpy(str_encryption,"psk2");
		else if(!strcmp(argv[2]+strlen(PARA_ENCRYPTION)+1,"6"))
			strcpy(str_encryption,"psk-mixed");
		
		sprintf(str_sp,"wireless.@wifi-iface[0].encryption=%s",str_encryption);
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);
		
	}
	else if(!strncmp(PARA_PASSWORD,argv[2],strlen(PARA_PASSWORD)))  
	{
		strcpy(data_str,argv[2]);
		encode_string(data_str);
		write_mtd(data_str,fp_mtd,PASSWORD_ADDR,strlen(data_str)+1);
		memset(data_str,0,sizeof(data_str));
		sprintf(str_sp,"wireless.@wifi-iface[0].key=%s",argv[2]+strlen(PARA_PASSWORD)+1);
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);
	}
	else if(!strncmp(PARA_IP,argv[2],strlen(PARA_IP)))  
	{
		char org_ip[32]="\0";
		char set_ip[32]="\0";
		strcpy(set_ip,argv[2]+strlen(PARA_IP)+1);
		
		
		strcpy(uci_option_str,"network.lan.ipaddr");  		
		uci_get_option_value(uci_option_str,org_ip);
		memset(uci_option_str,'\0',64);
		
		strcpy(data_str,argv[2]);
		encode_string(data_str);
		write_mtd(data_str,fp_mtd,IP_ADDR,strlen(data_str)+1);
		memset(data_str,0,sizeof(data_str));
		//sprintf(str_sp,"powertools.@system[0].address=%s",argv[2]+strlen(PARA_IP)+1);
		//uci_set_option_value(str_sp);
		//memset(str_sp,0,64);
		sprintf(str_sp,"network.lan.ipaddr=%s",argv[2]+strlen(PARA_IP)+1);
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);
		
		if( strcmp(org_ip,set_ip)!=0)
		{
			sprintf(str_sp,"sed -e 's/%s/%s/g' -i /etc/hosts",org_ip,set_ip);
			system(str_sp);
			
			memset(str_sp,0,64);
		}
		
		
	}
	else if(!strncmp(PARA_DHCP_START,argv[2],strlen(PARA_DHCP_START)))  //PARA_DHCP_START
	{
		strcpy(data_str,argv[2]);
		encode_string(data_str);
		write_mtd(data_str,fp_mtd,DHCP_START_ADDR,strlen(data_str)+1);
		memset(data_str,0,sizeof(data_str));
		sprintf(str_sp,"dhcp.lan.start=%s",argv[2]+strlen(PARA_DHCP_START)+1);
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);
	}
	else if(!strncmp(PARA_DHCP_END,argv[2],strlen(PARA_DHCP_END))) 
	{
		char start_num[32]="\0";
		char end_num[32]="\0";
		int istart=0;
		int iend=0;
		strcpy(data_str,argv[2]);
		encode_string(data_str);
		write_mtd(data_str,fp_mtd,DHCP_END_ADDR,strlen(data_str)+1);
		memset(data_str,0,sizeof(data_str));
		
		strcpy(uci_option_str,"dhcp.lan.start");  		
		uci_get_option_value(uci_option_str,start_num);
		memset(uci_option_str,'\0',64);
		
		strcpy(end_num,argv[2]+strlen(PARA_DHCP_END)+1);
		istart= atoi(start_num);
		iend=atoi(end_num);
		
		sprintf(str_sp,"dhcp.lan.limit=%d",iend-istart);
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);
	}
	else if(!strncmp(PARA_WPA_CIPHER,argv[2],strlen(PARA_WPA_CIPHER)))  //tkip---1 aes---2 tkip/aes---3
	{
		char str_encryp[32]="\0";
		char str_cipher[32]="\0";
		char tmp_cipher[32]="\0";
		strcpy(data_str,argv[2]);
		encode_string(data_str);
		write_mtd(data_str,fp_mtd,WPA_CIPHER_ADDR,strlen(data_str)+1);
		memset(data_str,0,sizeof(data_str));
		
		strcpy(uci_option_str,"wireless.@wifi-iface[0].encryption");  		
		uci_get_option_value(uci_option_str,str_encryp);
		memset(uci_option_str,'\0',64);
		
		if(!strcmp(argv[2]+strlen(PARA_WPA_CIPHER)+1,"1"))
			strcpy(tmp_cipher,"TKIP");
		else if(!strcmp(argv[2]+strlen(PARA_WPA_CIPHER)+1,"2"))
			strcpy(tmp_cipher,"AES");
		else if(!strcmp(argv[2]+strlen(PARA_WPA_CIPHER)+1,"3"))
			strcpy(tmp_cipher,"TKIP+AES");
		
		if(!strcmp(str_encryp,"psk")||!strcmp(str_encryp,"psk2")||!strcmp(str_encryp,"psk-mixed"))
		{
			sprintf(str_sp,"wireless.@wifi-iface[0].wpa_crypto=%s",tmp_cipher);
			uci_set_option_value(str_sp);
			memset(str_sp,0,64);
		}

	}
	#if 0
	else if(!strncmp(AIRPLAY_NAME,argv[2],strlen(AIRPLAY_NAME)))  //tkip---1 aes---2 tkip/aes---3
	{
		strcpy(data_str,argv[2]);
		encode_string(data_str);
		write_mtd(data_str,fp_mtd,AIRPLAY_ADDR,strlen(data_str)+1);
		memset(data_str,0,sizeof(data_str));

		bzero(set_nor_str,sizeof(set_nor_str));
		sprintf(set_nor_str,"nor set airplay_name=%s",argv[2]+strlen(AIRPLAY_NAME)+1);
		system(set_nor_str);

		
		sprintf(data_str,"shair.@shairname[0].airplay_name=%s",argv[2]+strlen(AIRPLAY_NAME)+1);
		uci_set_option_value(data_str);
	}
	else if(!strncmp(DLNA_NAME,argv[2],strlen(DLNA_NAME)))  //tkip---1 aes---2 tkip/aes---3
	{
		strcpy(data_str,argv[2]);
		write_mtd(data_str,fp_mtd,DLNA_ADDR,strlen(data_str)+1);
		memset(data_str,0,sizeof(data_str));

		bzero(set_nor_str,sizeof(set_nor_str));
		sprintf(set_nor_str,"nor set dlna_name=%s",argv[2]+strlen(DLNA_NAME)+1);
		system(set_nor_str);

		
		sprintf(data_str,"shair.@shairname[0].dlna_name=%s",argv[2]+strlen(DLNA_NAME)+1);
		uci_set_option_value(data_str);
	}
	#endif
	else if(!strncmp(DMS_NAME,argv[2],strlen(DMS_NAME)))  //tkip---1 aes---2 tkip/aes---3
	{
		strcpy(data_str,argv[2]);
		encode_string(data_str);
		write_mtd(data_str,fp_mtd,DMS_NAME_ADDR,strlen(data_str)+1);
		memset(data_str,0,sizeof(data_str));
		sprintf(data_str,"ushare.@ushare[0].servername=%s",argv[2]+strlen(DMS_NAME)+1);
		uci_set_option_value(data_str);
	}
	else if(!strncmp(DMS_ENABLE,argv[2],strlen(DMS_ENABLE)))
	{
		strcpy(data_str,argv[2]);
		write_mtd(data_str,fp_mtd,DMS_ENABLE_ADDR,strlen(data_str)+1);
		memset(data_str,0,sizeof(data_str));
		sprintf(data_str,"ushare.@ushare[0].enabled=%s",argv[2]+strlen(DMS_ENABLE)+1);

		uci_set_option_value(data_str);
	}

	else if(!strncmp(FW_VERSION,argv[2],strlen(FW_VERSION)))
	{
		DEBUG(printf("FW_VERSION %s\n",argv[2]));
		unsigned char tmp[32]={0}; 
		strcpy(tmp,argv[2]);
		write_mtd(tmp,fp_mtd,FW_VERSION_ADDR,strlen(tmp)+1);
		sprintf(set_nrender_str,"sed -e \'/^%s=.*/d\' -i /etc/nrender.conf",FW_VERSION);
		DEBUG(printf("delete::: %s\n",set_nrender_str));
		system(set_nrender_str);
		memset(set_nrender_str,0,sizeof(set_nrender_str));
		sprintf(set_nrender_str,"echo \'%s\' >> /etc/nrender.conf",tmp);
		DEBUG(printf("echo::: %s\n",set_nrender_str));
		system(set_nrender_str);

	}
	else if(!strncmp(MODEL_NAME,argv[2],strlen(MODEL_NAME)))
	{
		DEBUG(printf("MODEL_NAME %s\n",argv[2]));
		unsigned char tmp[32]={0}; 
		strcpy(tmp,argv[2]);
		write_mtd(tmp,fp_mtd,MODEL_NAME_ADDR,strlen(tmp)+1);
		sprintf(set_nrender_str,"sed -e \'/^%s=.*/d\' -i /etc/nrender.conf",MODEL_NAME);
		DEBUG(printf("delete::: %s\n",set_nrender_str));
		system(set_nrender_str);
		memset(set_nrender_str,0,sizeof(set_nrender_str));
		sprintf(set_nrender_str,"echo \'%s\' >> /etc/nrender.conf",tmp);
		DEBUG(printf("echo::: %s\n",set_nrender_str));
		system(set_nrender_str);

	}
	else if(!strncmp(MODEL_NUMBER,argv[2],strlen(MODEL_NUMBER)))
	{
		DEBUG(printf("MODEL_NUMBER %s\n",argv[2]));
		unsigned char tmp[32]={0}; 
		strcpy(tmp,argv[2]);
		write_mtd(tmp,fp_mtd,MODEL_NUMBER_ADDR,strlen(tmp)+1);
		sprintf(set_nrender_str,"sed -e \'/^%s=.*/d\' -i /etc/nrender.conf",MODEL_NUMBER);
		DEBUG(printf("delete::: %s\n",set_nrender_str));
		system(set_nrender_str);
		memset(set_nrender_str,0,sizeof(set_nrender_str));
		sprintf(set_nrender_str,"echo \'%s\' >> /etc/nrender.conf",tmp);
		DEBUG(printf("echo::: %s\n",set_nrender_str));
		system(set_nrender_str);


	}
	else if(!strncmp(MODEL_DESCRIPTION,argv[2],strlen(MODEL_DESCRIPTION)))
	{
		DEBUG(printf("MODEL_DESCRIPTION %s\n",argv[2]));
		unsigned char tmp[64]={0}; 
		strcpy(tmp,argv[2]);
		write_mtd(tmp,fp_mtd,MODEL_DESCRIPTION_ADDR1,strlen(tmp)+1);
		sprintf(set_nrender_str,"sed -e \'/^%s=.*/d\' -i /etc/nrender.conf",MODEL_DESCRIPTION);
		DEBUG(printf("delete::: %s\n",set_nrender_str));
		system(set_nrender_str);
		memset(set_nrender_str,0,sizeof(set_nrender_str));
		sprintf(set_nrender_str,"echo \'%s\' >> /etc/nrender.conf",tmp);
		DEBUG(printf("echo::: %s\n",set_nrender_str));
		system(set_nrender_str);


	}
	else if(!strncmp(MANUFACTURER_URL,argv[2],strlen(MANUFACTURER_URL)))
	{
		DEBUG(printf("set MANUFACTURER_URL %s\n",argv[2]));
		unsigned char tmp[64]={0}; 
		strcpy(tmp,argv[2]);
		write_mtd(tmp,fp_mtd,MANUFACTURER_URL_ADDR1,64);
		sprintf(set_nrender_str,"sed -e \'/^%s=.*/d\' -i /etc/nrender.conf",MANUFACTURER_URL);
		DEBUG(printf("delete::: %s\n",set_nrender_str));
		system(set_nrender_str);
		memset(set_nrender_str,0,sizeof(set_nrender_str));
		sprintf(set_nrender_str,"echo \'%s\' >> /etc/nrender.conf",tmp);
		DEBUG(printf("echo::: %s\n",set_nrender_str));
		system(set_nrender_str);


	}
	else if(!strncmp(MANUFACTURER,argv[2],strlen(MANUFACTURER)))
	{
		DEBUG(printf("MANUFACTURER %s\n",argv[2]));
		unsigned char tmp[32]={0}; 
		strcpy(tmp,argv[2]);
		write_mtd(tmp,fp_mtd,MANUFACTURER_ADDR,strlen(tmp)+1);
		sprintf(set_nrender_str,"sed -e \'/^%s=.*/d\' -i /etc/nrender.conf",MANUFACTURER);
		DEBUG(printf("delete::: %s\n",set_nrender_str));
		system(set_nrender_str);
		memset(set_nrender_str,0,sizeof(set_nrender_str));
		sprintf(set_nrender_str,"echo \'%s\' >> /etc/nrender.conf",tmp);
		DEBUG(printf("echo::: %s\n",set_nrender_str));
		system(set_nrender_str);

	}
	else if(!strncmp(SMB_USR_NAMR,argv[2],strlen(SMB_USR_NAMR)))
	{
		strcpy(data_str,argv[2]);
		DEBUG(printf("%s\n",data_str));
		unsigned char tmp[32];
		char old_name[32]={0};
		char change_etc_pwd_str[128]={0};
		memset(tmp,0,sizeof(tmp));
		read_mtd(tmp,fp_mtd,SMB_USR_NAMR_ADDR,32);
		DEBUG(printf("get %s\n",(char *)tmp));
		if(strstr(tmp,SMB_USR_NAMR)!=NULL)
		{
			strcpy(old_name,tmp+strlen(SMB_USR_NAMR)+1);
			memset(change_etc_pwd_str,0,sizeof(change_etc_pwd_str));
			sprintf(change_etc_pwd_str,"sed -e \'s/^%s:/%s:/g\' -i /etc/passwd",old_name,argv[2]+strlen(SMB_USR_NAMR)+1);
			DEBUG(printf("change /etc/passwd : %s",change_etc_pwd_str));
			system(change_etc_pwd_str);
			memset(change_etc_pwd_str,0,sizeof(change_etc_pwd_str));
			sprintf(change_etc_pwd_str,"sed -e \'s/:%s:/:%s:/g\' -i /etc/passwd",old_name,argv[2]+strlen(SMB_USR_NAMR)+1);
			DEBUG(printf("change /etc/passwd : %s",change_etc_pwd_str));
			system(change_etc_pwd_str);
//--------------------------------------------------------------------
			memset(change_etc_pwd_str,0,sizeof(change_etc_pwd_str));
			sprintf(change_etc_pwd_str,"sed -e \'s/^%s:/%s:/g\' -i /etc/samba/smbpasswd",old_name,argv[2]+strlen(SMB_USR_NAMR)+1);
			system(change_etc_pwd_str);
			memset(change_etc_pwd_str,0,sizeof(change_etc_pwd_str));
			sprintf(change_etc_pwd_str,"sed -e \'s/:%s:/:%s:/g\' -i /etc/samba/smbpasswd",old_name,argv[2]+strlen(SMB_USR_NAMR)+1);
			system(change_etc_pwd_str);
			
		}
		memset(change_etc_pwd_str,0,sizeof(change_etc_pwd_str));
		sprintf(change_etc_pwd_str,"sed -e \'s/^%s:/%s:/g\' -i /etc/samba/smbpasswd","airdisk",argv[2]+strlen(SMB_USR_NAMR)+1);
		system(change_etc_pwd_str);
		memset(change_etc_pwd_str,0,sizeof(change_etc_pwd_str));
		sprintf(change_etc_pwd_str,"sed -e \'s/:%s:/:%s:/g\' -i /etc/samba/smbpasswd","airdisk",argv[2]+strlen(SMB_USR_NAMR)+1);
		system(change_etc_pwd_str);
//-----------------------------------------------------------------
		memset(change_etc_pwd_str,0,sizeof(change_etc_pwd_str));
		sprintf(change_etc_pwd_str,"sed -e \'s/^%s:/%s:/g\' -i /etc/passwd","airdisk",argv[2]+strlen(SMB_USR_NAMR)+1);
		DEBUG(printf("change /etc/passwd : %s",change_etc_pwd_str));
		system(change_etc_pwd_str);
		memset(change_etc_pwd_str,0,sizeof(change_etc_pwd_str));
		sprintf(change_etc_pwd_str,"sed -e \'s/:%s:/:%s:/g\' -i /etc/passwd","airdisk",argv[2]+strlen(SMB_USR_NAMR)+1);
		DEBUG(printf("change /etc/passwd : %s",change_etc_pwd_str));
		system(change_etc_pwd_str);

		write_mtd(data_str,fp_mtd,SMB_USR_NAMR_ADDR,strlen(data_str)+1);
		
		memset(str_sp,0,sizeof(str_sp));
		sprintf(str_sp,"samba.@samba[0].description=%s",argv[2]+strlen(SMB_USR_NAMR)+1);
		DEBUG(printf("%s\n",str_sp));
		uci_set_option_value(str_sp);		
		
		memset(str_sp,0,sizeof(str_sp));
		sprintf(str_sp,"samba.@samba[0].name=%s",argv[2]+strlen(SMB_USR_NAMR)+1);
		DEBUG(printf("%s\n",str_sp));
		uci_set_option_value(str_sp);
		
		memset(str_sp,0,64);
		sprintf(str_sp,"samba.@samba[0].user=%s",argv[2]+strlen(SMB_USR_NAMR)+1);
		DEBUG(printf("%s\n",str_sp));
		uci_set_option_value(str_sp);
		
		memset(str_sp,0,64);
		sprintf(str_sp,"samba.@sambashare[0].name=%s",argv[2]+strlen(SMB_USR_NAMR)+1);
		DEBUG(printf("%s\n",str_sp));
		uci_set_option_value(str_sp);
		
		memset(str_sp,0,64);
		sprintf(str_sp,"samba.@sambashare[0].users=%s",argv[2]+strlen(SMB_USR_NAMR)+1);
		DEBUG(printf("%s\n",str_sp));
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);


		
	}
	else if(!strncmp(SMB_USR_PWD,argv[2],strlen(SMB_USR_PWD)))
	{
		strcpy(data_str,argv[2]);
		DEBUG(printf("%s\n",data_str));
		write_mtd(data_str,fp_mtd,SMB_USR_PWD_ADDR,strlen(data_str)+1);
		memset(data_str,0,sizeof(data_str));
		sprintf(str_sp,"samba.@samba[0].password=%s",argv[2]+strlen(SMB_USR_PWD)+1);
		DEBUG(printf("%s\n",str_sp));
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);

	}
	else if(!strncmp(SMB_GUEST_OK,argv[2],strlen(SMB_GUEST_OK)))
	{
		strcpy(data_str,argv[2]);
		DEBUG(printf("%s\n",data_str));
		write_mtd(data_str,fp_mtd,SMB_GUEST_OK_ADDR,strlen(data_str)+1);
		memset(data_str,0,sizeof(data_str));
		sprintf(str_sp,"samba.@sambashare[0].guest_ok=%s",argv[2]+strlen(SMB_GUEST_OK)+1);
		DEBUG(printf("%s\n",str_sp));
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);
//-----------set nor param---------------------------------------------------
		bzero(set_nor_str,sizeof(set_nor_str));
		if(!strcmp(argv[2]+strlen(SMB_GUEST_OK)+1,"yes"))
			sprintf(set_nor_str,"nor set smb_anonymous_en=%s","1");
		else
			sprintf(set_nor_str,"nor set smb_anonymous_en=%s","0");
		system(set_nor_str);

	}
	else if(!strncmp(SMB_ENABLED,argv[2],strlen(SMB_ENABLED)))
	{
		strcpy(data_str,argv[2]);
		DEBUG(printf("%s\n",data_str));
		write_mtd(data_str,fp_mtd,SMB_ENABLED_ADDR,strlen(data_str)+1);
		memset(data_str,0,sizeof(data_str));
		sprintf(str_sp,"samba.@samba[0].enabled=%s",argv[2]+strlen(SMB_ENABLED)+1);
		DEBUG(printf("%s\n",str_sp));
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);
//-----------set nor param---------------------------------------------------
		bzero(set_nor_str,sizeof(set_nor_str));
		sprintf(set_nor_str,"nor set smb_enable=%s",argv[2]+strlen(SMB_ENABLED)+1);
		system(set_nor_str);

	}
	else if(!strncmp(HOST_NAME,argv[2],strlen(HOST_NAME)))
	{
		char old_host_name[64]={0};
		char new_host_name[64]={0};
		strcpy(data_str,argv[2]);
		write_mtd(data_str,fp_mtd,HOST_NAME_ADDR,strlen(data_str)+1);
		memset(data_str,0,sizeof(data_str));
		
		strcpy(uci_option_str,"system.@system[0].hostname");  		
		uci_get_option_value(uci_option_str,old_host_name);
		memset(uci_option_str,'\0',64);
		strcpy(new_host_name,argv[2]+strlen(HOST_NAME)+1);
		memset(str_sp,0,64);
		sprintf(str_sp,"system.@system[0].hostname=%s",argv[2]+strlen(HOST_NAME)+1);
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);
		sprintf(str_sp,"network.wan.hostname=%s",argv[2]+strlen(HOST_NAME)+1);
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);
		sprintf(str_sp,"network.lan.hostname=%s",argv[2]+strlen(HOST_NAME)+1);
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);

		sprintf(str_sp,"sed -e 's/%s/%s/g' -i /etc/hosts",old_host_name,new_host_name);
		system(str_sp);
		memset(str_sp,0,64);
	}
	else if(!strncmp(VERSION_FLAG,argv[2],strlen(VERSION_FLAG)))
	{
		strcpy(data_str,argv[2]);
		write_mtd(data_str,fp_mtd,VERSION_ADDR,strlen(data_str)+1);
	}
	else if(!strncmp(REPAIR_AUTO,argv[2],strlen(REPAIR_AUTO)))
	{
		strcpy(data_str,argv[2]);
		write_mtd(data_str,fp_mtd,REPAIR_AUTO_ADDR,strlen(data_str)+1);
	}
	else if(!strncmp(SN,argv[2],strlen(SN)))
	{
		strcpy(data_str,argv[2]);
		if(strlen(argv[2]) != (SN_LEN+strlen(SN)+1))
		{	printf("Error qqsn=%s length=%d\n",argv[2],strlen(argv[2]));
			exit(1);
		}
		else
			{
				write_mtd(data_str,fp_mtd,SN_ADDR,strlen(data_str)+1);
				sprintf(str_sp,"echo %s >/etc/qq/Guid_file.txt",argv[2]+strlen(SN)+1);	
				system(str_sp);
				memset(str_sp,0,64);
			}
	}else if(!strncmp(LICENSE,argv[2],strlen(LICENSE)))
	{
		strcpy(license_str,argv[2]);
		//printf("license=%s\n",license_str);
		if(strlen(argv[2])>=(LICENSE_LEN+strlen(LICENSE)+1))
		{	printf("Error license=%s length=%d\n",argv[2],strlen(argv[2]));
			exit(1);
		}
		else
			{
				write_mtd(license_str,fp_mtd,LICENSE_ADDR,strlen(license_str)+1);
				sprintf(license_str,"echo %s >/etc/qq/licence.sign.file.txt",argv[2]+strlen(LICENSE)+1);	
				system(license_str);
			}
	}
	else if(!strncmp(ROOT_PWD,argv[2],strlen(ROOT_PWD)))
	{
		strcpy(data_str,argv[2]);
		write_mtd(data_str,fp_mtd,ROOT_PWD_ADDR,strlen(data_str)+1);
	}
	else
	{
		#if Debug_cfg
		printf("Invalid parameter!\r\n");
		usage();
		#endif
	}
	system("uci commit");
	fclose(fp_mtd);
}
static void cfg_list()
{
	char *arg_v[3];
	arg_v[2]="mac";
	cfg_get(arg_v);
	arg_v[2]="ssid";
	cfg_get(arg_v);
	arg_v[2]="encryption";
	cfg_get(arg_v);
	arg_v[2]="password";
	cfg_get(arg_v);
	arg_v[2]="host_name";
	cfg_get(arg_v);
	arg_v[2]="ip";
	cfg_get(arg_v);
	arg_v[2]="dhcp_start";
	cfg_get(arg_v);
	arg_v[2]="dhcp_end";
	cfg_get(arg_v);
	arg_v[2]="wpa_cipher";
	cfg_get(arg_v);
	arg_v[2]="airplay_name";
	cfg_get(arg_v);
	arg_v[2]="dlna_name";
	cfg_get(arg_v);
	arg_v[2]="fw_version";
	cfg_get(arg_v);
	arg_v[2]="model_name";
	cfg_get(arg_v);
	arg_v[2]="model_number";
	cfg_get(arg_v);
	arg_v[2]="model_description";
	cfg_get(arg_v);
	arg_v[2]="manufacturer";
	cfg_get(arg_v);
	arg_v[2]="url_manufacturer";
	cfg_get(arg_v);
	arg_v[2]="dms_name";
	cfg_get(arg_v);
	arg_v[2]="dms_enable";
	cfg_get(arg_v);
	arg_v[2]="smb_usr_name";
	cfg_get(arg_v);
	arg_v[2]="smb_usr_pwd";
	cfg_get(arg_v);
	arg_v[2]="smb_guest_ok";
	cfg_get(arg_v);
	arg_v[2]="smb_enabled";
	cfg_get(arg_v);	
	arg_v[2]="version_flag";
	cfg_get(arg_v);
	arg_v[2]="repair_auto";
	cfg_get(arg_v);
	arg_v[2]="qqsn";
	cfg_get(arg_v);
	arg_v[2]="license";
	cfg_get(arg_v);
	arg_v[2]="root_pwd";
	cfg_get(arg_v);
}

int main(int argc,char *argv[])
{
	ctx=uci_alloc_context();
	if(argc < 2 || !strcmp(argv[1],"-h"))
	{
		usage();
		exit(0);
	}
	if(!strcmp(argv[1],"get"))
	{
		//if(check_cfg_flag(1))
			cfg_get(argv);
	}
	else if(!strcmp(argv[1],"set"))
	{
		//set_cfg_flag();
		cfg_set(argv);
		
	}
	else if(!strcmp(argv[1],"checkflag"))
	{
		check_cfg_flag(0);
	}
	else if(!strcmp(argv[1],"setflag"))
	{
		set_cfg_flag();
	}
	else if(!strcmp(argv[1],"clearflag"))
	{
		clear_cfg_flag();
	}
	else if(!strcmp(argv[1],"list"))
	{
		cfg_list();
	}
	else
	{
		usage();
	}
	
	uci_free_context(ctx);
	return 0;
}
