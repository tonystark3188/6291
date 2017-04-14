#ifndef __DM_CRYPTO_INTERNAL_H
#define __DM_CRYPTO_INTERNAL_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "my_debug.h"
#include "dm_crypto.h"

#define ENCRYPT_FILE_FLAG "DMCRYPTO"
#define DEFAULT_KEY "damai888"
#define CRYPTO_KEY_FLAG "jh5254622"


/*
********==========CRYPTOED_FILE=============********
		| FILE_FLAG | FILE_HEADER |        FILE DATA      |
		|__________|____________|_________________|
		|    8Byte    |     512Byte     |ORIGINAL FILE SIZE|
********==========CRYPTOED_FILE=============********
*/

typedef struct _DMCRYPTO_DATA_STRUCT
	{
		unsigned char *input;		//stream of input data
		int inputl;					//length of input
		unsigned char *output;		//stream of output data
		int *outputl;				//length of output
		unsigned char *key;			//key of crypto
		CRYPTO_TYPE cryptoType;		//
	}DMCRYPTO_INTERNAL_STRUCT;



void delete_crytpo_file(const unsigned char *filepath);
char *get_dir_path(unsigned char *filepath);//warn: this function will broken filepath
char *get_filename(unsigned char *filepath);

#if DISABLE_OPENSSL
//TODO
#else
DMCRYPTO_RET encrypt_fp_openssl(FILE *fpRead, FILE *fpWrite,unsigned char *ikey,
	CRYPTO_TYPE cryptoType,unsigned char *filename,DMCRYPTO_RET (*progress)(int percent));

DMCRYPTO_RET Initial_Encrypt_info_openssl(const unsigned char *key,	/*IN*/
									const unsigned char *outfilepath,	/*IN-filepath of output file*/
									int filelen,						/*IN-filelen of origin file*/
									CRYPTO_TYPE cryptoType,				/*IN*/
									int *outfd,							/*OUT-fd of output file*/
									void **ctx);
DMCRYPTO_RET DmEncrypt_file_data_openssl(DMCRYPTO_FD_STRUCT *encryptInfo);


DMCRYPTO_RET Initial_Decrypt_info_openssl(const unsigned char *filepath,/*IN*/
									const unsigned char *key,/*IN*/
									int *fd,/*OUT*/
									void **ctx);/*OUT*/
DMCRYPTO_RET DmDecrypt_file_data_openssl(DMCRYPTO_FD_STRUCT *decryptInfo);
#endif

#ifdef AES_HARDWARE_X1000
int get_hardware_hd(void);

DMCRYPTO_RET encrypt_fp_hardware(FILE *fpRead, FILE *fpWrite,unsigned char *ikey,
	CRYPTO_TYPE cryptoType,unsigned char *filename,DMCRYPTO_RET (*progress)(int percent));

DMCRYPTO_RET Initial_Decrypt_info_hardware(const unsigned char *filepath,/*IN*/
									const unsigned char *key,/*IN*/
									int *fd,/*OUT*/
									CRYPTO_TYPE *cryptoType);/*OUT*/

DMCRYPTO_RET DmDecrypt_file_data_hardware(DMCRYPTO_FD_STRUCT *decryptInfo);

#endif

#endif//#ifndef __DM_CRYPTO_INTERNAL_H

