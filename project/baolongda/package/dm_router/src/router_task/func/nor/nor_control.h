#ifndef __NOR_CONTROL_H
#define __NOR_CONTROL_H
#include "router_defs.h"
#define DATA_LEN		256
#ifdef SUPPORT_LINUX_PLATFORM
int nor_set(char *str_name,unsigned char *value);
int cfg_nor_get(char *argv);
int cfg_nor_set(char *argv);
void reset_user_config();
void print_all_config(void);
#endif
#endif
