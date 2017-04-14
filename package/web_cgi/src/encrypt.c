/*
* aes.cc
* - Show the usage of AES encryption/decryption
*/
 
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include <string.h>

#include <openssl/aes.h>
#include "my_debug.h"
#include "random-string.h"
#include "jz_aes_v12.h"
#include "md5.h"

#define AES_KEY_LEN 32
#define AES_BLOCK_SIZE	16
#define MAX_DMA_LEN	16384
#define MAX_FILE_PATH_LEN 512
#define MAX_FILE_NAME_LEN 256
#define SAFE_BOX_PATH  "/tmp/mnt/USB-disk-1/.safebox"
#define e_file_path  "/tmp/mnt/USB-disk-1/.safebox/.e"

#define ENCRYPTED_FILE_NAME_LENGTH 20

typedef struct encrypted_file{
	struct encrypted_file *dn_next;
	char name[MAX_FILE_NAME_LEN];
	char orig[MAX_FILE_PATH_LEN];
	char encry[MAX_FILE_PATH_LEN];
	unsigned long size;
	time_t mod_time;
}EncrypFile;

struct	encrypted_file *e_file_list;


int get_file_info(char *path, struct encrypted_file *dn){

	char *p=strrchr(path,'\/');
	strncpy(dn->name,p,MAX_FILE_NAME_LEN);
	struct stat statbuff;  
    if(stat(path, &statbuff) < 0){  
		p_debug("stat file error");
        return -1;  
    }else{  
    	p_debug("stat file ok");
		p_debug("statbuff.st_size=%d",statbuff.st_size);
        dn->size = statbuff.st_size;  
		printf("dn-size=%d\n",dn->size);
		p_debug("size=%ld",dn->size);
		dn->mod_time=statbuff.st_mtime;
		p_debug("mtime=%d",dn->mod_time);
    } 	
}

int write_list_to_file()//写出数据
{
	char path[128]="\0";
	//p_debug("access write_list_to_file");	

	struct encrypted_file *dn=e_file_list;

	unsigned i = 0;
	int enRet = 0;
	FILE *record_fd=NULL;
	
	memset(path,"0",128);
	strcpy(path,e_file_path);

	//display_task_dnode(dn);
	if((record_fd = fopen(path,"w+"))== NULL)//改动：路径
	{
		p_debug("open file error");
		return -1;
	}

	for(i = 0;/*count - detected via !dn below*/;i++)
	{
		

			if(!dn)
			{
				//p_debug("task link is NULL");
				break;
			}
			//p_debug("save tasklist pid/vid=%s/%s",dn->pid,dn->vid);
	
		//p_debug("i = %d,dn->vid = %s",i,dn->vid);
		//p_debug("i = %d,dn->vid_path = %s",i,dn->vid_path);


			//usleep(100000);
			enRet = fwrite(dn,sizeof(struct encrypted_file),1,record_fd);
			if(enRet>0){		
				//system("cp /tmp/mnt/USB-disk-1/letv/.tasklist /tmp/mnt/USB-disk-1/letv/.tasklist.bak");
				//usleep(100000);
			}
	
		dn = dn->dn_next;


	}
	fclose(record_fd);

	return 0;
}

int add_to_record(struct encrypted_file **pdn,char *src, char *dest)
{
	p_debug("access add_to_record");

	int i;
    char tmpbuf[128];
	
	struct encrypted_file *dn=e_file_list;
	struct encrypted_file *cur=(struct encrypted_file *)malloc(sizeof(struct encrypted_file));
	memset(cur,0,sizeof(struct encrypted_file));


	int write_ret;

	if(get_file_info(src,cur)<0){
		p_debug("get_file_info error.");
	}
	if(strlen(src)<MAX_FILE_PATH_LEN) 
		strcpy(cur->orig,src);	
	else 
		return -1;

	if(strlen(dest)<MAX_FILE_PATH_LEN) 
		strcpy(cur->encry,dest);	
	else 
		return -1;
	
	for(i=0;;i++){
		if(!dn)break;
		if((!strcmp((dn)->orig,cur->orig))){
		//	if(type==1){//更新自动下载的地址
				p_debug("already encrypted?");
				p_debug("save orig=%s,encry=%s",dn->orig,dn->encry);
				safe_free(cur);
				return -2;

			}
			else//if(pdn->isDeleted==0) 手动添加
			{
				
				//write_list_to_file(VIDEO);
				//saveOneTaskToFile(dn);
				//p_debug("save orig=%s,encry=%s",dn->orig,dn->encry);
				//system("cp -f /tmp/mnt/USB-disk-1/letv/.tasklist /tmp/mnt/USB-disk-1/letv/.tasklist.bak");
				//usleep(100000);
			}
			//safe_free(cur);
			//p_debug("leave add_to_record");
			//return 1;
			dn=(dn)->dn_next;
	}
	cur->dn_next = *pdn;
	*pdn = cur;

	write_list_to_file();
	p_debug("leave add_to_record");
	return 0;
}

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
//	printf("result2=%s\n",result);
	return 0;
}


unsigned int str_to_uint(char *s){

	p_debug("s=%s",s);
	if(strlen(s) != 8) return -1;

	unsigned int ui[8];
	int i=0;
	for(i=0;i<8;i++){
		if(s[i]>='a')
			ui[i]=s[i]-87;
		else
			ui[i]=s[i]-'0';
		
		p_debug("s=%c,ui[%d]=%x",s[i],i,ui[i]);
		
	}
	p_debug("ui[0]=%x",ui[0]<<28);
	p_debug("ui[1]=%x",ui[1]<<24);
	p_debug("ui[2]=%x",ui[2]<<20);	
	unsigned int uin=ui[0]<<28|ui[1]<<24|ui[2]<<20|ui[3]<<16|ui[4]<<12|ui[5]<<8|ui[6]<<4|ui[7];
	p_debug("uin=%x",uin);

	return uin;
}


int gen_k(char *k){
//	char *mac="84:5d:d7:00:11:22";
	char *mac="84:5D:D7:A1:12:27";
	char *sn="84:5D:D7:A1:12:27";
	int i=0;
//	char *k[16]={0};
	get_md5sum(mac,k,AES_KEY_LEN);
	p_debug("k=%s",k);
	//unsigned int key[4] = {0xd55fda32,0x14d99bd7,0x9f7a0ef8,0x972df216};
	char tmp[8]={0};
	unsigned int key[4];

	p_debug("int %d",sizeof(int));
	p_debug("char %d",sizeof(char));
	p_debug("long %d",sizeof(long));		
	p_debug("uint %d",sizeof(unsigned int));		

	char *p=k;
	for(i=0;i<4;i++){
		strncpy(tmp,p,8);
		tmp[8]='\0';
		p=p+8;
		key[i]=str_to_uint(tmp);
		p_debug("k[%d]=%x",i,key[i]);
	}

}
int safe_free(char *p)
{
	if(p!=NULL)
		{free(p);p=NULL;}
}


int get_filename_from_path(char *abs_file_path,char *name){

	char *p=strrchr(abs_file_path,'/');
	printf("p=%s",p);
	if(p)
	{
		strncpy(name,p+1,(abs_file_path+strlen(abs_file_path)-p));
		p_debug("filename=%s",name);

	}
	else return -1;

}

int generate_dest_file_path(char *src, char *dest){

	char *random=NULL;//=(char*)malloc(ENCRYPTED_FILE_NAME_LENGTH);
//	randomString(random,ENCRYPTED_FILE_NAME_LENGTH);

	random=base64_encode(src,strlen(src));	
	if(random !=NULL)	
		p_debug("random=(%s)",random);	
	p_debug("src=%s",src);	

	char *dest_name=(char*)malloc(ENCRYPTED_FILE_NAME_LENGTH+1);
	memset(dest_name,0,ENCRYPTED_FILE_NAME_LENGTH+1);
	p_debug("src=%s",src);	
	get_md5sum(random,dest_name,ENCRYPTED_FILE_NAME_LENGTH);

//	p_debug("random[%d]=(%s)",random_len,random);
	p_debug("src=%s",src);
	//dest=(char*)malloc(strlen(src)+2);
	sprintf(dest,"%s/.%s",SAFE_BOX_PATH,dest_name);

	p_debug("dest=%s",dest);
//	free(random);
	return 0;
}

int encrypt_fd(FILE *fp_read)
{
	int fd;
    AES_KEY aes;

	unsigned char input[AES_BLOCK_SIZE ];
	unsigned char output[AES_BLOCK_SIZE];
	unsigned char input_block[MAX_DMA_LEN];
	unsigned char output_block[MAX_DMA_LEN];

	unsigned int align_len = 0;
	unsigned int pad, len, i;
	char *k=(char*)malloc(AES_KEY_LEN);
	memset(k,0,AES_KEY_LEN);
	gen_k(k);

	
	unsigned int key[4] = {0xd55fda32,0x14d99bd7,0x9f7a0ef8,0x972df216};
	unsigned int iv[4] = {0x7f444fd5,0xd2002d29,0x4b96c34d,0xc57d297e};
	for(i;i<4;i++){
		p_debug("key[%d]=%x",i,key[i]);
	}

	p_debug("k=%s",k);

	struct aes_key aes_key;
	struct aes_data aes_data;
	int ret = -1;
	long fsize = 0;
	int rd_byte = 0, wr_byte = 0;

	aes_key.key = (char *)k;
	aes_key.keylen = 16;
	aes_key.bitmode = AES_KEY_128BIT;
	aes_key.aesmode = AES_MODE_ECB;
	aes_key.iv = (char *)iv;
	aes_key.ivlen = 16;
	aes_key.encrypt = 0;

	if (fseek(fp_read, 0, SEEK_END) == -1) {
		printf("fseek SEEK_END error!\n");
		return -1;
	}

	fsize = ftell(fp_read);
	if (fsize == -1) {
		printf("ftell!\n");
		return -1;
	}

	if (fseek(fp_read, 0, SEEK_SET) == -1) {
		printf("fseek SEEK_SET error!\n");
		return -1;
	}

	if (fsize == 0){
		printf("fsize error!\n");
		return -1;
	}

	len = fsize;

    if (AES_set_encrypt_key(k, 128, &aes) < 0) {
        fprintf(stderr, "Unable to set encryption key in AES\n");
        exit(-1);
    }


	while(len) {
		rd_byte = fread(input_block, 1, MAX_DMA_LEN, fp_read);
		if (!rd_byte) {
			printf("fread! error\n");
			close(fd);
			return -1;
		}

		if (!(len - rd_byte)) {		//last read
			align_len = ((rd_byte / AES_BLOCK_SIZE) + 1) * AES_BLOCK_SIZE;

			pad = AES_BLOCK_SIZE - rd_byte % AES_BLOCK_SIZE;
			for (i = rd_byte; i < align_len; i++) {
				input_block[i] = pad;
//				printf("PKCS5:input[%d]=%02x ",i,input[i]);
			}
//			printf("\n");

			aes_data.input = input;
			aes_data.input_len = rd_byte + pad;
			aes_data.output = output;
			for(i=0;i<MAX_DMA_LEN;i=i+AES_BLOCK_SIZE){
					memcpy(input,input_block+i,AES_BLOCK_SIZE);
					
					AES_ecb_encrypt(input, output, &aes, 
						AES_ENCRYPT);
					
					memcpy(output_block+i,output,AES_BLOCK_SIZE);
			}

		//	ret = ioctl(aes_fd, AES_DO_CRYPT, &aes_data);
			if (fseek(fp_read, fsize-len, SEEK_SET) == -1) {
				printf("fseek SEEK_SET error!\n");
				return -1;
			}

			wr_byte = fwrite(output_block, 1, rd_byte + pad, fp_read);
			if (!wr_byte) {
				printf("last fwrite! error\n");
				close(fd);				
				return -1;
			}
		}
		else {

			for(i=0;i<MAX_DMA_LEN;i=i+AES_BLOCK_SIZE){
					memcpy(input,input_block+i,AES_BLOCK_SIZE);
					
					AES_ecb_encrypt(input, output, &aes, 
						AES_ENCRYPT);
					
					memcpy(output_block+i,output,AES_BLOCK_SIZE);
			}
			if (fseek(fp_read, fsize-len, SEEK_SET) == -1) {
				printf("fseek SEEK_SET error!\n");
				return -1;
			}
			wr_byte = fwrite(output_block, 1, rd_byte, fp_read);
			if (!wr_byte) {
				close(fd);
				printf("fwrite! error\n");
				return -1;
			}

		}
		len -= rd_byte;
		
	}			
	close(fd);
	safe_free(k);
	return fsize;
}

int decrypt_fd(FILE *fp_read, FILE *fp_write)
{
	p_debug("access decrypt_fd");
	unsigned int key[4] = {0xd55fda32,0x14d99bd7,0x9f7a0ef8,0x972df216};
	unsigned int iv[4] = {0x7f444fd5,0xd2002d29,0x4b96c34d,0xc57d297e};
	char *k=(char*)malloc(AES_KEY_LEN);
	memset(k,0,AES_KEY_LEN);
	gen_k(k);

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
	
    AES_KEY aes;


	if (fseek(fp_read, 0, SEEK_END) == -1) {
		printf("fseek SEEK_END error!\n");
		return -1;
	}

	fsize = ftell(fp_read);
	if (fsize == -1) {
		printf("ftell!\n");
		return -1;
	}
	//rewind(fp_read);

//	if (fseek(fp_read, 0, SEEK_SET) == -1) {
//		printf("fseek SEEK_SET error!\n");
//		return -1;
//	}

	if (fsize == 0){
		printf("fsize error!\n");
		return -1;
	}

	unsigned char input[AES_BLOCK_SIZE];
	unsigned char output[AES_BLOCK_SIZE];
	unsigned char input_block[MAX_DMA_LEN];
	unsigned char output_block[MAX_DMA_LEN];

	unsigned int align_len = 0;
	unsigned int pad, len, i;

	len = fsize;
	p_debug("access decrypt_fd");

	int start=0;
	int end=17;
	int start_bytes=(start/16+1)*16;
	int end_bytes=(end/16+1)*16;
	int need_read_bytes=end_bytes-start_bytes;
	fseek(fp_read,start,SEEK_SET);	
	//slen-=start_bytes;
	//p_debug("len=%d",len);

    if (AES_set_decrypt_key(k, 128, &aes) < 0) {
        fprintf(stderr, "Unable to set decryption key in AES\n");
        exit(-1);
    }

	
	while(len) {
		rd_byte = fread(input_block, 1, MAX_DMA_LEN, fp_read);
		if (!rd_byte) {
			printf("fread! error\n");
			return -1;
		}

		//p_debug("rd_byte=%d",rd_byte);

		aes_data.input = input;
		aes_data.input_len = rd_byte;
		aes_data.output = output;

		for(i=0;i<MAX_DMA_LEN;i=i+16){
			memcpy(input,input_block+i,AES_BLOCK_SIZE);
			AES_ecb_encrypt(input, output, &aes, 
				AES_DECRYPT);
			memcpy(output_block+i,output,AES_BLOCK_SIZE);
		}
		
		
	/*	
		for(i=0;i<16;i++){
			p_debug("input[%d]=%x",i,input[i]);
	//		p_debug("output[%d]=%x",i,output[i]);
		}
		for(i=0;i<16;i++){
//			p_debug("input[%d]=%x",i,input[i]);
			p_debug("output[%d]=%x",i,output[i]);
		}
		*/

		

		len -= rd_byte;
	//	p_debug("1 output=%s",output);

		if (len == 0) {
			pad = output_block[rd_byte - 1];
			p_debug("pad=%d",pad);			
			if (pad > AES_BLOCK_SIZE) {
				printf("pad error\n");
				return -1;
			}
			if (!(rd_byte - pad))
				continue;
		//	p_debug("last output=%s",output);

			
			wr_byte = fwrite(output_block, 1, rd_byte - pad, fp_write);
			if (!wr_byte) {
				printf("last fwrite! error\n");
				return -1;
			}
		}
		else {
		//	p_debug("output=%s",output);
			wr_byte = fwrite(output_block, 1, rd_byte, fp_write);
			if (!wr_byte) {
				printf("fwrite! error\n");
				return -1;
			}
		}
	}
	safe_free(k);
	close(fd);
	return fsize;
}

int decrypt_file(char *abs_file_path){
	char dest_path[MAX_FILE_PATH_LEN]="/tmp/mnt/USB-disk-1/safe/1.txt";
	FILE *fp_read, *fp_write;

	fp_read = fopen(abs_file_path, "r");
	if (!fp_read) {
		printf("fopen %s!\n",abs_file_path);
		return -1;
	}
	fp_write = fopen(dest_path, "wb+");
	if (!fp_write) {
		p_debug("fp_write fopen (%s)errno %d!\n",dest_path,errno);
		return -1;
	}

	if(decrypt_fd(fp_read,fp_write)<0)
	{
		fclose(fp_read);
		fclose(fp_write);
		printf("file_decrypt error!\n");
		return -1;
	}else{//encrypt success
		fclose(fp_read);
		fclose(fp_write);
		p_debug("file_decrypt success2!\n");
	}	
	return 0;
}

int encrypt_file(char *abs_file_path){
	
	FILE *fp_read, *fp_write;
	char name[MAX_FILE_NAME_LEN]={0};
	/*

	char *dest_path=(char*)malloc(MAX_FILE_PATH_LEN);
	memset(dest_path,0,MAX_FILE_PATH_LEN);
	p_debug("abs_file_path=%s",abs_file_path);

	get_filename_from_path(abs_file_path,name);

	if(generate_dest_file_path(name,dest_path)<0){
		return -2;
	}
	p_debug("dest_path=%s",dest_path);
*/	
	fp_read = fopen(abs_file_path, "rb+");
	if (!fp_read) {
		printf("fopen %s!\n",abs_file_path);
		return -1;
	}/*
	fp_write = fopen(dest_path, "wb+");
	if (!fp_write) {
		p_debug("fp_write fopen (%s)errno %d!\n",dest_path,errno);
		return -1;
	}*/
	if(encrypt_fd(fp_read)<0)
	{
		fclose(fp_read);
		//fclose(fp_write);
		printf("file_encrypt error!\n");
		//safe_free(dest_path);
		return -1;
	}else{//encrypt success
		fclose(fp_read);
		//fclose(fp_write);
		p_debug("file_encrypt success!\n");
		//add_to_record(&e_file,abs_file_path,dest_path);
		//remove the original file
		//TODO..
	}
	//safe_free(dest_path);
	return 0;

}

int main(int argc, char** argv) {
    AES_KEY aes;
    unsigned char key[AES_BLOCK_SIZE];        // AES_BLOCK_SIZE = 16
    unsigned char iv[AES_BLOCK_SIZE];        // init vector
    unsigned char* input_string;
    unsigned char* encrypt_string;
    unsigned char* decrypt_string;
    unsigned int len;        // encrypt length (in multiple of AES_BLOCK_SIZE)
    unsigned int i;
 
    // check usage
    if (argc != 2) {
        fprintf(stderr, "%s <plain text>\n", argv[0]);
        exit(-1);
    }

	encrypt_file(argv[1]);

	return 0;
 /*
    // set the encryption length
    len = 0;
    if ((strlen(argv[1]) + 1) % AES_BLOCK_SIZE == 0) {
        len = strlen(argv[1]) + 1;
    } else {
        len = ((strlen(argv[1]) + 1) / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
    }
	printf("len=%d/n",len);
 
    // set the input string
    input_string = (unsigned char*)calloc(len, sizeof(unsigned char));
    if (input_string == NULL) {
        fprintf(stderr, "Unable to allocate memory for input_string\n");
        exit(-1);
    }
    strncpy((char*)input_string, argv[1], strlen(argv[1]));
 
    // Generate AES 128-bit key
    for (i=0; i<16; ++i) {
        key[i] = 32 + i;
    }
 	char *k=(char*)malloc(AES_KEY_LEN);
	memset(k,0,AES_KEY_LEN);
	gen_k(k);
	
    // Set encryption key
    for (i=0; i<AES_BLOCK_SIZE; ++i) {
        iv[i] = 0;
    }
    if (AES_set_encrypt_key(k, 128, &aes) < 0) {
        fprintf(stderr, "Unable to set encryption key in AES\n");
        exit(-1);
    }
 
    // alloc encrypt_string
    encrypt_string = (unsigned char*)calloc(len, sizeof(unsigned char));    
    if (encrypt_string == NULL) {
        fprintf(stderr, "Unable to allocate memory for encrypt_string\n");
        exit(-1);
    }
 
    // encrypt (iv will change)
    //AES_cbc_encrypt(input_string, encrypt_string, len, &aes, iv, AES_ENCRYPT);

	// encrypt (iv will not change)
	AES_ecb_encrypt(input_string, encrypt_string, &aes, AES_ENCRYPT);
 
    // alloc decrypt_string
    decrypt_string = (unsigned char*)calloc(len, sizeof(unsigned char));
    if (decrypt_string == NULL) {
        fprintf(stderr, "Unable to allocate memory for decrypt_string\n");
        exit(-1);
    }
 
    // Set decryption key
    for (i=0; i<AES_BLOCK_SIZE; ++i) {
        iv[i] = 0;
    }
    if (AES_set_decrypt_key(k, 128, &aes) < 0) {
        fprintf(stderr, "Unable to set decryption key in AES\n");
        exit(-1);
    }
 
    // decrypt
    AES_ecb_encrypt(encrypt_string, decrypt_string, &aes, 
            AES_DECRYPT);
 
    // print
    printf("input_string = %s\n", input_string);
    printf("encrypted string = ");
    for (i=0; i<len; ++i) {
        printf("%x%x", (encrypt_string[i] >> 4) & 0xf, 
                encrypt_string[i] & 0xf);    
    }
    printf("\n");
    printf("decrypted string = %s\n", decrypt_string);
	safe_free(encrypt_string);
	safe_free(decrypt_string);
 	safe_free(k);
    return 0;
    */
}

