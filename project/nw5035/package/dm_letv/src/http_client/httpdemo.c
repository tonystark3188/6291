/************************************************************************
#
#  Copyright (c) 2015-2016  longsys(SHENTHEN) Co., Ltd.
#  All Rights Reserved
#
#  author: Oliver
#  create date: 2015-3-17
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "http_client.h"
#include "http_letv.h"

typedef int (*API_FUNC)(char* send_buf, char *retstr);
typedef struct _tag_handle
{
 char tag[16];
 API_FUNC tagfun;
}tag_handle;
int handle_query_list(char* send_buf, char *retstr);//query the user's device list
int handle_dev_login(char* send_buf,char *retstr);//login the device
int handle_g_parse_task(char* send_buf, char *retstr);//GET task for parsering information
int handle_p_parse_task(char* send_buf, char *retstr);//POST task for parsering infomation
int handle_g_new_task(char* send_buf, char *retstr);//creat a new task for GET
int handle_p_new_task(char* send_buf, char *retstr);//creat a new task fir POST
int handle_dev_bind(char* send_buf, char *retstr);//device binding
int handle_dev_unbind(char* send_buf, char *retstr);//device unbinding


#define FN_DEV_QUERY_LIST "query"
#define FN_DEV_LOGIN "login"
#define FN_DEV_G_PARSE_TASK "g_parse"
#define FN_DEV_P_PARSE_TASK "p_parse"
#define FN_DEV_G_NEW_TASK "g_task"
#define FN_DEV_P_NEW_TASK "p_task"
#define FN_DEV_BIND "dev_bind"
#define FN_DEV_UNBIND "dev_unbind"

#if 0
tag_handle all_tag_handle[]=
{
 {FN_DEV_QUERY_LIST,handle_query_list},
 {FN_DEV_LOGIN,handle_dev_login},
 {FN_DEV_G_PARSE_TASK,handle_g_parse_task},
 {FN_DEV_P_PARSE_TASK,handle_p_parse_task},
 {FN_DEV_G_NEW_TASK,handle_g_new_task},
 {FN_DEV_P_NEW_TASK,handle_p_new_task},
 {FN_DEV_BIND,handle_dev_bind},
 {FN_DEV_UNBIND,handle_dev_unbind},
};
#define TAGHANDLE_NUM (sizeof(all_tag_handle)/sizeof(all_tag_handle[0]))
#endif
/*
 * Desc:query the user's device list 
 *
 */
int handle_query_list(char* send_buf, char *retstr)
{
	http_tcpclient	t_client;
	char	page[BUFFER_SIZE];
	int	n, port = 80;
	char	*request = "p1=hello";
	char	*response = NULL;
	int     i = 0;
	char    url[BUFFER_SIZE];
	char    host[BUFFER_SIZE];
	char    *pHost = NULL;
	int 	time_out = 0;
	int 	in = 0;
	char 	*p[8];
	char 	*buf=send_buf;
	while((p[in]=strtok(buf," "))!=NULL) 
	{
		 in++;
		 buf=NULL; 
	}
	printf("Here we have %d strings\n",in);
	for (i=0; i<in; i++)
	 	printf("%s\n",p[i]);
	sprintf(url,"homecloud.yuancheng.xunlei.com/listPeer?v=1&type=1&token=tk10.DEF35D0EEDF72E4744BEEB7C4C2A11E588D3864930664D6CCB19DC8361AFC6AD65FD9BAE8A01C96485103A647611ED58");
	for (i=0,pHost= url;*pHost!='/'&&*pHost!='/0';pHost++,i++){  
            host[i] = *pHost;  
        }  
        host[i] = 0; 
	memset(page,0,BUFFER_SIZE);
	strcpy(page,pHost);
	memset(&t_client,0,sizeof(http_tcpclient));
	time_out = 5000;
	if ((n = http_tcpclient_create(&t_client, host, port,time_out)) < 0) {
		printf("Create socket error.\n");
		return -1;
	}
	if ((n = http_tcpclient_conn(&t_client)) < 0) {
		printf("Connet srv error.\n");
		return -2;
	}
	if (http_get(&t_client, page, "", &response,0)) {
		printf("GET !\n");
		return 1;
	}
	else{
		printf("GET:%d:%s\n",strlen(response),response);
	}
	free(response);
	http_tcpclient_close(&t_client);
	return 0;
}
/*
 * Desc:query the user's device list 
 *
 */
int handle_dev_login(char *send_buf,char *retstr)
{
	printf("handle_dev_login\n");
	http_tcpclient	t_client;
	char	page[BUFFER_SIZE];
	int	n, port = 80;
	char	*request = "p1=hello";
	char	*response = NULL;
	int     i = 0;
	char    url[BUFFER_SIZE];
	char    host[BUFFER_SIZE];
	char    *pHost = NULL;
	int 	time_out = 0;
	int 	in = 0;
	char 	*p[8];
	char 	*buf=send_buf;
	while((p[in]=strtok(buf," "))!=NULL) 
	{
		 in++;
		 buf=NULL; 
	}
	printf("Here we have %d strings\n",in);
	for (i=0; i<in; i++)
	 	printf("%s\n",p[i]);
	sprintf(url,"homecloud.yuancheng.xunlei.com/login?pid=000C432880DE302X0001&callback=&v=1&clientType=400&token=tk10.DEF35D0EEDF72E4744BEEB7C4C2A11E588D3864930664D6CCB19DC8361AFC6AD65FD9BAE8A01C96485103A647611ED58");
	for (i=0,pHost= url;*pHost!='/'&&*pHost!='/0';pHost++,i++){  
            host[i] = *pHost;  
        }  
        host[i] = 0; 
	memset(page,0,BUFFER_SIZE);
	strcpy(page,pHost);
	memset(&t_client,0,sizeof(http_tcpclient));
	time_out = 5000;
	if ((n = http_tcpclient_create(&t_client, host, port,time_out)) < 0) {
		printf("Create socket error.\n");
		return -1;
	}
	if ((n = http_tcpclient_conn(&t_client)) < 0) {
		printf("Connet srv error.\n");
		return -2;
	}
	if (http_get(&t_client, page, "", &response,0)) {
		printf("GET !\n");
		return 1;
	}
	else{
		printf("GET:%d:%s\n",strlen(response),response);
	}
	free(response);
	http_tcpclient_close(&t_client);
	return 0;
}
int handle_g_parse_task(char *send_buf,char *retstr)
{
	printf("access handle_get_task\n");
	http_tcpclient	t_client;
	char	page[BUFFER_SIZE];
	int	n, port = 80;
	char	*request = "p1=hello";
	char	*response = NULL;
	int     i = 0;
	char    url[BUFFER_SIZE];
	char    host[BUFFER_SIZE];
	char    *pHost = NULL;
	int 	time_out = 0;
	int 	in = 0;
	char 	*p[8];
	char 	*buf=send_buf;
	while((p[in]=strtok(buf," "))!=NULL) 
	{
		 in++;
		 buf=NULL; 
	}
	printf("Here we have %d strings\n",in);
	for (i=0; i<in; i++)
	 	printf("%s\n",p[i]);
	sprintf(url,"homecloud.yuancheng.xunlei.com/urlCheck?pid=845DD7D00088800X0001&url=magnet:?xt=urn:btih:463B55861D618F05B22AE7331FC1356BD18D8451&type=1&upload=&v=1&token=tk10.DEF35D0EEDF72E4744BEEB7C4C2A11E588D3864930664D6CCB19DC8361AFC6AD65FD9BAE8A01C96485103A647611ED58");
	for (i=0,pHost= url;*pHost!='/'&&*pHost!='/0';pHost++,i++){  
            host[i] = *pHost;  
        }  
        host[i] = 0; 
	memset(page,0,BUFFER_SIZE);
	strcpy(page,pHost);
	memset(&t_client,0,sizeof(http_tcpclient));
	time_out = 5000;
	if ((n = http_tcpclient_create(&t_client, host, port,time_out)) < 0) {
		printf("Create socket error.\n");
		return -1;
	}
	if ((n = http_tcpclient_conn(&t_client)) < 0) {
		printf("Connet srv error.\n");
		return -2;
	}
	if (http_get(&t_client, page, "", &response,0)) {
		printf("GET !\n");
		return 1;
	}
	else{
		printf("GET:%d:%s\n",strlen(response),response);
	}
	free(response);
	http_tcpclient_close(&t_client);
	return 0;
}
int handle_p_parse_task(char *send_buf,char *retstr)
{
	printf("handle_p_parse_task\n");
	http_tcpclient	t_client;
	char	page[BUFFER_SIZE];
	int	n, port = 80;
	char	*request = "<input name='filepath' type='file' />";
	char	*response = NULL;
	int     i = 0;
	char    url[BUFFER_SIZE];
	char    host[BUFFER_SIZE];
	char    *pHost = NULL;
	int 	time_out = 0;
	int 	in = 0;
	char 	*p[8];
	char 	*buf=send_buf;
	while((p[in]=strtok(buf," "))!=NULL) 
	{
		 in++;
		 buf=NULL; 
	}
	printf("Here we have %d strings\n",in);
	for (i=0; i<in; i++)
	 	printf("%s\n",p[i]);
	sprintf(url,"homecloud.yuancheng.xunlei.com/urlCheck?pid=845DD7D00088800X0001&url=magnet:?xt=urn:btih:463B55861D618F05B22AE7331FC1356BD18D8451&type=2&upload=1&callback=&v=&token=tk10.DEF35D0EEDF72E4744BEEB7C4C2A11E588D3864930664D6CCB19DC8361AFC6AD65FD9BAE8A01C96485103A647611ED58");
	for (i=0,pHost= url;*pHost!='/'&&*pHost!='/0';pHost++,i++){  
            host[i] = *pHost;  
        }  
        host[i] = 0; 
	memset(page,0,BUFFER_SIZE);
	strcpy(page,pHost);
	memset(&t_client,0,sizeof(http_tcpclient));
	time_out = 5000;
	if ((n = http_tcpclient_create(&t_client, host, port,time_out)) < 0) {
		printf("Create socket error.\n");
		return -1;
	}
	if ((n = http_tcpclient_conn(&t_client)) < 0) {
		printf("Connet srv error.\n");
		return -2;
	}
	if (http_post(&t_client, page, request, &response)) {
		printf("POST !");
		return 2;
	}
	else
		printf("POST:%d:%s\n",strlen(response),response);
	free(response);
	http_tcpclient_close(&t_client);
	return 0;
}
int handle_g_new_task(char *send_buf,char *retstr)
{
	printf("access get new task\n");
	http_tcpclient	t_client;
	char	page[BUFFER_SIZE];
	int	n, port = 80;
	char	*request = "p1=hello";
	char	*response = NULL;
	int     i = 0;
	char    url[BUFFER_SIZE];
	char    host[BUFFER_SIZE];
	char    *pHost;
	int 	time_out = 0;
	int 	in = 0;
	char 	*p[8];
	char 	*buf=send_buf;
	while((p[in]=strtok(buf," "))!=NULL) 
	{
		 in++;
		 buf=NULL; 
	}
	printf("Here we have %d strings\n",in);
	for (i=0; i<in; i++)
	 	printf("%s\n",p[i]);
	sprintf(url,"homecloud.yuancheng.xunlei.com/createOne?pid=845DD7D00088800X0001&type=2&url=magnet:?xt=urn:btih:463B55861D618F05B22AE7331FC1356BD18D8451&name=重返20岁.rmvb&path=&v=1&token=tk10.DEF35D0EEDF72E4744BEEB7C4C2A11E588D3864930664D6CCB19DC8361AFC6AD65FD9BAE8A01C96485103A647611ED58");
	for (i=0,pHost= url;*pHost!='/'&&*pHost!='/0';pHost++,i++){  
            host[i] = *pHost;  
        }  
        host[i] = 0; 
	memset(page,0,BUFFER_SIZE);
	strcpy(page,pHost);
	memset(&t_client,0,sizeof(http_tcpclient));
	time_out = 5000;
	if ((n = http_tcpclient_create(&t_client, host, port,time_out)) < 0) {
		printf("Create socket error.\n");
		return -1;
	}
	if ((n = http_tcpclient_conn(&t_client)) < 0) {
		printf("Connet srv error.\n");
		return -2;
	}
	if (http_get(&t_client, page, "", &response,0)) {
		printf("GET !\n");
		return 1;
	}
	else{
		printf("GET:%d:%s\n",strlen(response),response);
	}
	free(response);
	http_tcpclient_close(&t_client);
	return 0;
}
int handle_p_new_task(char *send_buf,char *retstr)
{
	printf("access post new task\n");
	http_tcpclient	t_client;
	char	page[BUFFER_SIZE];
	int	n, port = 80;
	char	*request = "<input name='filepath' type='file' />";
	char	*response = NULL;
	int     i = 0;
	char    url[BUFFER_SIZE];
	char    host[BUFFER_SIZE];
	char    *pHost;
	int 	time_out = 0;
	int 	in = 0;
	char 	*p[8];
	char 	*buf=send_buf;
	while((p[in]=strtok(buf," "))!=NULL) 
	{
		 in++;
		 buf=NULL; 
	}
	printf("Here we have %d strings\n",in);
	for (i=0; i<in; i++)
	 	printf("%s\n",p[i]);
	sprintf(url,"homecloud.yuancheng.xunlei.com/createOne?pid=845DD7D00088800X0001&type=2&url=magnet:?xt=urn:btih:463B55861D618F05B22AE7331FC1356BD18D8451&name=重返20岁.rmvb&name=&path=&btSub=&upload=1&callback=&v=1&token=tk10.DEF35D0EEDF72E4744BEEB7C4C2A11E588D3864930664D6CCB19DC8361AFC6AD65FD9BAE8A01C96485103A647611ED58");
	for (i=0,pHost= url;*pHost!='/'&&*pHost!='/0';pHost++,i++){  
            host[i] = *pHost;  
        }  
        host[i] = 0; 
	memset(page,0,BUFFER_SIZE);
	strcpy(page,pHost);
	memset(&t_client,0,sizeof(http_tcpclient));
	time_out = 5000;
	if ((n = http_tcpclient_create(&t_client, host, port,time_out)) < 0) {
		printf("Create socket error.\n");
		return -1;
	}
	if ((n = http_tcpclient_conn(&t_client)) < 0) {
		printf("Connet srv error.\n");
		return -2;
	}
	if (http_post(&t_client, page, request, &response)) {
		printf("POST !");
		return 2;
	}
	else
		printf("POST:%d:%s\n",strlen(response),response);
	free(response);
	http_tcpclient_close(&t_client);
	return 0;
}
int handle_dev_bind(char *send_buf,char *retstr)
{
	http_tcpclient	t_client;
	char	page[BUFFER_SIZE];
	int	n, port = 80;
	char	*request = "p1=hello";
	char	*response = NULL;
	int     i = 0;
	char    url[BUFFER_SIZE];
	char    host[BUFFER_SIZE];
	char    *pHost;
	int 	time_out = 0;
	int 	in = 0;
	char 	*p[8];
	char 	*buf=send_buf;
	while((p[in]=strtok(buf," "))!=NULL) 
	{
		 in++;
		 buf=NULL; 
	}
	printf("Here we have %d strings\n",in);
	for (i=0; i<in; i++)
	 	printf("%s\n",p[i]);
	sprintf(url,"homecloud.yuancheng.xunlei.com/bind?boxName=xdisk&key=rqcpqd&keyType=20&callback=&v=1&token=tk10.DEF35D0EEDF72E4744BEEB7C4C2A11E588D3864930664D6CCB19DC8361AFC6AD65FD9BAE8A01C96485103A647611ED58");
	for (i=0,pHost= url;*pHost!='/'&&*pHost!='/0';pHost++,i++){  
            host[i] = *pHost;  
        }  
        host[i] = 0; 
	memset(page,0,BUFFER_SIZE);
	strcpy(page,pHost);
	memset(&t_client,0,sizeof(http_tcpclient));
	time_out = 5000;
	if ((n = http_tcpclient_create(&t_client, host, port,time_out)) < 0) {
		printf("Create socket error.\n");
		return -1;
	}
	if ((n = http_tcpclient_conn(&t_client)) < 0) {
		printf("Connet srv error.\n");
		return -2;
	}
	if (http_get(&t_client, page, "", &response,0)) {
		printf("GET !\n");
		return 1;
	}
	else{
		printf("GET:%d:%s\n",strlen(response),response);
	}
	free(response);
	http_tcpclient_close(&t_client);
	return 0;
}
int handle_dev_unbind(char *send_buf,char *retstr)
{
	http_tcpclient	t_client;
	char	page[BUFFER_SIZE];
	int	n, port = 80;
	char	*request = "p1=hello";
	char	*response = NULL;
	int     i = 0;
	char    url[BUFFER_SIZE];
	char    host[BUFFER_SIZE];
	char    *pHost = NULL;
	int 	time_out = 0;
	int 	in = 0;
	char 	*p[8];
	char 	*buf=send_buf;
	while((p[in]=strtok(buf," "))!=NULL) 
	{
		 in++;
		 buf=NULL; 
	}
	printf("Here we have %d strings\n",in);
	for (i=0; i<in; i++)
	 	printf("%s\n",p[i]);
	sprintf(url,"homecloud.yuancheng.xunlei.com/unbind?pid=845DD7D00088800X0001&token=tk10.DEF35D0EEDF72E4744BEEB7C4C2A11E588D3864930664D6CCB19DC8361AFC6AD65FD9BAE8A01C96485103A647611ED58");
	for (i=0,pHost= url;*pHost!='/'&&*pHost!='/0';pHost++,i++){  
            host[i] = *pHost;  
        }  
        host[i] = 0; 
	memset(page,0,BUFFER_SIZE);
	strcpy(page,pHost);
	memset(&t_client,0,sizeof(http_tcpclient));
	time_out = 5000;
	if ((n = http_tcpclient_create(&t_client, host, port,time_out)) < 0) {
		printf("Create socket error.\n");
		return -1;
	}
	if ((n = http_tcpclient_conn(&t_client)) < 0) {
		printf("Connet srv error.\n");
		return -2;
	}
	if (http_get(&t_client, page, "", &response,0)) {
		printf("GET !\n");
		return 1;
	}
	else{
		printf("GET:%d:%s\n",strlen(response),response);
	}
	free(response);
	http_tcpclient_close(&t_client);
	return 0;
}
#if 0
int main()
{
	char	send_buf[BUFFER_SIZE];
	char 	retstr[BUFFER_SIZE];
	char 	*tmp = NULL;
	char 	header[16];
	int 	i = 0;
	int 	switch_flag = 0;
	while(1)
	{
		switch_flag = 0;
		memset(retstr,0,BUFFER_SIZE);
		printf("please enter the content you want to send:\n");
		memset(send_buf, 0, sizeof(send_buf));
		fgets(send_buf, sizeof(send_buf), stdin);
		if(strlen(send_buf) < 4)
		{
			printf("what you want to send is invalid\n");
			continue;
		}
		tmp = strstr(send_buf,"\n");
		if(tmp != NULL)
		{
			send_buf[tmp-send_buf] = 0;
		}
		memset(header,0,16);
		memcpy(header,send_buf,10);
		for(i = 0; i<TAGHANDLE_NUM; i++)
		{
			printf("all_tag_handle[%d].tag = %s\n",i,all_tag_handle[i].tag);
			printf("header = %s\n",header);
			if(strstr(header,all_tag_handle[i].tag))
			{
				printf("send_buf = %s\n",send_buf);
				all_tag_handle[i].tagfun(send_buf,retstr);
				switch_flag = 1;
			}
		}
		if(switch_flag == 0)
		{
			printf("input cmd is not finished!\n");
			continue;
		}
	}
	
	/*char    *url = "homecloud.yuancheng.xunlei.com/listPeer?v=&callback=&type=&token=";
	memset(xl_url,0,BUFFER_SIZE);
	strcpy(xl_url,url);
	for (i=0,pHost= xl_url;*pHost!='/'&&*pHost!='/0';pHost++,i++){  
            host[i] = *pHost;  
        }  
        host[i]='/0'; 
	memset(page,0,BUFFER_SIZE);
	strcpy(page,pHost);
	p_debug("start");
	if ((n = http_tcpclient_create(&t_client, host, port)) < 0) {
		p_debug("Create socket error.");
		return -1;
	}
	if ((n = http_tcpclient_conn(&t_client)) < 0) {
		p_debug("Connet srv error.");
		return -2;
	}
	
	
	if (http_get(&t_client, page, "", &response)) {
		p_debug("GET !");
		return 1;
	}
	else
		p_debug("GET:\n%d:%s",strlen(response),response);
	free(response);

	if (http_post(&t_client, page, request, &response)) {
		p_debug("POST Ê§°Ü!");
		return 2;
	}
	else
		p_debug("POST ÏìÓŠ:\n%d:%s",strlen(response),response);
	free(response);

	if (http_upload(&t_client, page, "fName.txt", &response)) {
		p_debug("UPLOAD Ê§°Ü!");
		return 3;
	}
	else
		p_debug("UPLOAD ÏìÓŠ:\n%d:%s",strlen(response),response);
	free(response);

	http_tcpclient_close(&t_client);*/
	return 0;
}
#endif


/*
int _handle_file_json_req(struct task_dnode *task_dn)
{
	p_debug("access _handle_file_json_req");
	PTHREAD_T ptid;
	//struct task_dnode *client = (struct task_dnode *)malloc(sizeof(struct task_dnode));
    if(task_dn == NULL)
    {
        p_debug("malloc error");
        return -1;
    }
    //memcpy(client,task_dn,sizeof(struct task_dnode));
	p_debug("client->vid = %s", task_dn->vid);
    if(PTHREAD_CREATE(&ptid,NULL, (void *)handle_download_task,&task_dn) != 0)
    {
        safe_free(task_dn);    //have problem
        p_debug("pthread_create error");
        return -1;
    }
	PTHREAD_DETACH(ptid);
	safe_free(task_dn->img_url);
	safe_free(task_dn->vid_re_url);
	safe_free(task_dn->vid_url);
	p_debug("access _handle_file_json_req outout");
    return 0;
}

*/
