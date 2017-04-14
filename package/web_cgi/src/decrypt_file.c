/*
 * =============================================================================
 *
 *       Filename:  dec_file_download.c
 *
 *    Description:  decrypt file download cgi.
 *
 *        Version:  1.0
 *        Created:  2017/3/17 10:54
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ifeng@dmsys.com
 *   Organization:  
 *
 * =============================================================================
 */
#include <openssl/aes.h>

#include "msg.h"
#include "router_task.h"
#include "jz_aes_v12.h"
#include "random-string.h"
#include "md5.h"
#include "encrypt_file.h"


#define BUF_LEN 65536 //32768

#define VIDEO_BUF_LEN 65536 // 32768 //65536  //131072 //524288//524288 1MB 1048576  4MB 4194304

int safe_free(char *p)
{
	if(p!=NULL)
		{free(p);p=NULL;}
}
typedef struct _ClientThreadInfo
{
    struct sockaddr_in clientAddr;
    struct sockaddr_in clientBroAddr;
    void *client_arg; // for future extend!
    int client_fd;  // for tcp or cgi_forward udp
    // for recv
    char *recv_buf;
    // for send response
    JObj *r_json;
    JObj *s_json;
    uint32_t cmd;
    uint32_t seq;
    char session[64]; // for session
    int error;
    int acc_fd;
    char *retstr;
    uint32_t time_out;

    uint8_t deviceType; // 1:ios,2:android,3:pc

    int tcp_server_port;
    char ip[32];
    int status;     /* ±??ˉμ?±ê???? */
    unsigned statusCode; /* udpó|′eá÷??o?*/
    //file download
    char *path;
   	_int64_t offset;
    _int64_t length;
    _int64_t content_len;
    _int64_t file_length;
    unsigned modifyTime;
    char isFolder;
    int count;

    char file_buf[DM_DOWN_PERSIZE];
	char *srcPath;
    struct file_list *file_list;

    // for thread manage
    char *send_buf;

    _int64_t totalSize;
    FILE_STREAM_READ stream_read;
    int download_seq;
    char *disk_name;
    char *fileUuid;
    char *deviceUuid;
    char *deviceName;
    char *dirName;
    char *fileName;

    FILE *record_fd;
    char *token;

}ClientTheadInfo;

void main()
{
		char ret_buf[RET_BUF_LEN];
		char code[CODE_LEN_65]="\0";
		char save_code[CODE_LEN_65]="\0";		
		char sid[SID_LEN]="\0";
		char fw_sid[SID_LEN]="\0";
		char type[32]="\0";
		char vid[VID_LEN_33]="\0";
		char aid[VID_LEN_33]="\0";		
		char tmp_buf[256]="\0";
		char tmp_vid[VID_LEN_33]="\0";
		char path[512]="\0";
		int i,j,k;
		char *web_str=NULL;
		unsigned int ret=0;
		char ip[32]="\0";
		char uci_option_str[128]="\0";
		
		long long read_bytes=0;
		long long read_len=0;
		long long write_len=0;
		FILE  *fd;
		char range[32]="\0";
		char str_start[32]="\0";
		long long start=0;
		long long end=0;
		long long need_read_len=0;
		long long fsize=0;
		char led_status[8]="\0";
		char key[65]={0};
		char name[512]={0};	
		char mime[32]={0};
		char token[32]={0};

		//p_debug(getenv("HTTP_USER_AGENT"));
		//p_debug(getenv("PATH_INFO"));
		//p_debug(getenv("CONTENT_TYPE"));
	//	p_debug(getenv("REMOTE_ADDR"));

		if(getenv("HTTP_HOST")!=NULL)
			strcpy(ip,getenv("HTTP_HOST"));
		else p_debug("get HTTP_HOST error!");

		if((web_str=GetStringFromWeb())==NULL)
		{
			printf("Content-type:text/plain\r\n\r\n");
			fprintf(stdout,"can't get string from web\n");
			exit(1);
		}
		processString(web_str,"key",key);		
		
		processString(web_str,"name",name);		
		p_debug("name=%s",name);

		sprintf(buf,"{\"follow_add\":{\"pid\":\"%s\",\"ext\":%s,\"tag\":%s}}",pid,pExt,pTag);
    p_client_info->client_fd = DM_InetClientInit(AF_INET, p_client_info->tcp_server_port, SOCK_STREAM,p_client_info->ip);
    if(p_client_info->client_fd >= 0)
    {
#ifdef __linux__
        if(anetKeepAlive(p_client_info->client_fd,interval) == -1)
        {
            DMCLOG_E("Unsupported option SO_KEEPALIVE: %d",errno);
            ret = PROCESS_TASK_ERROR;
            goto _HANDLE_FAILED;
        }
#endif
        
        set_non_blocking_mode(p_client_info->client_fd);
        if(file_send_result_to_server(p_client_info) != 0)
        {
            DMCLOG_D("send result to client failed, We will exit thread now!");
            ret = PROCESS_TASK_ERROR;
            goto _HANDLE_FAILED;
        }
        ret = file_recv_req_from_server(p_client_info);
        if(ret != 0)
        {
            DMCLOG_D("_recv_req_from_client failed!");
            goto _HANDLE_FAILED;
        }
        
        ret = file_parse_process(p_client_info);
        if(ret != 0)
        {
            DMCLOG_D("parser json error");
            goto _HANDLE_FAILED;
        }
        ret = dm_fcreate_local_file(p_client_info);
        if ( ret <= 0 )
        {
            DMCLOG_D("create local file error");
            ret = PROCESS_TASK_ERROR;
            goto _HANDLE_FAILED;
        }
        ret = 0;
        _lseek64(p_client_info->local_fd,p_client_info->offset,SEEK_SET);
        DMCLOG_D("p_client_info->offset = %lld,p_client_info->length = %lld",p_client_info->offset,p_client_info->content_len);
        do {
            per_bytes = read(p_client_info->client_fd, p_client_info->file_buf, DM_DOWN_PERSIZE);
            if ( per_bytes > 0 ) {
                already_bytes += per_bytes;
                if ( already_bytes > p_client_info->length ) { 	/* 处理最后多了\r\n两个字节 */
                    per_bytes -= (already_bytes-p_client_info->length);
                    already_bytes = p_client_info->length;

                }
//                PTHREAD_MUTEX_LOCK(p_client_info->mutex);
                p_client_info->rDownloadTask->already_bytes += per_bytes;
//                PTHREAD_MUTEX_UNLOCK(p_client_info->mutex);

                p_client_info->rDownloadTask->download_cb(p_client_info->rDownloadTask);
                write_bytes = write(p_client_info->local_fd,p_client_info->file_buf,per_bytes);
                if(write_bytes != per_bytes)
                {
                    ret = DOWNLOAD_LACK_LOCAL_STORAGE;
                    DMCLOG_E("lack local storage");
                    break;
                }
#if 1
                cur_time = time(NULL);
                if(cur_time > record_time)
                {
                    DMCLOG_D("p_client_info->expire_time = %ld",p_client_info->expire_time);
                    record_time = cur_time + p_client_info->expire_time;
                    PTHREAD_MUTEX_LOCK(p_client_info->mutex);
//                    fsync(p_client_info->local_fd);
                    cur_position = already_bytes + p_client_info->offset;
                    update_record_for_index(&p_client_info->dn,p_client_info->length,cur_position);
//                    display_record_dnode(p_client_info->dn);
                    write_list_to_stream(p_client_info->record_fd,p_client_info->dn);//写入数据
                    PTHREAD_MUTEX_UNLOCK(p_client_info->mutex);
                }
#endif
            } else if ( 0 == per_bytes ) {
                ret = DEVICE_DISCONNECTED;
                DMCLOG_E("error = %d",p_client_info->error);
                break;
            } else {
                if(errno == EINTR || errno == EWOULDBLOCK)
                {
                    continue;
                }
                if ( already_bytes < p_client_info->content_len ) {
                    ret = DEVICE_DISCONNECTED;
                    DMCLOG_E("error = %d",p_client_info->error);
                    break;
                }
            }
        } while ( already_bytes < p_client_info->content_len&&p_client_info->rDownloadTask->download_status == 0 );
		//printf("%s",ret_buf);
		fflush(stdout);
		free(web_str);
		destroy_record(e_file_list);
		return ;
}


