/*
 * =============================================================================
 *
 *       Filename:  process_json.c
 *
 *    Description:  process json data
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




int _parse_client_header_json(ClientTheadInfo *p_client_info)
{
	int res = 0;
	char *rcv_buf = NULL;
	int rcv_len = strlen(p_client_info->recv_buf);
	rcv_buf = (char *)malloc(sizeof(char)*(rcv_len + 1));
	strcpy(rcv_buf,p_client_info->recv_buf);
	p_client_info->r_json = JSON_PARSE(rcv_buf);
	if(p_client_info->r_json == NULL)
	{
		DMCLOG_D("access NULL");
		p_client_info->error = INVALIDE_COMMAND;
		goto exit;
	}
	if(is_error(p_client_info->r_json))
	{
		DMCLOG_D("### error:post data is not a json string");
		p_client_info->error = INVALIDE_COMMAND;
		goto exit;
	}
	
	JObj *header_json = JSON_GET_OBJECT(p_client_info->r_json,"header");
	if(header_json == NULL)
	{
		p_client_info->error = INVALIDE_COMMAND;
		goto exit;
	}
	JObj *cmd_json = JSON_GET_OBJECT(header_json,"cmd");
	JObj *session_json = JSON_GET_OBJECT(header_json,"session");
	JObj *seq_json = JSON_GET_OBJECT(header_json,"seq");

	if(cmd_json != NULL)
		p_client_info->cmd = JSON_GET_OBJECT_VALUE(cmd_json,int);
	if(session_json != NULL)
		strcpy(p_client_info->session,JSON_GET_OBJECT_VALUE(session_json,string));
	if(seq_json != NULL)
		p_client_info->seq = JSON_GET_OBJECT_VALUE(seq_json,int);

	DMCLOG_D("p_client_info->cmd = %d",p_client_info->cmd);
	if(p_client_info->cmd == FN_DM_LOGIN)
	{
		JObj *data_json = JSON_GET_OBJECT(p_client_info->r_json,"data");
		if(data_json == NULL)
		{
			p_client_info->error = INVALIDE_COMMAND;
			goto exit;
		}
		JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
		if(para_json == NULL)
		{
			p_client_info->error = INVALIDE_COMMAND;
			goto exit;
		}
		JObj *username_json = JSON_GET_OBJECT(para_json,"username");
		JObj *password_json = JSON_GET_OBJECT(para_json,"password");
		JObj *deviceType_json = JSON_GET_OBJECT(para_json,"deviceType");
		DMCLOG_D("header.cmd = %d",p_client_info->cmd);
		if(username_json == NULL||password_json == NULL||deviceType_json == NULL)
		{
			p_client_info->error = INVALIDE_COMMAND;
			goto exit;
		}
		strcpy(p_client_info->username,JSON_GET_OBJECT_VALUE(username_json,string));
		strcpy(p_client_info->password,JSON_GET_OBJECT_VALUE(password_json,string));
		p_client_info->deviceTpye = JSON_GET_OBJECT_VALUE(deviceType_json,int);
	}else if(p_client_info->cmd == FN_DM_LOGOUT)
	{
		JObj *data_json = JSON_GET_OBJECT(p_client_info->r_json,"data");
		if(data_json == NULL)
		{
			p_client_info->error = INVALIDE_COMMAND;
			goto exit;
		}
		JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
		if(para_json == NULL)
		{
			p_client_info->error = INVALIDE_COMMAND;
			goto exit;
		}
		JObj *session_json = JSON_GET_OBJECT(para_json,"session");
		DMCLOG_D("header.cmd = %d",p_client_info->cmd);
		if(session_json == NULL)
		{
			p_client_info->error = INVALIDE_COMMAND;
			goto exit;
		}
		char *session = JSON_GET_OBJECT_VALUE(session_json,string);
		if(session == NULL)
		{
			DMCLOG_D("session is NULL");
			p_client_info->error = INVALIDE_COMMAND;
			goto exit;
		}else if(strlen(session) < 16)
		{
			DMCLOG_D("session is normal");
			p_client_info->error = INVALIDE_COMMAND;
			goto exit;
		}
		memset(p_client_info->session,0,sizeof(p_client_info->session));
		strcpy(p_client_info->session,session);
	}else if(p_client_info->cmd == FN_DEL_CLIENT_INFO)
	{
		DMCLOG_D("p_client_info->cmd = %d",p_client_info->cmd);
		JObj *data_json = JSON_GET_OBJECT(p_client_info->r_json,"data");
		if(data_json == NULL)
		{
			p_client_info->error = INVALIDE_COMMAND;
			goto exit;
		}
		JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
		if(para_json == NULL)
		{
			p_client_info->error = INVALIDE_COMMAND;
			goto exit;
		}
		JObj *ip_json = JSON_GET_OBJECT(para_json,"ip");
		if(ip_json == NULL)
		{
			p_client_info->error = INVALIDE_COMMAND;
			goto exit;
		}
		strcpy(p_client_info->client_ip, JSON_GET_OBJECT_VALUE(ip_json,string));
		DMCLOG_D("cleint ip = %s",p_client_info->client_ip);
	}
	else if(p_client_info->cmd == FN_ROUTER_SET_STATUS_CHANGED_LISTENER)
	{
		DMCLOG_D("p_client_info->cmd = %d",p_client_info->cmd);
		JObj *data_json = JSON_GET_OBJECT(p_client_info->r_json,"data");
		if(data_json == NULL)
		{
			p_client_info->error = INVALIDE_COMMAND;
			goto exit;
		}
		JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
		if(para_json == NULL)
		{
			p_client_info->error = INVALIDE_COMMAND;
			goto exit;
		}
		JObj *port_json = JSON_GET_OBJECT(para_json,"port");
		JObj *statusFlag_json = JSON_GET_OBJECT(para_json,"statusFlag");
		if(port_json == NULL||statusFlag_json == NULL)
		{
			p_client_info->error = INVALIDE_COMMAND;
			goto exit;
		}
		p_client_info->client_port = JSON_GET_OBJECT_VALUE(port_json,int);
		p_client_info->statusFlag = JSON_GET_OBJECT_VALUE(statusFlag_json,int);
		DMCLOG_D("port = %d",p_client_info->client_port);
	}
	
exit:
	DMCLOG_D("header.cmd = %d",p_client_info->cmd);
	if(p_client_info->r_json != NULL)
			JSON_PUT_OBJECT(p_client_info->r_json);
	safe_free(rcv_buf);
	return p_client_info->error;
}

/*combination the send para to json format*/


