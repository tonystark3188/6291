#ifndef __NOR_H
#define __NOR_H



#define Debug_cfg 0

#define USERCONFIGPATH "/factory/"
#define DATA_LEN		256

int nor_set(char *str_name,unsigned char *value);
int cfg_nor_get(char *argv);
int cfg_nor_set(char *argv);
void reset_user_config();

void print_all_config(void);

#endif
