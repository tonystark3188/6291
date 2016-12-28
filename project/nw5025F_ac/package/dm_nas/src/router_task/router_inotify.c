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
#include <stdio.h>//printf  
#include <string.h> //strcmp  
#include <unistd.h>//close  
#include "router_inotify.h"
#include "router_task.h"
#include "msg_server.h"
#include "router_defs.h"
#include "socket_uart.h"
#include "router_cycle.h"
#include "defs.h"
     
#define ERR_EXIT(msg,flag)  {perror(msg);goto flag;}  
extern int exit_flag;
extern inotify_power_info_t power_info_global;
#ifdef DM_CYCLE_DISK
extern int disk_status;//0:on cp;1:on board
#endif
struct hardware_info p_hardware_info;

char *notify_comb_power_send_buf(power_info_t *power_info, int status, int statusCode)
{
	char *send_buf = NULL;
    int res_sz = 0;
	int cmd = 18;//FN_ROUTER_NOTIFY_STATUS_CHANGED;
	int seq = 0;
	int error = 0;
    JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj* response_data_array = JSON_NEW_ARRAY();
	JObj* para_info = JSON_NEW_EMPTY_OBJECT();
	JObj* status_info = JSON_NEW_EMPTY_OBJECT();
	//JObj* status_info_array = JSON_NEW_ARRAY();

	JSON_ADD_OBJECT(para_info, "status",JSON_NEW_OBJECT(status,int));
	JSON_ADD_OBJECT(para_info, "statusCode",JSON_NEW_OBJECT(statusCode,int));
	JSON_ADD_OBJECT(status_info, "power",JSON_NEW_OBJECT(power_info->power,int));
	JSON_ADD_OBJECT(status_info, "power_status",JSON_NEW_OBJECT(power_info->power_status,int));
	//JSON_ARRAY_ADD_OBJECT(status_info_array, status_info);
	//JSON_ADD_OBJECT(para_info, "statusInfo", status_info_array);
	JSON_ADD_OBJECT(para_info, "statusInfo", status_info);
	JSON_ARRAY_ADD_OBJECT (response_data_array,para_info);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	const char *response_str = JSON_TO_STRING(response_json);
	if(response_str == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}
	res_sz = strlen(response_str);
	send_buf = (char*)calloc(1,res_sz + 1);
	if(send_buf == NULL)
	{
		JSON_PUT_OBJECT(response_json);
		return NULL;
	}	
	strcpy(send_buf,response_str);
	JSON_PUT_OBJECT(response_json);
	return send_buf;
}

char * notify_comb_ssid_send_buf(int status,int statusCode)
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
	const char *response_str = JSON_TO_STRING(response_json);
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

char * notify_comb_db_send_buf(int status,int statusCode,unsigned data_base_seq)
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
	JSON_ADD_OBJECT(para_info, "dataBaseSeq",JSON_NEW_OBJECT(data_base_seq,int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,para_info);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	const char *response_str = JSON_TO_STRING(response_json);
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

char * notify_comb_pwd_send_buf(int status,int statusCode)
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
	const char *response_str = JSON_TO_STRING(response_json);
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
int router_handle_client_request(char *send_buf,struct dev_dnode *dn)
{
	ENTER_FUNC();
    int enRet;
	int client_fd;
	char *rcv_buf = NULL;
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
	DMCLOG_D("dn->ip = %s",dn->ip);
	enRet = DM_UdpSend(client_fd,send_buf, strlen(send_buf),&clientAddr);
	if(enRet < 0)
	{
		DMCLOG_D("sendto fail,errno = %d",errno);
		goto exit;
	}
	int time_out = 500;
	enRet = DM_UdpReceive(client_fd, &rcv_buf, &time_out, &clientAddr);
	if (enRet > 0)
	{
		DMCLOG_D("rcv_buf:%s",rcv_buf);
		
	}else{
		DMCLOG_D("rcv_buf timeout");
		enRet = -1;
	}
exit:
	safe_free(send_buf);
	safe_free(rcv_buf);
	DM_DomainClientDeinit(client_fd);
	EXIT_FUNC();
    return enRet;
}

void dev_info_reverse_trans(dev_dnode_t *dn)
{
	char *send_buf = NULL;
	int disk_info_flag = 0;//0:no get ; 1:has get
	int res = 0;

	//DMCLOG_D("dn->power_seq = %u,p_hardware_info.power_seq = %u",dn->power_seq,p_hardware_info.power_seq);
	if(dn->power_seq != p_hardware_info.power_seq)
	{
		if(dn->request_type & power_changed){			
			EnterCriticalSection(&power_info_global.mutex);
			send_buf = notify_comb_power_send_buf(&power_info_global.power_info, power_changed, dn->power_seq);
			LeaveCriticalSection(&power_info_global.mutex);
			if(NULL != send_buf)
			{
				if(router_handle_client_request(send_buf, dn)>0)
				{
					_update_seq_to_dev(dn->ip,p_hardware_info.power_seq,POWER_TYPE);
				}
			}	
		}
	}
	
	//DMCLOG_D("dn->disk_seq = %u,p_hardware_info.disk_seq = %u",dn->disk_seq,p_hardware_info.disk_seq);
	if(dn->disk_seq != p_hardware_info.disk_seq)
	{
		DMCLOG_D("disk_changed = %d,request_type & disk_changed = %d",disk_changed,dn->request_type & disk_changed);
		if(dn->request_type & disk_changed){
			
			if(1 == disk_info_flag)
				//send_buf = notify_comb_disk_send_buf(&mAll_disk_t, disk_changed, dn->disk_seq);

			if(NULL != send_buf){
				if(router_handle_client_request(send_buf,dn)>0){
					_update_seq_to_dev(dn->ip,p_hardware_info.disk_seq,DISK_TYPE);
				}
			}
		}
	}

	if(dn->ssid_seq != p_hardware_info.ssid_seq)
	{
		if(dn->request_type & ssid_changed){
			send_buf = notify_comb_ssid_send_buf(ssid_changed, dn->power_seq);
			if(NULL != send_buf){
				if(router_handle_client_request(send_buf,dn)>0){
					_update_seq_to_dev(dn->ip,p_hardware_info.ssid_seq,SSID_TYPE);
				}
			}
		}
	}
	
	if(dn->db_seq != p_hardware_info.db_seq)
	{
		if(dn->request_type & data_base_changed)
		{
			send_buf = notify_comb_db_send_buf(data_base_changed, dn->db_seq, get_database_sign());
			if(NULL != send_buf){
				if(router_handle_client_request(send_buf,dn)>0){
					_update_seq_to_dev(dn->ip,p_hardware_info.db_seq,DB_TYPE);
					DMCLOG_D("dn->db_seq = %u,p_hardware_info.db_seq = %u",dn->db_seq,p_hardware_info.db_seq);
				}
			}
		}
	}
	//DMCLOG_D("dn->pwd_seq = %u,p_hardware_info.pwd_seq = %u",dn->pwd_seq,p_hardware_info.pwd_seq);
	if(dn->pwd_seq != p_hardware_info.pwd_seq)
	{
		if(dn->request_type & pwd_changed)
		{
			send_buf = notify_comb_pwd_send_buf(pwd_changed, dn->pwd_seq);
			if(NULL != send_buf){
				if(router_handle_client_request(send_buf,dn)>0){
					_update_seq_to_dev(dn->ip,p_hardware_info.pwd_seq,PWD_TYPE);
					DMCLOG_D("dn->pwd_seq = %u,p_hardware_info.pwd_seq = %u",dn->pwd_seq,p_hardware_info.pwd_seq);
				}
			}
		}
	}
}

void update_hd_dnode()
{
	dev_dnode_t *dev_dnode = NULL;
	unsigned cnt = 0;
	int i = 0;
	
	dev_dnode_t **dnp = _get_dev_list(&cnt);

	if(dnp == NULL || cnt == 0)
		return;
	//DMCLOG_D("cnt = %u",cnt);
	
	for( i = 0;i < cnt;i++ )
	{
		dev_info_reverse_trans(dnp[i]);
	}
	_free_dev_list(dnp);
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
		if(enRet > 0 || enRet == -9004)
		{
			//更新路由器硬件参数流水号
			if(enRet > 0)
			{
				if(enRet&power_changed)
				{
					p_hardware_info.power_seq += 1;
				}
				
				if(enRet&disk_changed)
				{
					p_hardware_info.disk_seq += 1;
					//dm_tcp_scan_notify_no_wait(AIRDISK_OFF_PC);
				}

				if(enRet&ssid_changed)
				{
					p_hardware_info.ssid_seq += 1;
				}

				if(enRet&data_base_changed)
				{
					p_hardware_info.db_seq += 1;
				}

				if(enRet&pwd_changed)
				{
					p_hardware_info.pwd_seq += 1;
				}
			}
			update_hd_dnode();
		}
		// 更新指定登录的用户硬件参数流水号并往指定目标发包
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

