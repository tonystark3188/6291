/*
 * =============================================================================
 *
 *       Filename:  router_inotify.c
 *
 *    Description:  monitor the router hardware parameter changed
 *
 *        Version:  1.0
 *        Created:  2015/08/20 14:33
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver, 
 *   Organization:  
 *
 * =============================================================================*/
#include "router_inotify.h"
#include "router_task.h"
#include "msg_server.h"
#include "defs.h"

#include <stdio.h>//printf  
#include <string.h> //strcmp  
#include <unistd.h>//close  
     
#define ERR_EXIT(msg,flag)  {perror(msg);goto flag;}  
extern int exit_flag;
extern struct hd_dnode *router_dn;
struct hardware_info p_hardware_info;
void _exit_client(void *arg)
{
    ClientTheadInfo *p_client_info = (ClientTheadInfo *)(arg);
    DM_DomainClientDeinit(p_client_info->client_fd);
    safe_free(p_client_info->recv_buf);
    safe_free(p_client_info->retstr);
    DMCLOG_D("close client socket fd");
}
int notify_disk_scanning_send_buf(ClientTheadInfo *p_client_info)
{
    int res_sz = 0;
	int cmd = 6;
	int seq = 0;
	int error = 0;
    JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(error,int));
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

int notify_disk_scanning()
{
	ENTER_FUNC();
	extern int disk_change_flag;
	disk_change_flag = 1;
    /*int ret = 0;
	ClientTheadInfo p_client_info;
	memset(&p_client_info,0,sizeof(ClientTheadInfo));
    p_client_info.time_out = RECV_TIMEOUT;
	p_client_info.cmd = 6;
	strcpy(p_client_info.ip,LOCAL_ADDR);
	notify_disk_scanning_send_buf(&p_client_info);
	DMCLOG_D("p_client_info.ip = %s",p_client_info.ip);
    p_client_info.client_fd = DM_InetClientInit(AF_INET, DISK_MONITER_PORT, SOCK_STREAM,p_client_info.ip);
	DMCLOG_D("client_fd = %d",p_client_info.client_fd);
    if(p_client_info.client_fd > 0)
    {
        if((ret = DM_MsgSend(p_client_info.client_fd,p_client_info.retstr, strlen(p_client_info.retstr))) != 0)
        {
            DMCLOG_D("send result to client failed, We will exit thread now!");
            goto _HANDLE_FAILED;
        }
		DMCLOG_D("client_fd = %d",p_client_info.client_fd);
        ret = DM_MsgReceive(p_client_info.client_fd, &p_client_info.recv_buf, &p_client_info.time_out);
        if(ret <= 0)
        {
            DMCLOG_D("_recv_req_from_client failed!");
            goto _HANDLE_FAILED;
        }
		DMCLOG_D("p_client_info.recv_buf = %s",p_client_info.recv_buf);
    }
_HANDLE_FAILED:
    _exit_client(&p_client_info);*/
	EXIT_FUNC();
    return 0;
}


char * notify_comb_send_buf(int status,int statusCode)
{
	char *send_buf = NULL;
    int res_sz = 0;
	int cmd = 18;//FN_ROUTER_NOTIFY_STATUS_CHANGED;
	int seq = 0;
	int error = 0;
    JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* para_info = JSON_NEW_EMPTY_OBJECT();

	JSON_ADD_OBJECT(para_info, "status",JSON_NEW_OBJECT(status,int));
	JSON_ADD_OBJECT(para_info, "statusCode",JSON_NEW_OBJECT(statusCode,int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,para_info);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	char *response_str = JSON_TO_STRING(response_json);
	if(response_str == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}
	res_sz = strlen(response_str);
	send_buf = (char*)malloc(res_sz + 1);
	if(send_buf == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}	
	strcpy(send_buf,response_str);
    DMCLOG_D("send_buf = %s",send_buf);
	JSON_PUT_OBJECT(response_json);
	return send_buf;
}
char *notify_scanning_send_buf(int status)
{
	char *send_buf = NULL;
    int res_sz = 0;
	int cmd = 122;//FN_FILE_GET_SCAN_STATUS;
	int seq = 0;
	int error = 0;
    JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* para_info = JSON_NEW_EMPTY_OBJECT();

	JSON_ADD_OBJECT(para_info, "status",JSON_NEW_OBJECT(status,int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,para_info);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	char *response_str = JSON_TO_STRING(response_json);
	if(response_str == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}
	res_sz = strlen(response_str);
	send_buf = (char*)malloc(res_sz + 1);
	if(send_buf == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}	
	strcpy(send_buf,response_str);
    DMCLOG_D("send_buf = %s",send_buf);
	JSON_PUT_OBJECT(response_json);
	return send_buf;
}



/*
 * Desc: the entry function of handle client tcp request and response it.
 *       It just recv client request, handle the request, and response result to client.
 *
 */
int router_handle_client_request(int status,struct hd_dnode *dn)
{
    int enRet;
	int client_fd;
	char *send_buf;
	char *rcv_buf;
	int seq = 0;
	struct sockaddr_in clientAddr;
	 /*
         * Create a unix domain socket.
        */
	client_fd = DM_UdpClientInit(PF_INET, dn->port, SOCK_DGRAM,dn->ip,&clientAddr);
	if(client_fd <= 0)
	{
		DMCLOG_D("client_fd = %d",client_fd);
		enRet = -1;
		return enRet;
	}
	DMCLOG_D("client_fd = %d,port = %d,ip = %s",client_fd,dn->port,dn->ip);
	if(status == power_changed)
	{
		seq = dn->power_seq;
	}else if(status == disk_changed)
	{
		seq = dn->disk_seq;
	}else if(status == ssid_changed)
	{
		seq = dn->ssid_seq;
	}
	send_buf = notify_comb_send_buf(status,seq);
	if(send_buf == NULL)
	{
		enRet = -1;
		goto exit;;
	}
	DMCLOG_D("access udp send");
	enRet = DM_UdpSend(client_fd,send_buf, strlen(send_buf),&clientAddr);
	if(enRet == -1)
	{
		DMCLOG_D("sendto fail,errno = %d",errno);
		goto exit;
	}
	DMCLOG_D("DM_UdpReceive");
	int time_out = 3000;
	enRet = DM_UdpReceive(client_fd, &rcv_buf, &time_out, &clientAddr);
	if (enRet > 0)
	{
		DM_MsgPrintf((void *)rcv_buf, "reply message from server module", enRet, 1);
		
	}else{
		enRet = -1;
	}
exit:
	safe_free(send_buf);
	safe_free(rcv_buf);
	DM_DomainClientDeinit(client_fd);
    return enRet;
}


int analyze_hardware_status(struct hd_dnode *dn)
{
	//ENTER_FUNC();
	if(dn->power_seq != p_hardware_info.power_seq)
	{
		if(dn->request_type & power_changed)
		{
			if(router_handle_client_request(power_changed,dn)>0)
			{
				dn->power_seq = p_hardware_info.power_seq;
			}
		}
	}else if(dn->disk_seq != p_hardware_info.disk_seq)
	{
		if(dn->request_type & disk_changed)
		{
			if(router_handle_client_request(disk_changed,dn)>0)
			{
				dn->disk_seq = p_hardware_info.disk_seq;
			}
		}
	}else if(dn->ssid_seq != p_hardware_info.ssid_seq)
	{
		if(dn->request_type & ssid_changed)
		{
			if(router_handle_client_request(ssid_changed,dn)>0)
			{
				dn->ssid_seq = p_hardware_info.ssid_seq;
			}
		}
	}
	//EXIT_FUNC();
}
int notify_scanning_status(struct hd_dnode *dn,int status)
{
	int enRet;
	int client_fd;
	char *send_buf;
	char *rcv_buf;
	int seq = 0;
	struct sockaddr_in clientAddr;
	 /*
         * Create a unix domain socket.
        */
	client_fd = DM_UdpClientInit(PF_INET, dn->port, SOCK_DGRAM,dn->ip,&clientAddr);
	if(client_fd <= 0)
	{
		DMCLOG_D("client_fd = %d",client_fd);
		enRet = -1;
		return enRet;
	}
	DMCLOG_D("client_fd = %d,port = %d,ip = %s",client_fd,dn->port,dn->ip);
	send_buf = notify_scanning_send_buf(status);
	if(send_buf == NULL)
	{
		enRet = -1;
		goto exit;;
	}
	DMCLOG_D("access udp send");
	enRet = DM_UdpSend(client_fd,send_buf, strlen(send_buf),&clientAddr);
	if(enRet == -1)
	{
		DMCLOG_D("sendto fail,errno = %d",errno);
		goto exit;
	}
	DMCLOG_D("DM_UdpReceive");
	int time_out = 3000;
	enRet = DM_UdpReceive(client_fd, &rcv_buf, &time_out, &clientAddr);
	if (enRet > 0)
	{
		DM_MsgPrintf((void *)rcv_buf, "reply message from server module", enRet, 1);
		
	}else{
		enRet = -1;
	}
exit:
	safe_free(send_buf);
	safe_free(rcv_buf);
	DM_DomainClientDeinit(client_fd);
    return enRet;
}


void update_hd_dnode(struct hd_dnode *dn)
{
	unsigned i = 0;
	for(i = 0;/*count - detected via !dn below*/;i++)
	{
		if(!dn)
			break;
		DMCLOG_D("i = %d,dn->session = %s,dn->ip = %s",i,dn->session,dn->ip);
		analyze_hardware_status(dn);
		dn = dn->dn_next;
	}
}
void _notify_disk_scan_status(struct hd_dnode *dn,int status)
{
	unsigned i = 0;
	for(i = 0;/*count - detected via !dn below*/;i++)
	{
		if(!dn)
			break;
		DMCLOG_D("i = %d,dn->session = %s,dn->port = %d,status = %d",i,dn->session,dn->port,status);
		notify_scanning_status(dn,status);
		dn = dn->dn_next;
	}
}
void notify_disk_scan_status(int status)
{
	ENTER_FUNC();
	_notify_disk_scan_status(router_dn,status);
	EXIT_FUNC();
}
int notify_router_para(const char *inotify_path,int pnTimeOut)   
{  
    int fd;  
    int wd;   
	int enRet = 0;
  
	fd = DM_InotifyInit();
	if(fd < 0)
		ERR_EXIT("inotify_init",inotify_init_err);  
    wd = DM_InotifyAddWatch(fd,inotify_path);
  	if(wd < 0)
		ERR_EXIT("inofity_add_watch", inotify_add_watch_err);

    while(exit_flag == 0)  
    {  
        enRet = DM_InotifyReceive(fd,&pnTimeOut);
		if(enRet > 0)
		{
			DMCLOG_D("enRet = %d",enRet);
			//更新路由器硬件参数流水号
			if(enRet == power_changed)
			{
				p_hardware_info.power_seq += 1;
			}else if(enRet == disk_changed)
			{
				p_hardware_info.disk_seq += 1;
				notify_disk_scanning();
			}else if(enRet == ssid_changed)
			{
				p_hardware_info.ssid_seq += 1;
			}else if(enRet == power_disk_changed)
			{
				p_hardware_info.power_seq += 1;
				p_hardware_info.disk_seq += 1;
				notify_disk_scanning();
			}else if(enRet == power_ssid_changed)
			{
				p_hardware_info.power_seq += 1;
				p_hardware_info.ssid_seq += 1;
			}else if(enRet == power_disk_ssid_changed)
			{
				p_hardware_info.power_seq += 1;
				p_hardware_info.disk_seq += 1;
				p_hardware_info.ssid_seq += 1;
				notify_disk_scanning();
			}else if(enRet == disk_ssid_changed)
			{
				p_hardware_info.disk_seq += 1;
				p_hardware_info.ssid_seq += 1;
				notify_disk_scanning();
			}
		}
		// 更新指定登录的用户硬件参数流水号并往指定目标发包
		update_hd_dnode(router_dn);
		sleep(1);
    }  
success_exit:  
    ( void ) DM_InotifyRmWatch(fd,wd);  
    ( void ) DM_InotifyClose(fd);  
    return 0;  
  
read_err:  
select_err:  
inotify_add_watch_err:  
    ( void ) DM_InotifyRmWatch( fd, wd );  
inotify_init_err:  
    ( void ) DM_InotifyClose( fd );  
    return -1;  
}  

