/************************************************************************
#
#  Copyright (c) 2015-2016 longsys(SHENTHEN) Co., Ltd.
#  All Rights Reserved
#
#  author: Oliver
#  create date: 2015-3-19
# 
# Unless you and longsys execute a separate written software license 
# agreement governing use of this software, this software is licensed 
# to you under the terms of the GNU General Public License version 2 
# (the "GPL"), with the following added to such license:
# 
#    As a special exception, the copyright holders of this software give 
#    you permission to link this software with independent modules, and 
#    to copy and distribute the resulting executable under terms of your 
#    choice, provided that you also meet, for each linked independent 
#    module, the terms and conditions of the license of that module. 
#    An independent module is a module which is not derived from this
#    software.  The special exception does not apply to any modifications 
#    of the software.  
# 
# Not withstanding the above, under no circumstance
s may you combine 
# this software in any way with any other longsys software provided 
# under a license other than the GPL, without longsys's express prior 
# written consent. 
#
# Revision Table
#
# Version     | Name             |Date           |Description
# ------------|------------------|---------------|-------------------
#  0.1.0.1    |Oliver       |2015-3-19    |Trial Version
#
*************************************************************************/

#ifndef __MSG_SERVER_H__
#define __MSG_SERVER_H__

#if __cplusplus
extern "C" {
#endif

/*############################## Includes ####################################*/
#include "base.h"

#define RECV_TIMEOUT        3000
#define HEART_BEAT_PORT 27212



typedef struct _ClientThreadInfo
{
	struct sockaddr_in clientAddr;
	struct sockaddr_in clientAddrSend;
	void *client_arg; // for future extend!
    int client_fd;  // for tcp or cgi_forward udp
	int client_fd_send;  // for tcp or cgi_forward udp
    // for recv
    char *recv_buf;
    uint16_t recv_buf_len;
    JObj *header_json;
    JObj *r_json;
    JObj *s_json;
	uint32_t cmd;
	uint32_t seq;
	uint16_t error;
	int acc_fd;
	char *retstr;
	uint16_t time_out;
	//recv para
	char ver[32];
	char username[128];
	char password[128];
	uint32_t cur_time;
	char client_ip[32];
	int statusFlag;
	uint16_t client_port;
	uint32_t offset;
	uint32_t length;
	char ip[32];
    uint16_t _ver;
    uint16_t device;
    uint16_t appid;
	uint16_t code;
}ClientTheadInfo;





/*############################## Prototypes ##################################*/

#if __cplusplus
}
#endif

#endif /* __MSG_SERVER_H__ */

