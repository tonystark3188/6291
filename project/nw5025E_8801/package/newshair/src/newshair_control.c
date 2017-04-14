#include <stdio.h>
#include <stdlib.h>  
#include <string.h>  
#include <getopt.h>  
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "rtsp.h"
#include "common.h"

#define LOG_ERR(fmt, args...) fprintf(stderr, fmt, ##args)
#define LOG_MSG(fmt, args...) fprintf(stdout, fmt, ##args)
#define LOG_DEB(fmt, args...) fprintf(stdout, fmt, ##args)

char *get_command(int keyvalue)
{
    switch(keyvalue) {
        case NEXTSONG: 
            return "nextitem";
        case PRESONG:
            return "previtem";
        case VOLUMEUP: 
            return "volumeup";
        case VOLUMEDOWN:
            return "volumedown";
        case PLAYPAUSE: 
            return "playpause";
    }
}

int send_control_msg(char *remote_ip, unsigned short port, char* command, char* remote_id);

int keyhandle(int keyvalue)
{
    debug(1,"keyhandle start\n");
    char *remote_ip;
    unsigned short port;
    char *command;
    char *remote_id;

    remote_ip = get_remote_ip();
    if (remote_ip == NULL) {
        printf("can't get client's ip, control failed\n");
        goto out0;
    }
    port = get_port();
    if (port == 0) {
        printf("can't get DACP port, control failed\n");
        goto out0;
    }
    
    remote_id = get_remote_id();
    if (remote_id == NULL) {
        printf("can't get Active-Remote id, control failed\n");
        goto out0;
    }
    
    command = get_command(keyvalue);

    debug(1,"\nready to send\nremote_ip:%s\nport:%u\ncommand:%s\nremote_id:%s\n", remote_ip, port, command, remote_id);
 
    send_control_msg(remote_ip, port, command, remote_id);
out0:
    return 0;
}

int send_control_msg(char *remote_ip, unsigned short port, char* command, char* remote_id)
{
    int fd;
    int bytes;
    char *msg;
    struct sockaddr_in server_addr,client_addr;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == fd)
    {
        LOG_ERR("socket fail !\n");
        return -1;
    }
    LOG_DEB("create socket fd succeeded, fd:%d\n", fd);

    bzero(&server_addr,sizeof(struct sockaddr_in));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr= inet_addr(remote_ip);
    server_addr.sin_port=htons(port);

    if(-1 == connect(fd,(struct sockaddr *)(&server_addr), sizeof(struct sockaddr)))
    {
        LOG_ERR("connect fail !\n");
        return -1;
    }
    LOG_DEB("connect to server succeeded\n");
    
    msg = malloc(0);
    asprintf(&msg, "GET /ctrl-int/1/%s HTTP/1.1\r\nHost: %s:%u\r\nActive-Remote: %s\r\n", command, remote_ip, port, remote_id);

    bytes = write(fd,msg,strlen(msg));
    if(-1 == bytes) {
        LOG_ERR("write data fail !\n");
        goto err1;
    }
    LOG_DEB("write data succeeded, write %d bytes\n", bytes);
    LOG_MSG("send:\n%s\n",msg);

   /* bytes = read(fd,msg,1024);
    if(-1 == bytes)
    {
        LOG_ERR("read data fail !\n");
        goto err1;
    }
    LOG_DEB("receive data succeeded, receive %d bytes\n", bytes);

    msg[bytes]='\0';
    LOG_MSG("recv:\n%s\n",msg);*/
    close(fd);
    free(msg);
    return 0;
err1:
    free(msg);
    return -1;
}
/*int usage()
{
    printf("Usage : \n"  
            "-c command : set command\n"  
            "-i remote_id : set id\n"  
            "-h remote_ip : set remote_ip\n"  
            "-p port : set port\n"  
          );  
    return 0;
}


int main(int argc,char **argv)  
{  
    int c,index;  
    char command[20] = {0};  
    char remote_id[20] = {0};  
    char remote_ip[30] = {0};
    unsigned short port = -1;
    LOG_DEB("main start\n");

    struct option opts[] = {  
        {"command",required_argument,NULL,'c'},  
        {"Remote Id",required_argument,NULL,'i'},  
        {"remote_ip",required_argument,NULL,'h'},  
        {"port",required_argument,NULL,'p'},  

        {0,0,0,0}  
    };  

    opterr = 0;  

    LOG_DEB("parse commline args\n");
    while((c=getopt_long(argc,argv,"c:i:h:p:",opts,NULL)) != -1)  
    {  
        switch(c)  
        {  
            case 'c':  
                LOG_DEB("command:%s\n", optarg);
                strcpy(command,optarg);  
                break;  
            case 'i':  
                LOG_DEB("remote_id:%s\n", optarg);
                strcpy(remote_id, optarg);  
                break;   
            case 'h':  
                LOG_DEB("remote_ip:%s\n", optarg);
                strcpy(remote_ip, optarg);  
                break;   
            case 'p':  
                LOG_DEB("port:%s\n", optarg);
                port = atoi(optarg);
                break;  
            case '?':  
                usage();                                    
                return -1;  
            default:  
                break;  
        }  
    }  



    if (strlen(command) == 0||strlen(remote_id) == 0||strlen(remote_ip) == 0||port < 0) {
        usage();
        return -1;
    }
    printf( "command   : %s\n"
            "remote id : %s\n"
            "remote_ip : %s\n"
            "port : %u\n",  
            command, remote_id, remote_ip, port);  

    for(index = optind;index < argc;index++)  
        printf("Non-option argument %s\n",argv[index]);  

    LOG_DEB("send message to DACP server\n");
    if (send_control_msg(remote_ip, port, command, remote_id) < 0)
        return -1;

    return 0;  
}*/  
