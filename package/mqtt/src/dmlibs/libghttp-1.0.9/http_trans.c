/*
 * http_trans.c -- Functions for doing transport related stuff including
 * automatically extending buffers and whatnot.
 * Created: Christopher Blizzard <blizzard@appliedtheory.com>, 5-Aug-1998
 *
 * Copyright (C) 1998 Free Software Foundation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "http_trans.h"
#include "http_global.h"
#include "my_debug.h"


#include <setjmp.h>

#include <time.h>
#include <signal.h>

#define MAX_TIMEOUT 5000000
#define MIN_TIMEOUT 100000

struct host_name
{
    char *hostname;
    in_addr_t *s_addr;
};



static sigjmp_buf jmpbuf;

static void alarm_func()

{
    
    siglongjmp(jmpbuf, 1);
    
}

static struct hostent *gngethostbyname(char *HostName, int timeout)

{
    
    struct hostent *lpHostEnt;
//    sigset_t mask,oldmask;
//    sigemptyset(&mask);
//    sigaddset(&mask,SIGALRM);
//    pthread_sigmask(SIG_UNBLOCK,&mask,&oldmask);
    signal(SIGALRM, alarm_func);

    if(sigsetjmp(jmpbuf, 1) != 0)
    {

        alarm(0); /* 取消闹钟 */
        DMCLOG_D("OTA alarm time out....\n");
        signal(SIGALRM, SIG_IGN);
//        pthread_sigmask(SIG_SETMASK,&oldmask,NULL);
        return NULL;

    }

    alarm(timeout); /* 设置超时时间 */

    lpHostEnt = gethostbyname(HostName);
//    alarm(0); /* 取消闹钟 */
    signal(SIGALRM, SIG_IGN);
//    pthread_sigmask(SIG_SETMASK,&oldmask,NULL);

    return lpHostEnt;

}

static void get_host_by_name(void *self)
{
    struct host_name *p_host_name = (struct host_name *)self;
    printf("hostname = %s\n",p_host_name->hostname);
    struct hostent   *lpHostEnt = gethostbyname(p_host_name->hostname);
    if(lpHostEnt != NULL)
    {
        memcpy(p_host_name->s_addr,lpHostEnt->h_addr_list[0],sizeof(unsigned long));
    }
    if(p_host_name->hostname != NULL)
    {
        free(p_host_name->hostname);
        p_host_name->hostname = NULL;
    }
    
    if(p_host_name != NULL)
    {
        free(p_host_name);
        p_host_name = NULL;
    }
    
    
}

int create_get_host_task(char *host_name,in_addr_t *s_addr)
{
    int rslt = 0;
    int timeout = MAX_TIMEOUT;//5s
    if(host_name == NULL||s_addr == NULL)
    {
        printf("para is null\n");
        return -1;
    }
    pthread_t tid_task;
    struct host_name *p_host_name = (struct host_name *)calloc(1,sizeof(struct host_name));
    p_host_name->hostname = (char *)calloc(1,strlen(host_name) + 1);
    strcpy(p_host_name->hostname,host_name);
    p_host_name->s_addr = s_addr;
    
    if(0 != (rslt = pthread_create(&tid_task,NULL,(void *)get_host_by_name,p_host_name)))
    {
        printf("create thread error,errno = %d\n",errno);
        rslt = -1;
        return rslt;
    }
    pthread_detach(tid_task);
    
    do{
        usleep(MIN_TIMEOUT);
        if(timeout > 0)
            timeout -= MIN_TIMEOUT;
    }while(p_host_name == NULL&&timeout != 0);
    if(p_host_name != NULL&&timeout == 0)
    {
        printf("get host name time out\n");
        rslt = -1;
        return rslt;
    }
    printf("get host name time succ\n");
    return rslt;
}

static int
http_trans_buf_free(http_trans_conn *a_conn);

int
http_trans_connect(http_trans_conn *a_conn)
{
	ENTER_FUNC();
  if ((a_conn == NULL) || (a_conn->host == NULL))
  {
	printf("mqtt getsockopt errno = %d\n",errno);
	 goto ec;
  }
   
  if (a_conn->hostinfo == NULL)
    {
      /* look up the name of the proxy if it's there. */
      if (a_conn->proxy_host)
	{
	  if ((a_conn->hostinfo = gethostbyname(a_conn->proxy_host)) == NULL)
//        if ((a_conn->hostinfo = gngethostbyname(a_conn->proxy_host,5)) == NULL)
//        if(create_get_host_task(a_conn->host,&a_conn->saddr.sin_addr.s_addr) != 0)
	    {
	      a_conn->error_type = http_trans_err_type_host;
	      a_conn->error = h_errno;
	      printf("OTA access proxy host errno = %d",errno);
	      goto ec;
	    }
	}
      else
	{
	  /* look up the name */
	  if ((a_conn->hostinfo = gethostbyname(a_conn->host)) == NULL)
//        if ((a_conn->hostinfo = gngethostbyname(a_conn->host,5)) == NULL)
//        if(create_get_host_task(a_conn->host,&a_conn->saddr.sin_addr.s_addr) != 0)
	    {
	      a_conn->error_type = http_trans_err_type_host;
	      a_conn->error = h_errno;
	      printf("OTA access host errno = %d\n",h_errno);
	      goto ec;
	    }
	}
      /* set up the saddr */
      a_conn->saddr.sin_family = AF_INET;
      /* set the proxy port */
      if (a_conn->proxy_host)
	a_conn->saddr.sin_port = htons(a_conn->proxy_port);
      else
	a_conn->saddr.sin_port = htons(a_conn->port);
      /* copy the name info */
      memcpy(&a_conn->saddr.sin_addr.s_addr,a_conn->hostinfo->h_addr_list[0],sizeof(unsigned long));
    }
  /* set up the socket */
  if ((a_conn->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      a_conn->error_type = http_trans_err_type_errno;
      a_conn->error = errno;
	  printf("mqtt getsockopt errno = %d\n",errno);
      goto ec;
    }
    int flags = 0;
    if ( ( flags = fcntl( a_conn->sock, F_GETFL, 0 ) ) < 0 )
    {
        perror( "fcntl( F_GETFL ) error" );
		printf("mqtt getsockopt errno = %d\n",errno);
        goto ec;
    }
    
    if ( fcntl( a_conn->sock, F_SETFL, flags | O_NONBLOCK ) < 0 )
    {
        perror( "fcntl( F_SETFL ) error" );
		printf("mqtt getsockopt errno = %d\n",errno);
        goto ec;
    }
  /* set up the socket */
  if (connect(a_conn->sock,
	      (struct sockaddr *)&a_conn->saddr,
	      sizeof(struct sockaddr)) < 0)
    {
        if ( errno != EINPROGRESS )
        {
            a_conn->error_type = http_trans_err_type_errno;
            a_conn->error = errno;
			printf("mqtt connect errno = %d\n",errno);
            goto ec;
        }
      
    }
    int error_value,n;
    fd_set rset, wset;
    
    struct timeval tv;
    socklen_t error_value_len = sizeof(error_value);
    FD_ZERO( &rset );
    FD_SET( a_conn->sock, &rset );
    wset = rset;
    
    tv.tv_sec = 5;
    
    tv.tv_usec = 0;
    
    n = select( a_conn->sock + 1, &rset, &wset, NULL, 5 ? &tv : NULL );
    if ( n < 0 )
    {
        DMCLOG_D("OTA select returned with timeout1");
        goto ec;
    }
    else if ( n == 0 )
    {
        //fprintf( stderr, "select returned with timeout.\n" );
    	printf("OTA select returned with timeout1\n");
        goto ec;
    }
    else if ( FD_ISSET( a_conn->sock, &rset ) || FD_ISSET( a_conn->sock, &wset ) )
    {
        if ( getsockopt( a_conn->sock, SOL_SOCKET,SO_ERROR, &error_value,&error_value_len ) < 0 )
        {
        	printf("mqtt getsockopt errno = %d\n",errno);
            sprintf("getsockopt(SO_ERROR): %s", strerror(errno));
            goto ec;
        }
    }
    else
    {
        printf("OTA getsockopt errno = %d\n",errno);
        goto ec;
    }
	EXIT_FUNC();
  return 0;
 ec:
 	printf("access error.......,errno = %d\n",errno);
	EXIT_FUNC();
  return -1;
}

http_trans_conn *
http_trans_conn_new(void)
{
  http_trans_conn *l_return = NULL;

  /* allocate a new connection struct */
  l_return = (http_trans_conn *)malloc(sizeof(http_trans_conn));
  memset(l_return, 0, sizeof(http_trans_conn));
  /* default to 80 */
  l_return->port = 80;
  /* default to 1000 bytes at a time */
  l_return->io_buf_chunksize = 1024;
  /* allocate a new trans buffer */
  l_return->io_buf = malloc(l_return->io_buf_chunksize);
  memset(l_return->io_buf, 0, l_return->io_buf_chunksize);
  l_return->io_buf_len = l_return->io_buf_chunksize;
  /* make sure the socket looks like it's closed */
  l_return->sock = -1;
  return l_return;
}

void
http_trans_conn_destroy(http_trans_conn *a_conn)
{
  /* destroy the connection structure. */
  if (a_conn == NULL)
    return;
  if (a_conn->io_buf)
    free(a_conn->io_buf);
  if (a_conn->sock != -1)
    close(a_conn->sock);
  free(a_conn);
  return;
}

const char *
http_trans_get_host_error(int a_herror)
{
  switch (a_herror)
    {
    case HOST_NOT_FOUND:
      return "Host not found";
    case NO_ADDRESS:
      return "An address is not associated with that host";
    case NO_RECOVERY:
      return "An unrecoverable name server error occured";
    case TRY_AGAIN:
      return "A temporary error occurred on an authoritative name server.  Please try again later.";
    default:
      return "No error or error not known.";
    }
}

int
http_trans_append_data_to_buf(http_trans_conn *a_conn,
			      char *a_data,
			      int   a_data_len)
{
  if (http_trans_buf_free(a_conn) < a_data_len)
    {
      a_conn->io_buf = realloc(a_conn->io_buf, a_conn->io_buf_len + a_data_len);
      a_conn->io_buf_len += a_data_len;
    }
  memcpy(&a_conn->io_buf[a_conn->io_buf_alloc], a_data, a_data_len);
  a_conn->io_buf_alloc += a_data_len;
  return 1;
}

int
http_trans_read_into_buf(http_trans_conn *a_conn)
{
  int l_read = 0;
  int l_bytes_to_read = 0;

  /* set the length if this is the first time */
  if (a_conn->io_buf_io_left == 0)
    {
      a_conn->io_buf_io_left = a_conn->io_buf_chunksize;
      a_conn->io_buf_io_done = 0;
    }
  /* make sure there's enough space */
  if (http_trans_buf_free(a_conn) < a_conn->io_buf_io_left)
    {
      a_conn->io_buf = realloc(a_conn->io_buf,
			       a_conn->io_buf_len + a_conn->io_buf_io_left);
      a_conn->io_buf_len += a_conn->io_buf_io_left;
    }
  /* check to see how much we should try to read */
  if (a_conn->io_buf_io_left > a_conn->io_buf_chunksize)
    l_bytes_to_read = a_conn->io_buf_chunksize;
  else
    l_bytes_to_read = a_conn->io_buf_io_left;
  /* read in some data */
  if ((a_conn->last_read = l_read = read(a_conn->sock,
					 &a_conn->io_buf[a_conn->io_buf_alloc],
					 l_bytes_to_read)) < 0)
    {
      if (errno == EINTR)
	l_read = 0;
      else
	return HTTP_TRANS_ERR;
    }
  else if (l_read == 0)
    return HTTP_TRANS_DONE;
  /* mark the buffer */
  a_conn->io_buf_io_left -= l_read;
  a_conn->io_buf_io_done += l_read;
  a_conn->io_buf_alloc += l_read;
  /* generate the result */
  if (a_conn->io_buf_io_left == 0)
    return HTTP_TRANS_DONE;
  return HTTP_TRANS_NOT_DONE;
}

int
http_trans_write_buf(http_trans_conn *a_conn)
{
  int l_written = 0;

  if (a_conn->io_buf_io_left == 0)
    {
      a_conn->io_buf_io_left = a_conn->io_buf_alloc;
      a_conn->io_buf_io_done = 0;
    }
  /* write out some data */
  if ((a_conn->last_read = l_written = write (a_conn->sock,
					      &a_conn->io_buf[a_conn->io_buf_io_done],
					      a_conn->io_buf_io_left)) <= 0)
    {
      if (errno == EINTR)
	l_written = 0;
      else
	return HTTP_TRANS_ERR;
    }
  if (l_written == 0)
    return HTTP_TRANS_DONE;
  /* advance the counters */
  a_conn->io_buf_io_left -= l_written;
  a_conn->io_buf_io_done += l_written;
  if (a_conn->io_buf_io_left == 0)
    return HTTP_TRANS_DONE;
  return HTTP_TRANS_NOT_DONE;
}

void
http_trans_buf_reset(http_trans_conn *a_conn)
{
  if (a_conn->io_buf)
    free(a_conn->io_buf);
  a_conn->io_buf = malloc(a_conn->io_buf_chunksize);
  memset(a_conn->io_buf, 0, a_conn->io_buf_chunksize);
  a_conn->io_buf_len = a_conn->io_buf_chunksize;
  a_conn->io_buf_alloc = 0;
  a_conn->io_buf_io_done = 0;
  a_conn->io_buf_io_left = 0;
}

void
http_trans_buf_clip(http_trans_conn *a_conn, char *a_clip_to)
{
  int l_bytes = 0;
  
  /* get the number of bytes to clip off of the front */
  l_bytes = a_clip_to - a_conn->io_buf;
  if (l_bytes > 0)
    {
      memmove(a_conn->io_buf, a_clip_to, a_conn->io_buf_alloc - l_bytes);
      a_conn->io_buf_alloc -= l_bytes;
    }
  a_conn->io_buf_io_done = 0;
  a_conn->io_buf_io_left = 0;
}

char *
http_trans_buf_has_patt(char *a_buf, int a_len,
			char *a_pat, int a_patlen)
{
  int i = 0;
  for ( ; i <= ( a_len - a_patlen ); i++ )
    {
      if (a_buf[i] == a_pat[0])
	{
	  if (memcmp(&a_buf[i], a_pat, a_patlen) == 0)
	    return &a_buf[i];
	}
    }
  return NULL;
}

/* static functions */

static int
http_trans_buf_free(http_trans_conn *a_conn)
{
  return (a_conn->io_buf_len - a_conn->io_buf_alloc);
}
