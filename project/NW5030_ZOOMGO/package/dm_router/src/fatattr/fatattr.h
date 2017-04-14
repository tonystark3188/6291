/*
*	fatattr.h
*
*	Create by oliver in 16/9/12
*/

#ifndef ___fatattr__
#define ___fatattr__
#include <stdbool.h>
#include "my_debug.h"
#include "base.h"
/*
* func:set file or dir to hide that cannot access in windows
*
* para: the full path
*
* return : 0 succ, -1 failed
*/
int dm_set_attr_hide(char *path,bool attr,bool album);
/*
* func:get file or dir hide attr  in windows
*
* para: the full path
*
* return : true: hided,false:not hide
*/
bool dm_get_attr_hide(char *path);
#endif /*defined(___fatattr__)*/