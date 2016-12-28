/************************************************************************
#
#  Copyright (c) 2015-2016  longsys(SHENTHEN) Co., Ltd.
#  All Rights Reserved
#
#  author: Oliver
#  create date: 2015-4-9
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
# Not withstanding the above, under no circumstances may you combine 
# this software in any way with any other longsys software provided 
# under a license other than the GPL, without longsys's express prior 
# written consent. 
#
#
*************************************************************************/


/*############################## Includes ####################################*/
#include <unistd.h> 
#include <stdio.h> 
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>             
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>           
#include <sys/stat.h>
#include <signal.h>
#include <string.h> 
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include "dm_monitor.h"
#include "udp_broadcast.h"
#include "msg.h"
#include "base.h"
#include "defs.h"
#ifdef SUPPORT_OPENWRT_PLATFORM
#include <uci.h>
#include "uci_for_cgi.h"


/*############################## Global Variable #############################*/
static int InternetFlag = 0;


/*############################## Functions ###################################*/
int check_internet_status(void)
{
	p_debug("access get_wwanip\n");
	char value[64]= "\0";
	char uci_option_str[64]= "\0";
	struct ifreq ifr_wlan1;
	int inet_sock_wlan1;
	int ret = 0;

	ctx=uci_alloc_context();
	memset(uci_option_str, '\0', 64);
	memset(value, '\0', 64);
	strcpy(uci_option_str, "network.wan.workmode"); 
	uci_get_option_value(uci_option_str, value);
	uci_free_context(ctx);

	p_debug("value = %s\n",value);
	if(!strcmp(value, "0"))
		strcpy(ifr_wlan1.ifr_name, "eth0.1");
	else if(!strcmp(value, "1"))
		strcpy(ifr_wlan1.ifr_name, "apcli0");
	else if(!strcmp(value, "2"))
		strcpy(ifr_wlan1.ifr_name, "ppp0");
	else{
		p_debug("unknown workmode\n");
		return -1;
	}
	
	inet_sock_wlan1 = socket(AF_INET, SOCK_DGRAM, 0);
	if(-1 == inet_sock_wlan1)
	{
		p_debug("creat socket error!\n");
		ret = -1;
	}
	if (ioctl(inet_sock_wlan1, SIOCGIFADDR, &ifr_wlan1) <  0)
	{
		p_debug("get no wan ip!\n");
		ret = 0;
	}else
	{
		p_debug("get wan ip success!\n");
		ret = 1;
	}
	if(-1 != inet_sock_wlan1)
		close(inet_sock_wlan1);
	
	return ret;
}


int udp_send(void)
{
	p_debug("udp_send");
	struct sockaddr_in addr;
	int sockfd, len = 0;
	int addr_len = sizeof(struct sockaddr_in);
	char buffer[256];
	int broadcast = 1;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		p_debug("creat socket error");
		return -1;
	}

	if(-1 == setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,&broadcast,sizeof(broadcast)))
	{
		p_debug("setsockopt error");
		close(sockfd);
		return -1;
	}
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(BROADCAST_PORT);
	addr.sin_addr.s_addr = INADDR_BROADCAST;

    sprintf(buffer, "{\"header\":{\"cmd\":%d, \"seq\":%d, \"ver\":%d, \"device\":%d, \"appid\":%d, \
		\"code\":%d}, \"data\":[]}", BROADCAST_CMD, BROADCAST_SEQ, BROADCAST_VER, BROADCAST_DEVICE, \
		BROADCAST_APPID);
		
	p_debug("access sendto buffer = %s", buffer);
    if(-1 == sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, addr_len))
    {
		p_debug("sendto");
		close(sockfd);
		return -1;
	}

	close(sockfd);
	return 0;
}


/*combination the send para to json format*/
int comb_send_buf(SINT8 **pBuf)
{
    char retstr[256];
    int res_sz = 0;
	char *buffer = NULL;
	
    memset(retstr,0,256);
    //sprintf(retstr,"{\"data\": [ { \"ip\":%s}], \"header\": {\"code\": 0, \"device\": 1,\"cmd\": %d, \"appid\": 0, \"ver\": 0}}", BROADCAST_CMD, BROADCAST_CMD);//p_client_info->ip,p_client_info->cmd);
	sprintf(retstr, "{\"header\":{\"cmd\":%d, \"seq\":%d, \"ver\":%d, \"device\":%d, \"appid\":%d, \"code\":%d}, \"data\":[]}", \
		BROADCAST_CMD, BROADCAST_SEQ, BROADCAST_VER, BROADCAST_DEVICE, BROADCAST_APPID);
	res_sz = strlen(retstr);
	p_debug("retstr = %s, res_sz = %d", retstr,  res_sz);
	buffer = (char*)malloc(res_sz + 1);
	if(buffer == NULL)
    {
    	p_debug("pBuf error");
        return -1;
    }
	strcpy(buffer, retstr);
	p_debug("buffer = %s", buffer);
	
    *pBuf = buffer;
	return 0;
}

/*******************************************************************************
 * Function:
 *    int get_ap_ip(char *apIp)
 * Description:
 *    This function get the ip of AP
 * Parameters:
 *    apIp   (OUT) The ip of AP 
 * Returns:
 *    The result
 *    0:have ip ;others,error
 *******************************************************************************/
int get_ap_broadcast_ip(char *apIp)
{
	
	char *p = NULL;
	struct ifreq ifr_wlan1;	
	int inet_sock_wlan1;
	int len;
	int i= 0,j = 0;
	char ip_buf[IP_LEN];
	char *ip = ip_buf;
	
	strcpy(ifr_wlan1.ifr_name, "br-lan");
	inet_sock_wlan1 = socket(AF_INET, SOCK_DGRAM, 0);
	if(-1 == inet_sock_wlan1)
	{
		p_debug("creat socket error!\n");
		return -1;
	}
	if (ioctl(inet_sock_wlan1, SIOCGIFADDR, &ifr_wlan1) < 0)
	{
		p_debug("get lan ip error!\n");
		close(inet_sock_wlan1);
		return -1;
	}
	else{
		memset(ip, '\0', IP_LEN);
		ip=inet_ntoa(((struct sockaddr_in*)&(ifr_wlan1.ifr_addr))->sin_addr);
	}
	p_debug("ip = %s\n", ip);
	close(inet_sock_wlan1);

	p = strrchr(ip, '.');
	strcpy(p+1, "255");
	strcpy(apIp, ip);
	p_debug("apIp = %s\n", apIp);
	
	return 0;
}

/*******************************************************************************
 * Function:
 *    get_wwan_client_ip(char *wwanClientIp)
 * Description:
 *    This function get the ip of wired wan or client
 * Parameters:
 *    apIp   (OUT) The ip of wwan or client
 * Returns:
 *    The result
 *    0:have ip;1: no ip; others,error
 *******************************************************************************/
int get_wwan_client_broadcast_ip(char *wwanClientIp)
{
	p_debug("access get_wwan_client_broadcast_ip\n");
	//char *ip = NULL;
	char *p = NULL;
	char value[64]= "\0";
	char uci_option_str[64]= "\0";
	struct ifreq ifr_wlan1;
	int inet_sock_wlan1;
	char ip_buf[IP_LEN];
	char *ip = ip_buf;

	ctx=uci_alloc_context();
	memset(uci_option_str, '\0', 64);
	memset(value, '\0', 64);
	strcpy(uci_option_str, "network.wan.workmode"); 
	uci_get_option_value(uci_option_str, value);
	uci_free_context(ctx);

	p_debug("value = %s\n",value);
	memset(ip, '\0', IP_LEN);
	if(!strcmp(value, "0"))
		strcpy(ifr_wlan1.ifr_name, "eth0.1");
	else if(!strcmp(value, "1"))
		strcpy(ifr_wlan1.ifr_name, "apcli0");
	else{
		p_debug("unknown or other workmode\n");
		return -1;
	}
	
	inet_sock_wlan1 = socket(AF_INET, SOCK_DGRAM, 0);
	if(-1 == inet_sock_wlan1)
	{
		p_debug("creat socket error!\n");
		return -1;
	}
	if (ioctl(inet_sock_wlan1, SIOCGIFADDR, &ifr_wlan1) <  0)
	{
		p_debug("get wwan ip error!\n");
		close(inet_sock_wlan1);
		return -1;
	}else
	{
		memset(ip, '\0', IP_LEN);
		ip=inet_ntoa(((struct sockaddr_in*)&(ifr_wlan1.ifr_addr))->sin_addr);
	}
	close(inet_sock_wlan1);
	
	p = strrchr(ip, '.');
	strcpy(p+1, "255");
	strcpy(wwanClientIp, ip);
	p_debug("wwanClientIp = %s\n", wwanClientIp);
	return 0;
}

/*******************************************************************************
 * Function:
 *    int send_broadcast(char *ip)
 * Description:
 *    This function send broadcast to the ip
 * Parameters:
 *    ip   (IN) The ip 
 * Returns:
 *    The result
 *    0:success;others,error
 *******************************************************************************/
int send_broadcast(char *ip)
{
	p_debug("send_broadcast>>>>>>>>>>>>>");
	SINT32 enRet = 0;
	ClientTheadInfo p_client_info;
	char *pBuf = NULL;

	strcpy(p_client_info.ip, ip);
	p_client_info.client_fd = DM_UdpClientInit(AF_INET, BROADCAST_PORT, SOCK_DGRAM, p_client_info.ip, &p_client_info.clientAddr);
	if(p_client_info.client_fd <= 0)
	{
		p_debug("client_fd = %d\n",p_client_info.client_fd);
		return -1;
	}
	
	enRet = comb_send_buf(&pBuf);
	if(enRet < 0)
	{
		p_debug("comb_send_buf error");
		return -1;
	}
	p_client_info.send_buf = pBuf;
	
	p_debug("p_client_info.send_buf=%s",p_client_info.send_buf);
	enRet = DM_UdpSend(p_client_info.client_fd, p_client_info.send_buf, strlen(p_client_info.send_buf),&p_client_info.clientAddr);
	if(enRet == -1)
	{
		p_debug("sendto fail,errno = %d\n",errno);
		return -1;
	}

	safe_free(p_client_info.send_buf);	
	DM_DomainClientDeinit(p_client_info.client_fd);
	return 0;	
}

/*******************************************************************************
 * Function:
 *    int upd_broadcast(void)
 * Description:
 *    This function send broadcast for AP/wwan/client
 * Parameters:
 *    
 * Returns:
 *    The result
 *    0:success;others,error
 *******************************************************************************/
int _upd_broadcast(void)
{	
	char apIp[IP_LEN] = "\0";
	char wwanClientIp[IP_LEN] = "\0";

	/****AP broadcast*****/
	memset(apIp, '\0', IP_LEN);
	if(get_ap_broadcast_ip(apIp) == 0)
	{	
		p_debug("1111");
		p_debug("apIp = %s\n", apIp);
		if(send_broadcast(apIp) == 0)
		{
			p_debug("ap broadcast success");
		}
		else
		{
			p_debug("ap broadcast error");
		}
	}
	else
	{
		p_debug("get ap ip error");	
	}

	/****wwan and client broadcast****/
	memset(wwanClientIp, '\0', IP_LEN);
	if(get_wwan_client_broadcast_ip(wwanClientIp) == 0)
	{
		p_debug("wwanClientIp = %s\n", wwanClientIp);
		if(send_broadcast(wwanClientIp) == 0)
		{
			p_debug("wwan client broadcast success");
		}
		else
		{
			p_debug("wwan client broadcast error");
		}
	}
	else
	{
		p_debug("get wan client ip error");
	}
	return 0;
}


int upd_broadcast(void)
{
	int i = 0;
	int old_internet_status = InternetFlag;
	int current_internet_status = check_internet_status();
	if(-1 == current_internet_status)
	{
		p_debug("get internet status error");
		return -1;
	}
	else
	{
		p_debug("old_internet_status = %d;current_internet_status = %d", old_internet_status, current_internet_status);
		if((old_internet_status == 0) && (current_internet_status == 1))
		{
			usleep(500000);
			current_internet_status = check_internet_status();
			if(current_internet_status == 1)
			{
				p_debug("need udp broadcast");
				for(i = 0; i < BROADCAST_TIME; i++)
				{
					_upd_broadcast();
					usleep(100000);
				}
			}else if(-1 == current_internet_status)
				{
					p_debug("get internet status error");
					return -1;
				}
		}
		InternetFlag = current_internet_status;
	}
	return 0;
}
#endif


