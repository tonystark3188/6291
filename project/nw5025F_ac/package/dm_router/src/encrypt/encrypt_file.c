#include "defs.h"
#include "base.h"
#include "encrypt_file.h"
#include <openssl/aes.h>
#include "my_debug.h"
#include "random-string.h"
//#include "jz_aes_v12.h"
#include "md5.h"
//#include "nor_control.h"
//struct	encrypted_file *e_file_list=NULL;
int get_list_len(struct encrypted_file *q){
	struct encrypted_file *p=q;
	int i;
	for(i=0;;i++){
		if(!p)
			break;
			p=p->next;
	}
	return i;
}
int check_vault_is_empty(struct conn *c){

	ENTER_FUNC();
	int res = 0;
	int i = 0;
	struct file_list file_list_t;
	memset(&file_list_t,0,sizeof(struct file_list));

	io_clear(&c->rem.io);
	c->loc.io_class = &io_all_dir;
	c->loc.flags |= FLAG_R | FLAG_ALWAYS_READY;
	if((c->msg = new_message()) == NULL)
    {
        log_error("create_sync_message failed");
        return EMESSAGE_NEW;
    }
	

	c->disk_info = get_disk_node(c->disk_uuid);
	if(c->disk_info == NULL)
	{
		DMCLOG_E("get the disk info failed");
		c->error = ERROR_GET_FILE_LIST;
        return EMESSAGE_NEW;
	}
	
	dl_list_init(&c->msg->msg_data.db_obj.data.file_list.head);
	c->status = 0;//have token,do not filter encrypt file
	res = _handle_get_all_type_file_list_cmd(c);
	if(res != RET_SUCCESS)
	{
		DMCLOG_D("_handle_get_all_type_file_list_cmd error");
		c->error = ERROR_GET_FILE_LIST;
		c->loc.flags |= FLAG_CLOSED;
		return EMESSAGE_NEW;
	}
	return 0;
}
int set_token(char *key){
	FILE *read_fp = NULL;	
	int res = 0;
	if( (read_fp = fopen(VAULT_TOKEN_PATH, "wb+")) != NULL)
	{
		res = fwrite(key,1,VAULT_TOKEN_LEN,read_fp);
		if(!res){
			fclose(read_fp);
			return -1;
		}
		fclose(read_fp);
		return 0;
	}
}
int get_token(char *key){
	FILE *read_fp = NULL;
	int res = 0;
	char tmpkey[VAULT_TOKEN_LEN+1] = {0};
	if( (read_fp = fopen(VAULT_TOKEN_PATH, "rb")) != NULL)
	{
		res = fread(tmpkey,1,VAULT_TOKEN_LEN,read_fp);
		if(!res){
			fclose(read_fp);
			return -1;
		}
		strncpy(key,tmpkey,VAULT_TOKEN_LEN);
		fclose(read_fp);
		return 0;
	}
	return -1;
}
int save_tips_to_file(char *key){
	FILE *read_fp = NULL;
	
	int res = 0;
	if( (read_fp = fopen(VAULT_TIPS_PATH, "wb+")) != NULL)
	{
		res = fwrite(key,1,VAULT_TIPS_LEN,read_fp);
		if(!res){
			fclose(read_fp);
			return -1;
		}
		fclose(read_fp);
		return 0;
	}
}
int save_key_to_file(char *key){
	FILE *read_fp = NULL;
	
	int res = 0;
	if( (read_fp = fopen(ENC_KEY_PATH, "wb+")) != NULL)
	{
		res = fwrite(key,1,AES_KEY_LEN,read_fp);
		if(!res){
			fclose(read_fp);
			return -1;
		}
		fclose(read_fp);
		return 0;
	}
}
int nor_get_key(char *key){
	FILE *read_fp = NULL;
	int res = 0;
	char tmpkey[AES_KEY_LEN+1] = {0};
	if( (read_fp = fopen(ENC_KEY_PATH, "rb")) != NULL)
	{
		res = fread(tmpkey,1,AES_KEY_LEN,read_fp);
		if(!res){
			fclose(read_fp);
			return 0;
		}
		strncpy(key,tmpkey,AES_KEY_LEN);
		fclose(read_fp);
		return 0;
	}
	return 0;
}

int set_nor_str2(char *param,char *ret_str)
{
	char set_str[128]={0};
	sprintf(set_str,"nor set %s=%s",param,ret_str);
	system(set_str);
	return 0;
}

static int set_nor_key(char *pwd)
{
	if(pwd == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	int ret = set_nor_str2("enc_key",pwd);
	if(ret != 0)
	{
		DMCLOG_E("set root password error");
		return -1;
	}
	return 0;
}

static int set_nor_tips(char *pwd)
{
	if(pwd == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	int ret = set_nor_str2("enc_tips",pwd);
	if(ret != 0)
	{
		DMCLOG_E("set root password error");
		return -1;
	}
	return 0;
}


int store_key(char *key){
	int ret;
	char *tmpkey=(char*)malloc(AES_KEY_LEN+1);
	if(tmpkey == NULL)
	{
		return -1;
	}
	memset(tmpkey,0,AES_KEY_LEN+1);

	get_md5sum(key, tmpkey, AES_KEY_LEN);
	DMCLOG_D("key=%s,tmpkey=%s",key,tmpkey);
	ret = save_key_to_file(tmpkey);
	ret = set_nor_key(tmpkey);
	safe_free(tmpkey);
	
	return ret;
}
int store_tips(char *tips){
	int ret;

	ret = save_tips_to_file(tips);
	ret = set_nor_tips(tips);
	return ret;
}
int get_tips(char *tips){
	FILE *read_fp = NULL;
	int res = 0;
	char tmpkey[VAULT_TIPS_LEN+1] = {0};
	if( (read_fp = fopen(VAULT_TIPS_PATH, "rb")) != NULL)
	{
		res = fread(tmpkey,1,VAULT_TIPS_LEN,read_fp);
		if(!res){
			fclose(read_fp);
			return -1;
		}
		strncpy(tips,tmpkey,VAULT_TIPS_LEN);
		fclose(read_fp);
		return 0;
	}
	return -1;
}
int check_token(char *key){
	int ret;
	char *skey = (char *)malloc(VAULT_TOKEN_LEN+1);
	if(skey == NULL)
	{
		return -1;
	}
	memset(skey,0,VAULT_TOKEN_LEN+1);
	
	ret = get_token(skey);
	if(ret != 0)
	{//token失效
		safe_free(skey);
		return 0;
	}	
	if(!strcmp(key,skey)){
		ret = 0;
		goto EXIT;
	}
	else{
		ret = -1;
		goto EXIT;		
	} 
EXIT:
	safe_free(skey);
	return ret;
}
int check_key(char *key){
	int ret;
	char *skey = (char *)malloc(MAX_KEY_LEN+1);
	if(skey == NULL)
	{
		return -1;
	}
	memset(skey,0,MAX_KEY_LEN+1);
	
	ret = nor_get_key(skey);
	if(ret != 0)
	{
		safe_free(skey);
		return -1;
	}
	DMCLOG_D("key=%s,skey=%s,strlen(%d)",key,skey,strlen(skey));
	char *tmpkey=(char*)malloc(AES_KEY_LEN+1);
	if(tmpkey == NULL)
	{
		ret = -1;
		goto EXIT;		
	}
	memset(tmpkey,0,AES_KEY_LEN+1);
	get_md5sum(key, tmpkey, AES_KEY_LEN);
	DMCLOG_D("tmpkey=%s,strlen(%d)",tmpkey,strlen(tmpkey));
	if(!strcmp(tmpkey,skey)){
		ret = 0;
		goto EXIT;
	}
	else{
		ret = -1;
		goto EXIT;		
	} 
EXIT:
	safe_free(skey);
	safe_free(tmpkey);
	return ret;
}
char * find_the_real_file(const char *src,struct encrypted_file *q){
	struct encrypted_file *p = q;
	int i=0;
	char tmpstr[MAX_FILE_NAME_LEN]={0};
	for(i=0;;i++){
		if(!p) break;
		sprintf(tmpstr,"%s/%s",SAFE_BOX_PATH,p->name);
		DMCLOG_D("tmpstr=%s,src=%s",tmpstr,src);
		if(!strcmp(src,tmpstr))
		{
			DMCLOG_D("dest_path=%s",p->dest_path);			
			return p->dest_path;
		}
		p=p->next;
	}
	return NULL;
}
struct encrypted_file * find_the_real_file_node(const char *src,struct encrypted_file *p){
	int i=0;
	char tmpname[MAX_FILE_NAME_LEN];
	
	for(i=0;;i++){
		if(!p) break;
		get_filename_from_path(p->dest_path,tmpname);
		if(!strcmp(tmpname,src))
		{
			return p;
		}
		p = p->next;
	}
	return NULL;
}

int get_parent_dir_from_path(char *abs_file_path,char *parent_dir_path){

    char *p = strrchr(abs_file_path,'/');
    DMCLOG_D("p=%s",p);
    int len = strlen(abs_file_path);
    if(p)
    {
    	if(abs_file_path[len-1] == '/')
    	{
			char q[MAX_FILE_PATH_LEN] = {0};
			//DMCLOG_D("abs_file_path=%s",abs_file_path);
			strncpy(q,abs_file_path,len-1);
			//DMCLOG_D("q=%s",q);
			char *r = strrchr(q,'/');
			//DMCLOG_D("r=%s",r);
			int tmplen=strlen(r);
			strncpy(parent_dir_path,abs_file_path,len-tmplen);
			//DMCLOG_D("parent_dir_path=%s",parent_dir_path);
		}else 
		{
			strncpy(parent_dir_path,abs_file_path,p-abs_file_path+1); 	
		}
        //strncpy(parent_dir_path,abs_file_path,(p-abs_file_path)+1);
        DMCLOG_D("parent_dir_path=%s",parent_dir_path);
		return 0;
    }
    else return -1;

}
int
get_enc_path_info(struct conn *c)
{
	off_t file_size;
	time_t file_time;
	int ret;
	ret = handle_query_file_info_by_path(c->file_uuid,c->disk_uuid,c->src_path,&file_size,&file_time);
	if(ret != RET_SUCCESS){
		return ret;
	}
	c->loc.content_len = file_size;
	c->modifyTime = file_time;
	return 0;
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

int
my_enc_set_key(struct stream *stream)
{
	char k[AES_KEY_LEN]={0};
//	char *k=(char*)malloc(AES_KEY_LEN);
//	memset(k,0,AES_KEY_LEN);
	if(gen_k(k)<0){
		//error = DM_ERROR_DECRYPT_KEY_FAIL;
		//pInfo->status = -1;
		return -1;
	}
	//DMCLOG_D("k=%s",k);
    if (AES_set_decrypt_key(k, 128, &(stream->aes)) < 0) {
   			
		return -1;
    }
}

	
int
my_enc_fopen(const char *path, char * mode)
{
	return (fopen(path, mode));
}

static int
read_enc_file(struct stream *stream, void *buf, size_t len)
{
//	ENTER_FUNC();
//	DMCLOG_D("len=%ld",len);
	long n = 0;
	int i = 0;
	int nwrites=0;
	long mod_len;
	long need_read_encrypt_len=0;
//	AES_KEY aes;
	assert(stream->chan.fd != -1);
//	char *input_buf = NULL;

//重新换算为可正确解析的一段数据 AES_BLOCK_SIZE 整数倍
	if(len < AES_BLOCK_SIZE)
		mod_len = AES_BLOCK_SIZE;
	else
    	mod_len = (len/AES_BLOCK_SIZE)*AES_BLOCK_SIZE;
	//DMCLOG_D("len=%ld,mod=%ld",len,mod_len);

	char *input_buf = (char *)malloc(mod_len);
	memset(input_buf,0,(mod_len));
	
	(void)lseek64(stream->chan.fd, stream->offset, SEEK_SET);

	n = read(stream->chan.fd,input_buf,mod_len);
	if (n <0) {
		DMCLOG_D("fread! error\n");
		return -1;
	}
	//DMCLOG_D("n=%ld",n);

	need_read_encrypt_len=n;
	//DMCLOG_D("need_read_encrypt_len=%ld",need_read_encrypt_len);
	//DMCLOG_D("stream->offset=%lld",stream->offset);
	stream->offset = stream->offset + need_read_encrypt_len;
	//	DMCLOG_D("stream->offset=%lld",stream->offset);
	char aes_input_block[AES_BLOCK_SIZE]={0};
	char aes_output_block[AES_BLOCK_SIZE]={0};
    
	int start_byte = stream->start_byte;
	for(i=0; i< need_read_encrypt_len; i = i+AES_BLOCK_SIZE){	
		memcpy(aes_input_block,input_buf+i,AES_BLOCK_SIZE);
		AES_ecb_encrypt(aes_input_block, aes_output_block, &(stream->aes), AES_DECRYPT);		
		if(len < AES_BLOCK_SIZE)
			{
				memcpy(buf,aes_output_block,len);
				nwrites += len;
				break;
			}
			
		if(stream->i == 0){
		//first read first decrypt block
		/**/
			if(i==0)
			{
				//DMCLOG_D("aes_output_block=%s,i=%d,start_byte=%d",aes_output_block,i,start_byte);
				memcpy(buf,aes_output_block+start_byte,AES_BLOCK_SIZE-start_byte);
				nwrites += (AES_BLOCK_SIZE-start_byte);
				//DMCLOG_D("buf=%s",buf);
			}
			else{
				memcpy(buf+(i-start_byte),aes_output_block,AES_BLOCK_SIZE);
				nwrites += AES_BLOCK_SIZE;
				//DMCLOG_D("buf=%s",buf);
			}
		}else {
			memcpy(buf+i,aes_output_block,AES_BLOCK_SIZE);
			nwrites += AES_BLOCK_SIZE;
		}

		//need_read_encrypt_len -= AES_BLOCK_SIZE;
		//DMCLOG_D("nwrites=%d,%lld",nwrites,stream->nwrites);
	}

	stream->i++;		
	safe_free(input_buf);
	//n = read(stream->chan.fd, buf, len);
	stream->nwrites += nwrites;
	//DMCLOG_D("stream->nwrites=%lld",stream->nwrites);
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
get_enc_file(struct conn *c)
{
	big_int_t	cl; /* Content-Length */
	//DMCLOG_D("c->loc.content_len=%lld",c->loc.content_len);	
	cl = (big_int_t) c->loc.content_len;
//	DMCLOG_D("cl = %lld,stp->st_size = %lld,c->offset = %lld,c->length=%lld,stp->st_mtime = %lu",cl,stp->st_size,c->offset,c->length,stp->st_mtime);
	/* If Range: header specified, act accordingly */
	(void) lseek64(c->loc.chan.fd, c->offset/AES_BLOCK_SIZE, SEEK_SET);
	//DMCLOG_D("c->offset=%lld",c->offset);
	c->loc.offset = ((c->offset+1)/AES_BLOCK_SIZE)*AES_BLOCK_SIZE;
	c->loc.start_byte = c->offset % AES_BLOCK_SIZE;
	//DMCLOG_D("c->loc.offset=%lld",c->loc.offset);
	if(c->length <= 1)
	{
		cl = cl - c->offset;
	}else{
		cl = c->length - c->offset;
	}
	c->ctx->nactive_fd_cnt++;
	
	c->loc.content_len = cl;
	//DMCLOG_D("sizeof(big_int_t) = %d, cl = %lld, c->loc.content_len = %lld", sizeof(big_int_t), cl, c->loc.content_len);
	c->loc.io_class = &io_enc_file;
	c->loc.flags |= FLAG_R | FLAG_ALWAYS_READY;
}


int read_from_record(struct encrypted_file **dn,char *path)//读入数据
 {	 
 	ENTER_FUNC();

	  FILE *record_fd;
	  int enRet;
//	  struct encrypted_file **dn=&e_file_list;
	  struct encrypted_file *cur=NULL;
	  int ret = 0;
	  int i = 0;
	  if(access(path,R_OK)!=0){// 不存在
		  if((record_fd = fopen(path,"wb+"))==NULL)//改动：路径
		  {
			// DMCLOG_D("task list file does not exist error1[errno:%d]\n",errno);
			 return -1;
		  }
	  }else  if((record_fd = fopen(path,"r"))==NULL)//改动：路径
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
	struct dirent	*dp = NULL;
	struct stat	st;
	char *path = stream->chan.dir.path;
	//read_from_record(&e_file_list);
	struct encrypted_file *p = NULL;
	int i = 0, nwritten = 0;
	struct conn	*c = stream->conn;
	char	file[FILENAME_MAX], line[FILENAME_MAX + 512];
	int		n = 0;
	do{

		if (len < sizeof(line))
		{
			DMCLOG_D("out of stream");
			break;
		}
		if ((dp = readdir(stream->chan.dir.dirp)) == NULL) {
			stream->flags |= FLAG_CLOSED;
			stream->flags &= ~(FLAG_R | FLAG_W | FLAG_ALWAYS_READY);
			n = my_snprintf(line, sizeof(line)," ], \"startIndex\": %lld,\"count\": %u, \"totalCount\": 1, \"totalPage\": 1, \"pageSize\": %u },\"header\": { \"cmd\": %d, \"seq\": %d, \"error\": %d }}",
				c->offset,c->nfiles,c->nfiles,c->cmd,c->seq,c->error);
			(void) memcpy(buf, line, n);
			buf = (char *) buf + n;
			nwritten += n;
			len -= n;
			DMCLOG_D("read finish");
			break;
		}
		DMCLOG_D("dp->d_name=%s,stream->chan.dir.path=%s",dp->d_name,stream->chan.dir.path);
		if (!strcmp(dp->d_name,".")||!strcmp(dp->d_name,"..")) {
			continue;
		}
		(void) my_snprintf(file, sizeof(file),
		    "%s%s%s", stream->chan.dir.path, "/", dp->d_name);
		(void) my_stat(file, &st);

		if(S_ISDIR(st.st_mode)){
		//an encrypted dir 
			p = find_the_real_file_node(dp->d_name,e_dir_list);			
		}
		else {
		//an encrypted file
			p = find_the_real_file_node(dp->d_name,e_file_list);			
		}

		if(p == NULL){
			continue;// maybe not a encrypted file or encrypted data is corrupted. ignore this.
		}
		if(c->length <= 0)
		{
			if(c->nfiles == 0)
			{
				n = my_snprintf(line, sizeof(line),
			    "{ \"isFolder\": %d, \"size\": %lld, \"data\": %u, \"name\": \"%s\"}",
			    S_ISDIR(st.st_mode), p->size,p->mod_time, p->name);
			}else{
				n = my_snprintf(line, sizeof(line),
			    ",{ \"isFolder\": %d, \"size\": %lld, \"data\": %u, \"name\": \"%s\"}",
			    S_ISDIR(st.st_mode), p->size,p->mod_time, p->name);
			}
			
			(void) memcpy(buf, line, n);
			buf = (char *) buf + n;
			nwritten += n;
			len -= n;
			c->nfiles++;
		}
	} while (dp != NULL);
		
#if 0
		if(!p)//p=NULL last read
		{
			stream->flags |= FLAG_CLOSED;
			stream->flags &= ~(FLAG_R | FLAG_W | FLAG_ALWAYS_READY);
			n = my_snprintf(line, sizeof(line)," ], \"count\": %d, \"totalCount\": %d },\"header\": { \"cmd\": %d, \"seq\": %d, \"error\": %d }}",
				c->nfiles,c->nfiles,c->cmd,c->seq,c->error);
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
		if(c->nfiles == 0)
		{
			n = my_snprintf(line, sizeof(line),
		    "{ \"isFolder\": 0, \"size\": %lld,\"data\": %u, \"name\": \"%s\"}",
		     p->size,p->mod_time,p->name);
		}else{
			n = my_snprintf(line, sizeof(line),
		   ",{ \"isFolder\": 0 , \"size\": %lld,\"data\": %u, \"name\": \"%s\"}",
		     p->size,p->mod_time,p->name);
		}
		(void) memcpy(buf, line, n);
		buf = (char *) buf + n;
		nwritten += n;
		len -= n;
		c->nfiles++;
#endif		
		/*
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
	}*/
	//destroy_record(e_file_list);
	return (nwritten);
}

static int is_enc_dir_exist(char *src){
	int ret=0;
	
	return ret;
}

void get_enc_dir(struct conn *c){

	if(!strcmp(c->loc.chan.dir.path,SAFE_BOX_PATH)){

	}else {// need find the real dir path
		//find_the_real_file(c->src_path);
		char *p = find_the_real_file(c->src_path,e_dir_list);
		if(p != NULL)
		{
			strcpy(c->loc.chan.dir.path,p);
		}
		else {
			c->loc.flags |= FLAG_CLOSED;
			return;
		}
	}
	
	if ((c->loc.chan.dir.dirp = opendir(c->loc.chan.dir.path)) == NULL) {
		c->loc.flags |= FLAG_CLOSED;
	} else {
		c->loc.io.head = my_snprintf(c->loc.io.buf, c->loc.io.size,
		    "{ \"data\": { \"filelist\": [");
		io_clear(&c->rem.io);
		c->loc.io_class = &io_enc_dir;
		c->loc.flags |= FLAG_R | FLAG_ALWAYS_READY;
	}
}

int get_md5sum(const char *src,char *result,int len){
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
	//char *mac="84:5D:D7:A1:12:27";
	char mac[18]={0};
	//char *sn="84:5D:D7:A1:12:27";
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

	//unsigned int key[4] = {0xd55fda32,0x14d99bd7,0x9f7a0ef8,0x972df216};
	char tmp[8]={0};
	unsigned int key[4];	

	char *p=k;
	for(i=0;i<4;i++){
		strncpy(tmp,p,8);
		tmp[8]='\0';
		p=p+8;
		key[i]=str_to_uint(tmp);
		//DMCLOG_D("k[%d]=%x",i,key[i]);
	}

}

int decrypt_fd(FILE *fp_read, FILE *fp_write)
{

	unsigned int key[4] = {0xd55fda32,0x14d99bd7,0x9f7a0ef8,0x972df216};
	unsigned int iv[4] = {0x7f444fd5,0xd2002d29,0x4b96c34d,0xc57d297e};
	char *k=(char*)malloc(AES_KEY_LEN);
	memset(k,0,AES_KEY_LEN);
	gen_k(k);

	int ret = 0;
	long fsize = 0;
	int rd_byte = 0, wr_byte = 0;
	int fd;

	
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
		for(i=0;i<MAX_DMA_LEN;i=i+16){
			memcpy(input,input_block+i,AES_BLOCK_SIZE);
			AES_ecb_encrypt(input, output, &aes, 
				AES_DECRYPT);
			memcpy(output_block+i,output,AES_BLOCK_SIZE);
		}
		
		len -= rd_byte;
	//	p_debug("1 output=%s",output);

		if (len == 0) {
			pad = output_block[rd_byte - 1];
				
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

	return fsize;
}

int decrypt_fd2(struct file_encrypt_info *pInfo,struct file_dnode *dn,FILE *fp_read, FILE *fp_write)
{
	int ret = 0;
	int rd_byte = 0, wr_byte = 0;
	int fd;
	int error, i;
	unsigned char input[AES_BLOCK_SIZE];
	unsigned char output[AES_BLOCK_SIZE];
	unsigned char input_block[MAX_DMA_LEN];
	unsigned char output_block[MAX_DMA_LEN];
	unsigned int align_len = 0;
	unsigned int pad = 0;
	off_t len;
	time_t last_time = 0;	
	
	if (fseek(fp_read, 0, SEEK_SET) == -1) {
		printf("fseek SEEK_SET error!\n");
		return -1;
	}
	len = pInfo->total_size;
	
	while(len) {
		if(pInfo->quit_status == -1) 
		{
			DMCLOG_E("quit the decrypt task");
			return 0;
		}
		rd_byte = fread(input_block, 1, MAX_DMA_LEN, fp_read);
		if (!rd_byte) {
			printf("fread! error,errno=%d\n",errno);
			return -1;
		}
		for(i=0;i<MAX_DMA_LEN;i=i+16){
			memcpy(input,input_block+i,AES_BLOCK_SIZE);
			AES_ecb_encrypt(input, output, &(pInfo->aes),AES_DECRYPT);
			memcpy(output_block+i,output,AES_BLOCK_SIZE);
		}
		
		len -= rd_byte;
	//	p_debug("1 output=%s",output);

		if (len == 0) {
			pad = output_block[rd_byte - 1];
				
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
		pInfo->finished_size += wr_byte+pad;
		pInfo->status = 1;
		time_t now_time = time(NULL);
		//DMCLOG_D("now_time=%d",now_time);
		if(now_time > last_time+2){
			last_time = now_time;
			ret = decrypt_handle_tcp_inotify(error,pInfo,dn);
			if(ret != 0){
				DMCLOG_E("encrypt handle tcp inotifu error");
				break;
			}
		}		
	}
	return 0;
}


int encrypt_fd(struct file_encrypt_info *pInfo,struct file_dnode *dn,FILE *fp_read,FILE *fp_write)
{
	unsigned char input[AES_BLOCK_SIZE ];
	unsigned char output[AES_BLOCK_SIZE];
	unsigned char input_block[MAX_DMA_LEN];
	unsigned char output_block[MAX_DMA_LEN];

	unsigned int align_len = 0;
	unsigned int pad, i;
	int ret = -1;
	int rd_byte = 0, wr_byte = 0;
	int error;
	time_t last_time = 0;
	
	//encrypt file
  	
	if (fseek(fp_read, 0, SEEK_SET) == -1) {
		error = DM_ERROR_ENCRYPT_SEEK_FAIL;
		pInfo->status = -1;
		goto EXIT;
	}

	if (fseek(fp_write, 0, SEEK_SET) == -1) {
		error = DM_ERROR_ENCRYPT_SEEK_FAIL;
		pInfo->status = -1;
		goto EXIT;
	}

	uint64_t len = pInfo->total_size;

	while(len){
		if(pInfo->quit_status == -1) 
		{
			DMCLOG_E("quit the decrypt task");
			return 0;
		}
		rd_byte = fread(input_block, 1, MAX_DMA_LEN, fp_read);
		if (!rd_byte) {
			error = DM_ERROR_ENCRYPT_SEEK_FAIL;
			pInfo->status = -1;
			goto EXIT;
		}

		if (!(len - rd_byte)) {		//last read
			align_len = ((rd_byte / AES_BLOCK_SIZE) + 1) * AES_BLOCK_SIZE;

			pad = AES_BLOCK_SIZE - rd_byte % AES_BLOCK_SIZE;
			for (i = rd_byte; i < align_len; i++) {
				input_block[i] = pad;
			}

			for(i=0;i<MAX_DMA_LEN;i=i+AES_BLOCK_SIZE){
					memcpy(input,input_block+i,AES_BLOCK_SIZE);					
					AES_ecb_encrypt(input, output, &(pInfo->aes), AES_ENCRYPT);					
					memcpy(output_block+i,output,AES_BLOCK_SIZE);
			}
			wr_byte = fwrite(output_block, 1, rd_byte + pad, fp_write);
			if (!wr_byte) {
				error = DM_ERROR_ENCRYPT_WRITE_FAIL;
				pInfo->status = -1;
				goto EXIT;
			}
			
		}
		else {

			for(i=0;i<MAX_DMA_LEN;i=i+AES_BLOCK_SIZE){
					memcpy(input,input_block+i,AES_BLOCK_SIZE);					
					AES_ecb_encrypt(input, output, &(pInfo->aes), AES_ENCRYPT);					
					memcpy(output_block+i,output,AES_BLOCK_SIZE);
			}
			wr_byte = fwrite(output_block, 1, rd_byte, fp_write);
			if (!wr_byte) {
				error = DM_ERROR_ENCRYPT_WRITE_FAIL;
				pInfo->status = -1;
				goto EXIT;
			}				
		}
		len -= rd_byte;
		pInfo->finished_size += rd_byte;
		pInfo->status = 1;
		
		time_t now_time = time(NULL);
		//DMCLOG_D("now_time=%d",now_time);
		if(now_time > last_time+2){
			last_time = now_time;
			ret = encrypt_handle_tcp_inotify(error,pInfo,dn);
			if(ret != 0){
				DMCLOG_E("encrypt handle tcp inotifu error");
				break;
			}
		}
	}
	ret = handle_encrypt_file_insert_cmd(pInfo->file_uuid,dn->virtual_path,pInfo->bIsRegularFile,pInfo->disk_uuid,dn->dest_encrypt_path,pInfo->total_size);
	if(ret < 0)
	{
        DMCLOG_E("handle_encrypt_file_insert_cmd (%s) failed", dn->virtual_path); 
    }
	if(ret == 0){
	/*	if(rm(dn->fullname)<0){
			error = DM_ERROR_ENCRYPT_SAVE_FAIL;
			pInfo->status = -1;
		}
		DMCLOG_D("del %s",dn->fullname);
	*/	//TODO:delete the source file
	}
	else{
		error = DM_ERROR_ENCRYPT_SAVE_FAIL;
		pInfo->status = -1;
		goto EXIT;	
	}	
	/*
	ret = create_insert_to_db_task(pInfo);
	if(ret < 0)
	{
		DMCLOG_D("write to db (%s) failed" ,pInfo->src_path);
		error = DM_ERROR_ENCRYPT_SAVE_FAIL;
		goto EXIT;
	}
	if(ret == 0){
		//TODO:delete the source file
	}
	else{
		error = DM_ERROR_ENCRYPT_SAVE_FAIL;
		pInfo->status = -1;
		goto EXIT;	
	}
*/
	pInfo->status = 2;		

	
EXIT:
	return  ret;
}
char * generate_encrypted_file_name(const char *src){

	char *random=NULL;
	random=base64_encode(src,strlen(src));	

	char *dest_name=(char*)malloc(ENCRYPTED_FILE_NAME_LENGTH+1);
	memset(dest_name,0,ENCRYPTED_FILE_NAME_LENGTH+1);
	//DMCLOG_D("src=%s",src);	
	get_md5sum(random,dest_name,ENCRYPTED_FILE_NAME_LENGTH);

	//sprintf(dest,"%s/.%s",SAFE_BOX_PATH,dest_name);

//	DMCLOG_D("dest=%s",dest);
//	free(random);
	return dest_name;
}
int get_filename_from_path(char *abs_file_path,char *name){

    char *p=strrchr(abs_file_path,'/');
    printf("p=%s",p);
    if(p)
    {
        strncpy(name,p+1,(abs_file_path+strlen(abs_file_path)-p));
        DMCLOG_D("filename=%s",name);
		return 0 ;
    }
    else return -1;

}
int encrypt_dir(struct file_encrypt_info *pInfo,struct file_dnode *dn){
	ENTER_FUNC();
	int error;
	int ret = mkdir(dn->dest_encrypt_path, S_IRWXU | S_IRWXG | S_IRWXO);//创建加密目录  
	if(ret == 0 || errno == EEXIST){
		DMCLOG_D("create %s succ",dn->dest_encrypt_path);		
	}else
	{
		pInfo->status = -1;
		return  DM_ERROR_ENCRYPT_WRITE_FAIL;
	}
	//pInfo->file_name = dn->name;
	//pInfo->orig_path  = dn->fullname + strlen(DOCUMENT_ROOT);
	pInfo->total_size	 = 16384;
	pInfo->finished_size = pInfo->total_size;
	pInfo->status = 1;
	//pInfo->src_path = dn->virtual_path;
	//pInfo->dest_path = dn->dest_encrypt_path;
	ret = handle_encrypt_file_insert_cmd(pInfo->file_uuid,dn->virtual_path,pInfo->bIsRegularFile,pInfo->disk_uuid,dn->dest_encrypt_path,pInfo->total_size);
	if(ret < 0)
	{
        DMCLOG_E("handle_encrypt_file_insert_cmd (%s) failed", pInfo->src_path); 
    }
	if(ret == 0){
		//TODO:delete the source file
	}
	else{
		error = DM_ERROR_ENCRYPT_SAVE_FAIL;
		pInfo->status = -1;
		goto EXIT;	
	}	
	return 0;
EXIT:
	return ret;
}

int encrypt_file(struct file_encrypt_info *pInfo,struct file_dnode *dn){
	ENTER_FUNC();
	int ret;
	FILE *fp_read, *fp_write;
	fp_read = fopen(dn->fullname, "rb+");
	if (!fp_read) {
		DMCLOG_E("fopen %s!\n",dn->fullname);
		return -1;
	}
	fp_write = fopen(dn->dest_encrypt_path, "wb+");
	if (!fp_write) {
		DMCLOG_E("fp_write fopen (%s)errno %d!\n",dn->dest_encrypt_path,errno);
		return -1;
	}
	//pInfo->file_name  = dn->name;
	//pInfo->orig_path  = dn->fullname + strlen(DOCUMENT_ROOT);
	pInfo->total_size = dn->size;
	pInfo->finished_size = 0;
	//pInfo->src_path = dn->virtual_path;
	//pInfo->dest_path = dn->dest_encrypt_path;
	pInfo->bIsRegularFile = 1;
	struct stat s;
	stat(dn->fullname,&s);
	pInfo->ctime = s.st_ctime;
	pInfo->mtime = s.st_mtime;
	pInfo->atime = s.st_atime;
	ret = encrypt_fd(pInfo,dn,fp_read,fp_write);
	fclose(fp_read);
	fclose(fp_write);
	return ret;
}

int decrypt_dir(struct file_encrypt_info *pInfo,struct file_dnode *dn){
	int ret;
	ret = mkdir(dn->fullname, S_IRWXU | S_IRWXG | S_IRWXO);
	if(ret == 0 || errno == EEXIST){
		//DMCLOG_D("create %s succ",dn->dest_encrypt_path);		
	}else
	{
		pInfo->status = -1;
		return  DM_ERROR_DECRYPT_DIR_FAIL;
	}
	return 0;	
}
int decrypt_file(struct file_encrypt_info *pInfo,struct file_dnode *dn){
	int ret;
	FILE *fp_read, *fp_write;
	//DMCLOG_D("fullname=%s",dn->fullname);
	fp_read = fopen(dn->dest_encrypt_path, "rb+");
	if (!fp_read) {
		DMCLOG_E("fopen %s!\n",dn->dest_encrypt_path);
		return -1;
	}
	//DMCLOG_D("dest_encrypt_path=%s",dn->dest_encrypt_path);
	fp_write = fopen(dn->fullname, "wb+");
	if (!fp_write) {
		DMCLOG_E("fp_write fopen (%s)errno %d!\n",dn->fullname,errno);
		return -1;
	}

	pInfo->total_size = dn->size;
	pInfo->finished_size = 0;
	ret = decrypt_fd2(pInfo,dn,fp_read,fp_write);
	if(pInfo->total_size == pInfo->finished_size){
		if(rm(dn->dest_encrypt_path)<0){
			DMCLOG_E("del (%s) fail!\n",dn->dest_encrypt_path);			
			return -1;
		}
		DMCLOG_D("del %s",dn->dest_encrypt_path);
	}
	fclose(fp_read);
	fclose(fp_write);
	return ret;
}


int get_file_info(char *path, struct encrypted_file *dn){

	DMCLOG_D("path=%s",path);
	char *p = strrchr(path,'\/');
	int len = strlen(path);
	if(p != NULL)
	{
		if(path[len-1] == '\/')//is a dir ended with '/'
		{
			char q[MAX_FILE_PATH_LEN] = {0};
			strncpy(q,path,len-1);
			char *r = strrchr(q,'\/');
			strncpy(dn->name,r+1,MAX_FILE_NAME_LEN);
		}else //is a file or a dir 
		{
			strncpy(dn->name,p+1,MAX_FILE_NAME_LEN); 	
		}
		DMCLOG_D("dn->name=%s",dn->name);
		//get_parent_dir_from_path(path, dn->parent_dir_path);
		//DMCLOG_D("dn->parent_dir_path=%s",dn->parent_dir_path);
		struct stat statbuff;  
		if(stat(path, &statbuff) < 0){  
			DMCLOG_E("stat file error");
	        return -1;  
	    }else{  
	    	DMCLOG_D("stat file ok");
			DMCLOG_D("statbuff.st_size=%lld",statbuff.st_size);
	        dn->size = statbuff.st_size;  
			DMCLOG_D("dn-size=%lld\n",dn->size);
			DMCLOG_D("size=%lld",dn->size);
			dn->mod_time=statbuff.st_mtime;
			DMCLOG_D("mtime=%ld",dn->mod_time);
	    }
	}
	
}

int delete_encrypt_file(const char *src){
	char dest[MAX_FILE_PATH_LEN]={0};
	strcpy(dest , find_the_real_file(src,e_file_list));
	if(!strcmp(dest,"")){
		return -1;//not found
    }
    if(rm(dest) < 0)
	{
		return -2;//rm error
	}
	if(del_from_record()<0){

	}
	return 0;
}
int write_list_to_file(int is_file)//写出数据
{
	//p_debug("access write_list_to_file");	

	struct encrypted_file *dn = NULL;

	unsigned i = 0;
	int enRet = 0;
	char path[64]={0};
	FILE *record_fd=NULL;
	if(is_file == 0){
		dn = e_dir_list;
		strcpy(path,ENC_DATA_DIR_PATH);
	}else {
		dn = e_file_list;
		strcpy(path,ENC_DATA_FILE_PATH);
	}
	//display_task_dnode(dn);
	if((record_fd = fopen(path,"w+"))== NULL)//改动：路径
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
int del_from_record(int is_file,char *dest){

	struct encrypted_file *dn =NULL; 
    struct encrypted_file **pdn = NULL;//&e_file_list;	
    if(is_file == 1){
		//dn = ;
		dn = e_file_list;
		pdn = &e_file_list;
		
	}else if(is_file == 0){
		dn = e_dir_list;
		pdn = &e_dir_list;
		//is_file = 0;
	}else {
		return -2;
	}

}

int add_to_record(int is_file,char *src, char *dest)
{
#if 0
	DMCLOG_D("access add_to_record");
	// 2. query db.
    if((msg = new_message()) == NULL)
    {
        DMCLOG_E("create_sync_message failed");
        return EMESSAGE_NEW;
    }
    // add end.
    msg->msg_header.ret_flag = MSG_NO_RETURN;
    msg->msg_header.m_type = MSG_DB_FILE_DELETE; 
	get_db_opr_obj(msg).data.delete_data.path = src_path + strlen(DOCUMENT_ROOT);
	get_db_opr_obj(msg).data.delete_data.cmd = FILE_TABLE_DELETE_INFO;
	DMCLOG_D("path = %s",get_db_opr_obj(msg).data.delete_data.path);
    pthread_mutex_lock(&disk_info->mutex);
	disk_info->g_db_write_task.msg_cb(&disk_info->g_db_write_task,msg);
    pthread_mutex_unlock(&disk_info->mutex);
    // check result if ok
    if(get_db_opr_obj(msg).ret != RET_SUCCESS)
    {
        DMCLOG_E("query file full path failed");
        free_message(&msg);
        return ECLIENT_PARAM;
		
    }
	free_message(&msg);
#endif
#if 1
	int i;
    char tmpbuf[128];
   //int  is_file = 0;
    struct encrypted_file *dn =NULL; 
    struct encrypted_file **pdn = NULL;//&e_file_list;
	struct encrypted_file *cur=(struct encrypted_file *)malloc(sizeof(struct encrypted_file));
	if(cur == NULL)
	{
		return -1;
	}
	memset(cur,0,sizeof(struct encrypted_file));
	/**/
	if(is_file == 1){
		//dn = ;
		dn = e_file_list;
		pdn = &e_file_list;
		
	}else if(is_file == 0){
		dn = e_dir_list;
		pdn = &e_dir_list;
		//is_file = 0;
	}else {
		return -2;
	}
	
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
			
			dn = (dn)->next;
	}
	cur->next = *pdn;
	*pdn = cur;

	int ret = write_list_to_file(is_file);
#endif	
	DMCLOG_D("leave add_to_record");
	return ret;
}
/*
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
			goto EXIT;
		}

		ret = add_to_record(&e_file_list,src,dest);
EXIT:
		safe_free(dest);
		fclose(fp_read);
		fclose(fp_write);
		return ret;
}
*/
int encrypt_file_init(){
	ENTER_FUNC();
	if(is_dir_exist(SAFE_BOX_PATH) < 1){
	//creat dir

		if(mkdir(SAFE_BOX_PATH,S_IRWXU | S_IRWXG | S_IRWXO) == 0){
			int res = 0;
			char file_uuid[16]="1234567890";
			char disk_name[32]={0};
			char disk_root[64]={0};
			char disk_uuid[32]={0};
			char path[64]={0};
			get_parent_dir_from_path(SAFE_BOX_PATH,disk_root);
			//get_parent_path(SAFE_BOX_PATH, disk_root);
			//DMCLOG_D("disk_root=%s",disk_root);
			res = read_mark_file(disk_root,disk_uuid);
			//DMCLOG_D("disk_uuid=%s",disk_uuid);
			int bIsRegularFile = 0;//dir
			
			strcpy(path,SAFE_BOX_PATH);
			struct stat s;
			stat(path,&s);
			res = handle_encrypt_file_insert_cmd(file_uuid,path,bIsRegularFile,disk_uuid,path,s.st_size);
			if(res < 0)
			{
		        DMCLOG_E("handle_encrypt_file_insert_cmd (%s) failed", SAFE_BOX_PATH); 
		    }

		}
		else {
			DMCLOG_E("error create %s",SAFE_BOX_PATH);
			return -1;
		}
	}
//	read_from_record(&e_file_list,ENC_DATA_FILE_PATH);
//	read_from_record(&e_dir_list,ENC_DATA_DIR_PATH);
}
int encrypt_file_exit(){
//	destroy_record(e_file_list);
//	destroy_record(e_dir_list);
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

