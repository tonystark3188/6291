#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

//output direction
#define DIR_CONSOLE 	0 
#define DIR_PRINTF 		1

//debug level
#define LEVEL_DEBUG	 	4
#define LEVEL_INFO 		3
#define LEVEL_MESSAGE 	2
#define LEVEL_ERROR		1
#define LEVEL_NONE 		0

//set debug config
#define APPLICATION_NAME	"DM_ROUTER"
#define DEBUG_LEVEL 		LEVEL_DEBUG
#define DEBUG_DIR			DIR_PRINTF

#define c_Print_Ctrl_Off "\033[0m"
#define c_CharColor_Black "\033[1;30m"
#define c_CharColor_Red "\033[1;31m"
#define c_CharColor_Green "\033[1;32m"
#define c_CharColor_yellow      "\033[1;33m"
#define c_CharColor_Blue "\033[1;34m"
#define c_CharColor_Purple      "\033[1;35m"
#define c_CharColor_DarkGreen   "\033[1;36m"
#define c_CharColor_White "\033[1;37m"

#define c_Print_Ctrl_Off "\033[0m"


#if DEBUG_DIR == DIR_PRINTF
#if DEBUG_LEVEL == LEVEL_DEBUG
#define DMCLOG_M(log_fmt, log_arg...) \
    do{ \
		struct timeval tnow;\
				gettimeofday(&tnow, NULL);\
				printf( c_CharColor_yellow"[%s][%08d.%06d][%s:%d][%s][MESSAGE] " log_fmt "\n"c_Print_Ctrl_Off, APPLICATION_NAME,(unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec,\
							 __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)
#define DMCLOG_D(log_fmt, log_arg...) \
    do{ \
		struct timeval tnow;\
				gettimeofday(&tnow, NULL);\
				printf( "[%s][%08d.%06d][%s:%d][%s][DEBUG] " log_fmt "\n", APPLICATION_NAME,(unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec,\
							 __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)
#define DMCLOG_I(log_fmt, log_arg...) \
    do{ \
		struct timeval tnow;\
				gettimeofday(&tnow, NULL);\
				printf( "[%s][%08d.%06d][%s:%d][%s][INFO] " log_fmt "\n", APPLICATION_NAME,(unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec,\
							 __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)
#define DMCLOG_E(log_fmt, log_arg...) \
    do{ \
		struct timeval tnow;\
				gettimeofday(&tnow, NULL);\
				printf( c_CharColor_Red"[%s][%08d.%06d][%s:%d][%s][ERROR] " log_fmt "\n"c_Print_Ctrl_Off, APPLICATION_NAME,(unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec,\
							 __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)

	
#elif DEBUG_LEVEL == LEVEL_INFO
#define DMCLOG_M(log_fmt, log_arg...) \
    do{ \
		struct timeval tnow;\
				gettimeofday(&tnow, NULL);\
				printf( "[%s][%08d.%06d][%s:%d][%s][MESSAGE] " log_fmt "\n", APPLICATION_NAME,(unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec,\
							 __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)
#define DMCLOG_D(log_fmt, log_arg...) \
    do{ \
    } while (0)
#define DMCLOG_I(log_fmt, log_arg...) \
    do{ \
		struct timeval tnow;\
				gettimeofday(&tnow, NULL);\
				printf( "[%s][%08d.%06d][%s:%d][%s][INFO] " log_fmt "\n", APPLICATION_NAME,(unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec,\
							 __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)
#define DMCLOG_E(log_fmt, log_arg...) \
    do{ \
		struct timeval tnow;\
				gettimeofday(&tnow, NULL);\
				printf( "[%s][%08d.%06d][%s:%d][%s][ERROR] " log_fmt "\n", APPLICATION_NAME,(unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec,\
							 __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)


#elif DEBUG_LEVEL == LEVEL_MESSAGE
#define DMCLOG_M(log_fmt, log_arg...) \
	do{ \
		struct timeval tnow;\
		gettimeofday(&tnow, NULL);\
		printf( "[%s][%08d.%06d][%s:%d][%s][MESSAGE] " log_fmt "\n", APPLICATION_NAME,(unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec,\
					 __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
	} while (0)
#define DMCLOG_D(log_fmt, log_arg...) \
    do{ \
    } while (0)
#define DMCLOG_I(log_fmt, log_arg...) \
    do{ \
    } while (0)
#define DMCLOG_E(log_fmt, log_arg...) \
    do{ \
		struct timeval tnow;\
				gettimeofday(&tnow, NULL);\
				printf( "[%s][%08d.%06d][%s:%d][%s][ERROR] " log_fmt "\n", APPLICATION_NAME,(unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec,\
							 __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)

	
#elif DEBUG_LEVEL == LEVEL_ERROR
#define DMCLOG_M(log_fmt, log_arg...) \
	do{ \
	} while (0)
#define DMCLOG_D(log_fmt, log_arg...) \
	do{ \
	} while (0)
#define DMCLOG_I(log_fmt, log_arg...) \
	do{ \
	} while (0)
#define DMCLOG_E(log_fmt, log_arg...) \
	do{ \
		struct timeval tnow;\
				gettimeofday(&tnow, NULL);\
				printf( "[%s][%08d.%06d][%s:%d][%s][ERROR] " log_fmt "\n", APPLICATION_NAME,(unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec,\
							 __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
	} while (0)


#elif DEBUG_LEVEL == LEVEL_NONE
#define DMCLOG_M(log_fmt, log_arg...) \
    do{ \
    } while (0)
#define DMCLOG_D(log_fmt, log_arg...) \
    do{ \
    } while (0)
#define DMCLOG_I(log_fmt, log_arg...) \
    do{ \
    } while (0)
#define DMCLOG_E(log_fmt, log_arg...) \
    do{ \
    } while (0)
#endif


#elif DEBUG_DIR == DIR_CONSOLE
#if DEBUG_LEVEL == LEVEL_DEBUG
#define DMCLOG_M(log_arg...) \
    do{ \
		struct timeval tnow;\
        FILE *fp = fopen("/dev/console","w");\
        if(fp){\
                gettimeofday(&tnow, NULL);\
                fprintf(fp,"[%s][%08d.%06d][%s:%d][%s][MESSAGE] ", APPLICATION_NAME, (unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec, \
                			__FILE__, __LINE__, __FUNCTION__); \
                fprintf(fp,##log_arg);\
                fprintf(fp,"\n");\
                fclose(fp);\
        }\
	} while (0)
#define DMCLOG_D(log_arg...) \
    do{ \
		struct timeval tnow;\
        FILE *fp = fopen("/dev/console","w");\
        if(fp){\
                gettimeofday(&tnow, NULL);\
                fprintf(fp,"[%s][%08d.%06d][%s:%d][%s][DEBUG] ", APPLICATION_NAME, (unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec, \
                			__FILE__, __LINE__, __FUNCTION__); \
                fprintf(fp,##log_arg);\
                fprintf(fp,"\n");\
                fclose(fp);\
        	}\
    } while (0)
#define DMCLOG_I(log_arg...) \
    do{ \
		struct timeval tnow;\
        FILE *fp = fopen("/dev/console","w");\
        if(fp){\
                gettimeofday(&tnow, NULL);\
                fprintf(fp,"[%s][%08d.%06d][%s:%d][%s][INFO] ", APPLICATION_NAME, (unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec, \
                			__FILE__, __LINE__, __FUNCTION__); \
                fprintf(fp,##log_arg);\
                fprintf(fp,"\n");\
                fclose(fp);\
              }\
    } while (0)
#define DMCLOG_E(log_arg...) \
    do{ \
		struct timeval tnow;\
        FILE *fp = fopen("/dev/console","w");\
        if(fp){\
                gettimeofday(&tnow, NULL);\
                fprintf(fp,"[%s][%08d.%06d][%s:%d][%s][ERROR] ", APPLICATION_NAME, (unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec, \
                			__FILE__, __LINE__, __FUNCTION__); \
                fprintf(fp,##log_arg);\
                fprintf(fp,"\n");\
                fclose(fp);\
             }\
    } while (0)

	
#elif DEBUG_LEVEL == LEVEL_INFO
#define DMCLOG_M(log_arg...) \
    do{ \
		struct timeval tnow;\
        FILE *fp = fopen("/dev/console","w");\
        if(fp){\
                gettimeofday(&tnow, NULL);\
                fprintf(fp,"[%s][%08d.%06d][%s:%d][%s][MESSAGE] ", APPLICATION_NAME, (unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec, \
                			__FILE__, __LINE__, __FUNCTION__); \
                fprintf(fp,##log_arg);\
                fprintf(fp,"\n");\
                fclose(fp);\
             }\
    } while (0)
#define DMCLOG_D(log_arg...) \
    do{ \
    } while (0)
#define DMCLOG_I(log_arg...) \
    do{ \
		struct timeval tnow;\
        FILE *fp = fopen("/dev/console","w");\
        if(fp){\
                gettimeofday(&tnow, NULL);\
                fprintf(fp,"[%s][%08d.%06d][%s:%d][%s][INFO] ", APPLICATION_NAME, (unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec, \
                			__FILE__, __LINE__, __FUNCTION__); \
                fprintf(fp,##log_arg);\
                fprintf(fp,"\n");\
                fclose(fp);\
             }\
	} while (0)
#define DMCLOG_E(log_arg...) \
    do{ \
		struct timeval tnow;\
        FILE *fp = fopen("/dev/console","w");\
        if(fp){\
                gettimeofday(&tnow, NULL);\
                fprintf(fp,"[%s][%08d.%06d][%s:%d][%s][ERROR] ", APPLICATION_NAME, (unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec, \
                			__FILE__, __LINE__, __FUNCTION__); \
                fprintf(fp,##log_arg);\
                fprintf(fp,"\n");\
                fclose(fp);\
             }\
    } while (0)


#elif DEBUG_LEVEL == LEVEL_MESSAGE
#define DMCLOG_M(log_arg...) \
	do{ \
		struct timeval tnow;\
        FILE *fp = fopen("/dev/console","w");\
        if(fp){\
                gettimeofday(&tnow, NULL);\
                fprintf(fp,"[%s][%08d.%06d][%s:%d][%s][MESSAGE] ", APPLICATION_NAME, (unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec, \
                			__FILE__, __LINE__, __FUNCTION__); \
                fprintf(fp,##log_arg);\
                fprintf(fp,"\n");\
                fclose(fp);\
             }\
	} while (0)
#define DMCLOG_D(log_arg...) \
    do{ \
    } while (0)
#define DMCLOG_I(log_arg...) \
    do{ \
    } while (0)
#define DMCLOG_E(log_arg...) \
    do{ \
		struct timeval tnow;\
        FILE *fp = fopen("/dev/console","w");\
        if(fp){\
                gettimeofday(&tnow, NULL);\
                fprintf(fp,"[%s][%08d.%06d][%s:%d][%s][ERROR] ", APPLICATION_NAME, (unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec, \
                			__FILE__, __LINE__, __FUNCTION__); \
                fprintf(fp,##log_arg);\
                fprintf(fp,"\n");\
                fclose(fp);\
             }\
    } while (0)

	
#elif DEBUG_LEVEL == LEVEL_ERROR
#define DMCLOG_M(log_arg...) \
	do{ \
	} while (0)
#define DMCLOG_D(log_arg...) \
	do{ \
	} while (0)
#define DMCLOG_I(log_arg...) \
	do{ \
	} while (0)
#define DMCLOG_E(log_arg...) \
	do{ \
		struct timeval tnow;\
        FILE *fp = fopen("/dev/console","w");\
        if(fp){\
                gettimeofday(&tnow, NULL);\
                fprintf(fp,"[%s][%08d.%06d][%s:%d][%s][ERROR] ", APPLICATION_NAME, (unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec, \
                			__FILE__, __LINE__, __FUNCTION__); \
                fprintf(fp,##log_arg);\
                fprintf(fp,"\n");\
                fclose(fp);\
             }\
	} while (0)


#elif DEBUG_LEVEL == LEVEL_NONE
#define DMCLOG_M(log_arg...) \
    do{ \
    } while (0)
#define DMCLOG_D(log_arg...) \
    do{ \
    } while (0)
#define DMCLOG_I(log_arg...) \
    do{ \
    } while (0)
#define DMCLOG_E(log_arg...) \
    do{ \
    } while (0)
#endif

#endif


