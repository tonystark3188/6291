/************************************************************************
#
#  Copyright (c) 2014-2016  I-MOVE(SHENTHEN) Co., Ltd.
#  All Rights Reserved
#
#  author: Oliver
#  create date: 2015-3-17
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
#  0.1.0.1    |oliver       |2015-3-17     |Trial Version
#
*************************************************************************/

#ifndef __MSG_H__
#define __MSG_H__

/******************************************************************************
 *                               INCLUDES                                     *
 ******************************************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

 /* Integer and bool type definition. */
typedef int 		        SINT32;
typedef unsigned int        UINT32;
typedef short int 	        SINT16;
typedef unsigned short int  UINT16;
typedef char 		        SINT8;
typedef unsigned char       UINT8;
typedef unsigned char       UBOOL8;


/* MSG lib return value definition. */
typedef enum
{
	MSGRET_SUCCESS              = 0,
	MSGRET_RESOURCE_EXCEEDED    = -9001,
	MSGRET_INTERNAL_ERROR       = -9002,
	MSGRET_DISCONNECTED         = -9003,
	MSGRET_TIMED_OUT            = -9004,
	MSGRET_INVALID_ARGUMENTS    = -9005,
}MsgRet;


/** Number of micro-seconds in a milli-second. */
#define USECS_IN_MSEC 1000

/** Number of milliseconds in 1 second */
#define MSECS_IN_SEC  1000

/** Invalid file descriptor number. */
#define MSG_INVALID_FD  (-1)

typedef struct msg_header
{
    UINT32 dataLength; /**< Amount of data following the header.  0 if no additional data. */
} MsgHeader;


/******************************************************************************
 *                                 TYPEDEF                                    *
 ******************************************************************************/


SINT32 DM_UART_InetServerInit(SINT32 iDomain, SINT32 iPort, SINT32 iType, SINT32 iBackLog);

void DM_UART_InetServerDeinit(SINT32 iFd);

SINT32 DM_UART_ServerAcceptClient(SINT32 iListenFd);

void DM_UART_ServerAcceptClientClose(SINT32 iFd);

SINT32 DM_UART_InetClientInit(SINT32 iDomain, SINT32 iPort, SINT32 iType,SINT8 *ipaddr);

void DM_UART_InetClientDeinit(SINT32 iFd);

SINT32 DM_UART_MsgSend(SINT32 iFd, const SINT8 *pBuf, UINT32 nLen);

SINT32 DM_UART_MsgReceive(SINT32 iFd, SINT8 **pBuf, UINT32 pnTimeOut);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __MSG_H__ */
