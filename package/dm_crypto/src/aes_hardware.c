/*
 * =============================================================================
 *    Filename: aes_hardware.c
  *    Description:  achieve aes use x1000 DMA
 *
 *   Version:  1.0
 *   Created:  2017/02/21 15:41
 *   Revision:  none
 *   Compiler:  gcc
 *
 *   Author:  hu.jiang<jh5254622@126.com> 18520833995
 *   Organization: longsys/DAMAI
 *
 * =============================================================================
 */

#include "dm_crypto_internal.h"
#include "md5.h"

#ifdef AES_HARDWARE_X1000
#include "jz_aes_v12.h"

#define AES_BLOCK_SIZE	16
#define DMA_MAX_LEN	DATA_MAX_LEN
#define HARDWARE_PATH "/dev/jz-aes"

inline int get_hardware_hd(void)
{
	int fd = 0;
	fd = open(HARDWARE_PATH, O_RDWR);
	if(fd < 0) {
		perror("open jz-aes error");
		fd = 0;
	}
	return fd;
}

DMCRYPTO_RET encrypt_fp_hardware(FILE *fpRead, FILE *fpWrite,unsigned char *ikey,
	CRYPTO_TYPE cryptoType,unsigned char *filename,DMCRYPTO_RET (*progress)(int percent))
{
//	unsigned char key[KEY_MAX_LEN+1]={0};
    unsigned char iv[KEY_MAX_LEN]={'d','a','m','a','i','8','8','8','d','a','m','a','i','8','8','8'};
//	const EVP_CIPHER *cipher = NULL;
	unsigned char input[DATA_MAX_LEN+16]={0};
	unsigned char output[DATA_MAX_LEN+16]={0};
	struct aes_key aes_key;
	struct aes_data aes_data;
	int ret = 0;
	long fsize = 0;
	int rd_byte = 0, wr_byte = 0;
	unsigned int align_len = 0;
	unsigned int pad = 0, len = 0, i = 0;
	DMCRYPTO_FILE_HEADER *header = NULL;
	char *keyMD5code = NULL;
	int cryptoLen = 0,curProgress = 0;;
	int hd = 0;
#ifdef ENABLE_TIMER
	clock_t start,end;
	double TheTimes;
	double temp = 0;
#endif

	switch(cryptoType)
	{
		case AES_ECB_128:
			aes_key.bitmode = AES_KEY_128BIT;
			aes_key.aesmode = AES_MODE_ECB;
//			cipher = EVP_aes_128_ecb();
			break;
		case AES_ECB_192:
			aes_key.bitmode = AES_KEY_192BIT;
			aes_key.aesmode = AES_MODE_ECB;
//			cipher = EVP_aes_192_ecb();
			break;
		case AES_ECB_256:
			aes_key.bitmode = AES_KEY_256BIT;
			aes_key.aesmode = AES_MODE_ECB;
//			cipher = EVP_aes_256_ecb();
			break;
		case AES_CBC_128:
			aes_key.bitmode = AES_KEY_128BIT;
			aes_key.aesmode = AES_MODE_CBC;
//			cipher = EVP_aes_128_cbc();
			break;
		case AES_CBC_192:
			aes_key.bitmode = AES_KEY_192BIT;
			aes_key.aesmode = AES_MODE_CBC;
//			cipher = EVP_aes_192_cbc();
			break;
		case AES_CBC_256:
			aes_key.bitmode = AES_KEY_256BIT;
			aes_key.aesmode = AES_MODE_CBC;
//			cipher = EVP_aes_256_cbc();
			break;
		default:
			DMCLOG_E("INVALID CRYPTOTYPE!\n");
			return DMCRYPTORET_INVALID_CRYPTOTYPE;
	}

	aes_key.key = (char *)ikey;
	if(strlen(ikey) <= 16)
		aes_key.keylen = 16;
	else if(strlen(ikey) <= 24)
		aes_key.keylen = 24;
	else
		aes_key.keylen = 32;
	aes_key.iv = (char *)iv;
	aes_key.ivlen = 16;
	aes_key.encrypt = 0;



	if (fseek(fpRead, 0, SEEK_END) == -1) {
		DMCLOG_E("fseek SEEK_END error!\n");
		ret = DMCRYPTORET_UNKNOW_ERROR;
		goto HARDWARE_ENCRYPT_EXIT;
	}

	fsize = ftell(fpRead);
	if (fsize == -1) {
		DMCLOG_E("ftell!\n");
		ret = DMCRYPTORET_UNKNOW_ERROR;
		goto HARDWARE_ENCRYPT_EXIT;
	}

	if (fseek(fpRead, 0, SEEK_SET) == -1) {//SEEK fpRead position
			DMCLOG_E("fseek SEEK_SET error!\n");
			ret = DMCRYPTORET_INVALID_FILE;
			goto HARDWARE_ENCRYPT_EXIT;
	
		}

	if (fseek(fpWrite, FILE_DATA_POSITION, SEEK_SET) == -1) {//SEEK fpWrite position
		DMCLOG_E("fseek SEEK_SET error!\n");
		ret = DMCRYPTORET_INVALID_FILE;
		goto HARDWARE_ENCRYPT_EXIT;

	}

	if (fsize == 0){
		DMCLOG_E("fsize error!\n");
		ret = DMCRYPTORET_UNKNOW_ERROR;
		goto HARDWARE_ENCRYPT_EXIT;
	}
	/****************************
	printf("=======KEY====\n");
	for(i = 0; i < KEY_MAX_LEN;i++)
		printf("%d, ",key[i]);
		printf("==============\n\n");
	printf("=======IV====\n");
	for(i = 0; i < KEY_MAX_LEN;i++)
		printf("%d, ",iv[i]);
		printf("==============\n\n");*/
	
#ifdef ENABLE_TIMER
		start = clock();
#endif
	len = fsize;
	while(len) {

		//get and initial driver handler
		hd = get_hardware_hd();
		if(!hd)
		{	
			ret = DMCRYPTORET_UNKNOW_ERROR;
			goto HARDWARE_ENCRYPT_EXIT;
		}
		
		ret = ioctl(hd, AES_LOAD_KEY, &aes_key);
		if(ret < 0) {
			DMCLOG_E("ioctl! AES_LOAD_KEY error");
			ret = DMCRYPTORET_UNKNOW_ERROR;
			goto HARDWARE_ENCRYPT_EXIT;
		}

		if(progress)//callback
		{	
			curProgress = (cryptoLen*100)/fsize;
			ret = progress(curProgress);
			if(DMCRYPTORET_PAUSE == ret)//PAUSE ENCRYPT
			{
				usleep(10000);//sleep 10ms
				continue;
			}
			else if(DMCRYPTORET_CANCEL == ret)//CANCEL ENCRYPT
			{
				DMCLOG_E("DMCRYPTORET CANCEL!");
				ret = DMCRYPTORET_CANCEL;
				goto HARDWARE_ENCRYPT_EXIT;
			}
		}
		
		rd_byte = fread(input, 1, DMA_MAX_LEN, fpRead);
		if (!rd_byte) {
			DMCLOG_E("fread! error\n");
			perror("fread");
			ret = DMCRYPTORET_READFILE_FAILED;
			goto HARDWARE_ENCRYPT_EXIT;
		}

		if (!(len - rd_byte)) {		//last read
			align_len = ((rd_byte / AES_BLOCK_SIZE) + 1) * AES_BLOCK_SIZE;
			/* PKCS5Padding */
			pad = AES_BLOCK_SIZE - rd_byte % AES_BLOCK_SIZE;
			for (i = rd_byte; i < align_len; i++) {
				input[i] = pad;
			}
			aes_data.input = input;
			aes_data.input_len = rd_byte + pad;
			aes_data.output = output;

			ret = ioctl(hd, AES_DO_CRYPT, &aes_data);

			wr_byte = fwrite(output, 1, rd_byte + pad, fpWrite);
			if (!wr_byte) {
				DMCLOG_E("last fwrite! error\n");
				ret = DMCRYPTORET_WRITEFILE_FAILED;
				goto HARDWARE_ENCRYPT_EXIT;
			}
		}
		else {
			aes_data.input = (char *)input;
			aes_data.input_len = rd_byte;
			aes_data.output = (char *)output;

			ret = ioctl(hd, AES_DO_CRYPT, &aes_data);
			wr_byte = fwrite(output, 1, rd_byte, fpWrite);
			if (!wr_byte) {
				DMCLOG_E("fwrite! error\n");
				ret = DMCRYPTORET_WRITEFILE_FAILED;
				goto HARDWARE_ENCRYPT_EXIT;
			}
		}
		cryptoLen += wr_byte;
		len -= rd_byte;
		close(hd);
	}

	/************write file header***********/
	//write file header
	header = (DMCRYPTO_FILE_HEADER *)calloc(1,(FILE_DATA_POSITION));
	if(NULL == header)
	{
		perror("calloc failed!");
		ret = DMCRYPTORET_UNKNOW_ERROR;
		goto HARDWARE_ENCRYPT_EXIT;
	}
	snprintf(header->encrypt_flag,sizeof(header->encrypt_flag),"%s",ENCRYPT_FILE_FLAG);//encrypt flag
	snprintf(header->filename,FILENAME_MAX_LEN,"%s",filename);//origin name of the file
	
	keyMD5code = MD5Create(ikey);
	if(NULL == keyMD5code)
	{
		DMCLOG_E("ERROR MD5Create failed in %s",__FUNCTION__);
		ret = DMCRYPTORET_UNKNOW_ERROR;
		goto HARDWARE_ENCRYPT_EXIT;

	}
	snprintf(header->key,KEY_MAX_LEN,"%s",keyMD5code);////md5 of key(password)
	free(keyMD5code);
	keyMD5code = NULL;

	header->cryptoType = cryptoType;//crypto Type
	header->length = cryptoLen;

	if (fseek(fpWrite, 0, SEEK_SET) == -1) {//SEEK position
		DMCLOG_E("fseek SEEK_SET error!\n");
		ret = DMCRYPTORET_INVALID_FILE;
		goto HARDWARE_ENCRYPT_EXIT;

	}

	ret = fwrite(header,1,FILE_DATA_POSITION,fpWrite);
	if(FILE_DATA_POSITION != ret)
	{	
		DMCLOG_E("write() failed! ret=%d",ret);
		perror("write");
		ret = DMCRYPTORET_WRITEFILE_FAILED;
		goto HARDWARE_ENCRYPT_EXIT;
	}
	/***********write file header end**********/
	
#ifdef ENABLE_TIMER
	DMCLOG_D("#########ENCRYPT SPEED########## \n");
	end = clock();
	TheTimes = (double)(end - start) / CLOCKS_PER_SEC;
	DMCLOG_D("encrypt %f s.\n",TheTimes);

	TheTimes = (double)fsize/TheTimes;
	temp = TheTimes/1024;
	if(0 == temp)
		DMCLOG_D("encrypt speed:%f byte/S.\n",TheTimes);
	else
	{
		temp = TheTimes/1024/1024;
		if(0 == temp)
		{
			TheTimes = TheTimes/1024;
			DMCLOG_D("encrypt speed:%f KB/S.\n",TheTimes);
		}
		else
		{	 
			TheTimes = TheTimes/1024/1024;
			DMCLOG_D("encrypt speed:%f MB/S.\n",TheTimes);
		}
	}
#endif

	ret = DMCRYPTORET_SUCCESS;
HARDWARE_ENCRYPT_EXIT:
	if(header)
		free(header);
	if(hd)
		close(hd);
	return ret;
}

DMCRYPTO_RET Initial_Decrypt_info_hardware(const unsigned char *filepath,/*IN*/
									const unsigned char *key,/*IN*/
									int *fd,/*OUT*/
									CRYPTO_TYPE *cryptoType)
{
	int fdRead = 0;
//	unsigned char crypto_key[KEY_MAX_LEN+1]={0};
//    unsigned char iv[KEY_MAX_LEN]={0};
	int keylen = 0;
	int ret = 0;
	char *keyMD5code = NULL;
	DMCRYPTO_FILE_HEADER *header = NULL;
	unsigned char realKey[KEY_MAX_LEN+16] = {0};

	if((NULL == filepath) || ((NULL != key)&&(strlen(key) > 32))) {
		DMCLOG_E("please set correctly filepath or key too long\n");
		return DMCRYPTORET_INVALID_FILENAME;
	}

	fdRead = open(filepath,O_RDONLY);//fpRead = fopen(cryptoInfo->inpath, "r");
	if (fdRead == -1) {
		perror("open the filepath (file) failed\n");
		return DMCRYPTORET_OPENFILE_FAILED;
	}

	//get file header
	header = (DMCRYPTO_FILE_HEADER *)calloc(1,(FILE_DATA_POSITION));//sizeof(DMCRYPTO_FILE_HEADER)
	if(NULL == header)
	{
		perror("calloc failed!");
		ret = DMCRYPTORET_UNKNOW_ERROR;
		goto DECRYP_EXIT;
	}

	read(fdRead,header,FILE_DATA_POSITION);
	//check file flag
	if(strcmp(header->encrypt_flag,ENCRYPT_FILE_FLAG))
	{
		DMCLOG_E("cryptoFlag=%s,it should be %s\n",header->encrypt_flag,ENCRYPT_FILE_FLAG);
		ret = DMCRYPTORET_INVALID_INPUTFILE;
		goto DECRYP_EXIT;
	}

	//get file size
	/*fseek(fpRead, 0, SEEK_END);
	fsize = ftell(fpRead);
	fseek(fpRead, 0, SEEK_SET);
	if(header->length != (fsize - FILE_DATA_POSITION))
	{
        DMCLOG_E("DMCRYPTORET_FILESIZE_INCORRECT\nheader->length=%ld,fsize=%ld\n",
			header->length,fsize);
		ret = DMCRYPTORET_FILESIZE_INCORRECT;
		goto DECRYP_EXIT;
	}*/
	
	if(NULL == key)
	{	snprintf(realKey,sizeof(realKey),"%s%s",CRYPTO_KEY_FLAG,DEFAULT_KEY);
	}
	else
		snprintf(realKey,sizeof(realKey),"%s%s",CRYPTO_KEY_FLAG,key);

	keyMD5code = MD5Create(realKey);
	if(NULL == keyMD5code)
	{
        DMCLOG_E("ERROR MD5Create failed in %s",__FUNCTION__);
		ret = DMCRYPTORET_UNKNOW_ERROR;
		goto DECRYP_EXIT;

	}
	if(strcmp(keyMD5code,header->key))
	{
		ret = DMCRYPTORET_INVALID_KEY;
		DMCLOG_E("INVALID KEY\n");
		goto DECRYP_EXIT;
	}	

#if FIX_CRYPTO_TYPE
	*cryptoType = DEFAULT_CRYPTO_TYPE;
#else
	*cryptoType = header->cryptoType;
#endif
	ret = DMCRYPTORET_SUCCESS;

DECRYP_EXIT:
	//if(fpWrite)
	//	fclose(fpWrite);
	if(header)
		free(header);
	if(keyMD5code)
	{
		free(keyMD5code);
		keyMD5code = NULL;
	}
	if(ret != DMCRYPTORET_SUCCESS)
	{
		if(fdRead > 0)
			close(fdRead);
	}
	else
	{

		*fd = fdRead;
	}
	return ret;
}

DMCRYPTO_RET DmDecrypt_file_data_hardware(DMCRYPTO_FD_STRUCT *decryptInfo)
{
	unsigned char key[KEY_MAX_LEN+1]={0};
	unsigned char iv[KEY_MAX_LEN]={'d','a','m','a','i','8','8','8','d','a','m','a','i','8','8','8'};
	unsigned char input[DATA_MAX_LEN+16] = {0};
	int startBytes = 0;
	int endBytes = 0;
	int needReadBytes = 0;
	int cryptoLen = 0;
	int len = 0,pad = 0;
	struct aes_key aes_key;
	struct aes_data aes_data;
	int ret = 0;
	long fsize;
	int rd_byte = 0, wr_byte = 0;
	int hd = 0;

	if(decryptInfo->fd <= 0)
	{
		DMCLOG_E("didn't call function Initial_Decrypt_info?\n");
		return DMCRYPTORET_DECRYPT_UNINITED;
	}
	else if(decryptInfo->size > DATA_MAX_LEN)
	{
		DMCLOG_E("size too big\n");
		return DMCRYPTORET_SIZE_TOOBIG;
	}

	switch(decryptInfo->cryptoType)
	{
		case AES_ECB_128:
			aes_key.bitmode = AES_KEY_128BIT;
			aes_key.aesmode = AES_MODE_ECB;
//			cipher = EVP_aes_128_ecb();
			break;
		case AES_ECB_192:
			aes_key.bitmode = AES_KEY_192BIT;
			aes_key.aesmode = AES_MODE_ECB;
//			cipher = EVP_aes_192_ecb();
			break;
		case AES_ECB_256:
			aes_key.bitmode = AES_KEY_256BIT;
			aes_key.aesmode = AES_MODE_ECB;
//			cipher = EVP_aes_256_ecb();
			break;
		case AES_CBC_128:
			aes_key.bitmode = AES_KEY_128BIT;
			aes_key.aesmode = AES_MODE_CBC;
//			cipher = EVP_aes_128_cbc();
			break;
		case AES_CBC_192:
			aes_key.bitmode = AES_KEY_192BIT;
			aes_key.aesmode = AES_MODE_CBC;
//			cipher = EVP_aes_192_cbc();
			break;
		case AES_CBC_256:
			aes_key.bitmode = AES_KEY_256BIT;
			aes_key.aesmode = AES_MODE_CBC;
//			cipher = EVP_aes_256_cbc();
			break;
		default:
			DMCLOG_E("INVALID CRYPTOTYPE!\n");
			return DMCRYPTORET_INVALID_CRYPTOTYPE;
	}

	if(NULL == decryptInfo->key)
	{	snprintf(key,sizeof(key)-1,"%s%s",CRYPTO_KEY_FLAG,DEFAULT_KEY);
	}
	else
		snprintf(key,sizeof(key)-1,"%s%s",CRYPTO_KEY_FLAG,decryptInfo->key);
	aes_key.key = (char *)key;
	if(strlen(key) <= 16)
		aes_key.keylen = 16;
	else if(strlen(key) <= 24)
		aes_key.keylen = 24;
	else
		aes_key.keylen = 32;//printf("key=%s aes_key.keylen=%d\n",key,aes_key.keylen);

	aes_key.iv = (char *)iv;
	aes_key.ivlen = 16;
	aes_key.encrypt = 1;

	hd = get_hardware_hd();
	if(hd < 0)
	{	
		ret = DMCRYPTORET_UNKNOW_ERROR;
		goto HARDWARE_DECRYPT_EXIT;
	}

	startBytes = ((decryptInfo->startBytes+FILE_DATA_POSITION)/16)*16;
	if(decryptInfo->size % 16)
	{
		endBytes=(((decryptInfo->startBytes+FILE_DATA_POSITION)+(decryptInfo->size))/16+1)*16;
		needReadBytes=endBytes-startBytes;
	}
	else
	{
		needReadBytes = decryptInfo->size;
	}//printf("startBytes=%d\n",startBytes);

	if (lseek(decryptInfo->fd, startBytes, SEEK_SET) == -1) {//SEEK position
		DMCLOG_E("lseek SEEK_SET error!\n");
		ret = DMCRYPTORET_INVALID_FILE;
		goto HARDWARE_DECRYPT_EXIT;
	}
	*(decryptInfo->outputl) = 0;//printf("startBytes=%d\n",startBytes);


	ret = ioctl(hd, AES_LOAD_KEY, &aes_key);
	if(ret < 0) {
		DMCLOG_E("ioctl! AES_LOAD_KEY");
		ret = DMCRYPTORET_UNKNOW_ERROR;
		goto HARDWARE_DECRYPT_EXIT;
	}

#if 0
	int fdout = open("./nzbz.mp3.cbc128.dec",O_RDWR|O_CREAT|O_TRUNC,0744);
	len = lseek(decryptInfo->fd, 0, SEEK_END);
	lseek(decryptInfo->fd, 512, SEEK_SET);//seek to start;
	len -= 512;printf("len=%d\n",len);
#else
	len = needReadBytes;
#endif
	while(len)/**/ 
	{
		needReadBytes = (len > DMA_MAX_LEN)?DMA_MAX_LEN:len;
		rd_byte = read(decryptInfo->fd,input,needReadBytes);
		if (rd_byte < 0) {
			perror("read error!");
			ret = DMCRYPTORET_READFILE_FAILED;
			goto HARDWARE_DECRYPT_EXIT;
		}
		/*for(len = 0; len < rd_byte;len++)
			printf("0x%x, ",*(input+len));
		printf("\n");
		exit(1);*/
		aes_data.input = input;
		aes_data.input_len = rd_byte;
		aes_data.output = decryptInfo->output + cryptoLen;

		ret = ioctl(hd, AES_DO_CRYPT, &aes_data);

		len -= rd_byte;

		if (rd_byte != needReadBytes || (!rd_byte)) //file end?????
		{
			pad = *(decryptInfo->output + rd_byte - 1);//output[rd_byte - 1];
			if (pad > AES_BLOCK_SIZE) {
				DMCLOG_E("pad error\n");
				return -1;
			}printf("aaa rd_byte=%d,need=%d pad=%d\n",rd_byte,needReadBytes,pad);
			cryptoLen += rd_byte - pad;
			break;
		}
		else {
			cryptoLen += rd_byte;
		}
		
	}//while(len)
#if 0
	close(fdout);
	exit(1);
#endif
	*(decryptInfo->outputl) = cryptoLen;//printf("*(decryptInfo->outputl)=%d\n",*(decryptInfo->outputl));
	ret = DMCRYPTORET_SUCCESS;
HARDWARE_DECRYPT_EXIT:
	if(hd > 0)
		close(hd);
	return ret;
}

#endif
