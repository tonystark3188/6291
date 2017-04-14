/**
 * @file 3des.c
 * @jh DES and 3DES algorithm 
 *		The user can change Cyclic displacement table,PC-1 table,PC-2 table,IP table,IP-1 table,
 *		Extend table,P changable table,S box etc...
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "3des.h"

//#define FAIL			1
//#define SUCCESS			0

#define g_ENCRYPT_FLAG 1
#define g_DECRYPT_FLAG 2


static void xTran(unsigned char *q,unsigned char *p,unsigned char *xTab,int xLen);
static void genKey(unsigned char *key,unsigned char nkey[16][8]);
static void sReplace(unsigned char *right_s);

static unsigned char comDES(unsigned char in[8],unsigned char out[8],
					 unsigned char subkey[16][8],unsigned char flg);

static void randKey(unsigned char key[8]);

static char* base64_encode(const char* data, int data_len); 
static char *base64_decode(const char* data, int data_len); 
static char find_pos(char ch); 

const unsigned char movebit[16]=
{//Cyclic displacement table
    1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1
};

const unsigned char pc_1[56]=
{//PC-1 table
    57,49,41,33,25,17,9,
    1,58,50,42,34,26,18,
    10,2,59,51,43,35,27,
    19,11,3,60,52,44,36,
    63,55,47,39,31,23,15,
    7,62,54,46,38,30,22,
    14,6,61,53,45,37,29,
    21,13,5,28,20,12,4
};

const unsigned char pc_2[48]=
{//PC-2 table
    14,17,11,24,1,5,
    3,28,15,6,21,10,
    23,19,12,4,26,8,
    16,7,27,20,13,2,
    41,52,31,37,47,55,
    30,40,51,45,33,48,
    44,49,39,56,34,53,
    46,42,50,36,29,32
};

const unsigned char p[32]=
{//P changable table
    16,7,20,21,
    29,12,28,17,
    1,15,23,26,
    5,22,31,10,
    2,8,24,14,
    32,22,3,9,
    19,13,36,6,
    22,11,4,25 
};

const unsigned char s[][4][16] =
{
	{//S box1¡£   All of the box can be Variable
		14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7,
		0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8,
		4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0,
		15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13 
	},
	{//S box2
		15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10,
		3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5,
		0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15,
		13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9 
	},
	{//S box3
		10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8,
		13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1,
		13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7,
		1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12 
	},
	{//S box4
		7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15,
		13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9,
		10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4,
		3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14
	},
	{ //S box5
		2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9,
		14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6,
		4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14,
		11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3
	},
	{//S box6
		12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11,
		10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8,
		9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6,
		4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13 
		},
	{//S box7
		4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1,
		13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6,
		1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2,
		6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12 
	},
	{//S box8
		13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7,
		1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2,
		7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8,
		2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11
	}
};

const unsigned char ip[64]=
{////IP table
    58,50,42,34,26,18,10,2,
    60,52,44,36,28,20,12,4,
    62,54,46,38,30,22,14,6,
    64,56,48,40,32,24,16,8,
    57,49,41,33,25,17,9, 1,
    59,51,43,35,27,19,11,3,
    61,53,45,37,29,21,13,5,
    63,55,47,39,31,23,15,7
};
const unsigned char ip_1[64]=
{//IP-1 table¡£It can be generated based on the IP table
    40,8,48,16,56,24,64,32,
    39,7,47,15,55,23,63,31,
    38,6,46,14,54,22,62,30,
    37,5,45,13,53,21,61,29,
    36,4,44,12,52,20,60,28,
    35,3,43,11,51,19,59,27,
    34,2,42,10,50,18,58,26,
    33,1,41,9, 49,17,57,25
};
const unsigned char e[48] =
{//Extend table
    32,1, 2, 3, 4, 5,
    4, 5, 6, 7, 8, 9,
    8, 9, 10,11,12,13,
    12,13,14,15,16,17,
    16,17,18,19,20,21,
    20,21,22,23,24,25,
    24,25,26,27,28,29,
    28,29,30,31,32,1
};
const char base[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="; 

static char *base64_encode(const char* data, int data_len)
{ 
    //int data_len = strlen(data); 
    int prepare = 0; 
    int ret_len; 
    int temp = 0; 
    char *ret = NULL; 
    char *f = NULL; 
    int tmp = 0; 
    char changed[4]; 
    int i = 0; 
    ret_len = data_len / 3; 
    temp = data_len % 3; 
    if (temp > 0) 
    { 
        ret_len += 1; 
    } 
    ret_len = ret_len*4 + 1; 
    ret = (char *)malloc(ret_len); 
      
    if ( ret == NULL) 
    { 
        printf("%s:No enough memory.\n",__FUNCTION__); 
        exit(0); 
    } 
    memset(ret, 0, ret_len); 
    f = ret; 
    while (tmp < data_len) 
    { 
        temp = 0; 
        prepare = 0; 
        memset(changed, '\0', 4); 
        while (temp < 3) 
        { 
            //printf("tmp = %d\n", tmp); 
            if (tmp >= data_len) 
            { 
                break; 
            } 
            prepare = ((prepare << 8) | (data[tmp] & 0xFF)); 
            tmp++; 
            temp++; 
        } 
        prepare = (prepare<<((3-temp)*8)); 
        //printf("before for : temp = %d, prepare = %d\n", temp, prepare); 
        for (i = 0; i < 4 ;i++ ) 
        { 
            if (temp < i) 
            { 
                changed[i] = 0x40; 
            } 
            else 
            { 
                changed[i] = (prepare>>((3-i)*6)) & 0x3F; 
            } 
            *f = base[changed[i]]; 
            //printf("%.2X", changed[i]); 
            f++; 
        } 
    } 
    *f = '\0'; 
      
    return ret; 
      
} 
/* */ 
static char find_pos(char ch)   
{ 
    char *ptr = (char*)strrchr(base, ch);//the last position (the only) in base[] 
    return (ptr - base); 
} 
/* */ 
char *base64_decode(const char *data, int data_len) 
{ 
    int ret_len = (data_len / 4) * 3; 
    int equal_count = 0; 
    char *ret = NULL; 
    char *f = NULL; 
    int tmp = 0; 
    int temp = 0; 
    char need[3]; 
    int prepare = 0; 
    int i = 0; 
    if (*(data + data_len - 1) == '=') 
    { 
        equal_count += 1; 
    } 
    if (*(data + data_len - 2) == '=') 
    { 
        equal_count += 1; 
    } 
    if (*(data + data_len - 3) == '=') 
    {//seems impossible 
        equal_count += 1; 
    } 
    switch (equal_count) 
    { 
    case 0: 
        ret_len += 4;//3 + 1 [1 for NULL] 
        break; 
    case 1: 
        ret_len += 4;//Ceil((6*3)/8)+1 
        break; 
    case 2: 
        ret_len += 3;//Ceil((6*2)/8)+1 
        break; 
    case 3: 
        ret_len += 2;//Ceil((6*1)/8)+1 
        break; 
    } 
    ret = (char *)malloc(ret_len); 
    if (ret == NULL) 
    { 
        printf("%s:No enough memory.\n",__FUNCTION__); 
        exit(0); 
    } 
    memset(ret, 0, ret_len); 
    f = ret; 
    while (tmp < (data_len - equal_count)) 
    { 
        temp = 0; 
        prepare = 0; 
        memset(need, 0, 4); 
        while (temp < 4) 
        { 
            if (tmp >= (data_len - equal_count)) 
            { 
                break; 
            } 
            prepare = (prepare << 6) | (find_pos(data[tmp])); 
            temp++; 
            tmp++; 
        } 
        prepare = prepare << ((4-temp) * 6); 
        for (i=0; i<3 ;i++ ) 
        { 
            if (i == temp) 
            { 
                break; 
            } 
            *f = (char)((prepare>>((2-i)*8)) & 0xFF); 
            f++; 
        } 
    } 
    *f = '\0'; 
    return ret; 
}

/**
 * @jh According xTab q will be converted into p
 */
static void xTran(unsigned char *q,unsigned char *p,unsigned char *xTab,int xLen)
{
	int				i=0,qt=0,pt=0,tt=0;//qt represents which bytes of q,same to pt,tt is a Temporary variable

	for(i=0;i<8;i++)
		p[i]=0;//Initialization
	
	for(i=0;i<xLen;i++)
	{//Cycle set
		pt=i/8;
		qt=(xTab[i]-1)/8;
		tt=q[qt] << ((xTab[i]-1) % 8);
		tt=tt & 0x80;//1000 0000
		tt=tt >> (i % 8);
		p[pt]=p[pt] | tt;
	}
}

/**
 * @jh nkey--16 sub-keys generated by okey 
 */
static void genKey(unsigned char *key,unsigned char nkey[16][8])
{
	unsigned char	tkey[8],tt[8];//64 bytes
	unsigned int	key_c,key_d; //pre 28 bits and last 28 bits
	int				i=0,j=0;

	for(i=0;i<8;i++)
		tkey[i]=0;    //initialize tkey[]
	i=0;
	//strcpy(tkey,key);

	while(i<8)
	{   
		tkey[i]=*(key+i);
		i++; 
	}

	xTran(tkey,tt,(unsigned char *)pc_1,56); //PC-1 replacement

	key_c	= (*(tt+0)<<24)+(*(tt+1)<<16)+(*(tt+2)<<8)+(*(tt+3));
	key_c	= key_c & 0xfffffff0;//pre 28 bits
	key_d	= (*(tt+3)<<24)+(*(tt+4)<<16)+(*(tt+5)<<8)+(*(tt+6));
	key_d	= key_d & 0x0fffffff;//last 28 bits
	
	for(i=0;i<16;i++)
	{	
		key_c=(key_c<<movebit[i]) | ((key_c>>(28-movebit[i])) & 0xfffffff0);
		key_d=((key_d<<movebit[i])& 0x0fffffff) | (key_d>>(28-movebit[i]));
		
		for(j=0;j<8;j++)
			tt[j]=0;//clear

		*(tt+0) = key_c>>24;   *(tt+1) = key_c>>16;
		*(tt+2) = key_c>>8;    *(tt+3) = key_c;		//merge key_c to tt
		*(tt+3)|=(key_d>>24);  *(tt+4) = key_d>>16;
		*(tt+5)=key_d>>8;      *(tt+6) = key_d;		//merge key_d to tt
		xTran(tt,nkey[i],(unsigned char *)pc_2,48);	//PC-2 replacement
	}
}//end of genKey

/**
 * @jh Find S-box, to expand into 48 data, replacing 32 data
 */
static void sReplace(unsigned char *right_s)
{
	unsigned short	tt;
	int				i=0;
	unsigned char	row,col;
	unsigned char	tmp_s[8]={0,0,0,0,0,0,0,0};

	for(i=0;i<=7;i++)
	{//get data from S box
		row=0,col=0;tt=0;
		tt=((right_s[i*6/8]<<8) + (right_s[i*6/8+1])) >> (10-i*6 % 8);
		row=(row | ((tt>>5) & 0x01)) << 1;
		row=(row | ((tt>>0) & 0x01)) << 0;
		col=(tt >> 1) & 0x0f;

		tmp_s[i/2]=tmp_s[i/2]|s[i][row][col]<<4*((i+1)%2);
	} //s_out[0-3]:???????? ???????? ???????? ???????? 00000000...
	xTran(tmp_s,right_s,(unsigned char *)p,32);
}//end of sReplace


/**
 * @jh DES common function
 * @param[in] unsigned char in[8] input data
 * @param[out] unsigned char out[8] output data
 * @param[in] unsigned char subkey[16][8] KEY
 * @param[in] unsigned char flg exec type
 */
static unsigned char comDES(unsigned char in[8],unsigned char out[8],
					 unsigned char subkey[16][8],unsigned char flg)
{
	unsigned char	left[8],right[8],temp[8];
	int				i=0,j=0,k=0;

	xTran(in,temp,(unsigned char *)ip,64); //IP replacement

	for(i=0;i<=3;i++)
		left[i]=temp[i] ;

	for(i=4;i<=7;i++)
		right[i-4]=temp[i] ;

	for(i=0;i<16;i++)
	{
		if(flg==g_ENCRYPT_FLAG)
			k=i;
		else if(flg==g_DECRYPT_FLAG)
			k=15-i;
		else
			return 0;

		for(j=0;j<=3;j++)
		{
			temp[j]=left[j];
			left[j]=right[j];//L(n) = R(n-1)
		}

		xTran(left,right,(unsigned char *)e,48);
		
		for(j=0;j<6;j++)
			right[j]=right[j]^subkey[k][j];
		
		sReplace(right) ;
		for(j=0;j<=3;j++)
		{//get other right
			right[j]=temp[j] ^ right[j] ;//f(R(n-1),k)
		}
	}
	for(i=0;i<4;i++)
		temp[i]=right[i];

	for(i=4;i<8;i++)
		temp[i]=left[i-4];

	xTran(temp,out,(unsigned char *)ip_1,64);
	return 1;
}//end of comDES

/** 
 * @jh enDES  3DES encrypt
 * @param[in] unsigned char* indata Encrypt data
 * @param[in] unsigned int inlen Length
 * @param[in] unsigned char* key KEY
 * @param[out] unsigned char* outdata Result
 * @param[out] unsigned int* outlen Result length
 */
unsigned char enDES(unsigned char* indata,unsigned int inlen,unsigned char* key,
					unsigned char* outdata,unsigned int* outlen)
{//encrypt
	unsigned char	*p,*p1,s_key[16][8],tt[8];
	int				tlen=0,i=0,ttlen=0;
	unsigned char	padding[7] ={0x80,0,0,0,0,0,0};	//GPCardSpecV2.2.1--B.4
	//    if((indata==NULL)||(outdata==NULL)||(key==NULL)
	//        ||(outlen==NULL)||(inlen<=0)||(*outlen<inlen))
	if((indata==NULL)||(outdata==NULL)||(key==NULL)
		||(outlen==NULL)||(inlen<=0))
		return 0;//invalid parameter
	p=indata;
	tlen=inlen/8;
	if(inlen%8!=0)
	{
		memcpy(indata+inlen,padding,8-(inlen%8));
		tlen=tlen+1;	
	}
	tlen=tlen*8;

	//	printf("The result is£º\n");
	//	for (i = 0;i < tlen;i++)					//print result
	//	{
	//		if(i%8 == 0) printf("\n");
	//		printf("0x%X ",indata[i]);
	//	}

	*outlen=tlen;//length //must be an integer multiple of 64bit.

	p1=outdata;tlen=inlen;
	for(i=0;i<8;i++)
		tt[i]=0;

	genKey(key,s_key);
	while(tlen>0)
	{
		for(i=0;i<8;i++) tt[i]=0;
		ttlen=(tlen<8)?tlen:8; 
		for(i=0;i<ttlen;i++)
			tt[i]=*(p+i);
		comDES(tt,p1,s_key,g_ENCRYPT_FLAG);
		p=p+8;p1=p1+8;tlen=tlen-8;
	}
	return 1;
}

/**
 * @jh decrypt
 * @param[in] unsigned char* indata Encrypt data
 * @param[in] unsigned int inlen Length
 * @param[in] unsigned char* key KEY
 * @param[out] unsigned char* outdata Result
 * @param[out] unsigned int* outlen Result length
 */
unsigned char unDES(unsigned char* indata,unsigned int inlen,unsigned char* key,
					unsigned char* outdata,unsigned int* outlen)
{
	unsigned char	*p,*p1,s_key[16][8],tt[8];
	int				tlen=0,i=0,ttlen=0;

	if((indata==NULL)||(outdata==NULL)||(key==NULL)
		||(outlen==NULL)||(inlen<=0)||(*outlen<inlen))
		return 0;//invalid parameter

	p=indata;
	tlen=inlen/8;
	if(inlen%8!=0)
		tlen=tlen+1;
	tlen=tlen*8;
	*outlen=tlen;
	p1=outdata;tlen=inlen;
	for(i=0;i<8;i++)    tt[i]=0;
	genKey(key,s_key);
	while(tlen>0)
	{
		for(i=0;i<8;i++) tt[i]=0;
		ttlen=(tlen<8)?tlen:8;
		for(i=0;i<ttlen;i++)
			tt[i]=*(p+i);
		comDES(tt,p1,s_key,g_DECRYPT_FLAG);
		p=p+8;p1=p1+8;tlen=tlen-8;
	}
	return 1;
}

//================Randomly generated key========================
static void randKey(unsigned char key[8])
{//Randomly generated a 64 bit key
	unsigned char i;
	for(i=0;i<8;i++)
		key[i]=rand() % 0x0100;
};

/**
 * @jh 3DES encrypt
 * @param[in] unsigned char* indata Input data
 * @param[in] unsigned int inlen Input data length
 * @param[in] unsigned char* key1 KEY1
 * @param[in] unsigned char* key2 KEY2
 * @param[in] unsigned char* key3 KEY3
 * @param[out] unsigned char* outdata Result
 * @param[out] unsigned int* outlen Result length
 */
unsigned char en3DES(unsigned char* indata,unsigned int inlen,unsigned char* key_1,unsigned char* key_2,
					 unsigned char* key_3,unsigned char* outdata,unsigned int* outlen)
{
	unsigned char	*p,*p1,s_key_1[16][8],s_key_2[16][8],s_key_3[16][8],tt[8];
	int				tlen=0,i=0,ttlen=0;
	unsigned char	padding[7] ={0x80,0,0,0,0,0,0};

	if((indata==NULL)||(outdata==NULL)||(key_1==NULL)||(key_2==NULL)||(key_3==NULL)\
		||(outlen==NULL)||(inlen<=0))
		return 0;//invalid parameter

	p=indata;
	tlen=inlen/8;
	if(inlen%8!=0)
	{
		memcpy(indata+inlen,padding,8-(inlen%8));
		tlen=tlen+1;	
	}

	tlen=tlen*8;
	*outlen=tlen;
	genKey(key_1,s_key_1);
	genKey(key_2,s_key_2);
	genKey(key_3,s_key_3);
	p1=outdata;tlen=inlen;

	while(tlen>0)
	{
		for(i=0;i<8;i++) tt[i]=0;		
		ttlen=(tlen<8)?tlen:8; 
		for(i=0;i<ttlen;i++)  tt[i]=*(p+i);
		comDES(tt,p1,s_key_1,g_ENCRYPT_FLAG);
		for(i=0;i<8;i++) tt[i]=p1[i];
		comDES(tt,p1,s_key_2,g_DECRYPT_FLAG);		
		for(i=0;i<8;i++) tt[i]=p1[i];
		comDES(tt,p1,s_key_3,g_ENCRYPT_FLAG);
		p=p+8;p1=p1+8;tlen=tlen-8;
	}
	return 1;
}


/**
 * @jh 3DES decrypt
 * @param[in] unsigned char* indata Input data
 * @param[in] unsigned int inlen Input data length
 * @param[in] unsigned char* key1 KEY1
 * @param[in] unsigned char* key2 KEY2
 * @param[in] unsigned char* key3 KEY3
 * @param[out] unsigned char* outdata Result
 * @param[out] unsigned int* outlen Result length
 */
unsigned char un3DES(unsigned char* indata,unsigned int inlen,unsigned char* key_1,unsigned char* key_2,
					 unsigned char* key_3,unsigned char* outdata,unsigned int* outlen)
{
	unsigned char *p,*p1,s_key_1[16][8],s_key_2[16][8],s_key_3[16][8],tt[8];
	int tlen=0,i=0,ttlen=0;

	if((indata==NULL)||(outdata==NULL)||(key_1==NULL)||(key_2==NULL)||(key_3==NULL)
		||(outlen==NULL)||(inlen<=0))
		return 0;//invalid parameter

	p			= indata;
	tlen		= inlen/8;
	if(inlen%8!=0)
		tlen	= tlen+1;
	tlen		= tlen*8;

	*outlen		= tlen;
	genKey(key_1,s_key_1);
	genKey(key_2,s_key_2);
	genKey(key_3,s_key_3);
	p1			= outdata;
	tlen		= inlen;

	for(i=0;i<8;i++)
		tt[i]=0;

	while(tlen>0)
	{
		for(i=0;i<8;i++)
			tt[i]=0;		
		ttlen=(tlen<8)?tlen:8; 

		for(i=0;i<ttlen;i++)
			tt[i]=*(p+i);
		comDES(tt,p1,s_key_3,g_DECRYPT_FLAG);
		
		for(i=0;i<8;i++)
			tt[i]=p1[i];
		comDES(tt,p1,s_key_2,g_ENCRYPT_FLAG);

		for(i=0;i<8;i++)
			tt[i]=p1[i];
		comDES(tt,p1,s_key_1,g_DECRYPT_FLAG);

		p=p+8;p1=p1+8;tlen=tlen-8;
	}
	return 1;
}
char *getSerialNumber(char *snnumber)
{
    const char *key1="1234dcba";//KEY1
    const char *key2="abcd4321";//KEY2
    const char *key3="f8e75c6d";//KEY3
    char outdata[64] = {0};
    char *base64en = NULL;
    int outlen = 0;
    
    if(strlen(snnumber) != 12)
    {
        printf("invalid SN code \n");
        return NULL;
    }
    en3DES(snnumber,strlen(snnumber),key1,key2,key3,outdata,&outlen);
    base64en = base64_encode(outdata, outlen); 
    
    return base64en;
}
#if 0
int main(void)//Usage
{
     const char *key1="1234dcba";//KEY1
     const char *key2="abcd4321";//KEY2
     const char *key3="f8e75c6d";//KEY3
     char outdata[256] = {0};
     char outdata2[256] = {0};
     char *base64en = NULL;
     char *base64de = NULL;
     char encrystr[16]={'8','4','5','1','1','a','b','c','d','e','0','0',0};//"84511abcde00";  Serial Number
     int outlen = 0,i=0;
     
     /*randKey(key1);
     randKey(key2);
     randKey(key3);*/
     /************3DES encry***************/
     printf("key1=%s\n",key1);
     printf("key2=%s\n",key2);
     printf("key3=%s\n",key3);
     printf("encrystr=%s\n",encrystr);
     en3DES(encrystr,strlen(encrystr),key1,key2,key3,outdata,&outlen);
	   printf("encryresult:\n");
     //printf("outdata=%s    outlen=%d\n",outdata,outlen);
     base64en = base64_encode(outdata, outlen); 
     if(base64en)
     {
     	 printf("base64en=%s   %d  %d\n",base64en,outlen,strlen(base64en));
     }
     else 
     {
            printf("encry error!\n");
     	    exit(1);
     }

     /************3DES decry***************/
     base64de = base64_decode(base64en, strlen(base64en)); //printf("base64de=%s\n",base64de);
     if(base64de)
     {
         un3DES(base64de,strlen(base64de),key1,key2,key3,outdata2,&outlen);
         printf("dentryresult:\n");
	 /* printf("key1=%s\n",key1);
         printf("key2=%s\n",key2);
         printf("key3=%s\n",key3);*/
         printf("deoutdata=%s outlen=%d\n",outdata2,outlen);
     }
     else
     {
     	 printf("3DES decry error!\n");
         exit(1);
     }
     
     free(base64en);
     base64en = NULL;
     free(base64de);
     base64de = NULL;
/*
key1=1234dcba
key2=abcd4321
key3=f8e75c6d
encrystr=84511abcde00
encryresult:
base64en=YO0eIQdPfncVe8pgR0q1Pg==
dentryresult:
deoutdata=84511abcde00 outlen=16

*/

     return 0;
}
#endif