#include "cgihandle.h"
#include "cgiWireless.h"
#include <sys/time.h>
#include <unistd.h>
#include <linux/wireless.h>
#include "msg.h"
#include "socket_uart.h" 


#include "uci_for_cgi.h"
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


int getOnline(char **ret_buf)
{

	#if 0
	char buffer[BUFSIZ]; 
	FILE *read_fp; 
	int chars_read; 
	int count=0; 
	char *ppid;
	memset( buffer, 0, BUFSIZ ); 
	char tmpStr[64];

	sprintf(tmpStr,"%s","ps |grep file_download.cgi|wc -l");

	read_fp=popen(tmpStr,"r");
	if(read_fp!=NULL)
	{
		chars_read = fread(buffer, sizeof(char), BUFSIZ-1, read_fp); 
		if (chars_read > 0){
			//p_debug("buffer==%s==",buffer);
			//count=atoi(buffer);
			count=buffer[0]-'0';
			count=count-2;
			p_debug("count=%d",count);
			pclose(read_fp);
			//return count;
		}else {
			p_debug("chars_read=%d",chars_read);
			pclose(read_fp);
			//return 0;
		}

	}else {
		p_debug("read fp error");
		//return 0;
	}

	sprintf(retstr,"<Users count=\"%d\"/>",count);
	p_debug(retstr);
	return 0;
	#endif
	int    socket_id;
	struct   iwreq wrq;
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
			sprintf(ret_buf,"{\"status\":0,\"errorCode\":3,\"errorMessage\":\"socket error!\",\"data\":{}}");
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
			sprintf(ret_buf,"{\"status\":0,\"errorCode\":3,\"errorMessage\":\"ioctl error!\",\"data\":{}}");
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
				sprintf(ret_buf,"{\"status\":0,\"errorCode\":3,\"errorMessage\":\"popen error!\",\"data\":{}}");
//				sprintf(retstr,"<Return status=\"false\">popen error!</Return>");
				return -1;
			}
		}
		else
		{
			sprintf(ret_buf,"{\"status\":0,\"errorCode\":3,\"errorMessage\":\"popen error!\",\"data\":{}}");
//			sprintf(retstr,"<Return status=\"false\">popen error!</Return>");
			return -1;
		}
	}
	else
	{
		sprintf(ret_buf,"{\"status\":0,\"errorCode\":3,\"errorMessage\":\"wifi mode error!\",\"data\":{}}");
		//sprintf(retstr,"<Return status=\"false\">wifi mode error!</Return>");
		return -1;
	}
	sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"count\":%d}}",count);
	//sprintf(retstr,"<Users count=\"%d\"></Users>",count);
	p_debug(ret_buf);
	return 0;

}

void main()
{
		char ret_buf[2048];
		char name[32]="\0";
		char app_sid[SID_LEN]="\0";
		char *web_str=NULL;


		char uci_option_str[64]="\0";
		ctx=uci_alloc_context();

		strcpy(uci_option_str,"system.@system[0].name");			//name
		uci_get_option_value(uci_option_str,name);
		memset(uci_option_str,'\0',64);

		printf("Content-type:text/plain\r\n\r\n");

		getOnline(&ret_buf);

		//sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"name\":\"%s\"}}",name);
			
		
		printf("%s",ret_buf);
		fflush(stdout);
		uci_free_context(ctx);
		return ;
}

