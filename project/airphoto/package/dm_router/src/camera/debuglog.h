#ifndef __DEBUGLOG_H
#define __DEBUGLOG_H

#include "my_debug.h"
void die(char *format, ...);
void warn(char *format, ...);
void debug(int level, char *format, ...);

extern int debuglev;

#define c_Print_Ctrl_Off "\033[0m"
#define c_CharColor_Black "\033[1;30m"
#define c_CharColor_Red "\033[1;31m"
#define c_CharColor_Green "\033[1;32m"
#define c_CharColor_yellow      "\033[1;33m"
#define c_CharColor_Blue "\033[1;34m"
#define c_CharColor_Purple      "\033[1;35m"
#define c_CharColor_DarkGreen   "\033[1;36m"
#define c_CharColor_White "\033[1;37m"



#ifndef __MY_CHECK_MARCO_
#define __MY_CHECK_MARCO_

//if result < 0, printf error message, goto label
#define CK00(result, label)	\
do	\
{	\
 int __r = (result);	\
 if (__r < 0) {	\
  DMCLOG_E(c_CharColor_Red"result=%d < 0"c_Print_Ctrl_Off, __r);\
  goto label; \
 }	\
}while(0)
//if result < 0, printf error message
#define CK01(result)	\
{	\
 int __r = (result);	\
 if (__r < 0) {	\
  DMCLOG_E(c_CharColor_Red"result=%d < 0\n"c_Print_Ctrl_Off, __r);\
 }	\
}

//if result < 0, goto label
#define CK02(result, label)	\
do{	\
 int __r = (result);	\
 if (__r < 0) {	\
  goto label; \
 }	\
}while(0)


//if result == 0, printf error message, goto label
#define CK10(result, label)	\
{	\
 int __r = (result);	\
 if (__r == 0) {	\
  DMCLOG_E(c_CharColor_Red"result=0\n"c_Print_Ctrl_Off);\
  goto label; \
 }	\
}
//if result == 0, printf error message
#define CK11(result)	\
{	\
 int __r = (result);	\
 if (__r == 0) {	\
  DMCLOG_E(c_CharColor_Red"result=0\n"c_Print_Ctrl_Off);\
 }	\
}

//if result == 0, goto label
#define CK12(result, label)	\
{	\
 int __r = (result);	\
 if (__r == 0) {	\
  goto label; \
 }	\
}


//if result == NULL, printf error message, goto label
#define CK20(result, label)	\
{	\
 void* __r = (result);	\
 if (__r == NULL) {	\
  DMCLOG_E(c_CharColor_Red"result=NULL\n"c_Print_Ctrl_Off);\
  goto label; \
 }	\
}
//if result == NULL, printf error message
#define CK21(result)	\
{	\
 void* __r = (result);	\
 if (__r == NULL) {	\
  DMCLOG_E(c_CharColor_Red"result=NULL \n"c_Print_Ctrl_Off);\
 }	\
}

//if result == NULL, goto label
#define CK22(result, label)	\
{	\
 void* __r = (result);	\
 if (__r == NULL) {	\
  goto label; \
 }	\
}


//if result <=0, printf error message, goto label
#define CK30(result, label)	\
{	\
 int __r = (result);	\
 if (__r <= 0) {	\
  DMCLOG_E(c_CharColor_Red"result=%d <=0 \n"c_Print_Ctrl_Off, __r);\
  goto label; \
 }	\
}
//if result <=0, printf error message
#define CK31(result)	\
{	\
 int __r = (result);	\
 if (__r <= 0) {	\
  DMCLOG_E(c_CharColor_Red"result=%d <=0 \n"c_Print_Ctrl_Off, __r);\
 }	\
}

//if result <= 0, goto label
#define CK32(result, label)	\
{	\
 int __r = (result);	\
 if (__r <= 0) {	\
  goto label; \
 }	\
}
#endif




#endif
