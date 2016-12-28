#ifndef _TCP_SERVER_H
#define _TCP_SERVER_H

#define THREAD_STACK_SIZE (size_t)0x8000

#define UNNOTIFYEXIT 2
#define NOTIFYEXIT 1
#define MAXCLIENT 4


typedef enum __PlaySourceType{
    PST_PCAUDIO = 0,
    PST_UDISK = 1,
    PST_NETDISK=2,
    PST_QTFM=3,
    PST_XMFM=4,

	PST_DECODESELF = 0xf,
}PlaySourceType;

typedef enum __QTFMType{
    QTFMV4 = 0,
    QTFMV2 = 1,
}QTFMTYPE;

typedef enum __MessageTypeCode{
    TYPECODE_DEVICECONNECT = 0,/*设备连接*/
    TYPECODE_DEVICEDISCONNECT = 1,/*断开连接*/
    TYPECODE_PAGEDOWN = 2,/*下一页*/
    TYPECODE_PAGEUP = 3,/*上一页*/
    TYPECODE_PING = 4,/*Ping*/
    TYPECODE_PINGRESP = 5,/*PingResp*/

    TYPECODE_ERRORRESULT = 254,/**/
    TYPECODE_SUCCESSRESULT = 255,/**/
    TYPECODE_UNKNOW = 256,/**/
}MessageTypeCode;

typedef enum __AnswerResultCode{
    ARC_SUCCESSFUL = 0,/*Successfull 执行命令成功*/
    ARC_FAILED = 1,/*failed 执行命令失败*/
    ARC_DEVICEBUSY = 2,/*Device busy 设备忙*/
    ARC_INVALIDINFOR = 3,/*Ivalid command 无效(未知)的命令类型*/
    ARC_UNKNERROR = 4,/*unknow error 未知错误*/
    ARC_INVALIDUSER = 5,/*invalid user 非法用户*/
    ARC_INVALIDVALUE = 6,/*invalid value 非法值*/
    ARC_UNCOMPLETE = 7,/*uncomplete 命令不完整(有数据丢失)*/
    ARC_TIMEOUT = 8,/*响应超时*/
    ARC_ERRORPASSWORD = 9,/*密码错误*/
    ARC_UNLOGIN =10, /*设备未登录*/
    ARC_NETDISCONNECT=11,  /*网络未连接*/
    ARC_NOFREESPACE=12,  /*网络未连接*/

}AnswerResultCode;

typedef enum __ACTIONTYPE{
    ACT_DOWN = 0,
    ACT_UP = 1,
    ACT_DEVICEBUSY = 2,
    ACT_UNKNOW = 3,
}ACTIONTYPE;

struct hAction {
	int (*callback)(int fd,unsigned char *str,int len);
};

typedef struct __AMheader
{
    unsigned char chLen[4];
    unsigned char type;
    unsigned char resultcode;
}AMHEADER;

/*initial the socket server*/
void *startSocketServer(void *arg);
int getExitFlag(void);
void setExitFlag(int value);
void ClientClose(int sig);
int sendActiontoClient(ACTIONTYPE type);
int *getOnlineFd();





#if 0
int *getOnlineFd();
void ClientClose(int sig);
void ServerClose(int sig);



int SendDeviceDisConnect(int fd,PlaySourceType playsrctype,int extend,char *actionid);

/*just send the handle result to client:success or failed*/
int SendResultHeaderToClient
(int fd,PlaySourceType playsrctype,unsigned char extend1,MessageTypeCode type, AnswerResultCode resultcode,char *actionid);

/*send header to client,no resultcode and actionid,work for PingResp or disconnect command*/
int SendHeaderToClientAlone
(int fd,PlaySourceType playsrctype,unsigned char extend1,MessageTypeCode type);

/*send Ret message to client,because the server received GET... or Scan... command,this function is answer the pre command*/
int SendRetMessageToClient
(int fd,PlaySourceType playsrctype,unsigned char extend1,MessageTypeCode type,char *actionid,char *RetStr,int RetLen);


int HandleActions1(int typecode, char* buf, int len);
#endif
#endif
