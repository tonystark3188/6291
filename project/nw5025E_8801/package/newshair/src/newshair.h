 #include <stdio.h>
 
 #define F_OUT(fmt,args...) \
     do{ \
         FILE *fp; \
         fp = fopen("/tmp/newshair_log", "a+");\
         fprintf(fp, fmt, ##args); \
         fflush(fp);\
         fclose(fp);\
     }while(0)

