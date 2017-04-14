#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define ART_FILE "/tmp/boot_upgrade.bin"
#define uboot_dev "/dev/mtdblock0"
//#define FW_FILE "/mnt/disk-4/fwupgrade"
#define WINIE6_STR	"/octet-stream\x0d\x0a\0x0d\0x0a"
#define LINUXFX36_FWSTR "/x-ns-proxy-autoconfig\x0d\x0a\0x0d\0x0a"
#define MACIE5_FWSTR	"/macbinary\x0d\x0a\0x0d\0x0a"
#define OPERA_FWSTR	"/x-macbinary\x0d\x0a\0x0d\0x0a"
#define LINE_FWSTR	"\x0d\x0a\0x0d\0x0a"
#define CONF_HEADER                     ((char *)"conf")

//#define BOOT_END_FLAG "\x20\x2d\x2d\x2d\x2d\x2d\x2d"

#define BOOT_END_FLAG "------"

//#define BOOT_FLAG_ADDR		0x17000
//#define BOOT_FLAG			"AIRDISK"
#define NETCOM_LABLE "NETCOMAA"

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

unsigned int decode_boot(unsigned char *buf, int len)
{
	unsigned int dec;
	unsigned int dectmp;
	unsigned int *ibuf;
	unsigned int headlen;
	unsigned int i;

	if(len%4 != 0)
		return -1;

	if(memcmp(buf, NETCOM_LABLE, strlen(NETCOM_LABLE)) != 0)
		return -1;
	
	headlen = strlen(NETCOM_LABLE);
	dec = *(unsigned int *)(buf+headlen);
	ibuf = (unsigned int *)(buf+headlen+4);
	headlen = len -headlen-4;
	dectmp = 0;
	for(i=0; i<headlen/4; i++)
	{
		dectmp ^= *(ibuf+i);
	}
	if(dectmp != dec)
		return -1;

	return 0;
}

int getflagstr(unsigned char *buf , int len)
{
	char *strtag;
	int i=0;
	int taglen;
	int j;

	for(i; i< len; i++)
	{
		if(buf[i]==0x0d || buf[i]==0x0a)
			break;
	}

	if(i>= len)
		return;
	strtag = (char *)malloc(i+2);
	if(strtag != NULL)
	{
		memcpy(strtag, buf, i);
		strtag[i]=0;
	}
	//printf("strtag=%s<br>",strtag);
	taglen = strlen(strtag);

	j=len-taglen;
	while(1)
	{
		if(memcmp(buf+j, strtag, taglen) == 0)
		{
			break;
		}

		j--;
		if(j == 0)
			break;
	}

	if(j != 0)
	{
		//find the end
		//printf("%d",j);
		//printf("%c%c%c", *(buf+j-1), *(buf+j),*(buf+j+1));
		j--;
		if(*(buf+j) == 0x0d || *(buf+j) == 0x0a)
		{
			j--;
		}
		
		if(*(buf+j) == 0x0d || *(buf+j) == 0x0a)
		{
			j--;
		}	
		//printf("%d<br>",j);
		//printf("%c%c%c", *(buf+j-1), *(buf+j),*(buf+j+1));
		
			
	}
	free(strtag);
	return j;
}



int main()
{
	FILE *art_fp;
	int art_len;
	unsigned char * art_ptr;
	int art_head_offset=0;
	//const char * QUERYSTRING="method=FwUpgrade&result=0&image=/tmp/fwupgrade";
	//const char * up_grade_cmd="web_ssi Task";
	char mtd_fw[128]="\0";
	int f_size=0;
	
	fprintf(stdout,"Content-type:text/html\r\n\r\n");
	fprintf(stdout,"<html><title>boot upgrade</title>\n");
	fprintf(stdout,"<body>\n");
	
	if( (art_fp=fopen(ART_FILE,"wb+"))==NULL)    // write and read,binary
	{
		printf("open fw file failed\n");
		fprintf(stdout,"</body>\n");
		fprintf(stdout,"\n</html>\n");
		exit(1);
	}
	
	
	art_len=atoi(getenv("CONTENT_LENGTH"));
	art_ptr=(unsigned char *)malloc(sizeof(unsigned char)*art_len);
	if(art_ptr==NULL)
	{
		printf("<br>malloc failed <br>");
		goto error_exit;
	}
	//printf("<br> art_len =%d <br>",art_len);
	f_size=fread(art_ptr,1,art_len,stdin);
	if(f_size!=art_len)
	{
		printf("<br>fread failed <br>");
		goto error_exit;
	}
	
	//printf("%s<br>",art_ptr);
	//getflagstr(art_ptr,art_len);
	//goto error_exit;
	int boot_len=0;
	boot_len=getflagstr(art_ptr,art_len);  //boot_len的长度为从字符串开始去掉了尾部的长度
	//printf("%d<br>",boot_len);
	
	f_size=0;
	art_head_offset=find_head_offset(art_ptr);
	if(art_head_offset==-1)
	{
		printf("<br>find head offset failed <br>");
		goto error_exit;
	}
	art_ptr+=art_head_offset;
	//fwrite(art_ptr,1,art_len-art_head_offset,art_fp);
	//char *boot_end_ptr=NULL;
	//int boot_len=0;
	//boot_end_ptr=strstr(art_ptr,BOOT_END_FLAG);
	//boot_len=boot_end_ptr-art_ptr;
	//if(boot_end_ptr!=NULL)
	//printf("%s",boot_end_ptr);
	//printf("%d",boot_len);
	//goto error_exit;
	//printf("%d<br>",boot_len-art_head_offset);
	//printf("%s<br>",art_ptr);
	if(decode_boot(art_ptr, boot_len-art_head_offset+1)!=0)
	{
		printf("invalid uboot! <br>");
		goto error_exit;
	}
	int uboot_len=0;
	uboot_len=boot_len-art_head_offset+1-12;   //去掉头部，以及标志位和4字节的校验码
	
	f_size=fwrite(art_ptr+12,1,uboot_len,art_fp);
	if(f_size!=uboot_len)
	{
		printf("<br>fwrite failed <br>");
		goto error_exit;
	}
	
	
	
	fclose(art_fp);
	
	
	FILE *fp_mtd=NULL;
	fp_mtd=fopen(uboot_dev,"rb+");
	if(fp_mtd==NULL)
	{
		
		printf("open /dev/mtdblock0 failed\n");
		
		exit(1);
	}
	f_size=0;
	f_size=fwrite(art_ptr+12,1,uboot_len,fp_mtd);
	if(f_size!=uboot_len)
	{
		printf("<br>fwrite failed <br>");
		goto error_exit;
	}
	//printf("<br>uboot_len=%d<br>",uboot_len);
	fseek(fp_mtd, 0, SEEK_SET);
	unsigned char *read_buf=NULL;
	unsigned int en_num=0;
	unsigned int en_num_tmp=0;
	unsigned int *ibuf=NULL;
	int i=0;
	en_num= *(unsigned int *)(art_ptr+8);  //取出4字节的校验码
	int read_len=0;
	read_buf=(unsigned char *)malloc(uboot_len);
	memset(read_buf,0,uboot_len);
	read_len=fread(read_buf,1,uboot_len,fp_mtd);
	if(read_len!=uboot_len)
	{
		printf("<br>read uboot from mtdblock0 failed <br>");
		goto error_exit;
	}
	ibuf=(unsigned int *)(read_buf); 
	for(i=0; i<uboot_len/4; i++)
	{
		en_num_tmp ^= *(ibuf+i);
	}
	if(en_num_tmp!=en_num)
	{
		printf("<br>uboot check failed <br>");
		goto error_exit;
	}
	
	fclose(fp_mtd);
	free(art_ptr);
	free(read_buf);
	
	
	
	//sprintf(mtd_fw,"mtd write %s u-boot",ART_FILE);
	//system(mtd_fw);
	
	//setenv("MYQUERYSTRING",QUERYSTRING,1);
	//system(up_grade_cmd);
	printf("<br>boot upgrade successful :)<br>");
	fprintf(stdout,"</body>\n");
	fprintf(stdout,"\n</html>\n");
	return 0;
error_exit:
	printf("<br>boot upgrade failed :(<br>");
	fprintf(stdout,"</body>\n");
	fprintf(stdout,"\n</html>\n");
	return 1;
	
}
