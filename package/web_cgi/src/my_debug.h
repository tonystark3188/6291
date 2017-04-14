//#include "base.h"
//FILE *fp = fopen("/tmp/mnt/USB-disk-1/log.txt","a+");\
//        FILE *fp = fopen("/dev/console","w");\
//#define DEBUG

#ifdef DEBUG
#define p_debug(args...) do{\
        struct timeval tnow;\
        FILE *fp = fopen("/dev/console","w");\
        if(fp){\
                gettimeofday(&tnow, NULL);\
                fprintf(fp,"[CGI][%08d.%06d][%s][-%d] ", (unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec, __FUNCTION__, __LINE__);\
                fprintf(fp,##args);\
                fprintf(fp,"\n");\
                fclose(fp);\
        }\
}while(0)
#else
	#define p_debug(...)  
#endif

