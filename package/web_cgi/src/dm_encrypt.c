#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include <string.h>

#include "list.h"
#include "jz_aes_v12.h"
#include "my_debug.h"
#include "random-string.h"

#include "md5.h"

#define AES_BLOCK_SIZE	16
#define DMA_MAX_LEN	4096
#define MAX_FILE_PATH_LEN 512
#define MAX_FILE_NAME_LEN 256

int aes_fd;
typedef struct encrypted_file{
	struct encrypted_file *dn_next;
	char name[MAX_FILE_NAME_LEN];
	char orig[MAX_FILE_PATH_LEN];
	char encry[MAX_FILE_PATH_LEN];
	unsigned long size;
	time_t mod_time;
}EncrypFile;

struct	encrypted_file *e_file;

typedef struct VfileInfoTable
{
	char name[MAX_FILE_NAME_LEN];
 	struct dl_list next;
}v_file_info_t;

#define SAFE_BOX_PATH  "/tmp/mnt/USB-disk-1/safe"
#define e_file_path  "/tmp/mnt/USB-disk-1/.e"

#define ENCRYPTED_FILE_NAME_LENGTH 20

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

#define AES_KEY_LEN 32
int save_k(char *k){

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
int decrypt_file_from_src(char *path){
	
}
#if 0
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
	aes_key.aesmode = AES_MODE_CBC;
	aes_key.iv = (char *)iv;
	aes_key.ivlen = 16;
	aes_key.encrypt = 1;
	
	fd = open("/dev/jz-aes", O_RDWR);
	if(fd < 0) {
		printf("open error!");
		return -1;
	}

	aes_fd = fd;


	ret = ioctl(aes_fd, AES_LOAD_KEY, &aes_key);
	if(ret < 0) {
		printf("ioctl! AES_LOAD_KEY");
	}

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

	unsigned char input[AES_BLOCK_SIZE];
	unsigned char output[AES_BLOCK_SIZE];
	unsigned int align_len = 0;
	unsigned int pad, len, i;

	len = fsize;
	p_debug("access decr 	ypt_fd");
	//fseek(fp_read,2,SEEK_SET);
	
	while(len) {
		rd_byte = fread(input, 1, AES_BLOCK_SIZE, fp_read);
		if (!rd_byte) {
			printf("fread! error\n");
			return -1;
		}

		aes_data.input = input;
		aes_data.input_len = rd_byte;
		aes_data.output = output;

		ret = ioctl(aes_fd, AES_DO_CRYPT, &aes_data);

		len -= rd_byte;
		p_debug("1 output=%s",output);

		if (len == 0) {
			pad = output[rd_byte - 1];
			if (pad > AES_BLOCK_SIZE) {
				printf("pad error\n");
				return -1;
			}
			if (!(rd_byte - pad))
				continue;
			p_debug("last output=%s",output);
			wr_byte = fwrite(output, 1, rd_byte - pad, fp_write);
			if (!wr_byte) {
				printf("last fwrite! error\n");
				return -1;
			}
		}
		else {
			p_debug("output=%s",output);
			wr_byte = fwrite(output, 1, rd_byte, fp_write);
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
#endif 

#define MAX_DMA_LEN AES_BLOCK_SIZE*256
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
	
	fd = open("/dev/jz-aes", O_RDWR);
	if(fd < 0) {
		printf("open error!");
		return -1;
	}

	aes_fd = fd;


	ret = ioctl(aes_fd, AES_LOAD_KEY, &aes_key);
	if(ret < 0) {
		printf("ioctl! AES_LOAD_KEY");
	}

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
	p_debug("len=%d",len);
	while(len) {
		rd_byte = fread(input, 1, AES_BLOCK_SIZE, fp_read);
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
			pad = output[rd_byte - 1];
			p_debug("pad=%d",pad);			
			if (pad > AES_BLOCK_SIZE) {
				printf("pad error\n");
				return -1;
			}
			if (!(rd_byte - pad))
				continue;
		//	p_debug("last output=%s",output);

			
			wr_byte = fwrite(output, 1, rd_byte - pad, fp_write);
			if (!wr_byte) {
				printf("last fwrite! error\n");
				return -1;
			}
		}
		else {
		//	p_debug("output=%s",output);
			wr_byte = fwrite(output, 1, rd_byte, fp_write);
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

int encrypt_fd(FILE *fp_read, FILE *fp_write)
{
	int fd;

	unsigned char input[AES_BLOCK_SIZE];
	unsigned char output[AES_BLOCK_SIZE];
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
	
	fd = open("/dev/jz-aes", O_RDWR);
	if(fd < 0) {
		printf("open error!");
		return -1;
	}

	aes_fd = fd;

	ret = ioctl(aes_fd, AES_LOAD_KEY, &aes_key);
	if(ret < 0) {
		printf("ioctl! AES_LOAD_KEY");
	}


	while(len) {
		rd_byte = fread(input, 1, AES_BLOCK_SIZE, fp_read);
		if (!rd_byte) {
			printf("fread! error\n");
			close(fd);
			return -1;
		}

		if (!(len - rd_byte)) {		//last read
			align_len = ((rd_byte / AES_BLOCK_SIZE) + 1) * AES_BLOCK_SIZE;
			/* PKCS5Padding */
			pad = AES_BLOCK_SIZE - rd_byte % AES_BLOCK_SIZE;
			for (i = rd_byte; i < align_len; i++) {
				input[i] = pad;
//				printf("PKCS5:input[%d]=%02x ",i,input[i]);
			}
//			printf("\n");

			aes_data.input = input;
			aes_data.input_len = rd_byte + pad;
			aes_data.output = output;

			ret = ioctl(aes_fd, AES_DO_CRYPT, &aes_data);

			wr_byte = fwrite(output, 1, rd_byte + pad, fp_write);
			if (!wr_byte) {
				printf("last fwrite! error\n");
				close(fd);				
				return -1;
			}
		}
		else {
			aes_data.input = (char *)input;
			aes_data.input_len = rd_byte;
			aes_data.output = (char *)output;

			ret = ioctl(aes_fd, AES_DO_CRYPT, &aes_data);

			wr_byte = fwrite(output, 1, rd_byte, fp_write);
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

#define SCAN_DIR_DEPTH 2
int filter_dot(const struct dirent *ent)
{
	//if(ent->d_type != DT_REG)
		//return 0;
	return (ent->d_name[0] != '.');
}
#if 0
int scan_dir(char *abs_dir_path, int depth, struct dl_list *head){
	struct dirent **namelist;
	int i=0;
	int k=0;
	int j=0;
	int dir_num=0;
	int file_num=0;
	p_debug("abs_dir_path = %s\n", abs_dir_path);

	int total = scandir(abs_dir_path, &namelist, filter_dot, alphasort);
	if(total < 0){
        perror("scandir");
		return -1;
	}
    else
    {
     //   for(i = 0; i < total; i++)
        printf("dir------%s\n", namelist[i]->d_name);
        p_debug("total = %d\n", total);
    }	
	
	//char **file_array;
	//char **tmpfilelist;
	//*file_array =(char**) malloc(sizeof(char *)*total);
	//tmpfilelist =(char**) malloc(sizeof(char *)*total);
	for(i = 0; i < total; i++){
			//sprintf(real_file_path,"%s%s",abspath,namelist[i]->d_name);
			//stat(real_file_path,&statbuf);
			//printf("i=%d,real=%s\n",i,real_file_path);
			if(namelist[i]->d_type == 4)//is a dir
			{
				//strcpy(file_array[j],namelist[i]->d_name);
				dir_num++;
			}else{//is a file
				v_file_info_t *pfi = (v_file_info_t )calloc(1,sizeof(v_file_info_t));
				assert(pfi !=  NULL);
				strncpy(pfi->name,namelist[i]->d_name,512);
				
				dl_list_add_tail(head, &pfi->next);
				//strcpy(file_array[file_num],namelist[i]->d_name);
				//p_debug(file_array[file_num]);
				file_num++;
			}
	}
	//file_array_len=file_num;
	//p_debug("file_array_len=%d",*file_array_len);
	p_debug("dir_num=%d",dir_num);
	p_debug("file_nu=%d",file_num);
	//p_debug("i=%d",i);
	for(i = 0; i < total; i++)
	{
		//p_debug("ii=%d",i);

		free(namelist[i]);
		
		//free(filelist[i]);
		//free(tmpfilelist[i]);
	}
	free(namelist);
	//free(filelist);
	//free(tmpfilelist);
	//p_debug("return");
	return file_num;
}
#endif
int scan_dir(char *abs_dir_path, int depth, struct dl_list *head){
	struct dirent **namelist;
	int i=0;
	int k=0;
	int j=0;
	int dir_num=0;
	int file_num=0;
	p_debug("abs_dir_path = %s\n", abs_dir_path);

	int total = scandir(abs_dir_path, &namelist, filter_dot, alphasort);
	if(total < 0){
        perror("scandir");
		return -1;
	}
    else
    {
     //   for(i = 0; i < total; i++)
        printf("dir------%s\n", namelist[i]->d_name);
        p_debug("total = %d\n", total);
    }	
	
	//char **file_array;
	//char **tmpfilelist;
	//*file_array =(char**) malloc(sizeof(char *)*total);
	//tmpfilelist =(char**) malloc(sizeof(char *)*total);
	for(i = 0; i < total; i++){
			//sprintf(real_file_path,"%s%s",abspath,namelist[i]->d_name);
			//stat(real_file_path,&statbuf);
			//printf("i=%d,real=%s\n",i,real_file_path);
			if(namelist[i]->d_type == 4)//is a dir
			{
				//strcpy(file_array[j],namelist[i]->d_name);
				dir_num++;
			}else{//is a file
				v_file_info_t *pfi = (v_file_info_t *)calloc(1,sizeof(v_file_info_t));
			//	assert(pfi !=  NULL);
				strncpy(pfi->name,namelist[i]->d_name,MAX_FILE_NAME_LEN);
				
				dl_list_add_tail(head, &pfi->next);
				//strcpy(file_array[file_num],namelist[i]->d_name);
				//p_debug(file_array[file_num]);
				file_num++;
			}
	}
	//file_array_len=file_num;
	//p_debug("file_array_len=%d",*file_array_len);
	p_debug("dir_num=%d",dir_num);
	p_debug("file_nu=%d",file_num);
	//p_debug("i=%d",i);
	for(i = 0; i < total; i++)
	{
		//p_debug("ii=%d",i);

		free(namelist[i]);
		
	}
	free(namelist);
	return file_num;
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
int safe_free(char *p)
{
	if(p!=NULL)
		{free(p);p=NULL;}
}

int get_file_list(){

}
int remove_encrypted_file(){
	char cmd[64]={0};
	sprintf(cmd,"rm -rf %s;rm -rf %s",SAFE_BOX_PATH,e_file_path);
}

int updateSysVal(const char *para,const char *val){
	char set_str[128]={0};
	char tmp[128]={0};
	sprintf(set_str,"sed -e \'/%s/d\' -i /tmp/state/status",para);
	system(set_str);
	memset(set_str,0,sizeof(set_str));
	
	sprintf(set_str,"echo \'%s=%s\' >> /tmp/state/status",para, val);
	system(set_str);
}

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
int add_to_record(struct encrypted_file **pdn,char *src, char *dest)
{
	p_debug("access add_to_record");

	int i;
    char tmpbuf[128];
	
	struct encrypted_file *dn=e_file;
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

int destory_record(struct encrypted_file *dn)
{
	unsigned i = 0;
	struct encrypted_file *head;
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
	//printf("free succ\n");
	return 0;
}

int write_list_to_file()//写出数据
{
	char path[128]="\0";
	//p_debug("access write_list_to_file");	

	struct encrypted_file *dn=e_file;

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


}

int del_from_record(struct encrypted_file **head,char *orig)  
{  
	p_debug("access del_from_record");
    struct encrypted_file *dn = *head; 
	int i;
	
    struct encrypted_file *node1 = *head; 
    struct encrypted_file *node2 = NULL;  
    if (*head==NULL)  
    {  
        return -1;  
    }   
    else  
    {  
        if (!strcmp(node1->orig,orig))
        {  
            *head=(*head)->dn_next;  
			//task_dn=task_dn->dn_next;
			//p_debug("del PID/VID=%s/%s",node1->pid,node1->vid);
            safe_free(node1); 
			p_debug("del succ");
			write_list_to_file();				
            return 0;  
        }   
        else  
        {  
            while (node1!=NULL)  
            {  
                node2=node1;  
                node2=node2->dn_next; 

                if (!strcmp(node2->orig,orig))  
                {  
                    node1->dn_next=node2->dn_next;  
					p_debug("del orig/%s/%s",node2->orig,node2->orig);
                    safe_free(node2); 
                    break;  
                }			
                node1=node1->dn_next;  
            }  
			p_debug("del succ2");
			write_list_to_file();
            return 0;  
        }  
    }  
}

int read_record(struct	encrypted_file **dn)//读入数据
 {	 
 //printf("access read_list_from_file\n");
	  FILE *record_fd;
	  int enRet;
	  struct encrypted_file *cur=NULL;
	  int ret = 0;
	  int i = 0;
	  if(access(e_file_path,R_OK)!=0){// 不存在
		  if((record_fd = fopen(e_file_path,"wb+"))==NULL)//改动：路径
		  {
			// p_debug("task list file does not exist error1[errno:%d]\n",errno);
			 return -1;
		  }
	  }else  if((record_fd = fopen(e_file_path,"r"))==NULL)//改动：路径
	  {
		 //p_debug("task list file does not exist error2[errno:%d]\n",errno);
		 return -1;
	  }
	  
	  while(!feof(record_fd))
	  {
		 cur = (struct encrypted_file *)malloc(sizeof(*cur));
		 memset(cur,0,sizeof(*cur));
		 cur->dn_next=NULL;
		 enRet = fread(cur,sizeof(struct encrypted_file),1,record_fd);
		 if(enRet <= 0)
	     {	
	     	if(cur!=NULL)free(cur);
			//p_debug("fread error,enRet = %d,errno = %d\n",enRet,errno);
			break;
		 }

		if(!cur)
		{
			printf("malloc task_dn error\n");
			return -2;
		}
		cur->dn_next = *dn;
		*dn = cur;
		 i++;
	  }
	  fclose(record_fd);
	  //printf("leave read_list_from_file\n");

	  return ret;
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
int check_if_encrypted(char *abs_file_path){
	int i=0;
	struct encrypted_file *dn=e_file;

	for(i=0;;i++){
		if(!dn)break;
		if(!strcmp((dn)->orig,abs_file_path)){
			p_debug("already existed.");
			return 0;
		}
		dn=dn->dn_next;
	}
	return 1;

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

	char *dest_path=(char*)malloc(MAX_FILE_PATH_LEN);
	memset(dest_path,0,MAX_FILE_PATH_LEN);
	p_debug("abs_file_path=%s",abs_file_path);
	get_filename_from_path(abs_file_path,name);
	if(generate_dest_file_path(name,dest_path)<0){
		return -2;
	}
	p_debug("dest_path=%s",dest_path);
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
	if(encrypt_fd(fp_read,fp_write)<0)
	{
		fclose(fp_read);
		fclose(fp_write);
		printf("file_encrypt error!\n");
		safe_free(dest_path);
		return -1;
	}else{//encrypt success
		fclose(fp_read);
		fclose(fp_write);
		p_debug("file_encrypt success!\n");
		add_to_record(&e_file,abs_file_path,dest_path);
		//remove the original file
		//TODO..
	}
	safe_free(dest_path);
	return 0;

}
int encrypt_directory(char *abs_dir_path){

	char *file_array;
	int i=0;
	int file_array_len;
	char src_path[512]={0};
	char *dest_path;

	FILE *fp_read, *fp_write;
	
	struct dl_list head;
	dl_list_init(&head);
	file_array_len=scan_dir(abs_dir_path,SCAN_DIR_DEPTH,&head);
	if(file_array_len<0){
		p_debug("scan error");
		return -1;
	}
	char en_total[8]={0};
	sprintf(en_total,"%d",file_array_len);
	updateSysVal("en_total",en_total);

	v_file_info_t *p_file_info;
	v_file_info_t *n;
	dl_list_for_each(p_file_info , &head, v_file_info_t, next)
	 {
		p_debug("file_array=%s",p_file_info->name);
		sprintf(src_path,"%s%s",abs_dir_path,p_file_info->name);
		dest_path=(char*)malloc(MAX_FILE_PATH_LEN);
		memset(dest_path,0,MAX_FILE_PATH_LEN);
		if(generate_dest_file_path(p_file_info->name,dest_path)<0){
			return -2;
		}
		p_debug("dest_path=%s",dest_path);
		fp_read = fopen(src_path, "r");
		if (!fp_read) {
			printf("fopen %s!\n",src_path);
			return -1;
		}
		fp_write = fopen(dest_path, "wb+");
		if (!fp_write) {
			p_debug("fp_write fopen (%s)errno %d!\n",dest_path,errno);
			return -1;
		}
		if(encrypt_fd(fp_read,fp_write)<0)
		{
			fclose(fp_read);
			fclose(fp_write);
			printf("file_encrypt error!\n");
		}else{//encrypt success
			fclose(fp_read);
			fclose(fp_write);
			p_debug("file_encrypt success!\n");
			add_to_record(&e_file,src_path,dest_path);
		}
		i++;
		char en_already[8]={0};
		sprintf(en_already,"%d",i);
		updateSysVal("en_already",en_already);		
		
	 }
	safe_free(dest_path);
	dl_list_for_each_safe(p_file_info,n,&head,v_file_info_t,next)
	 {
		safe_free(p_file_info);
	 }

}


int main(int argc, char *argv[])
{
	int fd;
	FILE *fp_read, *fp_write;
	char *readpath = NULL;
	char *writepath = NULL;
	clock_t start,end;
	double TheTimes;
	long fsize = 0;
	double temp = 0;


	if(argc != 2) {
		printf("please intput correctly arguments!\n");
		return -1;
	}
	
	if(read_record(&e_file)<0){
		p_debug("read_record error");
		return -1;
	};

	start = clock();

	printf("#########CBC ENCRYPT########## \n");
	
	struct stat buf; 
	int result; 
	result = stat( argv[1], &buf ); 
	if (result == 0){
		
		if(S_IFDIR & buf.st_mode){ 
			printf("folder\n"); 
			encrypt_directory(argv[1]);
				
		}else 
		if(S_IFREG & buf.st_mode)
		{ 
			if(!check_if_encrypted(argv[1]))
			{
				p_debug("please input new file path.");
				return;
			}
			else
			{
				//encrypt_file(argv[1]);

				decrypt_file(argv[1]);
			}
		} 

	}
	else{
		printf("r=%d",result);
		p_debug("please input new file path.");
		return;
	}



	destory_record(&e_file);

	//fsize = file_encrypt(fp_read, fp_write);
	end = clock();
	/*
	TheTimes = (double)(end - start) / CLOCKS_PER_SEC;
	printf("encrypt %f s.\n",TheTimes);
	if(fsize>0)
	{
	     TheTimes = (double)fsize/TheTimes;
         temp = TheTimes/1024;
		 if(0 == temp)
		 	printf("encrypt speed:%f byte/S.\n",TheTimes);
		 else
		 {
			 temp = TheTimes/1024/1024;
			 if(0 == temp)
			 {
			     TheTimes = TheTimes/1024;
			     printf("encrypt speed:%f KB/S.\n",TheTimes);
			 }
			 else
			{	 
			     TheTimes = TheTimes/1024/1024;
				 printf("encrypt speed:%f MB/S.\n",TheTimes);
		    }


		 }
	}
	else
		printf("cbc_encrypt failed:ret=%d\n",fsize);

	close(fd);

	fclose(fp_read);
	fclose(fp_write);
	*/
	return 0;
}


