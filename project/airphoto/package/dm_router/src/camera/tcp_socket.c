#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "debuglog.h"


#define DEBG(level, format, arg...)


int bind_socket_tcp(unsigned int port)
{
	DEBG(1,"[debug][bind tcp socket port=%d]\n",port);
    int sockfd;
    int ret = 0;
    struct sockaddr_in server_addr;
    unsigned short portnum= port;
	int portreuse = 1;

    CK30 (sockfd = socket(AF_INET, SOCK_STREAM, 0), err_csocket);

	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &portreuse, sizeof(portreuse));
	

    bzero(&server_addr,sizeof(struct sockaddr_in));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons(portnum);

    CK00 (ret =  bind(sockfd,(struct sockaddr *)(&server_addr), sizeof(struct sockaddr)), err_cbind);

    CK00 (ret = listen(sockfd,5), err_clisten);
    return sockfd;

	
err_csocket:
	DEBG(0,"[socket failed][%s]\n",strerror(errno));
	return -1;
err_cbind:
	DEBG(0,"[bind failed][%s]\n",strerror(errno));
	return -1;
err_clisten:
	DEBG(0,"[listen failed][%s]\n",strerror(errno));
    close(sockfd);
	return -1;
	
}

int wait_connect(int listenfd)
{
	DEBG(1,"[debug][wait for connection listenfd=%d]\n",listenfd);
    int newfd;
    struct sockaddr_in client_addr;
    socklen_t addr_len;
    while(1)
    {
        addr_len = sizeof(struct sockaddr_in);

        newfd = accept(listenfd, (struct sockaddr *)(&client_addr), &addr_len);
        if(newfd < 0)
        {
            DEBG(0, "[accept fail][%s]\n", strerror(errno));
            return -1;
        }
        return newfd;

    }
    return 0;
}


int tcp_recv(int sockfd, int *length, unsigned char** buffer)
{
	int ret = 0;
	unsigned char* buffer1 = NULL;
	unsigned char ch[4];
	memset(ch, 0, sizeof ch);
	unsigned int byte_recv = 0;
	while(byte_recv < 4) {
		CK30 (ret = recv(sockfd, ch+byte_recv, 4-byte_recv, 0),err_crecv);
		byte_recv += ret;
	}
	unsigned int stream_len = 0;
	byte_recv = 0;
	
	stream_len = (ch[0] << 24) + (ch[1] << 16)  + (ch[2] << 8)  +ch[3] ;
//	DEBG(1, "[debug][stream len=%d]\n", stream_len);
	buffer1 = (unsigned char*)malloc(stream_len);
	while(byte_recv < stream_len) {
		CK30 (ret = recv(sockfd, buffer1+byte_recv, stream_len-byte_recv, 0),err_crecv);
		byte_recv += ret;
	}
	*length = stream_len;
	*buffer = buffer1;
	return 0;

err_crecv:
	DEBG(0,"[*****][recv failed][%s]\n", ret<0?strerror(errno):"peer has disconnected");
	if (buffer1)
		free(buffer1);
	*length = 0;
	*buffer = NULL;
	return -1;
}

int tcp_send(int sockfd, unsigned char* stream, int length)
{
    int ret;
    int byte_sent = 0;
    unsigned char len1[4] = {0};
	len1[0] = (length >> 24)&0xff;
	len1[1] = (length >> 16)&0xff;
	len1[2] = (length >> 8)&0xff;
	len1[3] = (length)&0xff;
    while(byte_sent < 4){
        CK30 (ret = send(sockfd, len1+byte_sent, 4-byte_sent, 0), err_csend);
        byte_sent+=ret;
    }
    byte_sent = 0;
    while(byte_sent < length){
        CK30 (ret = send(sockfd, stream+byte_sent, length-byte_sent, 0), err_csend);
        byte_sent+=ret;
    }
    return 0;
err_csend:
    DEBG(0,"[send fail][%s]",strerror(errno) );
    return -1;
}

int tcp_set_non_blocking_mode(int fd)
{
	int	ret = -1;
	int	flags;

	if ((flags = fcntl(fd, F_GETFL, 0)) == -1) {
		DEBG (0, "nonblock: fcntl(F_GETFL): %d", errno);
	} else if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) != 0) {
		DEBG (0, "nonblock: fcntl(F_SETFL): %d", errno);
	} else {
		ret = 0;	/* Success */
	}

	return (ret);
}

