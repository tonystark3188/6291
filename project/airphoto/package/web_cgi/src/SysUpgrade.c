#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define FW_FILE "/tmp/fwupgrade"
//#define FW_FILE "/mnt/disk-4/fwupgrade"
#define WINIE6_STR	"/octet-stream\x0d\x0a\0x0d\0x0a"
#define LINUXFX36_FWSTR "/x-ns-proxy-autoconfig\x0d\x0a\0x0d\0x0a"
#define MACIE5_FWSTR	"/macbinary\x0d\x0a\0x0d\0x0a"
#define OPERA_FWSTR	"/x-macbinary\x0d\x0a\0x0d\0x0a"
#define LINE_FWSTR	"\x0d\x0a\0x0d\0x0a"
#define CONF_HEADER                     ((char *)"conf")


int find_head_offset(char *upload_data)
{
	int head_offset=0 ;
	char *pStart=NULL;
	int iestr_offset=0;
	char *dquote;
	char *dquote1;
	
	if (upload_data==NULL) {
		//fprintf(stderr, "upload data is NULL\n");
		return -1;
	}

	pStart = strstr(upload_data, WINIE6_STR);
	if (pStart == NULL) {
		pStart = strstr(upload_data, LINUXFX36_FWSTR);
		if (pStart == NULL) {
			pStart = strstr(upload_data, MACIE5_FWSTR);
			if (pStart == NULL) {
				pStart = strstr(upload_data, OPERA_FWSTR);
				if (pStart == NULL) {
					pStart = strstr(upload_data, "filename=");
					if (pStart == NULL) {
						return -1;
					}
					else {
						dquote =  strstr(pStart, "\"");
						if (dquote !=NULL) {
							dquote1 = strstr(dquote, LINE_FWSTR);
							if (dquote1!=NULL) {
								iestr_offset = 4;
								pStart = dquote1;
							}
							else {
								return -1;
							}
						}
						else {
							return -1;
						}
					}
				}
				else {
					iestr_offset = 16;
				}
			} 
			else {
				iestr_offset = 14;
			}
		}
		else {
			iestr_offset = 26;
		}
	}
	else {
		iestr_offset = 17;
	}
	//fprintf(stderr,"####%s:%d %d###\n",  __FILE__, __LINE__ , iestr_offset);
	head_offset = (int)(((unsigned long)pStart)-((unsigned long)upload_data)) + iestr_offset;
	//printf("<br>head_offset=%d \n",head_offset);
	return head_offset;
}

int check_fw_sum(unsigned char *buf, int len)
{
	unsigned int dec;
	unsigned int dectmp;
	unsigned int *ibuf;
	unsigned int headlen;
	unsigned int i;
	
	if(len%4 != 0)
		return 0;
	dec = *(unsigned int *)(buf+(0x7c0000-4));
	if(dec==0xffffffff)
		return 1;
	ibuf = (unsigned int *)(buf);
	dectmp = 0;
	headlen=0x7c0000-4;
	for(i=0; i<headlen/4; i++)
	{
		dectmp ^= *(ibuf+i);
	}
	if(dectmp != dec)
		return 0;

	return 1;
}

int main()
{
	FILE *fw_fp;
	int fw_len;
	unsigned char * fw_ptr;
	int fw_head_offset=0;
	//const char * QUERYSTRING="method=FwUpgrade&result=0&image=/tmp/fwupgrade";
	//const char * up_grade_cmd="web_ssi Task";
	char mtd_fw[128]="\0";
	char op_fw_header[32]="\0";
	int f_size=0;
	
	fprintf(stdout,"Content-type:text/html\r\n\r\n");
	fprintf(stdout,"nidaye");
	
	system("rm -f /tmp/fwupgrade");
	system("sync");
	system("killall perl");
	system("killall checknren");
	system("killall minidlna");
	system("killall nrender");
	system("killall avahi-daemon");
	system("killall telnetd");
	system("killall miniupnpd");
	system("killall wpa_supplicant");
	system("killall wanlan");
	system("killall udhcpc");
	system("killall hotplug2");
	system("killall dbus-daemon");
	system("killall wifi_save");
	system("killall logger");
	system("echo 3 > /proc/sys/vm/drop_caches");
	sleep(2);
	system("echo 3 > /proc/sys/vm/drop_caches");
	sleep(2);
	if( (fw_fp=fopen(FW_FILE,"wb+"))==NULL)    // write and read,binary
	{
		printf("there is a error,please reboot the device and upgrade again! <br>");
		fprintf(stdout,"</body>\n");
		fprintf(stdout,"\n</html>\n");
		exit(1);
	}
	
	
	fw_len=atoi(getenv("CONTENT_LENGTH"));
	fw_ptr=(unsigned char *)malloc(sizeof(unsigned char)*fw_len);
	if(fw_ptr==NULL)
	{
		printf("there is a error,please reboot the device and upgrade again! <br>");
		goto error_exit;
	}
	//printf("<br> fw_len =%d <br>",fw_len);
	f_size=fread(fw_ptr,1,fw_len,stdin);
	if(f_size!=fw_len)
	{
		printf("there is a error,please reboot the device and upgrade again! <br>");
		goto error_exit;
	}
	
	char *error_str_tmp1=NULL;
	char *error_str_tmp2=NULL;
	int error_str_cnt=0;
	char error_str[128]="\0";
	if((error_str_tmp1=strstr(fw_ptr, "name"))!=NULL)
	{
		error_str_tmp2=error_str_tmp1+6;
		while((*error_str_tmp2)!='\"')
		{
			error_str_cnt++;
			error_str_tmp2++;
		}
		if(error_str_cnt>0)
			strncpy(error_str,error_str_tmp1+6,error_str_cnt);
		//printf("%s<br>",error_str);
	}
	
	//printf("%s",fw_ptr);
	//goto error_exit;
	f_size=0;
	fw_head_offset=find_head_offset(fw_ptr);
	if(fw_head_offset==-1)
	{
		printf("there is a error,please reboot the device and upgrade again! <br>");
		goto error_exit;
	}
	fw_ptr+=fw_head_offset;
	//fwrite(fw_ptr,1,fw_len-fw_head_offset,fw_fp);
	//printf("post_len=%x <br>",fw_len);
	//printf("fw_len=%x <br>",fw_len-fw_head_offset);
	
	//goto error_exit;
	//if(fw_len-fw_head_offset != 8126510)
	//{
	//	printf("firmware is error! <br>");
	//	goto error_exit;
	//}
	//goto error_exit;
	strncpy(op_fw_header,fw_ptr+4,7);
	if( strcmp(op_fw_header,"OpenWrt")!=0 )
	{
		printf("%s <br>",error_str);
		goto error_exit;
	}
	if(check_fw_sum(fw_ptr,0x7c0000)==0)  //check firmware sum
	{
		printf("there is a error,please reboot the device and upgrade again! <br>");
		goto error_exit;
	}
	//goto error_exit;
	f_size=fwrite(fw_ptr,1,0x7c0000,fw_fp);
	if(f_size!=0x7c0000)
	{
		printf("there is a error,please reboot the device and upgrade again! <br>");
		goto error_exit;
	}
	
	fseek(fw_fp,4,SEEK_SET);
	fread(op_fw_header,1,7,fw_fp);
	
	//printf("<br>%s<br>",op_fw_header);
	
	
	fclose(fw_fp);
	free(fw_ptr);
	
	if( strstr(op_fw_header,"OpenWrt")==NULL )
	{
		printf("%s <br>",error_str);
		goto error_exit;
	}
	

//	system("sysupdate");
	
	//setenv("MYQUERYSTRING",QUERYSTRING,1);
	//system(up_grade_cmd);


	return 0;
error_exit:
	system("rm -f /tmp/fwupgrade");
	system("sync");
	free(fw_ptr);
	fprintf(stdout,"</body>\n");
	fprintf(stdout,"\n</html>\n");
	return 1;
	
}
