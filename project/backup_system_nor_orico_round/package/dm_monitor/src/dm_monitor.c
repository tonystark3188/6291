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
#include "dm_monitor.h"
#include "base.h"
#include "defs.h"

/*############################## Global Variable #############################*/

/*############################## Functions ###################################*/

static int shell_system_cmd(const char * cmd,char *cmd_result) 
{ 
	FILE * fp; 
	char buffer[512];
	int chars_read=0;
	if (cmd == NULL) 
	{ 
		printf("my_system cmd is NULL!\n");
		return -1;
	}
	if ((fp = popen(cmd, "r") ) == NULL) 
	{ 
		perror("popen");
		return -1;
	} 
	else
	{	
		memset(buffer,0,512);
		chars_read=fread(buffer, sizeof(char), 512-1, fp); 
		//printf("read buffer=%s %d\n",buffer,chars_read);
		if(chars_read>0&&cmd_result!=NULL&&strcmp(cmd_result,"NULL")!=0)
		{
			if(buffer[chars_read-1]=='\r'||buffer[chars_read-1]=='\n')
			{
				strncpy(cmd_result,buffer,chars_read-1);
				pclose(fp);
				return chars_read-1;
			}
			else
				strncpy(cmd_result,buffer,chars_read);
		}
		if (pclose(fp)<0) 
		{ 
			printf("close popen file pointer fp error!,errno = %d\n",errno); return -1;
		} 
	}
	//printf("get Valuse=%s\n",cmd_result);
	return chars_read;
} 
int DM_GetHidiskVer()
{
    int ret = 0;
    ClientTheadInfo p_client_info;
    memset(&p_client_info,0,sizeof(ClientTheadInfo));
    p_client_info.cmd = 0x0001;
    p_client_info.error = 0;
    p_client_info.tcp_server_port = 13111;
    strcpy(p_client_info.ip,"127.0.0.1");
    ret = _handle_client_json_req(&p_client_info);
    if(ret != 0)
    {
        return -1;
    }
    return ret;
}

int is_app_alive(char *app_name,char *retstr)
{
	int is_exit;
	char cmd_buf[512];
	memset(cmd_buf,0,512);
	sprintf(cmd_buf,"ps -w |grep %s | grep -v grep | awk '{print $4}'",app_name);
	is_exit=shell_system_cmd(cmd_buf,retstr);
	return is_exit;
}


int restart_app(char *app_name)
{
	char cmd_buf[64];
	memset(cmd_buf,0,64);
	sprintf(cmd_buf,"%s &",app_name);
	p_debug("cmd_buf = %s",cmd_buf);
	system(cmd_buf);
	return 0;
}

int kill_zombie_app(char *app_name)
{
	int is_exit;
	char cmd_buf[512];
	char retstr[64];
	memset(cmd_buf,0,512);
	memset(retstr,0,64);
	sprintf(cmd_buf,"ps -w |grep %s | grep -v grep | awk '{print $1}'",app_name);
	is_exit=shell_system_cmd(cmd_buf,retstr);
	if(is_exit > 0){
		printf("retstr3 = %s\n",retstr);
		memset(cmd_buf,0,512);
		sprintf(cmd_buf,"kill -9 %s",retstr);
		system(cmd_buf);
		return 0;
	}
	else{
		return 1;
	}
}

int count_close_wait()
{
	int ret = 0;
	int count;
	char cmd_buf[512];
	char retstr[64];
	sprintf(cmd_buf,"netstat -apn | grep dm_server | grep CLOSE_WAIT | wc -l");
	ret = shell_system_cmd(cmd_buf,retstr); 
	if(ret > 0)
	{
		count = atoi(retstr);
	}
	return count;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	char *dm_app_name = "dm_router";
	char retstr[32];
	signal(SIGCHLD, SIG_IGN); 
	while(1)
	{
		sleep(3);
		if(DM_GetHidiskVer() < 0)
		{
			p_debug("get fw heart error");
			memset(retstr,0, 32);
			ret = is_app_alive(dm_app_name,retstr);
			if(ret <= 0)
			{
				restart_app(dm_app_name);
			}else{
				if(retstr[0] == 'T'||count_close_wait() > 1)
				{
					if(0 == kill_zombie_app(dm_app_name))
						restart_app(dm_app_name);
				}
			}
		}
		#ifdef SUPPORT_OPENWRT_PLATFORM
		if(upd_broadcast() < 0)
		{
			p_debug("udp broadcast error!");
		}
		else		
		{
			p_debug("udp broadcast ok!");	
		}
		#endif
	}
    return ret;
}
