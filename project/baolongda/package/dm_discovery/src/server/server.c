/*
 * =============================================================================
 *
 *       Filename:  server.c
 *
 *    Description:  hidisk server module.
 *
 *        Version:  1.0
 *        Created:  2015/3/19 10:54
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 515296288jf@163.com
 *   Organization:  
 *
 * =============================================================================
 */
#include "server.h"
#include "hidisk_udp_server.h"
#include "usr_manage.h"
#include "my_debug.h"

int file_parse_header_json(ClientTheadInfo *client_info)
{
	client_info->r_json = JSON_PARSE(client_info->recv_buf);
	if(client_info->r_json == NULL)
	{
		DMCLOG_D("access NULL");
		client_info->error = 10002;
		goto EXIT;
	}
	if(is_error(client_info->r_json))
	{
		DMCLOG_D("### error:post data is not a json string");
		client_info->error = 10002;
		goto EXIT;
	}
	
	JObj *header_json = JSON_GET_OBJECT(client_info->r_json,"header");
	if(header_json == NULL)
	{
		client_info->error = 10002;
		goto EXIT;
	}
	JObj *error_json = JSON_GET_OBJECT(header_json,"error");
	if(error_json == NULL)
	{
		client_info->error = 10002;
		goto EXIT;
	}
	client_info->error = JSON_GET_OBJECT_VALUE(error_json,int);
	DMCLOG_D("client_info->error = %d",client_info->error);
EXIT:
	return 0;
}

int _handle_client_json_req(ClientTheadInfo *client_info)
{
    return api_process(client_info);
}
void _exit_client(void *arg)
{
    ClientTheadInfo *p_client_info = (ClientTheadInfo *)(arg);
    DM_DomainClientDeinit(p_client_info->client_fd);
    safe_free(p_client_info->recv_buf);
    safe_free(p_client_info->retstr);
    DMCLOG_D("close client socket fd");
}
char *notify_comb_send_buf(ClientTheadInfo *p_client_info)
{
    int res_sz = 0;
    JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* para_info = JSON_NEW_EMPTY_OBJECT();

	JSON_ADD_OBJECT(para_info, "ip",JSON_NEW_OBJECT(p_client_info->client_ip,string));
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
    DMCLOG_D("send_buf = %s",p_client_info->retstr);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int notify_server_disconnect_status(const char *client_ip)
{
    int ret = 0;
	ClientTheadInfo p_client_info;
	memset(&p_client_info,0,sizeof(ClientTheadInfo));
    p_client_info.time_out = RECV_TIMEOUT;
	p_client_info.cmd = FN_NOTIFY_DEL_IP;
	strcpy(p_client_info.client_ip,client_ip);
	strcpy(p_client_info.ip,LOCAL_ADDR);
	notify_comb_send_buf(&p_client_info);
    p_client_info.client_fd = DM_InetClientInit(AF_INET, TCP_INIT_PORT, SOCK_STREAM,p_client_info.ip);
    if(p_client_info.client_fd > 0)
    {
        if((ret = DM_MsgSend(p_client_info.client_fd,p_client_info.retstr, strlen(p_client_info.retstr))) != 0)
        {
            DMCLOG_D("send result to client failed, We will exit thread now!");
            goto _HANDLE_FAILED;
        }
        ret = DM_MsgReceive(p_client_info.client_fd, &p_client_info.recv_buf, &p_client_info.time_out);
        if(ret <= 0)
        {
            DMCLOG_D("_recv_req_from_client failed!");
            goto _HANDLE_FAILED;
        }
		DMCLOG_D("p_client_info.recv_buf = %s",p_client_info.recv_buf);
        ret = file_parse_header_json(&p_client_info);
        if(ret < 0)
        {
            DMCLOG_D("parser json error");
            goto _HANDLE_FAILED;
        }
		ret = p_client_info.error;
    }
_HANDLE_FAILED:
    _exit_client(&p_client_info);
    return ret;
}

int update_usr_time(struct usr_dnode **dn,uint32_t cur_time)
{
	unsigned i = 0;
	int expire_time;
	char client_ip[32];
	int del_flag = 0;
	int ret = 0;
	struct usr_dnode *tmp = *dn;
	for(i = 0;/*count - detected via !dn below*/;i++)
	{
		if(!tmp)
			break;
		expire_time = cur_time - tmp->start_time;
		//DMCLOG_D("i = %d,tmp->ip = %s,expire_time = %d,tmp->count = %d, cur_time=%d, start_time=%d",i,tmp->ip,expire_time,tmp->count, cur_time, tmp->start_time);
		//if((expire_time >= 2)&&(2*expire_time > tmp->count))
		if(expire_time > 6)
		{
			//通知服务端释放指定客户端的资源
			DMCLOG_D("notify_server_disconnect_status");
			ret = notify_server_disconnect_status(tmp->ip);
			if(ret >= 0)
			{
				del_flag = 1;
				memset(client_ip,0,32);
				strcpy(client_ip,tmp->ip);
				break;
			}
		}
		tmp= tmp->dn_next;
	}
	if(del_flag == 1)
	{
		del_usr_from_list_for_ip(dn,client_ip);
	}
	return 0;
}


int _udp_listen_clients(void)
{
	int ret = -1;
	time_t timep;
	struct tm *p;
	uint32_t cur_time;
	ClientTheadInfo client_info;
	memset(&client_info,0,sizeof(ClientTheadInfo));
	client_info.client_fd = init_udp_server(LISTEN_PORT);
	if(client_info.client_fd < 0){
		DMCLOG_E("init udp server error!");
		return -1;
	}
	client_info.time_out = 1000;//default 3000
	
	while(1)
	{
		ret = recv_req_from_udp_client(&client_info);
        if(ret == 1){//TIME OUT
#ifdef DELETE_FUNC
			time(&timep);
			p = localtime(&timep);
			timep = mktime(p);
			//DMCLOG_D("time()->localtime()->mktime():%d",timep);
			extern struct usr_dnode *usr_dn;
			update_usr_time(&usr_dn,timep);
#endif
			continue;
		}
		else if(ret < 0){
		   DMCLOG_D("_recv_req_from_client failed!");
		   break;
		}
#ifdef DELETE_FUNC
		time(&timep);
		p = localtime(&timep);
		timep = mktime(p);
		//DMCLOG_D("time()->localtime()->mktime():%d",timep);
		extern struct usr_dnode *usr_dn;
		update_usr_time(&usr_dn,timep);
#endif
		ret = _parse_client_header_json(&client_info);
		if(ret != 0){
			DMCLOG_E("parse error");
			safe_free(client_info.recv_buf);
			continue;
		}
		
        ret = _handle_client_json_req(&client_info);
		if(ret != 0){
			DMCLOG_E("handle error");
			safe_free(client_info.recv_buf);
			safe_free(client_info.retstr);
			continue;
		}

		client_info.client_fd_send = DM_UdpClientInit(PF_INET, SEND_PORT, SOCK_DGRAM, client_info.ip, &client_info.clientAddrSend);
		if(client_info.client_fd_send < 0){
			DMCLOG_E("init udp client error!");
			safe_free(client_info.recv_buf);
			safe_free(client_info.retstr);
			continue;
		}
		
		if(send_result_to_udp_client(&client_info) != 0)
        {
            DMCLOG_D("send result to client failed!");
			safe_free(client_info.recv_buf);
			safe_free(client_info.retstr);
			safe_close(client_info.client_fd_send);
			continue;
        }
		safe_free(client_info.recv_buf);			
		safe_free(client_info.retstr);
		safe_close(client_info.client_fd_send);
	}
	stop_udp_listen_task(client_info.client_fd);
	return 0;
}



