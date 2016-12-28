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
#include <sys/sysinfo.h>
#include "server.h"
#include "hidisk_udp_server.h"
#include "usr_manage.h"
#include "base.h"
extern int exit_flag;

int dis_parse_header_json(ClientTheadInfo *client_info)
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
	p_client_info->retstr = (char*)calloc(1,res_sz + 1);
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
    p_client_info.time_out = UDP_RECV_TIMEOUT;
	p_client_info.cmd = FN_NOTIFY_DEL_IP;
	strcpy(p_client_info.client_ip,client_ip);
	strcpy(p_client_info.ip,LOCAL_ADDR);
	notify_comb_send_buf(&p_client_info);
    p_client_info.client_fd = DM_InetClientInit(AF_INET, TCP_INIT_PORT, SOCK_STREAM,p_client_info.ip);
    if(p_client_info.client_fd > 0)
    {
    	ret = DM_MsgSend(p_client_info.client_fd,p_client_info.retstr, strlen(p_client_info.retstr));
        if(ret <= 0)
        {
            DMCLOG_D("send result to client failed, We will exit thread now!ret = %d", ret);
			ret = -1;
			goto _HANDLE_FAILED;
        }
        /*ret = DM_MsgReceive(p_client_info.client_fd, &p_client_info.recv_buf, &p_client_info.time_out);
        if(ret <= 0)
        {
            DMCLOG_D("_recv_req_from_client failed!");
			ret = -1;
			goto _HANDLE_FAILED;
        }
		DMCLOG_D("p_client_info.recv_buf = %s",p_client_info.recv_buf);
        ret = dis_parse_header_json(&p_client_info);
        if(ret < 0)
        {
            DMCLOG_D("parser json error");
            goto _HANDLE_FAILED;
        }
		ret = p_client_info.error;*/
    }
_HANDLE_FAILED:
    _exit_client(&p_client_info);
    return ret;
}

int check_usr_time()
{
	unsigned i = 0;
	int ret = 0;
	int32_t expire_time;
	struct  sysinfo info; 
	int32_t timep;
	sysinfo(&info); 
	timep = info.uptime;

	unsigned cnt = 0;
	usr_dnode_t **dnp = _get_usr_list(&cnt);

	if(dnp == NULL || cnt == 0)
		return -1;
	//DMCLOG_D("cnt = %u",cnt);
	
	for( i = 0;i < cnt;i++ )
	{
		//DMCLOG_D("i = %d,ip = %s",i,dnp[i]->ip);
		//DMCLOG_D("timep = %ld,dnp[i]->start_time = %ld",timep,dnp[i]->start_time);
		expire_time = timep - dnp[i]->start_time;
		if(expire_time > EXPIRE_TIME)
		{
			//通知服务端释放指定客户端的资源
			DMCLOG_D("notify_server_disconnect_status");
			ret = notify_server_disconnect_status(dnp[i]->ip);
			//if(ret >= 0)
			{
				del_usr_from_list(dnp[i]->ip);
				DMCLOG_D("notify_server_disconnect_status succ");
				break;
			}
		}
	}
	_free_dev_list(dnp);
	return 0;
}


int _udp_listen_clients(void)
{
	int ret = -1;
	ClientTheadInfo client_info;
	usr_list_init();
	memset(&client_info,0,sizeof(ClientTheadInfo));
	client_info.client_fd = init_udp_server(LISTEN_PORT);
	if(client_info.client_fd < 0){
		DMCLOG_E("init udp server error!");
		return -1;
	}
	client_info.time_out = 1000;//default 3000
	
	while(exit_flag == 0)
	{
		ret = recv_req_from_udp_client(&client_info);
        if(ret == 1){//TIME OUT
#ifdef DELETE_FUNC
			check_usr_time();
#endif
			continue;
		}
		else if(ret < 0){
		   DMCLOG_D("_recv_req_from_client failed!");
		   break;
		}
#ifdef DELETE_FUNC
		check_usr_time();
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
	usr_list_destroy();
	return 0;
}



