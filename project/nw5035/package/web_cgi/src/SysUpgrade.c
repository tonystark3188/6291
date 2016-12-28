#include <unistd.h> 
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>             
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>           
#include <sys/stat.h>
#include <signal.h>


#define FW_FILE "/tmp/fwupgrade"
#define FW_LEN 55574528
//#define FW_FILE "/mnt/disk-4/fwupgrade"
#define WINIE6_STR	"/octet-stream\x0d\x0a\0x0d\0x0a"
#define LINUXFX36_FWSTR "/x-ns-proxy-autoconfig\x0d\x0a\0x0d\0x0a"
#define MACIE5_FWSTR	"/macbinary\x0d\x0a\0x0d\0x0a"
#define OPERA_FWSTR	"/x-macbinary\x0d\x0a\0x0d\0x0a"
#define WIN64IE_FWSTR	"/plain\x0d\x0a\0x0d\0x0a"
#define LINE_FWSTR	"\x0d\x0a\0x0d\0x0a"

#define GZ_STR	"/x-gzip-compressed\x0d\x0a\0x0d\0x0a"   //22
#define CONF_HEADER                     ((char *)"conf")

#define MODEL_NAME "nw5025F_ac"

#define MALLOC_SIZE   (1024)

void logstr(char *str)
{		
#if 1
	int fw_fp;
	int f_size;
	if( (fw_fp=fopen("/tmp/feng.txt","a+"))==NULL)    // write and read,binary
	{
		exit(1);
	}		
	
	f_size=fwrite(str,1,strlen(str),fw_fp);

	fclose(fw_fp);
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
    if(strstr(upload_data, WINIE6_STR))
    {
       pStart = strstr(upload_data, WINIE6_STR);
       iestr_offset = 17;
	}
	else if(strstr(upload_data, GZ_STR))
    {
       pStart = strstr(upload_data, GZ_STR);
       iestr_offset = 22;
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
	  logstr("\nWIN64IE_FWSTR:");
	  logstr(WIN64IE_FWSTR);
      iestr_offset = 10;
	}else if(strstr(upload_data, "filename="))
	{
	    pStart = strstr(upload_data, "filename=");
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
	int fw_head_offset = 0;
	int len_no_header = 0;
	char mtd_fw[128]="\0";
	unsigned char *	fw_ptr = NULL;
	unsigned char *	fw_read = NULL;
	unsigned char* header = NULL;
	char op_fw_header[32]="\0";
	char product_model[32]="\0";
	int f_size_r =0, f_size_w = 0;;
	int fh=NULL;
	char error_str[512] = "\0";
	char tmp[64];
	int read_byte = 0;
	int try_times = 0;
	int tail_count=0;
	int true_len=0;
	int i;
	char line_buff[256]="\0";
	char md5_str1[40]="\0";
	char md5_str2[40]="\0";
	
	fprintf(stdout,"Content-type:text/html\r\n\r\n");
	//fprintf(stdout,"nidaye");
	system("rm -f /tmp/fwupgrade");
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");

	if( (fw_fp=fopen(FW_FILE,"wb+"))==NULL)    // write and read,binary
	{
	    logstr("there is a error,please reboot the device and upgrade again!\n");
		printf("there is a error,please reboot the device and upgrade again! <br>");
		fprintf(stdout,"</body>\n");
		fprintf(stdout,"\n</html>\n");
		exit(1);
	}
	fw_len=atoi(getenv("CONTENT_LENGTH"));
	fw_ptr=(unsigned char *)malloc(sizeof(unsigned char)*fw_len+1);
	if(fw_ptr==NULL)
	{
	    logstr("fw_ptr==NULL\n");
		printf("there is a error,please reboot the device and upgrade again! <br>");
		goto error_exit;
	}

	fw_read = fw_ptr;
	while(read_byte < fw_len)
	{
		f_size_r = fread(fw_read,1,MALLOC_SIZE,stdin);
		if(f_size_r == 0){
			logstr("fread zero\n");
			try_times++;
			if(try_times > 10000){//10s no receive data
				logstr("fread error\n");
				goto error_exit;
			}
			usleep(1000);
		}
		else if(f_size_r < 0){
			logstr("fread error\n");
			printf("there is a error,please reboot the device and upgrade again! <br>");
			goto error_exit;
		}
		else{
			read_byte += f_size_r;
			fw_read += f_size_r;
			try_times = 0;
		}
	}

	/*  find header */
	fw_head_offset=find_head_offset(fw_ptr);
	memset(tmp, 0, 64);
	sprintf(tmp, "fw_head_offset = %d\n",fw_head_offset);
	logstr(tmp);
	if(fw_head_offset==-1)
	{
	    logstr("fw_head_offset==-1");
		printf("there is a error,please reboot the device and upgrade again! <br>");
		goto error_exit;
	}
	header = fw_ptr+fw_head_offset;
	/*   cut the tail */
	len_no_header=fw_len-fw_head_offset;
	if( header[len_no_header-1]== 0x0a && header[len_no_header-2]== 0x0d )
	{
		i=len_no_header-2;
		while(header[i-1]!=0x0a && header[i-2]!=0x0d )
		{
			tail_count++;
			i--;
		}
	}
	else
	{
		logstr("no find tail\n");
		printf("there is a error,please reboot the device and upgrade again! <br>");
		goto error_exit;
	}
	tail_count+=4;
	true_len=fw_len-fw_head_offset-tail_count;

	#if 0
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
	#endif

	f_size_w = fwrite(header,1,true_len,fw_fp);
	if(f_size_w != true_len)
	{
		printf("write fail\n");
		printf("there is a error,please reboot the device and upgrade again! <br>");
		goto error_exit;
	}
	
	logstr("\nfree close\n");	

	if(fw_ptr != NULL)
	{
		free(fw_ptr);
		fw_ptr=NULL;
	}
	fclose(fw_fp);
	logstr("\ndone");

	// system("mv /tmp/fwupgrade /tmp/fwupgrade.gz");
	// system("gzip -d /tmp/fwupgrade.gz");
	system("tar -zxf /tmp/fwupgrade -C /tmp");
	system("mv /tmp/6291-update-fw.bin /tmp/fwupgrade");
	if( (fw_fp=fopen(FW_FILE,"rb"))==NULL)
	{
		system("rm -f /tmp/fwupgrade.gz");
		return 1;
		
	}
	fseek(fw_fp,0x20,SEEK_SET);
	fread(op_fw_header,1,32,fw_fp);
	fclose(fw_fp);

	system("md5sum /tmp/fwupgrade >/tmp/fwupgrade.md5");

	if( (fw_fp=fopen("/tmp/6291-update-fw.bin.md5","rb"))==NULL)
	{
		system("rm -f /tmp/6291-update-fw.bin.md5");
		return 1;
	}
	
	fgets(line_buff,256,fw_fp);
	char *p_stok_line=line_buff;
	char *p_stok_md5=NULL;
	p_stok_md5=strtok(p_stok_line, " ");
	strcpy(md5_str1,p_stok_md5);
	fclose(fw_fp);
	memset(line_buff,0,256);

	logstr(md5_str1);
	logstr("\n");

	if( (fw_fp=fopen("/tmp/fwupgrade.md5","rb"))==NULL)
	{
		system("rm -f /tmp/fwupgrade.md5");
		return 1;
	}
	fgets(line_buff,256,fw_fp);
	p_stok_line=line_buff;
	p_stok_md5=strtok(p_stok_line, " ");
	strcpy(md5_str2,p_stok_md5);
	fclose(fw_fp);

	logstr(md5_str2);
	logstr("\n");

	if(strcmp(md5_str1,md5_str2)!=0)
	{
		return 1;
	}


	if(strcmp(op_fw_header,MODEL_NAME)!=0){
		memset(tmp, 0, 64);
		sprintf(tmp, "op_fw_header = %s, MODEL_NAME = %s\n", op_fw_header, MODEL_NAME);
		logstr(tmp);
		return 1;
	}
	fprintf(stdout,"nidaye");
	return 0;
	
error_exit:
	logstr("error exit\n");
	system("rm -f /tmp/fwupgrade");
	system("sync");
	if(fw_ptr != NULL)
	{
		free(fw_ptr);
	}
	fclose(fw_fp);
	fprintf(stdout,"</body>\n");
	fprintf(stdout,"\n</html>\n");
	return 1;
	
}
