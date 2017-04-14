#ifndef _HTTP_TCPCLIENT_
#define _HTTP_TCPCLIENT_

#include <netinet/in.h>
#include <sys/socket.h>
#include "my_debug.h"
#include "my_json.h"
#include "msg.h"
#include "defs.h"

#include "jz_aes_v12.h"

#define BUFFER_SIZE 2048
#define BUFFER_SIZE_1024 1024
#define HOST_IP_LEN_256 256

#define	MIN_REQ_LEN	16		/* "GET / HTTP/1.1\n\n"		*/
#define DM_DOWN_PERSIZE        4096
int download_img_first;

typedef struct _http_tcpclient{
	int 	socket;
	int 	remote_port;
	char 	remote_ip[16];
	struct sockaddr_in _addr; 
	int 	connected;
	char 	host[256];
	int 	time_out;
	char 	*request;
	char 	*headers;
	int 	minor_version;
	int 	major_version;
	struct headers	ch;		/* Parsed client headers	*/
	int 	local_fd;
	char 	tmp_path[256];
	size_t 	offset;
	char 	fw_update_url[URL_LEN];
	char 	fw_update_re_url[URL_LEN];
	struct task_dnode *dn;
	struct album_node *adn;	
	char album_id[33];    /*¾ç¼¯ID*/

	char error_msg[128];
	int errorCode;
} http_tcpclient;


//letv server http request type
typedef enum {
	REQ_DOWNLOAD,
	REQ_QUERY_ALBUM_INFO,
	REQ_QUERY_ALBUM,
	REQ_GET_DRAMA_INFO,
	REQ_CHECK_FIRMWARE,
	REQ_REPORT_STATUS
}LetvReqType;

#define ALBUM 2
#define VIDEO 0
#define IMAGE 3
#define VIDEO_IMAGE 8 
#define ALBUM_IMAGE 9 

#define GET_VIDEO_SIZE 7 


int report_status_err_time;


int http_tcpclient_create(http_tcpclient *,const char *host, int port,int time_out);
int http_tcpclient_conn(http_tcpclient *);
int http_tcpclient_recv(http_tcpclient *,char **lpbuff,int size);
int http_tcpclient_download(http_tcpclient *,int type);
int http_tcpclient_send(http_tcpclient *,char *buff,int size);
int http_tcpclient_close(http_tcpclient *);
int http_get(http_tcpclient *pclient,char *page, char *request, char **response,int type);
int http_get_info(http_tcpclient *pclient,char *page, char *request, char **response);
int http_get_re_url(http_tcpclient *pclient,char *page, char *request, char **response);
int http_post(http_tcpclient *pclient,char *page, char *request, char **response);
int http_upload(http_tcpclient *pclient,char *page, char *filename, char **response);

#endif

