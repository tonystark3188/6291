#ifndef __CFG_API_H__
#define __CFG_API_H__
extern void del_n(char *str);
extern void set_cfg_str(char *str);
extern int get_cfg_str(char *param,char *ret_str);
extern char *get_cfg_str_malloc(char *param);
extern int cfg_set_and_check_str(char *str);

#endif
