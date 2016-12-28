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

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <stdarg.h>
#include <netdb.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <time.h>
#include <netinet/in.h>
#include "base.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

/******************************************************************************
 *                                 MACRO                                      *
 ******************************************************************************/


/*
 * This is the number of fully connected connections that can be queued
 *  up at the message server socket.
 *
 *  It is highly unlikely that this needs to be changed.
 */
#define APP_MESSAGE_BACKLOG  3

/* MSG lib debug macro. */
#define DM_MSG_LOG_ERR              0x00000001
#define DM_MSG_LOG_WARN             0x00000002
#define DM_MSG_LOG_INFO             0x00000004
#define DM_MSG_LOG_TRACE            0x00000008

/* Default, print the first three levels log */
#define DM_MSG_LOG_FLAG             0x00000007

#define DM_LOG(IMLogFlag, IMLogLevel, fmt, args...) do { \
    if ((IMLogFlag) & (IMLogLevel)) { \
        FILE *fp = fopen("/dev/console", "w"); \
    	if (fp) { \
            fprintf(fp, "[DM_MSG][%s]-%d ", __FUNCTION__, __LINE__); \
    		fprintf(fp, fmt, ## args); \
    		fprintf(fp, "\n"); \
    		fclose(fp); \
    	} \
    } \
} while (0)

/* Some macros in common use. */
#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/** Number of micro-seconds in a milli-second. */
#define USECS_IN_MSEC 1000

/** Number of milliseconds in 1 second */
#define MSECS_IN_SEC  1000

/** Invalid file descriptor number. */
#define MSG_INVALID_FD  (-1)

/** Free a buffer and set the pointer to null. */
#define DM_FREE(p) \
   do { \
      if ((p) != NULL) {free((p)); (p) = NULL;}   \
   } while (0)

#define flags_event        flags.bits.event                         /**< Convenience macro for accessing event bit in msg hdr */
#define flags_request      flags.bits.request                       /**< Convenience macro for accessing request bit in msg hdr */
#define flags_response     flags.bits.response                      /**< Convenience macro for accessing response bit in msg hdr */
#define EMPTY_MSG_HEADER   {{0}, {0}, {0}, {0}, 0, 0, 0, 0, 0}      /**< Initialize msg header to empty */

/* Size definition for APP process information table's member. */
#define APP_NAME_LEN        64
#define MSG_ID_LEN          36

/******************************************************************************
 *                                 TYPEDEF                                    *
 ******************************************************************************/

 /* Integer and bool type definition. */
typedef int 		        SINT32;
typedef unsigned int        UINT32;
typedef short int 	        SINT16;
typedef unsigned short int  UINT16;
typedef char 		        SINT8;
typedef unsigned char       UINT8;
typedef unsigned char       UBOOL8;

/******************************************************************************
 *                                 ENUM                                       *
 ******************************************************************************/

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


/******************************************************************************
 *                                STRUCT                                      *
 ******************************************************************************/
 
/*
 * This header must be at the beginning of every message.
 * The header may then be followed by additional optional data.
 */
typedef struct msg_header
{
   UINT32 dataLength; /**< Amount of data following the header.  0 if no additional data. */
} MsgHeader;

SINT32 DM_InetClientInit(SINT32 iDomain, SINT32 iPort, SINT32 iType,SINT8 *ipaddr);

SINT32 DM_MsgSend(SINT32 iFd, const SINT8 *pBuf, UINT32 nLen);

SINT32 DM_MsgReceive(SINT32 iFd, SINT8 **pBuf, UINT32 *pnTimeOut);

void DM_DomainClientDeinit(SINT32 iFd);

SINT32 DM_UdpReceive(SINT32 iFd, SINT8 **pBuf, UINT32 *pnTimeOut,struct sockaddr_in *addr);
SINT32 DM_UdpServerInit(SINT32 iDomain, SINT32 iPort, SINT32 iType, SINT32 iBackLog) ;
SINT32 DM_UdpSend(SINT32 iFd, const SINT8 *pBuf, UINT32 nLen,struct sockaddr_in *addr );
SINT32 DM_UdpClientInit(SINT32 iDomain, SINT32 iPort, SINT32 iType,SINT8 *ipaddr,struct sockaddr_in *stServerAddr);
SINT32 DM_exit_udp_server(SINT32 sockfd);





#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __MSG_H__ */
