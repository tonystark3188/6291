#include <unistd.h> 
#include <stdio.h> 
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>             
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>           
#include <sys/stat.h>
#include <signal.h>
#include "my_debug.h"

//Maximum 13 bytes
#define PRODUCT_MODEL  "letv"
#define FW_LEN 8060928
#define SAVE_FILE "/tmp/mnt/USB-disk-1/public"
#define WINIE6_STR	"/octet-stream\x0d\x0a\0x0d\0x0a"
#define LINUXFX36_FWSTR "/x-ns-proxy-autoconfig\x0d\x0a\0x0d\0x0a"
#define MACIE5_FWSTR	"/macbinary\x0d\x0a\0x0d\0x0a"
#define OPERA_FWSTR	"/x-macbinary\x0d\x0a\0x0d\0x0a"

#define WIN64IE_FWSTR	"/plain\x0d\x0a\0x0d\0x0a"

#define LINE_FWSTR	"\x0d\x0a\0x0d\0x0a"
#define CONF_HEADER                     ((char *)"conf")

#define MALLOC_SIZE   (1024)

#if 0

void p_debug(char *str)
{		

	int fw_fp;
	int f_size;
	if( (fw_fp=fopen("/tmp/feng.txt","a+"))==NULL)    // write and read,binary
	{
		exit(1);
	}		
	
	f_size=fwrite(str,1,strlen(str),fw_fp);

	fclose(fw_fp);

}
#endif

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
    if(strstr(upload_data, WINIE6_STR))
    {
       pStart = strstr(upload_data, WINIE6_STR);
       iestr_offset = 17;
	}else if(strstr(upload_data, LINUXFX36_FWSTR))
	{
	   pStart = strstr(upload_data, LINUXFX36_FWSTR);
       iestr_offset = 26;
	}else if(strstr(upload_data, MACIE5_FWSTR))
	{
	  pStart = strstr(upload_data, MACIE5_FWSTR);
      iestr_offset = 14;
	}else if(strstr(upload_data, OPERA_FWSTR))
	{
	  pStart = strstr(upload_data, OPERA_FWSTR);
      iestr_offset = 16;
	}else if(strstr(upload_data, WIN64IE_FWSTR))
	{
	  pStart = strstr(upload_data, WIN64IE_FWSTR);
      iestr_offset = 10;
	}
	else
	if(strstr(upload_data, "filename="))
	{
	    pStart = strstr(upload_data, "filename=");
#if 0
		char *fnStart=strstr(pStart,"\"");
		char *fnEnd=strstr(fnStart+1,"\"");
		char *filename=malloc(fnEnd-fnStart);
		memset(filename,0,fnEnd-fnStart);
		strncpy(filename,fnStart+1,(fnEnd-fnStart-1));
		
		p_debug("get file name=%s",filename);
#endif		
#if 1
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
#endif		
	}
	
    //fprintf(stderr,"####%s:%d %d###\n",  __FILE__, __LINE__ , iestr_offset);
	head_offset = (int)(((unsigned long)pStart)-((unsigned long)upload_data)) + iestr_offset;
	//printf("<br>head_offset=%d \n",head_offset);
	return head_offset;
}


int main()
{
	FILE *fw_fp;
	int fw_len;
	int fw_head_offset=0;
	char mtd_fw[128]="\0";
	char op_fw_header[32]="\0";
	char product_model[32]="\0";
	int f_size=0;
	int fh=NULL;
	char error_str[512] = "\0";
	int cnt = 0;
	fprintf(stdout,"Content-type:text/html\r\n\r\n");
	//fprintf(stdout,"nidaye");
	
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");


	fw_len=atoi(getenv("CONTENT_LENGTH"));
	unsigned char *	fw_ptr=(unsigned char *)malloc(MALLOC_SIZE);
	if(fw_ptr==NULL)
	{
	    p_debug("fw_ptr==NULL");
		printf("there is a error,please reboot the device and upgrade again! <br>");
		goto error_exit;
	}

	f_size=fread(fw_ptr,1,MALLOC_SIZE,stdin);
	if(f_size!=MALLOC_SIZE)
	{
		printf("there is a error,please reboot the device and upgrade again! <br>");
		goto error_exit;
	}
	
	p_debug("http header=%s",fw_ptr);
	fw_head_offset=find_head_offset(fw_ptr);

	char *tagE=strstr(fw_ptr, LINE_FWSTR);
	char *tagS=fw_ptr;
	int tagLen=(tagE-tagS);
	char *tag=malloc(tagE-tagS+1);
	memset(tag,0,tagE-tagS+1); 
	strncpy(tag,tagS,(tagE-tagS));
	p_debug("tagLen=%d,tag=%s",tagLen,tag);

    char *pStart = strstr(fw_ptr, "filename=");
	char *fnStart=strstr(pStart,"\"");
	char *fnEnd=strstr(fnStart+1,"\"");
	char *filename=malloc(fnEnd-fnStart);
	memset(filename,0,fnEnd-fnStart);
	strncpy(filename,fnStart+1,(fnEnd-fnStart-1));
	p_debug("filename=%s",filename);
	int savefilelen=strlen(SAVE_FILE)+strlen(filename);
	char *savefile=malloc(savefilelen+1);
	memset(savefile,0,savefilelen+1);
	
	sprintf(savefile,"%s/%s",SAVE_FILE,filename);

	if( (fw_fp=fopen(savefile,"wb+"))==NULL)    // write and read,binary
	{
	    p_debug("there is a error,please reboot the device and upgrade again!");
		fprintf(stdout,"\n</html>\n");
		exit(1);
	}

		
	unsigned char* header = fw_ptr+fw_head_offset;
	//strncpy(op_fw_header,header+37,7);
	//p_debug(op_fw_header);
	/*
	if( strcmp(op_fw_header,"OpenWrt")!=0 )
	{
	    p_debug("op_fw_header12");
		strcpy(error_str,"bad op_fw_header");
		printf("%s <br>",error_str);
		goto error_exit;
	}
	strncpy(product_model,header+0x33,13);
	product_model[13] = '\0';
	if( strcmp(product_model,PRODUCT_MODEL)!=0 )
	{
		strcpy(error_str,"bad product_model");
		printf("%s <br>",error_str);
		goto error_exit;
	}
	*/
	f_size=fwrite(header,1,(MALLOC_SIZE-fw_head_offset),fw_fp);
	if(f_size!=(MALLOC_SIZE-fw_head_offset))
	{
		printf("there is a error,please reboot the device and upgrade again! <br>");
		goto error_exit;
	}
	cnt = (fw_len - MALLOC_SIZE) / MALLOC_SIZE;
	while(cnt > 0)
	{
		cnt--;
		f_size=fread(fw_ptr,1,MALLOC_SIZE,stdin);
		if(f_size!=MALLOC_SIZE)
		{
			printf("there is a error,please reboot the device and upgrade again! <br>");
			goto error_exit;
		}
		f_size=fwrite(fw_ptr,1,(MALLOC_SIZE),fw_fp);
		if(f_size!=(MALLOC_SIZE))
		{
			printf("there is a error,please reboot the device and upgrade again! <br>");
			goto error_exit;
		}
	}
	cnt = (fw_len - MALLOC_SIZE) % MALLOC_SIZE;//-(tagLen+4+2);
	f_size=fread(fw_ptr,1,cnt,stdin);
	if(f_size!=cnt)
	{
		printf("there is a error,please reboot the device and upgrade again! <br>");
		goto error_exit;
	}
	f_size=fwrite(fw_ptr,1,(cnt-(tagLen+4+2)),fw_fp);
	if(f_size!=(cnt-(tagLen+4+2)))
	{
		printf("there is a error,please reboot the device and upgrade again! <br>");
		goto error_exit;
	}

	p_debug("\nfree close\n");	
	#if 0
	fseek(fw_fp,0,SEEK_END);
	fw_len = ftell(fw_fp);
	if(fw_len < FW_LEN)
	{	
		printf("fw length error\n");
		fclose(fw_fp);
		goto error_exit;
	}
	#endif
	if(fw_ptr != NULL)
	{
		free(fw_ptr);
		fw_ptr=NULL;
	}
	fclose(fw_fp);

	if(savefile != NULL)free(savefile);
	if(filename != NULL)free(filename);
	if(tag != NULL)free(tag);


	fprintf(stdout,"success");
	fflush(stdout);
	p_debug("\ndone");	
	return 0;
	
error_exit:
//	system("rm -f /tmp/mnt/USB-disk-1/fwupgrade");
	system("sync");
	if(fw_ptr != NULL)
	{
		free(fw_ptr);
	}
	if(savefile != NULL)free(savefile);
	if(filename != NULL)free(filename);
	if(tag != NULL)free(tag);
	fprintf(stdout,"</body>\n");
	fprintf(stdout,"\n</html>\n");
	return 1;
	
}

