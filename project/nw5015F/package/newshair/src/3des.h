#ifndef _DES_H_
#define _DES_H_

unsigned char un3DES(unsigned char* indata,unsigned int inlen,unsigned char* key_1,unsigned char* key_2,
					 unsigned char* key_3,unsigned char* outdata,unsigned int* outlen);

unsigned char en3DES(unsigned char* indata,unsigned int inlen,unsigned char* key_1,unsigned char* key_2,
					 unsigned char* key_3,unsigned char* outdata,unsigned int* outlen);

unsigned char enDES(unsigned char* indata,unsigned int inlen,unsigned char* key,
					unsigned char* outdata,unsigned int* outlen);
unsigned char unDES(unsigned char* indata,unsigned int inlen,unsigned char* key,
					unsigned char* outdata,unsigned int* outlen);
char *getSerialNumber(char *snnumber);
#endif
