#ifndef __DM_CRYPTO_H
#define __DM_CRYPTO_H


#define FILENAME_MAX_LEN 255//must be not bigger than 255
#define KEY_MAX_LEN 63
#define ENCRYPT_FILE_FLAG_LEN 8
#define FILE_DATA_POSITION 512

#define ENABLE_TIMER //speed test
//#define AES_HARDWARE_X1000//use x1000 hardware crypto

#ifdef AES_HARDWARE_X1000
	#define DISABLE_OPENSSL 1
	#define DATA_MAX_LEN	4096//must < 8KB
	#define FIX_CRYPTO_TYPE 1//only use ecb
#else
	#define DISABLE_OPENSSL 0
	#define DATA_MAX_LEN	16384
	#define FIX_CRYPTO_TYPE 0
#endif


#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#if DISABLE_OPENSSL
//TODO
#else
#include <openssl/evp.h>
#endif


#if FIX_CRYPTO_TYPE
#define DEFAULT_CRYPTO_TYPE AES_ECB_128
#endif


typedef enum _DMCRYPTO_RET
	{
		DMCRYPTORET_SUCCESS = 0,			//SUCCESS
		DMCRYPTORET_INVALID_DATA, 			// 1 the input/output DATA is NULL
		DMCRYPTORET_INVALID_INPUTFILE, 		// 2 the input file is not a ENCRYTPOED file
		DMCRYPTORET_INVALID_FILENAME, 		// 3 the  filepath is NULL or filename too long(<FILENAME_MAX_LEN)
		DMCRYPTORET_INVALID_FILE, 			// 4 the  file length is too short...
		DMCRYPTORET_OPENFILE_FAILED, 		// 5 open file failed
		DMCRYPTORET_INVALID_KEY, 			// 6 the password is not correct
		DMCRYPTORET_INVALID_CRYPTOTYPE, 	// 7 UNKNOW CRYPTO_TYPE
		DMCRYPTORET_CANCEL,					// 8 canceled by callback function int(*progress)(int percent)
		DMCRYPTORET_PAUSE,					// 9 it is the ret value of callback function
		DMCRYPTORET_READFILE_FAILED,		// 10 readfile failed
		DMCRYPTORET_WRITEFILE_FAILED,		// 11 writefile failed
		DMCRYPTORET_ENCRYPT_ERROR,			// 12 encrypt failed
		DMCRYPTORET_DECRYPT_ERROR,			// 13 decrypt failed
		DMCRYPTORET_FILESIZE_INCORRECT,		// 14 FILESIZE INCORRECT,maybe edited or broken
		DMCRYPTORET_DECRYPT_UNINITED,		//didn't call function 'Initial_Decrypt_info'?
		DMCRYPTORET_SIZE_TOOBIG,			//size must < DATA_MAX_LEN
		DMCRYPTORET_UNKNOW_ERROR,			// 16 

		DMCRYPTORET_COUNT		
	}DMCRYPTO_RET ;

typedef enum _CRYPTO_TYPE
	{
		AES_CBC_128 = 1,
		AES_CBC_192,
		AES_CBC_256, 
		AES_ECB_128,//---
		AES_ECB_192,
		AES_ECB_256,
		CRYPTO_TYPE_UNKNOW
	}CRYPTO_TYPE;


typedef struct _DMENCRYPT_FILE_STRUCT//use for local file
	{
		unsigned char *inpath;			//(origin) path of the file which want to be encrypted or decrypted
		unsigned char *outpath;			//output path of the file which want to be encrypted or decrypted
		unsigned char *key;				//key of crypto
		CRYPTO_TYPE cryptoType;			//crypto type of file

		//parameter percent's value is the real value of progress(could initialize to be NULL)
		//if progress function return DMCRYPTORET_CANCEL,crypto will be canceled
		//if progress function DMCRYPTORET_PAUSE,crypto will be paused
		DMCRYPTO_RET (*progress)(int percent);	
	}DMENCRYPT_FILE_STRUCT;

typedef struct _DMCRYPTO_FD_STRUCT
	{
		int fd;						//the fd of decrypt file
#ifdef AES_HARDWARE_X1000
//		int hd; 					//handle fd of hardware driver
		unsigned char *key;			//key of decrypt(just for hardware)
		CRYPTO_TYPE cryptoType;		//crypto type of file
#endif
		int startBytes;				//file position to decrypted (startBytes%16 == 0)
		int size;					//size of data to be decrypted(size <= DATA_MAX_LEN &&  (size%16 == 0))
		unsigned char *output;		//stream of output data
		int *outputl;				//length of output
#if DISABLE_OPENSSL
		//TODO
#else
		EVP_CIPHER_CTX *ctx;		//decrypt ctx
#endif
	}DMCRYPTO_FD_STRUCT;

typedef struct _DMCRYPTO_FILE_HEADER
	{
		unsigned char encrypt_flag[ENCRYPT_FILE_FLAG_LEN+1];//encrypt flag
		unsigned char key[KEY_MAX_LEN+1];		//md5 of (password)
		unsigned char filename[FILENAME_MAX_LEN+1];	//origin name of the file
		long length;						//length = fileLength - 520
		char *private;						//private information(reserved)
		CRYPTO_TYPE cryptoType;				//crypto type of file
	}DMCRYPTO_FILE_HEADER;










//encrypt file----just use for local file
DMCRYPTO_RET DmEncrypt_file(DMENCRYPT_FILE_STRUCT *encryptInfo);

#if (0 == DISABLE_OPENSSL)
/****************
***encrypt file
***process:
***(1)'Initial_Encrypt_info'
***(2) loop call 'DmEncrypt_file_data' .......
***(3)'Uninitial_crypto_info'
*/
DMCRYPTO_RET Initial_Encrypt_info(const unsigned char *key,	/*IN*/
									const unsigned char *outfilepath,	/*IN-filepath of output file*/
									int filelen,						/*IN-filepath of origin file*/
									CRYPTO_TYPE cryptoType,				/*IN*/
									int *outfd,							/*OUT-fd of output file*/
									void **ctx);						/*OUT*/
DMCRYPTO_RET DmEncrypt_file_data(DMCRYPTO_FD_STRUCT *encryptInfo);
#endif//#if (0 == DISABLE_OPENSSL)

/****************
***decrypt file
***process:
***(1)'Initial_Decrypt_info'
***(2) loop call 'Initial_Decrypt_info' .......
***(3)'Uninitial_crypto_info'
*/
DMCRYPTO_RET Initial_Decrypt_info(const unsigned char *filepath,	/*IN-filepath of decrypted file*/
									const unsigned char *key,		/*IN*/
									int *fd,						/*OUT-fd of decrypted file*/
									void **ctx,						/*OUT*/
									CRYPTO_TYPE *cryptoType);		/*OUT*/
DMCRYPTO_RET DmDecrypt_file_data(DMCRYPTO_FD_STRUCT *decryptInfo);
DMCRYPTO_RET Uninitial_crypto_info(DMCRYPTO_FD_STRUCT *decryptInfo);


/****************
***check the file if a crypto file,file information store in 'fileHeader'
***yes:return TRUE;no:return FALSE
*****************/
int is_DmCrypto_file(const unsigned char *filepath,DMCRYPTO_FILE_HEADER **fileHeader);


#endif

