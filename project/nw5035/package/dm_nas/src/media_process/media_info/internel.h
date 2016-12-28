#ifndef _INTERNEL_H_
#define _INTERNEL_H_

#define DEBUG_ENABLE

#ifdef DEBUG_ENABLE
#define LOG_E(fmt, args...) fprintf(stderr, fmt, ##args); 
#define LOG_I(fmt, args...) fprintf(stderr, fmt, ##args); 
#define LOG_W(fmt, args...) fprintf(stderr, fmt, ##args); 
#else
#define LOG_E(fmt, args...)
#define LOG_I(fmt, args...)
#define LOG_W(fmt, args...)
#endif

#define safe_free(p) do{\
    if((p) != NULL)\
    {\
        free((p));\
        (p) = NULL;\
    }\
    }while(0)


#endif
