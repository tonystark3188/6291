/*
 * =============================================================================
 *    Filename:  dm_crypto_utils.c
 *    Description:  dm_crypto_utils
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

void delete_crytpo_file(const unsigned char *filepath)
{
	if( remove(filepath) == 0 )
		DMCLOG_D("Removed %s.", filepath);
	else
		perror("remove");
}

char *get_dir_path(unsigned char *filepath)//warn: this function will broken filepath
{
	if(NULL == filepath)
		return NULL;
	else
	{
		int len = strlen(filepath);
		while(len)
		{
			if('/' == *(filepath + len))
			{
				*(filepath + len + 1) = '\0'; 
				break;
			}
			len--;
		}
		return filepath;
	}
}


char *get_filename(unsigned char *filepath)
{
	if(NULL == filepath)
		return NULL;
	else
	{
		int len = strlen(filepath);
		while(len)
		{
			if('/' == *(filepath + len))
				return (filepath + len + 1); 
			len--;
		}
		return filepath;
	}
}


/*********************************ENCRYPT**************************************/

/*******************************************************************************
** Name: DmEncrypt_file
** Input:DMENCRYPT_FILE_STRUCT cryptoInfo
** Return: int
** Owner:hu.jiang
** Date: 2017.1.10
** Time: 10:34:41
*******************************************************************************/
DMCRYPTO_RET DmEncrypt_file(DMENCRYPT_FILE_STRUCT *encryptoInfo)
{
	FILE *fpRead = NULL,*fpWrite = NULL;
	unsigned char *filename = NULL;
#ifdef AES_HARDWARE_X1000
	int hd = 0;
#endif

#ifdef ENABLE_TIMER
	clock_t start,end;
	double TheTimes;
	double temp = 0;
#endif
	DMCRYPTO_RET ret = DMCRYPTORET_COUNT;
	unsigned char generateKey[KEY_MAX_LEN+16] = {0};
	
	if((NULL == encryptoInfo->inpath) || (NULL == encryptoInfo->outpath) ||
		(strlen(get_filename(encryptoInfo->inpath)) > FILENAME_MAX_LEN) ||
		(strlen(get_filename(encryptoInfo->outpath)) > FILENAME_MAX_LEN)||
		((NULL != encryptoInfo->key)&&(strlen(encryptoInfo->key) > 32))) {
		DMCLOG_E("please set correctly inpath/outpath or key too long\n");
		return DMCRYPTORET_INVALID_FILENAME;
	}
	if(NULL == encryptoInfo->key)
	{	snprintf(generateKey,sizeof(generateKey),"%s%s",CRYPTO_KEY_FLAG,DEFAULT_KEY);
	}
	else
		snprintf(generateKey,sizeof(generateKey),"%s%s",CRYPTO_KEY_FLAG,encryptoInfo->key);
	
	fpRead = fopen(encryptoInfo->inpath, "r");
	if (!fpRead) {
		DMCLOG_E("fopen %s failed!\n",encryptoInfo->inpath);
		return DMCRYPTORET_OPENFILE_FAILED;
	}
	
	fpWrite = fopen(encryptoInfo->outpath, "w+");
	if (!fpWrite) {
		DMCLOG_E("fopen %s failed!\n",encryptoInfo->outpath);
		return DMCRYPTORET_OPENFILE_FAILED;
	}

#if FIX_CRYPTO_TYPE
	encryptoInfo->cryptoType = DEFAULT_CRYPTO_TYPE;
#endif
	
	filename = get_filename(encryptoInfo->inpath);
#if DISABLE_OPENSSL//ifdef AES_HARDWARE_X1000
	ret = encrypt_fp_hardware(fpRead, fpWrite,generateKey,encryptoInfo->cryptoType,filename,encryptoInfo->progress);
#else
	ret = encrypt_fp_openssl(fpRead, fpWrite,generateKey,encryptoInfo->cryptoType,filename,encryptoInfo->progress);
#endif//DISABLE_OPENSSL
	
	if(fpRead)
		fclose(fpRead);
	if(fpWrite)
		fclose(fpWrite);
	if(ret)
	{
		DMCLOG_D("cbc_encrypt failed:ret=%d\n",ret);
		delete_crytpo_file(encryptoInfo->outpath);//when failed,delete the output file
	}
	return ret;
}

#if (0 == DISABLE_OPENSSL)
DMCRYPTO_RET Initial_Encrypt_info(const unsigned char *key,			/*IN*/
									const unsigned char *outfilepath,	/*IN-filepath of output file*/
									int filelen,						/*IN-filepath of origin file*/
									CRYPTO_TYPE cryptoType,				/*IN*/
									int *outfd,							/*OUT-fd of output file*/
									void **ctx)							/*OUT*/
{
	return Initial_Encrypt_info_openssl(key,outfilepath,filelen,cryptoType,outfd,ctx);
}

DMCRYPTO_RET DmEncrypt_file_data(DMCRYPTO_FD_STRUCT *encryptInfo)
{
	return DmEncrypt_file_data_openssl(encryptInfo);
}
#endif//#if (0 == DISABLE_OPENSSL)

/************************DECRYPT*****************************/
DMCRYPTO_RET Initial_Decrypt_info(const unsigned char *filepath,	/*IN*/
									const unsigned char *key,		/*IN*/
									int *fd,						/*OUT*/
									void **ctx,						/*OUT*/
									CRYPTO_TYPE *cryptoType)		/*OUT*/
{
	DMCRYPTO_RET ret = DMCRYPTORET_COUNT;

#ifdef AES_HARDWARE_X1000
	ret = Initial_Decrypt_info_hardware(filepath,key,fd,cryptoType);
#else
	ret = Initial_Decrypt_info_openssl(filepath,key,fd,ctx);
#endif
	return ret;
}

DMCRYPTO_RET DmDecrypt_file_data(DMCRYPTO_FD_STRUCT *decryptInfo)
{
	DMCRYPTO_RET ret = DMCRYPTORET_COUNT;

#ifdef AES_HARDWARE_X1000
	ret = DmDecrypt_file_data_hardware(decryptInfo);
#else
	ret = DmDecrypt_file_data_openssl(decryptInfo);
#endif
	return ret;
}

DMCRYPTO_RET Uninitial_crypto_info(DMCRYPTO_FD_STRUCT *cryptoInfo)
{
#if DISABLE_OPENSSL
	//
#else
	if(cryptoInfo->ctx)
	{
		EVP_CIPHER_CTX_cleanup((EVP_CIPHER_CTX *)(cryptoInfo->ctx));
		free(cryptoInfo->ctx);
		cryptoInfo->ctx = NULL;
	}
#endif

	if((cryptoInfo->fd) > 0)
	{
		close(cryptoInfo->fd);
		cryptoInfo->fd = 0;
	}
}

int is_DmCrypto_file(const unsigned char *filepath,DMCRYPTO_FILE_HEADER **fileHeader)
{
	FILE *fpRead = NULL;
	long fsize = 0;

	DMCRYPTO_FILE_HEADER *header = NULL;

	fpRead = fopen(filepath, "r");
	if (!fpRead) {
		DMCLOG_E("fopen %s failed!\n",filepath);
		return FALSE;
	}

	//get file header
	header = (DMCRYPTO_FILE_HEADER *)calloc(1,FILE_DATA_POSITION);//sizeof(DMCRYPTO_FILE_HEADER)
	if(NULL == header)
	{
		perror("calloc failed!");
		fclose(fpRead);
		return FALSE;
	}
	fread(header,FILE_DATA_POSITION,1,fpRead);
	//check file flag
	if(strcmp(header->encrypt_flag,ENCRYPT_FILE_FLAG))
	{
		DMCLOG_E("cryptoFlag=%s,it should be %s\n",header->encrypt_flag,ENCRYPT_FILE_FLAG);
		fclose(fpRead);
		return FALSE;
	}

	//get & check file size
	/*fseek(fpRead, 0, SEEK_END);
	fsize = ftell(fpRead);
	fseek(fpRead, 0, SEEK_SET);
	if(header->length != (fsize - FILE_DATA_POSITION))
	{
        DMCLOG_E("DMCRYPTORET_FILESIZE_INCORRECT\nheader->length=%ld,fsize=%ld\n",
			header->length,fsize);
		free(header);
		fclose(fpRead);
		return FALSE;
	}*/
	
	*fileHeader = header;
	fclose(fpRead);
	return TRUE;
}

