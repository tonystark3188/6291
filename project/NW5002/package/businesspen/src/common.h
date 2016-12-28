#ifndef _COMMON_H
#define _COMMON_H

#define MAXPATHSIZE 128

#define RETSUCCESS 0
#define RETERROR -1
#define RET_SUCCESS (RETSUCCESS)
#define RET_ERROR (RETERROR)
#define RET_INIT (1)
#define RET_FDINVALID (RETERROR)

#define MAXURLSIZE 512
#define MINURLSIZE 8
#define MAXACTIONLEN 64
#define MAXSONGIDSIZE 15
#define WORDLEN 256
#define BAKEXTEND 0

#define TESTUSERNAME "PENaabbcc"


#ifndef FreeBuf
#define FreeBuf(x) \
{ \
    if(x != NULL) \
    {\
    	free(x);	\
    	x = NULL;\
    }\
}
#endif

#define MAXRECORDNODE 16
#define HAVEENCODETHREAD 0
#define ENABLENOISEREDUCE 0
#define ENCODECHANNEL 1
#define ENCODESPEED 16000
#define DEFAULT_BUFF_SIZE (4096 * 4)//(16 * ENCODECHANNEL * ENCODESPEED / 8)//1S


#define OpenfileToAdd(fp,name) (fp = fopen(name,"a+"))
#define OpenfileToWrite(fp,name) (fp = fopen(name,"w+"))
#define OpenfileToRead(fp,name) (fp = fopen(name,"r"))

/*
#define LOG_ERR(fmt, args...) fprintf(stderr, fmt, ##args)
#define LOG_MSG(fmt, args...) fprintf(stdout, fmt, ##args)
#define LOG_DEB(fmt, args...) fprintf(stdout, fmt, ##args)
*/

#define PRINTFERROR(fmt, args...) do \
                       {  \
                          printf("ERROR in %s line %d :",__FUNCTION__,__LINE__); \
                          printf(fmt, ##args); \
                       }while(0)

#define PRINTFWARNING(fmt, args...) do \
                       {  \
                          printf("WARNING in %s line %d\n",__FUNCTION__,__LINE__); \
                          printf(fmt, ##args); \
                       }while(0)


#define YES 1
#define NO 0

#ifndef ENTER
#define ENTER() do{ printf("enter %s\n",__FUNCTION__);}while(0)
#define LEAVE() do{ printf("leave %s\n",__FUNCTION__);}while(0)
#endif


extern int g_logLevel;
#endif
