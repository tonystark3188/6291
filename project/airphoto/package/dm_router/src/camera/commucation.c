#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "camera.h"
#include "handle_request.h"
#include "debuglog.h"
#include "tcp_socket.h"
#include "base.h"

#define MAX_CLIENT 3
#define LC_RECV_TIMEOUT        6000

pthread_t g_alive_thread;
pthread_t g_handlelconnect_thread[MAX_CLIENT];
static int g_clientfd[MAX_CLIENT];

unsigned int g_lcport = 8111;

enum LONGCONNECT_CMD {
	LCCMD_PING = 0,
	LCCMD_CAMERACONN = 1,
	LCCMD_COUNT,
};

struct lcconn {
	int cmd;
	int error;
	char* respbuf;
	char* recvbuf;
	JObj* recv_json;
	JObj* resp_json;
	int timeout;
};

static int lc_parse_header(struct lcconn* lcconn)
{
	int res = 0;
	lcconn->recv_json = JSON_PARSE(lcconn->recvbuf);
	if(!lcconn->recv_json) {
		DMCLOG_E("json is null");
		lcconn->error = DM_ERROR_CMD_PARAMETER;
		goto OUT;
	}
	JObj* item = JSON_GET_OBJECT(lcconn->recv_json, "header");
	if(!item) {
		DMCLOG_E("header is null");
		lcconn->error = DM_ERROR_CMD_PARAMETER;
		goto OUT;
	}
	item = JSON_GET_OBJECT(item,"cmd");
	if(!item) {
		DMCLOG_E("cmd is null");
		lcconn->error = DM_ERROR_CMD_PARAMETER;
		goto OUT;
	}
	lcconn->cmd = JSON_GET_OBJECT_VALUE(item, int);
OUT:
	JSON_PUT_OBJECT(lcconn->recv_json);
	return res;
	
}
int lc_build_data_ping(struct lcconn* lcconn)
{
	lcconn->resp_json = JSON_NEW_EMPTY_OBJECT();
	JObj* j_data = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(lcconn->resp_json, "data", j_data);
	int camera_count = CamD_get_camera_count();
	JSON_ADD_OBJECT(j_data,"cam_cnt", JSON_NEW_OBJECT(camera_count, int));
	return 0;
}
int lc_build_data_camdete(struct lcconn* lcconn)
{
	lcconn->resp_json = JSON_NEW_EMPTY_OBJECT();
	JObj* j_data = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(lcconn->resp_json, "data", j_data);
	int camera_count = CamD_get_camera_count();
	JSON_ADD_OBJECT(j_data,"cam_cnt", JSON_NEW_OBJECT(camera_count, int));
	return 0;
}

static int lc_build_header(struct lcconn* lcconn)
{
	//int res;
	if (!lcconn->resp_json)		
		lcconn->resp_json = JSON_NEW_EMPTY_OBJECT();

	JObj* header_json = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(lcconn->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(0,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(lcconn->error,int));
	JSON_ADD_OBJECT(lcconn->resp_json, "header", header_json);

	return 0;
}

static int lc_json_to_string(struct lcconn* lcconn)
{
	int ret = 0;
	const char* jstr = JSON_TO_STRING(lcconn->resp_json);
	lcconn->respbuf = (char*)calloc(1, strlen(jstr)+1);
	if (!lcconn->respbuf) {
		DMCLOG_E("out of memory");
		ret = -1;
		goto out;
	}
		
	sprintf(lcconn->respbuf, "%s", jstr);
out:
	return ret;
}
static int lc_build_return_string(struct lcconn* lcconn)
{
	int ret = 0;
	ret = lc_parse_header(lcconn);
	if (ret !=0 || lcconn->error != 0) {
		goto build_resp_header;
	}
	//DMCLOG_D("cmd = %d", lcconn->cmd);
	switch (lcconn->cmd) {
		case LCCMD_PING:
			lc_build_data_ping(lcconn);
			break;
	}
build_resp_header:
	lc_build_header(lcconn);
	ret = lc_json_to_string(lcconn);
	return ret;
}

int free_lcconn(struct lcconn *lcconn)
{
	safe_free(lcconn->respbuf);
	safe_free(lcconn->recvbuf);
	JSON_PUT_OBJECT(lcconn->recv_json);
	JSON_PUT_OBJECT(lcconn->resp_json);
	return 0;
}
static int lc_a_request_session(int sockfd)
{
	int ret = 0;
	
	struct lcconn lcconn;
	memset(&lcconn, 0, sizeof(struct lcconn) );
	lcconn.timeout = LC_RECV_TIMEOUT;
	CK02 (ret = DM_MsgReceive(sockfd, &lcconn.recvbuf, (UINT32*)&lcconn.timeout), out);
	CK02 (ret = lc_build_return_string(&lcconn), out);
	CK02 (ret = DM_MsgSend(sockfd, lcconn.respbuf, strlen(lcconn.respbuf) ), out);
	
out:
	free_lcconn(&lcconn);
	return ret;
}

static void* handle_longconnect(void *args)
{
	pthread_detach(pthread_self());
	int *fd = (int*)args;
	int ret = 0;
	while(1) {		
		CK02 (ret = lc_a_request_session(*fd), err_clientfd) ;
	}
err_clientfd:
	DMCLOG_M("close a long-connecting,fd = %d", *fd);
	close(*fd);
	*fd = 0;
	return NULL;
}

static int create_handle_longconnect_thread(int* fd)
{
	int ret = 0;
	CK00 (ret = pthread_create(&g_handlelconnect_thread[0], NULL, handle_longconnect, fd), err_pthcreate);
	return 0;
	
err_pthcreate:
	//DEBG (0, "[error][pthread_create error][%s]\n", strerror(errno) );
//err_no:
	return -1;
	
}

static int* obtain_vaild_fd()
{
	int i = 0;
	for (i = 0; i < MAX_CLIENT; ++i) {
		if (g_clientfd[i] == 0)
			return &g_clientfd[i];
	}
	return NULL;
}

static void* longconnect(void* args)
{
	pthread_detach(pthread_self());
	int ret = 0;
	int listenfd = 0;
	char ip[32] = {0};
rebind:
	CK30 (ret = listenfd =  bind_socket_tcp(g_lcport), err_no);
	while(1) {
		int *clientfd = obtain_vaild_fd();
		if (clientfd == NULL) {
			sleep(3);
			continue;
		}
		CK00 (ret = *clientfd = DM_ServerAcceptGetIp(listenfd, ip), err_listenfd);
		if (ret == 0) {
			DMCLOG_E("accept return 0, rebind long connect socket");
			close(listenfd);
			listenfd = 0;
			goto rebind;
		}
		DMCLOG_M("start a new long-connecting,fd = %d,ip = %s", *clientfd, ip);
		CK00 (ret = create_handle_longconnect_thread(clientfd), err_listenfd);
	}
err_listenfd:
	DMCLOG_E ("long connection failed, should exit");
	close(listenfd);
err_no:
	exit(0);
		
}

int create_longconnect_thread()
{
	int ret = 0;
	CK00 (ret = pthread_create(&g_alive_thread, NULL, longconnect, NULL), err_pthcreate);
	return 0;
	
err_pthcreate:
	//DEBG (0, "[error][pthread_create error][%s]\n", strerror(errno) );
//err_no:
	return -1;

}




int sendto_clients(char* stream)
{
	int i = 0;
	int ret = 0;
	for (i = 0; i < MAX_CLIENT; ++i) {
		if (g_clientfd[i] != 0) {
			ret = DM_MsgSend(g_clientfd[i], stream, strlen(stream) );
			if (ret < 0)
				DMCLOG_E("send message to client failed,stream = %s, client = %d", stream, i);
		}
	}
	return 0;
}

int send_camera_detect(int count)
{
	int ret = 0;
	struct lcconn lcconn;
	memset(&lcconn, 0, sizeof(struct lcconn) );
	lcconn.cmd = LCCMD_CAMERACONN;
	lc_build_data_camdete(&lcconn);
	lc_build_header(&lcconn);
	ret = lc_json_to_string(&lcconn);
	if (ret < 0)
		goto out;
	DMCLOG_D("%s", lcconn.respbuf);
	ret = sendto_clients(lcconn.respbuf);
out:
	free_lcconn(&lcconn);
	return ret;
}

