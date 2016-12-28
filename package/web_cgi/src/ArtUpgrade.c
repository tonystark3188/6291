#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


#define WINIE6_STR	"/octet-stream\x0d\x0a\0x0d\0x0a"
#define LINUXFX36_FWSTR "/x-ns-proxy-autoconfig\x0d\x0a\0x0d\0x0a"
#define MACIE5_FWSTR	"/macbinary\x0d\x0a\0x0d\0x0a"
#define OPERA_FWSTR	"/x-macbinary\x0d\x0a\0x0d\0x0a"
#define LINE_FWSTR	"\x0d\x0a\0x0d\0x0a"
#define CONF_HEADER                     ((char *)"conf")

#define ART_SIZE 0X10000

#define NETCOM_LABLE "NETCOMBB"
#define ART_DEV "/dev/mtdblock4"


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


int main()
{
	FILE *art_fp;
	int art_len;
	unsigned char * art_ptr;
	int art_head_offset=0;
	//const char * QUERYSTRING="method=FwUpgrade&result=0&image=/tmp/fwupgrade";
	//const char * up_grade_cmd="web_ssi Task";
	
	int f_size=0;
	
	fprintf(stdout,"Content-type:text/html\r\n\r\n");
	fprintf(stdout,"<html><title>art upgrade</title>\n");
	fprintf(stdout,"<body>\n");
	
	
	
	
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
	f_size=0;
	art_head_offset=find_head_offset(art_ptr);
	if(art_head_offset==-1)
	{
		printf("<br>find head offset failed <br>");
		goto error_exit;
	}
	art_ptr+=art_head_offset;
	//fwrite(art_ptr,1,art_len-art_head_offset,art_fp);
	int art_en_len=ART_SIZE+12;
	if(decode_boot(art_ptr, art_en_len)!=0)
	{
		printf("invalid art! <br>");
		goto error_exit;
	}
	
	
	
	
	FILE *fp_mtd=NULL;
	fp_mtd=fopen(ART_DEV,"rb+");
	if(fp_mtd==NULL)
	{
		
		printf("open /dev/mtdblock4 failed\n");
		
		goto error_exit;
	}
	f_size=0;
	f_size=fwrite(art_ptr+12,1,ART_SIZE,fp_mtd);
	if(f_size!=ART_SIZE)
	{
		printf("<br>fwrite failed <br>");
		goto error_exit;
	}
	fseek(fp_mtd, 0, SEEK_SET);
	unsigned char *read_buf=NULL;
	unsigned int en_num=0;
	unsigned int en_num_tmp=0;
	unsigned int *ibuf=NULL;
	int i=0;
	en_num= *(unsigned int *)(art_ptr+8);  //取出4字节的校验码
	int read_len=0;
	read_buf=(unsigned char *)malloc(ART_SIZE);
	memset(read_buf,0,ART_SIZE);
	read_len=fread(read_buf,1,ART_SIZE,fp_mtd);
	if(read_len!=ART_SIZE)
	{
		printf("<br>read uboot from mtdblock4 failed <br>");
		goto error_exit;
	}
	ibuf=(unsigned int *)(read_buf);
	for(i=0; i<ART_SIZE/4; i++)
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
	
	
	
	//setenv("MYQUERYSTRING",QUERYSTRING,1);
	//system(up_grade_cmd);
	printf("<br>ART upgrade successful :)<br>");
	fprintf(stdout,"</body>\n");
	fprintf(stdout,"\n</html>\n");
	return 0;
error_exit:
	printf("<br>ART upgrade failed :(<br>");
	fprintf(stdout,"</body>\n");
	fprintf(stdout,"\n</html>\n");
	return 1;
	
}
