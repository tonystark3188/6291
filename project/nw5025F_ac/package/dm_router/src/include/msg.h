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
#include <sys/inotify.h>//inotify_init inotify_add_watch....

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


/** Number of micro-seconds in a milli-second. */
#define USECS_IN_MSEC 1000

/** Number of milliseconds in 1 second */
#define MSECS_IN_SEC  1000

/** Invalid file descriptor number. */
#define MSG_INVALID_FD  (-1)

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

/******************************************************************************
 *                               FUNCTION                                     *
 ******************************************************************************/

/** Internet socket **/



SINT32 DM_InetClientInit(SINT32 iDomain, SINT32 iPort, SINT32 iType,SINT8 *ipaddr);

/*******************************************************************************
 * Function:
 *    SINT32 DM_InetServerInit(SINT32 iDomain, SINT32 iPort, SINT32 iType, SINT32 iBackLog)
 * Description:
 *    This function creates and initializes a TCP or UDP listening socket
 *    for an application.
 * Parameters:
 *    iDomain  (IN) Specifies whether it is a client-side socket or 
 *                  server-side socket.
 *    iPort    (IN) The application TCP or UDP port.
 *    iType    (IN) The socket type, either SOCK_STREAM or SOCK_DGRAM.
 *    iBackLog (IN) Number of connections to queue. 
 * Returns:
 *    The socket file descriptor
 *******************************************************************************/
SINT32 DM_InetServerInit(SINT32 iDomain, SINT32 iPort, SINT32 iType, SINT32 iBackLog);

/*******************************************************************************
 * Function:
 *    void DM_InetServerDeinit(SINT32 iFd)
 * Description:
 *    This function deinit the socket created for internet server
 * Parameters:
 *    iFd    (IN) The socket file descriptor 
 * Returns:
 *    void
 *******************************************************************************/
void DM_InetServerDeinit(SINT32 iFd);


/*******************************************************************************
 * Function:
 *    void DM_DomainServerDeinit(SINT32 iFd)
 * Description:
 *    This function deinit the socket created for domain(local) server
 * Parameters:
 *    iFd    (IN) The socket file descriptor 
 * Returns:
 *    void
 *******************************************************************************/
void DM_DomainServerDeinit(SINT32 iFd);


/*******************************************************************************
 * Function:
 *    void DM_DomainClientDeinit(SINT32 iFd)
 * Description:
 *    This function deinit the socket created for domain(local) client
 * Parameters:
 *    iFd    (IN) The socket file descriptor 
 * Returns:
 *    void
 *******************************************************************************/
void DM_DomainClientDeinit(SINT32 iFd);

SINT32 DM_ServerAcceptClient(SINT32 iListenFd);

/*******************************************************************************
 * Function:
 *    SINT32 DM_ServerAcceptGetIp(SINT32 iListenFd,char *clientIp);
 * Description:
 *    Before server recieve or send message,server should accept a client,then get client ip for session
 * Parameters:
 *    iFd    (IN) The listening socket file descriptor 
 *    clientIp (OUT) the client ip
 * Returns:
 *    The accept socket file descriptor
 *******************************************************************************/
SINT32 DM_ServerAcceptGetIp(SINT32 iListenFd,char *clientIp);


/*******************************************************************************
 * Function:
 *    SINT32 DM_MsgSend(SINT32 iFd, const SINT8 *pBuf, UINT32 nLen)
 * Description:
 *    Use this function,client send message to server,
 *    or server send response message to client
 * Parameters:
 *    iFd    (IN) The socket file descriptor used to send message
 *    pBuf   (IN) Message for sending 
 *    nLen   (IN) Message length
 * Returns:
 *    The sending result
 *    0:success;others,error
 *******************************************************************************/
SINT32 DM_MsgSend(SINT32 iFd, const SINT8 *pBuf, UINT32 nLen);

/*******************************************************************************
 * Function:
 *    SINT32 DM_MsgSend_Try(SINT32 iFd, const SINT8 *pBuf, UINT32 nLen)
 * Description:
 *    Use this function,client send message to server,
 *    or server send response message to client
 * Parameters:
 *    iFd    (IN) The socket file descriptor used to send message
 *    pBuf   (IN) Message for sending
 *    nLen   (IN) Message length
 * Returns:
 *    The sending result
 *    0:success;others,error
 *******************************************************************************/
SINT32 DM_MsgSend_Try(SINT32 iFd, const SINT8 *pBuf, UINT32 nLen);

/*******************************************************************************
 * Function:
 *    SINT32 DM_MsgReceive(SINT32 iFd, SINT8 **pBuf, UINT32 *pnTimeOut)
 * Description:
 *    Use this function,server recieve message from client,
 *    or client recieve response message from server
 * Parameters:
 *    iFd         (IN)  The socket file descriptor used to recieve message
 *    pBuf        (OUT) The recieve message buf 
 *    pnTimeOut   (IN)  Timeout for recieving
 * Returns:
 *    When > 0,the recieved message bytes;or not,recieve error
 *******************************************************************************/
SINT32 DM_MsgReceive(SINT32 iFd, SINT8 **pBuf, UINT32 *pnTimeOut);

/*******************************************************************************
 * Function:
 *    void DM_TimeToString(time_t stWhen, SINT8 *pBuffer, UINT32 nSize)
 * Description:
 *    convert time to string
 * Parameters:
 *    stWhen      (IN)  The time
 *    pBuffer     (OUT) String buffer 
 *    nSize       (IN)  Size of string buffer
 * Returns:
 *    void
 *******************************************************************************/
void DM_TimeToString(time_t stWhen, SINT8 *pBuffer, UINT32 nSize);

/*******************************************************************************
 * Function:
 *    void DM_MsgPrintf(const void *pBuf, SINT8 *pszDscrptn, UINT32 nLen, SINT32 iFlag)
 * Description:
 *    printf message recieved or for sending
 * Parameters:
 *    pBuf        (IN)  Message buf
 *    pszDscrptn  (IN)  Message description 
 *    nLen        (IN)  Message length
 *    iFlag       (IN)  Message flag:1,RX;2:TX;other,unkown
 * Returns:
 *    void
 *******************************************************************************/
void DM_MsgPrintf(const void *pBuf, SINT8 *pszDscrptn, UINT32 nLen, SINT32 iFlag);
/*******************************************************************************
 * Function:
 *    DM_exit_tcp_server(SINT32 port)
 * Description:
 *    exit tcp server of long connection
 * Parameters:
 *    port        (IN)  port
 * Returns:
 *    void
 *******************************************************************************/

SINT32 DM_exit_tcp_server(SINT32 port);
SINT32 DM_UdpReceive(SINT32 iFd, SINT8 **pBuf, UINT32 *pnTimeOut,struct sockaddr_in *addr);
SINT32 DM_UdpServerInit(SINT32 iDomain, SINT32 iPort, SINT32 iType, SINT32 iBackLog) ;
SINT32 DM_UdpSend(SINT32 iFd, const SINT8 *pBuf, UINT32 nLen,struct sockaddr_in *addr );
SINT32 DM_UdpClientInit(SINT32 iDomain, SINT32 iPort, SINT32 iType,SINT8 *ipaddr,struct sockaddr_in *stServerAddr);
SINT32 DM_exit_udp_server(SINT32 sockfd);


int DM_InotifyInit();
int DM_InotifyAddWatch(int fd,const char *inotifyPath);
void DM_InotifyRmWatch(int fd,int wd);
void DM_InotifyClose(int fd);




#ifdef __cplusplus
#if __cplusplus

}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __MSG_H__ */
