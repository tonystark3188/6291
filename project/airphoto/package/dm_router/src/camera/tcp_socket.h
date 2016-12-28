#ifndef __TCP_SOCKET_H
#define __TCP_SOCKET_H

int bind_socket_tcp(unsigned int port);
int wait_connect(int listenfd);
int tcp_recv(int sockfd, int *length, unsigned char** buffer);
int tcp_send(int sockfd, unsigned char* stream, int length);
int set_non_blocking_mode(int fd);

#endif
