#include "hidisk_tcp_monitor.h"
#include "msg.h"

static int json_to_string(ClientTheadInfo *p_client_info,JObj* response_json)
{
    int res_sz = 0;
    char *response_str = JSON_TO_STRING(response_json);
    if(response_str == NULL)
    {
        return -1;
    }
    res_sz = strlen(response_str);
    p_client_info->retstr = (char*)malloc(res_sz + 1);
    if(p_client_info->retstr == NULL)
    {
        return -1;
    }
    strcpy(p_client_info->retstr,response_str);
    return 0;
}

int comb_dm_get_fw_ver(ClientTheadInfo *p_client_info)
{
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    int res = 0;
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}
int parser_dm_get_fw_ver(ClientTheadInfo *p_client_info)
{
	int res = 0;
    p_client_info->r_json = JSON_PARSE(p_client_info->rcv_buf);
    if(p_client_info->r_json == NULL)
    {
        DMCLOG_D("access NULL");
        res = -1;
        goto exit;
    }
    if(is_error(p_client_info->r_json))
    {
        DMCLOG_D("### error:post data is not a json string");
        res = -1;
        goto exit;
    }
    JObj *header_json = JSON_GET_OBJECT(p_client_info->r_json,"header");
    if(header_json == NULL)
    {
        DMCLOG_D("access NULL");
        res = -1;
        goto exit;
    }
    p_client_info->cmd = JSON_GET_OBJECT_VALUE(JSON_GET_OBJECT(header_json,"cmd"),int);
    p_client_info->error = JSON_GET_OBJECT_VALUE(JSON_GET_OBJECT(header_json,"error"),int);
    JObj *session_json = JSON_GET_OBJECT(header_json,"session");
    if(session_json != NULL)
        strcpy(p_client_info->session,JSON_GET_OBJECT_VALUE(session_json,string));
    JObj *seq_json = JSON_GET_OBJECT(header_json,"seq");
    if(seq_json != NULL)
        p_client_info->seq = JSON_GET_OBJECT_VALUE(seq_json,int);
	JObj *data_json = JSON_GET_OBJECT(p_client_info->r_json,"data");
    if(data_json == NULL)
    {
        DMCLOG_D("access NULL");
        res = -1;
        goto exit;
    }
    JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
    if(para_json == NULL)
    {
        DMCLOG_D("access NULL");
        res = -1;
        goto exit;
    }
    JObj *ver_json = JSON_GET_OBJECT(para_json,"ver");
    if(ver_json == NULL)
    {
        DMCLOG_D("access NULL");
        res = -1;
        goto exit;
    }
    strcpy(p_client_info->ver,JSON_GET_OBJECT_VALUE(ver_json,string));
    DMCLOG_D("ver = %s",p_client_info->ver);
exit:
	return res;
}
/*
 * Desc: client thread exit function, call it before thread exit.
 *
 */
void _exit_client_thread(void *arg)
{
    ClientTheadInfo *p_client_info = (ClientTheadInfo *)(arg);
//    DMCLOG_D("close client socket fd");
    DM_DomainClientDeinit(p_client_info->client_fd);
    safe_free(p_client_info->rcv_buf);
    safe_free(p_client_info->retstr);
}
/*
 * Desc: recv tcp request from client.
 *
 * client_info: input/output,
 * Return: success on RET_SUCCESS,
 */
int recv_req_from_tcp_client(ClientTheadInfo *p_client_info)
{
    int ret = -1;
    ret = DM_MsgReceive(p_client_info->client_fd, &p_client_info->rcv_buf, &p_client_info->time_out);
    if (ret <= 0)
    {
    	DMCLOG_D("ret = %d",ret);
        return -1;
    }
	DMCLOG_D("ret = %d,p_client_info->rcv_buf = %s",ret,p_client_info->rcv_buf);
    return 0;
}

/*
 * Desc: send handle result to tcp client.
 *
 * client_info: input/output,
 * Return: success on RET_SUCCESS,
 */
int send_result_to_tcp_client(ClientTheadInfo *client_info)
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

int dm_client_prcs_task(ClientTheadInfo *p_client_info)
{
    int ret = 0;
    p_client_info->time_out = 3000;
    p_client_info->client_fd = DM_InetClientInit(AF_INET, p_client_info->tcp_server_port, SOCK_STREAM,p_client_info->ip);
    
    if(p_client_info->client_fd >= 0)
    {
        if(send_result_to_tcp_client(p_client_info) != 0)
        {
            DMCLOG_D("send result to client failed, We will exit thread now!");
			ret = -1;
            goto _HANDLE_FAILED;
        }
        ret = recv_req_from_tcp_client(p_client_info);
        if(ret != 0)
        {
            DMCLOG_D("_recv_req_from_client failed!");
			ret = -1;
            goto _HANDLE_FAILED;
        }
        ret = parser_dm_get_fw_ver(p_client_info);
        if(ret < 0)
        {
            goto _HANDLE_FAILED;
        }
    }else{
		ret = -1;
	}
_HANDLE_FAILED:	
    _exit_client_thread(p_client_info);
    return ret;
}
/*
 * Desc: handle client json request. 
 *
 * client_info: input/output,
 * Return: success on RET_SUCCESS.
 */
int _handle_client_json_req(ClientTheadInfo *client_info)
{
    int ret = 0;
    ret = comb_dm_get_fw_ver(client_info);//merge json date according to cmd and para
    if(ret < 0)
    {
        return -1;
    }
    ret = dm_client_prcs_task(client_info);
    if(ret < 0)
    {
        return -1;
    }
    return ret;
}

