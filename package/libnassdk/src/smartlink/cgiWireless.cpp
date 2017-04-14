#include <stdio.h>
#include <iwlib.h>

static int hasEncrypt=0;
static int haswpa=0;
static int encrypttype=0;

static char macaddr[100];
static char ssidstr[100];
static char channelStr[10];
static char rssi[10];
static int rssidbm;

#define IW_SCAN_HACK		0x8000

#define WPA2_FLAG 0x01
#define WPA_FLAG 0x02
#define WPA1X_FLAG 0x04

#define WPA_MIX 0x03

#define ENCRYPT_TKIP 0x01
#define ENCRYPT_CCMP 0x02
#define ENCRYPT_WEP_40 0x04
#define ENCRYPT_WEP_104 0x08
#define ENCRYPT_WRAP 0x10

/*
 * Scan state and meta-information, used to decode events...
 */
typedef struct iwscan_state
{
  /* State */
  int			ap_num;		/* Access Point number 1->N */
  int			val_index;	/* Value in table 0->(N-1) */
} iwscan_state;

typedef struct ap_info{
	char ssid[32];				/* ssid name */
	int ssid_len;				/* ssid length */
	char mac[32];				/* mac */
	int channel;				/* channel */
	char encrypt[32];			/* encryption method */
	char tkip_aes[16];			/* encryption algorithm */
	int wifi_signal;			/* ap signal */
	int record;					/* 0:no record;1:has record */
	char password[64];			/* record ap password */
}ap_info_t;

typedef struct ap_list_info{    
	int count;					/* the count of ap */
	char fre[8];				/* 2.4G or 5G */
	ap_info_t ap_info[100];		/* ap list */
}ap_list_info_t;

#define is_ascii  0x11
#define is_gb2312 0x22
#define is_utf8   0x33

static int is_UTF8_or_gb2312(const char* str,long length)
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

static void changeMacStr(char *instr, char *outstr)
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

//#if defined(WIFI_DRIVER_WEXT)
/*------------------------------------------------------------------*/
/*
 * Output the link statistics, taking care of formating
 */
static void
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
static void
iw_essid_escape(char *dest,
		const char *src,
		const int slen)
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
iw_print_gen_ie(unsigned char *buffer,
		int	buflen)
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


static void formatScanList(ap_list_info_t *ap_list_info)
{
	char tmp[30];
	char *ptmp;
	int i;
	int c_2_i_1=0;
	int c_2_i_2=0;
	int type_ssid_code=0;
	//int wifimode=get_wifi_mode();
	int i_channel=atoi(channelStr);
	
	if(!strlen(ssidstr))  //remove the empty ssid
		return;
	#if 0
	//DMCLOG_D("wifimode = %d,i_channel = %d", wifimode, i_channel);
	if(wifimode==M2G && i_channel >14)
		return;
	if(wifimode==M5G && i_channel <36)
		return;
	type_ssid_code=is_UTF8_or_gb2312(ssidstr,strlen(ssidstr));
	if(type_ssid_code==is_gb2312){
		return;
	}
	//DMCLOG_D("200ap_list_info->count=%d", ap_list_info->count);
	for(i = 0; i < ap_list_info->count; i++){
		if(0 == strcmp(ssidstr, ap_list_info->ap_info[i].ssid)){
			DMCLOG_D("ssid is same with \"%s\"", ap_list_info->ap_info[i].ssid);
			return ;
		}
	}
	#endif
	
	strcpy(ap_list_info->ap_info[ap_list_info->count].ssid, ssidstr);
	//DMCLOG_D("211ssid=%s", ap_list_info->ap_info[ap_list_info->count].ssid);
	strcpy(ap_list_info->ap_info[ap_list_info->count].mac, macaddr);
	//formatMac(ap_list_info->ap_info[ap_list_info->count].mac, macaddr);
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

	ap_list_info->count++;
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
				iw_essid_escape(essid, (char *)event->u.essid.pointer, event->u.essid.length);
			
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
      iw_print_gen_ie((unsigned char *)event->u.data.pointer, event->u.data.length);
      break;
    case IWEVCUSTOM:
      break;
    default:
		break;
   }	/* switch(event->cmd) */
}

static void formatMac(char *destMac, char *sourMac)
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

static int cgi_get_scan(int skfd, char *ifname, ap_list_info_t *ap_list_info)		/* Args count */
{
	//printf("skfd: %d, ifname: %s\n", skfd, ifname);
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
			newbuf = (unsigned char *)realloc(buffer, buflen);
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

	if(wrq.u.data.length)
    {
		struct iw_event		iwe;
		struct stream_descr	stream;
		struct iwscan_state	state;
		state.ap_num = 1;
		state.val_index = 0;
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
    }
	else
	    ;//printf("%-8.16s  No scan results\n\n", ifname);
	free(buffer);
	return(0);
}


int cgi_scan(const char *pssid,char *pmac,char *pchanel,char *pencrypt,char *ptkip_aes)
{
	//printf("pssid: %s\n", pssid);
	int ret = 0;
	char ifname[10];
	int skfd;	
	int apNum = 0;
	int hasMatch = 0;
	ap_list_info_t apList;

	if(pssid == NULL || pmac == NULL || pchanel == NULL || pencrypt == NULL || ptkip_aes == NULL){
		printf("argument null\n");
		return -1;
	}

	memset(ifname, 0, sizeof(ifname));
	strcpy(ifname,"mlan0");
	
	if((skfd = iw_sockets_open()) < 0){
		printf("iw open socket fail\n");
		return -1;
	}

	memset(&apList, 0, sizeof(ap_list_info_t));
	ret = cgi_get_scan(skfd, ifname, &apList);
	if(ret < 0){
		printf("cgi_get_scan fail\n");
		iw_sockets_close(skfd);
		return -1;
	}
	iw_sockets_close(skfd);

	//printf("pssid: %s\n", pssid);
	for(apNum = 0; apNum < apList.count; apNum++){
		//printf("apList.ap_info[%d].ssid: %s\n", apNum, apList.ap_info[apNum].ssid);
		if(!strcmp(pssid, apList.ap_info[apNum].ssid)){
			strcpy(pmac, apList.ap_info[apNum].mac);
			sprintf(pchanel, "%d", apList.ap_info[apNum].channel);
			strcpy(pencrypt, apList.ap_info[apNum].encrypt);
			strcpy(ptkip_aes, apList.ap_info[apNum].tkip_aes);
			hasMatch = 1;
			break;
		}
	}

	if(hasMatch)
		return 0;
	else
		return -1;	
}

