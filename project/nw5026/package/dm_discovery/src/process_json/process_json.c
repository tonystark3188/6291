/*
 * =============================================================================
 *
 *       Filename:  hd_route.c
 *
 *    Description:  set and get wifi settings,set and get network settings
 *
 *        Version:  1.0
 *        Created:  2015/04/03 14:33
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver, 
 *   Organization:  
 *
 * =============================================================================*/
#include "process_json.h"
#include "my_debug.h"


int api_response(ClientTheadInfo *p_client_info)
{
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
	JSON_ADD_OBJECT(header_json, "ver", JSON_NEW_OBJECT(p_client_info->ver,int));
	JSON_ADD_OBJECT(header_json, "device", JSON_NEW_OBJECT(p_client_info->device,int));
	JSON_ADD_OBJECT(header_json, "appid", JSON_NEW_OBJECT(p_client_info->appid,int));
	JSON_ADD_OBJECT(header_json, "code", JSON_NEW_OBJECT(p_client_info->code,int));
    JSON_ADD_OBJECT(response_json, "header", header_json);
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
    JSON_PUT_OBJECT(response_json);
	DMCLOG_D("header->retstr = %s",p_client_info->retstr);
	return 0;
}



int _parse_client_header_json(ClientTheadInfo *p_client_info)
{
	int ret = 0;
	char *rcv_buf = NULL;
	int rcv_len = strlen(p_client_info->recv_buf);
	rcv_buf = (char *)malloc(sizeof(char)*(rcv_len + 1));
	if(rcv_buf == NULL){
		DMCLOG_E("malloc buffer error!");
		ret = -1;
		goto EXIT3;
	}
	strcpy(rcv_buf,p_client_info->recv_buf);
	p_client_info->r_json = JSON_PARSE(rcv_buf);
	if(p_client_info->r_json == NULL){
		DMCLOG_E("access json NULL");
		ret = -1;
		goto EXIT2;
	}
	if(is_error(p_client_info->r_json)){
		DMCLOG_E("### error:post data is not a json string");
		ret = -1;
		goto EXIT1;
	}
	
	JObj *header_json = JSON_GET_OBJECT(p_client_info->r_json,"header");
	if(header_json == NULL){
		DMCLOG_E("access json NULL");
		ret = -1;
		goto EXIT1;
	}

	JObj *cmd_json = JSON_GET_OBJECT(header_json,"cmd");
	if(cmd_json == NULL){
		DMCLOG_E("access json NULL");
		ret = -1;
		goto EXIT1;
	}
	p_client_info->cmd = JSON_GET_OBJECT_VALUE(cmd_json, int);
/*
	JObj *session_json = JSON_GET_OBJECT(header_json,"session");
	if(session_json == NULL){
		DMCLOG_E("access json NULL");
		ret = -1;
		goto EXIT1;
	}
	p_client_info->session_id= JSON_GET_OBJECT_VALUE(session_json, int);
*/
	JObj *seq_json = JSON_GET_OBJECT(header_json,"seq");
	if(seq_json == NULL){
		DMCLOG_E("access json NULL");
		ret = -1;
		goto EXIT1;
	}
	p_client_info->seq = JSON_GET_OBJECT_VALUE(seq_json, int);
/*
	JObj *ver_json = JSON_GET_OBJECT(header_json,"ver");
	if(ver_json == NULL){
		DMCLOG_E("access json NULL");
		ret = -1;
		goto EXIT1;
	}
	p_client_info->ver = JSON_GET_OBJECT_VALUE(ver_json,int);

	JObj *device_json = JSON_GET_OBJECT(header_json,"device");
	if(device_json == NULL){
		DMCLOG_E("access json NULL");
		ret = -1;
		goto EXIT1;
	}
	p_client_info->device = JSON_GET_OBJECT_VALUE(device_json, int);

	JObj *appid_json = JSON_GET_OBJECT(header_json, "appid");
	if(appid_json == NULL){
		DMCLOG_E("access json NULL");
		ret = -1;
		goto EXIT1;
	}
	p_client_info->appid = JSON_GET_OBJECT_VALUE(appid_json, int);
*/	
	p_client_info->code =0;
	//DMCLOG_D("header.cmd = %d",p_client_info->cmd);
	
EXIT1:
	if(p_client_info->r_json != NULL)
		JSON_PUT_OBJECT(p_client_info->r_json);
EXIT2:
	safe_free(rcv_buf);
EXIT3:
	return ret;
}



