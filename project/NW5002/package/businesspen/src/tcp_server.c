#include <stdio.h>
#include <stdlib.h>
#include <linux/tcp.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdarg.h>
#include <memory.h>
#include <stdarg.h>

#include <sys/time.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <errno.h>
#include "common.h"
#include "tcp_server.h"

/***************multi thread to process multi client***********************/

#define SERVER_LOCK  pthread_mutex_lock(&g_server_mutex)
#define SERVER_UNLOCK pthread_mutex_unlock(&g_server_mutex)

#define MAX_IDLECONNCTIME 30//There is no communicatin during x seconds,the connect will be idle and disconnnected.
#define TIMEOUTSTEP 2
#define MAXCONNECTTIME (MAX_IDLECONNCTIME/(TIMEOUTSTEP*2))

char *AllocChar(int length);
void *RecvDataForCurrentUser(void *fd);

static int g_ExitAllCommunicate = 0;

static int g_ExitCurCommunicate[MAXCLIENT+2] = {0};
static int g_OnlineFd[MAXCLIENT+2] = {0};

int g_ConnectAmount = 0;    // current connection amount
static pthread_mutex_t g_server_mutex;
int HandleDeviceDisConnect(int fd,unsigned char *str,int len);
int HandleDeviceConnect(int fd,unsigned char *str,int len);
int HandleSuccessResult(int fd,unsigned char *str,int len);
int HandleErrorResult(int fd,unsigned char *str,int len);
int HandlePageDOWNRet(int fd,unsigned char *str,int len);
int HandlePageUPRet(int fd,unsigned char *str,int len);


static struct hAction g_HandleActions[] = {
    [TYPECODE_DEVICECONNECT] = {HandleDeviceConnect},/*0 设备连接*/
    [TYPECODE_DEVICEDISCONNECT] = {HandleDeviceDisConnect},/*1 断开连接*/
    [TYPECODE_PAGEDOWN] = {HandlePageDOWNRet},/*下一页*/
    [TYPECODE_PAGEUP] = {HandlePageUPRet},/*上一页*/
    [TYPECODE_ERRORRESULT] = {HandleErrorResult},/*254 **/
    [TYPECODE_SUCCESSRESULT] = {HandleSuccessResult}/*255 */
};

void debug(int level, char *format, ...) 
{
    if(g_logLevel < level)
		return;
	//printf("g_logLevel=%d  %d\n",g_logLevel,level);
#if 0
		char strlog[8096] = {0};
		int fw_fp=0;
		int f_size=0;
		va_list ap;
	
		if( (fw_fp=fopen("/tmp/jianghupen.txt","a+"))==NULL)	// write and read,binary
		{
			exit(1);
		}		
		
		va_start(ap, format);
		vsnprintf(strlog, sizeof(strlog), format, ap);
		f_size=fwrite(strlog,1,strlen(strlog),fw_fp);
		fputc('\n',fw_fp);
		va_end(ap);
		fclose(fw_fp);
#else
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
#endif
}

void printfCmdAndFunc(char *cmd,unsigned char *value, int len)
{
    int i = 0;

    if(0 == g_logLevel)
		return;
    printf("=================================\nENTER %s  len=%d\n",cmd,len);
    for(i = 0; i < (len); i++)
    {
        if(0 == *(value+i))//if(*(value+i) > 30)
            printf("[%d]0 ",i);
        else if( (*(value+i) > (0x20)) && (*(value+i) < 0x80) )
            printf("[%d]%c ",i,*(value+i));
        else
            printf("[%d]%d ",i,*(value+i));
    }
    printf("\n=================================\n");
}

void mystrcpy(unsigned char *dest, unsigned char *src,int len)
{    
    int i = 0;	

	if(len&&src&&dest)	
	{        
	    for(i = 0; i < len; i++)      
		{           
		    dest[i] = src[i];  
		}	
	}
}

char *getActionIdByTime(void)
{
    char *ActionId = NULL;    
	struct timeval tv;    

	gettimeofday(&tv,NULL);  // printf("frame %d,StartTime=%ld %ld \n",framenum++,tv.tv_sec,(tv.tv_usec/1000));  
	ActionId = (char *)calloc(8,sizeof(char));   
	if(ActionId)    
    {
        sprintf(ActionId,"$%02d%02d",((tv.tv_sec)%100),((tv.tv_usec/1000)%1000));//sprintf(ActionId,"$%02d%02d",(p->tm_min),(p->tm_sec));   
    }    
	else       
		printf("malloc ActionId==NULL %s\n",__FUNCTION__);//    printf("ActionId = %s\n",ActionId);   
	return ActionId;
}


/*****************************************
* name：RecvDataForCurrentUser
* function：receive the message from the client
* parm：fd--socket

* return：void
*****************************************/
void *RecvDataForCurrentUser(void *fd)
{
    unsigned char ch[4] = {0};//header
    unsigned char *recvstr = NULL;
    unsigned char *recvstrheader = NULL;
    unsigned char recvchar = 0;
    int typecode = -1;
    int scokfd = *((int*)fd);
    int ret = RET_INIT;
//    int tryConnectTime = 0;
    int haveCheckId = YES;//changed by jh 0320
    int i = 0;
    int messageLen = 0;
    int recvlen = 0;
    int StartSeconds = 0;
    int CurrentTime_seconds = 0;
    int fileplay = 1;
    int n = 0;
    int IdleTimeLimit = MAX_IDLECONNCTIME;
    PlaySourceType curpst = 0;
	int mynum = 0;
    struct timeval CurrentTime = {0,0};//current time struct
    
    debug(1,"\n\n\n\n\n====================enter RecvDataForCurrentUser:scokfd=%d==============\n\n",scokfd);

    SERVER_LOCK;
	mynum = g_ConnectAmount;
	for(i = 0; i < mynum;i++)
	{
        if(g_OnlineFd[i] == scokfd)//exit the same fd
		{
		    g_ExitCurCommunicate[i] = UNNOTIFYEXIT;
			g_OnlineFd[i] = 0;
		}
	}
    g_ExitCurCommunicate[mynum] = 0;
    g_OnlineFd[mynum] = scokfd;
	g_ConnectAmount++;
    SERVER_UNLOCK;
    
    gettimeofday(&CurrentTime,NULL);
    CurrentTime_seconds = CurrentTime.tv_sec;
    StartSeconds = CurrentTime_seconds;
    
    while(1)
    {
        if(g_ExitCurCommunicate[mynum])
            break;

        i = 0;
        while( (i < 4) && (0 == g_ExitCurCommunicate[mynum]) )
        {

            /******timeout module******/
            gettimeofday(&CurrentTime,NULL);//update time
            CurrentTime_seconds = CurrentTime.tv_sec;
            if(CurrentTime_seconds != StartSeconds) //seconds change ?
            {
                StartSeconds = CurrentTime_seconds;      //debug(1,"IdleTimeLimit = %d  tryConnectTime=%d\n",IdleTimeLimit,tryConnectTime);
                //SecondsChange = 1;
                IdleTimeLimit--;
                if((IdleTimeLimit == 0) || (IdleTimeLimit == (MAX_IDLECONNCTIME)/2)|| (IdleTimeLimit == (MAX_IDLECONNCTIME)/3) )
                {
                    //ping client
                    SendHeaderToClientAlone(scokfd,curpst,0,TYPECODE_PING);
                }
                else if(IdleTimeLimit < -2)
                {
                    debug(1,"WARNING!!!!!!!!Idle time out,exit!\n");
					IdleTimeLimit = MAX_IDLECONNCTIME;//goto exitThread; 
                }
            }
            /*****************/

            recvlen = recv(scokfd,&ch[i],1,0);
            if(recvlen==0)// || (tryConnectTime > MAXCONNECTTIME))
            {
                 debug(1,"client is closed or reponse timeout222! recvlen=%d line %d\n",recvlen,__LINE__);
                 goto exitThread;
            }
            else if(recvlen != 1)
            {
                //debug(1,">>loop>>recv failed  %d recvlen=%d\n",i,recvlen);
                //sleep(1);
                //tryConnectTime++;
                perror("recvlen != 1");
				debug(1,"IdleTimeLimit=%d fd=%d errno=%d\n",IdleTimeLimit,scokfd,errno);
                usleep(100000);
				//EAGAIN=Resource temporarily unavailable
                if(errno == EAGAIN || errno == 32)//(errno == EAGAIN || errno == ENOTRECOVERABLE)
                {
                    //usleep(500000);
                    debug(1,"errno == EAGAIN  || errno == 32 errno=%d\n",errno);
				    continue;
				}
				else
				{
                    debug(1,"ERROR! errno=%d,EAGAIN=%d,pipe=32 exit thread!\n",errno,EAGAIN);
					//if(errno==ENOTRECOVERABLE)
						g_ExitCurCommunicate[mynum] = UNNOTIFYEXIT;
				    goto exitThread;
				}
            }
            //tryConnectTime = 0;
            i++;
        }
		messageLen = ((ch[0] << 24) + (ch[1] << 16) + (ch[2] << 8) + (ch[3]));//printf("---%d %d %d %d ---messageLen=%d\n",ch[0],ch[1],ch[2],ch[3],messageLen);
		memset(ch,0x0,4);
		if(0 >= messageLen || messageLen > 65536)
		{
			debug(1,"error! messageLen is invalid  messageLen=%d\n",messageLen);
			usleep(10000);
			continue;
		}
		
        recvlen = 0;
        recvstrheader = recvstr = (unsigned char *)calloc(1,(messageLen+4));//malloc(messageLen+1);
        if(recvstr)
        {
            n = messageLen;
            while( (n > 0) && (0 == g_ExitCurCommunicate[mynum]) )
            {
                recvlen=recv(scokfd,recvstr,1,0);//recv 1 char everytime
				if(recvlen > 0)
                {
                    IdleTimeLimit = MAX_IDLECONNCTIME;
                    recvstr+=recvlen;
                    n -= recvlen;
                }
                else if(recvlen==0)
                {
                    debug(1,"client is closed! %d\n",__LINE__);
					if(recvstr)
                    {
                        free(recvstr);
                        recvstr = NULL;
					}
                    goto exitThread;   
                }
                else if(recvlen < 0)
                {
                    debug(1,"recvlen=%d messageLen=%d  receive  error in %s %d\n",recvlen,(messageLen),__FUNCTION__,__LINE__);
                    //AnswerResult(scokfd,TYPECODE_ERRORRESULT,ARC_INVALIDMESSAGE,AnswerResultException[ARC_INVALIDMESSAGE],NULL);
                    usleep(500000);
                    //free(recvstr);
                    continue;
                }
            }
            recvstr = recvstrheader;
            curpst = *(recvstr+1);//update playsource
            typecode = (int)(*recvstr); //printf("------typecode=%d\n",typecode);
            if( ((NO == haveCheckId) && (typecode != TYPECODE_DEVICECONNECT)) ||
                     ((typecode == TYPECODE_DEVICECONNECT) && (ret == RETERROR)) )
            {
                debug(1,"ERROR! USER ID is invalid or have not checked,exit the recv thread\n");
                g_ExitCurCommunicate[mynum] = NOTIFYEXIT;
                break;
            }
            if(g_HandleActions[typecode].callback)
                ret = g_HandleActions[typecode].callback(scokfd,recvstr+1,messageLen-1);
            else if((typecode != TYPECODE_ERRORRESULT) && (typecode != TYPECODE_SUCCESSRESULT) && (typecode != TYPECODE_PINGRESP))
            {
                debug(1,"error,receive message cannot handle,code:%d\n",typecode);
                SendResultHeaderToClient(scokfd,curpst,0,typecode,ARC_INVALIDINFOR,NULL);
            }
			else
				debug(1,">>>>>>>unprocess type,typecode=%d\n",typecode);
			if(recvstr)
			{
				free(recvstr);
				recvstr = NULL;
			}
            if((typecode == TYPECODE_DEVICECONNECT) && (ret == RET_SUCCESS))
                haveCheckId = YES;
        }
        else
        {
            debug(1,"recvstr malloc failed! line%d\n",__LINE__);
            usleep(5000);
        }
         
    }
    if(NOTIFYEXIT == g_ExitCurCommunicate[mynum])
    {
        char *aid = NULL;
        aid = getActionIdByTime();
        SendDeviceDisConnect(scokfd,curpst,BAKEXTEND,aid);
        if(aid)
        {
            free(aid);aid = NULL;
        }

    }

exitThread:
	g_ExitCurCommunicate[mynum] = 1;
	usleep(50000);
    if(fd)
    {
        free(fd);
        fd = NULL;
    }
       
    if(recvstr)
        free(recvstr);
        
    close(scokfd);
    shutdown(scokfd,2);

   SERVER_LOCK;
   g_ExitCurCommunicate[mynum] = 1;
   g_OnlineFd[mynum] = 0;
   g_ConnectAmount--;
   SERVER_UNLOCK;
   debug(1,"pthread_exit start fd=%d\n",scokfd);
   pthread_exit(NULL);
   //debug(1,"exit current read data thread4444 fd=%d\n",scokfd);

   return NULL;
}

char *AllocChar(int length)
{
    char *ptr = NULL;
    ptr = (char *)malloc(sizeof(char) * length);
    if(NULL == ptr)
    {  
       debug(1,"malloc try again!\n");

       ptr = (char *)malloc(sizeof(char) * (length-2));//try again
       if(NULL == ptr)
         return NULL;
    }
    memset(ptr,0,length);
    return ptr;
}

int *getOnlineFd()
{
    return g_OnlineFd;
}


int HandlePageDOWNRet(int fd,unsigned char *str,int len)
{
    char actionid[MAXACTIONLEN] = {0};//action id
    PlaySourceType curpst = *(str);//
    AnswerResultCode resultcode = *(str+2);
    int aIdLenH = *(str+3);//action id length
    int aIdLenL = *(str+4);
    int aIdLen = (aIdLenH<<8) + (aIdLenL&0xff);

    printfCmdAndFunc(__FUNCTION__,str,len);

    if(aIdLen < MAXACTIONLEN && aIdLen > 0)
    {
        mystrcpy(actionid,(str+5),aIdLen);
    }
    else
    {
        printf("INVALID ACTIONID IN %s\n",__FUNCTION__);
        //SendResultHeaderToClient(fd,curpst,BAKEXTEND,TYPECODE_SUCCESSRESULT,ARC_SUCCESSFUL,NULL);
    }
    if(0 == resultcode)
        printf("PageDOWN ok actionid=%s\n",actionid);
	else
		printf("ERROR!PageDOWN FAILED resultcode=%d  actionid=%s\n",resultcode,actionid);
  
   return RET_SUCCESS;
}

int HandlePageUPRet(int fd,unsigned char *str,int len)
{
    char actionid[MAXACTIONLEN] = {0};//action id
    PlaySourceType curpst = *(str);//
    AnswerResultCode resultcode = *(str+2);
    int aIdLenH = *(str+3);//action id length
    int aIdLenL = *(str+4);
    int aIdLen = (aIdLenH<<8) + (aIdLenL&0xff);

    printfCmdAndFunc(__FUNCTION__,str,len);

    if(aIdLen < MAXACTIONLEN && aIdLen > 0)
    {
        mystrcpy(actionid,(str+5),aIdLen);
    }
    else
    {
        printf("INVALID ACTIONID IN %s\n",__FUNCTION__);
        //SendResultHeaderToClient(fd,curpst,BAKEXTEND,TYPECODE_SUCCESSRESULT,ARC_SUCCESSFUL,NULL);
    }
    if(0 == resultcode)
        printf("PageDOWN ok actionid=%s\n",actionid);
	else
		printf("ERROR!PageDOWN FAILED resultcode=%d  actionid=%s\n",resultcode,actionid);
  
   return RET_SUCCESS;
}


int HandleDeviceConnect(int fd,unsigned char *str,int len)
{    
    char UserId[64] = {0};
	char actionid[64] = {0};
	int aIdLenH = *(str+2);//action id length    
    int aIdLenL = *(str+3);    
    int aIdLen = (aIdLenH<<8) + (aIdLenL&0xff);    
    int uidLenH = *(str+4+aIdLen);//user id length    
    int uidLenL = *(str+5+aIdLen);    
    int uidLen = (uidLenH<<8) + (uidLenL&0xff);   	
    debug(1,"aIdLen=%d  uidLen=%d\n",aIdLen,uidLen);
    int i = 0;for(i = 0; i < len; i++)   printf("%d,",*(str+i));printf("\n");/**/    
    if(aIdLen > MAXACTIONLEN || aIdLen < 0)    
	{
	    SendResultHeaderToClient(fd,0,0,TYPECODE_DEVICECONNECT,ARC_UNCOMPLETE,NULL); 
        return RETERROR;    
    }

	mystrcpy(actionid,str+4,aIdLen);    
    if(uidLen <= MAXACTIONLEN && uidLen >0)   
	{
	    mystrcpy(UserId,str+6+aIdLen,uidLen);  
	    if(!strcmp(UserId,TESTUSERNAME))
	    {
			SendResultHeaderToClient(fd,0,0,TYPECODE_DEVICECONNECT,ARC_SUCCESSFUL,actionid);
			debug(1,"user id is correct:%s  %d\n",UserId,uidLen);
		    return RETSUCCESS;        
	    }
	    else
	    {         
		    debug(1,"user id is uncorrect:%s  %d\n",UserId,uidLen);       
		    SendResultHeaderToClient(fd,0,0,TYPECODE_DEVICECONNECT,ARC_INVALIDUSER,actionid);          
		    return RETERROR;      
	    }
	}   
    debug(1,"11user id is uncorrect:%s  %d\n",UserId,uidLen);   
    SendResultHeaderToClient(fd,0,0,TYPECODE_DEVICECONNECT,ARC_INVALIDUSER,actionid);
    return RETERROR;
}


int HandleDeviceDisConnect(int fd,unsigned char *str,int len)
{
    char actionid[MAXACTIONLEN] = {0};
    PlaySourceType curpst = *(str);
    int aIdLenH = *(str+2);//action id length
    int aIdLenL = *(str+3);
    int aIdLen = (aIdLenH<<8) + (aIdLenL&0xff);
	int mynum = 0;
	int i = 0;

	for(i = 0; i < MAXCLIENT;i++)
	{
        if(g_OnlineFd[i] == fd)
        {
            mynum = i;
			break;
		}
	}

	if((fd <= 0) || (i >= MAXCLIENT))
	{
	    printf("ERROR! (fd <= 0) || (i >= MAXCLIENT)\n");
        return RET_ERROR;
	}

    printfCmdAndFunc(__FUNCTION__,str,len);

    if(aIdLen < MAXACTIONLEN && aIdLen > 0)
    {
        mystrcpy(actionid,(str+4),aIdLen);
        //SendResultHeaderToClient(fd,curpst,BAKEXTEND,TYPECODE_DEVICEDISCONNECT,ARC_SUCCESSFUL,actionid);
    }
    else
    {
       // SendResultHeaderToClient(fd,curpst,BAKEXTEND,TYPECODE_DEVICEDISCONNECT,ARC_SUCCESSFUL,NULL);
    }

    debug(1,"Receive DeviceDisConnect command!  %d\n",fd);
    SERVER_LOCK;
    g_ExitCurCommunicate[mynum] = UNNOTIFYEXIT;
    SERVER_UNLOCK;
   
   return RET_SUCCESS;
}

int HandleErrorResult(int fd,unsigned char *str,int len)
{
    char actionid[MAXACTIONLEN] = {0};
    PlaySourceType curpst = *(str);
    AnswerResultCode resultcode = *(str+2);
    int aIdLenH = *(str+3);//action id length
    int aIdLenL = *(str+4);
    int aIdLen = (aIdLenH<<8) + (aIdLenL&0xff);

    printfCmdAndFunc(__FUNCTION__,str,len);

    if(aIdLen < MAXACTIONLEN && aIdLen > 0)
    {
        mystrcpy(actionid,(str+5),aIdLen);
    }
    else
    {
        printf("ERROR! aIdLen=%d is invalid in %s line %d",aIdLen,__FUNCTION__,__LINE__);
        return RET_ERROR;
    }
    printf("!!!!!!!!receive ErrorResult:\n    actionid=%s,resultcode=%d\n",actionid,resultcode);
	
    return RET_SUCCESS;
}


int HandleSuccessResult(int fd,unsigned char *str,int len)
{
    char actionid[MAXACTIONLEN] = {0};
    PlaySourceType curpst = *(str);
    AnswerResultCode resultcode = *(str+2);
    int aIdLenH = *(str+3);//action id length
    int aIdLenL = *(str+4);
    int aIdLen = (aIdLenH<<8) + (aIdLenL&0xff);

    printfCmdAndFunc(__FUNCTION__,str,len);

    if(aIdLen < MAXACTIONLEN && aIdLen > 0)
    {
        mystrcpy(actionid,(str+5),aIdLen);
    }
    else
    {
        printf("ERROR! aIdLen=%d is invalid in %s line %d",aIdLen,__FUNCTION__,__LINE__);
        return RET_ERROR;
    }

    printf("******receive ErrorResult:\n    actionid=%s\n",actionid);
    return RET_SUCCESS;
}

void ClientClose(int sig)
{
    int i = 0;
   SERVER_LOCK;
   for(i = 0; i < MAXCLIENT; i++)
       g_ExitCurCommunicate[i] = UNNOTIFYEXIT;
   SERVER_UNLOCK;
}

static void donothing(int sig)
{
    debug(1,"receive signal brokePIPE donothing! sig=%d\n",sig);
}

void *startSocketServer(void *arg)
{
    int server_sockfd;
    int *client_sockfd;
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int portreuse = 1;
	int nodelay = 1;
    //int mNetTimeout = 1; //1S
    struct timeval NetTimeout = {TIMEOUTSTEP,0};//{s,us}    500ms {0,500000}
   // int tcp_connect = 0;
   int nrcvbuf=4; //and now the default buffer is 16384
   int nRB = 0;  
   int getLength = sizeof(int);
   int portnum = *((int*)arg);
   
    signal(SIGPIPE,donothing);
    /*create socket--IPv4 ,TCP*/
    if((server_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)
    {
       perror("socket start");
       return;
    }
    /* Enable address reuse */
    setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &portreuse, sizeof(portreuse) );//reuse the port
    setsockopt( server_sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&NetTimeout, sizeof(struct timeval) );//send timeout
    setsockopt( server_sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&NetTimeout, sizeof(struct timeval) );//recv timeout
	//getsockopt(server_sockfd,SOL_SOCKET,SO_RCVBUF,(char *)&nRB,&getLength);
	//debug(1,"getLength=%d\n",getLength);
	//if(getLength < 4)
	//	setsockopt(server_sockfd,SOL_SOCKET,SO_RCVBUF,(char *)&nrcvbuf,sizeof(nrcvbuf));

   // setsockopt(server_sockfd, IPPROTO_TCP, TCP_NODELAY,(char *) &nodelay, sizeof(int));//no delay

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr =  INADDR_ANY;
    server_address.sin_port = htons(portnum);
    server_len = sizeof(struct sockaddr);

    //tcp_connect=fcntl(server_sockfd,F_GETFL,0);  
    //fcntl(server_sockfd,F_SETFL,tcp_connect | O_NONBLOCK);//no-block

   
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);//bind socket
    //templen = sizeof(struct sockaddr);

    /*listen request-length 5*/
    listen(server_sockfd,5);

    debug(1,"server waiting for connect\n");

    pthread_mutex_init(&g_server_mutex,NULL);
	
    while(1)
    {
        if(getExitFlag())
		{
		    printf(">>>>>>>>>>>>>exit the socket pregress\n");
		    break;
		}
        usleep(500000);

        pthread_t thread;//creat threads for different clients
        client_sockfd = (int *)malloc(sizeof(int));
        client_len = sizeof(struct sockaddr_in);
        *client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address, &client_len);

        if(*client_sockfd <= 0)
        {
            //perror("accept");
            if(client_sockfd)
            {
                free(client_sockfd);
                client_sockfd = NULL;
            }
            continue;
        }
        if(g_ConnectAmount>MAXCLIENT)
        {   

/********************************/
          char *tempstr = NULL;
          unsigned char tempactionid[MAXACTIONLEN] = {0};
          int aIdLenH = 0;//action id length
          int aIdLenL = 0;
          int aIdLen = 0;
          int templen = 0;
          unsigned char templenstr[4]={0};
          int recvn = 0;
          int recvlen = 0;
		 
          while(recvn < 4)
          {
            recvlen = recv(*client_sockfd,&templenstr[recvn],1,0);
            if(recvlen==0)// || (tryConnectTime > MAXCONNECTTIME))

            {
                 debug(1,"client is closed or reponse timeout! recvlen=%d line %d\n",recvlen,__LINE__);
                 break;
            }
            else if(recvlen != 1)
            {
                if(errno == EAGAIN || errno == EINTR)
                    continue;
                else
                {
                    printf("ERROR! errno=%d,EAGAIN=%d,exit!\n",errno,EAGAIN);
                    break;
                }
            }printf("templenstr[%d]=%d \n",recvn,templenstr[recvn]);
            recvn++;
          }
          if(recvn == 4)
          {
              templen=((templenstr[0] << 24) + (templenstr[1] << 16) + (templenstr[2] << 8) + (templenstr[3]));printf("templen=%d\n",templen);
              tempstr = (unsigned char *)malloc(templen +1);

              recv(*client_sockfd,tempstr,templen,0);
              aIdLenH = *(tempstr+3);//action id length
              aIdLenL = *(tempstr+4);
              aIdLen = (aIdLenH<<8) + (aIdLenL&0xff);
              mystrcpy(tempactionid,(tempstr+5),aIdLen);//printf("tempactionid=%s\n",tempactionid);
              SendResultHeaderToClient(*client_sockfd,0,0,TYPECODE_ERRORRESULT,ARC_DEVICEBUSY,tempactionid);
              if(tempstr)
                  free(tempstr);
          }
/**********************************/
/**********char *aid = NULL;
            debug(1,"device busy\n");
            aid = getActionIdByTime();
            SendResultHeaderToClient(*client_sockfd,0,0,TYPECODE_ERRORRESULT,ARC_DEVICEBUSY,aid);
            //SendDeviceDisConnect(scokfd,curpst,BAKEXTEND,aid);
            if(aid)
            {
                free(aid);aid = NULL;
            }**********/		
            close(*client_sockfd);
            shutdown(*client_sockfd,2);
            if(client_sockfd)
            {
                free(client_sockfd);
                client_sockfd = NULL;
            }
            continue;
        }
        if(pthread_create(&thread, NULL, RecvDataForCurrentUser, client_sockfd)!=0)//create thread
        {
            perror("pthread_create");
            break;
        }
        else
        {    
            debug(1,"%s:create thread success!\n",__FUNCTION__);  
            //SERVER_LOCK;
			
            //SERVER_UNLOCK;
        }
    }
    shutdown(server_sockfd,2);
    if(client_sockfd)
       free(client_sockfd);
}


