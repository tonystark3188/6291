#include <utime.h>
#include "hidisk_file_client.h"

#define LOCAL_ADDR "127.0.0.1"
#define REMOTE_ADDR "192.168.222.254"
#define TCP_INIT_SERVER_PORT 13111
#define RECV_TIMEOUT    5000
void file_exit_client(void *arg)
{
    ClientTheadInfo *p_client_info = (ClientTheadInfo *)(arg);
    DM_DomainClientDeinit(p_client_info->client_fd);
    safe_free(p_client_info->recv_buf);
    safe_free(p_client_info->retstr);
    DMCLOG_D("close client socket fd");
}
/*
 * Desc: recv tcp request from client.
 *
 * client_info: input/output,
 * Return: success on RET_SUCCESS,
 */
int file_recv_req_from_server(ClientTheadInfo *p_client_info)
{
    ENTER_FUNC();
    int ret = -1;
    ret = DM_MsgReceive(p_client_info->client_fd, &p_client_info->recv_buf, &p_client_info->time_out);
    if (ret <= 0)
    {
        if(ret == MSGRET_DISCONNECTED)
        {
            return -1;
        }
        else if(ret == MSGRET_TIMED_OUT)
        {
            return -1;
        }
        EXIT_FUNC();
        return -1;
    }
    EXIT_FUNC();
    return 0;
}

/*
 * Desc: send handle result to tcp client.
 *
 * client_info: input/output,
 * Return: success on RET_SUCCESS,
 */
int file_send_result_to_server(ClientTheadInfo *client_info)
{
    int ret = 0;
    ret = DM_MsgSend(client_info->client_fd,client_info->retstr, strlen(client_info->retstr));
    if (ret != 0)
    {
        DMCLOG_D("DM send failed with %d,acc_fd = %d",ret,client_info->client_fd);
        return -1;
    }
    return 0;
}

int dm_check_prcs_task(ClientTheadInfo *p_client_info)
{
	ENTER_FUNC();
    int ret = 0;
    p_client_info->time_out = RECV_TIMEOUT;
    ret = file_process(p_client_info);//merge json date according to cmd and para
    if(ret < 0)
    {
        ret = -1;
        goto _HANDLE_FAILED;
    }
    p_client_info->client_fd = DM_InetClientInit(AF_INET, TCP_INIT_SERVER_PORT, SOCK_STREAM,REMOTE_ADDR);
    if(p_client_info->client_fd >= 0)
    {
        if(file_send_result_to_server(p_client_info) != 0)
        {
            DMCLOG_D("send result to client failed, We will exit thread now!");
            ret = -1;
            goto _HANDLE_FAILED;
        }
        ret = file_recv_req_from_server(p_client_info);
        if(ret != 0)
        {
            DMCLOG_D("_recv_req_from_client failed!");
            ret = -1;
            goto _HANDLE_FAILED;
        }
        DMCLOG_D("recv_buf = %s",p_client_info->recv_buf);
        ret = file_parse_process(p_client_info);
		if(ret != 0)
        {
            DMCLOG_D("parser json error");
            ret = p_client_info->error;
            goto _HANDLE_FAILED;
        }
    }else{
        ret = -1;
        goto _HANDLE_FAILED;
    }
_HANDLE_FAILED:
    file_exit_client(p_client_info);
	EXIT_FUNC();
    return ret;
}

int dm_open_prcs_task(ClientTheadInfo *p_client_info)
{
    int ret = file_process(p_client_info);//merge json date according to cmd and para
    if(ret < 0)
    {
        ret = -1;
        goto _HANDLE_FAILED;
    }
    p_client_info->time_out = RECV_TIMEOUT;
    p_client_info->client_fd = DM_InetClientInit(AF_INET, TCP_INIT_SERVER_PORT, SOCK_STREAM,REMOTE_ADDR);
    assert(p_client_info->client_fd >= 0);
    if(file_send_result_to_server(p_client_info) != 0)
    {
        DMCLOG_D("send result to client failed, We will exit thread now!");
        ret = -1;
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
    
_HANDLE_FAILED:
    safe_free(p_client_info->recv_buf);
    safe_free(p_client_info->retstr);
    return ret;
}

/*
 * 获取文件列表
 */
int dm_file_list_prcs_task(ClientTheadInfo *p_client_info)
{
    _int64_t per_bytes;
    int i = 1;
    int ret = 0;
    p_client_info->time_out = RECV_TIMEOUT;
    ret = file_process(p_client_info);//merge json date according to cmd and para
    if(ret < 0)
    {
        ret = -1;
        goto _HANDLE_FAILED;
    }
    p_client_info->client_fd = DM_InetClientInit(AF_INET, TCP_INIT_SERVER_PORT, SOCK_STREAM,REMOTE_ADDR);
    if(p_client_info->client_fd >= 0)
    {
        if(file_send_result_to_server(p_client_info) != 0)
        {
            DMCLOG_D("send result to client failed, We will exit thread now!");
            ret = -1;
            goto _HANDLE_FAILED;
        }
        p_client_info->recv_buf = (char *)calloc(1,DM_DOWN_PERSIZE + 1);
        
        do{
            if (p_client_info->time_out)
            {
                if (DM_WaitDataAvailable_t(p_client_info->client_fd, p_client_info->time_out) == 0)
                {
                    DMCLOG_D("timeout");
                    ret = -1;
                    goto _HANDLE_FAILED;
                }
            }
            
            per_bytes = readn(p_client_info->client_fd, p_client_info->recv_buf + (i-1)*DM_DOWN_PERSIZE, DM_DOWN_PERSIZE);
            if( 0 == per_bytes ) {
                DMCLOG_D("p_client_info->recv_buf = %s",p_client_info->recv_buf);
                DMCLOG_E("sock disconnect");
                break;
            }
            i++;
            p_client_info->recv_buf = (char *)realloc(p_client_info->recv_buf,DM_DOWN_PERSIZE*i + 1);
            memset(p_client_info->recv_buf + DM_DOWN_PERSIZE*(i-1) + 1,0,DM_DOWN_PERSIZE);
        }while(1);
        
        ret = file_parse_process(p_client_info);
        if(ret != 0)
        {
            DMCLOG_D("parser json error");
            goto _HANDLE_FAILED;
        }
    }
_HANDLE_FAILED:
    file_exit_client(p_client_info);
    return ret;
}


