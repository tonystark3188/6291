/************************************************************************
#
#  Copyright (c) 2014-2016 LONGSYS(SHENTHEN) Co., Ltd.
#  All Rights Reserved
#
#  author: Oliver
#  create date: 2015-6-18
# 
# Unless you and longsys execute a separate written software license 
# agreement governing use of this software, this software is licensed 
# to you under the terms of the GNU General Public License version 2 
# (the "GPL"), with the following added to such license:
# 
#    As a special exception, the copyright holders of this software give 
#    you permission to link this software with independent modules, and 
#    to copy and distribute the resulting executable under terms of your 
#    choice, provided that you also meet, for each linked independent 
#    module, the terms and conditions of the license of that module. 
#    An independent module is a module which is not derived from this
#    software.  The special exception does not apply to any modifications 
#    of the software.  
# 
# Not withstanding the above, under no circumstances may you combine 
# this software in any way with any other longsys software provided 
# under a license other than the GPL, without longsys's express prior 
# written consent. 
#
# Revision Table
#
# Version     | Name             |Date           |Description
# ------------|------------------|---------------|-------------------
#  0.1.0.1    |Oliver       |2015-6-18     |Trial Version
#
*************************************************************************/

#ifndef __MY_DEBUG_H__
#define __MY_DEBUG_H__

#if __cplusplus
extern "C" {
#endif

/*############################## Includes ####################################*/
#include <sys/stat.h>      
#include <unistd.h>      
#include <stdio.h>      
#include <stdlib.h>      
#include <sys/socket.h>      
#include <sys/types.h>      
#include <string.h>             
#include <stddef.h>   
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/syscall.h>


/*############################## Macros ######################################*/

  
#define MONITOR_MODULE       "dm_monitor"

/*############################## Enums ######################################*/

/*############################## Structs #####################################*/

/*############################## Prototypes ##################################*/

//#define DEBUG
#ifdef DEBUG
#define p_debug(args...) do{\
        struct timeval tnow;\
        FILE *fp = fopen("/dev/console","w");\
        if(fp){\
                gettimeofday(&tnow, NULL);\
                fprintf(fp,"[DM_MONITOR][%08d.%06d][%s][-%d] ", (unsigned int)tnow.tv_sec, (unsigned int)tnow.tv_usec, __FUNCTION__, __LINE__);\
                fprintf(fp,##args);\
                fprintf(fp,"\n");\
                fclose(fp);\
        }\
}while(0)
#else
#define p_debug(...)  
#endif


#if __cplusplus
}
#endif

#endif /* __MSG_CLIENT_H__ */


