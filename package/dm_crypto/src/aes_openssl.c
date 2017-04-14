/*
 * =============================================================================
 *    Filename: aes_openssl.c
  *    Description:  achieve aes call libopenssl
 *
 *   Version:  1.0
 *   Created:  2017/02/22 10:11
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


/*********************************ENCRYPT**************************************/
#if DISABLE_OPENSSL
//TODO
#else
/*******************************************************************************
** Name: encrypt_fp_openssl
** Input:FILE *fpRead, FILE *fpWrite,unsigned char *ikey,int iType,unsigned char *filename,DMCRYPTO_RET (*progress)(int percent)
** Return: int
** Owner:hu.jiang
** Date: 2017.1.10
** Time: 11:04:41
*******************************************************************************/
DMCRYPTO_RET encrypt_fp_openssl(FILE *fpRead, FILE *fpWrite,unsigned char *ikey,
	CRYPTO_TYPE cryptoType,unsigned char *filename,DMCRYPTO_RET (*progress)(int percent))
{
	unsigned char key[EVP_MAX_KEY_LENGTH]={0};
	unsigned char iv[EVP_MAX_IV_LENGTH]={0};
	unsigned char input[DATA_MAX_LEN+16]={0};
	unsigned char output[DATA_MAX_LEN+16]={0};
    int keylen = 0,curProgress = 0;
	long cryptoLen = 0;
	int ret = 0;
	int readlen = 0,writelen = 0;
	long fsize = 0;
    DMCRYPTO_FILE_HEADER *header = NULL;
	char *keyMD5code = NULL;
	EVP_CIPHER_CTX ctx;
	const EVP_CIPHER *cipher = NULL;
#ifdef ENABLE_TIMER
	clock_t start,end;
	double TheTimes;
	double temp = 0;
#endif

	switch(cryptoType)
	{
		case AES_ECB_128:
			cipher = EVP_aes_128_ecb();
			break;
		case AES_ECB_192:
			cipher = EVP_aes_192_ecb();
			break;
		case AES_ECB_256:
			cipher = EVP_aes_256_ecb();
			break;
		case AES_CBC_128:
			cipher = EVP_aes_128_cbc();
			break;
		case AES_CBC_192:
			cipher = EVP_aes_192_cbc();
			break;
		case AES_CBC_256:
			cipher = EVP_aes_256_cbc();
			break;
		default:
			DMCLOG_E("INVALID CRYPTOTYPE!\n");
			return DMCRYPTORET_INVALID_CRYPTOTYPE;
	}
	if(NULL == cipher)//init failed
	{
		DMCLOG_E("NULL == cipher\n");
		return DMCRYPTORET_INVALID_CRYPTOTYPE;
	}

	if (fseek(fpRead, 0, SEEK_END) == -1) {
		DMCLOG_E("fseek SEEK_END error!\n");
		return DMCRYPTORET_INVALID_FILE;
	}
	fsize = ftell(fpRead);
	if (fsize == -1) {
		DMCLOG_E("ftell!\n");
		return DMCRYPTORET_INVALID_FILE;
	}

	if (fseek(fpRead, 0, SEEK_SET) == -1) {
		DMCLOG_E("fseek SEEK_SET error!\n");
		return DMCRYPTORET_INVALID_FILE;
	}

	if (fsize == 0){
		DMCLOG_E("fsize error!\n");
		return DMCRYPTORET_INVALID_FILE;
	}

	//generate key and iv
	keylen = strlen(ikey);
	EVP_BytesToKey(cipher,EVP_md5(),NULL,ikey,keylen,1,key,iv);
	EVP_CIPHER_CTX_init(&ctx);
	
	ret = EVP_EncryptInit_ex(&ctx,cipher,NULL,key,iv);
	if(!ret)
	{
		DMCLOG_E("EVP_EncryptInit_ex() failed");
		EVP_CIPHER_CTX_cleanup(&ctx);
		return DMCRYPTORET_ENCRYPT_ERROR;
	}

	if (fseek(fpWrite, FILE_DATA_POSITION, SEEK_SET) == -1) {//SEEK position
		DMCLOG_E("fseek SEEK_SET error!\n");
		ret = DMCRYPTORET_INVALID_FILE;
		goto OPENSSL_ENCRYPT_EXIT;

	}
	/****************************
	int i = 0;
	printf("=======KEY====\n");
	for(i = 0; i < EVP_MAX_KEY_LENGTH;i++)
		printf("%d, ",key[i]);
		printf("==============\n\n");
	printf("=======IV====\n");
	for(i = 0; i < EVP_MAX_IV_LENGTH;i++)
		printf("%d, ",iv[i]);
		printf("==============\n\n");*/

	
#ifdef ENABLE_TIMER
		start = clock();
#endif
	while(1)
	{
		if(progress)
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
				goto OPENSSL_ENCRYPT_EXIT;

			}
		}
		
		readlen = fread(input,1,DATA_MAX_LEN,fpRead);
		if(readlen<=0)
			break;
		ret = EVP_EncryptUpdate(&ctx,output,&writelen,input,readlen);
		if(!ret)
		{
			DMCLOG_E("EVP_EncryptUpdate() failed");
			ret = DMCRYPTORET_ENCRYPT_ERROR;
			goto OPENSSL_ENCRYPT_EXIT;

		}
		ret = fwrite(output,1,writelen,fpWrite);
		if(ret != writelen)
		{	
			DMCLOG_E("fwrite() failed! ret=%d",ret);
			perror("fwrite");
			ret = DMCRYPTORET_WRITEFILE_FAILED;
			goto OPENSSL_ENCRYPT_EXIT;
		}
		cryptoLen += ret;
	}
	ret = EVP_EncryptFinal_ex(&ctx,output,&writelen);
	if(!ret)
	{
		DMCLOG_E("EVP_EncryptFinal_ex() failed");
		ret = DMCRYPTORET_ENCRYPT_ERROR;
		goto OPENSSL_ENCRYPT_EXIT;
	}
	ret = fwrite(output,1,writelen,fpWrite);
	cryptoLen += ret;

	/************write file header***********/
	//write file header
	header = (DMCRYPTO_FILE_HEADER *)calloc(1,(FILE_DATA_POSITION));
	if(NULL == header)
	{
		perror("calloc failed!");
		EVP_CIPHER_CTX_cleanup(&ctx);
		return DMCRYPTORET_UNKNOW_ERROR;
	}
	snprintf(header->encrypt_flag,sizeof(header->encrypt_flag),"%s",ENCRYPT_FILE_FLAG);//encrypt flag
	//printf("header->encrypt_flag=%s %d\n",header->encrypt_flag,sizeof(header->encrypt_flag));
	snprintf(header->filename,FILENAME_MAX_LEN,"%s",filename);//origin name of the file
	
	keyMD5code = MD5Create(ikey);
	if(NULL == keyMD5code)
	{
		DMCLOG_E("ERROR MD5Create failed in %s",__FUNCTION__);
		ret = DMCRYPTORET_UNKNOW_ERROR;
		goto OPENSSL_ENCRYPT_EXIT;

	}
	snprintf(header->key,KEY_MAX_LEN,"%s",keyMD5code);////md5 of key(password)
	free(keyMD5code);
	keyMD5code = NULL;

	header->cryptoType = cryptoType;//crypto Type
	header->length = cryptoLen;

	if (fseek(fpWrite, 0, SEEK_SET) == -1) {//SEEK position
		DMCLOG_E("fseek SEEK_SET error!\n");
		ret = DMCRYPTORET_INVALID_FILE;
		goto OPENSSL_ENCRYPT_EXIT;

	}

	writelen = FILE_DATA_POSITION;
	ret = fwrite(header,1,writelen,fpWrite);
	if(writelen != ret)
	{	
		DMCLOG_E("write() failed! ret=%d",ret);
		perror("write");
		ret = DMCRYPTORET_WRITEFILE_FAILED;
		goto OPENSSL_ENCRYPT_EXIT;
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
OPENSSL_ENCRYPT_EXIT:
	EVP_CIPHER_CTX_cleanup(&ctx);
	if(header)
		free(header);
	return ret;
}

DMCRYPTO_RET Initial_Encrypt_info_openssl(const unsigned char *key,	/*IN*/
									const unsigned char *outfilepath,	/*IN-filepath of output file*/
									int filelen,						/*IN-filelen of origin file*/
									CRYPTO_TYPE cryptoType,				/*IN*/
									int *outfd,							/*OUT-fd of output file*/
									void **ctx)							/*OUT*/
{
	int fdWrite = 0;
	unsigned char crypto_key[EVP_MAX_KEY_LENGTH]={0};
	unsigned char iv[EVP_MAX_IV_LENGTH]={0};
	int keylen = 0;
	int ret = 0;
	char *keyMD5code = NULL;
	CRYPTO_TYPE EncryptoType= AES_ECB_128;
	DMCRYPTO_FILE_HEADER *header = NULL;
	EVP_CIPHER_CTX *ctx_new = NULL;
	const EVP_CIPHER *cipher = NULL;
	unsigned char realKey[KEY_MAX_LEN+16] = {0};
	char *filename = NULL;

	if(NULL == outfilepath || ((NULL != key)&&(strlen(key) > 32))) {
		DMCLOG_E("please set correctly filepath or key too long\n");
		return DMCRYPTORET_INVALID_FILENAME;
	}

#if FIX_CRYPTO_TYPE
	EncryptoType = DEFAULT_CRYPTO_TYPE;
#else
	EncryptoType = cryptoType;
#endif
	switch(EncryptoType)
	{
		case AES_ECB_128:
			cipher = EVP_aes_128_ecb();
			break;
		case AES_ECB_192:
			cipher = EVP_aes_192_ecb();
			break;
		case AES_ECB_256:
			cipher = EVP_aes_256_ecb();
			break;
		case AES_CBC_128:
			cipher = EVP_aes_128_cbc();
			break;
		case AES_CBC_192:
			cipher = EVP_aes_192_cbc();
			break;
		case AES_CBC_256:
			cipher = EVP_aes_256_cbc();
			break;
		default:
			DMCLOG_E("INVALID CRYPTOTYPE!\n");
			return DMCRYPTORET_INVALID_CRYPTOTYPE;
	}

	fdWrite = open(outfilepath,O_RDWR|O_CREAT|O_TRUNC,0744);
	if (fdWrite == -1) {
		perror("open the filepath (file) failed\n");
		return DMCRYPTORET_OPENFILE_FAILED;
	}
	
	/************write file header***********/
	//write file header
	header = (DMCRYPTO_FILE_HEADER *)calloc(1,(FILE_DATA_POSITION));
	if(NULL == header)
	{
		perror("calloc failed!");
		return DMCRYPTORET_UNKNOW_ERROR;
	}
	snprintf(header->encrypt_flag,sizeof(header->encrypt_flag),"%s",ENCRYPT_FILE_FLAG);//encrypt flag
	//printf("header->encrypt_flag=%s %d\n",header->encrypt_flag,sizeof(header->encrypt_flag));
	filename = get_filename(outfilepath);//??? stream fd,so filename unknow
	snprintf(header->filename,FILENAME_MAX_LEN,"%s",filename);

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
		goto ENCRYP_INIT_EXIT;

	}
	snprintf(header->key,KEY_MAX_LEN,"%s",keyMD5code);////md5 of key(password)
	free(keyMD5code);
	keyMD5code = NULL;

#if FIX_CRYPTO_TYPE
	header->cryptoType = DEFAULT_CRYPTO_TYPE;
#else
	header->cryptoType = cryptoType;//crypto Type
#endif
	header->length = filelen;//maybe unknow

	if (lseek(fdWrite, 0, SEEK_SET) == -1) {//SEEK position
		DMCLOG_E("fseek SEEK_SET error!\n");
		ret = DMCRYPTORET_INVALID_FILE;
		goto ENCRYP_INIT_EXIT;

	}

	ret = write(fdWrite,header,FILE_DATA_POSITION);
	if(FILE_DATA_POSITION != ret)
	{	
		DMCLOG_E("write() failed! ret=%d",ret);
		perror("write");
		ret = DMCRYPTORET_WRITEFILE_FAILED;
		goto ENCRYP_INIT_EXIT;
	}
	/***********write file header end**********/
	
	ctx_new = (EVP_CIPHER_CTX *)malloc(sizeof(EVP_CIPHER_CTX));
	if(NULL == ctx_new)
	{
		DMCLOG_E("malloc g_ctx failed\n");
		ret = DMCRYPTORET_DECRYPT_UNINITED;
		goto ENCRYP_INIT_EXIT;
	}
	memset(ctx_new,0x0,sizeof(EVP_CIPHER_CTX));
	
	//generate key and iv
	keylen = strlen(realKey);
	EVP_BytesToKey(cipher,EVP_md5(),NULL,realKey,keylen,1,crypto_key,iv);
	EVP_CIPHER_CTX_init(ctx_new);
	
	ret = EVP_EncryptInit_ex(ctx_new,cipher,NULL,crypto_key,iv);
	if(!ret)
	{
		DMCLOG_E("EVP_DecryptInit_ex() failed");
		EVP_CIPHER_CTX_cleanup(ctx_new);
		ret = DMCRYPTORET_DECRYPT_ERROR;
		goto ENCRYP_INIT_EXIT;
	}
	ret = DMCRYPTORET_SUCCESS;
	
ENCRYP_INIT_EXIT:
	if(header)
		free(header);
	if(ret != DMCRYPTORET_SUCCESS)
	{
		if(ctx_new)
		{
			free(ctx_new);
			ctx_new = NULL;
		}
		if(fdWrite> 0)
			close(fdWrite);
	}
	else
	{
		*ctx = ctx_new;
		*outfd = fdWrite;
	}
	return ret;
}

DMCRYPTO_RET DmEncrypt_file_data_openssl(DMCRYPTO_FD_STRUCT *encryptInfo)
{
	unsigned char input[DATA_MAX_LEN+1] = {0};
	int startBytes = 0;
	int endBytes = 0;
	long needReadBytes = 0;
	int inputl = 0;
	int cryptoLen = 0;
//	long decrypting_len = 0;
	int ret = 0;

	if(NULL == encryptInfo->ctx || (encryptInfo->fd <= 0))
	{
		DMCLOG_E("didn't call function Initial_Decrypt_info?\n");
		return DMCRYPTORET_DECRYPT_UNINITED;
	}
	else if(encryptInfo->size > DATA_MAX_LEN)
	{
		DMCLOG_E("size too big\n");
		return DMCRYPTORET_SIZE_TOOBIG;
	}/**/

#if 0
	startBytes = ((encryptInfo->startBytes+FILE_DATA_POSITION)/16)*16;
	if(encryptInfo->size % 16)
	{
		endBytes=(((encryptInfo->startBytes+FILE_DATA_POSITION)+(encryptInfo->size))/16+1)*16;
		needReadBytes=endBytes-startBytes;
	}
	else
	{
		needReadBytes = encryptInfo->size;
	}//DMCLOG_D("needReadBytes = %d\n",needReadBytes);

	if (lseek(encryptInfo->fd, startBytes, SEEK_SET) == -1) {//SEEK position
		DMCLOG_E("lseek SEEK_SET error!\n");
		return DMCRYPTORET_INVALID_FILE;
	}
#else
	if(encryptInfo->size % 16)
	{
		DMCLOG_E("must size%%16==0\n");
		return DMCRYPTORET_SIZE_TOOBIG;
	}/**/
	needReadBytes = encryptInfo->size;
#endif


	*(encryptInfo->outputl) = 0;
//	decrypting_len = needReadBytes;
	
	//while(decrypting_len > 0)
	{
		//needReadBytes = (decrypting_len > DATA_MAX_LEN)?16:decrypting_len;
		inputl = read(encryptInfo->fd,input,needReadBytes);
		if(inputl < 0)
		{
			perror("read");
			return DMCRYPTORET_READFILE_FAILED;
		}
		
	
		ret = EVP_EncryptUpdate((EVP_CIPHER_CTX *)(encryptInfo->ctx),
						encryptInfo->output/*+(*(decryptInfo->outputl))*/,&cryptoLen,input,inputl);
		if(!ret)
		{
			DMCLOG_E("EVP_EncryptUpdate() failed");
			return DMCRYPTORET_DECRYPT_ERROR;
		}
		*(encryptInfo->outputl) += cryptoLen;
		//decrypting_len -= needReadBytes;
	}
	if(inputl%16 || (inputl != needReadBytes))
	{
		ret = EVP_EncryptFinal_ex((EVP_CIPHER_CTX *)(encryptInfo->ctx),encryptInfo->output+(*(encryptInfo->outputl)),&cryptoLen);
		if(!ret)
		{
			DMCLOG_E("EVP_EncryptFinal_ex() failed");
			return DMCRYPTORET_ENCRYPT_ERROR;
		}
		*(encryptInfo->outputl) += cryptoLen;
	}
	//DMCLOG_D("needReadBytes = %d\n",needReadBytes);

	return DMCRYPTORET_SUCCESS;

}



/************************DECRYPT*****************************/
DMCRYPTO_RET Initial_Decrypt_info_openssl(const unsigned char *filepath,/*IN*/
									const unsigned char *key,/*IN*/
									int *fd,/*OUT*/
									void **ctx)
{
	int fdRead = 0;
	unsigned char crypto_key[EVP_MAX_KEY_LENGTH]={0};
    unsigned char iv[EVP_MAX_IV_LENGTH]={0};
	int keylen = 0;
	int ret = 0;
	char *keyMD5code = NULL;
	DMCRYPTO_FILE_HEADER *header = NULL;
	unsigned char realKey[KEY_MAX_LEN+16] = {0};
	EVP_CIPHER_CTX *ctx_new = NULL;
	const EVP_CIPHER *cipher = NULL;

	if(NULL == filepath || ((NULL != key)&&(strlen(key) > 32))) {
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
		goto DECRYP_INIT_EXIT;
	}

	read(fdRead,header,FILE_DATA_POSITION);
	//check file flag
	if(strcmp(header->encrypt_flag,ENCRYPT_FILE_FLAG))
	{
		DMCLOG_E("cryptoFlag=%s,it should be %s\n",header->encrypt_flag,ENCRYPT_FILE_FLAG);
		ret = DMCRYPTORET_INVALID_INPUTFILE;
		goto DECRYP_INIT_EXIT;
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
		goto DECRYP_INIT_EXIT;

	}
	if(strcmp(keyMD5code,header->key))
	{
		ret = DMCRYPTORET_INVALID_KEY;
		DMCLOG_E("INVALID KEY\n");
		goto DECRYP_INIT_EXIT;
	}	

	ctx_new = (EVP_CIPHER_CTX *)malloc(sizeof(EVP_CIPHER_CTX));
	if(NULL == ctx_new)
	{
		DMCLOG_E("malloc g_ctx failed\n");
		ret = DMCRYPTORET_DECRYPT_UNINITED;
		goto DECRYP_INIT_EXIT;
	}
	memset(ctx_new,0x0,sizeof(EVP_CIPHER_CTX));

#if FIX_CRYPTO_TYPE
	header->cryptoType = DEFAULT_CRYPTO_TYPE;
#endif
	switch(header->cryptoType)
	{
		case AES_ECB_128:
			cipher = EVP_aes_128_ecb();
			break;
		case AES_ECB_192:
			cipher = EVP_aes_192_ecb();
			break;
		case AES_ECB_256:
			cipher = EVP_aes_256_ecb();
			break;
		case AES_CBC_128:
			cipher = EVP_aes_128_cbc();
			break;
		case AES_CBC_192:
			cipher = EVP_aes_192_cbc();
			break;
		case AES_CBC_256:
			cipher = EVP_aes_256_cbc();
			break;
		default:
			DMCLOG_E("INVALID CRYPTOTYPE!\n");
			ret = DMCRYPTORET_INVALID_CRYPTOTYPE;
			goto DECRYP_INIT_EXIT;
	}

	//generate key and iv
	keylen = strlen(realKey);
	EVP_BytesToKey(cipher,EVP_md5(),NULL,realKey,keylen,1,crypto_key,iv);
	EVP_CIPHER_CTX_init(ctx_new);

	ret = EVP_DecryptInit_ex(ctx_new,cipher,NULL,crypto_key,iv);
	if(!ret)
	{
		DMCLOG_E("EVP_DecryptInit_ex() failed");
		EVP_CIPHER_CTX_cleanup(ctx_new);
		ret = DMCRYPTORET_DECRYPT_ERROR;
		goto DECRYP_INIT_EXIT;
	}
	ret = DMCRYPTORET_SUCCESS;

DECRYP_INIT_EXIT:
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
		if(ctx_new)
		{
			free(ctx_new);
			ctx_new = NULL;
		}
		if(fdRead > 0)
			close(fdRead);
	}
	else
	{
		*ctx = ctx_new;
		*fd = fdRead;
	}
	return ret;
}

DMCRYPTO_RET DmDecrypt_file_data_openssl(DMCRYPTO_FD_STRUCT *decryptInfo)
{
	unsigned char input[DATA_MAX_LEN+1] = {0};
	int startBytes = 0;
	int endBytes = 0;
	int needReadBytes = 0;
	int inputl = 0;
	int cryptoLen = 0;
//	int decrypting_len = 0;
	int ret = 0;

	if(NULL == decryptInfo->ctx || (decryptInfo->fd <= 0))
	{
		DMCLOG_E("didn't call function Initial_Decrypt_info?\n");
		return DMCRYPTORET_DECRYPT_UNINITED;
	}
	else if(decryptInfo->size > DATA_MAX_LEN)
	{
		DMCLOG_E("size too big\n");
		return DMCRYPTORET_SIZE_TOOBIG;
	}/**/

	startBytes = ((decryptInfo->startBytes+FILE_DATA_POSITION)/16)*16;
	if(decryptInfo->size % 16)
	{
		endBytes=(((decryptInfo->startBytes+FILE_DATA_POSITION)+(decryptInfo->size))/16+1)*16;
		needReadBytes=endBytes-startBytes;
	}
	else
	{
		needReadBytes = decryptInfo->size;
	}//DMCLOG_D("needReadBytes = %d\n",needReadBytes);

	if (lseek(decryptInfo->fd, startBytes, SEEK_SET) == -1) {//SEEK position
		DMCLOG_E("lseek SEEK_SET error!\n");
		return DMCRYPTORET_INVALID_FILE;
	}

//	decrypting_len = needReadBytes;
	*(decryptInfo->outputl) = 0;
	//while(decrypting_len > 0)
	{
		//needReadBytes = (decrypting_len > 16)?16:decrypting_len;
		inputl = read(decryptInfo->fd,input,needReadBytes);
		if(inputl < 0)
		{
			perror("read");
			return DMCRYPTORET_READFILE_FAILED;
		}
		
	
		ret = EVP_DecryptUpdate((EVP_CIPHER_CTX *)(decryptInfo->ctx),
						decryptInfo->output/*+(*(decryptInfo->outputl))*/,&cryptoLen,input,inputl);
		if(!ret)
		{
			DMCLOG_E("EVP_DecryptUpdate() failed");
			return DMCRYPTORET_DECRYPT_ERROR;
		}
		*(decryptInfo->outputl) += cryptoLen;
		//decrypting_len -= needReadBytes;
	}
	if(inputl%16 || (inputl != needReadBytes))//readlen%16//the EVP_DecryptFinal_ex function maybe failed
	{
		ret = EVP_DecryptFinal_ex((EVP_CIPHER_CTX *)(decryptInfo->ctx),decryptInfo->output+(*(decryptInfo->outputl)),&cryptoLen);
		if(!ret)
		{
			DMCLOG_E("EVP_DecryptFinal_ex() failed");
			return DMCRYPTORET_ENCRYPT_ERROR;
		}
		*(decryptInfo->outputl) += cryptoLen;
	}
	//DMCLOG_D("needReadBytes = %d\n",needReadBytes);

	return DMCRYPTORET_SUCCESS;
}
#endif//if DISABLE_OPENSSL

