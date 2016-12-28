#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "tcp_server.h"
#include "common.h"

#define NULLSPACE 16


/*int SendPlayStatusChangeEvent(int fd,PlaySourceType playsrctype,int extend,STATEEVENTTYPE type,int value)
{
    unsigned char header[WORDLEN*2];
    int ret = RET_INIT;
    int actionidLen = 0;
    int totallen = 3+1+1;//1+1+1

    char *actionid = getActionIdByTime();

    if(fd <= 0)
    {
        debug(1,"ERROR! fd <= 0,fd=%d in %s line %d\n",fd,__FUNCTION__,__LINE__);
        return RET_FDINVALID;
    }

    if(actionid)
    {
        actionidLen = strlen(actionid);
        totallen += actionidLen+2;
    }
    header[0] = ((totallen>>24)&0xff);
    header[1] = ((totallen>>16)&0xff);
    header[2] = ((totallen>>8)&0xff);
    header[3] = ((totallen)&0xff);

    header[4] = TYPECODE_PLAYSTATUSCHANGEEVENT;

    header[5] = playsrctype;
    header[6] = extend;

    if(actionid)
    {
        *(header+7) = ((actionidLen>>8)&0xff);//uidlen
        *(header+8) = ((actionidLen)&0xff);
        mystrcpy((header+9), actionid,actionidLen);
        *(header+9+actionidLen) = type;
        *(header+10+actionidLen) = value;
    }
    else
    {
        debug(1,"ERROR !command actionId= NULL in %s %d\n",__FUNCTION__,__LINE__);
        return RET_ERROR;
    }

    printfCmdAndFunc(__FUNCTION__,header,(totallen+4));
    ret = send(fd,header,(totallen+4),0);
    if(ret == -1)
    {
        printf("send error in %s   %d\n",__FUNCTION__,__LINE__);
        if(actionid)
            free(actionid);
        return RET_ERROR;
    }
    
    if(actionid)
        free(actionid);
    return RET_SUCCESS;
}
*/

int SendDeviceDisConnect(int fd,
PlaySourceType playsrctype,
int extend,char *actionid)
{
    unsigned char header[WORDLEN*2];
    int ret = RET_INIT;
    int actionidLen = 0;
    int totallen = 3;//1+1+1
    ENTER();

    if(fd <= 0)
    {
        debug(1,"ERROR! fd <= 0,fd=%d in %s line %d\n",fd,__FUNCTION__,__LINE__);
        return RET_FDINVALID;
    }

    if(actionid)
    {
        actionidLen = strlen(actionid);
        totallen += actionidLen+2;
    }
    header[0] = ((totallen>>24)&0xff);
    header[1] = ((totallen>>16)&0xff);
    header[2] = ((totallen>>8)&0xff);
    header[3] = ((totallen)&0xff);

    header[4] = TYPECODE_DEVICEDISCONNECT;

    header[5] = playsrctype;
    header[6] = extend;

    if(actionid)
    {
        *(header+7) = ((actionidLen>>8)&0xff);//uidlen
        *(header+8) = ((actionidLen)&0xff);
        mystrcpy((header+9), actionid,actionidLen);
    }
    
    printfCmdAndFunc(__FUNCTION__,header,(totallen+4));
    ret = send(fd,header,(totallen+4),0);
    if(ret == -1)
    {
        printf("send error in %s   %d\n",__FUNCTION__,__LINE__);
        return RET_ERROR;
    }
    
    return RET_SUCCESS;
}


int SendHeaderToClientAlone(int fd,
PlaySourceType playsrctype,
unsigned char extend1,
MessageTypeCode type)// just send header
{
    unsigned char header[WORDLEN];
    int ret = RET_INIT;
    int totallen = 3;//1+1+1
                         //debug(1,"enter %s  %s  %d\n",__FUNCTION__,__FILE__,__LINE__);
    if(fd <= 0)
    {
        debug(1,"ERROR! fd <= 0,fd=%d in %s line %d\n",fd,__FUNCTION__,__LINE__);
        return RET_FDINVALID;
    }

    header[0] = ((totallen>>24)&0xff);
    header[1] = ((totallen>>16)&0xff);
    header[2] = ((totallen>>8)&0xff);
    header[3] = ((totallen)&0xff);
 
    header[4] = type;

    header[5] = playsrctype;
    header[6] = extend1;

    printfCmdAndFunc(__FUNCTION__,header,(3+4));
    ret = send(fd,header,(3+4),0);
    if(ret == -1)
    {
        debug(1,"send error in %s   %d\n",__FUNCTION__,__LINE__);
        return RET_ERROR;
    }
    
    return RET_SUCCESS;
}

// just send answer header
int SendResultHeaderToClient(int fd,
PlaySourceType playsrctype,
unsigned char extend1,
MessageTypeCode type, 
AnswerResultCode resultcode,
char *actionid)
{
    //AMHEADER ResultHeader;
    unsigned char ResultHeader[WORDLEN*4] = {0};
    int ret = RET_INIT;
    int idlen = 0;
    int totallen = 4;//1+1+1+1=4
                              
    if(fd <= 0)
    {
        debug(1,"ERROR! fd <= 0,fd=%d in %s line %d\n",fd,__FUNCTION__,__LINE__);
        return RET_FDINVALID;
    }

    if(actionid)
    {   
        idlen = strlen(actionid);
        totallen+=idlen+2;
    }

    ResultHeader[0] = ((totallen>>24)&0xff);
    ResultHeader[1] = ((totallen>>16)&0xff);
    ResultHeader[2] = ((totallen>>8)&0xff);
    ResultHeader[3] = ((totallen)&0xff);

    ResultHeader[4] = type;
    ResultHeader[5] = playsrctype;
    ResultHeader[6] = extend1;

    ResultHeader[7] = resultcode;
    if(actionid)
    {   
        debug(1,"command actionId= %s\n",actionid);
        ResultHeader[8] = ((idlen>>8)&0xff);
        ResultHeader[9] = idlen&0xff;
        mystrcpy(ResultHeader+10,actionid,idlen);
    }
    else
        debug(1,"WARNING !command actionId= NULL in %s %d\n",__FUNCTION__,__LINE__);

    printfCmdAndFunc(__FUNCTION__,ResultHeader,(totallen+4));
    ret = send(fd,ResultHeader,totallen+4,0);
    if(ret == -1)
    {

        debug(1,"send error in %s   %d\n",__FUNCTION__,__LINE__);
        return RET_ERROR;
    }
    
    return RET_SUCCESS;
}


int SendRetMessageToClient
(int fd,
PlaySourceType playsrctype,
unsigned char extend1,
MessageTypeCode type,
char *actionid,
char *RetStr,
int RetLen)
{
    //AMHEADER ResultHeader;
    unsigned char ResultHeader[WORDLEN*4] = {0};
    int ret = RET_INIT;
    int idlen = 0;
    int totallen = 3+RetLen;//1+1+1=3

    if(0 == RetLen)
        totallen+= NULLSPACE;//add 0 to the end
                   debug(1,"enter %s  %s  %d\n",__FUNCTION__,__FILE__,__LINE__);
    if(fd <= 0)
    {
        if(RetStr)
            free(RetStr);
        debug(1,"ERROR! fd <= 0,fd=%d in %s line %d\n",fd,__FUNCTION__,__LINE__);
        return RET_FDINVALID;
    }
   
    if(actionid)
    {   
        idlen = strlen(actionid);
        totallen+=idlen+2;
    }

    ResultHeader[0] = ((totallen>>24)&0xff);
    ResultHeader[1] = ((totallen>>16)&0xff);
    ResultHeader[2] = ((totallen>>8)&0xff);
    ResultHeader[3] = ((totallen)&0xff);

    ResultHeader[4] = type;
    ResultHeader[5] = playsrctype;
    ResultHeader[6] = extend1;

    //ResultHeader[7] = resultcode;
    if(actionid)
    {   
        //debug(1,"command actionId= %s\n",actionid);
        ResultHeader[7] = ((idlen>>8)&0xff);
        ResultHeader[8] = idlen&0xff;
        mystrcpy(ResultHeader+9,actionid,idlen);
    }
    else
        debug(1,"command actionId= NULL\n");

    //ResultHeader[9+idlen] = ((RetLen>>8)&0xff);//Return Message
    //ResultHeader[10+idlen] = RetLen&0xff;
//printf("sssssssssssss=%d RetLen=%d\n",(2+7+idlen+(RetLen?0:NULLSPACE)),RetLen);
    printfCmdAndFunc(__FUNCTION__,ResultHeader,(2+7+idlen+(RetLen?0:NULLSPACE)));
    ret = send(fd,ResultHeader,(2+7+idlen+(RetLen?0:NULLSPACE)),0);
    if(ret == -1)
    {
        printf("send error in %s   %d\n",__FUNCTION__,__LINE__);
        return RET_ERROR;
    }

    if(RetStr)
    {   //printf("ttttttttt\n\n");
        printfCmdAndFunc(__FUNCTION__,RetStr,(RetLen));
        ret = send(fd,RetStr,RetLen,0);
        if(ret == -1)
        {
            printf("send error in %s   %d\n",__FUNCTION__,__LINE__);
            return RET_ERROR;
        }

       // free(RetStr); //JJJHHH
       //RetStr = NULL;
    }
    return RET_SUCCESS;
}

int sendActiontoClient(ACTIONTYPE type)
{
    int *fdarray = NULL;
    int i = 0;
	char senstr[64] = {0};
		
    fdarray = getOnlineFd();
	switch(type)
    {
        case ACT_DOWN:
			for(i = 0; i < MAXCLIENT; i++)
				if(*(fdarray+i))
				{   debug(1,"fdarray[%d]=%d\n",i,(*(fdarray+i)));
				    senstr[0] = 0;senstr[1] = 4;strcpy(senstr+2,"DOWN");
				    SendRetMessageToClient(*(fdarray+i),0,0,TYPECODE_PAGEDOWN,"1111",senstr,6);//4+2=6
				}
			break;
		case ACT_UP:
			for(i = 0; i < MAXCLIENT; i++)
				if(*(fdarray+i))
				{   
				    debug(1,"fdarray[%d]=%d\n",i,(*(fdarray+i)));
				    senstr[0] = 0;senstr[1] = 2;strcpy(senstr+2,"UP");
				    SendRetMessageToClient(*(fdarray+i),0,0,TYPECODE_PAGEUP,"2222",senstr,4);//2+2=6
				}
			break;
		case ACT_DEVICEBUSY:
			for(i = 0; i < MAXCLIENT; i++)
				if(*(fdarray+i))
					SendResultHeaderToClient(*(fdarray+i),0,0,TYPECODE_ERRORRESULT,ARC_NOFREESPACE,"333");
			break;
    }

}

