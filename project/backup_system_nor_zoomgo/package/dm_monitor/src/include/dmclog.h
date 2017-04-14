#ifndef  _DMCLOG_H
#define  _DMCLOG_H

#define LEVEL_DEBUG 3
#define LEVEL_INFO 2
#define LEVEL_MESSAGE 1
#define LEVEL_NONE 0

#define DEBUG_LEVEL LEVEL_NONE

#if DEBUG_LEVEL == LEVEL_DEBUG
#define DMCLOG_M(log_fmt, log_arg...) \
    do{ \
        printf( "[%s:%d][%s][MESSAGE] " log_fmt "\n", \
                     __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)
#define DMCLOG_D(log_fmt, log_arg...) \
    do{ \
        printf( "[%s:%d][%s][DEBUG] " log_fmt "\n", \
                     __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)
#define DMCLOG_I(log_fmt, log_arg...) \
    do{ \
        printf( "[%s:%d][%s][INFO] " log_fmt "\n", \
                     __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)

#define DMCLOG_E(log_fmt, log_arg...) \
    do{ \
        printf( "[%s:%d][%s][ERROR] " log_fmt "\n", \
                     __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)
#elif DEBUG_LEVEL == LEVEL_INFO

#define DMCLOG_M(log_fmt, log_arg...) \
    do{ \
        printf( "[%s:%d][%s][MESSAGE] " log_fmt "\n", \
                     __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)
#define DMCLOG_D(log_fmt, log_arg...) \
    do{ \
    } while (0)
#define DMCLOG_I(log_fmt, log_arg...) \
    do{ \
        printf( "[%s:%d][%s][debug] " log_fmt "\n", \
                     __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)

#define DMCLOG_E(log_fmt, log_arg...) \
    do{ \
        printf( "[%s:%d][%s][ERROR] " log_fmt "\n", \
                     __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)
#elif DEBUG_LEVEL == LEVEL_MESSAGE

#define DMCLOG_M(log_fmt, log_arg...) \
    do{ \
        printf( "[%s:%d][%s][MESSAGE] " log_fmt "\n", \
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
        printf( "[%s:%d][%s][ERROR] " log_fmt "\n", \
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

//#define DM_UTILS_MIN(x, y) {	long z;	if(x>y) z=y;	else z=x;	return (z);}



#endif
