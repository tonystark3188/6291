#include "cgihandle.h"
#include "cgiWireless.h"
#include <sys/time.h>
#include <unistd.h>
#include "my_def.h"
#include "socket_uart.h"
#include "cfg_api.h"

// #include "auto_connect.h"
extern char *get_conf_str(char *var);

#define get_power_level_num  1
#define get_Firmware_Edition 5
#define rtl_encryp_control "/proc/rtl_encryp_control"
int error_num=0;
char error_info[1024]="\0";

typedef struct _wifilist
{
	char ssid[64];
	char encryption[32];
	char key[32];
}wifilist_t, *pwifilist_t;


enum wifi_mode
{
	M_2G=2,
	M_5G=5
};

///////////////////////////////////////////////////////
//for storage//////////////////////////////////////////
///////////////////////////////////////////////////////
static const unsigned long long G = 1024*1024*1024ull;
static const unsigned long long M = 1024*1024;
static const unsigned long long K = 1024;
static char str[20];

int getsys(xmlNodePtr root);
int setsys(xmlNodePtr root);

cgiHandle grootHandle[]=
{
	{GETSYSSTR, getsys},
	{SETSYSSTR, setsys},
};


int getssid(xmlNodePtr tag, char *retstr);
int getRemoteAP(xmlNodePtr tag, char *retstr);
int getWorkMode(xmlNodePtr tag, char *retstr);
int getFTP(xmlNodePtr tag, char *retstr);
int getSAMBA(xmlNodePtr tag, char *retstr);
int getDMS(xmlNodePtr tag, char *retstr);
int getScan(xmlNodePtr tag, char *retstr);
int getStorage(xmlNodePtr tag, char *retstr);
int getVersion(xmlNodePtr tag, char *retstr);
int getJoinWired(xmlNodePtr tag, char *retstr);
int getWebDAV(xmlNodePtr tag, char *retstr);
int getpower(xmlNodePtr tag, char *retstr);
int getClientStatus(xmlNodePtr tag, char *retstr);
int get3G(xmlNodePtr tag, char *retstr);
int getBtnReset(xmlNodePtr tag, char *retstr);
int getairplay(xmlNodePtr tag, char *retstr);

tagHandle gtaghandle_get[]=
{
	{FN_GET_SSID, getssid},
	{FN_GET_RemoteAP, getRemoteAP},
	{FN_GET_WorkMode, getWorkMode},
	{FN_GET_APList, getScan},
	{FN_GET_Power, getpower},
	{FN_GET_Storage, getStorage},
	{FN_GET_FTP, getFTP},
	{FN_GET_SAMBA, getSAMBA},
	{FN_GET_DMS, getDMS},
	{FN_GET_DDNS, NULL},
	{FN_GET_WebDAV, getWebDAV},
	{FN_GET_JoinWired,getJoinWired},
	{FN_GET_Version, getVersion},
	{FN_GET_Client_Status,getClientStatus},
	{FN_GET_3G,get3G},
	{FN_GET_BTN_RST,getBtnReset},
	{FN_SET_AIRPLAY_NAME,getairplay},
};

void printstr(char *str)
{
#if 1
	FILE *fw_fp;
	int f_size=0;
	if( (fw_fp=fopen("/tmp/1.txt","a+"))==NULL)    // write and read,binary
	{
		exit(1);
	}		
	
	f_size=fwrite(str,1,strlen(str),fw_fp);
	fwrite("\n\n", 2, 2, fw_fp);
	fclose(fw_fp);
#endif
}

int setssid(xmlNodePtr tag, char *retstr);
int setWorkMode(xmlNodePtr tag, char *retstr);
int setJoinWireless(xmlNodePtr tag, char *retstr);
int setJoinWired(xmlNodePtr tag, char *retstr);
int setFTP(xmlNodePtr tag, char *retstr);
int setSAMBA(xmlNodePtr tag, char *retstr);
int setDMS(xmlNodePtr tag, char *retstr);
int Upgrade(xmlNodePtr tag, char *retstr);
int halt(xmlNodePtr tag, char *retstr);
int TimeSync(xmlNodePtr tag, char *retstr);
int setClient(xmlNodePtr tag, char *retstr);
int set3G(xmlNodePtr tag, char *retstr);
int setairplay(xmlNodePtr tag, char *retstr);
int setiperf(xmlNodePtr tag, char *retstr);

tagHandle gtaghandle_set[]=
{
	{FN_SET_SSID, setssid},
	{FN_SET_WorkMode, setWorkMode},
	{FN_SET_JoinWireless, setJoinWireless},
	{FN_SET_JoinWired, setJoinWired},
	{FN_SET_FTP, setFTP},
	{FN_SET_SAMBA, setSAMBA},
	{FN_SET_DMS, setDMS},
	{FN_SET_DDNS, NULL},
	{FN_SET_WebDAV, NULL},
	{FN_Upgrade,Upgrade},
	{FN_HALT,halt},
	{FN_Time_Sync,TimeSync},
	{FN_SET_Client,setClient},
	{FN_SET_3G,set3G},
	{FN_SET_AIRPLAY_NAME, setairplay},
	{FN_SET_iperf,setiperf},
};


#ifdef CGI_LOG	
void cgi_log( char *string){
	FILE *fw_fp;
	int f_size=0;
	if( (fw_fp=fopen("/tmp/cgi_log.txt","ab+"))==NULL)    // write and read,binary
	{
		exit(1);
	}		
	f_size=fwrite(string,1,strlen(string),fw_fp);
	fclose(fw_fp);
	return;
}
#endif
char *xmldecode(char *xml)
{
		int i,j,k;
		char *ret;
		int f;
		char tmp[10];
		
		i = strlen(xml);
		ret = (char *)malloc(i);
		
		k=0;
		for(j=0; j<i;)
		{
				if(xml[j] != '&')
				{
					ret[k]=xml[j];
					k++;
					j++;
				}
				else
				{
					j++;
					f=0;
					while(xml[j] != ';')
					{
							tmp[f++]=xml[j];
							j++;
					}
					tmp[f]=0;
					
					if(strcmp(tmp, "lt") == 0)
					{
							ret[k]='<';
					}
					else if(strcmp(tmp, "gt") == 0)
					{
							ret[k]='>';
					}
					else if(strcmp(tmp, "amp") == 0)
					{
							ret[k]='&';
					}
					else if(strcmp(tmp, "apos") == 0)
					{
							ret[k]='\'';
					}	
					else if(strcmp(tmp, "quot") == 0)
					{
							ret[k]='\"';
					}		
					k++;
					j++;												
				}
		}		
		ret[k]=0;
		return ret;
}


#define ROOTHANDLE_NUM (sizeof(grootHandle)/sizeof(grootHandle[0]))

#define GET_TAGHANDLE_NUM (sizeof(gtaghandle_get)/sizeof(gtaghandle_get[0]))
#define SET_TAGHANDLE_NUM (sizeof(gtaghandle_set)/sizeof(gtaghandle_set[0]))


int roothandle(xmlNodePtr root, xmlNodePtr other)
{
	int i;
	int flag=-1;
	
	for(i = 0; i< ROOTHANDLE_NUM; i++)
	{
		
		if(!strcmp(root,( const xmlChar *)grootHandle[i].tag))
		{
			
			if(grootHandle[i].rootfun)
				flag = grootHandle[i].rootfun(other);
		}
		
	}

	return flag;
}


int getsys(xmlNodePtr root)
{
	int flag=0;
	int tagflag=-1;
	char *tagstr;
	xmlNodePtr curNode=root;//root->children;
	char *relstr;	
	char nodestrp[128];
	//char tmp[200];
	char *tstr;
	int j;
	
	
	relstr = (char *)malloc(8192);
	tagstr = (char *)malloc(8192);
	memset(relstr, 0, 8192);
	memset(tagstr, 0, 8192);
	xmlGetTag(root,nodestrp );
	//while(curNode != NULL)
	while(1)
	{
		int i;
		for(i = 0; i<GET_TAGHANDLE_NUM; i++)
		{
			if(!strcmp(nodestrp,( const xmlChar *)gtaghandle_get[i].tag))
			{
				if(gtaghandle_get[i].tagfun)
				{
					memset(tagstr, 0, 8192);
					tagflag += gtaghandle_get[i].tagfun(curNode, tagstr);
					strcat(relstr, tagstr);
					flag = 1;
				}
			}
		}
		tstr = strstr(curNode, nodestrp);
		j=0;
		while(tstr[j++]!='>');
		curNode = tstr+j;
		memset(nodestrp, 0, 128);
		if(xmlGetNextTag(curNode, nodestrp) == 0)
			break;
		//
		//curNode = curNode->next;
	}
	//printf("%s\n",relstr);

	
	//发送字符串
	postStr(relstr, 0,flag);
	
	free(relstr);
	free(tagstr);
	
	if(flag)
		return tagflag;
	
	return -1;

}

int setsys(xmlNodePtr root)
{
	int flag=0;
	int tagflag=0;
	char tagstr[128];
	char nodestrp[128];
	//char tmp[200];
	
	xmlNodePtr curNode=root;//root->children;

	char *relstr;
	char *tstr;
	int j;

	relstr = (char *)malloc(4096);
	memset(relstr, 0, 4096);
	//printf("Content-type:text/html\r\n\r\n");
	xmlGetTag(root,nodestrp );
	//while(curNode != NULL)
	//sprintf(stderr, "set sys:%s", root);
	
	//sprintf(tmp, "echo \"set sys:%s\n\" >> /tmp/1", root);
	//system(tmp);
	//while(1)
	{
		int i;
		for(i = 0; i<SET_TAGHANDLE_NUM; i++)
		{
			if(!strcmp(nodestrp,( const xmlChar *)gtaghandle_set[i].tag))
			{
				if(gtaghandle_set[i].tagfun)
				{
					memset(tagstr, 0, 128);
					tagflag += gtaghandle_set[i].tagfun(curNode, tagstr);
					strcat(relstr, tagstr);
					flag = 1;
				}
			}
		}
		//curNode = curNode->next;
		
		//tstr = strstr(curNode, nodestrp);
		//j=0;
		///while(tstr[j++]!='>');
		//curNode = tstr+j;
		//memset(nodestrp, 0, 128);
		////if(xmlGetNextTag(curNode, nodestrp) == 0)
		//	break;
	}

	//发送字符串
	postStr(relstr, 1,flag);

	free(relstr);
	
	
	if(flag)
		return tagflag;
	
	return -1;
	
}

static const char *human_fstype(long f_type)
{
	static const struct types {
		long type;
		const char *const fs;
	} humantypes[] = {
		{ 0xADFF,     "affs" },
		{ 0x1Cd1,     "devpts" },
		{ 0x137D,     "ext" },
		{ 0xEF51,     "ext2" },
		{ 0xEF53,     "ext2/ext3" },
		{ 0x3153464a, "jfs" },
		{ 0x58465342, "xfs" },
		{ 0xF995E849, "hpfs" },
		{ 0x9660,     "isofs" },
		{ 0x4000,     "isofs" },
		{ 0x4004,     "isofs" },
		{ 0x137F,     "minix" },
		{ 0x138F,     "minix (30 char.)" },
		{ 0x2468,     "minix v2" },
		{ 0x2478,     "minix v2 (30 char.)" },
		{ 0x4d44,     "msdos" },
		{ 0x4006,     "fat" },
		{ 0x564c,     "novell" },
		{ 0x6969,     "nfs" },
		{ 0x9fa0,     "proc" },
		{ 0x517B,     "smb" },
		{ 0x012FF7B4, "xenix" },
		{ 0x012FF7B5, "sysv4" },
		{ 0x012FF7B6, "sysv2" },
		{ 0x012FF7B7, "coh" },
		{ 0x00011954, "ufs" },
		{ 0x012FD16D, "xia" },
		{ 0x5346544e, "ntfs" },
		{ 0x1021994,  "tmpfs" },
		{ 0x52654973, "reiserfs" },
		{ 0x28cd3d45, "cramfs" },
		{ 0x7275,     "romfs" },
		{ 0x858458f6, "romfs" },
		{ 0x73717368, "squashfs" },
		{ 0x62656572, "sysfs" },
		{ 0, "UNKNOWN" }
	};

	int i;

	for (i = 0; humantypes[i].type; ++i)
		if (humantypes[i].type == f_type)
			break;
	return humantypes[i].fs;
}

char* kscale(unsigned long b, unsigned long bs)
{
	unsigned long long size = b * (unsigned long long)bs;
	if (size > G)
	{
		sprintf(str, "%0.1fGB", size/(G*1.0));
		return str;
	}
	else if (size > M)
	{
		sprintf(str, "%0.1fMB", size/(1.0*M));
		return str;
	}
	else if (size > K)
	{
		sprintf(str, "%0.1fK", size/(1.0*K));
		return str;
	}
	else
	{
		sprintf(str, "%0.1fB", size*1.0);
		return str;
	}
}



////////////////////////////////////////////////////////////
//typedef int (*TAGFUNC)(xmlNodePtr tag, char *retstr);
///////////////////////////////////////////////////////////
int getssid(xmlNodePtr tag, char *retstr)
{
	char name[32]="\0";
	char encrypt[32]="\0";
	char channel[5]="\0";
	char password[32]="\0";
	char encrypt_len[5]="\0";
	char format[8]="\0";
	char mac[32]="\0";
	char ifname[10]="\0";
	
	char uci_option_str[64]="\0";
	int i=0,j=0;
	//name="valueStr"
	//valueStr=xmlGetProp(tag,(const xmlChar*)"name");
	
	//ctx=uci_alloc_context();
	strcpy(uci_option_str,"wireless.@wifi-iface[0].ssid");  		//name
	uci_get_option_value(uci_option_str,name);
	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"wireless.@wifi-iface[0].encryption");  	//encrypt
	uci_get_option_value(uci_option_str,encrypt);
	memset(uci_option_str,'\0',64);
	if(!strcmp(encrypt,"none"))
	{
		memset(encrypt,0,strlen(encrypt));
		strcpy(encrypt,"NONE");
	}
	else if( !strcmp(encrypt,"psk2+ccmp") || !strcmp(encrypt,"psk2+tkip") || !strcmp(encrypt,"psk2+tkip+ccmp") )
	{
		memset(encrypt,0,strlen(encrypt));
		strcpy(encrypt,"WPA2");
	}
	else if( !strcmp(encrypt,"psk+ccmp") || !strcmp(encrypt,"psk+tkip") || !strcmp(encrypt,"psk+tkip+ccmp") )
	{
		memset(encrypt,0,strlen(encrypt));
		strcpy(encrypt,"WPA");
	}
	else if( !strcmp(encrypt,"mixed-psk+ccmp") || !strcmp(encrypt,"mixed-psk+tkip") || !strcmp(encrypt,"mixed-psk+tkip+ccmp") )
	{
		memset(encrypt,0,strlen(encrypt));
		strcpy(encrypt,"WPA/WPA2");
	}
	else if( !strcmp(encrypt,"wep") )
	{
		memset(encrypt,0,strlen(encrypt));
		strcpy(encrypt,"WEP");
	}
	else
		NULL;
	strcpy(ifname,"wlan0");
	cgi_get_channel(ifname, channel);  								//channel
	strcpy(uci_option_str,"wireless.@wifi-iface[0].key");  			//password
	uci_get_option_value(uci_option_str,password);
	memset(uci_option_str,'\0',64);
	// strcpy(uci_option_str,"wireless.radio0.macaddr");  				//mac
	// uci_get_option_value(uci_option_str,mac);
	// memset(uci_option_str,'\0',64);
	FILE *mac_fp=NULL;
	if((mac_fp=fopen("/etc/mac.txt","r"))!=NULL)
	{
		fread(mac,1,17,mac_fp);
		fclose(mac_fp);
	}

	
	strcpy(uci_option_str,mac);
	memset(mac,'\0',32);
	
	while(uci_option_str[i])  //去掉:
	{
		if( uci_option_str[i] == ':')
		{
			i++;
		}
		else
		{
			mac[j] = uci_option_str[i] ;
			i++;
			j++;
		}
		
	}
	memset(uci_option_str,'\0',64);
	
	sprintf(retstr,"<SSID name=\"%s\" encrypt=\"%s\" channel=\"%s\" password=\"%s\" encrypt_len=\"\" format=\"\" mac=\"%s\"></SSID>",xmlEncode(name),encrypt,channel,xmlEncode(password),mac);
	
	//uci_free_context(ctx);
	return 0;

}

int getRemoteAP(xmlNodePtr tag, char *retstr)
{
	char name[64]="\0";
	char encrypt[32]="\0";
	char channel[5]="\0";
	char password[64]="\0";
	//char status[2]="\0";
	int status=1;
	char uci_option_str[64]="\0";
	char ifname[10]="\0";
	char ip[32]="\0";
	char mask[32]="\0";
	char gateway[32]="\0";
	char dns1[32]="\0";
	char dns2[32]="\0";
	char dns[64]="\0";
	int i=0;
	int j=0;
	char *tmp;
	
	strcpy(uci_option_str,"wireless.@wifi-iface[1].ssid");  //name
	uci_get_option_value(uci_option_str,name);
	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"wireless.@wifi-iface[1].encryption");  //encrypt
	uci_get_option_value(uci_option_str,encrypt);
	memset(uci_option_str,'\0',64);
	if(!strcmp(encrypt,"none"))
	{
		memset(encrypt,0,strlen(encrypt));
		strcpy(encrypt,"NONE");
	}
	else if( !strcmp(encrypt,"psk2+ccmp") || !strcmp(encrypt,"psk2+tkip") || !strcmp(encrypt,"psk2+tkip+ccmp") )
	{
		memset(encrypt,0,strlen(encrypt));
		strcpy(encrypt,"WPA2");
	}
	else if( !strcmp(encrypt,"psk+ccmp") || !strcmp(encrypt,"psk+tkip") || !strcmp(encrypt,"psk+tkip+ccmp") )
	{
		memset(encrypt,0,strlen(encrypt));
		strcpy(encrypt,"WPA");
	}
	else if( !strcmp(encrypt,"mixed-psk+ccmp") || !strcmp(encrypt,"mixed-psk+tkip") || !strcmp(encrypt,"mixed-psk+tkip+ccmp") )
	{
		memset(encrypt,0,strlen(encrypt));
		strcpy(encrypt,"WPA/WPA2");
	}
	else if( !strcmp(encrypt,"wep") )
	{
		memset(encrypt,0,strlen(encrypt));
		strcpy(encrypt,"WEP");
	}
	else
		NULL;
	strcpy(ifname,"wlan1");
	cgi_get_channel(ifname, channel);  //channel
	strcpy(uci_option_str,"wireless.@wifi-iface[1].key");  //password
	uci_get_option_value(uci_option_str,password);
	memset(uci_option_str,'\0',64);
	
	
	system("/usr/mips/cgi-bin/script/ClientStatus.sh");
	FILE *fp_client=NULL;
	if( (fp_client=fopen("/tmp/client_is_connected","r")) != NULL )
	{
		status = 0;
		fclose(fp_client);
	}
	if(status==0){
		//ctx=uci_alloc_context();
		//uci_add_delta_path(ctx,"/tmp/state");
		//uci_set_savedir(ctx,"/tmp/state");
		
		strcpy(uci_option_str,"network.wwan.ipaddr"); 
		uci_get_option_value(uci_option_str,ip);
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wwan.netmask"); 
		uci_get_option_value(uci_option_str,mask);
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wwan.gateway"); 
		uci_get_option_value(uci_option_str,gateway);
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wwan.dns"); 
		uci_get_option_value(uci_option_str,dns);
		memset(uci_option_str,'\0',64);
		
		if( strchr(dns,' ')==NULL )
		{
			strcpy(dns1,dns);
		}
		else
		{
			while(dns[i]!=' ')
			{
				i++;
			}
			for(j=0;j<i;j++)
			{
				dns1[j]=dns[j];
			}
			for(j=0,i=i+1;i<strlen(dns);j++,i++)
			{
				dns2[j]=dns[i];
			}
		}
	}
	system("rm -f /tmp/client_is_connected");
	tmp = xmlEncode(name);
	sprintf(retstr,"<RemoteAP name=\"%s\" encrypt=\"%s\" channel=\"%s\" password=\"%s\" status=\"%d\" ip=\"%s\" mask=\"%s\" gateway=\"%s\" dns1=\"%s\" dns2=\"%s\"></RemoteAP>",tmp,encrypt,channel,xmlEncode(password),status,ip,mask,gateway,dns1,dns2);
	free(tmp);
	//uci_free_context(ctx);
	return 0;
}

char * getPIDformPS(char *iface)
{
	char buffer[BUFSIZ]; 
	FILE *read_fp; 
	int chars_read; 
	char *pid; 
	memset( buffer, 0, BUFSIZ ); 
	char tmpStr[30];
//	printf("int size=%d\n",sizeof(int));


	sprintf(tmpStr,"ps -w |grep %s",iface);
//	printf("tmpStr=%s\n",tmpStr);

	read_fp=popen(tmpStr,"r");
	if(read_fp!=NULL)
		{
		chars_read = fread(buffer, sizeof(char), BUFSIZ-1, read_fp); 
		if (chars_read > 0) 
		{ 
//			printf("buffer=%s\n",buffer);
			pid=strtok(buffer," ");
			if(pid!=NULL)
				{
//				printf("pid2=%s\n",pid);
				pclose(read_fp); 
				return pid;
			}else
				{
				pclose(read_fp); 
				return 0;
			}
		} 
		else 
		{ 
			pclose(read_fp); 
			return 0; 
		} 

	}
}

int getWorkMode(xmlNodePtr tag, char *retstr)
{
	char value[2]="\0";
//	char wan_ifname[10]="\0";
	char uci_option_str[64]="\0";
	
/*	int inet_sock;
	struct ifreq ifr;
	char *pid_num;
	char cmdStr[20]="\0";
	char wan_proto[10]="\0";
	//	ctx=uci_alloc_context();
	memset(uci_option_str,'\0',64);
	
	char buffer[BUFSIZ]; 
	FILE *read_fp; 
	int chars_read; 
	char *pid; 
	memset( buffer, 0, BUFSIZ ); 
	char tmpStr[30];
	sprintf(tmpStr,"ip route |grep default");
//	printf("tmpStr=%s\n",tmpStr);

	read_fp=popen(tmpStr,"r");
	if(read_fp!=NULL)
		{
		chars_read = fread(buffer, sizeof(char), BUFSIZ-1, read_fp); 
		if (chars_read > 0) 
		{ 
//			printf("buffer=%s\n",buffer);
			pid=strstr(buffer,"eth0");
			if(pid!=NULL)
			{
				strcpy(value,"0");//wired access mode;
			}else if((pid=strstr(buffer,"pppoe"))!=NULL){
				strcpy(value,"0");//wired access mode;
			}
			else
			{
				strcpy(value,"1");//wireless access mode;				
			}

		}else 	strcpy(value,"1");//default wireless access mode;
	}else{
			strcpy(value,"1");//default wireless access mode;
		}
		pclose(read_fp);
*/
	strcpy(uci_option_str,"network.wan.workmode"); 
	uci_get_option_value(uci_option_str,value);

	sprintf(retstr,"<WorkMode value=\"1\"></WorkMode>");
	
	//uci_free_context(ctx);
	return 0;
}

int getFTP(xmlNodePtr tag, char *retstr)
{
	char user[64]="\0";
	char password[64]="\0";
	char path[64]="ftp://pisen";
	char status[5]="\0";
	char anonymous_en[5]="\0";
	char uci_option_str[64]="\0";
	strcpy(uci_option_str,"vsftpd.@vsftpd[0].user");  //user
	uci_get_option_value(uci_option_str,user);
	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"vsftpd.@vsftpd[0].password");  //password
	uci_get_option_value(uci_option_str,password);
	memset(uci_option_str,'\0',64);
	//strcpy(uci_option_str,"vsftpd.@vsftpd[0].path");  //path
	//uci_get_option_value(uci_option_str,path);
	//memset(uci_option_str,'\0',64);
	
	strcpy(uci_option_str,"vsftpd.@vsftpd[0].enabled");  //status
	uci_get_option_value(uci_option_str,status);
	memset(uci_option_str,'\0',64);
	if(status[0]=='1')
		strcpy(status,"ON");
	else
		strcpy(status,"OFF");
	strcpy(uci_option_str,"vsftpd.@vsftpd[0].anonymous_en");  //anonymous_en
	uci_get_option_value(uci_option_str,anonymous_en);
	memset(uci_option_str,'\0',64);
	if(anonymous_en[0]=='1')
		strcpy(anonymous_en,"ON");
	else
		strcpy(anonymous_en,"OFF");
	sprintf(retstr,"<FTP user=\"%s\" password=\"%s\" port=\"21\" path=\"%s\" status=\"%s\" anonymous_en=\"%s\" enable=\"OFF\"></FTP>",user,xmlEncode(password),path,status,anonymous_en);
	return 0;
}

int getSAMBA(xmlNodePtr tag, char *retstr)
{
	char user[64]="\0";
	char password[64]="\0";
	char anonymous_en[5]="\0";
	char path[32]="\\\\";
	char status[5]="\0";
	char uci_option_str[64]="\0";
	#if 1
	strcpy(uci_option_str,"samba.@samba[0].user");  //user
	uci_get_option_value(uci_option_str,user);
	#else
	strcpy(uci_option_str,"samba.@sambashare[0].users");  //user
	uci_get_option_value(uci_option_str,user);
	#endif
	
	//path
	strcpy(uci_option_str,"samba.@sambashare[0].name");  //user
	uci_get_option_value(uci_option_str,path+strlen(path));


	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"samba.@samba[0].password");  //password
	uci_get_option_value(uci_option_str,password);
	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"samba.@sambashare[0].guest_ok");  //anonymous_en
	uci_get_option_value(uci_option_str,anonymous_en);
	memset(uci_option_str,'\0',64);
	//strcpy(uci_option_str,"samba.@sambashare[0].path");  //path
	//uci_get_option_value(uci_option_str,path);
	//memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"samba.@samba[0].enabled");  //status
	uci_get_option_value(uci_option_str,status);
	memset(uci_option_str,'\0',64);
	
	if(status[0]=='1')
		strcpy(status,"ON");
	else
		strcpy(status,"OFF");
	if(!strcmp(anonymous_en,"yes"))
		strcpy(anonymous_en,"ON");
	else
		strcpy(anonymous_en,"OFF");
	sprintf(retstr,
	"<SAMBA user=\"%s\" password=\"%s\" port=\"137\" path=\"%s\" status=\"%s\" anonymous_en=\"%s\" enable=\"ON\"></SAMBA>",
	user,xmlEncode(password),path,status,anonymous_en);
	return 0;
}

int getDMS(xmlNodePtr tag, char *retstr)
{
	char name[64]="\0";
	char status[5]="\0";
	char path[32]="192.168.222.254";
	char uci_option_str[64]="\0";
	strcpy(uci_option_str,"ushare.@ushare[0].servername");  //name
	uci_get_option_value(uci_option_str,name);
	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"ushare.@ushare[0].enabled");  //status
	uci_get_option_value(uci_option_str,status);
	memset(uci_option_str,'\0',64);
	if(status[0]=='1')
		strcpy(status,"ON");
	else
		strcpy(status,"OFF");
	//strcpy(uci_option_str,"ushare.@ushare[0].content_directories");  //path
	//uci_get_option_value(uci_option_str,path);
	//memset(uci_option_str,'\0',64);
	sprintf(retstr,"<DMS name=\"%s\" status=\"%s\" path=\"%s\" enable=\"ON\"></DMS>",name,status,path);
	return 0;
}

int getWebDAV(xmlNodePtr tag, char *retstr)
{
	strcpy(retstr,"<WebDAV enable=\"OFF\"></WebDAV>");
	return 0;
}


int getScan(xmlNodePtr tag, char *retstr)
{
	char mode[3]="\0";
	char wlan1_disabled[3]="\0";
	char uci_option_str[64]="\0";
	//xmlDoc		   *doc = NULL;
	//xmlNodePtr     curNode;
	//xmlNodePtr	   rootnode;
	
	strcpy(uci_option_str,"powertools.@system[0].mode"); 
	uci_get_option_value(uci_option_str,mode);
	memset(uci_option_str,'\0',64);
	
	strcpy(uci_option_str,"wireless.@wifi-iface[1].disabled"); 
	uci_get_option_value(uci_option_str,wlan1_disabled);
	memset(uci_option_str,'\0',64);
	
	if( !strcmp(mode,"2") )
	{
		//return 1;
		;
	}
/*	
	if( !strcmp(wlan1_disabled,"1") )
	{
		system("uci set wireless.@wifi-iface[1].disabled=0");
		system("uci commit");
		//system("/etc/init.d/network restart");
		system("wifi down >/dev/null 2>&1");
		system("wifi >/dev/null 2>&1");
	}
*/
	//system("killall wpa_supplicant >/dev/null 2>&1");
	//system("killall udhcpc >/dev/null 2>&1");
	//system("ifconfig wlan1 up");
	
	//system("killall -9 wpa_supplicant >/dev/null 2>&1");
	
	cgi_scan("wlan0", retstr);
	
	//system("udhcpc -t 0 -i wlan1 -H pisen -b -p /var/run/dhcp-wlan1.pid -O rootpath -R >/dev/null &");
	//system("wifi >/dev/null 2>&1");
	
#if 0
	printf("Content-type:text/html\r\n\r\n");
	doc = xmlParseMemory(retstr,strlen(retstr));
	if(doc == NULL)
	{
		//WRONG
		//goto main_error;
		
		return 1;
	}
	rootnode = xmlDocGetRootElement(doc);
	

	if(rootnode == NULL)
	{
		//WRONG
		//goto main_error;
		xmlFreeDoc(doc);
		return 1;
	}
	xmlChar *ap_str;
	curNode = rootnode->xmlChildrenNode;
	while (curNode != NULL) 
	{
	    if ((!strcmp(curNode->name, (const xmlChar *)"AP"))) 
	    {
		    //ap_str = xmlNodeListGetString(doc, curNode->xmlChildrenNode, 1);
		    //strcpy(ap_str,xmlGetProp(curNode,(const xmlChar*)"name"));
		    ap_str=xmlGetProp(curNode,(const xmlChar*)"name");
		    if( !strlen(ap_str) )
		    {
		    	xmlNodePtr tempNode;
		    	tempNode=curNode->next;
		    	xmlUnlinkNode(curNode);
		    	xmlFreeNode(curNode);
		    	curNode=tempNode;
		    	xmlFree(ap_str);
		    	continue;
		    }
		    //printf("ap_str: %s\n", ap_str);
		    //memset(ap_str,0,32);
		    xmlFree(ap_str);
		    
 	    }
		curNode = curNode->next;
	}
	memset(retstr,0,4096);
	//xmlSaveFile("-",doc);
	printf("%s",doc->_private);
	xmlFreeDoc(doc);
	//printf("%s",retstr);
#endif
	
	return 0;
	
}

int getStorage(xmlNodePtr tag, char *retstr)
{
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
	  strcpy(retstr,"<Storage>");
	  
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
	      sprintf(tmpstr,"<Section volume=\"%s\"",mount_point+9);//   /tmp/mnt/
	      //if (printf("\n<Section volume=\"%s\"" + 1, device) > 20)
	      //    printf("%1s", "");
	      strcat(retstr,tmpstr);
	      memset(tmpstr,0,128);
	      for (j = 0; j < i; j++)
	      {

	        if(!strcmp(myusb[j].devpath,device))
	        {

	        //  printf("j=%d,myusbdevice=%s,device=%s\n",j,p[j].devpath,device);
	          sprintf(tmpstr," vid=\"%s\" pid=\"%s\"",p[j].vid,p[j].pid);
	        //  printf(" vid=\"%s\" pid=\"%s\"",p[j].vid,p[j].pid);
	        }
	      }
	      strcat(retstr,tmpstr);
	      memset(tmpstr,0,128);
	      
	      char s1[20];
	      char s2[20];
	      char s3[20];
	    //  printf("total blocks=%10d\nblock size=%10d\nfree blocks=%10d\n",fs.f_blocks,fs.f_bsize,fs.f_bfree);
	      strcpy(s1, kscale(fs.f_blocks, fs.f_bsize));
	      strcpy(s2, kscale(fs.f_blocks - fs.f_bfree, fs.f_bsize));
	      strcpy(s3, kscale(fs.f_bavail, fs.f_bsize));
	      sprintf(tmpstr," total=\"%s\" used=\"%s\" free=\"%s\" fstype=\"%s\" ></Section>",
	          s1,
	          s2,
	          s3,
	          human_fstype(fs.f_type)
	          );
	      strcat(retstr,tmpstr);
	      memset(tmpstr,0,128);
	    }
	     
	  }
	  
	  if(tmpi==0)
	  {

	    strcat(retstr,"</Storage>");
	    //strcat(retstr,"</Storage><Return status=\"false\">Disk has already mount PC !</Return>");
	    //printf("</Storage><Return status=\"false\">Empty disk !</Return></getSysInfo>");
	    return -1;
	  }
	 strcat(retstr,"</Storage>");
		
	return 0;

}

int getJoinWired(xmlNodePtr tag, char *retstr)
{
	char mode[3]="\0";
	char WiredMode[16]="\0";
	char user[32]="\0";
	char password[32]="\0";
	char ip[32]="\0";
	char mask[32]="\0";
	char gateway[32]="\0";
	char dns1[32]="\0";
	char dns2[32]="\0";
	char dns[64]="\0";
	char str_sp[64]="\0";
	char uci_option_str[64]="\0";
	int i=0;
	int j=0;
	
	
	strcpy(uci_option_str,"powertools.@system[0].mode"); 
	uci_get_option_value(uci_option_str,mode);
	memset(uci_option_str,'\0',64);
	if( mode[0] != '2' )
	{
		//return 1;
		;
	}
	strcpy(uci_option_str,"network.wan.proto"); 
	uci_get_option_value(uci_option_str,WiredMode);
	memset(uci_option_str,'\0',64);
	
	if(!strcmp(WiredMode,"dhcp"))
	{
		ctx=uci_alloc_context();
		uci_add_delta_path(ctx,"/tmp/state");
		uci_set_savedir(ctx,"/tmp/state");
		
		strcpy(uci_option_str,"network.wan.ipaddr"); 
		uci_get_option_value(uci_option_str,ip);
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.netmask"); 
		uci_get_option_value(uci_option_str,mask);
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.gateway"); 
		uci_get_option_value(uci_option_str,gateway);
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.dns"); 
		uci_get_option_value(uci_option_str,dns);
		memset(uci_option_str,'\0',64);
		
		if( strchr(dns,' ')==NULL )
		{
			strcpy(dns1,dns);
		}
		else
		{
			while(dns[i]!=' ')
			{
				i++;
			}
			for(j=0;j<i;j++)
			{
				dns1[j]=dns[j];
			}
			for(j=0,i=i+1;i<strlen(dns);j++,i++)
			{
				dns2[j]=dns[i];
			}
		}
	
		sprintf(retstr,"<JoinWired enable=\"OFF\" flag=\"0\"><DHCP enable=\"ON\" ip=\"%s\" mask=\"%s\" gateway=\"%s\" dns1=\"%s\" dns2=\"%s\"></DHCP></JoinWired>",ip,mask,gateway,dns1,dns2);
		uci_free_context(ctx);
	}
	else if(!strcmp(WiredMode,"pppoe"))
	{
		strcpy(uci_option_str,"network.wan.username"); 
		uci_get_option_value(uci_option_str,user);
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.password"); 
		uci_get_option_value(uci_option_str,password);
		memset(uci_option_str,'\0',64);
		ctx=uci_alloc_context();
		uci_add_delta_path(ctx,"/tmp/state");
		uci_set_savedir(ctx,"/tmp/state");
		
		strcpy(uci_option_str,"network.wan.ipaddr"); 
		uci_get_option_value(uci_option_str,ip);
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.netmask"); 
		uci_get_option_value(uci_option_str,mask);
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.gateway"); 
		uci_get_option_value(uci_option_str,gateway);
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.dns"); 
		uci_get_option_value(uci_option_str,dns);
		memset(uci_option_str,'\0',64);
		
		if( strchr(dns,' ')==NULL )
		{
			strcpy(dns1,dns);
		}
		else
		{
			while(dns[i]!=' ')
			{
				i++;
			}
			for(j=0;j<i;j++)
			{
				dns1[j]=dns[j];
			}
			for(j=0,i=i+1;i<strlen(dns);j++,i++)
			{
				dns2[j]=dns[i];
			}
		}
		
		sprintf(retstr,"<JoinWired enable=\"OFF\" flag=\"0\"><PPPOE enable=\"ON\" user=\"%s\" password=\"%s\"  ip=\"%s\" mask=\"255.255.255.255\" gateway=\"%s\" dns1=\"%s\" dns2=\"%s\"></PPPOE></JoinWired>",user,password,ip,gateway,dns1,dns2);
		uci_free_context(ctx);
	}
	else if(!strcmp(WiredMode,"static"))
	{
		strcpy(uci_option_str,"network.wan.ipaddr"); 
		uci_get_option_value(uci_option_str,ip);
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.netmask"); 
		uci_get_option_value(uci_option_str,mask);
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.gateway"); 
		uci_get_option_value(uci_option_str,gateway);
		memset(uci_option_str,'\0',64);
		strcpy(uci_option_str,"network.wan.dns"); 
		uci_get_option_value(uci_option_str,dns);
		memset(uci_option_str,'\0',64);
		
		if( strchr(dns,' ')==NULL )
		{
			strcpy(dns1,dns);
		}
		else
		{
			while(dns[i]!=' ')
			{
				i++;
			}
			for(j=0;j<i;j++)
			{
				dns1[j]=dns[j];
			}
			for(j=0,i=i+1;i<strlen(dns);j++,i++)
			{
				dns2[j]=dns[i];
			}
			
		}
		
		sprintf(retstr,"<JoinWired enable=\"OFF\" flag=\"0\"><StaticIP enable=\"ON\" ip=\"%s\" mask=\"%s\" gateway=\"%s\" dns1=\"%s\" dns2=\"%s\"></StaticIP></JoinWired>", ip,mask,gateway,dns1,dns2); 
		
	}
	return 0;
	
}

int getVersion(xmlNodePtr tag, char *retstr)
{
	char *version=get_conf_str("fw_version");
	int cfg_ret = 0;
	char version_flag[32] = "\0";
	if(!version)
		return 0;
	cfg_ret = get_cfg_str("version_flag",version_flag);
	if( (cfg_ret == 0) || (strcmp(version_flag,"1") == 0))
	{
		sprintf(retstr,"<Version fw1=\"%s\" fw2=\"%s-%s\"></Version>",FW_1,FW_3,version);
	}
	else
	{
		sprintf(retstr,"<Version fw1=\"%s\" fw2=\"%s-%s\"></Version>",FW_1,FW_2,version);
	}
	free(version);
	return 0;
}


int getpower(xmlNodePtr tag, char *retstr)
{
	int ret = 0;
	int run_time = 0;
	unsigned short power_percent = 0;
	unsigned short power_status = 0;
	char tmp[32];
	socket_uart_cmd_t g_uart_cmd;

	for(run_time = 0; run_time < 5; run_time++)
	{
		memset(&g_uart_cmd, 0, sizeof(socket_uart_cmd_t));
		g_uart_cmd.mode = UART_R;
		g_uart_cmd.regaddr = SOCKET_UART_GET_CHARGE_STATUS;
		ret = SocketUartClientStart(&g_uart_cmd);
		if(ret < 0)
		{
			usleep(100000);
			continue;
		}
		else
		{
			power_status = g_uart_cmd.data;
		}

		usleep(100000);
		memset(&g_uart_cmd, 0, sizeof(socket_uart_cmd_t));
		g_uart_cmd.mode = UART_R;
		g_uart_cmd.regaddr = SOCKET_UART_GET_POWER_PERCENT;
		ret = SocketUartClientStart(&g_uart_cmd);
		if(ret < 0)
		{
			usleep(100000);
			continue;
		}
		else
		{
			power_percent = g_uart_cmd.data;
		}
		
		if((power_status != POWER_CHARGING) && (power_percent <= 10))
			power_status = POWER_LOW;	
		break;
	}
	
	if(run_time == 5)
	{
		sprintf(retstr,"<Return status=\"false\">Get power level error!</Return>");
	}
	else
	{
		sprintf(retstr,"<Power percent=\"%d\" status=\"%d\"></Power>",power_percent,power_status);
	}
	return 0;
}

int getClientStatus(xmlNodePtr tag, char *retstr)
{
	char status[5]="\0";
	char str_sp[64]="\0";
	char uci_option_str[64]="\0";
	
	strcpy(uci_option_str,"wireless.@wifi-iface[1].disabled"); 
	uci_get_option_value(uci_option_str,status);
	memset(uci_option_str,'\0',64);
	
	if(status[0]=='1')
	{
		strcpy(status,"OFF");
	}
	else
	{
		strcpy(status,"ON");
	}
	
	sprintf(retstr,"<Client enable=\"%s\"></Client>",status);
}

int get3G(xmlNodePtr tag, char *retstr)
{
	char operator[32]="\0";
	char status[8]="\0";
	char apn_mode[16]="\0";
	char apn[32]="\0";
	char username[32]="\0";
	char password[32]="\0";
	char dialnumber[32]="\0";
	char uci_option_str[64]="\0";
	char ipaddr[32]="\0";
	char gateway[32]="\0";
	char netmask[32]="\0";
	int i=0;
	
	
	memset(operator,0,sizeof(operator));
	memset(apn_mode,0,sizeof(apn_mode));
	memset(apn,0,sizeof(apn));
	memset(username,0,sizeof(username));
	memset(password,0,sizeof(password));
	memset(dialnumber,0,sizeof(dialnumber));
	memset(status,0,sizeof(status));
	
	
	
	memset(uci_option_str,0,sizeof(uci_option_str));
	strcpy(uci_option_str,"3g.@3g[0].apnmode"); 
	uci_get_option_value(uci_option_str,apn_mode);
	
	memset(uci_option_str,0,sizeof(uci_option_str));
	strcpy(uci_option_str,"3g.@3g[0].apn"); 
	uci_get_option_value(uci_option_str,apn);
	memset(uci_option_str,'\0',64);
	if(!strcmp(apn,"NULL"))
		memset(apn,0,sizeof(apn));
	
	strcpy(uci_option_str,"3g.@3g[0].username"); 
	uci_get_option_value(uci_option_str,username);
	memset(uci_option_str,'\0',64);
	if(!strcmp(username,"NULL"))
		memset(username,0,sizeof(username));
	
	strcpy(uci_option_str,"3g.@3g[0].password"); 
	uci_get_option_value(uci_option_str,password);
	memset(uci_option_str,'\0',64);
	if(!strcmp(password,"NULL"))
		memset(password,0,sizeof(password));
		
	strcpy(uci_option_str,"3g.@3g[0].dialnumber"); 
	uci_get_option_value(uci_option_str,dialnumber);
	memset(uci_option_str,'\0',64);
	if(!strcmp(dialnumber,"NULL"))
		memset(dialnumber,0,sizeof(dialnumber));
	
	strcpy(uci_option_str,"3g.@3g[0].operator"); 
	uci_get_option_value(uci_option_str,operator);
	memset(uci_option_str,'\0',64);
	if(!strcmp(operator,"NULL"))
		memset(operator,0,sizeof(operator));
	
	strcpy(uci_option_str,"3g.@3g[0].status"); 
	uci_get_option_value(uci_option_str,status);
	memset(uci_option_str,'\0',64);
	
	if(!strcmp(status,"2"))
	{
		memset(uci_option_str,0,sizeof(uci_option_str));
		strcpy(uci_option_str,"3g.@3g[0].ipaddr"); 
		uci_get_option_value(uci_option_str,ipaddr);

		memset(uci_option_str,0,sizeof(uci_option_str));
		strcpy(uci_option_str,"3g.@3g[0].gateway"); 
		uci_get_option_value(uci_option_str,gateway);
		
		memset(uci_option_str,0,sizeof(uci_option_str));
		strcpy(uci_option_str,"3g.@3g[0].netmask"); 
		uci_get_option_value(uci_option_str,netmask);
	}
	
	if(!strcmp(apn_mode,"auto"))
	{
		sprintf(retstr,"<G3 enable=\"ON\" apnmode=\"%s\" apn=\"\" username=\"\" password=\"\" dialnumber=\"\" status=\"%s\" operator=\"%s\" ip=\"%s\" gateway=\"%s\" netmask=\"%s\"></G3>",
		apn_mode,status,operator,ipaddr,gateway,netmask);
	}
	else
	{
		sprintf(retstr,"<G3 enable=\"ON\" apnmode=\"%s\" apn=\"%s\" username=\"%s\" password=\"%s\" dialnumber=\"%s\" status=\"%s\" operator=\"%s\" ip=\"%s\" gateway=\"%s\" netmask=\"%s\"></G3>",
		apn_mode,apn,username,password,dialnumber,status,operator,ipaddr,gateway,netmask);
	}
	
	//for(i=0;i<sizeof(type);i++)  //小写转大写
	//	type[i]=toupper(type[i]);
	//sprintf(retstr,"<G3 enable=\"ON\" apnmode=\"auto\" apn=\"\" username=\"\" password=\"\" status=\"OFF\" operator=\"\" ></G3>");
	return 0;
}

int getBtnReset(xmlNodePtr tag, char *retstr)
{
int fd1,fd2;
	if( (fd1=fopen("/tmp/noreset","r")) == NULL )
	{
		system("touch /tmp/noreset");
	}else
	fclose(fd1);

	if( (fd2=fopen("/tmp/reset","r")) != NULL )
	{
		sprintf(retstr,"<BtnReset status=\"down\"></BtnReset>");
		fclose(fd2);
	}
	else{
		sprintf(retstr,"<BtnReset status=\"up\"></BtnReset>");
	}

	return 0;
	
}
///////////////////////////////////////////////////////////
//设置需要先检查参数是否正确，然后应答一个正确或者错误码
//////////////////////////////////////////////////////////

int setssid(xmlNodePtr tag, char *retstr)  //network restart
{
	
	char *name=NULL;
	char *encrypt=NULL;
	char *password=NULL;
	char *tkip_aes=NULL;
	char channel[5]="\0";
	char encrypt_len[5]="\0";
	char format[8]="\0";
	
	char encrypt_config[32]="\0";
	char tkip_aes_config[8]="\0";
	//xmlChar *xml_str;
	
	char str_sp[64]="\0";
	//printf("Content-type:text/html\r\n\r\n");
	char *pxml=NULL;
	char *ntmp=NULL;
	
	//name="valueStr"
	//valueStr=xmlGetProp(tag,(const xmlChar*)"name");
	printstr(tag);
	
	if((pxml=xmlGetprop(tag,(const xmlChar*)SSID_SET_name))!=NULL)
	{
		if((name=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(name,0,strlen(pxml)+1);
			ntmp = xmldecode(pxml);
			strcpy(name,ntmp);
			free(pxml);
			free(ntmp);
			pxml=NULL;
			ntmp=NULL;
		}
	}
	else
	{
		error_num++;
		strcat(error_info, PARAMETER_ERROR);
		free(name);
		free(encrypt);
		free(password);
		free(tkip_aes);
		return 1;
	}
	printstr(name);
	if((pxml=xmlGetprop(tag,(const xmlChar*)SSID_SET_encrypt))!=NULL)
	{
		if((encrypt=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(encrypt,0,strlen(pxml)+1);
			strcpy(encrypt,pxml);
			pxml=NULL;
		}
	}
	else
	{
		error_num++;
		strcat(error_info, PARAMETER_ERROR);
		free(name);
		free(encrypt);
		free(password);
		free(tkip_aes);
		return 1;
	}
	printstr(encrypt);
	if((pxml=xmlGetprop(tag,(const xmlChar*)SSID_SET_password))!=NULL)
	{
		if((password=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(password,0,strlen(pxml)+1);
			ntmp = xmldecode(pxml);
			strcpy(password,ntmp);
			free(pxml);
			free(ntmp);
			pxml=NULL;
			ntmp=NULL;
		}
	}
	// printstr(password);
	if((pxml=xmlGetprop(tag,(const xmlChar*)SSID_SET_tkip_aes))!=NULL)
	{
		if((tkip_aes=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(tkip_aes,0,strlen(pxml)+1);
			strcpy(tkip_aes,pxml);
			pxml=NULL;
		}
	}
	// printstr(tkip_aes);

	//strcpy(name,xmlGetProp(tag,(const xmlChar*)SSID_SET_name));
	//strcpy(encrypt,xmlGetProp(tag,(const xmlChar*)SSID_SET_encrypt));
	//strcpy(password,xmlGetProp(tag,(const xmlChar*)SSID_SET_password));
	//strcpy(tkip_aes,xmlGetProp(tag,(const xmlChar*)SSID_SET_tkip_aes));
	if( !strlen(name) || strlen(name)>32)
	{
		error_num++;
		strcat(error_info, ERROR_SSID);
		free(name);
		free(encrypt);
		free(password);
		free(tkip_aes);
		return 1;
	}
	if( !strcmp(encrypt,"WEP") )
	{
		if( !strlen(password) || ( strlen(password)!=5 && strlen(password)!=10 && strlen(password)!=13 && strlen(password)!=26 ) )
		{
			error_num++;
			strcat(error_info,ERROR_PASSWORD);
			free(name);
			free(encrypt);
			free(password);
			free(tkip_aes);
			return 1;
		}
	}
	if( !strcmp(encrypt,"WPA") || !strcmp(encrypt,"WPA2") || !strcmp(encrypt,"WPA/WPA2") )
	{
		if( !strlen(password) || strlen(password)<8 )
		{
			error_num++;
			strcat(error_info,ERROR_PASSWORD);
			free(name);
			free(encrypt);
			free(password);
			free(tkip_aes);
			return 1;
		}
	}
	printf("Content-type:text/html\r\n\r\n");
	printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	printf("<%s><Return status=\"true\" delay=\"20\"></Return></%s>\r\n",SETSYSSTR,SETSYSSTR);
	fflush(stdout);
	
	if(!strcmp(encrypt,"NONE"))
		strcpy(encrypt_config,"none");
	else if(!strcmp(encrypt,"WEP"))
		strcpy(encrypt_config,"wep");
	else if(!strcmp(encrypt,"WPA"))
		strcpy(encrypt_config,"psk");
	else if(!strcmp(encrypt,"WPA2"))
		strcpy(encrypt_config,"psk2");
	else if(!strcmp(encrypt,"WPA/WPA2"))
		strcpy(encrypt_config,"mixed-psk");
	else
		NULL;
	if(tkip_aes!=NULL)
	{
		if(!strcmp(tkip_aes,"tkip"))
			strcpy(tkip_aes_config,"tkip");
		else if(!strcmp(tkip_aes,"aes"))
			strcpy(tkip_aes_config,"ccmp");
		else if(!strcmp(tkip_aes,"tkip/aes"))
			strcpy(tkip_aes_config,"tkip+ccmp");
		else
			NULL;
	}
	if(strlen(name))
	{
		sprintf(str_sp,"uci set wireless.@wifi-iface[0].ssid='%s'",name);
		// uci_set_option_value(str_sp);
		system(str_sp);
		memset(str_sp,0,64);
		sprintf(str_sp,"ssid_name");
		nor_set(str_sp,name);
// //		system(str_sp);
		memset(str_sp,0,64);
	}
	printstr("1");
	if(!strcmp(encrypt_config,"none") || !strcmp(encrypt_config,"wep"))
	{
		sprintf(str_sp,"uci set wireless.@wifi-iface[0].encryption=%s",encrypt_config);
		// uci_set_option_value(str_sp);
		system(str_sp);
		memset(str_sp,0,64);
//		sprintf(str_sp,"nor set encryption=%s",encrypt_config);
//		system(str_sp);

		sprintf(str_sp,"encryption");
		nor_set(str_sp,encrypt_config);
		memset(str_sp,0,64);
	}
	else if(!strcmp(encrypt_config,"psk") || !strcmp(encrypt_config,"psk2") || !strcmp(encrypt_config,"mixed-psk"))
	{
		if(strlen(tkip_aes_config)>0)
		{
			sprintf(str_sp,"uci set wireless.@wifi-iface[0].encryption=%s+%s",encrypt_config,tkip_aes_config);
			// uci_set_option_value(str_sp);
			system(str_sp);
			memset(str_sp,0,64);
//			sprintf(str_sp,"nor set encryption=%s+%s",encrypt_config,tkip_aes_config);
//			system(str_sp);

			sprintf(str_sp,"%s+%s",encrypt_config,tkip_aes_config);
			nor_set("encryption",str_sp);

			memset(str_sp,0,64);
		}
	}
	printstr("2");
	// printstr(password);
	if(password!=NULL)
	{
		if(strlen(password))
		{
			// sprintf(str_sp,"wireless.@wifi-iface[0].key=%s",password);
			// uci_set_option_value(str_sp);
			sprintf(str_sp,"uci set wireless.@wifi-iface[0].key='%s'",password);
			system(str_sp);
			memset(str_sp,0,64);
//			sprintf(str_sp,"nor set ssid_password=%s",password);
//			system(str_sp);
			sprintf(str_sp,"ssid_password");
			nor_set(str_sp,password);
			memset(str_sp,0,64);
		}
	}
	printstr("3");
	free(name);
	free(encrypt);
	free(password);
	free(tkip_aes);
	// free(pxml);
	system("uci commit");
	
	
	system("touch /tmp/wifi_client_is_connecting");
	
	system("set_ssid.sh & >/dev/null 2>&1");
	// sleep(5);
	system("rm -f /tmp/wifi_client_is_connecting");

	printstr("over");

	
	return 0;

}

int setWorkMode(xmlNodePtr tag, char *retstr)  //reboot
{
	char *value=NULL;
	char mode[3]="\0";
	char cur_mode[3]="\0";
	char str_sp[64]="\0";
	char uci_option_str[64]="\0";
	char *pxml=NULL;
	if((pxml=xmlGetprop(tag,(const xmlChar*)WorkMode_SET_value))!=NULL)
	{
		if((value=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(value,0,strlen(pxml)+1);
			strcpy(value,pxml);
			pxml=NULL;
		}
	}
	//strcpy(value,xmlGetProp(tag,(const xmlChar*)WorkMode_SET_value));
	if(!strcmp(value,"0")) //eth0 is wan,mode 2
		strcpy(mode,"2");
	if(!strcmp(value,"1")) //eth0 is lan,mode 1
		strcpy(mode,"1");
	strcpy(uci_option_str,"powertools.@system[0].mode");  //cur_mode
	uci_get_option_value(uci_option_str,cur_mode);
	memset(uci_option_str,'\0',64);
	if(!strcmp(mode,cur_mode))  //mode=cur_mode , exit
		return 1;
	sprintf(str_sp,"/usr/mips/cgi-bin/script/ChangeNetworkMode.sh %s",mode);
	
	system(str_sp);
//	memset(str_sp,0,64);
//	sprintf(str_sp,"nor set network_mode=%s",mode);
//	system(str_sp);
	memset(str_sp,0,64);
	free(value);
	free(pxml);
	return 0;
}

static int get_wifi_mode()
{
	char uci_option_str[64]="\0";
	char mode[16]="\0";
	// ctx=uci_alloc_context();
	strcpy(uci_option_str,"wireless2.@wifi[0].mode"); 
	uci_get_option_value(uci_option_str,mode);
	memset(uci_option_str,'\0',64);
	// d_printf("wifimode=%s\n", mode);

	// uci_free_context(ctx);

	if(strcmp(mode,"2g")==0)
		return M_2G;
	else
		return M_5G;
}

static wifilist_t *get_wifi_list(int wifimode,int *number)
{
	char uci_option_str[64]="\0";
	char wifinum[16]="\0";
	int i_wifinum=0;
	int i=0;
	int j;
	// ctx=uci_alloc_context();

	if(wifimode==M_2G)
	{
		strcpy(uci_option_str,"wifilist.number2g.num"); 
		uci_get_option_value(uci_option_str,wifinum);
		memset(uci_option_str,'\0',64);
	}
	else
	{
		strcpy(uci_option_str,"wifilist.number5g.num"); 
		uci_get_option_value(uci_option_str,wifinum);
		memset(uci_option_str,'\0',64);
	}
	i_wifinum=atoi(wifinum);
	*number=i_wifinum;

	if(i_wifinum==0)
		return NULL;



	wifilist_t *wifi_list=(wifilist_t *)malloc(sizeof(wifilist_t)*i_wifinum);
	memset(wifi_list,0,sizeof(wifilist_t)*i_wifinum);
	for(i=0;i<i_wifinum;i++)
	{
		if(wifimode==M_2G)
		{
			sprintf(uci_option_str,"wifilist.@wifi2g[%d].ssid",i);
			uci_get_option_value(uci_option_str,(wifi_list+i)->ssid);
			memset(uci_option_str,'\0',64);
			sprintf(uci_option_str,"wifilist.@wifi2g[%d].encryption",i);
			uci_get_option_value(uci_option_str,(wifi_list+i)->encryption);
			memset(uci_option_str,'\0',64);
			if(strcmp((wifi_list+i)->encryption,"none")!=0)
			{
				sprintf(uci_option_str,"wifilist.@wifi2g[%d].key",i);
				uci_get_option_value(uci_option_str,(wifi_list+i)->key);
				memset(uci_option_str,'\0',64);
			}
		}
		else
		{
			sprintf(uci_option_str,"wifilist.@wifi5g[%d].ssid",i);
			uci_get_option_value(uci_option_str,(wifi_list+i)->ssid);
			memset(uci_option_str,'\0',64);
			sprintf(uci_option_str,"wifilist.@wifi5g[%d].encryption",i);
			uci_get_option_value(uci_option_str,(wifi_list+i)->encryption);
			memset(uci_option_str,'\0',64);
			if(strcmp((wifi_list+i)->encryption,"none")!=0)
			{
				sprintf(uci_option_str,"wifilist.@wifi5g[%d].key",i);
				uci_get_option_value(uci_option_str,(wifi_list+i)->key);
				memset(uci_option_str,'\0',64);
			}
		}
		//i++;
	}

	// uci_free_context(ctx);
	// for(i=0;i<i_wifinum;i++)
	// // 	d_printf("%s %s %s\n", (wifi_list+i)->ssid,(wifi_list+i)->encryption,(wifi_list+i)->key);
	// 	printstr((wifi_list+i)->ssid);
	return wifi_list;
}


int setJoinWireless(xmlNodePtr tag, char *retstr)  //wlan1 disabled=0;network restart
{

	char *name=NULL;
	char *encrypt=NULL;
	char *password=NULL;
	char *tkip_aes=NULL;
	char *channel=NULL;
	char *mac=NULL;
	int channel_num=0;
	char macaddr[18]="\0";
	
	char cur_mode[3]="\0";
	
	char encrypt_config[32]="\0";
	char tkip_aes_config[16]="\0";
	
	char str_sp[64]="\0";
	// char str_sp_wifilist[64]="\0";

	char *pxml=NULL;
	char *ttag;
	char *ntmp;

	char uci_option_str[64]="\0";
	char ssid_str[64]="\0";
	char WiredMode[10]="\0";

	strcpy(uci_option_str,"network.wan.proto");  //cur_mode
	uci_get_option_value(uci_option_str,WiredMode);
	memset(uci_option_str,'\0',64);
	
	if(strcmp(WiredMode,"dhcp"))//static ip or pppoe,reset to dhcp
	{
		system("uci delete network.wan");
		system("uci set network.wan=interface");
		system("uci set network.wan.ifname=eth0");
		system("uci set network.wan.proto=dhcp");
		system("uci set network.wan.hostname=pisen");
	}
	
	strcpy(uci_option_str,"powertools.@system[0].mode");  //cur_mode
	uci_get_option_value(uci_option_str,cur_mode);
	memset(uci_option_str,'\0',64);

	memset(str_sp,0,64);
	strcpy(str_sp,"wireless.@wifi-iface[1].disabled=0");
	uci_set_option_value(str_sp);
	memset(str_sp,0,64);
	
	ttag = strstr(tag, JoinWireless_SET_AP);

	printstr(ttag);

	if((pxml=xmlGetprop(ttag,(const xmlChar*)JoinWireless_SET_AP_name))!=NULL)
	{
		if((name=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			
			
			memset(name,0,strlen(pxml)+1);
			// ntmp = xmldecode(pxml);
			// strcpy(name,ntmp);
			// free(ntmp);
			strcpy(name,pxml);
			free(pxml);
			pxml=NULL;
			// ntmp=NULL;
		}
	}
	else
	{
		error_num++;
		strcat(error_info, PARAMETER_ERROR);
		free(name);
		free(encrypt);
		free(password);
		free(tkip_aes);
		return 1;
	}
	printstr(name);
		
	if((pxml=xmlGetprop(ttag,(const xmlChar*)JoinWireless_SET_AP_encrypt))!=NULL)
	{
		if((encrypt=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(encrypt,0,strlen(pxml)+1);
			strcpy(encrypt,pxml);
			pxml=NULL;
		}
	}
	else
	{
		error_num++;
		strcat(error_info, PARAMETER_ERROR);
		free(name);
		free(encrypt);
		free(password);
		free(tkip_aes);
		return 1;
	}
	if((pxml=xmlGetprop(ttag,(const xmlChar*)JoinWireless_SET_AP_mac))!=NULL)
	{
		if((mac=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(mac,0,strlen(pxml)+1);
			strcpy(mac,pxml);
			pxml=NULL;
		}
	}
	else
	{
		error_num++;
		strcat(error_info, PARAMETER_ERROR);
		free(name);
		free(encrypt);
		free(password);
		free(tkip_aes);
		return 1;
	}
	// printstr(encrypt);
	
	//strcpy(name,xmlGetProp(tag->children,(const xmlChar*)JoinWireless_SET_AP_name));
	//strcpy(encrypt,xmlGetProp(tag->children,(const xmlChar*)JoinWireless_SET_AP_encrypt));
	/************************需要修改************************************/
	if( strcmp(encrypt,"NONE") )
	{
		if((pxml=xmlGetprop(ttag,(const xmlChar*)JoinWireless_SET_AP_password))!=NULL)
		{
			if((password=(char *)malloc(strlen(pxml)+1))!=NULL)
			{
				memset(password,0,strlen(pxml)+1);
				// ntmp = xmldecode(pxml);
				// strcpy(password,ntmp);
				strcpy(password,pxml);
				free(pxml);
				// free(ntmp);
				pxml=NULL;
				// ntmp=NULL;
			}
		}
	}
	printstr(password);
		//strcpy(password,xmlGetProp(tag->children,(const xmlChar*)JoinWireless_SET_AP_password));
	if( !strcmp(encrypt,"WPA-PSK") || !strcmp(encrypt,"WPA2-PSK") || !strcmp(encrypt,"WPA/WPA2-PSK") )
	{
		if((pxml=xmlGetprop(ttag,(const xmlChar*)JoinWireless_SET_AP_tkip_aes))!=NULL)
		{
			if((tkip_aes=(char *)malloc(strlen(pxml)+1))!=NULL)
			{
				memset(tkip_aes,0,strlen(pxml)+1);
				strcpy(tkip_aes,pxml);
				pxml=NULL;
			}
		}
		
	}
		//strcpy(tkip_aes,xmlGetProp(tag->children,(const xmlChar*)JoinWireless_SET_AP_tkip_aes));
	

	if( !strlen(name) || strlen(name)>32)
	{
		error_num++;
		strcat(error_info, ERROR_SSID);
		free(name);
		free(encrypt);
		free(password);
		free(tkip_aes);
		return 1;
	}
/*
	if( !strcmp(encrypt,"WEP") )
	{
		if( !strlen(password) || ( strlen(password)!=5 && strlen(password)!=10 && strlen(password)!=13 && strlen(password)!=26 ) )
		{
			error_num++;
			strcat(error_info,ERROR_PASSWORD);
			return 1;
		}
	}
*/
	if( !strcmp(encrypt,"WPA-PSK") || !strcmp(encrypt,"WPA2-PSK") || !strcmp(encrypt,"WPA/WPA2-PSK") )
	{
		if( !strlen(password) || strlen(password)<8 )
		{
			error_num++;
			strcat(error_info,ERROR_PASSWORD);
			free(name);
			free(encrypt);
			free(password);
			free(tkip_aes);
			return 1;
		}
	}
	if((pxml=xmlGetprop(ttag,(const xmlChar*)JoinWireless_SET_AP_channel))!=NULL)
	{
		if((channel=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(channel,0,strlen(pxml)+1);
			strcpy(channel,pxml);
			pxml=NULL;
		}
	}

/* feng 	
	if(cur_mode[0]!='1')//cur_mode=1 wireless mode
	{
		sprintf(str_sp,"/usr/mips/cgi-bin/script/ChangeNetworkMode.sh 1");
		system(str_sp);
	//	memset(str_sp,0,64);
	//	sprintf(str_sp,"nor set network_mode=%s","1");
	//	system(str_sp);
		memset(str_sp,0,64);
		printf("Content-type:text/html\r\n\r\n");
		printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
		printf("<%s><Return status=\"true\" delay=\"30\"></Return></%s>\r\n",SETSYSSTR,SETSYSSTR);
		fflush(stdout);
	}
	else
*/	
	
	printf("Content-type:text/html\r\n\r\n");
	printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	printf("<%s><Return status=\"true\" delay=\"5\"></Return></%s>\r\n",SETSYSSTR,SETSYSSTR);
	fflush(stdout);

	if(encrypt!=NULL)
	{
		if(!strcmp(encrypt,"NONE"))
			strcpy(encrypt_config,"none");
		else if(!strcmp(encrypt,"WEP"))
			strcpy(encrypt_config,"wep");
		else if(!strcmp(encrypt,"WPA-PSK"))
			strcpy(encrypt_config,"psk");
		else if(!strcmp(encrypt,"WPA2-PSK"))
			strcpy(encrypt_config,"psk2");
		else if(!strcmp(encrypt,"WPA/WPA2-PSK"))
			strcpy(encrypt_config,"mixed-psk");
		else
			NULL;
	}
	
	if(tkip_aes!=NULL)
	{
		if(!strcmp(tkip_aes,"tkip"))
			strcpy(tkip_aes_config,"tkip");
		else if(!strcmp(tkip_aes,"aes"))
			strcpy(tkip_aes_config,"ccmp");
		else if(!strcmp(tkip_aes,"tkip/aes"))
			strcpy(tkip_aes_config,"tkip+ccmp");
		else
			NULL;
	}

	if(strlen(name))
	{
		
		sprintf(str_sp,"wireless.@wifi-iface[1].ssid=%s",name);
		
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);
		memcpy(ssid_str,name,strlen(name));
		// printstr(name);
		
	}

// bc:d1:77:c2:d3:f4
	if(mac!=NULL)
	{
		
		memset(macaddr,0,sizeof(macaddr));
		macaddr[0]=*mac;macaddr[1]=*(mac+1);macaddr[2]=':';
		macaddr[3]=*(mac+2);macaddr[4]=*(mac+3);macaddr[5]=':';
		macaddr[6]=*(mac+4);macaddr[7]=*(mac+5);macaddr[8]=':';
		macaddr[9]=*(mac+6);macaddr[10]=*(mac+7);macaddr[11]=':';
		macaddr[12]=*(mac+8);macaddr[13]=*(mac+9);macaddr[14]=':';
		macaddr[15]=*(mac+10);macaddr[16]=*(mac+11);
		sprintf(str_sp,"wireless.@wifi-iface[1].bssid=%s",macaddr);
		
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);
		
	}
	// printstr("1");
	if(!strcmp(encrypt_config,"none") || !strcmp(encrypt_config,"wep"))
	{
		sprintf(str_sp,"wireless.@wifi-iface[1].encryption=%s",encrypt_config);
		
	}
	else
	{
		sprintf(str_sp,"wireless.@wifi-iface[1].encryption=%s+%s",encrypt_config,tkip_aes_config);
		
	}
	uci_set_option_value(str_sp);
	memset(str_sp,0,64);
	// printstr("2");
	
	if(password!=NULL)
	{
		if(strlen(password))
		{
			
		
			sprintf(str_sp,"wireless.@wifi-iface[1].key=%s",password);
			uci_set_option_value(str_sp);
			memset(str_sp,0,64);
			free(name);
			name=NULL;
		}
	}
	// printstr("3");

#if 0
	if(channel!=NULL)
	{
		if(strlen(channel))
		{
			sprintf(str_sp,"wireless.radio0.channel=%s",channel);
			uci_set_option_value(str_sp);
			memset(str_sp,0,64);
		}

		channel_num=atoi(channel);
		if(channel_num>7)
		{
			sprintf(str_sp,"wireless.radio0.htmode=HT40-");
			uci_set_option_value(str_sp);
			memset(str_sp,0,64);
		}
		else
		{
			sprintf(str_sp,"wireless.radio0.htmode=HT40+");
			uci_set_option_value(str_sp);
			memset(str_sp,0,64);
		}

	}
#endif


	wifilist_t *wifilist;

	int iwifimode=get_wifi_mode();
	
	int wifinumber=0;
	wifilist=get_wifi_list(iwifimode,&wifinumber);

	int i=0;
	int flag=0;


	
	
	
	
	for(i=0;i<wifinumber;i++)
	{
		if( !memcmp( (wifilist+i)->ssid, ssid_str ,strlen(ssid_str)) )
		{
			
			flag=1;
			break;
		}
	}
	
	// sprintf(str_sp,"flag=%d",flag);
	// printstr(str_sp);
	// memset(str_sp,0,64);

	// sprintf(str_sp,"iwifimode=%d",iwifimode);
	// printstr(str_sp);
	// memset(str_sp,0,64);

	if(flag)
	{
		sprintf(str_sp,"uci set wifilist.@wifi%dg[%d].ssid='%s'",iwifimode,i,ssid_str);
		//uci_set_option_value(str_sp);
		system(str_sp);
		memset(str_sp,0,64);

		if(!memcmp(encrypt_config,"none",4) || !memcmp(encrypt_config,"wep",3))
		{
			sprintf(str_sp,"uci set wifilist.@wifi%dg[%d].encryption=%s",iwifimode,i,encrypt_config);
		}
		else
		{
			sprintf(str_sp,"uci set wifilist.@wifi%dg[%d].encryption=%s+%s",iwifimode,i,encrypt_config,tkip_aes_config);
		}
		//uci_set_option_value(str_sp);
		system(str_sp);
		memset(str_sp,0,64);
		if(memcmp(encrypt_config,"none",4)!=0)
		{
			sprintf(str_sp,"uci set wifilist.@wifi%dg[%d].key='%s'",iwifimode,i,password);
		}
		//uci_set_option_value(str_sp);
		system(str_sp);
		memset(str_sp,0,64);
		sprintf(str_sp,"uci set wifilist.@wifi%dg[%d].bssid=%s",iwifimode,i,macaddr);
		//uci_set_option_value(str_sp);
		system(str_sp);
		memset(str_sp,0,64);
		system("uci commit wifilist");

	
	}
	else
	{
		sprintf(str_sp,"uci add wifilist wifi%dg",iwifimode);
		system(str_sp);
		memset(str_sp,0,64);
		sprintf(str_sp,"uci set wifilist.@wifi%dg[-1].ssid='%s'",iwifimode,ssid_str);
		system(str_sp);
		memset(str_sp,0,64);
		sprintf(str_sp,"uci set wifilist.@wifi%dg[-1].bssid='%s'",iwifimode,macaddr);
		system(str_sp);
		memset(str_sp,0,64);
		if(!strcmp(encrypt_config,"none") || !strcmp(encrypt_config,"wep"))
		{
			sprintf(str_sp,"uci set wifilist.@wifi%dg[-1].encryption=%s",iwifimode,encrypt_config);
		}
		else
		{
			sprintf(str_sp,"uci set wifilist.@wifi%dg[-1].encryption=%s+%s",iwifimode,encrypt_config,tkip_aes_config);
		}
		system(str_sp);
		memset(str_sp,0,64);
		if(strcmp(encrypt_config,"none")!=0)
		{
			sprintf(str_sp,"uci set wifilist.@wifi%dg[-1].key='%s'",iwifimode,password);
		}
		system(str_sp);
		memset(str_sp,0,64);
		sprintf(str_sp,"uci set wifilist.number%dg.num=%d",iwifimode,wifinumber+1);
		system(str_sp);
		memset(str_sp,0,64);

		system("uci commit wifilist");


	}
	system("cp -f /etc/config/wifilist /user/wifilist");

	
	free(name);
	free(encrypt);
	free(password);
	free(tkip_aes);
	free(channel);
	free(pxml);

	free(wifilist);

	// system("uci set network.wan.workmode=1");
	// system("brctl addif br-lan eth0");
	system("uci commit");
	
	
	//system("/etc/init.d/network restart");
	
	//system("wifi down >/dev/null 2>&1");
	//if(cur_mode[0]!= '1')
// 	if(strcmp(WiredMode,"dhcp"))
// 	{
// 	//	
// 		system("sync");
// 		sleep(1);
// 	//	system("reboot");
// 		system("ifup wan");
// //		system("/etc/init.d/network restart");
// 	}
// 	else
	system("touch /tmp/wifi_client_is_connecting");
	// system("wifi >/dev/null 2>&1");
	// sleep(5);
	//system("set_client.sh >/dev/null 2>&1 &");
	system("control_dns.sh >/dev/null 2>&1 &");
	system("rm -f /tmp/wifi_client_is_connecting");
	
	return 0;
}
int setJoinWired(xmlNodePtr tag, char *retstr)  //wlan1 disabled=1;network restart
{
	char WiredMode[16]="\0";
	char *user=NULL;
	char *password=NULL;
	char *ip=NULL;
	char *mask=NULL;
	char *gateway=NULL;
	char *dns1=NULL;
	char *dns2=NULL;
	char cur_proto[16]="\0";
	char wan_hostname[32]="\0";
	
	char str_sp[64]="\0";
	char uci_option_str[64]="\0";
	char *pxml=NULL;
	char tagstr[20]="\0";
	char *p, *p1;
	int j;
	
	strcpy(uci_option_str,"network.wan.proto"); 
	uci_get_option_value(uci_option_str,cur_proto);
	memset(uci_option_str,'\0',64);
	strcpy(uci_option_str,"network.wan.hostname"); 
	uci_get_option_value(uci_option_str,wan_hostname);
	memset(uci_option_str,'\0',64);
	
//	sprintf(str_sp,"/usr/mips/cgi-bin/script/ChangeNetworkMode.sh 2");
//	system(str_sp);
	
//	memset(str_sp,0,64);
//	sprintf(str_sp,"nor set network_mode=%s","2");
//	system(str_sp);
//	memset(str_sp,0,64);
	
//	strcpy(str_sp,"wireless.@wifi-iface[1].ssid=Disabled");
//	uci_set_option_value(str_sp);
//	memset(str_sp,0,64);
	//delete the wlan1 network in wireless
	//strcpy(str_sp,"wireless.@wifi-iface[1].ssid=Disabled");
	//uci_set_option_value(str_sp);
	//memset(str_sp,0,64);
	
	printf("Content-type:text/html\r\n\r\n");
	printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	printf("<%s><Return status=\"true\" delay=\"30\"></Return></%s>\r\n",SETSYSSTR,SETSYSSTR);
	fflush(stdout);
	
	printstr(tag);
	p = strstr(tag, FN_SET_JoinWired);
	j=0;
	while(p[j++]!='>');
	p1 = p+j;
	xmlGetNextTag(p1, tagstr);	
	printstr(tagstr);
	
	if(!strcmp(tagstr,JoinWired_SET_DHCP))  //DHCP
	{
		//system("uci delete network.wan");
		//system("uci set network.wan=interface");
	//	system("uci set network.wan.ifname=eth0");
		system("uci delete network.wan.dns");
		system("uci delete network.wan.gateway");
		system("uci delete network.wan.ipaddr");
		system("uci delete network.wan.netmask");
		system("uci commit");
		system("uci set network.wan.proto=dhcp");
		//system("uci set network.wan.hostname=PISEN");
		//sprintf(str_sp,"network.wan.hostname=%s",wan_hostname);
		if(!strcmp(cur_proto,"dhcp"))
			{
			system("uci set network.wan.workmode=0");
			system("uci commit");
			system("brctl delif br-lan eth0");
			system("ifconfig wlan1 down");
			system("killall udhcpc >/dev/null 2>&1");
			system("udhcpc -t 0 -i eth0  -b -p /var/run/dhcp-eth0.pid -O rootpath -R >/dev/null &");
			goto quit;
		}
		//uci_set_option_value(str_sp);
		//memset(str_sp,0,64);
		
		sprintf(str_sp,"nor set wan_mode=%s","dhcp");
		system(str_sp);
		memset(str_sp,0,64);
	}
	else if(!strcmp(tagstr,JoinWired_SET_PPPOE))  //PPPOE
	{
		if((pxml=xmlGetprop(p1,(const xmlChar*)JoinWired_SET_PPPOE_user))!=NULL)
		{
			if((user=(char *)malloc(strlen(pxml)+1))!=NULL)
			{
				memset(user,0,strlen(pxml)+1);
				strcpy(user,pxml);
				pxml=NULL;
			}
		}
			
		if((pxml=xmlGetprop(p1,(const xmlChar*)JoinWired_SET_PPPOE_password))!=NULL)
		{
			if((password=(char *)malloc(strlen(pxml)+1))!=NULL)
			{
				memset(password,0,strlen(pxml)+1);
				strcpy(password,pxml);
				pxml=NULL;
			}
		}
		//system("uci delete network.wan");
		//system("uci commit");
	//	sprintf(str_sp,"network.wan=interface");
	//	uci_set_option_value(str_sp);
	//	sprintf(str_sp,"network.wan.ifname=eth0");
	//	uci_set_option_value(str_sp);
	//	sprintf(str_sp,"network.wan.hostname=PISEN");
	//	uci_set_option_value(str_sp);
		system("uci delete network.wan.dns");
		system("uci delete network.wan.gateway");
		system("uci delete network.wan.ipaddr");
		system("uci delete network.wan.netmask");
		system("uci commit");
		memset(str_sp,0,64);
		sprintf(str_sp,"network.wan.proto=pppoe");
		uci_set_option_value(str_sp);

		sprintf(str_sp,"network.wan.username=%s",user);
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);
		sprintf(str_sp,"network.wan.password=%s",password);
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);
			
	}
	else if(!strcmp(tagstr,JoinWired_SET_StaticIP))  //StaticIP
	{
		sprintf(str_sp,"nor set wan_mode=%s","static");
		system(str_sp);
		memset(str_sp,0,64);
		if((pxml=xmlGetprop(p1,(const xmlChar*)JoinWired_SET_StaticIP_ip))!=NULL)
		{
			if((ip=(char *)malloc(strlen(pxml)+1))!=NULL)
			{
				memset(ip,0,strlen(pxml)+1);
				strcpy(ip,pxml);
				pxml=NULL;
			}
		}
			
		if((pxml=xmlGetprop(p1,(const xmlChar*)JoinWired_SET_StaticIP_mask))!=NULL)
		{
			if((mask=(char *)malloc(strlen(pxml)+1))!=NULL)
			{
				memset(mask,0,strlen(pxml)+1);
				strcpy(mask,pxml);
				pxml=NULL;
			}
		}
			
		if((pxml=xmlGetprop(p1,(const xmlChar*)JoinWired_SET_StaticIP_gateway))!=NULL)
		{
			if((gateway=(char *)malloc(strlen(pxml)+1))!=NULL)
			{
				memset(gateway,0,strlen(pxml)+1);
				strcpy(gateway,pxml);
				pxml=NULL;
			}
		}
			
		if((pxml=xmlGetprop(p1,(const xmlChar*)JoinWired_SET_StaticIP_dns1))!=NULL)
		{
			if((dns1=(char *)malloc(strlen(pxml)+1))!=NULL)
			{
				memset(dns1,0,strlen(pxml)+1);
				strcpy(dns1,pxml);
				pxml=NULL;
			}
		}
			
		if((pxml=xmlGetprop(p1,(const xmlChar*)JoinWired_SET_StaticIP_dns2))!=NULL)
		{
			if((dns2=(char *)malloc(strlen(pxml)+1))!=NULL)
			{
				memset(dns2,0,strlen(pxml)+1);
				strcpy(dns2,pxml);
				pxml=NULL;
			}
		}
				
		
		//system("uci delete network.wan");
	//	system("uci commit");
	//	sleep(2);
//		memset(str_sp,0,64);
//		sprintf(str_sp,"network.wan=interface");
//		uci_set_option_value(str_sp);
//		memset(str_sp,0,64);
//		sprintf(str_sp,"network.wan.ifname=eth0");
	//	uci_set_option_value(str_sp);
//			memset(str_sp,0,64);
	//	sprintf(str_sp,"network.wan.hostname=PISEN");
//		uci_set_option_value(str_sp);
			memset(str_sp,0,64);
		sprintf(str_sp,"network.wan.proto=static");
		uci_set_option_value(str_sp);
		//sprintf(str_sp,"network.wan.hostname=%s",wan_hostname);
		//uci_set_option_value(str_sp);
		//memset(str_sp,0,64);
		if(ip!=NULL)
		{
			memset(str_sp,0,64);
			sprintf(str_sp,"network.wan.ipaddr=%s",ip);
			uci_set_option_value(str_sp);
			memset(str_sp,0,64);
			sprintf(str_sp,"nor set stc_ip=%s",ip);
			system(str_sp);
			memset(str_sp,0,64);
		}
		if(mask!=NULL)
		{
			memset(str_sp,0,64);
			sprintf(str_sp,"network.wan.netmask=%s",mask);
			uci_set_option_value(str_sp);
			memset(str_sp,0,64);
			sprintf(str_sp,"nor set stc_mask=%s",mask);
			system(str_sp);
			memset(str_sp,0,64);
		}
		
		if(gateway!=NULL)
		{
			memset(str_sp,0,64);
			sprintf(str_sp,"network.wan.gateway=%s",gateway);
			uci_set_option_value(str_sp);
			memset(str_sp,0,64);
			sprintf(str_sp,"nor set stc_gw=%s",gateway);
			system(str_sp);
			memset(str_sp,0,64);
		}
		if(dns1!=NULL || dns2!=NULL)
		{
			if(dns1!=NULL && dns2!=NULL)
			{
				memset(str_sp,0,64);
				sprintf(str_sp,"network.wan.dns=%s %s",dns1,dns2);
				uci_set_option_value(str_sp);
				//uci_set_option_value(str_sp);
				memset(str_sp,0,64);
				sprintf(str_sp,"nor set stc_dns1=%s",dns1);
				system(str_sp);
				memset(str_sp,0,64);
				sprintf(str_sp,"nor set stc_dns2=%s",dns2);
				system(str_sp);
				memset(str_sp,0,64);
			}
			else
			{
				if(dns1!=NULL)
				{
					memset(str_sp,0,64);
					sprintf(str_sp,"network.wan.dns=%s",dns1);
					uci_set_option_value(str_sp);
					memset(str_sp,0,64);
					sprintf(str_sp,"nor set stc_dns1=%s",dns1);
					system(str_sp);
					memset(str_sp,0,64);
				}
				if(dns2!=NULL)
				{
					memset(str_sp,0,64);
					sprintf(str_sp,"network.wan.dns=%s",dns2);
					uci_set_option_value(str_sp);
					memset(str_sp,0,64);
					sprintf(str_sp,"nor set stc_dns2=%s",dns2);
					system(str_sp);
					memset(str_sp,0,64);
				}
			}
		}
		
	}
	else
		return 1;
	free(user);
	free(password);
	free(ip);
	free(mask);
	free(gateway);
	free(dns1);
	free(dns2);
	free(pxml);
	
	system("brctl delif br-lan eth0");
	system("uci set network.wan.workmode=0");
	system("uci commit");
	system("sync");
//	system("killall wanlan");
//	sleep(1);
//	system("ifup wan");
	sleep(1);
//	system("/etc/init.d/wanlan start");
	system("reboot -f");
	//system("/etc/init.d/network restart");
quit:	
	return 0;
}
int setFTP(xmlNodePtr tag, char *retstr)
{
	char user[64]="\0";
	char *password=NULL;
	char *path=NULL;
	char *anonymous_en=NULL;
	char *enable=NULL;
	
	char anonymous_en_config[3]="\0";
	char enable_config[3]="\0";
	char cur_password[32]="\0";
	char uci_option_str[64]="\0";
	char samba_status[5]="\0";
	
	char str_sp[64]="\0";
	char *pxml=NULL;
	
	strcpy(uci_option_str,"samba.@samba[0].enabled");  //status
	uci_get_option_value(uci_option_str,samba_status);
	memset(uci_option_str,'\0',64);
	
	
	//strcpy(user,xmlGetProp(tag,(const xmlChar*)FTP_SET_user));
	if((pxml=xmlGetprop(tag,(const xmlChar*)FTP_SET_password))!=NULL)
	{
		if((password=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(password,0,strlen(pxml)+1);
			strcpy(password,pxml);
			pxml=NULL;
		}
		sprintf(str_sp,"(echo %s;echo %s) | passwd pisen >/dev/null",password,password);
		system(str_sp);
		memset(str_sp,0,64);
		
		sprintf(str_sp,"vsftpd.@vsftpd[0].password=%s",password);
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);
//		sprintf(str_sp,"nor set ftp_password=%s",password);
//		system(str_sp);
		sprintf(str_sp,"ftp_password");
		nor_set(str_sp,password);
		memset(str_sp,0,64);
	}
	if((pxml=xmlGetprop(tag,(const xmlChar*)FTP_SET_path))!=NULL)
	{
		if((path=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(path,0,strlen(pxml)+1);
			strcpy(path,pxml);
			pxml=NULL;
		}
	}
	if((pxml=xmlGetprop(tag,(const xmlChar*)FTP_SET_anonymous_en))!=NULL)
	{
		if((anonymous_en=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(anonymous_en,0,strlen(pxml)+1);
			strcpy(anonymous_en,pxml);
			pxml=NULL;
		}
		
		if(!strcmp(anonymous_en,"ON"))
			strcpy(anonymous_en_config,"1");
		else
			strcpy(anonymous_en_config,"0");
		//!strcmp(anonymous_en,"ON") ? strcpy(anonymous_en_config,"1") : strcpy(anonymous_en_config,"0");
		sprintf(str_sp,"vsftpd.@vsftpd[0].anonymous_en=%s",anonymous_en_config);
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);
//		sprintf(str_sp,"nor set ftp_anonymous_en=%s",anonymous_en_config);
//		system(str_sp);
		sprintf(str_sp,"ftp_anonymous_en");
		nor_set(str_sp,anonymous_en_config);

		memset(str_sp,0,64);
	}
	if((pxml=xmlGetprop(tag,(const xmlChar*)FTP_SET_enable))!=NULL)
	{
		if((enable=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(enable,0,strlen(pxml)+1);
			strcpy(enable,pxml);
			pxml=NULL;
		}
		
		if(!strcmp(enable,"ON") )
		{
			if(samba_status[0]=='1')
			{
				//stop samba ; disable samba
				system("/etc/init.d/samba stop >/dev/null 2>&1");
				sprintf(str_sp,"samba.@samba[0].enabled=%s","0");
				uci_set_option_value(str_sp);
				memset(str_sp,0,64);
				sprintf(str_sp,"nor set smb_enable=%s","0");
				system(str_sp);
				memset(str_sp,0,64);
			}
			strcpy(enable_config,"1");
		}
		else
		{
			strcpy(enable_config,"0");
		}
		//!strcmp(enable,"ON") ? strcpy(enable_config,"1") : strcpy(enable_config,"0");
		sprintf(str_sp,"vsftpd.@vsftpd[0].enabled=%s",enable_config);
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);
		sprintf(str_sp,"nor set ftp_enable=%s",enable_config);
		system(str_sp);
		memset(str_sp,0,64);
	}
	//strcpy(password,xmlGetProp(tag,(const xmlChar*)FTP_SET_password));
	//strcpy(path,xmlGetProp(tag,(const xmlChar*)FTP_SET_path));
	//strcpy(anonymous_en,xmlGetProp(tag,(const xmlChar*)FTP_SET_anonymous_en));
	//strcpy(enable,xmlGetProp(tag,(const xmlChar*)FTP_SET_enable));
	
	
	system("uci commit");
	if(!strcmp(enable,"ON"))
	{
		
		system("/etc/init.d/vsftpd stop >/dev/null 2>&1");
		system("/etc/init.d/vsftpd start >/dev/null 2>&1");
		
	}
	else if(!strcmp(enable,"OFF"))
	{
		system("/etc/init.d/vsftpd stop >/dev/null 2>&1");
	}
	free(password);
	free(path);
	free(anonymous_en);
	free(enable);
	free(pxml);
	
	return 0;
}
int setSAMBA(xmlNodePtr tag, char *retstr)
{
	char user[64]="\0";
	char *enable=NULL;
	char *password=NULL;
	char *anonymous_en=NULL;
	char anonymous_en_config[8]="\0";
	char enable_config[3]="\0";
	char *pxml=NULL;
	char uci_option_str[64]="\0";
	char ftp_status[5]="\0";
	
	char smb_usr_name[64]="\0";
	
	char str_sp[64]="\0";
	
	//strcpy(uci_option_str,"vsftpd.@vsftpd[0].enabled");  //status
	//uci_get_option_value(uci_option_str,ftp_status);
	//memset(uci_option_str,'\0',64);
	
	strcpy(uci_option_str,"samba.@samba[0].user");  //status
	uci_get_option_value(uci_option_str,smb_usr_name);
	memset(uci_option_str,'\0',64);

	if((pxml=xmlGetprop(tag,(const xmlChar*)SAMBA_SET_enable))!=NULL)
	{
		if((enable=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(enable,0,strlen(pxml)+1);
			strcpy(enable,pxml);
			free(pxml);

			pxml=NULL;
			
		}
		
		if(!strcmp(enable,"ON"))
		{
			#if 0
			if(ftp_status[0]=='1')
			{
				//stop ftp ; disable ftp
				system("/etc/init.d/vsftpd stop >/dev/null 2>&1");
				sprintf(str_sp,"vsftpd.@vsftpd[0].enabled=%s","0");
				uci_set_option_value(str_sp);
				memset(str_sp,0,64);
				sprintf(str_sp,"nor set ftp_enable=%s","0");
				system(str_sp);
				memset(str_sp,0,64);
			}
			#endif
			strcpy(enable_config,"1");
		}
		else
		{
			strcpy(enable_config,"0");
		}
		sprintf(str_sp,"uci set samba.@samba[0].enabled=%s",enable_config);
		// uci_set_option_value(str_sp);
		system(str_sp);
		memset(str_sp,0,64);
		sprintf(str_sp,"nor set smb_enable=%s",enable_config);
		system(str_sp);
		memset(str_sp,0,64);
	}
	
	//strcpy(user,xmlGetProp(tag,(const xmlChar*)SAMBA_SET_user));
	if((pxml=xmlGetprop(tag,(const xmlChar*)SAMBA_SET_password))!=NULL)
	{
		if((password=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(password,0,strlen(pxml)+1);
			//memcpy(password,pxml,strlen(pxml));
			strcpy(password,pxml);
			free(pxml);

			pxml=NULL;
			
		}
		
		sprintf(str_sp,"(echo %s;echo %s) | smbpasswd -s %s >/dev/null",password,password,smb_usr_name);
		system(str_sp);
		memset(str_sp,0,64);
		
		sprintf(str_sp,"uci set samba.@samba[0].password=%s",password);
		// uci_set_option_value(str_sp);
		system(str_sp);
		memset(str_sp,0,64);
		sprintf(str_sp,"nor set smb_password=%s",password);
		system(str_sp);
		memset(str_sp,0,64);
	}
	
	if( (pxml=xmlGetprop(tag,(const xmlChar*)SAMBA_SET_anonymous_en))!= NULL)
	{
		if((anonymous_en=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(anonymous_en,0,strlen(pxml)+1);
			//memcpy(anonymous_en,pxml,strlen(pxml));
			strcpy(anonymous_en,pxml);
			free(pxml);

			pxml=NULL;
			
		}
		
		if(!strcmp(anonymous_en,"ON"))
		{
			strcpy(anonymous_en_config,"yes");
			system("uci delete samba.@sambashare[0].users");
		}
		else
		{
			strcpy(anonymous_en_config,"no");
			sprintf(str_sp,"uci set samba.@sambashare[0].users=%s",smb_usr_name);
			// uci_set_option_value(str_sp);
			system(str_sp);
			memset(str_sp,0,64);
		}
		sprintf(str_sp,"uci set samba.@sambashare[0].guest_ok=%s",anonymous_en_config);
		// uci_set_option_value(str_sp);
		system(str_sp);
		memset(str_sp,0,64);
		if(!strcmp(anonymous_en_config,"yes"))
			sprintf(str_sp,"nor set smb_anonymous_en=%s","1");
		else
			sprintf(str_sp,"nor set smb_anonymous_en=%s","0");
		system(str_sp);
		memset(str_sp,0,64);
	}
	

	printf("Content-type:text/html\r\n\r\n");
	printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	printf("<%s><Return status=\"true\" ></Return></%s>\r\n",SETSYSSTR,SETSYSSTR);
	fflush(stdout);
	
	
	system("uci commit");
	if(!strcmp(enable,"ON"))
	{
		
		system("/etc/init.d/samba stop >/dev/null 2>&1");
		system("/etc/init.d/samba start >/dev/null 2>&1");
		
	}
	else if(!strcmp(enable,"OFF"))
	{
		system("/etc/init.d/samba stop >/dev/null 2>&1");
	}
	free(anonymous_en);
	free(password);
	free(enable);
	free(pxml);
	return 0;
}
int setDMS(xmlNodePtr tag, char *retstr)
{
	char *enable=NULL;
	char *name=NULL;
	char enable_config[3]="\0";
	char str_sp[64]="\0";
	char *pxml=NULL;
	
	if((pxml=xmlGetprop(tag,(const xmlChar*)DMS_SET_enable))!=NULL)
	{
		if((enable=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(enable,0,strlen(pxml)+1);
			strcpy(enable,pxml);
			pxml=NULL;
		}

		
		if(!strcmp(enable,"ON"))
		{
			strcpy(enable_config,"1");
		}
		else
		{
			strcpy(enable_config,"0");
		}
		sprintf(str_sp,"ushare.@ushare[0].enabled=%s",enable_config);
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);
		sprintf(str_sp,"nor set dms_enable=%s",enable_config);
		system(str_sp);
		memset(str_sp,0,64);
	}
	if((pxml=xmlGetprop(tag,(const xmlChar*)DMS_SET_name))!=NULL)
	{
		if((name=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(name,0,strlen(pxml)+1);
			strcpy(name,pxml);
			pxml=NULL;
		}

		
		sprintf(str_sp,"ushare.@ushare[0].servername=%s",name);
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);
//		sprintf(str_sp,"nor set dms_name=%s",name);
//		system(str_sp);
		sprintf(str_sp,"dms_name");
		nor_set(str_sp,name);

		memset(str_sp,0,64);
	}
	
	
	
	
	system("uci commit");
	if(!strcmp(enable,"ON"))
	{
		system("/etc/init.d/ushare stop >/dev/null 2>&1");
		sleep(2);
		system("/etc/init.d/ushare start >/dev/null 2>&1");
	}
	else if(!strcmp(enable,"OFF"))
	{
		system("/etc/init.d/ushare stop >/dev/null 2>&1");
	}
	free(enable);
	free(name);
	free(pxml);
	return 0;
}

int Upgrade(xmlNodePtr tag, char *retstr)
{
	int i=0;
	char cmd[64]="\0";
	char op_fw_header[32]="\0";
	char mtd_fw[64]="\0";
	char mnt_path[]="/tmp/mnt";
	struct dirent* ent = NULL;
	DIR *pDir;
	char dir[128];
	char fw_path[64];
	struct stat statbuf;
	int hasFW=0;
	FILE *fw_fp;
	memset(dir,0,sizeof(dir));
	memset(fw_path,0,sizeof(fw_path));
	if( (pDir=opendir(mnt_path))==NULL )
	{
		fprintf( stderr, "Cannot open directory:%s\n", mnt_path );
		return 1;
	}
	while( (ent=readdir(pDir))!=NULL )
	{
		snprintf( dir, 128 ,"%s/%s", mnt_path, ent->d_name );
		//printf("%s\n",dir);
		lstat(dir, &statbuf);
		if( S_ISDIR(statbuf.st_mode) )  //is a dir
		{
			if(strcmp( ".",ent->d_name) == 0 || strcmp( "..",ent->d_name) == 0) 
				continue;
			sprintf(fw_path,"%s/update.bin",dir);
			if( (fw_fp=fopen(fw_path,"rb")) != NULL )
			{
				hasFW=1;
				//strcpy(path,fw_path);
				//fclose(fw_fp);
				break;
			}
		}
	}
	
	closedir(pDir);
	
	
	if(hasFW==0)
	{
		error_num++;
		strcat(error_info, "firmware file is not existed!");
		return 1;
	}
//		error_num++;
//		strcat(error_info,fw_path);
//		return 1;

//led blink 
	int fd1;
	fd1=open("/proc/led_net_ioctl",0);
	if(fd1<0)
	{
		error_num++;
		strcat(error_info, "can't open led_net_ioctl\n");
		exit(1);
	}
	ioctl(fd1,0);
	close(fd1);
    	int fd = open("/proc/led_upgrade_ioctl", 0);
    	if(fd < 0)
    	{
		error_num++;
	  	strcat(error_info, "can't open led_upgrade_ioctl\n");
      	 	exit(1);
    	} 
	 ioctl(fd,2);
	 close(fd);

	fseek(fw_fp,4,SEEK_SET);
	fread(op_fw_header,1,7,fw_fp);

	fclose(fw_fp); 
	if( strstr(op_fw_header,"OpenWrt")==NULL )
	{
		error_num++;
		strcat(error_info, "firmware file is wrong!");
		return 1;
	}

	printf("Content-type:text/html\r\n\r\n");
	printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	printf("<%s><Return status=\"true\" delay=\"160\"></Return></%s>\r\n",SETSYSSTR,SETSYSSTR);
	fflush(stdout);
	sprintf(mtd_fw,"mtd -r write %s firmware",fw_path);
	system(mtd_fw);
	
	return 0;
	
}

int halt(xmlNodePtr tag, char *retstr)
{

	printf("Content-type:text/html\r\n\r\n");
	printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	printf("<%s><Return status=\"true\" ></Return></%s>\r\n",SETSYSSTR,SETSYSSTR);
	fflush(stdout);
	//1.关闭网络
	system("/etc/init.d/network stop");
	//2.关机
	system("halt");
	
	return 0;
}


int TimeSync(xmlNodePtr tag, char *retstr)
{
	char *value=NULL;
	char *zone=NULL;
	char *zone_tmp=NULL;
	char zone_str[32]="\0";
	char zone_cur[32]="\0";
	char str_sp[64]="\0";
	struct tm time_tm;
    struct timeval time_tv;
    time_t timep;
	struct timezone time_tz;
	int hour;
	int minutes;
	
	char *pxml=NULL;
	char *pxml1=NULL;
	char uci_option_str[64]="\0";
	memset(zone_str,0,sizeof(zone_str));
	memset(uci_option_str,0,sizeof(uci_option_str));
	memset(zone_cur,0,sizeof(zone_cur));
	
	
	strcpy(uci_option_str,"system.@system[0].timezone");  //status
	uci_get_option_value(uci_option_str,zone_cur);
	memset(uci_option_str,'\0',64);
	//Time_Sync_zone
	if((pxml=xmlGetprop(tag,(const xmlChar*)Time_Sync_zone))!=NULL && (pxml1=xmlGetprop(tag,(const xmlChar*)Time_Sync_value))!=NULL)
	{
		if((zone=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(zone,0,strlen(pxml)+1);
			strcpy(zone,pxml);
			pxml=NULL;
		}
		if((value=(char *)malloc(strlen(pxml1)+1))!=NULL)
		{
			memset(value,0,strlen(pxml1)+1);
			strcpy(value,pxml1);
			pxml1=NULL;
		}
		sscanf(value, "%d-%d-%d %d:%d:%d", &time_tm.tm_year, &time_tm.tm_mon, &time_tm.tm_mday, &time_tm.tm_hour, &time_tm.tm_min, &time_tm.tm_sec);
		time_tm.tm_year -= 1900;
		time_tm.tm_mon -= 1;
		time_tm.tm_wday = 0;
		time_tm.tm_yday = 0;
		time_tm.tm_isdst = 0;
		
		timep = mktime(&time_tm);
		time_tv.tv_sec = timep;
		time_tv.tv_usec = 0;
		time_tz.tz_dsttime=0;
		
		zone_tmp=zone;
		memcpy(zone_str,zone_tmp,3);
		zone_tmp+=3;
		
		
		if(*zone_tmp == '-')
		{
			zone_tmp++;
			strcat(zone_str,zone_tmp);
			sscanf(zone_tmp,"%d:%d",&hour,&minutes);
			time_tz.tz_minuteswest=hour*60+minutes;
			
		}
		else
		{
			zone_str[3]='-';
			zone_tmp++;
			strcat(zone_str,zone_tmp);
			sscanf(zone_tmp,"%d:%d",&hour,&minutes);
			time_tz.tz_minuteswest=hour*60+minutes;
			time_tz.tz_minuteswest=-time_tz.tz_minuteswest;
		}
		
		
		if(strcmp(zone_cur,zone_str)!=0)
		{
			sprintf(str_sp,"system.@system[0].timezone=%s",zone_str);
			uci_set_option_value(str_sp);
			memset(str_sp,0,64);
			sprintf(str_sp,"echo \"%s\">/tmp/TZ",zone_str);
			system(str_sp);
			memset(str_sp,0,64);
		}
		settimeofday(&time_tv, &time_tz);
		
	}
	
	
	system("uci commit");
	
	free(value);
	free(zone);
	free(pxml);
	free(pxml1);
	return 0;
}

int setClient(xmlNodePtr tag, char *retstr)
{
	char *enable=NULL;
	char enable_config[3]="\0";
	char str_sp[64]="\0";
	char htmode[16]="\0";
	char uci_option_str[64]="\0";
	char status[5]="\0";
	char *pxml=NULL;
	FILE *fp;
	int shutdown_client_in_system=0;
	if( (fp=fopen("/tmp/shutdown_client_in_system","r")) != NULL )
	{
		shutdown_client_in_system=1;
		fclose(fp);
		system("rm -f /tmp/shutdown_client_in_system");
	}
	
	
	strcpy(uci_option_str,"wireless.@wifi-iface[1].disabled"); 
	uci_get_option_value(uci_option_str,status);
	memset(uci_option_str,'\0',64);
	
	if((pxml=xmlGetprop(tag,(const xmlChar*)SET_Client_enable))!=NULL)
	{
		if((enable=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(enable,0,strlen(pxml)+1);
			strcpy(enable,pxml);
			pxml=NULL;
		}
		
		if(!strcmp(enable,"ON"))
		{
			strcpy(enable_config,"0");
			//strcpy(htmode,"HT20");
			if(shutdown_client_in_system)
				system("ifconfig wlan1 up");
		}
		else if(!strcmp(enable,"OFF"))
		{
			system("brctl delif br-lan eth0");	
			strcpy(enable_config,"1");
			//strcpy(htmode,"HT40+");
			system("touch /tmp/shutdown_client_in_system");
			system("ifconfig wlan1 down");
			//system("touch /tmp/manul_shutdown_client");
		}
		
		sprintf(str_sp,"wireless.@wifi-iface[1].disabled=%s",enable_config);
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);
		//sprintf(str_sp,"wireless.radio0.htmode=%s",htmode);
		//uci_set_option_value(str_sp);
		//memset(str_sp,0,64);
		system("uci commit");
		
	}
	
	if(status[0]!=enable_config[0])
	{
		if(!strcmp(enable,"ON"))
		{
			printf("Content-type:text/html\r\n\r\n");
			printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
			printf("<%s><Return status=\"true\" delay=\"20\"></Return></%s>\r\n",SETSYSSTR,SETSYSSTR);
			fflush(stdout);
	sleep(2);
			system("wifi >/dev/null 2>&1");
		}
	}
	
	free(pxml);
	free(enable);
	
	return 0;
}

int set3G(xmlNodePtr tag, char *retstr)
{
	char *apn_mode=NULL;
	char *apn=NULL;
	char *username=NULL;
	char *password=NULL;
	char *dialnumber=NULL;
	char *pxml=NULL;
	
	char str_sp[64]="\0";
	
	if((pxml=xmlGetprop(tag,(const xmlChar*)SET_3G_apnmode))!=NULL)
	{
		if((apn_mode=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(apn_mode,0,strlen(pxml)+1);
			strcpy(apn_mode,pxml);
			pxml=NULL;
		}
		sprintf(str_sp,"3g.@3g[0].apnmode=%s",apn_mode);
		uci_set_option_value(str_sp);
		memset(str_sp,0,64);
	}
	
	if((pxml=xmlGetprop(tag,(const xmlChar*)SET_3G_apn))!=NULL)
	{
		if((apn=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(apn,0,strlen(pxml)+1);
			strcpy(apn,pxml);
			pxml=NULL;
		}
		
	}
	
	if((pxml=xmlGetprop(tag,(const xmlChar*)SET_3G_username))!=NULL)
	{
		if((username=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(username,0,strlen(pxml)+1);
			strcpy(username,pxml);
			pxml=NULL;
		}
		
	}
	
	if((pxml=xmlGetprop(tag,(const xmlChar*)SET_3G_password))!=NULL)
	{
		if((password=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(password,0,strlen(pxml)+1);
			strcpy(password,pxml);
			pxml=NULL;
		}
		
	}
	//SET_3G_dialnumber
	if((pxml=xmlGetprop(tag,(const xmlChar*)SET_3G_dialnumber))!=NULL)
	{
		if((dialnumber=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(dialnumber,0,strlen(pxml)+1);
			strcpy(dialnumber,pxml);
			pxml=NULL;
		}
		
	}
	if(strcmp(apn_mode,"auto")!=0)
	{
		if(strlen(apn)!=0)
		{
			sprintf(str_sp,"3g.@3g[0].apn=%s",apn);
			uci_set_option_value(str_sp);
			memset(str_sp,0,64);
		}
		else
		{
			sprintf(str_sp,"3g.@3g[0].apn=NULL");
			uci_set_option_value(str_sp);
			memset(str_sp,0,64);
		}
		if(strlen(username)!=0)
		{
			sprintf(str_sp,"3g.@3g[0].username=%s",username);
			uci_set_option_value(str_sp);
			memset(str_sp,0,64);
		}
		else
		{
			sprintf(str_sp,"3g.@3g[0].username=NULL");
			uci_set_option_value(str_sp);
			memset(str_sp,0,64);
		}
		if(strlen(password)!=0)
		{
			sprintf(str_sp,"3g.@3g[0].password=%s",password);
			uci_set_option_value(str_sp);
			memset(str_sp,0,64);
		}
		else
		{
			sprintf(str_sp,"3g.@3g[0].password=NULL");
			uci_set_option_value(str_sp);
			memset(str_sp,0,64);
		}
		if(strlen(dialnumber)!=0)
		{
			sprintf(str_sp,"3g.@3g[0].dialnumber=%s",dialnumber);
			uci_set_option_value(str_sp);
			memset(str_sp,0,64);
		}
		else
		{
			sprintf(str_sp,"3g.@3g[0].dialnumber=NULL");
			uci_set_option_value(str_sp);
			memset(str_sp,0,64);
		}
	}

	system("uci set network.wan.workmode=2");
	system("brctl addif br-lan eth0");
	system("uci commit");
	free(apn_mode);
	free(apn);
	free(username);
	free(password);
	free(dialnumber);
	free(pxml);
	
	//system("killall pppd");
	system("killall -9 pppd");
	//sleep(2);
	system("touch /tmp/3g_reconnect");
	
	
	return 0;
}

int setairplay(xmlNodePtr tag, char *retstr)
{

	char *name;
	char *pxml=NULL;	
	char uci_option_str[128]="\0";
	
	if((pxml=xmlGetprop(tag,(const xmlChar*)AIRPLAY_SET_name))!=NULL)
	{
		if(strlen(pxml) == 0)
			goto out;
		
		if((name=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
		
				char *ntmp;
			
				memset(name,0,strlen(pxml)+1);
				ntmp = xmldecode(pxml);
				strcpy(name,ntmp);
				free(ntmp);
		}
	}
	else
	{
out:
		error_num++;
		strcat(error_info, PARAMETER_ERROR);
		free(name);
		return 1;
	}

	//set airplay name
	
	printf("Content-type:text/html\r\n\r\n");
	printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	printf("<%s><Return status=\"true\" delay=\"30\"></Return></%s>\r\n",SETSYSSTR,SETSYSSTR);
	fflush(stdout);

	memset(uci_option_str,0,64);
	sprintf(uci_option_str,"nor set airplay_name=\"%s\"",name);
	system(uci_option_str);
	
	sprintf(uci_option_str,"shair.@shairname[0].name=%s",name);
	uci_set_option_value(uci_option_str);
	
	free(name);
	system("uci commit");
	system("sync");
	system("shairport stop");
	sleep(1);
	system("shairport start");

	return 0;

	
}

int getairplay(xmlNodePtr tag, char *retstr)
{
	char name[64]="\0";
	char uci_option_str[128]="\0";
	int i=0,j=0;
	
	//ctx=uci_alloc_context();
	strcpy(uci_option_str,"shair.@shairname[0].name");  		//name
	uci_get_option_value(uci_option_str,name);

	sprintf(retstr,"<%s name=\"%s\"></%s>",FN_GET_AIRPLAY_NAME, name, FN_GET_AIRPLAY_NAME);
	
	return 0;

}

int setiperf(xmlNodePtr tag, char *retstr)
{
	char *enable=NULL;
	char *pxml=NULL;
	
	if((pxml=xmlGetprop(tag,(const xmlChar*)SET_iperf_enable))!=NULL)
	{
		if((enable=(char *)malloc(strlen(pxml)+1))!=NULL)
		{
			memset(enable,0,strlen(pxml)+1);
			strcpy(enable,pxml);
			pxml=NULL;
		}
		if(!strcmp(enable,"ON"))
		{
			system("killall iperf");
			system("iperf -s -D >/dev/null 2>&1");
		}
		if(!strcmp(enable,"OFF"))
		{
			system("killall iperf");
		}
		
	}
	
	free(enable);
	free(pxml);
	return 0;
}
