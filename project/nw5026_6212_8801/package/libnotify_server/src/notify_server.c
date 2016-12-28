#include "base.h"
#include "my_debug.h"
#include "notify_server.h"


void _exit_client(void *arg)
{
    ClientTheadInfo *p_client_info = (ClientTheadInfo *)(arg);
    DM_DomainClientDeinit(p_client_info->client_fd);
    safe_free(p_client_info->recv_buf);
    safe_free(p_client_info->retstr);
    DMCLOG_D("close client socket fd");
}


int file_parse_header_json(ClientTheadInfo *p_client_info)
{
	p_client_info->r_json = JSON_PARSE(p_client_info->recv_buf);
	if(p_client_info->r_json == NULL)
	{
		DMCLOG_D("access NULL");
		p_client_info->error = -10002;
		goto FAIL_EXIT;
	}
	if(is_error(p_client_info->r_json))
	{
		DMCLOG_D("### error:post data is not a json string");
		p_client_info->error = -10002;
		goto FAIL_EXIT;
	}
	
	JObj *header_json = JSON_GET_OBJECT(p_client_info->r_json,"header");
	if(header_json == NULL)
	{
		p_client_info->error = -10002;
		goto FAIL_EXIT;
	}
	JObj *cmd_json = JSON_GET_OBJECT(header_json,"cmd");
	if(cmd_json == NULL)
	{
		p_client_info->error = -10002;
		goto FAIL_EXIT;
	}
	p_client_info->error = JSON_GET_OBJECT_VALUE(cmd_json,int);
	JObj *error_json = JSON_GET_OBJECT(header_json,"error");
	if(error_json == NULL)
	{
		p_client_info->error = -10002;
		goto FAIL_EXIT;
	}
	p_client_info->error = JSON_GET_OBJECT_VALUE(error_json,int);
	DMCLOG_D("client_info->error = %d",p_client_info->error);
	return p_client_info->error;
FAIL_EXIT:
	if(p_client_info->r_json != NULL)
		JSON_PUT_OBJECT(p_client_info->r_json);
	return p_client_info->error;
}




int file_parse_release_disk_data_json(ClientTheadInfo *p_client_info)
{
	int ret = 0;
	JObj *data_json = JSON_GET_OBJECT(p_client_info->r_json,"data");
	if(data_json == NULL){
		p_client_info->error = -10002;
		goto FAIL_EXIT;
	}

	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL){
		p_client_info->error = -10002;
		goto FAIL_EXIT;
	}
			
	JObj *status_json = JSON_GET_OBJECT(para_json,"status");
	if(status_json == NULL){
		p_client_info->error = -10002;
		goto FAIL_EXIT;
	}
	p_client_info->status = JSON_GET_OBJECT_VALUE(status_json,int);

	if(p_client_info->r_json != NULL)
		JSON_PUT_OBJECT(p_client_info->r_json);
	return 0;
FAIL_EXIT:
	if(p_client_info->r_json != NULL)
		JSON_PUT_OBJECT(p_client_info->r_json);
	return p_client_info->error;
}


char *notify_comb_release_disk_send_buf(ClientTheadInfo *p_client_info)
{
    int res_sz = 0;
    JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* para_info = JSON_NEW_EMPTY_OBJECT();

	JSON_ADD_OBJECT(para_info, "release_flag",JSON_NEW_OBJECT(p_client_info->release_flag,int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,para_info);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	char *response_str = JSON_TO_STRING(response_json);
	if(response_str == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}
	res_sz = strlen(response_str);
	p_client_info->retstr = (char*)malloc(res_sz + 1);
	if(p_client_info->retstr == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}	
	strcpy(p_client_info->retstr,response_str);
    DMCLOG_D("send_buf = %s", p_client_info->retstr);
	JSON_PUT_OBJECT(response_json);
	return 0;
}


int notify_server_release_disk(int release_flag,int *p_status)
{
    int ret = 0;
	ClientTheadInfo client_info;
	memset(&client_info,0,sizeof(ClientTheadInfo));
    client_info.time_out = RECV_TIMEOUT;
	client_info.cmd = FN_NOTIFY_RELEASE_DISK;
	strcpy(client_info.ip,LOCAL_ADDR);
	client_info.release_flag = release_flag;
	notify_comb_release_disk_send_buf(&client_info);
    client_info.client_fd = DM_InetClientInit(AF_INET, TCP_INIT_PORT, SOCK_STREAM,client_info.ip);
    if(client_info.client_fd > 0){
		ret = DM_MsgSend(client_info.client_fd,client_info.retstr, strlen(client_info.retstr));
        if(ret != 0){
            DMCLOG_D("send result to client failed, We will exit thread now!");
			//ret = -1;
            goto _HANDLE_FAILED;
        }
        ret = DM_MsgReceive(client_info.client_fd, &client_info.recv_buf, &client_info.time_out);
        if(ret <= 0){
            DMCLOG_D("_recv_req_from_client failed!");
			//ret = -1;
            goto _HANDLE_FAILED;
        }
		DMCLOG_D("p_client_info.recv_buf = %s",client_info.recv_buf);
        ret = file_parse_header_json(&client_info);
        if(ret != 0){
            DMCLOG_D("parser json error");
			//ret = client_info.error;
            goto _HANDLE_FAILED;
        }

		ret = file_parse_release_disk_data_json(&client_info);
		if(ret != 0){
            DMCLOG_D("parser data json error");
			//ret = client_info.error;
            goto _HANDLE_FAILED;
        }
		*p_status = client_info.status;
    }
	_exit_client(&client_info);
	return 0;
_HANDLE_FAILED:
    _exit_client(&client_info);
    return ret;
}



