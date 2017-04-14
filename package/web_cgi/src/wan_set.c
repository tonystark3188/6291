
#include "msg.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>        
#include <linux/wireless.h>


#if WIRELESS_EXT <= 11
#ifndef SIOCDEVPRIVATE
#define SIOCDEVPRIVATE      0x8BE0
#endif
#define SIOCIWFIRSTPRIV      SIOCDEVPRIVATE
#endif

#define RT_PRIV_IOCTL    (SIOCIWFIRSTPRIV + 0x01)
#define RTPRIV_IOCTL_SET       (SIOCIWFIRSTPRIV + 0x02)
#define RTPRIV_IOCTL_BBP   (SIOCIWFIRSTPRIV + 0x03)
#define RTPRIV_IOCTL_MAC    (SIOCIWFIRSTPRIV + 0x05)
#define RTPRIV_IOCTL_E2P    (SIOCIWFIRSTPRIV + 0x07)
#define RTPRIV_IOCTL_STATISTICS    (SIOCIWFIRSTPRIV + 0x09)
#define RTPRIV_IOCTL_ADD_PMKID_CACHE    (SIOCIWFIRSTPRIV + 0x0A)
#define RTPRIV_IOCTL_RADIUS_DATA     (SIOCIWFIRSTPRIV + 0x0C)
#define RTPRIV_IOCTL_GSITESURVEY    (SIOCIWFIRSTPRIV + 0x0D)
#define RTPRIV_IOCTL_ADD_WPA_KEY    (SIOCIWFIRSTPRIV + 0x0E)
#define RTPRIV_IOCTL_GET_MAC_TABLE    (SIOCIWFIRSTPRIV + 0x0F)
#define RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT	(SIOCIWFIRSTPRIV + 0x1F)
#define RTPRIV_IOCTL_SHOW_CONNSTATUS	(SIOCIWFIRSTPRIV + 0x1B)
#define RTPRIV_IOCTL_GET_APCLI_RSSI	    (SIOCIWFIRSTPRIV + 0x1C)

#define MAC_ADDR_LEN        6
#define ETH_LENGTH_OF_ADDRESS      6
#define MAX_LEN_OF_MAC_TABLE      64


#define SSID_MAX_LEN 128

#define SSID_MAX_NUM 20

void StringToHex(unsigned char* string, unsigned char* hexstring)
{
       unsigned char ch,i,j,len;
 
       len = strlen(string);
 
       for(i=0,j=0;i<len;i++,j+=2)
       {
            ch = string[i];
 
            if( ch >= 0 && ch <= 0x0F)
            {
                hexstring[j] = 0x30;
 
                if(ch >= 0 && ch <= 9)
                    hexstring[j+1] = 0x30 + ch;
                else
                    hexstring[j+1] = 0x37 + ch;
            }
            else  if( ch >= 0x10 && ch <= 0x1F)
            {
                hexstring[j] = 0x31;
                ch -= 0x10;
 
                if(ch >= 0 && ch <= 9)
                    hexstring[j+1] = 0x30 + ch;
                else
                    hexstring[j+1] = 0x37 + ch;
            }
            else  if( ch >= 0x20 && ch <= 0x2F)
            {
                hexstring[j] = 0x32;
                ch -= 0x20;
 
                if(ch >= 0 && ch <= 9)
                    hexstring[j+1] = 0x30 + ch;
                else
                    hexstring[j+1] = 0x37 + ch;
            }
            else  if( ch >= 0x30 && ch <= 0x3F)
            {
                hexstring[j] = 0x33;
                ch -= 0x30;
 
                if(ch >= 0 && ch <= 9)
                    hexstring[j+1] = 0x30 + ch;
                else
                    hexstring[j+1] = 0x37 + ch;
            }
            else  if( ch >= 0x40 && ch <= 0x4F)
            {
                hexstring[j] = 0x34;
                ch -= 0x40;
 
                if(ch >= 0 && ch <= 9)
                    hexstring[j+1] = 0x30 + ch;
                else
                    hexstring[j+1] = 0x37 + ch;
            }
            else  if( ch >= 0x50 && ch <= 0x5F)
            {
                hexstring[j] = 0x35;
                ch -= 0x50;
 
                if(ch >= 0 && ch <= 9)
                    hexstring[j+1] = 0x30 + ch;
                else
                    hexstring[j+1] = 0x37 + ch;
            }
            else  if( ch >= 0x60 && ch <= 0x6F)
            {
                hexstring[j] = 0x36;
                ch -= 0x60;
 
                if(ch >= 0 && ch <= 9)
                    hexstring[j+1] = 0x30 + ch;
                else
                    hexstring[j+1] = 0x37 + ch;
            } 
            else  if( ch >= 0x70 && ch <= 0x7F)
            {
                hexstring[j] = 0x37;
                ch -= 0x70;
 
                if(ch >= 0 && ch <= 9)
                    hexstring[j+1] = 0x30 + ch;
                else
                    hexstring[j+1] = 0x37 + ch;
            }
       }
       hexstring[j] = 0x00;
}
int htoi(char s[])
{
    int n = 0;
    int i = 0;
		//p_debug("s[]=%s",s);
    while (s[i] != '\0' && s[i] != '\n') {
		//p_debug("s[%d]=%c",i,s[i]);
        if (s[i] == '0') {
            if (s[i+1] == 'x' || s[i+1] == 'X')
                            i+=2;
        }
        if (s[i] >= '0' && s[i] <= '9') {
            n = n * 16 + (s[i] - '0');
        } else if (s[i] >= 'a' && s[i] <= 'f') {
            n = n * 16 + (s[i] - 'a') + 10;
        } else if (s[i] >= 'A' && s[i] <= 'F') {
            n = n * 16 + (s[i] - 'A') + 10;
        } else
            return -1;
        ++i;
 
    }
	//p_debug("n=%d",n);
    return n;
}

void string2hex(unsigned char* string, unsigned char* hexstring){
	unsigned char i,j,len;
	
	len = strlen(string);
	char tmp_str[2]="\0";
	//malloc(len+1);
	//memset(tmp_str,0,len+1);
	//strcpy(tmp_str,string);

for(i=0,j=0;j<len;i++,j+=2)
	{

		 tmp_str[0] = string[j];
		 tmp_str[1] = string[j+1];
		 tmp_str[2]='\0';
		 //p_debug("tmp_str=%s",tmp_str);
		 //hexstring[i]=atoi(tmp_str);
		 hexstring[i]=htoi(tmp_str);
 		//p_debug("	string[%d]=%c,string[%d]=%c",j,string[j],j+1,string[j+1]); 
		//p_debug("hexstring[%d]=%x",i,hexstring[i]); 
	}	
	//hexstring[i] = 0x00;

}
void save_ssid(const char** ssid,const char** key, int ssid_total_num){

	int i=0;
	char pssid[64]="\0";
	char pkey[64]="\0";

	for(i=0;i<ssid_total_num;i++)
	{	
		//p_debug("ssid[%d]=%s,key[%d]=%s",i,ssid[i],i,key[i]);
		string2hex(ssid[i],pssid);
		string2hex(key[i],pkey);
		//p_debug("pssid=%s",pssid);
		//p_debug("pkey=%s",pkey);		
		refreshAPlist(pssid,pkey);
		memset(pssid,0,strlen(pssid));
		memset(pkey,0,strlen(pkey));		
	}


#if 0
	char ap_sum[8];
	char cmd[64]="\0";
	char uci_option_str[64]="\0";
	int i,j;
	unsigned int is_saved=0;
	char tmp_ssid[SSID_MAX_LEN]="\0";
	char tmp_key[SSID_MAX_LEN]="\0";
	int ssid_sum;
	int now_sum;
	unsigned int already_saved_sum=0;
	unsigned int new_saved_sum=0;
	strcpy(uci_option_str,"remoteAPlist.ap.sum");			//sum
	uci_get_option_value(uci_option_str,ap_sum);
	memset(uci_option_str,'\0',64);
	ssid_sum = atoi(ap_sum);
	now_sum = ssid_sum;
	//p_debug("ssid_sum=%d\n",now_sum);
	for(i=0;i<((ssid_total_num>SSID_MAX_NUM)?SSID_MAX_NUM:(ssid_total_num));i++){
		for(j=0;j<ssid_sum;j++){
			sprintf(uci_option_str,"remoteAPlist.@ap[%d].ssid",j);			//name
			uci_get_option_value(uci_option_str,tmp_ssid);
			memset(uci_option_str,'\0',64);			
			//p_debug("i=%d,j=%d,tmp_ssid=%s\n",i,j,tmp_ssid);

			sprintf(uci_option_str,"remoteAPlist.@ap[%d].key",j);			//key
			uci_get_option_value(uci_option_str,tmp_key);
			memset(uci_option_str,'\0',64);			
			//p_debug("tmp_key=%s\n",tmp_key);

			if(!strcmp(tmp_ssid,ssid[i])){//already saved
				is_saved=1;
				already_saved_sum++;
				if(!strcmp(tmp_key,key[i]))
				{	
					break;//
				}
				else{//update key
					sprintf(cmd,"uci set remoteAPlist.@ap[%d].key=%s",j,key[i]);
					system(cmd);
					system("uci commit");					
					break;
				}
			}
		}
		if(is_saved){
			is_saved=0;
			continue;//check the next ssid[i]
		}else {//not saved
			new_saved_sum++;
			if((ssid_sum+(i-already_saved_sum))>=SSID_MAX_NUM) {// big than the MAX_NUM
				now_sum=(ssid_sum+i-already_saved_sum-SSID_MAX_NUM);
			}
				//p_debug("%d,%d,%d,%d\n",ssid_sum,i,already_saved_sum,now_sum);
				memset(cmd,0,128);
				sprintf(cmd,"uci set remoteAPlist.@ap[%d].ssid=%s",now_sum,ssid[i]);
				system(cmd);
						
				sprintf(cmd,"uci set remoteAPlist.@ap[%d].key=%s",now_sum,key[i]);
				system(cmd);
				//p_debug("ssid[%d]=%s\n",i,ssid[i]);
		
		}

	}
	if(ssid_sum>=SSID_MAX_NUM)
		ssid_sum=SSID_MAX_NUM;
	else 
		ssid_sum=ssid_sum+new_saved_sum;
	sprintf(cmd,"uci set remoteAPlist.ap.sum=%d ",ssid_sum);
	system(cmd);
	system("uci commit");
#endif
}

#define AP_SUM  20

int refreshAPlist(char *ssid, char *password) 
{
	char uci_option_str[64]="\0";
	//char encrypt[32]="\0";
	//char channel[5]="\0";
	//char password[64]="\0";
	//char wpa_crypto[64]="\0";
	//char ssid[64]="\0";
	char remote_ap_ssid[64]="\0";
	//char mac[64]="\0";
	//char disabled[8] = "\0";
	//int have_disabled_ssid = -1;
	char sum[8];
	int remote_ap_sum = 0;
	int tmp_sum = 0;
	//char data[128];
	int cnt;
	int i;

	ctx=uci_alloc_context();


	sprintf(uci_option_str,"remoteAPlist.ap.sum");
	memset(sum,0,sizeof(sum));
	uci_get_option_value(uci_option_str,sum);
	memset(uci_option_str,'\0',64);
	remote_ap_sum=atoi(sum);
	tmp_sum = remote_ap_sum;

	if(remote_ap_sum == AP_SUM)
	{
		for(cnt = 0; cnt < AP_SUM; cnt++)
		{
			sprintf(uci_option_str,"remoteAPlist.@ap[%d].ssid",cnt);
			memset(remote_ap_ssid,0,sizeof(remote_ap_ssid));
			uci_get_option_value(uci_option_str,remote_ap_ssid);
			memset(uci_option_str,'\0',64);
			if(strcasecmp(ssid,remote_ap_ssid) == 0)
			{
				
				memset(uci_option_str,0,64);
				sprintf(uci_option_str,"remoteAPlist.@ap[%d].ssid=%s",cnt,ssid);
				uci_set_option_value(uci_option_str);

				memset(uci_option_str,0,64);
				sprintf(uci_option_str,"remoteAPlist.@ap[%d].key=%s",cnt,password);
				uci_set_option_value(uci_option_str);				
					
				system("uci commit");
				uci_free_context(ctx);
				return 0;
			}
		}
		if(cnt == AP_SUM)
		{


			memset(uci_option_str,0,64);
			sprintf(uci_option_str,"remoteAPlist.@ap[0].ssid=%s",ssid);
			uci_set_option_value(uci_option_str);
	

			memset(uci_option_str,0,64);
			sprintf(uci_option_str,"remoteAPlist.@ap[0].key=%s",password);
			uci_set_option_value(uci_option_str);				

			system("uci commit");
			uci_free_context(ctx);
			return 0;
		}
			
	}
	else
	{

		for(cnt = 0; cnt < tmp_sum; cnt++)
		{
			memset(uci_option_str,'\0',64);
			sprintf(uci_option_str,"remoteAPlist.@ap[%d].ssid",cnt);
			memset(remote_ap_ssid,0,sizeof(remote_ap_ssid));
			uci_get_option_value(uci_option_str,remote_ap_ssid);
			if(strcasecmp(ssid,remote_ap_ssid) == 0)
			{


				memset(uci_option_str,0,64);
				sprintf(uci_option_str,"remoteAPlist.@ap[%d].ssid=%s",cnt,ssid);
				uci_set_option_value(uci_option_str);

				memset(uci_option_str,0,64);
				sprintf(uci_option_str,"remoteAPlist.@ap[%d].key=%s",cnt,password);
				uci_set_option_value(uci_option_str);				
				
				system("uci commit");
				uci_free_context(ctx);
				return 0;
			}

		}

		if(cnt == tmp_sum)
		{

			memset(uci_option_str,0,64);
			sprintf(uci_option_str,"remoteAPlist.@ap[%d].ssid=%s",cnt,ssid);
			uci_set_option_value(uci_option_str);
		
			memset(uci_option_str,0,64);
			sprintf(uci_option_str,"remoteAPlist.@ap[%d].key=%s",cnt,password);
			uci_set_option_value(uci_option_str);				

			memset(uci_option_str,0,64);
			sprintf(uci_option_str,"remoteAPlist.ap.sum=%d",remote_ap_sum+1);
			uci_set_option_value(uci_option_str);
			system("uci commit");
			uci_free_context(ctx);
			return 0;
		}
			
	}

	
}
int isConnected(){
	char data[128];
	int    socket_id;
	struct   iwreq wrq;
	int client_status=0;
	int ret = -1;
	//p_debug("c\n");
	
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_id < 0)
	{
		p_debug("error::Open socket error!\n\n");
		return -1;
	}
	strcpy(wrq.ifr_name, "ra0");
	wrq.u.data.length = 128;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;
	while(ret != 0)
	{
		ret = ioctl(socket_id, SIOCGIWNAME, &wrq);
		if(ret != 0)
		{
			p_debug("error::get wireless name\n\n");
			sleep(5);
		}
	}
	//sleep(10);
	memset(data, 0x00, 128);
	strcpy(wrq.ifr_name, "apcli0");
	wrq.u.data.length = 128;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;
	ret = ioctl(socket_id, RTPRIV_IOCTL_SHOW_CONNSTATUS, &wrq);
	if(ret != 0)
	{
		p_debug("error::get apcli0 connstatus error\n\n");
		return -1;
	}
	close(socket_id);
	client_status = *(unsigned int *)wrq.u.data.pointer;
	if(client_status == 1)
	{
		p_debug("apcli0 connected\n");
		//system("uci set wireless.@wifi-iface[1].connected=1");
		//system("uci commit");
	}
	
	return client_status;

}
void main()
{
		char ret_buf[4096];
		char code[CODE_LEN]="\0";
		char sid[SID_LEN]="\0";
		char fw_sid[SID_LEN]="\0";


		//char ssid[1024]="\0";
		//char key[1024]="\0";

		char *pssid=NULL;
		char *pkey=NULL;

		char *ssid=NULL;
		char *key=NULL;
		
		char tmp_buf[256]="\0";
		char tmp_vid[65]="\0";

		int ssid_num=1;
		int key_num=1;
		int i,j,k;
		char *web_str=NULL;
		int ret=0;
		char **ssid_array=NULL;
		char **key_array=NULL;
		char uci_option_str[UCI_BUF_LEN]="\0";
		unsigned int errCode=0;
		char  errMsg[256]="\0";
		char connected[8]={};
		char wifimode[8]={};
		ctx=uci_alloc_context();
		
		strcpy(uci_option_str,"system.@system[0].sid");			//name
		uci_get_option_value(uci_option_str,fw_sid);
		memset(uci_option_str,'\0',UCI_BUF_LEN);

		memset(uci_option_str,'\0',UCI_BUF_LEN);
		strcpy(uci_option_str,"system.@system[0].wifimode");			//name
		uci_get_option_value(uci_option_str,wifimode);
		memset(uci_option_str,'\0',UCI_BUF_LEN);
		
		printf("Content-type:text/plain\r\n\r\n");
		return 1;

		if((web_str=GetStringFromWeb())==NULL)
		{
			fprintf(stdout,"can't get string from web\n");
			exit(1);
		}
		//processString(web_str,SID,sid);		
		
		processString(web_str,CODE,code);		
		pssid=(char*)malloc(strlen(web_str));
		pkey=(char*)malloc(strlen(web_str));
		processString(web_str,"ssid",pssid);
		processString(web_str,"key",pkey);

		ssid=urlDecode(pssid);
		key=urlDecode(pkey);
		//p_debug("ssid=%s",ssid);
		//p_debug("key=%s",key);
		if(1){//是管理员
			//remove
			//if(strstr(vid,',')!=NULL)
			{
				if(strcmp(ssid,"")){//ssid could  not be null but key can
						
					for(i=0;i<strlen(ssid);i++){
						if(ssid[i]==',')
							{
							ssid_num++;
						}
					}
					
					for(i=0;i<strlen(key);i++){
						if(key[i]==',')
							{
							key_num++;
						}
					}
					if(ssid_num==key_num){
						ssid_array=(char**)malloc(sizeof(char*)*ssid_num);
						key_array=(char**)malloc(sizeof(char*)*ssid_num);

						for(i=0;i<ssid_num;i++){
							ssid_array[i] = (char*)malloc(SSID_MAX_LEN+1);
							key_array[i] = (char*)malloc(SSID_MAX_LEN+1);
						}
					}else 
						{
						errCode=4;
						strcpy(errMsg,"SSID or KEY is wrong");
						sprintf(ret_buf,"{\"status\":0,\"data\":{},\"errorCode\":%d,\"errorMessage\":\"%s\"}",errCode,errMsg);
						goto error;

					}
				}else {
					errCode=4;
					strcpy(errMsg,"SSID can't be NULL");
					sprintf(ret_buf,"{\"status\":0,\"data\":{},\"errorCode\":%d,\"errorMessage\":\"%s\"}",errCode,errMsg);
					goto error;
				}
				
			 	//sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"%s\":[","ssid");

				//char *str=strstr(vid,',');
	//			for(i=0,j=0,k=0;i<strlen(vid);i++)
				for(i=0,j=0,k=0;i<strlen(ssid);i++)
					{
						if(ssid[i]==',')
						{ 
							if(ssid[i-1]==','){//
								sprintf(ssid_array[j],"%s","");
								errCode=4;
								strcpy(errMsg,"ssid name can't be empty");
								sprintf(ret_buf,"{\"status\":0,\"data\":{},\"errorCode\":%d,\"errorMessage\":\"%s\"}",errCode,errMsg);
								goto error;
							}else {
								//strncpy(tmp_vid,vid+i,(i+1));
								strcpy(ssid_array[j],tmp_vid);//save the SSID
								
								//p_debug("ssid_array[%d]=%s\n",j,ssid_array[j]);
								sprintf(tmp_buf,"\"%s\",",tmp_vid);
				//				strcat(ret_buf,tmp_buf);
								memset(tmp_vid,0,strlen(tmp_vid));
							}
							j++;
							k=0;
						}
						else {
							tmp_vid[k]=ssid[i];
							k++;
						}
				}
				strcpy(ssid_array[j],tmp_vid);
				//p_debug("ssid_array[%d]=%s\n",j,ssid_array[j]);

				sprintf(tmp_buf,"\"%s\"",tmp_vid);
			//	strcat(ret_buf,tmp_buf);
			//	strcat(ret_buf,"],\"key\":[");
				memset(tmp_vid,0,strlen(tmp_vid));
				for(i=0,j=0,k=0;i<strlen(key);i++)
					{
						if(key[i]==',')
						{ 
							if(key[i-1]==','){// key is empty
								sprintf(key_array[j],"%s","");
							}else {
								//strncpy(tmp_vid,vid+i,(i+1));
								strcpy(key_array[j],tmp_vid);
								//p_debug("key_array[%d]=%s\n",j,key_array[j]);

								sprintf(tmp_buf,"\"%s\",",tmp_vid);
								//strcat(ret_buf,tmp_buf);
								memset(tmp_vid,0,strlen(tmp_vid));
							}
							j++;
							k=0;
						}
						else {
							tmp_vid[k]=key[i];
							k++;
						}
				}
				strcpy(key_array[j],tmp_vid);
				//p_debug("key_array[%d]=%s\n",j,key_array[j]);

				sprintf(tmp_buf,"\"%s\"",tmp_vid);
			//	strcat(ret_buf,tmp_buf);
			//	strcat(ret_buf,"],\"result\":\"0\"}}");				
			}
			//else{
			//	sprintf(buf,"{\"%s\":{\"%s\":\"%s\"}}",TASK_REMOVE,VID,);
			//}
		
			
		
		}else{
				sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"Not Admin\"}");
				goto error;
		} 

		save_ssid(ssid_array,key_array,ssid_num);
		

		sprintf(ret_buf,"{\"status\":1,\"data\":{},\"errorCode\":0,\"errorMessage\":\"success\"}");
		//p_debug("buf=====%s",ret_buf);

		if(strcmp(ssid,"")){
			for(i=0;i<ssid_num;i++){
			//p_debug("free %d.",i);
				
				free(ssid_array[i]);// = (char*)malloc(SSID_MAX_LEN);
				free(key_array[i]);//= (char*)malloc(SSID_MAX_LEN);
			}
			free(ssid_array);
			free(key_array);
		}


		if(isConnected()){
			//connected.
			p_debug("wifi is connected.");
		}
		else {
		p_debug("check connect.");
			//not connected.
			if(!strcmp(wifimode,"ap")) {
				p_debug("wifi_connect start.");
				system("wifi_connect & >/dev/null");
				
			}
			else 
			p_debug("wifi_connect start2.");
		}
error:
		printf("%s",ret_buf);
		fflush(stdout);
		free(web_str);
		
		free(ssid);
		free(key);		
		free(pssid);
		free(pkey);

		uci_free_context(ctx);

		return ;
}

