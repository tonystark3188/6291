
/************************************************************************
#
#  Copyright (c) 2015-2016 longsys(SHENTHEN) Co., Ltd.
#  All Rights Reserved
#
#  author: Oliver
#  create date: 2015-3-19
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
# Not withstanding the above, under no circumstance
s may you combine 
# this software in any way with any other longsys software provided 
# under a license other than the GPL, without longsys's express prior 
# written consent. 
#
# Revision Table
#
# Version     | Name             |Date           |Description
# ------------|------------------|---------------|-------------------
#  0.1.0.1    |Oliver       |2015-3-19    |Trial Version
#
*************************************************************************/
#ifndef __API_PROCESS_H
#define __API_PROCESS_H


#ifdef __cplusplus
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
#include <asm/types.h>      
#include <linux/netlink.h>      
#include <linux/socket.h>      
#include <stddef.h>   
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/syscall.h>

#include "base.h"

#include "process_json.h"

  
#define FN_GET_ROUTE_IP     1024
#define FM_GET_ROUTE_DETAIL 1025
#define FM_GET_ROUTE_BEAT   1027



int _dm_get_route_ip(ClientTheadInfo *p_client_info);
int _dm_get_route_detail(ClientTheadInfo *p_client_info);
int _dm_get_route_beat(ClientTheadInfo *p_client_info);
int _dm_notify_del_ip(ClientTheadInfo *p_client_info);




/*############################## Enums ######################################*/

/*############################## Structs #####################################*/



/*############################## Macros ######################################*/


int api_process(ClientTheadInfo *p_client_info);
int api_response(ClientTheadInfo *p_client_info);



/*############################## Prototypes ##################################*/

#ifdef __cplusplus
}
#endif

#endif 

