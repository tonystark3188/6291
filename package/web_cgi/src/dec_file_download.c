/*
 * =============================================================================
 *
 *       Filename:  dec_file_download.c
 *
 *    Description:  decrypt file download cgi.
 *
 *        Version:  1.0
 *        Created:  2017/3/17 10:54
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ifeng@dmsys.com
 *   Organization:  
 *
 * =============================================================================
 */
#include <openssl/aes.h>

#include "msg.h"
#include "router_task.h"
#include "jz_aes_v12.h"
#include "random-string.h"
#include "md5.h"
#include "encrypt_file.h"

//#define AES_HW
#define LETV_ENCRYPT 
#define AES_SW 

#ifndef AES_HW

#include <openssl/aes.h>

#endif
int _handle_client_json_req(ClientTheadInfo *client_info)
{
 //   return api_process(client_info);
}

#define TYPE_VIDEO "0"
#define TYPE_VIDEO_COVER "1"
#define TYPE_ALBUM_COVER "2"

#define ROOT_FILE_PATH "/tmp/mnt/USB-disk-1/safebox/"
#define SAFE_BOX_PATH  "/tmp/mnt/USB-disk-1/safebox"
#define BUF_LEN 65536 //32768

#define VIDEO_BUF_LEN 65536 // 32768 //65536  //131072 //524288//524288 1MB 1048576  4MB 4194304

int parse_range(char *buf, long long *range0, long long *range1)
{
#define MAX_RANGE 32 // 10^16

	p_debug("buf is %s", buf);

	//p_debug("Entry parse_range.........");
	char *p = strstr(buf, "=");
	char *q = strstr(buf, "-");
	char R0[MAX_RANGE] = {0};
	char R1[MAX_RANGE] = {0};

	*range0 = 0;
	*range1 = 0;
	
	if(p == NULL || q == NULL)
	{
		return 0;
		
	}else
	{
		p++; // after '='
		while(*p == ' ')
			p++;		

		int num = q-p;

		if(num < MAX_RANGE)
		{
			strncpy(R0, p, num);
			*range0 = atoll(R0);
			p_debug("range0 is %s(%lld),num = %d", R0,*range0, num);
		}else
		{
			*range0 = 0;
		}

		q = q+1;
		strcpy(R1,q);
		p_debug("rang1 is (%s),strlen=%d",R1,strlen(R1));
		*range1 = atoll(R1);

		p_debug("range0 is %lld, range1 is %lld(%lld)", *range0, *range1,atoll(R1));
		
	}

	return 0;
}

int destory_task_list(struct task_dnode *dn)
{
	unsigned i = 0;
	struct task_dnode *head;
	if(dn == NULL)
		return;
	for(i = 0;/*!dn*/;i++)
	{
		head = dn->dn_next;
		free(dn);
		if(!head)
			break;
		dn = head;
	}
	p_debug("free succ");
	return 0;
}
#define AES_BLOCK_SIZE	16
#define AES_KEY_LEN 32

#define MAX_DMA_LEN (AES_BLOCK_SIZE*256)
int aes_fd;


int get_md5sum(char *src,char *result,int len){
	int i = 0;
	yasm_md5_context context;
	unsigned char checksum[16]="\0";;
	char tmp[64]="\0";
	if(result == NULL || src == NULL)
	{
		return -1;
	}
	
	memset(checksum, 0, 16);
    yasm_md5_init (&context);
	yasm_md5_update (&context, src, strlen (src));
	yasm_md5_final (checksum, &context);
	memset(tmp, 0, 64);
	for(i = 0; i < 16; i++)
	{
		//p_debug ("%02x", (unsigned int) checksum[i]);
 		sprintf(tmp+2*i, "%02x", (unsigned int) checksum[i]);
	}
	//printf("result=%s\n",tmp);
	//strncpy(result, tmp,len);
	strncpy(result, tmp,len);
	result[len]='\0';
	//printf("result2=%s\n",result);
	return 0;
}

int safe_free(char *p)
{
	if(p!=NULL)
		{free(p);p=NULL;}
}

unsigned int str_to_uint(char *s){

	//p_debug("s=%s",s);
	if(strlen(s) != 8) return -1;

	unsigned int ui[8];
	int i=0;
	for(i=0;i<8;i++){
		if(s[i]>='a')
			ui[i]=s[i]-87;
		else
			ui[i]=s[i]-'0';
		
		//p_debug("s=%c,ui[%d]=%x",s[i],i,ui[i]);
		
	}
	//p_debug("ui[0]=%x",ui[0]<<28);
	//p_debug("ui[1]=%x",ui[1]<<24);
	//p_debug("ui[2]=%x",ui[2]<<20);	
	unsigned int uin=ui[0]<<28|ui[1]<<24|ui[2]<<20|ui[3]<<16|ui[4]<<12|ui[5]<<8|ui[6]<<4|ui[7];
	//p_debug("uin=%x",uin);

	return uin;
}

int gen_k(char *k){
//	char *mac="84:5d:d7:00:11:22";
	//char *mac="84:5D:D7:A1:12:27";
	char mac[18]={0};
	char *sn="84:5D:D7:A1:12:27";
	int i=0;
	
    FILE *mac_fp=NULL;
	if((mac_fp=fopen("/etc/mac.txt","r"))!=NULL)
	{
	        fread(mac,1,17,mac_fp);
	        fclose(mac_fp);
	}else {
		return -1;
	}
//	char *k[16]={0};
	get_md5sum(mac,k,AES_KEY_LEN);
	p_debug("k=%s",k);
	//unsigned int key[4] = {0xd55fda32,0x14d99bd7,0x9f7a0ef8,0x972df216};
	char tmp[8]={0};
	unsigned int key[4];

	char *p=k;
	for(i=0;i<4;i++){
		strncpy(tmp,p,8);
		tmp[8]='\0';
		p=p+8;
		key[i]=str_to_uint(tmp);
		p_debug("k[%d]=%x",i,key[i]);
	}

}


int decrypt_fd(FILE *fp_read,unsigned long start,unsigned long end)
{
	p_debug("access decrypt_fd");
	unsigned int key[4] = {0xd55fda32,0x14d99bd7,0x9f7a0ef8,0x972df216};
	unsigned int iv[4] = {0x7f444fd5,0xd2002d29,0x4b96c34d,0xc57d297e};
	char *k=(char*)malloc(AES_KEY_LEN);
	memset(k,0,AES_KEY_LEN);
	if(gen_k(k)<0){
		return -1;
	}

	struct aes_key aes_key;
	struct aes_data aes_data;
	int ret = 0;
	long fsize = 0;
	int rd_byte = 0, wr_byte = 0;
	int fd;
	aes_key.key = (char *)k;
	aes_key.keylen = 16;
	aes_key.bitmode = AES_KEY_128BIT;
	aes_key.aesmode = AES_MODE_ECB;
	aes_key.iv = (char *)iv;
	aes_key.ivlen = 16;
	aes_key.encrypt = 1;


	if (fseek(fp_read, 0, SEEK_END) == -1) {
		printf("fseek SEEK_END error!\n");
		return -1;
	}

	fsize = ftell(fp_read);
	if (fsize == -1) {
		printf("ftell!\n");
		return -1;
	}

	if (fsize == 0){
		printf("fsize error!\n");
		return -1;
	}

	unsigned char input[MAX_DMA_LEN];
	unsigned char output[MAX_DMA_LEN];
	unsigned int align_len = 0;
	unsigned int pad, len, i;

	len = fsize;


	unsigned long start_bytes=(start/MAX_DMA_LEN)*MAX_DMA_LEN;
	unsigned long end_bytes=(end/MAX_DMA_LEN+1)*MAX_DMA_LEN;
	unsigned long need_read_block_size=((end/MAX_DMA_LEN)-(start/MAX_DMA_LEN)+1)*MAX_DMA_LEN;
	unsigned long  need_read_bytes;
	unsigned long end_block_num=(end/MAX_DMA_LEN);
	unsigned long start_block_num=(start/MAX_DMA_LEN);
	
	if(len==end)
		need_read_bytes=len;
	else 
		need_read_bytes=end-start+1;

	
	fseek(fp_read,start_bytes,SEEK_SET);	
	//len-=start_bytes;
	p_debug("len=%d,start_block_num=%d,end_block_num=%d,need_bytes=%d.start_bytes=%d",len,start_block_num,end_block_num,need_read_bytes,start_bytes);
	
	int isReadFirstBlock=1;
	unsigned long send_bytes=0;
	
#ifdef AES_HW

	fd = open("/dev/jz-aes", O_RDWR);
	if(fd < 0) {
		p_debug("open error!");
		return -1;
	}

	aes_fd = fd;


	ret = ioctl(aes_fd, AES_LOAD_KEY, &aes_key);
	if(ret < 0) {
		p_debug("ioctl! AES_LOAD_KEY");
	}

#else
	AES_KEY aes;

    if (AES_set_decrypt_key(k, 128, &aes) < 0) {
        fprintf(stderr, "Unable to set decryption key in AES\n");
        exit(-1);
    }

#endif


	if(end_block_num>start_block_num){//在不同的块
		//
		while(need_read_block_size){
			rd_byte = fread(input, 1, MAX_DMA_LEN, fp_read);
			if (!rd_byte) {
				p_debug("fread! error\n");
				return -1;
			}
#ifdef AES_HW			
			aes_data.input = input;
			aes_data.input_len = rd_byte;
			aes_data.output = output;

			ret = ioctl(aes_fd, AES_DO_CRYPT, &aes_data);
			if(ret < 0) {
					p_debug("ioctl decrypt error");
			}
#else
			char aes_input_block[AES_BLOCK_SIZE]={0};
			char aes_output_block[AES_BLOCK_SIZE]={0};
		    
			
			for(i=0;i<MAX_DMA_LEN;i=i+AES_BLOCK_SIZE){
				memcpy(aes_input_block,input+i,AES_BLOCK_SIZE);
				AES_ecb_encrypt(aes_input_block, aes_output_block, &aes, AES_DECRYPT);
				memcpy(output+i,aes_output_block,AES_BLOCK_SIZE);
			}

#endif			
			//p_debug("need_read_block_size=%d",need_read_block_size);

			need_read_block_size -= MAX_DMA_LEN;
		
			if(isReadFirstBlock){

				wr_byte = fwrite(output+(start%MAX_DMA_LEN), 1, rd_byte-(start%MAX_DMA_LEN), stdout);
				//p_debug("wr_byte=%d",wr_byte);
				if (!wr_byte) {
					p_debug("fwrite! error\n");
					return -1;
				}
				isReadFirstBlock=0;
				need_read_bytes-=(rd_byte-(start%MAX_DMA_LEN));
//				p_debug("1. first read wr_byte=%d,need=%d",wr_byte,need_read_bytes);
			}else 
			if(need_read_bytes>=MAX_DMA_LEN)
			{
			
				wr_byte = fwrite(output, 1, rd_byte, stdout);
				//p_debug("wr_byte=%d",wr_byte);
				if (!wr_byte) {
					p_debug("fwrite! error\n");
					return -1;
				}
				need_read_bytes-=rd_byte;

//				p_debug("2. read wr_byte=%d,need=%d",wr_byte,need_read_bytes);
			}else {//last read
			
				wr_byte = fwrite(output, 1, need_read_bytes, stdout);

				if (!wr_byte) {
					p_debug("fwrite! error\n");
					return -1;
				}
				need_read_bytes-=wr_byte;
//				p_debug("3.last read wr_byte=%d need_read_bytes=%d",wr_byte,need_read_bytes);				
			}

		}
		
	
	}else {// 在同一块 4096上,一次读完
	
	
		while(need_read_block_size) {
				rd_byte = fread(input, 1, MAX_DMA_LEN, fp_read);
				if (!rd_byte) {
					p_debug("fread! error\n");
					return -1;
				}

				//p_debug("rd_byte=%d",rd_byte);
#ifdef AES_HW
				aes_data.input = input;
				aes_data.input_len = rd_byte;
				aes_data.output = output;

				ret = ioctl(aes_fd, AES_DO_CRYPT, &aes_data);
				if(ret < 0) {
						p_debug("ioctl decrypt error");
				}
				
				p_debug("need_read_block_size=%d",need_read_block_size);
#else
				char aes_input_block[AES_BLOCK_SIZE]={0};
				char aes_output_block[AES_BLOCK_SIZE]={0};


				for(i=0;i<MAX_DMA_LEN;i=i+AES_BLOCK_SIZE){
					//p_debug("i,%d",i);
					memcpy(aes_input_block,input+i,AES_BLOCK_SIZE);
					AES_ecb_encrypt(aes_input_block, aes_output_block, &aes, AES_DECRYPT);
					memcpy(output+i,aes_output_block,AES_BLOCK_SIZE);
				}

#endif
				need_read_block_size -= MAX_DMA_LEN;
			
				
				wr_byte = fwrite(output, 1, need_read_bytes, stdout);
				p_debug("wr_byte=%d",wr_byte);
				if (!wr_byte) {
					p_debug("fwrite! error\n");
					return -1;
				}
						
		}
	}

////////////////////////////////////////////////////////////////////////
	/*
	while(need_read_block_size) {
		rd_byte = fread(input, 1, MAX_DMA_LEN, fp_read);
		if (!rd_byte) {
			printf("fread! error\n");
			return -1;
		}

		//p_debug("rd_byte=%d",rd_byte);

		aes_data.input = input;
		aes_data.input_len = rd_byte;
		aes_data.output = output;

		ret = ioctl(aes_fd, AES_DO_CRYPT, &aes_data);
        if(ret < 0) {
                printf("ioctl decrypt error");
        }
		
		p_debug("need_read_block_size=%d",need_read_block_size);

		need_read_block_size -= rd_byte;

		
		if (need_read_block_size == 0) {//last block
			pad = output[rd_byte - 1];
			p_debug("pad=%d",pad);			
			if (pad > AES_BLOCK_SIZE) {
				p_debug("pad error\n");
				return -1;
			}
			if (!(rd_byte - pad))
				continue;
			//p_debug("last output=%s",output);

			if(send_bytes==0)
				{
				wr_byte = fwrite(output+(start%MAX_DMA_LEN), 1, (end-start) + 1 - pad, stdout);
				send_bytes+=((end-start) + 1 - pad);
				need_read_bytes-=send_bytes;
				p_debug("need_read_bytes===%d",need_read_bytes);
			}
			else {
				if((need_read_bytes)>rd_byte)
				{
					wr_byte=fwrite(output, 1, rd_byte-pad, stdout);
					send_bytes+=rd_byte;
					need_read_bytes-=send_bytes;					
					p_debug("need_read_bytes===%d",need_read_bytes);
				}
				else {
					wr_byte=fwrite(output, 1,need_read_bytes , stdout);	
					p_debug("need_read_bytes===%d",need_read_bytes);
				}


			}
			if (!wr_byte) {
				p_debug("last fwrite! error\n");
				return -1;
			}
			send_bytes+=((end-start) + 1 - pad);
			fflush(stdout);
		}
		else {
			p_debug("need_read_block_size=%d",need_read_block_size);

			if((need_read_bytes)<MAX_DMA_LEN){
				wr_byte=fwrite(output+(start % MAX_DMA_LEN), 1, (end-start)+1, stdout);
				send_bytes+=((end-start)+1);
				need_read_bytes-=send_bytes;
				p_debug("need_read_bytes===%d",need_read_bytes);
				p_debug("send_bytes===%d",send_bytes);
			}else// if((end-start)>=DMA_MAX_LEN)
			{
				if(isReadFirstBlock) {	
					wr_byte=fwrite(output+(start % MAX_DMA_LEN), 1, (rd_byte-(start % MAX_DMA_LEN)), stdout);
					send_bytes+=(rd_byte-(start % MAX_DMA_LEN));
					isReadFirstBlock=0;	
					need_read_bytes-=send_bytes;
					p_debug("need_read_bytes===%d",need_read_bytes);
					p_debug("send_bytes===%d",send_bytes);
				}else{
					if((end-send_bytes)>rd_byte)
					{
						wr_byte=fwrite(output, 1, rd_byte, stdout);
						send_bytes+=rd_byte;
						need_read_bytes-=send_bytes;
						p_debug("need_read_bytes===%d",need_read_bytes);

						p_debug("send_bytes===%d",send_bytes);
					}
					else {
						wr_byte=fwrite(output, 1, need_read_bytes, stdout);	
						//need_read_bytes-=send_bytes;
						p_debug("need_read_bytes===%d",need_read_bytes);						
						p_debug("send_bytes===%d",send_bytes);
					}
				}

			}
			if (!wr_byte) {
				p_debug("fwrite! error\n");
				return -1;
			}

			fflush(stdout);
		}
	}
	*/
	/////////////////////////////////////////////////////
	safe_free(k);
	close(fd);
	return fsize;
}

int display_node(struct	encrypted_file *dn){
	int i=0;
	struct encrypted_file *p=dn;
	for(i;;i++){
		if(!p) break;
		p_debug("p->name=%s,dest_path=%s",p->name,p->dest_path);
		p=p->next;
	}
}

int read_from_record(struct	encrypted_file **dn)//读入数据
 {	 


	  FILE *record_fd;
	  int enRet;
	  struct encrypted_file *cur=NULL;
	  int ret = 0;
	  int i = 0;
	  if(access(ENC_DATA_PATH,R_OK)!=0){// 不存在
		  if((record_fd = fopen(ENC_DATA_PATH,"wb+"))==NULL)//改动：路径
		  {
			// DMCLOG_D("task list file does not exist error1[errno:%d]\n",errno);
			 return -1;
		  }
	  }else  if((record_fd = fopen(ENC_DATA_PATH,"r"))==NULL)//改动：路径
	  {
		 //DMCLOG_D("task list file does not exist error2[errno:%d]\n",errno);
		 return -1;
	  }
	  
	  while(!feof(record_fd))
	  {
		 cur = (struct encrypted_file *)malloc(sizeof(*cur));
		 memset(cur,0,sizeof(*cur));
		 cur->next=NULL;
		 enRet = fread(cur,sizeof(struct encrypted_file),1,record_fd);
		 if(enRet <= 0)
	     {	
	     	if(cur!=NULL)free(cur);
			//DMCLOG_D("fread error,enRet = %d,errno = %d\n",enRet,errno);
			break;
		 }
		//DMCLOG_D("NAME=%s/%s",cur->src_path,cur->dest_path);
		if(!cur)
		{
			//DMCLOG_D("malloc task_dn error\n");
			return -2;
		}
		cur->next = *dn;
		*dn = cur;
		 i++;
	  }
	  fclose(record_fd);


	  return ret;
 }


int destroy_record(struct encrypted_file *dn)
{
	
	unsigned i = 0;
	struct encrypted_file *head;
	if(dn == NULL)
		return;
	for(i = 0;/*!dn*/;i++)
	{
		head = dn->next;
		free(dn);
		if(!head)
			break;
		dn = head;
	}
	//printf("free succ\n");
	return 0;
}

char * find_the_real_file(char *src){
	//char tmpsrc[1024]={0};
	//sprintf(tmpsrc,"%s/%s",SAFE_BOX_PATH,src);
	//src= a/b/c.mp4
//	/tmp/mnt/USB-disk-1/xxx/a/b/c.mp4
	struct encrypted_file *p= e_file_list;
	int i=0;
	for(i=0;;i++){
		if(!p) break;
		if(!strcmp(p->name,src))
		{
			p_debug("p->dest_path=%s",p->dest_path);
			return p->dest_path;
		}
		p=p->next;
	}
	return NULL;
}
void main()
{
		char ret_buf[RET_BUF_LEN];
		char code[CODE_LEN_65]="\0";
		char save_code[CODE_LEN_65]="\0";		
		char sid[SID_LEN]="\0";
		char fw_sid[SID_LEN]="\0";
		char type[32]="\0";
		char vid[VID_LEN_33]="\0";
		char aid[VID_LEN_33]="\0";		
		char tmp_buf[256]="\0";
		char tmp_vid[VID_LEN_33]="\0";
		char path[512]="\0";
		int i,j,k;
		char *web_str=NULL;
		unsigned int ret=0;
		char ip[32]="\0";
		char uci_option_str[128]="\0";
		
		long long read_bytes=0;
		long long read_len=0;
		long long write_len=0;
		FILE  *fd;
		char range[32]="\0";
		char str_start[32]="\0";
		long long start=0;
		long long end=0;
		long long need_read_len=0;
		long long fsize=0;
		char led_status[8]="\0";
		char key[65]={0};
		char name[512]={0};	
		char mime[32]={0};


		//p_debug(getenv("HTTP_USER_AGENT"));
		//p_debug(getenv("PATH_INFO"));
		//p_debug(getenv("CONTENT_TYPE"));
	//	p_debug(getenv("REMOTE_ADDR"));

		if(getenv("HTTP_HOST")!=NULL)
			strcpy(ip,getenv("HTTP_HOST"));
		else p_debug("get HTTP_HOST error!");

		if((web_str=GetStringFromWeb())==NULL)
		{
			printf("Content-type:text/plain\r\n\r\n");
			fprintf(stdout,"can't get string from web\n");
			exit(1);
		}
		processString(web_str,"key",key);		
		
		processString(web_str,"name",name);		
		p_debug("name=%s",name);
		read_from_record(&e_file_list);
		//if(!strcmp(sid,fw_sid)||!strcmp(save_code,code)){//是管理员，或者是有效访客
		display_node(e_file_list);

			if(1){
				
				if(getenv("HTTP_RANGE")==NULL){
					printf("Status:200 OK\r\n");
			
					//strcpy(path,"/tmp/mnt/USB-disk-1/safe/.b80a86fddfe383f010e8");
					strcpy(path,find_the_real_file(name));
				
					fd=fopen(path,"rb");
					if(fd==NULL){
						printf("Status:404 Not Found\r\n");
						p_debug("open %s failed.",path);
						free(web_str);

						
						destroy_record(e_file_list);
						return;
					}
					fseek(fd, 0, SEEK_END);
					fsize = ftell(fd);
					rewind(fd);
					
					
					printf("Accept-Ranges: bytes\r\n");

					printf("Content-Range: bytes 0-%lld/%lld\r\n",fsize-1,fsize); 
					p_debug("name=%s",name);
					strcpy(mime,get_mime_type(name, strlen(name)));
					p_debug("mime=%s",mime);
					printf("Content-Type: %s\r\n",mime);
					fprintf(stdout,"Content-Length: %lld\r\n\r\n",fsize);
					fflush(stdout);
				
					p_debug(getenv("HTTP_USER_AGENT"));					
					decrypt_fd(fd,0,fsize);
					free(web_str);

					

					destroy_record(e_file_list);
					fclose(fd);
					return;
				}
				else{
					printf("Status: 206 Partial Content\r\n");
					//p_debug("start 206 Partial Content download,");
					strcpy(range,getenv("HTTP_RANGE"));
					//p_debug("range====%s==",range);
					parse_range(range,&start,&end);
					p_debug("start=%lld,end=%lld",start,end);

				
					//strcpy(path,"/tmp/mnt/USB-disk-1/safe/.b80a86fddfe383f010e8");
					strcpy(path,find_the_real_file(name));	
					fd=fopen(path,"rb");
					if(fd==NULL){
						printf("Status: 404 Not Found\r\n");
						//printf("open %s failed.",path);
						free(web_str);
							
						destroy_record(e_file_list);						
						return;
					}
					fseek(fd, 0, SEEK_END);
					fsize = ftell(fd);
					rewind(fd);
					
					fseek(fd,start,SEEK_SET);

					if((end != 0)){ 						
						need_read_len=end-start+1;
						//sprintf(stdout,"Content-Length:%lu",need_read_len);
						fprintf(stdout,"Content-length: %lld\r\n",need_read_len);
						fflush(stdout);//sprintf(stdout,"Content-Length:%lu",need_read_len);
						

						printf("Content-Range: bytes %lld-%lld/%lld\r\n",start,end,fsize);
			//			p_debug("Content-Range: bytes %lld-%lld/%lld\r\n",start,end,fsize);		
					}
					else {
						need_read_len=fsize-start;
						fprintf(stdout,"Content-length: %lld\r\n",need_read_len);
						fflush(stdout);//sprintf(stdout,"Content-Length:%lu",need_read_len);
						printf("Content-Range: bytes %lld-%lld/%lld\r\n",start,fsize-1,fsize);		
			//			p_debug("Content-Range: bytes %lld-%lld/%lld\r\n",start,fsize-1,fsize);		
					}

			//		p_debug("need_read_len=%lu",need_read_len);

					strcpy(mime,get_mime_type(name, strlen(name)));
					printf("Content-Type: %s\r\n\r\n",mime);
				
					decrypt_fd(fd,start,need_read_len+start-1);
		
					fclose(fd);	
				//	p_debug("access turn_led_on0..");					
					
					free(web_str);
					destroy_record(e_file_list);					
					return;
					
				
					}				
			}
		
		//printf("%s",ret_buf);
		fflush(stdout);
		free(web_str);
		destroy_record(e_file_list);
		return ;
}

