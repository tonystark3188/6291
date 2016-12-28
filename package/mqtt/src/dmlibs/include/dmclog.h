
#ifndef  _DMCLOG_H
#define  _DMCLOG_H

#define DMCLOG_I(log_fmt, log_arg...) \
    do{ \
        printf( "[%s:%d][%s][INFO] " log_fmt "\n", \
                     __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)

#define DMCLOG_D(log_fmt, log_arg...) \
do{ \
    printf( "[%s:%d][%s][DEBUG] " log_fmt "\n", \
           __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
} while (0)

#define DMCLOG_E(log_fmt, log_arg...) \
    do{ \
        printf( "[%s:%d][%s][ERROR] " log_fmt "\n", \
                     __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)
#endif
