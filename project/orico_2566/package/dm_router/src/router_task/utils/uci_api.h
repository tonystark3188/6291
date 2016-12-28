#ifndef _UCI_API_H
#define _UCI_API_H

#ifdef SUPPORT_OPENWRT_PLATFORM
int uci_set_option_value(char *uci_str);
int uci_get_option_value(char *uci_option_str,char *uci_str_value);
static int shell_system_cmd(const char * cmd,char *cmd_result);
int shell_uci_get_value(char *uci_option_str,char *uci_str_value);
int shell_uci_set_value(char *uci_str);
#endif

#endif

