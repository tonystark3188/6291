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

#include "msg.h"
#include "my_debug.h"

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
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_INFO, "commFd=%d connected to ",iFd);
    }
    
    return iFd;
}


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
        DMCLOG_D("realloc to %ld bytes failed", sizeof(MsgHeader) + nLen);
        free(pstMsg);
        return MSGRET_RESOURCE_EXCEEDED;
    }
    memcpy((SINT8 *)(pstMsg+1), pBuf, nLen);
    
    nTotalLen = sizeof(MsgHeader) + nLen;
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
            return MSGRET_INTERNAL_ERROR;
        }
    }
    else if (iRc != (SINT32) nTotalLen)
    {
        DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_ERR, "unexpected rc %d, expected %u",
               iRc, nTotalLen);
        free(pstMsg);
        return MSGRET_INTERNAL_ERROR;
    }
    
    free(pstMsg);
    return enRet;
}

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
        if ((enRet = DM_WaitDataAvailable(iFd, *pnTimeOut)) != MSGRET_SUCCESS)
        {
            return enRet;
        }
    }
    
    /*
     * Read just the header in the first read.
     * Do not try to read more because we might get part of
     * another message in the TCP
     socket.
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
    nTotalRemaining = pstMsg->dataLength + 1;
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
            DM_LOG(DM_MSG_LOG_FLAG, DM_MSG_LOG_INFO, "reading segment: soFar=%d total=%d", 
                nTotalReadSoFar, nTotalRemaining);
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
	//DMCLOG_D("pstMsg->dataLength = %d",pstMsg->dataLength);
    *pBuf = pInBuf - nTotalReadSoFar;
	free(pstMsg);
    return nTotalReadSoFar;
}

void DM_DomainClientDeinit(SINT32 iFd)
{
    if (MSG_INVALID_FD != iFd)
    {
        close(iFd);
    }
}

/******************************************************************************
 *                               FUNCTIONS                                    *
 ******************************************************************************/

/*******************************************************************************
 * Function:
 *    static MsgRet DM_WaitDataAvailable_Udp(SINT32 iFd, UINT32 nTimeOut)
 * Description:
 *    Wait for available data until timeout
 * Parameters:
 *    iFd      (IN) socket file descriptor
 *    nTimeOut (IN) Time for timeout
 * Returns:
 *    0:success;others,error
 *******************************************************************************/
static MsgRet DM_WaitDataAvailable_Udp(SINT32 iFd, UINT32 nTimeOut)
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

    if (iRc == -1)
    {
        return MSGRET_INTERNAL_ERROR;
    }
    else if(iRc == 0)
    {
        return MSGRET_TIMED_OUT;
    }else
    {
		return MSGRET_SUCCESS;
	}
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
    len = sizeof(struct sockaddr_in);
    if (NULL == pBuf)
    {
        DMCLOG_D("Send buffer is null!");
        return MSGRET_INVALID_ARGUMENTS;
    }
    
    pstMsg = (MsgHeader *)malloc(sizeof(MsgHeader));
    if (pstMsg == NULL)
    {
        DMCLOG_D("malloc of msg header failed");
        return MSGRET_RESOURCE_EXCEEDED;
    }
    memset(pstMsg, 0, sizeof(MsgHeader));
    pstMsg->dataLength = nLen;
    
    pstMsg = (MsgHeader *)realloc(pstMsg, sizeof(MsgHeader) + nLen + 1);
    if (pstMsg == NULL)
    {
        DMCLOG_D("realloc to %d bytes failed",sizeof(MsgHeader) + nLen);
        free(pstMsg);
        return MSGRET_RESOURCE_EXCEEDED;
    }
    memcpy((SINT8 *)(pstMsg + 1), pBuf, nLen);
    nTotalLen = sizeof(MsgHeader) + nLen;
    //iRc = write(iFd, (SINT8 *)pstMsg, nTotalLen);
    iRc = sendto(iFd, (SINT8 *)pstMsg, nTotalLen, 0, (struct sockaddr*)addr, len);
    //iRc = sendto(iFd, (SINT8 *)pstMsg, send_len, 0, (struct sockaddr*)addr, len);
    if (iRc < 0)
    {
        DMCLOG_D("write failed, errno=%d reason:%s",errno, strerror(errno));
        free(pstMsg);
        return MSGRET_INTERNAL_ERROR;
    }
    free(pstMsg);
    return enRet;
}
SINT32 DM_UdpReceive(SINT32 iFd, SINT8 **pBuf, UINT32 *pnTimeOut,struct sockaddr_in *addr)
{
    MsgHeader *pstMsg;
    SINT32 iRc;
    MsgRet enRet;
    UINT32 nTotalRemaining;
    SINT8 *pInBuf = NULL;
    SINT8 len;
    UINT32 buffer_len = 512;
    UINT32 total_len = 0;
    if(pBuf == NULL)
    {
        DMCLOG_D("buf is NULL!");
        return MSGRET_INVALID_ARGUMENTS;
    }
    else
    {
        *pBuf = NULL;
    }

	if (pnTimeOut)
	{
		if ((enRet = DM_WaitDataAvailable(iFd, *pnTimeOut)) != MSGRET_SUCCESS)
		{
			return enRet;
		}
	}

	/*
     * Read just the header in the first read.
     * Do not try to read more because we might get part of
     * another message in the UDP socket.
     */
    len = sizeof(struct sockaddr_in);
    total_len = sizeof(MsgHeader) + buffer_len;
    pstMsg = (MsgHeader *)malloc(total_len);
    if (pstMsg == NULL)
    {
        DMCLOG_D("alloc of msg header failed");
        return MSGRET_RESOURCE_EXCEEDED;
    }
    memset(pstMsg, 0, total_len);
    iRc = recvfrom(iFd, pstMsg, buffer_len, 0, (struct sockaddr*)addr, &len);
    if (iRc == -1) 
    {
        /* broken connection */
        free(pstMsg);
        return MSGRET_DISCONNECTED;
    }
    nTotalRemaining = pstMsg->dataLength + 1;
    pInBuf = (SINT8 *)calloc(sizeof(SINT8),nTotalRemaining);
    memcpy(pInBuf,pstMsg+1,nTotalRemaining);
//    DMCLOG_D("pInBuf = %s ,iRc = %d ,errno = %d",pInBuf, iRc, errno);
    *pBuf = pInBuf;
	free(pstMsg);
    return iRc;
}

SINT32 DM_exit_udp_server(SINT32 sockfd)
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
    /*
     * Create a unix domain socket.
     */
    iFd = socket(iDomain, iType, 0);
    if (iFd < 0)
    {
        DMCLOG_D("Could not create socket");
        return -1;
    }	
	
	//setsockopt(iFd,SOL_SOCKET,SO_BROADCAST|SO_REUSEADDR,&optval,sizeof(SINT32));
	//setsockopt(iFd,SOL_SOCKET,SO_BROADCAST,&optval,sizeof(SINT32));
    /*
    * Connect to server.
    */
    bzero(stServerAddr, sizeof(struct sockaddr_in));
	stServerAddr->sin_family = AF_INET;
	stServerAddr->sin_port = htons(iPort);
	//stServerAddr->sin_addr.s_addr = INADDR_BROADCAST;
	stServerAddr->sin_addr.s_addr = inet_addr(ipaddr);
    return iFd;
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
SINT32 DM_UdpBroadcastClientInit(SINT32 iDomain, SINT32 iPort, SINT32 iType,SINT8 *ipaddr,struct sockaddr_in *stServerAddr)
{
	SINT32 iFd, iRc;
	SINT32 optval = 1;

    /*
    * Create a unix domain socket.
    */
    iFd = socket(iDomain, iType, 0);
    if (iFd < 0)
    {
        DMCLOG_D("Could not create socket");
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
        DMCLOG_D("socket errno=%d port=%d reason:%s", errno, iPort, strerror(errno));
        return MSG_INVALID_FD;
    }
    /* Set up the local address */
    if (iDomain == AF_INET)
    {
        struct sockaddr_in stServerAddr;
        memset(&stServerAddr, 0, sizeof(stServerAddr));
        stServerAddr.sin_family = AF_INET;
        stServerAddr.sin_port   = htons(iPort);
        stServerAddr.sin_addr.s_addr  = htonl(INADDR_ANY);
        
        /* Bind socket to local address */
        if (bind(iFd, (struct sockaddr *)&stServerAddr, sizeof(stServerAddr)) < 0)
        {
            DMCLOG_D("bind errno=%d port=%d fd=%d reason:%s",
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
            DMCLOG_D("listen errno=%d port=%d fd=%d reason:%s", errno, iPort, iFd, strerror(errno));
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
