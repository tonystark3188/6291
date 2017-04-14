#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>




#define WINIE6_STR	"/octet-stream\x0d\x0a\0x0d\0x0a"
#define LINUXFX36_FWSTR "/x-ns-proxy-autoconfig\x0d\x0a\0x0d\0x0a"
#define MACIE5_FWSTR	"/macbinary\x0d\x0a\0x0d\0x0a"
#define OPERA_FWSTR	"/x-macbinary\x0d\x0a\0x0d\0x0a"
#define LINE_FWSTR	"\x0d\x0a\0x0d\0x0a"
#define CONF_HEADER                     ((char *)"conf")

#define WEB_PARTITION_LEN (256*1024)

#define WEB_HEADER "webf"
#define WEB_DEV "/dev/mtdblock8"
#define WEB_FILE "/tmp/webfile"

/* Firmware image file header */
typedef struct img_header {
	unsigned char signature[4];//webf
	unsigned int startAddr;
	unsigned int burnAddr;
	unsigned int len;
} IMG_HEADER_T, *IMG_HEADER_Tp;

typedef IMG_HEADER_T WEB_HEADER_T;

void logstr(char *str)
{		
#if 1
	int fd;
	int f_size;
	if( (fd=fopen("/tmp/feng.txt","a+"))==NULL)    // write and read,binary
	{
		exit(1);
	}		
	
	f_size=fwrite(str,1,strlen(str),fd);

	fclose(fd);
#endif
}

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

static int read_flash_webpage(char *prefix, char *webfile)
{

printf("checksum ok!\n");
}

int CHECKSUM_OK(unsigned char *data, int len)
{
	int i;
	unsigned char sum=0;
	WEB_HEADER_T header;

	for (i=0; i<len; i++)
		sum += data[i];

	if (sum == 0)
		return 1;
	else
		return 0;
}
#define DWORD_SWAP(v) ( (((v&0xff)<<24)&0xff000000) | ((((v>>8)&0xff)<<16)&0xff0000) | \
				((((v>>16)&0xff)<<8)&0xff00) | (((v>>24)&0xff)&0xff) )

unsigned int check_web(unsigned char *buf)
{
/*	unsigned int i;
	i=strlen(buf);
	logstr(i);

	if(strlen(buf) != WEB_PARTITION_SIZE){
		logstr("error web size\n");
		logstr(i);
		return -1;
	}
*/	WEB_HEADER_T header;
	char * fw_ptr;
	char tmpstr[100];
	int fd;
	unsigned int f_size; 

	//检查文件头
	if(memcmp(buf,WEB_HEADER,sizeof(WEB_HEADER))){
		logstr("wrong web header\n");
		return -3;
	}
	
	//读入文件头结构
	fd = open(WEB_FILE, O_RDWR|O_CREAT|O_TRUNC,0777);
	if(fd == NULL)    //read,binary
	{
		//printf("open /dev/mtdblock5 failed\n");
		logstr("1 open webfile failed.\n");
		return -1;
	}
	
	fw_ptr=(unsigned char *)malloc(WEB_PARTITION_LEN);
	memcpy(fw_ptr,buf,WEB_PARTITION_LEN);
	
	f_size=write(fd,fw_ptr,WEB_PARTITION_LEN);
	if(f_size != WEB_PARTITION_LEN)
		{
		logstr("webfile save error \n");
		return -2;
	}
	sync();
	lseek(fd,0,SEEK_SET);
	if ((f_size=read(fd, &header, sizeof(header))) != sizeof(header)) {
		sprintf(tmpstr,"sizeof header=%d,f_size=%d",sizeof(header),f_size);
		logstr(tmpstr);
		close(fd);
		return -1;
	}
	header.len = DWORD_SWAP(header.len);

	sprintf(tmpstr,"head.len=%d",header.len);
	logstr(tmpstr);
	close(fd);

	if(header.len>WEB_PARTITION_LEN)
		{
		logstr("WEB file too large!\n");
		return -5;
	}
	
	if ( !CHECKSUM_OK((unsigned char *)(fw_ptr+sizeof(header)), header.len) ) {
		logstr("Web image invalid!\n");
		free(fw_ptr);
		return -4;
	}
	logstr("check sum ok\n");

	free(fw_ptr);
	
	return 0;
}


int main()
{
	FILE *art_fp;
	int art_len;
	unsigned char * art_ptr;
	unsigned char * tmp_ptr;
	unsigned char md5_value[128];
	int art_head_offset=0;
	//const char * QUERYSTRING="method=FwUpgrade&result=0&image=/tmp/fwupgrade";
	//const char * up_grade_cmd="web_ssi Task";
	
	int f_size=0;
	
	fprintf(stdout,"Content-type:text/html\r\n\r\n");
	fprintf(stdout,"<html><title>art upgrade</title>\n");
	fprintf(stdout,"<body>\n");
	
	//分配内存
	art_len=atoi(getenv("CONTENT_LENGTH"));
	
	tmp_ptr=(unsigned char *)malloc(sizeof(unsigned char)*art_len);
	art_ptr=tmp_ptr;

	if(art_ptr==NULL)
	{
		logstr("malloc failed \n");
		goto error_exit;
	}
	//printf("<br> art_len =%d <br>",art_len);

	//存入内存
	f_size=fread(art_ptr,1,art_len,stdin);
	if(f_size!=art_len)
	{
		logstr("fread failed \n");
		goto error_exit;
	}
	f_size=0;

	//找到文件内容开头
	art_head_offset=find_head_offset(art_ptr);
	if(art_head_offset==-1)
	{
		logstr("find head offset failed \n");
		goto error_exit;
	}
	art_ptr+=art_head_offset;
	//fwrite(art_ptr,1,art_len-art_head_offset,art_fp);


	//校验文件是否合法	
	if(check_web(art_ptr)!=0)
	{
		printf("<br>Wrong WEB file<br>");
		goto error_exit;
	}
	
	//开始写入文件
	FILE *fp_mtd=NULL;
	fp_mtd=fopen(WEB_DEV,"rb+");
	if(fp_mtd==NULL)
	{
		logstr("open /dev/mtdblock6 failed\n");
		goto error_exit;
	}
	memset(md5_value,0,128);
	fseek(fp_mtd, -80L, SEEK_END);
	f_size = fread(md5_value,1,80,fp_mtd); //save the md5 value and version info
	if(f_size!=80)
	{
		logstr("fread failed \n");
		goto error_exit;
	}
	fseek(fp_mtd, 0, SEEK_SET);
	f_size=0;
/**/	
	f_size=fwrite(art_ptr,1,WEB_PARTITION_LEN,fp_mtd);
	if(f_size!=WEB_PARTITION_LEN)
	{
		logstr("fwrite failed \n");
		goto error_exit;
	}
	fseek(fp_mtd, -80L, SEEK_END);
	f_size=fwrite(md5_value,1,80,fp_mtd);
	if(f_size!=80)
	{
		logstr("fwrite failed \n");
		goto error_exit;
	}
	logstr("upgrade done\n");
	fclose(fp_mtd);
	free(tmp_ptr);

	printf("<br>WEB upgrade successful :)<br>");
	fprintf(stdout,"</body>\n");
	fprintf(stdout,"\n</html>\n");
	return 0;
error_exit:
	printf("<br>WEB upgrade failed :(<br>");
	fprintf(stdout,"</body>\n");
	fprintf(stdout,"\n</html>\n");
	return 1;
	
}
