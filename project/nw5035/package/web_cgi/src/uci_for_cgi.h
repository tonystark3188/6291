#ifndef __UCI_FOR_CGI_H
#define __UCI_FOR_CGI_H

#define SID_LEN 64
#define CODE_LEN 64
#define UCI_BUF_LEN 128


extern struct uci_context *ctx;
//extern int uci_get_str_len;

extern int uci_get_option_value(char *uci_option_str,char *uci_str_value);
extern int uci_set_option_value(char *uci_str);
extern void cgi_log(char *string);
extern char * GetStringFromWeb();
extern int processString(char *string,char *name,char *value);

unsigned char* urlDecode(char *string);

#define debug
#ifdef debug
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
#endif

#endif
