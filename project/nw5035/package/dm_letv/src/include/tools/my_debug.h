#include "base.h"
#define DEBUG
#ifdef DEBUG
//FILE *fp = fopen("/dev/console","w");\
//fclose(fp);
//FILE *fp = fopen("/tmp/mnt/USB-disk-1/log.txt","a+");\

#define p_debug(args...) do{\
        struct timeval tnow;\
		FILE *fp = stdout;\
        if(fp){\
                gettimeofday(&tnow, NULL);\
                fprintf(fp,"[LETV][%08d.%06d][%s][-%d] ", (unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec, __FUNCTION__, __LINE__);\
                fprintf(fp,##args);\
                fprintf(fp,"\n");\
                fflush(stdout);\
        }\
}while(0)
#else
#define p_debug(...)  
#endif

