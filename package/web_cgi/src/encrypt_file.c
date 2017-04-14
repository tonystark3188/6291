#include "defs.h"
#include "base.h"
#include "encrypt_file.h"

#include <openssl/aes.h>
#include "my_debug.h"
#include "random-string.h"
//#include "jz_aes_v12.h"
#include "md5.h"

//struct	encrypted_file *e_file_list=NULL;

int store_key(char *key){
	return 1;
}

char * find_the_real_file(char *src){
	struct encrypted_file *p= e_file_list;
	int i=0;
	for(i=0;;i++){
		if(!p) break;
		if(!strcmp(p->src_path,src))
		{
			return p->dest_path;
		}
		p=p->next;
	}
}

int
get_enc_path_info(struct conn *c, char *path, struct stat *stp)
{
	if (my_stat(path, stp) == 0)
		return (0);
	return (-1);
}


static int
write_enc_file(struct stream *stream, const void *buf, size_t len)
{
	struct stat	st;
	struct stream	*rem = &stream->conn->rem;
	int		n, fd = stream->chan.fd;

	assert(fd != -1);
	n = write(fd, buf, len);
	if (n <= 0) {
		stop_stream(stream);
	}
	stream->conn->ctx->watch_dog_time = WATCH_DOG_SYNC_TIMEOUT;
	return (n);
}

static int
read_enc_file(struct stream *stream, void *buf, size_t len)
{
	int n = 0;
	int i = 0;
	int nwrites=0;
	AES_KEY aes;
	assert(stream->chan.fd != -1);

	char *input_buf=(char *)malloc(len+AES_BLOCK_SIZE);
	memset(input_buf,0,(len+AES_BLOCK_SIZE));
	n = fread(input_buf, 1, len, stream->chan.fd);
	if (!n) {
		DMCLOG_D("fread! error\n");
		return -1;
	}

	char aes_input_block[AES_BLOCK_SIZE]={0};
	char aes_output_block[AES_BLOCK_SIZE]={0};
    
	int start_byte=(stream->offset%AES_BLOCK_SIZE);
	for(i=0; i<n; i=i+AES_BLOCK_SIZE){
	
		memcpy(aes_input_block,input_buf+i,AES_BLOCK_SIZE);
		AES_ecb_encrypt(aes_input_block, aes_output_block, &aes, AES_DECRYPT);
		if(i == 0){
			if(len<(AES_BLOCK_SIZE-start_byte)) break;
			memcpy(buf,aes_output_block+start_byte,AES_BLOCK_SIZE-start_byte);
			nwrites += (AES_BLOCK_SIZE-start_byte);
			len -= (AES_BLOCK_SIZE-start_byte);
		}
		else
			if(len<(AES_BLOCK_SIZE)) break;
			memcpy(buf+i+start_byte,aes_output_block,AES_BLOCK_SIZE);
			nwrites += AES_BLOCK_SIZE;
			len -= AES_BLOCK_SIZE;
	}
	safe_free(input_buf);
	//n = read(stream->chan.fd, buf, len);
	return nwrites;
}

static void
close_enc_file(struct stream *stream)
{
	assert(stream->chan.fd != -1);
	(void) close(stream->chan.fd);
	struct conn	*c = stream->conn;
	if(c->ctx->nactive_fd_cnt  > 0)
		c->ctx->nactive_fd_cnt--;
}


void
get_enc_file(struct conn *c, struct stat *stp)
{
	big_int_t	cl; /* Content-Length */
	cl = (big_int_t) stp->st_size;
	DMCLOG_D("cl = %lld,stp->st_size = %lld,c->offset = %lld,c->length=%lld,stp->st_mtime = %lu",cl,stp->st_size,c->offset,c->length,stp->st_mtime);
	/* If Range: header specified, act accordingly */
	(void) lseek64(c->loc.chan.fd, c->offset/AES_BLOCK_SIZE, SEEK_SET);
	c->loc.offset=c->offset;
	if(c->length <= 1)
	{
		cl = cl - c->offset;
	}else{
		cl = c->length - c->offset;
	}
	c->ctx->nactive_fd_cnt++;
	c->modifyTime = stp->st_mtime;
	c->loc.content_len = cl;
	DMCLOG_D("sizeof(big_int_t) = %d, cl = %lld, c->loc.content_len = %lld", sizeof(big_int_t), cl, c->loc.content_len);
	c->loc.io_class = &io_enc_file;
	c->loc.flags |= FLAG_R | FLAG_ALWAYS_READY;
}


int read_from_record(struct	encrypted_file **dn)//读入数据
 {	 
 	ENTER_FUNC();

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
		DMCLOG_D("NAME=%s/%s",cur->src_path,cur->dest_path);
		if(!cur)
		{
			DMCLOG_D("malloc task_dn error\n");
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
	ENTER_FUNC();
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

static int read_enc_dir(struct stream *stream, void *buf, size_t len){
	ENTER_FUNC();
	char *path = stream->chan.dir.path;
	//read_from_record(&e_file_list);
	struct encrypted_file *p = e_file_list;
	int i = 0, nwritten = 0;
	struct conn	*c = stream->conn;
	char	file[FILENAME_MAX], line[FILENAME_MAX + 512];
	int		n = 0;
	for(i=0;;i++){

		if (len < sizeof(line))
		{
			DMCLOG_D("out of stream");
			break;
		}

		if(!p)//p=NULL last read
		{
			stream->flags |= FLAG_CLOSED;
			stream->flags &= ~(FLAG_R | FLAG_W | FLAG_ALWAYS_READY);
			n = my_snprintf(line, sizeof(line)," ], \"total\": %d },\"header\": { \"cmd\": %d, \"seq\": %d, \"error\": %d }}",
				c->nfiles,c->cmd,c->seq,c->error);
			(void) memcpy(buf, line, n);
			buf = (char *) buf + n;
			nwritten += n;
			len -= n;
			DMCLOG_D("read finish");
			break;
		}
		int plen = strlen(path);// /tmp/mnt/USB-disk-1/safe/aaa/
		int isFolder = 0;
		DMCLOG_D("p->src_path=%s",p->src_path);
		if(!strncmp(path,p->src_path,plen))//tmp/mnt/USB-disk-1/safe/aaa/bbb/ccc.mp4
		{
			char *tmp_dir_path = (char*)malloc(strlen(p->src_path)+1);
			memset(tmp_dir_path,0,strlen(p->src_path)+1);
			char *q = (char*)malloc(strlen(p->src_path)+1);
			memset(q,0,strlen(strlen(p->src_path)+1));

			if(path[plen-1] == '/'){
				char *src = p->src_path;
				strcpy(q,src+plen);// bbb/ccc.mp4
				char *m = NULL;
				if(m = strchr(q,'/'))// contain a dir
				{
					strncpy(tmp_dir_path,q,plen+m-q);
					DMCLOG_D("tmp_dir_path=%s",tmp_dir_path);
					isFolder = 1;
				}else {//is a file. add to reply json
					isFolder = 0;
				}

			}else {
				char *src = p->src_path;
				strcpy(q,src+plen+1);// bbb/ccc.mp4
				DMCLOG_D("q=%s",q);
				char *m=NULL;
				if(m = strchr(q,'/'))// contain a dir
				{				
					strncpy(tmp_dir_path,src,plen+(m-q)+1);
					DMCLOG_D("tmp_dir_path=%s",tmp_dir_path);
					isFolder = 1;
				}else {//is a file. add to reply json
					isFolder = 0;
				}				
			}
			if(isFolder == 1){
					if(c->nfiles == 0)
					{
						n = my_snprintf(line, sizeof(line),
					    "{ \"isFolder\": 1, \"size\": 4096,\"data\": %u, \"name\": \"%s\",\"path\": \"%s\"}",
					     p->mod_time,p->name,tmp_dir_path);
					}else{
						n = my_snprintf(line, sizeof(line),
					   ",{ \"isFolder\": 1 , \"size\": 4096,\"data\": %u, \"name\": \"%s\",\"path\": \"%s\"}",
					     p->mod_time,p->name,tmp_dir_path);
					}			
			}else {
					if(c->nfiles == 0)
					{
						n = my_snprintf(line, sizeof(line),
					    "{ \"isFolder\": 0, \"size\": %lld,\"data\": %u, \"name\": \"%s\",\"path\": \"%s\"}",
					     p->size,p->mod_time,p->name, p->src_path);
					}else{
						n = my_snprintf(line, sizeof(line),
					   ",{ \"isFolder\": 0 , \"size\": %lld,\"data\": %u, \"name\": \"%s\",\"path\": \"%s\"}",
					     p->size,p->mod_time,p->name, p->src_path);
					}

			}
			safe_free(tmp_dir_path);
			safe_free(q);

			(void) memcpy(buf, line, n);
			buf = (char *) buf + n;
			nwritten += n;
			len -= n;
			c->nfiles++;
		}

		p = p->next;
	}
	//destroy_record(e_file_list);
	return (nwritten);
}

static int is_enc_dir_exist(char *src){
	int ret=0;
	
	return ret;
}

void get_enc_dir(struct conn *c){

	c->loc.io.head = my_snprintf(c->loc.io.buf, c->loc.io.size,
	    "{ \"data\": { \"filelist\": [");
	io_clear(&c->rem.io);
	c->loc.io_class = &io_enc_dir;
	c->loc.flags |= FLAG_R | FLAG_ALWAYS_READY;
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


	if(strlen(s) != 8) return -1;

	unsigned int ui[8];
	int i=0;
	for(i=0;i<8;i++){
		if(s[i]>='a')
			ui[i]=s[i]-87;
		else
			ui[i]=s[i]-'0';

	}
	unsigned int uin=ui[0]<<28|ui[1]<<24|ui[2]<<20|ui[3]<<16|ui[4]<<12|ui[5]<<8|ui[6]<<4|ui[7];
	return uin;
}

int gen_k(char *k){
//	char *mac="84:5d:d7:00:11:22";
	char *mac="84:5D:D7:A1:12:27";
	char *sn="84:5D:D7:A1:12:27";
	int i=0;
//	char *k[16]={0};
	get_md5sum(mac,k,AES_KEY_LEN);

	//unsigned int key[4] = {0xd55fda32,0x14d99bd7,0x9f7a0ef8,0x972df216};
	char tmp[8]={0};
	unsigned int key[4];
	

	char *p=k;
	for(i=0;i<4;i++){
		strncpy(tmp,p,8);
		tmp[8]='\0';
		p=p+8;
		key[i]=str_to_uint(tmp);
		DMCLOG_D("k[%d]=%x",i,key[i]);
	}

}


int encrypt_fd(FILE *fp_read,FILE *fp_write)
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

	DMCLOG_D("k=%s",k);

	int ret = -1;
	long fsize = 0;
	int rd_byte = 0, wr_byte = 0;

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

	if (fseek(fp_write, 0, SEEK_SET) == -1) {
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


			for(i=0;i<MAX_DMA_LEN;i=i+AES_BLOCK_SIZE){
					memcpy(input,input_block+i,AES_BLOCK_SIZE);
					
					AES_ecb_encrypt(input, output, &aes, 
						AES_ENCRYPT);
					
					memcpy(output_block+i,output,AES_BLOCK_SIZE);
			}

		//	ret = ioctl(aes_fd, AES_DO_CRYPT, &aes_data);
		/*	if (fseek(fp_read, fsize-len, SEEK_SET) == -1) {
				printf("fseek SEEK_SET error!\n");
				return -1;
			}
		*/
			wr_byte = fwrite(output_block, 1, rd_byte + pad, fp_write);
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
		/*	if (fseek(fp_read, fsize-len, SEEK_SET) == -1) {
				printf("fseek SEEK_SET error!\n");
				return -1;
			}
		*/
			wr_byte = fwrite(output_block, 1, rd_byte, fp_write);
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
char * generate_encrypted_file_name(char *src){

	
	char *random=NULL;
	random=base64_encode(src,strlen(src));	
	if(random !=NULL)	
		DMCLOG_D("random=(%s)",random);	
	DMCLOG_D("src=%s",src);	

	char *dest_name=(char*)malloc(ENCRYPTED_FILE_NAME_LENGTH+1);
	memset(dest_name,0,ENCRYPTED_FILE_NAME_LENGTH+1);
	DMCLOG_D("src=%s",src);	
	get_md5sum(random,dest_name,ENCRYPTED_FILE_NAME_LENGTH);

	//sprintf(dest,"%s/.%s",SAFE_BOX_PATH,dest_name);

//	DMCLOG_D("dest=%s",dest);
//	free(random);
	return dest_name;
}
int get_file_info(char *path, struct encrypted_file *dn){

	char *p=strrchr(path,'\/');
	strncpy(dn->name,p+1,MAX_FILE_NAME_LEN);
	struct stat statbuff;  
    if(stat(path, &statbuff) < 0){  
		DMCLOG_E("stat file error");
        return -1;  
    }else{  
    	DMCLOG_D("stat file ok");
		DMCLOG_D("statbuff.st_size=%d",statbuff.st_size);
        dn->size = statbuff.st_size;  
		DMCLOG_D("dn-size=%d\n",dn->size);
		DMCLOG_D("size=%ld",dn->size);
		dn->mod_time=statbuff.st_mtime;
		DMCLOG_D("mtime=%d",dn->mod_time);
    } 	
}

int write_list_to_file()//写出数据
{
	//p_debug("access write_list_to_file");	

	struct encrypted_file *dn=e_file_list;

	unsigned i = 0;
	int enRet = 0;
	FILE *record_fd=NULL;
	
	//display_task_dnode(dn);
	if((record_fd = fopen(ENC_DATA_PATH,"w+"))== NULL)//改动：路径
	{
		DMCLOG_E("open file error");
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
	
		dn = dn->next;


	}
	fclose(record_fd);

	return 0;
}

int add_to_record(struct encrypted_file **pdn,char *src, char *dest)
{
	DMCLOG_D("access add_to_record");

	int i;
    char tmpbuf[128];
	
	struct encrypted_file *dn=e_file_list;
	struct encrypted_file *cur=(struct encrypted_file *)malloc(sizeof(struct encrypted_file));
	memset(cur,0,sizeof(struct encrypted_file));

	if(get_file_info(src,cur)<0){
		DMCLOG_D("get_file_info error.");
	}
	if(strlen(src)<MAX_FILE_PATH_LEN) 
	{
		strcpy(cur->src_path,src);
		//sprintf(cur->virtual_path,"%s/%s",SAFE_BOX_PATH,src);
	}
	else 
		return -1;

	if(strlen(dest)<MAX_FILE_PATH_LEN) 
		strcpy(cur->dest_path,dest);	
	else 
		return -1;
	
	for(i=0;;i++){
		if(!dn)break;
		if((!strcmp(dn->src_path,cur->src_path))){
	
				DMCLOG_D("already encrypted?");
				DMCLOG_D("save orig=%s,encry=%s",dn->src_path,dn->dest_path);
				safe_free(cur);
				return -2;

			}
			
			dn=(dn)->next;
	}
	cur->next = *pdn;
	*pdn = cur;

	int ret = write_list_to_file();
	DMCLOG_D("leave add_to_record");
	return ret;
}
int do_encrypt_file(const char *src){
		FILE *fp_read, *fp_write;
		char *dest_name=generate_encrypted_file_name(src);
		char * dest=(char*)malloc(strlen(dest_name)+strlen(SAFE_BOX_PATH)+1);
		memset(dest,0,strlen(dest_name)+strlen(SAFE_BOX_PATH)+1);
		
		sprintf(dest,"%s/.%s",SAFE_BOX_PATH,dest_name);
		safe_free(dest_name);
		
		DMCLOG_D("generate_encrypted_file_path = %s",dest);

		fp_read = fopen(src, "rb+");
		if (!fp_read) {
			DMCLOG_D("fopen %s!\n",src);
			return -1;
		}
		fp_write = fopen(dest, "wb+");
		if (!fp_write) {
			DMCLOG_D("fp_write fopen (%s)errno %d!\n",dest,errno);
			return -1;
		}
		int ret = encrypt_fd(fp_read,fp_write);
		if(ret <0){
			DMCLOG_D("encrypt failed.");
			return ret;
		}

		ret=add_to_record(&e_file_list,src,dest);

		safe_free(dest);
		
		return ret;
}

int encrypt_file_init(){
	read_from_record(&e_file_list);
}
int encrypt_file_exit(){
	destroy_record(e_file_list);
}
const struct io_class	io_enc_dir =  {
	"dir",
	read_enc_dir,
	NULL,
	NULL
};


const struct io_class	io_enc_file =  {
	"enc_file",
	read_enc_file,
	write_enc_file,
	close_enc_file
};

