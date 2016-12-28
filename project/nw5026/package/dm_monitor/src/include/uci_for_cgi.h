#ifndef __UCI_FOR_CGI_H
#define __UCI_FOR_CGI_H
#ifdef SUPPORT_OPENWRT_PLATFORM

extern struct uci_context *ctx;
//extern int uci_get_str_len;

extern int uci_get_option_value(char *uci_option_str,char *uci_str_value);
extern int uci_set_option_value(char *uci_str);
#endif

#endif
