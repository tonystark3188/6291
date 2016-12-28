/************************************************************************
#
#  Copyright (c) 2015-2016 longsys(SHENTHEN) Co., Ltd.
#  All Rights Reserved
#
#  author:Oliver
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
#
*************************************************************************/
#ifdef __cplusplus
#if __cplusplus
    extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

/******************************************************************************
 *                               INCLUDES                                     *
 ******************************************************************************/
#include <unistd.h> 
#include <stdio.h> 
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>             
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>           
#include <sys/stat.h>
#include <signal.h>
#include <string.h> 
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include "msg.h"
#include "dmclog.h"
//#include "dm_udp_client.h"


/******************************************************************************
 *                         PRIVATE FUNCTIONS                                  *
 ******************************************************************************/

static MsgRet DM_WaitDataAvailable(SINT32 iFd, UINT32 nTimeOut);


/******************************************************************************
 *                               FUNCTIONS                                    *
 ******************************************************************************/

/*******************************************************************************
 * Function:
 *    static MsgRet DM_WaitDataAvailable(SINT32 iFd, UINT32 nTimeOut)
 * Description:
 *    Wait for available data until timeout
 * Parameters:
 *    iFd      (IN) socket file descriptor 
 *    nTimeOut (IN) Time for timeout
 * Returns:
 *    0:success;others,error
 *******************************************************************************/
static MsgRet DM_WaitDataAvailable(SINT32 iFd, UINT32 nTimeOut)
{
    struct timeval stTv;
    fd_set fsReadFds;
	
    SINT32 iRc;

    FD_ZERO(&fsReadFds);
    FD_SET(iFd, &fsReadFds);

	if(nTimeOut > 0)
	{
		stTv.tv_sec = nTimeOut / MSECS_IN_SEC;
		stTv.tv_usec = (nTimeOut % MSECS_IN_SEC) * USECS_IN_MSEC;
		iRc = select(iFd+1, &fsReadFds, NULL, NULL, &stTv);
	}else{
		iRc = select(iFd+1, &fsReadFds, NULL, NULL, NULL);
	}
   
    if ((iRc == 1) && (FD_ISSET(iFd, &fsReadFds)))
    {
        return MSGRET_SUCCESS;
    }
    else
    {
        return MSGRET_TIMED_OUT;
    }
}

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
SINT32 DM_InetServerInit(SINT32 iDomain, SINT32 iPort, SINT32 iType, SINT32 iBackLog) 
{
    SINT32 iFd;
    SINT32 iOptVal;

    /* Create a TCP or UDP based socket */
    if ((iFd = socket(iDomain, iType, 0)) < 0)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "socket errno=%d port=%d reason:%s", errno, iPort, strerror(errno));
        return MSG_INVALID_FD;
    }

    /* Set socket options */
    iOptVal = 1;
    if (setsockopt(iFd, SOL_SOCKET, SO_REUSEADDR, &iOptVal, sizeof(iOptVal)) < 0)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "setsockopt errno=%d port=%d fd=%d reason:%s", 
            errno, iPort, iFd, strerror(errno));
        close(iFd);
        return MSG_INVALID_FD;
    }

    /* Set up the local address */
    if (iDomain == AF_INET)
    {
        struct sockaddr_in stServerAddr;

        if (iType == SOCK_DGRAM)
        {
            /* set option for getting the to ip address. */
            if (setsockopt(iFd, IPPROTO_IP, IP_PKTINFO, &iOptVal, sizeof(iOptVal)) < 0)
            {
                DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "setsockopt errno=%d port=%d fd=%d reason:%s", 
                    errno, iPort, iFd, strerror(errno));
                close(iFd);
                return MSG_INVALID_FD;
            }
        }

        memset(&stServerAddr, 0, sizeof(stServerAddr));
        stServerAddr.sin_family = AF_INET;
        stServerAddr.sin_port   = htons(iPort);
        stServerAddr.sin_addr.s_addr  = htonl(INADDR_ANY);

        /* Bind socket to local address */
        if (bind(iFd, (struct sockaddr *)&stServerAddr, sizeof(stServerAddr)) < 0)
        {
            DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "bind errno=%d port=%d fd=%d reason:%s", 
                errno, iPort, iFd, strerror(errno));
            close(iFd);
            return MSG_INVALID_FD;
        }
    }
    else
    {
        struct sockaddr_in6 stServerAddr;

        if (iType == SOCK_DGRAM)
        {
            /* set option for getting the to ip address. */
#ifdef IPV6_RECVPKTINFO
            if (setsockopt(iFd, IPPROTO_IPV6, IPV6_RECVPKTINFO, &iOptVal, sizeof(iOptVal)) < 0)
#else
            if (setsockopt(iFd, IPPROTO_IPV6, IPV6_PKTINFO, &iOptVal, sizeof(iOptVal)) < 0)
#endif
            {
                DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "setsockopt errno=%d port=%d fd=%d reason:%s", 
                    errno, iPort, iFd, strerror(errno));
                close(iFd);
                return MSG_INVALID_FD;
            }
        }

        memset(&stServerAddr, 0, sizeof(stServerAddr));
        stServerAddr.sin6_family = AF_INET6;
        stServerAddr.sin6_port   = htons(iPort);
        stServerAddr.sin6_addr   = in6addr_any;

        /* Bind socket to local address */
        if (bind(iFd, (struct sockaddr *)&stServerAddr, sizeof(stServerAddr)) < 0)
        {
            DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "bind errno=%d port=%d fd=%d reason:%s", 
                errno, iPort, iFd, strerror(errno));
            close(iFd);
            return MSG_INVALID_FD;
        }
    }

    if (iType == SOCK_STREAM)
    {
        /* Enable connection to SOCK_STREAM socket */
        if (listen(iFd, iBackLog) < 0)
        {
            DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "listen errno=%d port=%d fd=%d reason:%s", 
                errno, iPort, iFd, strerror(errno));
            close(iFd);
            return MSG_INVALID_FD;
        }
    }
    return (iFd);
}


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
void DM_InetServerDeinit(SINT32 iFd)
{
   if (MSG_INVALID_FD != iFd)
   {
      close(iFd);
   }
}
/*
 * Desc: exit tcp server.
 *
 */
int DM_exit_tcp_server(SINT32 port)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		return -1;
	}
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	sleep(3);
	close(sockfd);
	return 0;
}

/*******************************************************************************
 * Function:
 *    SINT32 DM_DomainServerInit(const SINT8 *pName)
 * Description:
 *    This function creates a socket, binds server with path and listen
 * Parameters:
 *    pName   (IN) Name of process used as a server 
 * Returns:
 *    The socket file descriptor
 *******************************************************************************/
SINT32 DM_DomainServerInit(const SINT8 *pName)
{
    struct sockaddr_un stServerAddr;
    SINT32 iFd, iRc;
    SINT8 szTmp[128] = {0};

    if (NULL == pName)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "APP's name is null!");
        return -1;
    }
    
    if (sizeof(pName) + sizeof(APP_MESSAGE_PATH) - 1 > sizeof(stServerAddr.sun_path))
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "Socket path is too long to init!");
        return -1;
    }

    snprintf(szTmp, sizeof(szTmp) - 1, "rm -fr "APP_MESSAGE_PATH"%s", pName);
    system(szTmp);

    memset(szTmp, 0, sizeof(szTmp));
    snprintf(szTmp, sizeof(szTmp) - 1, "mkdir -p "APP_MESSAGE_PATH);
    system(szTmp);

    if ((iFd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "Could not create socket");
        return -1;
    }
    
    /*
    * Bind my server address and listen.
    */
    memset(&stServerAddr, 0, sizeof(stServerAddr));
    stServerAddr.sun_family = AF_LOCAL;
    snprintf(stServerAddr.sun_path, sizeof(stServerAddr.sun_path), APP_MESSAGE_PATH"%s", pName);
    
    iRc = bind(iFd, (struct sockaddr *) &stServerAddr, sizeof(stServerAddr));
    if (iRc != 0)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "bind to %s failed, rc=%d errno=%d reason:%s", 
            stServerAddr.sun_path, iRc, errno, strerror(errno));
        close(iFd);
        return -1;
    }

    iRc = listen(iFd, APP_MESSAGE_BACKLOG);
    if (iRc != 0)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "listen to %s failed, rc=%d errno=%d reason:%s", 
            stServerAddr.sun_path, iRc, errno, strerror(errno));
        close(iFd);
        return -1;
    }

    DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_INFO, "%s msg socket opened and ready (fd=%d)", 
        stServerAddr.sun_path, iFd);

    return iFd;
}



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
void DM_DomainServerDeinit(SINT32 iFd)
{
   if (MSG_INVALID_FD != iFd)
   {
      close(iFd);
   }
}

SINT32 DM_InetClientInit(SINT32 iDomain, SINT32 iPort, SINT32 iType,SINT8 *ipaddr)
{
    SINT32 iFd, iRc;
    struct sockaddr_in stServerAddr;
    /*
     * Create a unix domain socket.
     */
    iFd = socket(iDomain, iType, 0);
    if (iFd < 0)
    {
        DMCLOG_D("Could not create socket");
        return -1;
    }
    /*
     * Set close-on-exec, even though all apps should close their
     * fd's before fork and exec.
     */
    if ((iRc = fcntl(iFd, F_SETFD, FD_CLOEXEC)) != 0)
    {
        DMCLOG_D( "set close-on-exec failed, rc=%d errno=%d reason:%s",
               iRc, errno, strerror(errno));
        close(iFd);
        return -1;
    }
//    int val = fcntl(iFd,F_GETFL);
//    if ((iRc = fcntl(iFd,F_SETFL,val | O_NONBLOCK)) != 0)
//    {
//        DMCLOG_D( "set close-on-exec failed, rc=%d errno=%d reason:%s",
//                iRc, errno, strerror(errno));
//        close(iFd);
//        return -1;
//    }
    /*
     * Connect to server.
     */
    bzero(&stServerAddr, sizeof(stServerAddr));
    stServerAddr.sin_family = AF_INET;
    stServerAddr.sin_port = htons(iPort);
    memset(&(stServerAddr.sin_zero), 0, 8);
    if (inet_aton(ipaddr, (struct in_addr*)&stServerAddr.sin_addr.s_addr) == 0)
    {
        perror(ipaddr);
        return -1;
    }
    iRc = connect(iFd, (struct sockaddr *) &stServerAddr, sizeof(stServerAddr));
    if (iRc != 0)
    {
        close(iFd);
        return -1;
    }
    else
    {
        DMCLOG_D("commFd=%d connected to "APP_MESSAGE_PATH,
               iFd);
    }
    return iFd;
}


/*******************************************************************************
 * Function:
 *    SINT32 DM_DomainClientInit(const SINT8 *pServerName)
 * Description:
 *    This function creates a socket, connects to server
 * Parameters:
 *    pServerName   (IN) Name of server process 
 * Returns:
 *    The socket file descriptor
 *******************************************************************************/
SINT32 DM_DomainClientInit(const SINT8 *pServerName)
{
    SINT32 iFd, iRc;
    struct sockaddr_un stServerAddr;

    if (NULL == pServerName)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "Server APP's name is null!");
        return -1;
    }

    if (sizeof(pServerName) + sizeof(APP_MESSAGE_PATH) - 1 > sizeof(stServerAddr.sun_path))
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "Socket path is too long to init!");
        return -1;
    }

    /*
    * Create a unix domain socket.
    */
    iFd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (iFd < 0)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "Could not create socket");
        return -1;
    }

    /*
    * Set close-on-exec, even though all apps should close their
    * fd's before fork and exec.
    */
    if ((iRc = fcntl(iFd, F_SETFD, FD_CLOEXEC)) != 0)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "set close-on-exec failed, rc=%d errno=%d reason:%s", 
            iRc, errno, strerror(errno));
        close(iFd);
        return -1;
    }

    /*
    * Connect to server.
    */
    memset(&stServerAddr, 0, sizeof(stServerAddr));
    stServerAddr.sun_family = AF_LOCAL;
    snprintf(stServerAddr.sun_path, sizeof(stServerAddr.sun_path), APP_MESSAGE_PATH"%s", pServerName);

    iRc = connect(iFd, (struct sockaddr *) &stServerAddr, sizeof(stServerAddr));
    if (iRc != 0)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "connect to "APP_MESSAGE_PATH"%s failed, rc=%d errno=%d reason:%s", 
            pServerName, iRc, errno, strerror(errno));
        close(iFd);
        return -1;
    }
    else
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_INFO, "commFd=%d connected to "APP_MESSAGE_PATH"%s", 
            iFd, pServerName);
    }

    return iFd;
}

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
void DM_DomainClientDeinit(SINT32 iFd)
{
   if (MSG_INVALID_FD != iFd)
   {
      close(iFd);
   }
}

/*******************************************************************************
 * Function:
 *    SINT32 DM_ServerAcceptClient(SINT32 iListenFd)
 * Description:
 *    Before server recieve or send message,server should accept a client
 * Parameters:
 *    iFd    (IN) The listening socket file descriptor 
 * Returns:
 *    The accept socket file descriptor
 *******************************************************************************/
SINT32 DM_ServerAcceptClient(SINT32 iListenFd)
{
    struct sockaddr_un stClientAddr;
    UINT32 nSockAddrSize;
    SINT32 iAcceptFd;

    nSockAddrSize = sizeof(stClientAddr);
    if ((iAcceptFd = accept(iListenFd, (struct sockaddr *)&stClientAddr, &nSockAddrSize)) < 0)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "accept IPC connection failed. errno=%d reason:%s", 
            errno, strerror(errno));
        return -1;
    }

    return iAcceptFd;
}

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
SINT32 DM_MsgSend(SINT32 iFd, const SINT8 *pBuf, UINT32 nLen)
{
    UINT32 nTotalLen;
    SINT32 iRc;
    MsgRet enRet = MSGRET_SUCCESS;
    MsgHeader *pstMsg;

    if (NULL == pBuf)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "Send buffer is null!");
        return MSGRET_INVALID_ARGUMENTS;
    }

    pstMsg = (MsgHeader *)malloc(sizeof(MsgHeader));
    if (pstMsg == NULL)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "malloc of msg header failed");
        return MSGRET_RESOURCE_EXCEEDED;
    }
    memset(pstMsg, 0, sizeof(MsgHeader));
    pstMsg->dataLength = nLen;

    pstMsg = (MsgHeader *)realloc(pstMsg, sizeof(MsgHeader) + nLen);
    if (pstMsg == NULL)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "realloc to %d bytes failed", 
            sizeof(MsgHeader) + nLen);
        free(pstMsg);
        return MSGRET_RESOURCE_EXCEEDED;
    }
    memcpy((SINT8 *)(pstMsg + 1), pBuf, nLen);

    nTotalLen = sizeof(MsgHeader) + nLen;
	p_debug("nTotalLen = %d",nTotalLen);
    iRc = write(iFd, (SINT8 *)pstMsg, nTotalLen);
    if (iRc < 0)
    {
        if (errno == EPIPE)
        {
            /*
             * This could happen when tries to write to an app that
             * has exited.  Don't print out a scary error message.
             * Just return an error code and let upper layer app handle it.
             */
            DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "got EPIPE, dest app is dead, errno:%d reason:%s", 
                errno, strerror(errno));
            free(pstMsg);
            return MSGRET_DISCONNECTED;
        }
        else
        {
            DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "write failed, errno=%d reason:%s", 
                errno, strerror(errno));
            free(pstMsg);
            enRet = MSGRET_INTERNAL_ERROR;
        }
    }
    else if (iRc != (SINT32) nTotalLen)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "unexpected rc %d, expected %u", 
            iRc, nTotalLen);
        free(pstMsg);
        enRet = MSGRET_INTERNAL_ERROR;
    }

    free(pstMsg);
    return enRet;
}

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
SINT32 DM_MsgReceive(SINT32 iFd, SINT8 **pBuf, UINT32 *pnTimeOut)
{
    MsgHeader *pstMsg;
    SINT32 iRc;
    MsgRet enRet;
    UINT32 nTotalRemaining;
    UINT32 nTotalReadSoFar = 0;
    SINT8 *pInBuf = NULL;

    if(pBuf == NULL)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "buf is NULL!");
        return MSGRET_INVALID_ARGUMENTS;
    }
    else
    {
        *pBuf = NULL;
    }

    if (pnTimeOut)
    {
    	p_debug("waiting1");
        if ((enRet = DM_WaitDataAvailable(iFd, *pnTimeOut)) != MSGRET_SUCCESS)
        {
            return enRet;
        }
		p_debug("waiting2");
    }

    /*
     * Read just the header in the first read.
     * Do not try to read more because we might get part of 
     * another message in the TCP socket.
     */
    pstMsg = (MsgHeader *)malloc(sizeof(MsgHeader));
    if (pstMsg == NULL)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "alloc of msg header failed");
        return MSGRET_RESOURCE_EXCEEDED;
    }
    memset(pstMsg, 0, sizeof(MsgHeader));

    iRc = read(iFd, pstMsg, sizeof(MsgHeader));
    if ((iRc == 0) ||
       ((iRc == -1) && (errno == 131)))  /* new 2.6.21 kernel seems to give us this before rc==0 */
    {
        /* broken connection */
        free(pstMsg);
        return MSGRET_DISCONNECTED;
    }
    else if (iRc < 0 || iRc != sizeof(MsgHeader))
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "bad read, rc=%d errno=%d reason:%s", 
            iRc, errno, strerror(errno));
        free(pstMsg);
        return MSGRET_INTERNAL_ERROR;
    }

    /* get additional data size and free msg head */
    nTotalRemaining = pstMsg->dataLength +1;
	p_debug("nTotalRemaining = %d",nTotalRemaining);
    if (nTotalRemaining > 0)
    {
        /* there is additional data in the message */
        pInBuf = (SINT8 *)calloc(1, nTotalRemaining);
        if (pInBuf == NULL)
        {
            DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "malloc to %d bytes failed", 
                nTotalRemaining);
            free(pstMsg);
            free(pInBuf);
            return MSGRET_RESOURCE_EXCEEDED;
        }
        while (nTotalReadSoFar < nTotalRemaining)
        {
            //DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_INFO, "reading segment: soFar=%d total=%d", 
                //nTotalReadSoFar, nTotalRemaining);
            if (pnTimeOut)
            {
                if ((enRet = DM_WaitDataAvailable(iFd, *pnTimeOut)) != MSGRET_SUCCESS)
                {
                    free(pstMsg);
                    free(pInBuf-nTotalReadSoFar);
                    return enRet;
                }
            }

            iRc = read(iFd, pInBuf, nTotalRemaining);
            if (iRc <= 0)
            {
                DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "bad data read, rc=%d errno=%d readSoFar=%d remaining=%d reason:%s", 
                    iRc, errno, nTotalReadSoFar, nTotalRemaining, strerror(errno));
                free(pstMsg);
                free(pInBuf);
                return MSGRET_INTERNAL_ERROR;
            }
            else
            {
                pInBuf += iRc;
                nTotalReadSoFar += iRc;
                nTotalRemaining -= iRc;
            }
        }
    }
	p_debug("pstMsg->dataLength = %d",pstMsg->dataLength);
    *pBuf = pInBuf - nTotalReadSoFar;
	free(pstMsg);
    return nTotalReadSoFar;
}

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
void DM_TimeToString(time_t stWhen, SINT8 *pBuffer, UINT32 nSize)
{
    struct tm stWhenTm;

    localtime_r(&stWhen, &stWhenTm);
    snprintf(pBuffer, nSize - 1, "%04d/%02d/%02d %02d:%02d:%02d",
        1900 + stWhenTm.tm_year,
        1 + stWhenTm.tm_mon,
        stWhenTm.tm_mday,
        stWhenTm.tm_hour,
        stWhenTm.tm_min,
        stWhenTm.tm_sec);
}

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
void DM_MsgPrintf(const void *pBuf, SINT8 *pszDscrptn, UINT32 nLen, SINT32 iFlag)
{
    const UINT32 nLineWidth = 16;
    SINT32 i = 0, j = 0;
    SINT32 iTail = 0;
    const UINT8 *pucBuf = NULL;
    SINT8 szFlagStr[8] = {0};
    SINT8 szTimeDscrptn[32] = {0};

    if (nLen == 0 || NULL == pBuf)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "message is null or length(%d) is 0", 
            nLen);
        return;
    }

    if (iFlag == 1)
    {
        snprintf(szFlagStr, sizeof(szFlagStr) - 1, "%s", "Rx");
    }
    else if (iFlag == 2)
    {
        snprintf(szFlagStr, sizeof(szFlagStr) - 1, "%s", "Tx");
    }
    else
    {
        snprintf(szFlagStr, sizeof(szFlagStr) - 1, "%s", "unkown");
    }
    
    DM_TimeToString(time(NULL), szTimeDscrptn, sizeof(szTimeDscrptn));
    printf("[%s] %s %s[len:%d] msg:\n", szTimeDscrptn, szFlagStr, pszDscrptn, nLen);
    printf("------------------------------------------------------------------\n");

    pucBuf = pBuf;

    for (; i < nLen; i++)
    {
        if ((i > 0) && (i % nLineWidth == 0))
        {
            j = i - nLineWidth;
            printf("; ");
            for (; j < i; j++)
            {
                if (pucBuf[j] < nLineWidth)
                    printf("%c", '.');
                else
                    printf("%c", pucBuf[j]);
            }
            printf("\x0a\x0d");
        }
        printf("%02X ", pucBuf[i]);
    }

    iTail = nLen % nLineWidth == 0 ? nLen - nLineWidth : (nLen / nLineWidth) * nLineWidth;
    if (iTail != nLen - nLineWidth)
    {
        for (i = 0; i < 48 - (nLen - iTail) * 3; i++)
        {
            printf("%c", ' ');
        }
    }

    printf("; ");

    for (i = iTail; i < nLen; i++)
    {
        if (pucBuf[i] < nLineWidth)
            printf("%c", '.');
        else
            printf("%c", pucBuf[i]);
    }

    printf("\x0a\x0d");
    printf("------------------------------------------------------------------\n\n");
}

/*******************************************************************************
 * Function:
 *    SINT32 _DM_UdpSend(SINT32 iFd, const SINT8 *pBuf, UINT32 nLen)
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
SINT32 DM_UdpSend(SINT32 iFd, const SINT8 *pBuf, UINT32 nLen,struct sockaddr_in *addr )
{
    UINT32 nTotalLen;
    SINT32 iRc;
    MsgRet enRet = MSGRET_SUCCESS;
    MsgHeader *pstMsg;
	SINT8 len;
	SINT8 send_len = 512;
	len = sizeof(struct sockaddr_in);
    if (NULL == pBuf)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "Send buffer is null!");
        return MSGRET_INVALID_ARGUMENTS;
    }

    pstMsg = (MsgHeader *)malloc(sizeof(MsgHeader));
    if (pstMsg == NULL)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "malloc of msg header failed");
        return MSGRET_RESOURCE_EXCEEDED;
    }
    memset(pstMsg, 0, sizeof(MsgHeader));
    pstMsg->dataLength = nLen;

    pstMsg = (MsgHeader *)realloc(pstMsg, sizeof(MsgHeader) + nLen);
    if (pstMsg == NULL)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "realloc to %d bytes failed", 
            sizeof(MsgHeader) + nLen);
        free(pstMsg);
        return MSGRET_RESOURCE_EXCEEDED;
    }
    memcpy((SINT8 *)(pstMsg + 1), pBuf, nLen);
    nTotalLen = sizeof(MsgHeader) + nLen;
	p_debug("nTotalLen = %d, len = %d, pstMsg = %s, pstMsg+1 = %s",nTotalLen , len, pstMsg, (pstMsg+1) );
    //iRc = write(iFd, (SINT8 *)pstMsg, nTotalLen);
	iRc = sendto(iFd, (SINT8 *)pstMsg, nTotalLen, 0, (struct sockaddr*)addr, len);
	//iRc = sendto(iFd, (SINT8 *)pstMsg, send_len, 0, (struct sockaddr*)addr, len);
    if (iRc < 0)
    {
            p_debug(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "write failed, errno=%d reason:%s", 
                errno, strerror(errno));
            free(pstMsg);
            enRet = MSGRET_INTERNAL_ERROR;
			return enRet;
    }
    free(pstMsg);
    return enRet;
}
SINT32 DM_UdpReceive(SINT32 iFd, SINT8 **pBuf, UINT32 *pnTimeOut,struct sockaddr_in *addr)
{
	p_debug("DM_UdpReceive>>>>>>>>>>");
    MsgHeader *pstMsg;
    SINT32 iRc;
    MsgRet enRet;
    UINT32 nTotalRemaining;
    UINT32 nTotalReadSoFar = 0;
    SINT8 *pInBuf = NULL;
	SINT8 len;
	UINT32 buffer_len = 512;

    if(pBuf == NULL)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "buf is NULL!");
        return MSGRET_INVALID_ARGUMENTS;
    }
    else
    {
        *pBuf = NULL;
    }
#if 0
    if (pnTimeOut)
    {
        if ((enRet = DM_WaitDataAvailable(iFd, *pnTimeOut)) != MSGRET_SUCCESS)
        {
            return enRet;
        }
    }
#endif
    /*
     * Read just the header in the first read.
     * Do not try to read more because we might get part of 
     * another message in the UDP socket.
     */
    len = sizeof(struct sockaddr_in);
    //pstMsg = (MsgHeader *)malloc(sizeof(MsgHeader));
    pstMsg = (MsgHeader *)malloc(buffer_len);
    if (pstMsg == NULL)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "alloc of msg header failed");
        return MSGRET_RESOURCE_EXCEEDED;
    }
    //memset(pstMsg, 0, sizeof(MsgHeader));
	memset(pstMsg, 0, buffer_len);

	//iRc = recvfrom(iFd, pstMsg, sizeof(pstMsg), 0, (struct sockaddr*)addr, &len); 
	iRc = recvfrom(iFd, pstMsg, buffer_len, 0, (struct sockaddr*)addr, &len); 
p_debug("Received a string from client iRc = %d, sizeof(pstMsg) = %d, sizeof(MsgHeader) = %d, string is: %s\n", 
				iRc, sizeof(pstMsg), sizeof(MsgHeader), pstMsg+1);

	if ((iRc == 0) || ((iRc == -1) && (errno == 131)))  /* new 2.6.21 kernel seems to give us this before rc==0 */
    {
        /* broken connection */

        free(pstMsg);
        return MSGRET_DISCONNECTED;
    }


	*pBuf = pstMsg + 1;
	return iRc;
}

int DM_exit_udp_server(SINT32 sockfd)
{
	close(sockfd);
	return 0;
}

/*******************************************************************************
 * Function:
 *    SINT32 DM_UdpClientInit(SINT32 iDomain, SINT32 iPort, SINT32 iType,SINT8 *ipaddr)
 * Description:
 *    This function creates a socket, init s socket
 * Parameters:
 *    pServerName   (IN) Name of server process 
 * Returns:
 *    The socket file descriptor
 *******************************************************************************/
SINT32 DM_UdpClientInit(SINT32 iDomain, SINT32 iPort, SINT32 iType,SINT8 *ipaddr,struct sockaddr_in *stServerAddr)
{
	SINT32 iFd, iRc;
	SINT32 optval = 1;
	p_debug("ipaddr = %s\n", ipaddr);
    /*
    * Create a unix domain socket.
    */
    iFd = socket(iDomain, iType, 0);
    if (iFd < 0)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "Could not create socket");
        return -1;
    }	
	
	//setsockopt(iFd,SOL_SOCKET,SO_BROADCAST|SO_REUSEADDR,&optval,sizeof(SINT32));
	setsockopt(iFd,SOL_SOCKET,SO_BROADCAST,&optval,sizeof(SINT32));
    /*
    * Connect to server.
    */
    bzero(stServerAddr, sizeof(struct sockaddr_in));
	stServerAddr->sin_family = AF_INET;
	stServerAddr->sin_port = htons(iPort);
	//stServerAddr->sin_addr.s_addr = INADDR_BROADCAST;
	stServerAddr->sin_addr.s_addr = inet_addr(ipaddr);
	
	#if 0
	memset(&(stServerAddr->sin_zero), 0, 8);
	if (inet_aton(ipaddr, (struct in_addr*)stServerAddr->sin_addr.s_addr) == 0)
	{
		perror(ipaddr);
		return -1;
	}
	#endif
    return iFd;
}


/*******************************************************************************
 * Function:
 *    SINT32 DM_UdpServerInit(SINT32 iDomain, SINT32 iPort, SINT32 iType, SINT32 iBackLog)
 * Description:
 *    This function creates and initializes a UDP listening socket
 *    for an application.
 * Parameters:
 *    iDomain  (IN) Specifies whether it is a client-side socket or 
 *                  server-side socket.
 *    iPort    (IN) The application UDP port.
 *    iType    (IN) The socket type, either SOCK_STREAM or SOCK_DGRAM.
 *    iBackLog (IN) Number of connections to queue. 
 * Returns:
 *    The socket file descriptor
 *******************************************************************************/
SINT32 DM_UdpServerInit(SINT32 iDomain, SINT32 iPort, SINT32 iType, SINT32 iBackLog) 
{
    SINT32 iFd;
    SINT32 iOptVal;
    /* Create a TCP or UDP based socket */
    if ((iFd = socket(iDomain, iType, 0)) < 0)
    {
        p_debug(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "socket errno=%d port=%d reason:%s", errno, iPort, strerror(errno));
        return MSG_INVALID_FD;
    }
#if 0
    /* Set socket options */
    iOptVal = 1;
    if (setsockopt(iFd, SOL_SOCKET, SO_REUSEADDR, &iOptVal, sizeof(iOptVal)) < 0)
    {
        p_debug(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "setsockopt errno=%d port=%d fd=%d reason:%s", 
            errno, iPort, iFd, strerror(errno));
        close(iFd);
        return MSG_INVALID_FD;
    }
#endif
    /* Set up the local address */
    if (iDomain == AF_INET)
    {
        struct sockaddr_in stServerAddr;

		#if 0
        if (iType == SOCK_DGRAM)       
		{
            /* set option for getting the to ip address. */
            if (setsockopt(iFd, IPPROTO_IP, IP_PKTINFO, &iOptVal, sizeof(iOptVal)) < 0)
            {
                DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "setsockopt errno=%d port=%d fd=%d reason:%s", 
                    errno, iPort, iFd, strerror(errno));
                close(iFd);
                return MSG_INVALID_FD;
            }
        }
		#endif

		memset(&stServerAddr, 0, sizeof(stServerAddr));
        stServerAddr.sin_family = AF_INET;
        stServerAddr.sin_port   = htons(iPort);
        stServerAddr.sin_addr.s_addr  = htonl(INADDR_ANY);

        /* Bind socket to local address */
        if (bind(iFd, (struct sockaddr *)&stServerAddr, sizeof(stServerAddr)) < 0)
        {
            DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "bind errno=%d port=%d fd=%d reason:%s", 
                errno, iPort, iFd, strerror(errno));
            close(iFd);
            return MSG_INVALID_FD;
        }
    }
    if (iType == SOCK_STREAM)
    {
        /* Enable connection to SOCK_STREAM socket */
        if (listen(iFd, iBackLog) < 0)
        {
            DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "listen errno=%d port=%d fd=%d reason:%s", 
                errno, iPort, iFd, strerror(errno));
            close(iFd);
            return MSG_INVALID_FD;
        }
    }
    return (iFd);
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
