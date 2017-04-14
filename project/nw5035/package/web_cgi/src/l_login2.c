#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include "msg.h"
#include "socket_uart.h" 
#include <linux/wireless.h>
#include <mntent.h>
#include <sys/vfs.h>
#include <dirent.h>

#include <sys/stat.h>
#include <libudev.h>

#include "uci_for_cgi.h"
// /etc/config/system sid
#define SID "sid"
char tmp_mac[32];

#if WIRELESS_EXT <= 11
#ifndef SIOCDEVPRIVATE
#define SIOCDEVPRIVATE      0x8BE0
#endif
#define SIOCIWFIRSTPRIV      SIOCDEVPRIVATE
#endif
#define RTPRIV_IOCTL_SHOW_CONNSTATUS	(SIOCIWFIRSTPRIV + 0x1B)
#define RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT	(SIOCIWFIRSTPRIV + 0x1F)

#define MAC_ADDR_LEN        6
#define ETH_LENGTH_OF_ADDRESS      6
#define MAX_LEN_OF_MAC_TABLE      64

typedef union _MACHTTRANSMIT_SETTING {
	struct {
		unsigned short MCS:7;	/* MCS */
		unsigned short BW:1;	/*channel bandwidth 20MHz or 40 MHz */
		unsigned short ShortGI:1;
		unsigned short STBC:2;	/*SPACE */
		unsigned short rsv:3;
		unsigned short MODE:2;	/* Use definition MODE_xxx. */
	} field;
	unsigned short word;
} MACHTTRANSMIT_SETTING, *PMACHTTRANSMIT_SETTING;

typedef struct _RT_802_11_MAC_ENTRY {
	unsigned char ApIdx;
	unsigned char Addr[MAC_ADDR_LEN];
	unsigned char Aid;
	unsigned char Psm;		/* 0:PWR_ACTIVE, 1:PWR_SAVE */
	unsigned char MimoPs;		/* 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled */
	signed char AvgRssi0;
	signed char AvgRssi1;
	signed char AvgRssi2;
	unsigned int ConnectedTime;
	MACHTTRANSMIT_SETTING TxRate;
	unsigned int LastRxRate;
	signed short StreamSnr[3];				/* BF SNR from RXWI. Units=0.25 dB. 22 dB offset removed */
	signed short SoundingRespSnr[3];			/* SNR from Sounding Response. Units=0.25 dB. 22 dB offset removed */
/*	signed short TxPER;	*/					/* TX PER over the last second. Percent */
/*	signed short reserved;*/

} RT_802_11_MAC_ENTRY, *PRT_802_11_MAC_ENTRY;

typedef struct _RT_802_11_MAC_TABLE {
	unsigned long Num;
	RT_802_11_MAC_ENTRY Entry[MAX_LEN_OF_MAC_TABLE];
} RT_802_11_MAC_TABLE, *PRT_802_11_MAC_TABLE;
int updateSysVal(const char *para,const char *val){
	char set_str[128]={0};
	char tmp[128]={0};
	sprintf(set_str,"sed -e \'/%s/d\' -i /tmp/state/status",para);
	system(set_str);
	memset(set_str,0,sizeof(set_str));
	
	sprintf(set_str,"echo \'%s=%s\' >> /tmp/state/status",para, val);
	system(set_str);
#if 0
	char uci_option_str[64]="\0";
	ctx=uci_alloc_context();
	uci_add_delta_path(ctx,"/tmp/state");
	uci_set_confdir(ctx,"/tmp/state");
	memset(uci_option_str,'\0',64); 
	sprintf(uci_option_str,"status.@downloading[0].%s=%s",para,val);		
	uci_set_option_value(uci_option_str);

	//system("uci commit");
	uci_free_context(ctx);
#endif
}


int getOnline()
{
		int    socket_id;
		struct	 iwreq wrq;
		char data[4096]="\0";
		int    ret;
		RT_802_11_MAC_TABLE    *mp;
		int count = 0;
		char mode[64] = "\0";
		char uci_option_str[64] = "\0";
		FILE *read_fp; 
		int chars_read = 0;
		char buffer[64] = "\0";
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"system.@system[0].wifimode"); 
		uci_get_option_value(uci_option_str,mode);
		if(strcmp(mode,"ap") == 0)
		{	
		
			socket_id = socket(AF_INET, SOCK_DGRAM, 0);
			if(socket_id < 0)
			{
				//sprintf(ret_buf,"{\"status\":0,\"errorCode\":3,\"errorMessage\":\"socket error!\",\"data\":{}}");
				//sprintf(ret_buf,"<Return status=\"false\">socket error!</Return>");
				return -1;
			}
			memset(data, 0x00, 4096);
			strcpy(wrq.ifr_name, "ra0");
			wrq.u.data.length = 4096;
			wrq.u.data.pointer = data;
			wrq.u.data.flags = 0;
			ret = ioctl(socket_id, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &wrq);
			if(ret != 0)
			{
				//sprintf(ret_buf,"{\"status\":0,\"errorCode\":3,\"errorMessage\":\"ioctl error!\",\"data\":{}}");
			//	sprintf(retstr,"<Return status=\"false\">ioctl error!</Return>");
				close(socket_id);
				return -1;
			}
			mp = (RT_802_11_MAC_TABLE *)wrq.u.data.pointer;
			count = mp->Num;
			close(socket_id);
		}
		else if (strcmp(mode,"sta") == 0)
		{
			read_fp=popen("wpa_cli all_sta | grep dot11RSNAStatsSTAAddress | wc -l","r");
			if(read_fp != NULL)
			{
				chars_read = fread(buffer, sizeof(char), 64-1, read_fp); 
				if (chars_read > 0)
				{
					count = atoi(buffer);
					pclose(read_fp);
					
				}else
				{
					pclose(read_fp);
					//sprintf(ret_buf,"{\"status\":0,\"errorCode\":3,\"errorMessage\":\"popen error!\",\"data\":{}}");
	//				sprintf(retstr,"<Return status=\"false\">popen error!</Return>");
					return -1;
				}
			}
			else
			{
				//sprintf(ret_buf,"{\"status\":0,\"errorCode\":3,\"errorMessage\":\"popen error!\",\"data\":{}}");
	//			sprintf(retstr,"<Return status=\"false\">popen error!</Return>");
				return -1;
			}
		}
		else
		{
			//sprintf(ret_buf,"{\"status\":0,\"errorCode\":3,\"errorMessage\":\"wifi mode error!\",\"data\":{}}");
			//sprintf(retstr,"<Return status=\"false\">wifi mode error!</Return>");
			return -1;
		}
		//sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"count\":%d}}",count);
		//sprintf(retstr,"<Users count=\"%d\"></Users>",count);
		//p_debug(ret_buf);
		return count;
}

int getMac(){
	FILE *read_fp = NULL;
	read_fp = popen("hexdump -s 4 -n 6 -C /dev/mtd3 | head -n 1 | sed 's/\ \ /:/g' | cut -d: -f 2 | sed 's/\ /:/g' | tr \"[a-z]\" \"[A-Z]\"", "r");
	if(read_fp != NULL)
	{
		memset(tmp_mac,0,32);
		fgets(tmp_mac, 18, read_fp);
		return 0;
	}
	else
		return -1;
	p_debug("tmp_mac=%s",tmp_mac);
}


int get_conf_str(char *dest,char *var)
{
	FILE *fp=fopen("/tmp/state/status","r");
	if(NULL == fp)
	{
		//printf("open /etc/config/nrender.conf failed \n");
		return 0;
	}
	char tmp[128];
	char *ret_str;
	bzero(tmp,128);
	while(fgets(tmp,128,fp)!=NULL)
	{
		if('\n'==tmp[strlen(tmp)-1])
		{
			tmp[strlen(tmp)-1]=0;
		}
		//printf("get string from /etc/config/nrender.conf:%s\n",tmp);
		if(!strncmp(var,tmp,strlen(var)))
		{
			ret_str = malloc(strlen(tmp)-strlen(var));
			if(!ret_str)
			{
				fclose(fp);
				return 0;
			}
			bzero(ret_str,strlen(tmp)-strlen(var));
			strcpy(ret_str,tmp+strlen(var)+1);
			
			//printf("ret string:%s\n",ret_str);
			fclose(fp);
			strcpy(dest,ret_str);
			free(ret_str);
			return 0;
		}
		
	}
	fclose(fp);
	return 0;
}

int getPower()
{

	char bat[8]={0};
	system("letv_gpio 1 & >/dev/null");
	get_conf_str(bat,"power");
	//	p_debug("bat=%s\n",bat);
	return atoi(bat);
/*
	int percent=0;
	unsigned char stat=0;
	unsigned char tmpStatus=0;
	int bat;
	unsigned char chargflag = 0;
	int count = 0;
	int fh=NULL;
	int ret=0;
	socket_uart_cmd_t g_uart_cmd;
	bzero(&g_uart_cmd, sizeof(socket_uart_cmd_t));

	p_debug("ret===sssssssss");
	g_uart_cmd.mode = UART_R;
	g_uart_cmd.regaddr =SOCKET_UART_GET_POWER_PERCENT;

	ret=SocketUartClientStart(&g_uart_cmd);
	p_debug("ret===%d",ret);

	if(ret==0)
		bat = g_uart_cmd.data & 0xFF;
	else{
		//error_num++;
		//sprintf(retstr,"{\"status\":0,\"errorCode\":3,\"errorMessage\":\"get power error\",\"data\":{}}");
		return 0;
	}
	//p_debug("bat=%d",bat);
	
	if(bat<=5)percent=0;
	else if(bat>5&&bat<=10)percent=10;
	else if(bat>10&&bat<=25)percent=25;
	else if(bat>25&&bat<=50)percent=50;
	else if(bat>50&&bat<=75)percent=75;
	else if(bat>75&&bat<=100)percent=100;
	
	//sprintf(retstr,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"power\":%d}}",percent);
	return bat;
	*/
}

typedef struct storage{
	unsigned long long total;
	unsigned long long used;
}stor;
int  getStorage( stor *storage)
{
	p_debug("%s","access getStorage");

	FILE* mount_table;
	struct mntent *mount_entry;
	struct statfs fs;
	unsigned long blocks_used;
	unsigned blocks_percent_used;
	char tmpi=0;
	char tmpstr[128]="\0";
	 struct udev *udev;
	 struct udev_enumerate *enumerate;
	 struct udev_list_entry *devices, *dev_list_entry;
	 struct udev_device *dev;
	 int i=0;
	 int j=0;
	  typedef struct myudevstruct{
	  char devpath[20];
	 // char * mountpoint;
	  char pid[10];
	  char vid[10];
	 }myudev;
	 myudev myusb[20];
	 myudev *p=myusb;

	//stor  storage;
	 /* Create the udev object */
	 udev = udev_new();
	 if (!udev) {
	  printf("Can't create udev\n");       
	  exit(1);
	 }
	 
	 enumerate = udev_enumerate_new(udev);
	 udev_enumerate_add_match_subsystem(enumerate, "block");
	 udev_enumerate_scan_devices(enumerate);
	 devices = udev_enumerate_get_list_entry(enumerate);
	 udev_list_entry_foreach(dev_list_entry, devices) {
	  const char *path;
	  
	  path = udev_list_entry_get_name(dev_list_entry);
	  dev = udev_device_new_from_syspath(udev, path);

	  strcpy(p[i].devpath,udev_device_get_devnode(dev));
	  dev = udev_device_get_parent_with_subsystem_devtype(
	         dev,
	         "usb",
	         "usb_device");
	  if (!dev) {
	    break;
	  // printf("Unable to find parent usb device.");
	  // exit(1);
	  }
	 // p=myusb malloc(sizeof(myusb));
	 // myusb[i].pid=malloc(8*sizeof(char));
	  strcpy(p[i].vid,udev_device_get_sysattr_value(dev,"idVendor"));
	  strcpy(p[i].pid,udev_device_get_sysattr_value(dev, "idProduct"));
	  //printf("myusb[%d]:dev=%s,vid=%s,pid=%s\n",i, p[i].devpath,p[i].vid,p[i].pid);
	  i++;
	  udev_device_unref(dev);
	 }
	 /* Free the enumerator object */
	 udev_enumerate_unref(enumerate);

	 udev_unref(udev);

	  mount_table = setmntent("/proc/mounts", "r");
	  if (!mount_table)
	  {
	    fprintf(stderr, "set mount entry error\n");
	    return -1;
	  }
	  //printf("<Storage>");
	  //strcpy(retstr,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{");
	  
	  while (1) {
	    const  char *device;
	    const char *mount_point;
	    if (mount_table) {
	      mount_entry = getmntent(mount_table);
	      if (!mount_entry) {
	        endmntent(mount_table);
	        break;
	      }
	    } 
	    else
	      break;
	    device = mount_entry->mnt_fsname;
	    mount_point = mount_entry->mnt_dir;
	  
	  //  sprintf(retstr, "mount info:mountpoint=%s\n",mount_point);
	    if(strstr(mount_point,"disk")==NULL) continue;
	     tmpi++;
	    if (statfs(mount_point, &fs) != 0) 
	    {
	      fprintf(stderr, "statfs failed!\n");  
	      break;
	    }
	    if ((fs.f_blocks > 0) || !mount_table ) 
	    {
	      blocks_used = fs.f_blocks - fs.f_bfree;
	      blocks_percent_used = 0;

	      if (strcmp(device, "rootfs") == 0)
	        continue;
	      sprintf(tmpstr,"\"volume\":\"%s\",",mount_point+9);//   /tmp/mnt/
	      //if (printf("\n<Section volume=\"%s\"" + 1, device) > 20)
	      //    printf("%1s", "");
	      //strcat(retstr,tmpstr);
	      memset(tmpstr,0,128);
	      for (j = 0; j < i; j++)
	      {

	        if(!strcmp(myusb[j].devpath,device))
	        {

	        //  printf("j=%d,myusbdevice=%s,device=%s\n",j,p[j].devpath,device);
	          sprintf(tmpstr,"\"vid\":\"%s\",\"pid\":\"%s\",",p[j].vid,p[j].pid);
	        //  printf(" vid=\"%s\" pid=\"%s\"",p[j].vid,p[j].pid);
	        }
	      }
	     // strcat(retstr,tmpstr);
	      memset(tmpstr,0,128);

		//cgi_log(mount_entry->mnt_opts);
		  if(strstr(mount_entry->mnt_opts,"rw")){
			  sprintf(tmpstr,"\"rw\":\"%s\",","rw");
			  //strcat(retstr,tmpstr);
	      	  memset(tmpstr,0,128);
		  }else{
			  sprintf(tmpstr,"\"rw\":\"%s\",","ro");
			  //strcat(retstr,tmpstr);
			  memset(tmpstr,0,128);

		  }
		  
	      char s1[20];
	      char s2[20];
	      char s3[20];
	    //  printf("total blocks=%10d\nblock size=%10d\nfree blocks=%10d\n",fs.f_blocks,fs.f_bsize,fs.f_bfree);
	      //strcpy(s1, kscale(fs.f_blocks, fs.f_bsize));
	      //strcpy(s2, kscale(fs.f_blocks - fs.f_bfree, fs.f_bsize));
	      //strcpy(s3, kscale(fs.f_bavail, fs.f_bsize));
	      sprintf(tmpstr,"\"total\":\"%lld\",\"used\":\"%lld\",\"free\":\"%lld\"}",
			  (unsigned long long)(fs.f_blocks*(unsigned long long)fs.f_bsize),
			  (unsigned long long)((fs.f_blocks - fs.f_bfree)* (unsigned long long)fs.f_bsize),
			  (unsigned long long)(fs.f_bavail*(unsigned long long)fs.f_bsize)
	          );
		p_debug("%lld,%lld",storage->total,storage->used);
		storage->total=(unsigned long long)(fs.f_blocks*(unsigned long long)fs.f_bsize);
		storage->used=(unsigned long long)((fs.f_blocks - fs.f_bfree)* (unsigned long long)fs.f_bsize);
     // strcat(retstr,tmpstr);
	  //   		p_debug("%lld,%lld",storage->total,storage->used);
	// p_debug("%s",tmpstr);
	      memset(tmpstr,0,128);
	    }
	     
	  }
	   //strcat(retstr,"}");
	return 0;

}

#define STR_VERSION_CODE		"versionCode"
#define STR_VERSION_NAME		"versionName"
#define INFO_HAS_NEW			"hasNew"
#define INFO_NEXT_VERSION_CODE	"nextVersionCode"
#define INFO_NEXT_VERSION_NAME	"nextVersionName"
#define INFO_NEXT_FEATURE		"nextFeature"
#define INFO_NEXT_UPDATE_URL	"nextUpdateUrl"
#define INFO_IS_FORCE			"isForce"

#define FILE_VERSION_INFO_PATH		"/etc/fw_version.conf"
#define FILE_UPGRADE_INFO_PATH		"/etc/fw_upgrade.conf"

typedef struct _fw_version_info{
	char curVersionCode[16];
	char curVersionName[32];
	char nextVersionCode[16];
	char nextVersionName[32];
	char hasNew[16];
	char nextFeature[512];
	char nextUpdateUrl[1024];
	char isForce[16];
	char errorMsg[128];
}fw_version_info;


/* return -1:error;0:get value success;1:no value */
int get_conf_str2(char *var, char *dest, char *file_path)
{
	char tmp[512];

	if((var == NULL) || (dest == NULL) || (file_path == NULL))
	{
		//p_debug("argument invalid \n", file_path);
		return -1;
	}
	
	FILE *fp=fopen(file_path, "r");
	if(NULL == fp)
	{
		//p_debug("open %s failed \n", file_path);
		return -1;
	}
	
	bzero(tmp,512);
	while(fgets(tmp,512,fp)!=NULL)
	{
		if('\n'==tmp[strlen(tmp)-1])
		{
			tmp[strlen(tmp)-1]=0;
		}
		p_debug("get string from %s:%s\n", file_path, tmp);
		if(!strncmp(var,tmp,strlen(var)))
		{	
			bzero(dest, strlen(dest));
			strcpy(dest, tmp+strlen(var)+1);
			p_debug("dest=%s",dest);
			fclose(fp);
			return 0;
		}
		bzero(tmp, 512);
	}
	fclose(fp);
	return 1;
}


int get_fw_upgrade(fw_version_info *p_version_info)
{
	int ret = 0;
	if(p_version_info == NULL){
		return -1;
	}

	ret = get_conf_str2(INFO_HAS_NEW, p_version_info->hasNew, FILE_UPGRADE_INFO_PATH);
	if(ret == 0){
		//p_debug("ret = %d, version_code = %s", ret, p_version_info->curVersionCode);
	}
	else{
		return -1;
	}

	if(0 == strcmp(p_version_info->hasNew, "true"))
	{
		ret = get_conf_str2(INFO_NEXT_VERSION_CODE, p_version_info->nextVersionCode, FILE_UPGRADE_INFO_PATH);
		if(ret == 0){
			//p_debug("ret = %d, version_code = %s", ret, p_version_info->curVersionCode);
		}
		else{
			return -1;
		}

		ret = get_conf_str2(INFO_NEXT_VERSION_NAME, p_version_info->nextVersionName, FILE_UPGRADE_INFO_PATH);
		if(ret == 0){
			//p_debug("ret = %d, version_code = %s", ret, p_version_info->curVersionCode);
		}
		else{
			return -1;
		}

		ret = get_conf_str2(INFO_NEXT_FEATURE, p_version_info->nextFeature, FILE_UPGRADE_INFO_PATH);
		if(ret == 0){
			//p_debug("ret = %d, version_code = %s", ret, p_version_info->curVersionCode);
		}
		else{
			return -1;
		}

		ret = get_conf_str2(INFO_IS_FORCE, p_version_info->isForce, FILE_UPGRADE_INFO_PATH);
		if(ret == 0){
			//p_debug("ret = %d, version_code = %s", ret, p_version_info->curVersionCode);
		}
		else{
			return -1;
		}
	}
	return 0;
}


int get_fw_version(fw_version_info *p_version_info)
{
	int ret = 0;

	if(p_version_info == NULL){
		return -1;
	}

	ret = get_conf_str2(STR_VERSION_CODE, p_version_info->curVersionCode, FILE_VERSION_INFO_PATH);
	if(ret == 0){
		//p_debug("ret = %d, version_code = %s", ret, p_version_info->curVersionCode);
	}
	else{
		return -1;
	}

	ret = get_conf_str2(STR_VERSION_NAME, p_version_info->curVersionName, FILE_VERSION_INFO_PATH);
	if(ret == 0){
		//p_debug("ret = %d, version_code = %s", ret, p_version_info->curVersionName);
	}
	else{
		return -1;
	}
	
	return 0;
}


void main()
{
		char ret_buf[2048];
		char fw_sid[SID_LEN]="\0";
		char app_sid[SID_LEN]="\0";
		char save_code[CODE_LEN]="\0";	
		char code[CODE_LEN]="\0";
		char *web_str=NULL;
		char ap_ssid[33]={};
		char connected_ssid[33]={};
		char mac[32]={};
		char ap_key[64]={};
		char led_status[8]={};
		char wifimode[8]={0};
		char name[32]="\0";
		int power=0;
		int count=0;		
		char uci_option_str[UCI_BUF_LEN]="\0";
		ctx=uci_alloc_context();
		fw_version_info version_info;
		char smb_pwd[32]={0};
		char smb_guest_pwd[32]={0};
		char read_only[32]={0};
		char right[32]={0};


		memset(&version_info, 0, sizeof(fw_version_info));
		memset(ret_buf, 0, 2048);

		printf("Content-type:text/plain\r\n\r\n");

		strcpy(uci_option_str,"system.@system[0].sid");			//name
		uci_get_option_value(uci_option_str,fw_sid);
		memset(uci_option_str,'\0',UCI_BUF_LEN);

		strcpy(uci_option_str,"system.@system[0].wifimode");			//name
		uci_get_option_value(uci_option_str,wifimode);
		memset(uci_option_str,'\0',UCI_BUF_LEN);

		strcpy(uci_option_str,"system.@system[0].code");			//name
		uci_get_option_value(uci_option_str,save_code);
		memset(uci_option_str,'\0',UCI_BUF_LEN);
		if(!strcmp(wifimode,"sta")){
				strcpy(uci_option_str,"wireless.@wifi-iface[0].p2p_go_ssid");			//name
				uci_get_option_value(uci_option_str,ap_ssid);
				memset(uci_option_str,'\0',UCI_BUF_LEN);

				strcpy(uci_option_str,"wireless.@wifi-iface[0].p2p_go_key");			//name
				uci_get_option_value(uci_option_str,ap_key);
				memset(uci_option_str,'\0',UCI_BUF_LEN);
		}
		else {
				strcpy(uci_option_str,"wireless.@wifi-iface[0].ssid");			//name
				uci_get_option_value(uci_option_str,ap_ssid);
				memset(uci_option_str,'\0',UCI_BUF_LEN);

				strcpy(uci_option_str,"wireless.@wifi-iface[0].key");			//name
				uci_get_option_value(uci_option_str,ap_key);
				memset(uci_option_str,'\0',UCI_BUF_LEN);

				strcpy(uci_option_str,"wireless.@wifi-iface[1].ssid");			//name
				uci_get_option_value(uci_option_str,connected_ssid);
				memset(uci_option_str,'\0',UCI_BUF_LEN);
		}

		strcpy(uci_option_str,"system.@system[0].name");			//name
		uci_get_option_value(uci_option_str,name);
		memset(uci_option_str,'\0',64);
		//p_debug("name=%s",name);
		
		power=getPower();

		/**/
		count=getOnline();

		stor storage;
		stor *sto=&storage;
		getStorage(sto);

		p_debug("total=%lld,used=%lld",sto->total,sto->used);

		if((web_str=GetStringFromWeb())==NULL)
		{
			fprintf(stdout,"can't get string from web\n");
			exit(1);
		}
		processString(web_str,SID,app_sid);
		processString(web_str,"code",code);		

		
		char *pssid=NULL;
		char *pkey=NULL;

		char *ssid=NULL;
		char *key=NULL;

		//pssid=(char*)malloc(strlen(web_str));
		//pkey=(char*)malloc(strlen(web_str));
		//processString(web_str,"ssid",pssid);
		//processString(web_str,"key",pkey);

		//ssid=urlDecode(pssid);
		//key=urlDecode(pkey);
		//p_debug("ssid=%s",ssid);
		//p_debug("key=%s",key);

		getMac();
		strcpy(uci_option_str,"samba.@samba[0].guest_password");			//name
		uci_get_option_value(uci_option_str,smb_guest_pwd);
		memset(uci_option_str,'\0',UCI_BUF_LEN);
		
		strcpy(uci_option_str,"samba.@sambashare[0].read_only");			//name
		uci_get_option_value(uci_option_str,read_only);
		memset(uci_option_str,'\0',UCI_BUF_LEN);

		if(!strcmp(read_only,"no"))
			strcpy(right,"rw");
		else 
			strcpy(right,"ro");

		get_fw_version(&version_info);
		get_fw_upgrade(&version_info);
//		if(!strcmp(fw_sid,"")||(!strcmp(fw_sid,app_sid)&&(!strcmp(save_code,code))))//是管理员
		if(!strcmp(fw_sid,"")||(!strcmp(fw_sid,app_sid)))//是管理员
		{
			if(!strcmp(led_status,"3")){
				system("pwm_control 1  1 0;pwm_control 1 0 0 >/dev/null");// wifi led on
				updateSysVal("led_status","1");
			}
			sprintf(uci_option_str,"system.@system[0].sid=%s",app_sid);
			uci_set_option_value(uci_option_str);
			system("uci commit");

			if(strcmp(save_code,code))//如果是管理员，随机码，则更新
				{
					sprintf(uci_option_str,"system.@system[0].code=%s",code);
					uci_set_option_value(uci_option_str);
					memset(uci_option_str,'\0',UCI_BUF_LEN);
					system("uci commit");
			}
			
			strcpy(uci_option_str,"samba.@samba[0].password");			//name
			uci_get_option_value(uci_option_str,smb_pwd);
			memset(uci_option_str,'\0',UCI_BUF_LEN);

			sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"isAdmin\":1,\"info\":{\"SCHEME\":\"LEHE\",\"VERSION\":\"1\",\"MAC\":\"%s\",\"SSID\":\"%s\",\"PASSWORD\":\"%s\"},\"name\":\"%s\",\"power\":%d,\"count\":%d,\"ssid\":\"%s\",\"storage\":{\"total\":%lld,\"used\":%lld},\"ota\":{\"curVersion\":\"%s\",\"hasNew\":\"%s\",\"nextVersion\":\"%s\",\"nextFeature\":\"%s\",\"isForce\":\"%s\"},\"smb\":{\"adminPwd\":\"%s\",\"guestPwd\":\"%s\",\"right\":\"%s\"}}}", \
			tmp_mac,ap_ssid,ap_key,name,power,count,connected_ssid,sto->total,sto->used,version_info.curVersionName, version_info.hasNew, version_info.nextVersionName, version_info.nextFeature, version_info.isForce,smb_pwd,smb_guest_pwd,right);			
		}else if(!strcmp(save_code,code)){//有效访客
			//sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"isAdmin\":0,\"info\":{}}}");
			
			sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"isAdmin\":0,\"info\":{\"SCHEME\":\"LEHE\",\"VERSION\":\"1\",\"MAC\":\"%s\",\"SSID\":\"%s\",\"PASSWORD\":\"%s\"},\"name\":\"%s\",\"power\":%d,\"count\":%d,\"ssid\":\"%s\",\"storage\":{\"total\":%lld,\"used\":%lld},\"ota\":{\"curVersion\":\"%s\",\"hasNew\":\"%s\",\"nextVersion\":\"%s\",\"nextFeature\":\"%s\",\"isForce\":\"%s\"},\"smb\":{\"adminPwd\":\"%s\",\"guestPwd\":\"%s\",\"right\":\"%s\"}}}", \
			tmp_mac,ap_ssid,ap_key,name,power,count,connected_ssid,sto->total,sto->used,version_info.curVersionName, version_info.hasNew, version_info.nextVersionName, version_info.nextFeature, version_info.isForce,"",smb_guest_pwd,right);			
				
		}else{//无效访客
			sprintf(ret_buf,"{\"status\":0,\"errorCode\":1,\"errorMessage\":\"security error\",\"data\":{}}");
		}
		p_debug(ret_buf);
		printf("%s",ret_buf);
		fflush(stdout);
		free(web_str);
		uci_free_context(ctx);
		return ;
}

