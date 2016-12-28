#include <sys/wait.h>
#include <termios.h>
#include <ctype.h>
#include "hd_wifi.h"
#include "router_defs.h"
#include "scan_ap.h"
#include "route.h"
#if defined(LINUX_X1000) || defined(OPENWRT_X1000) 
#include "iwlib.h"
#if defined(WIFI_DRIVER_NL80211)
#include "iw_nl80211.h"
#endif
#endif


#ifdef SUPPORT_OPENWRT_PLATFORM
static int change_fre_device_name(char *fre,char *device_name)
{
	if(fre==NULL||device_name==NULL)
		return -1;
	if(strcmp(fre,WIFI_2_4G)==0)
		strcpy(device_name,WIFI_DEVICE_24G);
	else if(strcmp(fre,WIFI_5G)==0)
		strcpy(device_name,WIFI_DEVICE_5G);
	else
		return -1;
	return 0;
}

static int judge_encrypt_tkip(char *security,char *encrypt,char *tkip_aes)
{
	if(!strncmp(security, encrypt_none, strlen(encrypt_none))){
		strcpy(encrypt,"NONE");
		strcpy(tkip_aes,"");
	}else if(!strncmp(security,encrypt_wep, strlen(encrypt_wep))){
		strcpy(encrypt,"WEP");
		strcpy(tkip_aes,"");
	}else if(!strncmp(security,encrypt_wpapsk_tkip_aes, strlen(encrypt_wpapsk_tkip_aes))){
		strcpy(encrypt,"WPA");
		strcpy(tkip_aes,"tkip/aes");
	}else if(!strncmp(security,encrypt_wpa2psk_tkip_aes, strlen(encrypt_wpa2psk_tkip_aes))){
		strcpy(encrypt,"WPA2");
		strcpy(tkip_aes,"tkip/aes");
	}else if(!strncmp(security,encrypt_wpa2_wpa1_psk_tkip_aes, strlen(encrypt_wpa2_wpa1_psk_tkip_aes))){
		strcpy(encrypt,"WPA/WPA2");
		strcpy(tkip_aes,"tkip/aes");
	}else if(!strncmp(security,encrypt_wpapsk_tkip, strlen(encrypt_wpapsk_tkip))){
		strcpy(encrypt,"WPA");
		strcpy(tkip_aes,"tkip");
	}else if(!strncmp(security,encrypt_wpapsk_aes, strlen(encrypt_wpapsk_aes))){
		strcpy(encrypt,"WPA");
		strcpy(tkip_aes,"aes");
	}else if(!strncmp(security,encrypt_wpa2psk_tkip, strlen(encrypt_wpa2psk_tkip))){
		strcpy(encrypt,"WPA2");
		strcpy(tkip_aes,"tkip");
	}else if(!strncmp(security,encrypt_wpa2psk_aes, strlen(encrypt_wpa2psk_aes))){
		strcpy(encrypt,"WPA2");
		strcpy(tkip_aes,"aes");
	}else if(!strncmp(security,encrypt_wpa2_wpa1_psk_tkip, strlen(encrypt_wpa2_wpa1_psk_tkip))){
		strcpy(encrypt,"WPA/WPA2");
		strcpy(tkip_aes,"tkip");
	}else if(!strncmp(security,encrypt_wpa2_wpa1_psk_aes, strlen(encrypt_wpa2_wpa1_psk_aes))){
		strcpy(encrypt,"WPA/WPA2");
		strcpy(tkip_aes,"aes");
	}
	return 0;
}

static int get_member(char *buffer,int rank,char *member)
{
	char *start;
	char *head;
	int i = 0;
	start = buffer;
	head = buffer;
	while(i <= rank){
		DMCLOG_D("head = %s",head);
		start = strchr(head,'\t');
		if(start == NULL)
		{
			DMCLOG_D("NULL");
		}
		DMCLOG_D("start = %s",start);
		i++;
		head = start + 1;
	}
	memcpy(member,head,start-head);
	DMCLOG_D("member = %s",member);
	return 0;
}

void del_space(unsigned char *tmp_buf,int leng)
{
	int i;
	for(i=leng;i>0;i--)
	{
		if(tmp_buf[i-1]==32)
		{
			tmp_buf[i-1]=0;
		}else
		{
			break;
		}
	}
}

int is_connected_ap(const char *name,char *ppasswd)
{
	int i,m=0; 
	int ret = 0;
	char uci_option_str[64]="\0";
	char sum[8]="\0";
	char mac[64]="\0"; 
	char password[64]="\0"; 
	char disabled[8] = "\0";
	#if defined(OPENWRT_MT7628)
	memset(sum,0,sizeof(sum));	
	strcpy(uci_option_str,"remoteAPlist.ap.sum");  
	uci_get_option_value(uci_option_str,sum);
	memset(uci_option_str,'\0',64);
	m=atoi(sum);
	for(i = 0;i < m; i++)
	{
		memset(disabled,0,sizeof(disabled));
		sprintf(uci_option_str,"remoteAPlist.@ap[%d].disabled",i);
		uci_get_option_value(uci_option_str,disabled);
		memset(uci_option_str,0,64);
		if(strcmp(disabled,"1") == 0)
		{
			m++;	
		}
	}

	for (i=0;i<m;i++)
	{
		memset(disabled,0,sizeof(disabled));
		sprintf(uci_option_str,"remoteAPlist.@ap[%d].disabled",i);
		uci_get_option_value(uci_option_str,disabled);
		memset(uci_option_str,0,64);
		if(strcmp(disabled,"1") == 0)
		{
			continue;	
		}
		sprintf(uci_option_str,"remoteAPlist.@ap[%d].mac",i); 
		memset(mac,0,sizeof(mac));
		uci_get_option_value(uci_option_str,mac);
		memset(uci_option_str,'\0',64);	
		if(!strcasecmp(mac,name))
		{
			sprintf(uci_option_str,"remoteAPlist.@ap[%d].key",i); 
			memset(password,0,sizeof(password));
			uci_get_option_value(uci_option_str,password);
			memset(uci_option_str,'\0',64);	
			strncpy(ppasswd,password,strlen(password));
			ret = 1;
		}
	}
	#elif defined(OPENWRT_X1000)
	wifilist_t *wifilist = NULL;
	int wifinumber = 0;
	int iwifimode = 0;
	iwifimode = get_wifi_mode();
		
	wifilist=get_wifi_list(iwifimode,&wifinumber);

	for(i = 0; i < wifinumber; i++){
		if(!strcasecmp(wifilist[i].ssid,name))
		{
			strncpy(ppasswd,wifilist[i].key,strlen(wifilist[i].key));
			ret = 1;
		}
	}

	if(wifilist != NULL)
		free(wifilist);
	#endif
	return ret;
}
#endif

#if defined(LINUX_X1000) || defined(OPENWRT_X1000) 
int cgi_get_channel(char *ifname, char *chstr)
{
	int skfd;
	
	struct iwreq	  wrq;
	struct iw_range   range;
	double		  freq;
	int 		  k;
	int 		  channel;

	if((skfd = iw_sockets_open()) < 0)
	{
		//error
		return -1;
	}

	
	if(iw_get_range_info(skfd, ifname, &range) < 0)
	{
		iw_sockets_close(skfd);
		return -1;//fprintf(stderr, "%-8.16s  no frequency information.\n\n",ifname);
	}
	else
	{
		/* Get current frequency / channel and display it */
		if(iw_get_ext(skfd, ifname, SIOCGIWFREQ, &wrq) >= 0)
		{
			freq = iw_freq2float(&(wrq.u.freq));
			sprintf(chstr, "%d", (int)freq);
			//channel = iw_freq_to_channel(freq, &range);
			//if(channel > 0)
				//sprintf(chstr, "%d", channel);
		}
	}

	iw_sockets_close(skfd);
	
	return 0;
	
}
#endif


int hasEncrypt=0;
int haswpa=0;
int encrypttype=0;

char macaddr[100];
char ssidstr[100];
char channelStr[10];
char rssi[10];
int rssidbm;

#if defined(OPENWRT_X1000)
#define is_ascii  0x11
#define is_gb2312 0x22
#define is_utf8   0x33

int is_UTF8_or_gb2312(const char* str,long length)
{
   int i;
   int nBytes=0;//UFT8可用1-6个字节编码,ASCII用一个字节
   unsigned char chr;
   int bAllAscii=1; //如果全部都是ASCII, 说明不是UTF-8
   for(i=0;i<length;i++)
   {
      chr= *(str+i);
      if( (chr&0x80) != 0 ) // 判断是否ASCII编码,如果不是,说明有可能是UTF-8,ASCII用7位编码,但用一个字节存,最高位标记为0,o0xxxxxxx
        bAllAscii= 0;
      if(nBytes==0) //如果不是ASCII码,应该是多字节符,计算字节数
      {
         if(chr>=0x80)
         {
            if(chr>=0xFC&&chr<=0xFD)
             nBytes=6;
            else if(chr>=0xF8)
             nBytes=5;
            else if(chr>=0xF0)
             nBytes=4;
            else if(chr>=0xE0)
             nBytes=3;
            else if(chr>=0xC0)
             nBytes=2;
            else
            {
              return is_gb2312;
            }
            nBytes--;
         }
      }
      else //多字节符的非首字节,应为 10xxxxxx
      {
         if( (chr&0xC0) != 0x80 )
         {
            return is_gb2312;
         }
         nBytes--;
      }
   }

   if( nBytes > 0 ) //违返规则
   {
      return is_gb2312;
   }

   if( bAllAscii ) //如果全部都是ASCII, 说明不是UTF-8
   {
      return is_ascii;
   }
   return is_utf8;
}

void changeMacStr(char *instr, char *outstr)
{
	//instr aa:bb:cc:dd:ee:ff
	//oustr aabbccddeeff
	outstr[0]=instr[0];
	outstr[1]=instr[1];
	outstr[2]=instr[3];
	outstr[3]=instr[4];
	outstr[4]=instr[6];
	outstr[5]=instr[7];
	outstr[6]=instr[9];
	outstr[7]=instr[10];
	outstr[8]=instr[12];
	outstr[9]=instr[13];
	outstr[10]=instr[15];
	outstr[11]=instr[16];
	outstr[13]=0;
}

#if defined(WIFI_DRIVER_WEXT)
/*------------------------------------------------------------------*/
/*
 * Output the link statistics, taking care of formating
 */
void
iw_get_dbm(char *		buffer,
	       int		buflen,
	       const iwqual *	qual,
	       const iwrange *	range,
	       int		has_range)
{
  int		len;


	/* Just do it...
	* The old way to detect dBm require both the range and a non-null
	* level (which confuse the test). The new way can deal with level of 0
	* because it does an explicit test on the flag. */
	if(has_range && ((qual->level != 0)
		   || (qual->updated & (IW_QUAL_DBM | IW_QUAL_RCPI))))
    {

		/* Check if the statistics are in RCPI (IEEE 802.11k) */
		if(qual->updated & IW_QUAL_RCPI)
		{
			/* Deal with signal level in RCPI */
			/* RCPI = int{(Power in dBm +110)*2} for 0dbm > Power > -110dBm */

		}
		else
		{
			/* Check if the statistics are in dBm */
			if((qual->updated & IW_QUAL_DBM)
			|| (qual->level > range->max_qual.level))
			{
				/* Deal with signal level in dBm  (absolute power measurement) */
				if(!(qual->updated & IW_QUAL_LEVEL_INVALID))
				{
					int	dblevel = qual->level;
					/* Implement a range for dBm [-192; 63] */
					if(qual->level >= 64)
						dblevel -= 0x100;
					//len = snprintf(buffer, buflen, "Signal level%c%d dBm  ",qual->updated & IW_QUAL_LEVEL_UPDATED ? '=' : ':',dblevel);
					//printf("%s\n", buffer);
					rssidbm = dblevel;
					buffer += len;
					buflen -= len;
				}

			}

		}
    }

}


/*------------------------------------------------------------------*/
/*
 * Escape non-ASCII characters from ESSID.
 * This allow us to display those weirds characters to the user.
 *
 * Source is 32 bytes max.
 * Destination buffer needs to be at least 129 bytes, will be NUL
 * terminated.
 */
void
iw_essid_escape(char *		dest,
		const char *	src,
		const int	slen)
{
  const unsigned char *	s = (const unsigned char *) src;
  const unsigned char *	e = s + slen;
  char *		d = dest;

  /* Look every character of the string */
  while(s < e)
    {
      int	isescape;

      /* Escape the escape to avoid ambiguity.
       * We do a fast path test for performance reason. Compiler will
       * optimise all that ;-) */
      if(*s == '\\')
	{
	  /* Check if we would confuse it with an escape sequence */
	  /*
	  if((e-s) > 4 && (s[1] == 'x')
	     && (isxdigit(s[2])) && (isxdigit(s[3])))
	    {
	      isescape = 1;
	    }
	  else*/
	    isescape = 0;
	}
      else
	isescape = 0;
      

      /* Is it a non-ASCII character ??? */
	  /*
      if(isescape || !isascii(*s) || iscntrl(*s))
	{

	  sprintf(d, "\\x%02X", *s);
	  d += 4;
	}
      else*/
	{
	  /* Plain ASCII, just copy */
	  *d = *s;
	  d++;
	}
      s++;
    }

  /* NUL terminate destination */
  *d = '\0';
}



/*------------------------------------------------------------------*/
/*
 * Parse, and display the results of a WPA or WPA2 IE.
 *
 */
static inline void 
iw_print_ie_wpa(unsigned char *	iebuf,
		int		buflen)
{
	int			ielen = iebuf[1] + 2;
	int			offset = 2;	/* Skip the IE id, and the length. */
	unsigned char		wpa1_oui[3] = {0x00, 0x50, 0xf2};
	unsigned char		wpa2_oui[3] = {0x00, 0x0f, 0xac};
	unsigned char *	wpa_oui;
	int			i;
	uint16_t		ver = 0;
	uint16_t		cnt = 0;

	if(ielen > buflen)
		ielen = buflen;


	switch(iebuf[0])
	{
		case 0x30:		/* WPA2 */
			/* Check if we have enough data */
			if(ielen < 4)
			{
				//iw_print_ie_unknown(iebuf, buflen);
				return;
			}

			wpa_oui = wpa2_oui;
		break;

		case 0xdd:		/* WPA or else */
			wpa_oui = wpa1_oui;

			/* Not all IEs that start with 0xdd are WPA. 
			* So check that the OUI is valid. Note : offset==2 */
			if((ielen < 8)
			|| (memcmp(&iebuf[offset], wpa_oui, 3) != 0)
			|| (iebuf[offset + 3] != 0x01))
			{
				//iw_print_ie_unknown(iebuf, buflen);
				return;
			}

			/* Skip the OUI type */
			offset += 4;
		break;

		default:
			return;
	}
  
	/* Pick version number (little endian) */
	ver = iebuf[offset] | (iebuf[offset + 1] << 8);
	offset += 2;

	if(iebuf[0] == 0xdd)
		haswpa |= WPA_FLAG;
	if(iebuf[0] == 0x30)
		haswpa |= WPA2_FLAG;

	/* From here, everything is technically optional. */

	/* Check if we are done */
	if(ielen < (offset + 4))
    {
		/* We have a short IE.  So we should assume TKIP/TKIP. */
		// printf("                        Group Cipher : TKIP\n");
		// printf("                        Pairwise Cipher : TKIP\n");
		//just TKIP
		encrypttype |= ENCRYPT_TKIP;
		return;
    }
 
	/* Next we have our group cipher. */
	if(memcmp(&iebuf[offset], wpa_oui, 3) != 0)
	{
		//printf("                        Group Cipher : Proprietary\n");
	}
	else
	{
		//printf("                        Group Cipher :");
		//iw_print_value_name(iebuf[offset+3], iw_ie_cypher_name, IW_IE_CYPHER_NUM);
		//printf("\n");
	}
	offset += 4;

	/* Check if we are done */
	if(ielen < (offset + 2))
	{
		/* We don't have a pairwise cipher, or auth method. Assume TKIP. */
		//printf("                        Pairwise Ciphers : TKIP\n");
		////just TKIP
		encrypttype |= ENCRYPT_TKIP;
		return;
	}

	/* Otherwise, we have some number of pairwise ciphers. */
	cnt = iebuf[offset] | (iebuf[offset + 1] << 8);
	offset += 2;
	//printf("                        Pairwise Ciphers (%d) :", cnt);

	if(ielen < (offset + 4*cnt))
		return;

	for(i = 0; i < cnt; i++)
	{
		if(memcmp(&iebuf[offset], wpa_oui, 3) != 0)
		{
			//printf(" Proprietary");
		}
		else
		{
			//encryptmode
			if(iebuf[offset+3] == 0)
			{
				//none
			}
			else if(iebuf[offset+3] == 1)
			{
				//wep-40
				encrypttype |= ENCRYPT_WEP_40;
			}
			else if(iebuf[offset+3] == 2)
			{
				//tkip
				encrypttype |= ENCRYPT_TKIP;
			}
			else if(iebuf[offset+3] == 3)
			{
				//wrap
				encrypttype |= ENCRYPT_WRAP;
			}
			else if(iebuf[offset+3] == 4)
			{
				//ccmp
				encrypttype |= ENCRYPT_CCMP;
			}
			else if(iebuf[offset+3] == 5)
			{
				//wep-104
				encrypttype |= ENCRYPT_WEP_104;
			}			
		}
		offset+=4;
	}
 
	/* Check if we are done */
	if(ielen < (offset + 2))
		return;

	/* Now, we have authentication suites. */
	cnt = iebuf[offset] | (iebuf[offset + 1] << 8);
	offset += 2;
	//printf("                        Authentication Suites (%d) :", cnt);

	if(ielen < (offset + 4*cnt))
		return;

	for(i = 0; i < cnt; i++)
	{
		if(memcmp(&iebuf[offset], wpa_oui, 3) != 0)
		{
			//printf(" Proprietary");
		}
		else
		{

			if(iebuf[offset+3] == 1)
			{
				//802.1x
				//wpa-1x or wpa2-1x
				haswpa |= WPA1X_FLAG;
			}
			else if(iebuf[offset+3] == 2)
			{
				//psk
				//wpa-psk or wpa2-psk
			}
			else
			{
				//none
			}
		}
		offset+=4;
	}
 
	/* Check if we are done */
	if(ielen < (offset + 1))
		return;

	/* Otherwise, we have capabilities bytes.
	* For now, we only care about preauth which is in bit position 1 of the
	* first byte.  (But, preauth with WPA version 1 isn't supposed to be 
	* allowed.) 8-) */
	if(iebuf[offset] & 0x01)
	{
		//printf("                       Preauthentication Supported\n");
	}
}


/*------------------------------------------------------------------*/
/*
 * Process a generic IE and display the info in human readable form
 * for some of the most interesting ones.
 * For now, we only decode the WPA IEs.
 */
static inline void
iw_print_gen_ie(unsigned char *	buffer,
		int		buflen)
{
  int offset = 0;

  /* Loop on each IE, each IE is minimum 2 bytes */
  while(offset <= (buflen - 2))
    {
      /* Check IE type */
      switch(buffer[offset])
		{
		case 0xdd:	/* WPA1 (and other) */
		case 0x30:	/* WPA2 */
		  iw_print_ie_wpa(buffer + offset, buflen);
		  break;
		default:
		  break;
		}
      /* Skip over this IE to the next one in the list. */
      offset += buffer[offset+1] + 2;
    }
}


/*------------------------------------------------------------------*/
/*
 * Print one element from the scanning results
 */
static inline void
print_scanning_token(struct stream_descr *	stream,	/* Stream of events */
		     struct iw_event *		event,	/* Extracted token */
		     struct iwscan_state *	state,
		     struct iw_range *	iw_range,	/* Range info */
		     int		has_range, ap_list_info_t *ap_list_info)
{
	char		buffer[128];	/* Temporary buffer */
	char tmp[10];
	/* Now, let's decode the event */
	switch(event->cmd)
    {
    case SIOCGIWAP:
		//mac address
		if(state->ap_num != 1)			
		{
			//formatScanStr(str);
			formatScanList(ap_list_info);
			hasEncrypt = 0;
			haswpa = 0;
			encrypttype = 0;
		}
		
		iw_saether_ntop(&event->u.ap_addr, buffer);
		changeMacStr(buffer, macaddr);
		
		state->ap_num++;
		break;
    case SIOCGIWNWID:
		break;
    case SIOCGIWFREQ:
		{
			double		freq;			/* Frequency/channel */
			int		channel = -1;		/* Converted to channel */
			freq = iw_freq2float(&(event->u.freq));
			channel = iw_freq_to_channel(freq, iw_range);
			sprintf(channelStr, "%d", channel);
		}
		break;
	case SIOCGIWMODE:
		break;
	case SIOCGIWNAME:
		break;
	case SIOCGIWESSID:
		{
			char essid[4*IW_ESSID_MAX_SIZE+1];
			memset(essid, '\0', sizeof(essid));
			if((event->u.essid.pointer) && (event->u.essid.length))
				iw_essid_escape(essid,event->u.essid.pointer, event->u.essid.length);
			
			if(event->u.essid.flags)
			{
				#if 0
				/* Does it have an ESSID index ? */
				if((event->u.essid.flags & IW_ENCODE_INDEX) > 1)
					printf("                    ESSID:\"%s\" [%d]\n", essid,(event->u.essid.flags & IW_ENCODE_INDEX));
				else
					printf("                    ESSID:\"%s\"\n", essid);
				#endif
				
				strcpy(ssidstr, essid);
			}
			else
			{
				ssidstr[0]=0;
			}
		}
		break;
    case SIOCGIWENCODE:
		{
			unsigned char	key[IW_ENCODING_TOKEN_MAX];
			if(event->u.data.pointer)
			  memcpy(key, event->u.data.pointer, event->u.data.length);
			else
			  event->u.data.flags |= IW_ENCODE_NOKEY;
			//printf("                    Encryption key:");
			if(event->u.data.flags & IW_ENCODE_DISABLED)
			{
				//encrypt = none;
			}
			else
			{
				hasEncrypt = 1;
			}
		}
		break;
    case SIOCGIWRATE:
		break;
    case SIOCGIWMODUL:
      break;
    case IWEVQUAL:
	//signal level
      iw_get_dbm(buffer, sizeof(buffer),&event->u.qual, iw_range, has_range);
      break;
    case IWEVGENIE:
      /* Informations Elements are complex, let's do only some of them */
      iw_print_gen_ie(event->u.data.pointer, event->u.data.length);
      break;
    case IWEVCUSTOM:
      break;
    default:
		break;
   }	/* switch(event->cmd) */
}

void formatMac(char *destMac, char *sourMac)
{
	int i = 0;
	char *p_dest = destMac,*p_sour = sourMac;
	for(i = 1; i<=17; i++){
		if(i%3 == 0){
			*p_dest = ':';
			p_dest++;
		}
		else{
			*p_dest = *p_sour;
			p_dest++;
			p_sour++;
		}
	}
	return ;
}


void formatScanList(ap_list_info_t *ap_list_info)
{
	char tmp[30];
	char *ptmp;
	int i;
	int c_2_i_1=0;
	int c_2_i_2=0;
	int type_ssid_code=0;
	int wifimode=get_wifi_mode();
	int i_channel=atoi(channelStr);
	
	if(!strlen(ssidstr))  //remove the empty ssid
		return;
	//DMCLOG_D("wifimode = %d,i_channel = %d", wifimode, i_channel);
	if(wifimode==M2G && i_channel >14)
		return;
	if(wifimode==M5G && i_channel <36)
		return;
	type_ssid_code=is_UTF8_or_gb2312(ssidstr,strlen(ssidstr));
	if(type_ssid_code==is_gb2312)
	{
		return;
	}
	//DMCLOG_D("200ap_list_info->count=%d", ap_list_info->count);
	for(i = 0; i < ap_list_info->count; i++){
		if(0 == strcmp(ssidstr, ap_list_info->ap_info[i].ssid)){
			DMCLOG_D("ssid is same with \"%s\"", ap_list_info->ap_info[i].ssid);
			return ;
		}
	}
	
	strcpy(ap_list_info->ap_info[ap_list_info->count].ssid, ssidstr);
	//DMCLOG_D("211ssid=%s", ap_list_info->ap_info[ap_list_info->count].ssid);
	//strcpy(ap_list_info->ap_info[ap_list_info->count].mac, macaddr);
	formatMac(ap_list_info->ap_info[ap_list_info->count].mac, macaddr);
	//DMCLOG_D("211mac=%s", ap_list_info->ap_info[ap_list_info->count].mac);
	ap_list_info->ap_info[ap_list_info->count].channel = atoi(channelStr);
	//DMCLOG_D("211channel=%d", ap_list_info->ap_info[ap_list_info->count].channel);
	ap_list_info->ap_info[ap_list_info->count].wifi_signal = rssidbm;
	//DMCLOG_D("211channel=%d", ap_list_info->ap_info[ap_list_info->count].wifi_signal);
	if(hasEncrypt)
	{
		if(haswpa)
		{
			if(haswpa&WPA1X_FLAG)
			{
				if((haswpa&WPA_FLAG) && (haswpa&WPA2_FLAG))
				{
					strcpy(ap_list_info->ap_info[ap_list_info->count].encrypt, "WPA/WPA2-1X");
				}
				else 
				{
					if(haswpa&WPA_FLAG)
					{
						strcpy(ap_list_info->ap_info[ap_list_info->count].encrypt, "WPA-1X");
					}
					else if(haswpa&WPA2_FLAG)
					{
						strcpy(ap_list_info->ap_info[ap_list_info->count].encrypt, "WPA2-1X");
					}
				}
			}
			else
			{
				if((haswpa&WPA_FLAG) && (haswpa&WPA2_FLAG))
				{	
					strcpy(ap_list_info->ap_info[ap_list_info->count].encrypt, "WPA/WPA2-PSK");
				}
				else 
				{
					if(haswpa&WPA_FLAG)
					{	
						strcpy(ap_list_info->ap_info[ap_list_info->count].encrypt, "WPA-PSK");
					}
					
					if(haswpa&WPA2_FLAG)
					{
						strcpy(ap_list_info->ap_info[ap_list_info->count].encrypt, "WPA2-PSK");
					}
				}
			}
		}
		else
		{
			//wep mode		
			strcpy(ap_list_info->ap_info[ap_list_info->count].encrypt, "WEP");
		}
	}
	else
	{
		//none
		strcpy(ap_list_info->ap_info[ap_list_info->count].encrypt, "NONE");
	}

	if((encrypttype&ENCRYPT_CCMP) && (encrypttype&ENCRYPT_TKIP))
	{
		strcpy(ap_list_info->ap_info[ap_list_info->count].tkip_aes, "tkip/aes");
	}
	else
	{
		if(encrypttype&ENCRYPT_CCMP)
		{
			strcpy(ap_list_info->ap_info[ap_list_info->count].tkip_aes, "aes");
		}
		else if(encrypttype&&ENCRYPT_TKIP)
		{
			strcpy(ap_list_info->ap_info[ap_list_info->count].tkip_aes, "tkip");
		}
	}

	if(is_connected_ap(ap_list_info->ap_info[ap_list_info->count].ssid, ap_list_info->ap_info[ap_list_info->count].password))
		ap_list_info->ap_info[ap_list_info->count].record = 1;
	else
		ap_list_info->ap_info[ap_list_info->count].record = 0;

	ap_list_info->count++;
}


static int cgi_get_scan(int skfd, char *	ifname, ap_list_info_t *ap_list_info)		/* Args count */
{
	struct iwreq		wrq;
	struct iw_scan_req    scanopt;		/* Options for 'set' */
	int			scanflags = 0;		/* Flags for scan */
	unsigned char *	buffer = NULL;		/* Results */
	int			buflen = IW_SCAN_MAX_DATA; /* Min for compat WE<17 */
	struct iw_range	range;
	int			has_range;
	struct timeval	tv;				/* Select timeout */
	int			timeout = 15000000;		/* 15s */

	/* Get range stuff */
	has_range = (iw_get_range_info(skfd, ifname, &range) >= 0);

	/* Check if the interface could support scanning. */
	if((!has_range) || (range.we_version_compiled < 14))
    {
      //fprintf(stderr, "%-8.16s  Interface doesn't support scanning.\n\n",
	  //    ifname);
      return(-1);
    }

	/* Init timeout value -> 250ms between set and first get */
	tv.tv_sec = 0;
	tv.tv_usec = 250000;

	/* Clean up set args */
	memset(&scanopt, 0, sizeof(scanopt));


	/* Check if we have scan options */
	if(scanflags)
    {
      wrq.u.data.pointer = (caddr_t) &scanopt;
      wrq.u.data.length = sizeof(scanopt);
      wrq.u.data.flags = scanflags;
    }
	else
    {
      wrq.u.data.pointer = NULL;
      wrq.u.data.flags = 0;
      wrq.u.data.length = 0;
    }

	/* If only 'last' was specified on command line, don't trigger a scan */
	if(scanflags == IW_SCAN_HACK)
    {
      /* Skip waiting */
      tv.tv_usec = 0;
    }
	else
    {
		/* Initiate Scanning */
		if(iw_set_ext(skfd, ifname, SIOCSIWSCAN, &wrq) < 0)
		{
		  if((errno != EPERM) || (scanflags != 0))
		    {
		      //fprintf(stderr, "%-8.16s  Interface doesn't support scanning : %s\n\n",
			  //    ifname, strerror(errno));
		      return(-1);
		    }
		  /* If we don't have the permission to initiate the scan, we may
		   * still have permission to read left-over results.
		   * But, don't wait !!! */
		  tv.tv_usec = 0;
		}
    }
	timeout -= tv.tv_usec;

	/* Forever */
	while(1)
    {
		fd_set		rfds;		/* File descriptors for select */
		int		last_fd;	/* Last fd */
		int		ret;

		/* Guess what ? We must re-generate rfds each time */
		FD_ZERO(&rfds);
		last_fd = -1;

		/* In here, add the rtnetlink fd in the list */

		/* Wait until something happens */
		ret = select(last_fd + 1, &rfds, NULL, NULL, &tv);

		/* Check if there was an error */
		if(ret < 0)
		{
			if(errno == EAGAIN || errno == EINTR)
			continue;
			//fprintf(stderr, "Unhandled signal - exiting...\n");
			return(-1);
		}

		/* Check if there was a timeout */
		if(ret == 0)
		{
			unsigned char *	newbuf;

crealloc:
			/* (Re)allocate the buffer - realloc(NULL, len) == malloc(len) */
			newbuf = realloc(buffer, buflen);
			if(newbuf == NULL)
			{
				if(buffer)
				free(buffer);
				//fprintf(stderr, "%s: Allocation failed\n", __FUNCTION__);
				return(-1);
			}
			
			buffer = newbuf;

			/* Try to read the results */
			wrq.u.data.pointer = buffer;
			wrq.u.data.flags = 0;
			wrq.u.data.length = buflen;
			if(iw_get_ext(skfd, ifname, SIOCGIWSCAN, &wrq) < 0)
			{
				/* Check if buffer was too small (WE-17 only) */
				if((errno == E2BIG) && (range.we_version_compiled > 16)
				&& (buflen < 0xFFFF))
				{
					/* Some driver may return very large scan results, either
					* because there are many cells, or because they have many
					* large elements in cells (like IWEVCUSTOM). Most will
					* only need the regular sized buffer. We now use a dynamic
					* allocation of the buffer to satisfy everybody. Of course,
					* as we don't know in advance the size of the array, we try
					* various increasing sizes. Jean II */

					/* Check if the driver gave us any hints. */
					if(wrq.u.data.length > buflen)
						buflen = wrq.u.data.length;
					else
						buflen *= 2;

					/* wrq.u.data.length is 16 bits so max size is 65535 */
					if(buflen > 0xFFFF)
						buflen = 0xFFFF;

		  			/* Try again */
		  			goto crealloc;
				}

				/* Check if results not available yet */
				if(errno == EAGAIN)
				{
					/* Restart timer for only 100ms*/
					tv.tv_sec = 0;
					tv.tv_usec = 100000;
					timeout -= tv.tv_usec;
					if(timeout > 0)
						continue;	/* Try again later */
				}

				/* Bad error */
				free(buffer);
				//fprintf(stderr, "%-8.16s  Failed to read scan data : %s\n\n",
				//  ifname, strerror(errno));
				return(-2);
		    }
			else
				/* We have the results, go to process them */
				break;
		}

	  /* In here, check if event and event type
	   * if scan event, read results. All errors bad & no reset timeout */
	}

	DMCLOG_D("start scan");
	if(wrq.u.data.length)
    {
		struct iw_event		iwe;
		struct stream_descr	stream;
		struct iwscan_state	state = { .ap_num = 1, .val_index = 0 };
		int			ret;
      
		//printf("%-8.16s  Scan completed :\n", ifname);
		iw_init_event_stream(&stream, (char *) buffer, wrq.u.data.length);
		
		hasEncrypt = 0;
		haswpa = 0;
		encrypttype = 0;

		do
		{
			/* Extract an event and print it */
			ret = iw_extract_event_stream(&stream, &iwe,
			range.we_version_compiled);
			if(ret > 0)
			{
				print_scanning_token(&stream, &iwe, &state,&range, has_range, ap_list_info);
			}
		}
		while(ret > 0);

		if(state.ap_num != 1)
		{
			formatScanList(ap_list_info);
		}
		DMCLOG_D("scan end");
    }
	else
	    ;//printf("%-8.16s  No scan results\n\n", ifname);
	free(buffer);
	return(0);
}
#else defined(WIFI_DRIVER_NL80211)
int cgi_get_scan_nl80211(char *ifname, ap_list_info_t *p_ap_list)
{
	int i = 0, j = 0;
	ap_list_info_t ap_list_all;
	int wifi_channel;
	int wifi_mode = get_wifi_mode();
	int ret = 0;
	int same_flag = 0;
	int type_ssid_code=0;
	char uci_option_str[64]="\0";
	char tmp_str[64];
	char cmd_str[64];
	
	if(ifname == NULL || p_ap_list == NULL){
		return -1;
	}

	memset(&ap_list_all, 0, sizeof(ap_list_info_t));
	ret = get_scan_list_nl80211(ifname, &ap_list_all);
	if (ret){
		if(wifi_mode == M5G){
			memset(uci_option_str, '\0', 64);
			memset(tmp_str, '\0', 64);
			memset(cmd_str, '\0', 64);
			strcpy(uci_option_str,"wireless2.@wifi[0].ch5g"); 
			ret = uci_get_option_value(uci_option_str, tmp_str);
			if (ret){
				return -1;
			}
			sprintf(cmd_str, "wl down;wl channel %s;wl up", tmp_str);			
			system(cmd_str);
			
			memset(&ap_list_all, 0, sizeof(ap_list_info_t));
			ret = get_scan_list_nl80211(ifname, &ap_list_all);
			if (ret){
				return -1;
			}
		}
		else{
			return -1;
		}
	}

	for(i = 0; i < ap_list_all.count; i++){
		wifi_channel = ap_list_all.ap_info[i].channel;
		if(!((wifi_mode == M2G && wifi_channel <= 14 && wifi_channel > 0) 
			|| (wifi_mode == M5G && wifi_channel >= 36))){
			continue;
		}

		type_ssid_code=is_UTF8_or_gb2312(ap_list_all.ap_info[i].ssid,strlen(ap_list_all.ap_info[i].ssid));
		if(type_ssid_code==is_gb2312){
			continue;
		}
		
		for(j = 0; j < p_ap_list->count; j++){
			if(0 == strcmp(ap_list_all.ap_info[i].ssid, p_ap_list->ap_info[j].ssid)){
				DMCLOG_D("ssid is same with \"%s\"", p_ap_list->ap_info[j].ssid);
				same_flag = 1;
				break;
			}
		}
		
		if(same_flag){
			same_flag = 0;
			continue;
		}
		
		strcpy(p_ap_list->ap_info[p_ap_list->count].ssid, ap_list_all.ap_info[i].ssid);
		strcpy(p_ap_list->ap_info[p_ap_list->count].mac, ap_list_all.ap_info[i].mac);
		strcpy(p_ap_list->ap_info[p_ap_list->count].encrypt, ap_list_all.ap_info[i].encrypt);
		strcpy(p_ap_list->ap_info[p_ap_list->count].tkip_aes, ap_list_all.ap_info[i].tkip_aes);
		p_ap_list->ap_info[p_ap_list->count].channel = ap_list_all.ap_info[i].channel;
		p_ap_list->ap_info[p_ap_list->count].wifi_signal = ap_list_all.ap_info[i].wifi_signal;

		if(is_connected_ap(p_ap_list->ap_info[p_ap_list->count].ssid, p_ap_list->ap_info[p_ap_list->count].password))
			p_ap_list->ap_info[p_ap_list->count].record = 1;
		else
			p_ap_list->ap_info[p_ap_list->count].record = 0;		
		p_ap_list->count++;
		if(p_ap_list->count >= 100)
			break;
	}
	return 0;
}

int cgi_get_scan_nl80211_add_network(char *ifname, char *ssid, ap_info_t *p_ap_info)
{
	int ret = 0;
	int wifi_channel;
	char uci_option_str[64]="\0";
	char tmp_str[64];
	char cmd_str[64];
	ap_list_info_t *scan_ap_list = NULL;
	int wifi_mode = get_wifi_mode();

	if(ifname == NULL || ssid == NULL || p_ap_info == NULL){
		ret = -1;
		goto EXIT;
	}

	scan_ap_list = (ap_list_info_t *)malloc(sizeof(ap_list_info_t));
	if(scan_ap_list == NULL){
		ret = -1;
		goto EXIT;
	}

	memset(scan_ap_list, 0, sizeof(ap_list_info_t));
	ret = get_scan_list_nl80211_add_network(ifname, ssid, scan_ap_list);
	if(0 == ret){
		if(scan_ap_list->count == 0){
			memset(scan_ap_list, 0, sizeof(ap_list_info_t));
			ret = get_scan_list_nl80211_add_network(ifname, ssid, scan_ap_list);
			if (ret){
				ret = -1;
				goto EXIT;
			}
		}
	}
	else{
		if(wifi_mode == M5G){
			memset(uci_option_str, '\0', 64);
			memset(tmp_str, '\0', 64);
			memset(cmd_str, '\0', 64);
			strcpy(uci_option_str,"wireless2.@wifi[0].ch5g"); 
			ret = uci_get_option_value(uci_option_str, tmp_str);
			if (ret){
				ret = -1;
				goto EXIT;
			}
			sprintf(cmd_str, "wl down;wl channel %s;wl up", tmp_str);			
			system(cmd_str);
			
			memset(scan_ap_list, 0, sizeof(ap_list_info_t));
			ret = get_scan_list_nl80211_add_network(ifname, ssid, scan_ap_list);
			DMCLOG_D("p_ap_list->count: %d", scan_ap_list->count);
			if (ret){
				ret = -1;
				goto EXIT;
			}
		}
		else{
			ret = -1;
			goto EXIT;
		}
	}

	if(scan_ap_list->count != 0){
		wifi_channel = scan_ap_list->ap_info[0].channel;
		if(!((wifi_mode == M2G && wifi_channel <= 14 && wifi_channel > 0) 
			|| (wifi_mode == M5G && wifi_channel >= 36))){
			ret = -1;
			goto EXIT;
		}
		else{
			strcpy(p_ap_info->ssid, scan_ap_list->ap_info[0].ssid);
			strcpy(p_ap_info->mac, scan_ap_list->ap_info[0].mac);
			strcpy(p_ap_info->encrypt, scan_ap_list->ap_info[0].encrypt);
			strcpy(p_ap_info->tkip_aes, scan_ap_list->ap_info[0].tkip_aes);
			p_ap_info->channel = scan_ap_list->ap_info[0].channel;
			p_ap_info->wifi_signal = scan_ap_list->ap_info[0].wifi_signal;
		}
	}
	else{
		ret = -1;
		goto EXIT;
	}

EXIT:
	safe_free(scan_ap_list);
	return ret;
}
#endif
#endif

int dm_wlan_scan(ap_list_info_t *ap_list_info)
{
#if defined(SUPPORT_OPENWRT_PLATFORM)
#if defined(OPENWRT_MT7628)
	char cmd[128];
	char device_name[16];
	char buffer[1024];
	char security[32];
	int chars_read = 0;
	FILE *read_fp;
	int i = 0;
	char member[64];

	memset(cmd,0,128);
	memset(device_name,0,16);
	int temp_ret = change_fre_device_name(ap_list_info->fre,device_name);
	if(temp_ret < 0)
		return -2;
	sprintf(cmd,"iwpriv %s set SiteSurvey=1",device_name);
	system(cmd);
	sleep(1);
	read_fp = popen("iwpriv ra0 get_site_survey", "r");
	if(read_fp != NULL)
	{
		do{
			memset(buffer,0,1024);
			chars_read = fgets(buffer, 1024-1, read_fp);
			//DMCLOG_D("buffer = %s",buffer);
			if(chars_read > 0)
			{
				if((58>buffer[0])&&(buffer[0]>=48))
				{
					memset(security,0,32);
					memset(member,0,64);
					memcpy(member,buffer,4);
					//DMCLOG_D("member1 = %s",member);
					ap_list_info->ap_info[ap_list_info->count].channel = atoi(member);
					
					memcpy(ap_list_info->ap_info[ap_list_info->count].ssid,buffer+4,33);

					memset(member,0,64);
					memcpy(member,buffer+37,4);
					//DMCLOG_D("member2 = %s",member);
					ap_list_info->ap_info[ap_list_info->count].ssid_len=atoi(member);
					if(ap_list_info->ap_info[ap_list_info->count].ssid_len == 0)
					{
						continue;
					}
					for(i = ap_list_info->ap_info[ap_list_info->count].ssid_len;i < 33;i++)
					{
						ap_list_info->ap_info[ap_list_info->count].ssid[i]=0;
					}
					memcpy(ap_list_info->ap_info[ap_list_info->count].mac,buffer+41,17);

					memset(member,0,64);
					memcpy(member,buffer+61,23);
					//DMCLOG_D("member3 = %s",member);
					judge_encrypt_tkip(member,ap_list_info->ap_info[ap_list_info->count].encrypt,ap_list_info->ap_info[ap_list_info->count].tkip_aes);
					memset(member,0,64);
					memcpy(member,buffer+84,9);
					//DMCLOG_D("member4 = %s",member);
					ap_list_info->ap_info[ap_list_info->count].wifi_signal = atoi(member);

					if(is_connected_ap(ap_list_info->ap_info[ap_list_info->count].mac, ap_list_info->ap_info[ap_list_info->count].password))
						ap_list_info->ap_info[ap_list_info->count].record = 1;
					else
						ap_list_info->ap_info[ap_list_info->count].record = 0;
							
					ap_list_info->count++;		
				}
			}
		}while(chars_read > 0);
	}
	else
	{
		return ROUTER_ERRORS_SHELL_HANDLE;
	}
	pclose(read_fp);
#elif defined(OPENWRT_X1000)
	char ifname[10];
	memset(ifname, 0, sizeof(ifname));
	strcpy(ifname, "wlan0");	
	int ret;
#if defined(WIFI_DRIVER_WEXT)
	int skfd;	
	if((skfd = iw_sockets_open()) < 0){
		return ROUTER_ERRORS_SOCKET_IOCTL;
	}

	ap_list_info->count=0;
	ret = cgi_get_scan(skfd, ifname, ap_list_info);
	if(ret < 0){
		return ROUTER_ERRORS_SOCKET_IOCTL;
	}

	iw_sockets_close(skfd);
#else defined(WIFI_DRIVER_NL80211)
	ap_list_info->count=0;
	ret = cgi_get_scan_nl80211(ifname, ap_list_info);
	if(ret < 0){
		return ROUTER_ERRORS_SOCKET_IOCTL;
	}
#endif
#endif
#elif defined(SUPPORT_LINUX_PLATFORM)
#endif
	return 0;
}



