/*
 * =============================================================================
 *    Filename:  main.c
 *    Description:  main function of dm_crypto
 *
 *   Version:  1.0
 *   Created:  2017/01/11 10:11
 *   Revision:  none
 *   Compiler:  gcc
 *
 *   Author:  hu.jiang<jh5254622@126.com> 18520833995
 *   Organization: longsys/DAMAI
 *
 * =============================================================================
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
 
#include "dm_crypto.h"

static void usage_print(int type)
{
	printf("please intput correctly arguments! type=%d\n",type);
	printf("ENCRYPT:\ndm_crypto --key 12345678 --type ecb128 --inpath ./aaa.mp3 --outpath ./aaa.mp3.ecb128.enc\n");
	printf("DECRYPT:\ndm_crypto --key 12345678 --inpath ./aaa.mp3.ecb128.enc\n");
	printf("\n--key       [password]\n");
	printf("--type      [ecb128 ecb192 ecb256 cbc128 cbc192 cbc256]\n");
	printf("--inpath    [file to be encrypted or decrypted]\n");
	printf("--outpath   [output file's path]\n");
}

static void Print_dmcrypto_file_header(DMCRYPTO_FILE_HEADER *fileheader)
{
	printf("encrypt_flag=%s\n",fileheader->encrypt_flag);//encrypt flag
	printf("key=%s\n",fileheader->key);//md5 of (password)
	printf("filename=%s\n",fileheader->filename);//origin name of the file
	printf("cryptoType=%d\n",fileheader->cryptoType);//cryptoType
	printf("length=%ld\n",fileheader->length);//file length

	if(fileheader->private)//reserve
		printf("private=%s\n",fileheader->private);
}

DMCRYPTO_RET show_progress(int percent)
{
    printf("current percent is:%d %%\n",percent);
	return DMCRYPTORET_SUCCESS;
	//return DMCRYPTORET_CANCEL;
	//return DMCRYPTORET_PAUSE;
}

#define MULTI_ENCRYPT_THREAD 0
#if 0//MULTI_ENCRYPT_THREAD
#include <pthread.h>

unsigned char *inputpath = NULL;
unsigned char *outputpath = NULL;
unsigned char *inputkey = NULL;
unsigned char *inputtype = NULL;
CRYPTO_TYPE incryptoType = AES_ECB_128;

int EncryptoThread2(void)
{
	unsigned char myinput[256]={0};
	unsigned char myoutput[256]={0};
	int i = 0;
	DMCRYPTO_RET ret = DMCRYPTORET_UNKNOW_ERROR;
	DMENCRYPT_FILE_STRUCT *encryptInfo = NULL;

	snprintf(myinput,sizeof(myinput),"%saaa",inputpath);
	snprintf(myoutput,sizeof(myoutput),"%saaa",outputpath);
	printf("myinput=%s\nmyoutput=%s\n",myinput,myoutput);
	encryptInfo = (DMENCRYPT_STRUCT *)calloc(1,sizeof(DMENCRYPT_FILE_STRUCT));
	if(NULL == encryptInfo)
	{
		printf("calloc encryptInfo failed\n");
		return -1;
	}

	//set value
	encryptInfo->inpath = myinput;
	encryptInfo->outpath= myoutput;
	encryptInfo->key = inputkey;
#if FIX_CRYPTO_TYPE
	encryptInfo->cryptoType = DEFAULT_CRYPTO_TYPE;
#else
	encryptInfo->cryptoType = incryptoType;
#endif
	//encryptInfo->progress= show_progress;
	ret = DmEncrypt_file(encryptInfo);
	free(encryptInfo);
	encryptInfo = NULL;
	return ret;
}
int DecryptoThread2(void)
{
	unsigned char filepath[256] = {0};
	unsigned char *key = inputkey;
	int fd = 0;
	int outputl = 0;
	DMCRYPTO_FD_STRUCT decryptInfo;
	void *ctx = NULL;
	int ret = 0;
#ifdef ENABLE_TIMER
	clock_t start,end;
	double TheTimes;
	double temp = 0;
	start = clock();
#endif

	snprintf(filepath,sizeof(filepath),"%saaa",inputpath);
#ifdef AES_HARDWARE_X1000
	CRYPTO_TYPE decryptoType = AES_ECB_128;
	ret = Initial_Decrypt_info(filepath,key,&fd,NULL,&decryptoType);
#else
	ret = Initial_Decrypt_info(filepath,key,&fd,&ctx,NULL);
#endif
	if(DMCRYPTORET_SUCCESS == ret)
	{
		memset(&decryptInfo,0x0,sizeof(DMCRYPTO_FD_STRUCT));
#if 1
		unsigned char outfilepath[512] = {0};
		int fdout = 0;
		long decryptLen = 0;
		long fileLen = 0;
	
		snprintf(outfilepath,sizeof(outfilepath),"%s.dec",filepath);
		fdout = open(outfilepath,O_RDWR|O_CREAT|O_TRUNC,0744);
		if (fdout == -1)
		{
			perror("open the output (file) failed\n");
			return -1;
		}
				
		decryptInfo.fd = fd;
		decryptInfo.startBytes = 0;
		decryptInfo.size = DATA_MAX_LEN;
		decryptInfo.output = (unsigned char*)malloc(DATA_MAX_LEN+1024);//must larger than decryptInfo.size
		if(NULL == decryptInfo.output)
		{
			perror("malloc");
			return -1;
		}
		decryptInfo.outputl = &outputl;
#ifdef AES_HARDWARE_X1000
		decryptInfo.cryptoType = decryptoType;
		decryptInfo.key = key;
#else
		decryptInfo.ctx = ctx;
#endif
				
		fileLen = lseek(decryptInfo.fd, 0, SEEK_END);
		lseek(decryptInfo.fd, 0, SEEK_SET);//seek to start;
		fileLen -= FILE_DATA_POSITION;//file header length
#ifdef ENABLE_TIMER
		clock_t start,end;
		double TheTimes;
		double temp = 0;
		start = clock();
#endif
		while(decryptLen < fileLen)
		{
			ret = DmDecrypt_file_data(&decryptInfo);
			if(DMCRYPTORET_SUCCESS == ret)//save to file;
			{
				write(fdout,decryptInfo.output,*(decryptInfo.outputl));
				decryptLen += decryptInfo.size;
				decryptInfo.startBytes = decryptLen;
				//printf("decryptLen = %ld *(decryptInfo.outputl)=%d\n",decryptLen,*(decryptInfo.outputl));
			}
			else
			{
				printf("DmDecrypt_file_data failed:ret=%d\n",ret);
				break;
			}
		}
#ifdef ENABLE_TIMER
		printf("#########DECRYPT SPEED########## \n");
		end = clock();
		TheTimes = (double)(end - start) / CLOCKS_PER_SEC;
		printf("encrypt %f s.\n",TheTimes);
				
		TheTimes = (double)fileLen/TheTimes;
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
#endif
		close(fdout);
		free(decryptInfo.output);
		Uninitial_crypto_info(&decryptInfo);
#endif//#if 0
	}
	return 0;

}
#endif//MULTI_ENCRYPT_THREAD

/*================ENCRYPT TEST1====================*/	
//use 'DmEncrypt_file'----just for local file
int example_encrypt_file(unsigned char *OrgFilePath,unsigned char *OutFilePath,
								unsigned char *EncKey,CRYPTO_TYPE EncryptType)
{
	DMENCRYPT_FILE_STRUCT *encryptInfo = NULL;
	int ret = -1;
#if 0
	pthread_t nextenc_thread;
	memset(&nextenc_thread,0x0,sizeof(pthread_t));
	if((pthread_create(&nextenc_thread, NULL, EncryptoThread2, NULL)) != 0)  
	{
		printf("create the nextenc_thread failed!\n");	 
	}
	else 
	{
		printf("create the nextenc_thread success!\n");
	}
#endif
	encryptInfo = (DMENCRYPT_FILE_STRUCT *)calloc(1,sizeof(DMENCRYPT_FILE_STRUCT));
	if(NULL == encryptInfo)
	{
		printf("calloc encryptInfo failed\n");
		return -1;
	}

	//set value
	encryptInfo->inpath = OrgFilePath;
	encryptInfo->outpath= OutFilePath;
	encryptInfo->key = EncKey;
#if FIX_CRYPTO_TYPE
	encryptInfo->cryptoType = DEFAULT_CRYPTO_TYPE;
#else
	encryptInfo->cryptoType = EncryptType;
#endif
	//encryptInfo->progress= show_progress;
	ret = DmEncrypt_file(encryptInfo);
	free(encryptInfo);
	encryptInfo = NULL;
#if 0
	pthread_join(nextenc_thread,NULL);
#endif
	return 0;
	/*...................................CHECK ENCRYPTED FILE....................................
	DMCRYPTO_FILE_HEADER *fileHeader = NULL;
	ret = is_DmCrypto_file(OutFilePath,&fileHeader);
	if(TRUE == ret)
		Print_dmcrypto_file_header(fileHeader);
	if(fileHeader)
	{
		free(fileHeader);
		fileHeader = NULL;
	}*/
	/*...................................CHECK ENCRYPTED FILE END...............................*/
}



#if (0 == DISABLE_OPENSSL)
/*================ENCRYPT TEST2====================*/	
int example_encrypt_data(unsigned char *OrgFilePath,unsigned char *OutFilePath,
								unsigned char *EncKey,CRYPTO_TYPE EncryptType)
{
//	unsigned char *filepath = OrgFilePath;
//	unsigned char *key = EncKey;
	long org_filelen = 0;
	int fdin = 0;
	int fdout = 0;
	long encryptedLen = 0;
	int outputl = 0;
	DMCRYPTO_FD_STRUCT encryptInfo;
	void *ctx = NULL;
	int ret = -1;

	fdout = open(OutFilePath,O_RDWR|O_CREAT|O_TRUNC,0744);
	if (fdout < 0)
	{
		perror("open the output (file) failed\n");
		return -1;
	}

	fdin = open(OrgFilePath,O_RDONLY);
	if(fdin < 0)
	{
		perror("open the input (file) failed");
		ret = -1;
		goto exit_encrypt_data;
	}
	org_filelen = lseek(fdin, 0, SEEK_END);
	lseek(fdin, 0, SEEK_SET);//seek to start;
	
	ret = Initial_Encrypt_info(EncKey,OutFilePath,org_filelen,EncryptType,&fdout,&ctx);
	if(DMCRYPTORET_SUCCESS == ret)
	{
		memset(&encryptInfo,0x0,sizeof(DMCRYPTO_FD_STRUCT));

		encryptInfo.fd = fdin;
		encryptInfo.startBytes = 0;
		encryptInfo.size = DATA_MAX_LEN;//must size%16==0
		encryptInfo.output = (unsigned char*)malloc(DATA_MAX_LEN+1024);//must larger than decryptInfo.size
		encryptInfo.ctx = ctx;
		if(NULL == encryptInfo.output)
		{
			perror("malloc");
			ret = -1;
			goto exit_encrypt_data;
		}
		encryptInfo.outputl = &outputl;
#ifdef ENABLE_TIMER
		clock_t start,end;
		double TheTimes;
		double temp = 0;
		start = clock();
#endif
		while(encryptedLen < org_filelen)
		{
			ret = DmEncrypt_file_data(&encryptInfo);
			if(DMCRYPTORET_SUCCESS == ret)//save to file;
			{
				write(fdout,encryptInfo.output,*(encryptInfo.outputl));
				#if 0//it's ok but too slow
					//decryptLen += decryptInfo.size%16?((decryptInfo.size/16+1)*16):decryptInfo.size;
				#else
					encryptedLen += encryptInfo.size;
				#endif
				encryptInfo.startBytes = encryptedLen;
				//printf("decryptLen = %ld *(decryptInfo.outputl)=%d\n",decryptLen,*(decryptInfo.outputl));
			}
			else
			{
				printf("DmDecrypt_file_data failed:ret=%d\n",ret);
				break;
				ret = -1;
			}
		}
#ifdef ENABLE_TIMER
		printf("#########DECRYPT SPEED########## \n");
		end = clock();
		TheTimes = (double)(end - start) / CLOCKS_PER_SEC;
		printf("encrypt %f s.\n",TheTimes);
		
		TheTimes = (double)org_filelen/TheTimes;
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
#endif//ENABLE_TIMER
		ret = 0;
	}
	else
	{
		if(fdin > 0)
			close(fdin);
	}
exit_encrypt_data:
	if(encryptInfo.output)
		free(encryptInfo.output);
	Uninitial_crypto_info(&encryptInfo);//will close fdin
	if(fdout > 0)
		close(fdout);
	return ret;
}
#endif


/*================DECRYPT TEST====================*/	
int example_decrypt_data(unsigned char *org_filepath,	unsigned char *inputkey)
{
	unsigned char *filepath = org_filepath;
	unsigned char *key = inputkey;
	int fdin = 0;
	int outputl = 0;
	DMCRYPTO_FD_STRUCT decryptInfo;
	void *ctx = NULL;
	int ret = -1;
	unsigned char outfilepath[512] = {0};
	int fdout = 0;
	long decryptLen = 0;
	long fileLen = 0;

	//return cbc_decrypt(inputpath,inputpath,inputkey);
#if 0
	pthread_t nextdec_thread;
	memset(&nextdec_thread,0x0,sizeof(pthread_t));
	if((pthread_create(&nextdec_thread, NULL, DecryptoThread2, NULL)) != 0)  
	{
		printf("create the nextdec_thread failed!\n");	 
	}
	else 
	{
		printf("create the nextdec_thread success!\n");
	}
#endif
	
#ifdef AES_HARDWARE_X1000
	CRYPTO_TYPE decryptoType = AES_ECB_128;
	ret = Initial_Decrypt_info(filepath,key,&fdin,NULL,&decryptoType);
#else
	ret = Initial_Decrypt_info(filepath,key,&fdin,&ctx,NULL);
#endif
	if(DMCRYPTORET_SUCCESS == ret)
	{
		memset(&decryptInfo,0x0,sizeof(DMCRYPTO_FD_STRUCT));
#if 0//=======test stream data==============
		decryptInfo.fd = fdin;
		decryptInfo.startBytes = 0;
		decryptInfo.size = 100;
		decryptInfo.output = (unsigned char*)malloc(128);//must larger than decryptInfo.size
		if(NULL == decryptInfo.output)
		{
			perror("malloc");
			return -1;
		}
		decryptInfo.outputl = &outputl;
		decryptInfo.ctx = ctx;
		ret = DmDecrypt_file_data(&decryptInfo);
		if(DMCRYPTORET_SUCCESS == ret)
		{
			for(i = 0; i < *(decryptInfo.outputl);i++)//just for test
			{
				printf("%2x ",*(decryptInfo.output+i));
				if((i+1) % 16 == 0)
					printf("\n");
			}
			printf("outputl=%d\n",*(decryptInfo.outputl));
		}
		free(decryptInfo.output);
		Uninitial_crypto_info(&decryptInfo);
#else//==========the decrypted data store in file===========
		snprintf(outfilepath,sizeof(outfilepath),"%s.dec",org_filepath);
		fdout = open(outfilepath,O_RDWR|O_CREAT|O_TRUNC,0744);
		if (fdout == -1)
		{
			perror("open the output (file) failed\n");
			ret = -1;
			goto exit_decrypt_data;
		}
				
		decryptInfo.fd = fdin;
		decryptInfo.startBytes = 0;
		decryptInfo.size = DATA_MAX_LEN;
		decryptInfo.output = (unsigned char*)malloc(DATA_MAX_LEN+1024);//must larger than decryptInfo.size
		if(NULL == decryptInfo.output)
		{
			perror("malloc");
			ret = -1;
			goto exit_decrypt_data;
		}
		decryptInfo.outputl = &outputl;
#ifdef AES_HARDWARE_X1000
		decryptInfo.cryptoType = decryptoType;
		decryptInfo.key = key;
#else
		decryptInfo.ctx = ctx;
#endif
		fileLen = lseek(decryptInfo.fd, 0, SEEK_END);
		lseek(decryptInfo.fd, 0, SEEK_SET);//seek to start;
		fileLen -= FILE_DATA_POSITION;//file header length
#ifdef ENABLE_TIMER
		clock_t start,end;
		double TheTimes;
		double temp = 0;
		start = clock();
#endif
		while(decryptLen < fileLen)
		{
			ret = DmDecrypt_file_data(&decryptInfo);
			if(DMCRYPTORET_SUCCESS == ret)//save to file;
			{
				write(fdout,decryptInfo.output,*(decryptInfo.outputl));
				#if 0//it's ok but too slow
				//decryptLen += decryptInfo.size%16?((decryptInfo.size/16+1)*16):decryptInfo.size;
				#else
				decryptLen += decryptInfo.size;
				#endif
				decryptInfo.startBytes = decryptLen;
				//printf("decryptLen = %ld *(decryptInfo.outputl)=%d\n",decryptLen,*(decryptInfo.outputl));
			}
			else
			{
				printf("DmDecrypt_file_data failed:ret=%d\n",ret);
				ret = -1;
				break;
			}
		}
#ifdef ENABLE_TIMER
		printf("#########DECRYPT SPEED########## \n");
		end = clock();
		TheTimes = (double)(end - start) / CLOCKS_PER_SEC;
		printf("decrypt %f s.\n",TheTimes);
		
		TheTimes = (double)fileLen/TheTimes;
		temp = TheTimes/1024;
		if(0 == temp)
			printf("decrypt speed:%f byte/S.\n",TheTimes);
		else
		{
			temp = TheTimes/1024/1024;
			if(0 == temp)
			{
				TheTimes = TheTimes/1024;
				printf("decrypt speed:%f KB/S.\n",TheTimes);
			}
			else
			{	 
				TheTimes = TheTimes/1024/1024;
				printf("decrypt speed:%f MB/S.\n",TheTimes);
			}
		}
#endif//ENABLE_TIMER
#endif//#if 0
		ret = 0;
	}
#if 0
	pthread_join(nextdec_thread,NULL);
#endif

exit_decrypt_data:
	if(decryptInfo.output)
		free(decryptInfo.output);
	Uninitial_crypto_info(&decryptInfo);//will close fdin
	if(fdout > 0)
		close(fdout);

	return ret;

}


int main(int argc, char** argv)
{  
	/**********input value**********/
	unsigned char *inputpath = NULL;
	unsigned char *outputpath = NULL;
	unsigned char *inputkey = NULL;
	unsigned char *inputtype = NULL;
	CRYPTO_TYPE incryptoType = AES_ECB_128;

	int i = 0;
	DMCRYPTO_RET ret = DMCRYPTORET_UNKNOW_ERROR;

	if(argc != 9 && argc != 5 ) 
	{
		usage_print(argc);
		return -1;	
	}

    for(i = 1; i < argc;)
    {
    	//DMCLOG_D("argv[%d]=%s\n",i,argv[i]);
        if(!strcmp("--inpath", argv[i]))
        {
            inputpath= argv[i+1];
            i+=2;
			printf("inputpath=%s\n",inputpath);
        }
		else if(!strcmp("--outpath", argv[i]))
        {
            outputpath= argv[i+1];
            i+=2;
			printf("outputpath=%s\n",outputpath);
        }
        else if(!strcmp("--key", argv[i]))
        {
        	inputkey = argv[i+1];
	        i+=2;
        }
        else if(!strcmp("--type", argv[i]))
        {
            inputtype = argv[i+1];
            i+=2;
			if(!strcmp(inputtype,"ecb128"))
				incryptoType = AES_ECB_128;
			else if(!strcmp(inputtype,"ecb192"))
				incryptoType = AES_ECB_192;
			else if(!strcmp(inputtype,"ecb256"))
				incryptoType = AES_ECB_256;
			else if(!strcmp(inputtype,"cbc128"))
				incryptoType = AES_CBC_128;
			else if(!strcmp(inputtype,"cbc192"))
				incryptoType = AES_CBC_192;
			else if(!strcmp(inputtype,"cbc256"))
				incryptoType = AES_CBC_256;
			else
			{
				printf("unknow crypto type:%s",inputtype);
				return -1;
			}
        }
       	else
			i++;
    }
	if((argc == 9) && ((NULL == inputkey) || (NULL == inputpath)||
		(NULL == inputtype) || (NULL == outputpath)))
	{
		usage_print(argc);
		return -1;	
	}
	else if((argc == 5) && ((NULL == inputkey) || (NULL == inputpath)))
	{
		usage_print(argc);
		return -1;	
	}

	if(9 == argc)//ENCRYPT
	{
		#if 0
		example_encrypt_file(inputpath,outputpath,inputkey,incryptoType);
		#else//or
		example_encrypt_data(inputpath,outputpath,inputkey,incryptoType);
		#endif

	}
	else//DECRYPT
	{
		example_decrypt_data(inputpath,inputkey);
	}

	return ret;
}  

