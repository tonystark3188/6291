/************************************************************************
#
#  Copyright (c) 2014-2016  DAMAI(SHENTHEN) Co., Ltd.
#  All Rights Reserved
#
#  author: Oliver
#  create date: 2015-8-20
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
#include "network/net_util.h"
#include "router_inotify.h"
#include "defs.h"
#include "base.h"
#include "db_server_prcs.h"
#include "router_cycle.h"
#include "config.h"
#include "server.h"

static const char	*config_file = CONFIG;

extern inotify_power_info_t power_info_global;
int	exit_flag;
static void
signal_handler(int sig_num)
{
	switch (sig_num) {
#ifndef _WIN32
	case SIGCHLD:
		while (waitpid(-1, &sig_num, WNOHANG) > 0) ;
		break;
#endif /* !_WIN32 */
	default:
		DMCLOG_D("exit_flag = %d",sig_num);
		exit_flag = sig_num;
		break;
	}
}

/*****************************************router module start*******************************************************************/
void router_discovery_prcs_task(void)
{
	ENTER_FUNC();
	if(_udp_listen_clients() < 0){
		DMCLOG_E("udp listen server error!");
	}
	EXIT_FUNC();
    return;
}


/*****************************************router module start*******************************************************************/
void router_server_prcs_task(void)
{
	ENTER_FUNC();
	int res = 0;
	res = mkdir(INOTIFY_DIR_PATH, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	char *inotify_path = INOTIFY_DIR_PATH;
	int pnTimeOut = 3000;
	res = notify_router_para(inotify_path,pnTimeOut);
	EXIT_FUNC();
    return;
}
/*****************************************router module end***********************************************************/


void file_server_prcs_task(void)
{	
	struct shttpd_ctx	*ctx;
	ctx = shttpd_init();
	open_listening_ports(ctx);
	DMCLOG_D("dm_server started on port(s) %d,%d,%d", get_sys_init_port(),get_sys_router_port(), get_sys_file_port());
	while (exit_flag == 0)
		shttpd_poll(ctx, 5000);
	shttpd_fini(ctx);
	DMCLOG_D("exit succ");
	return;
	
}


static void set_option(const char *opt,const char *val)
{
	extern SystemConfigInfo *g_p_sys_conf_info;
	g_p_sys_conf_info       = &g_sys_info.sys_conf_info;
	if(strcmp(opt,AIRDISK_DM_ROUTER_VERSION)==0)
	{
		strcpy(g_p_sys_conf_info->router_ver,val);
	}else if(strcmp(opt,AIRDISK_PRO_DB_VERSION)==0)
	{
		strcpy(g_p_sys_conf_info->db_ver,val);
	}else if(strcmp(opt,AIRDISK_PRO_FW_VERSION)==0)
	{
		strcpy(g_p_sys_conf_info->fw_ver,val);
	}else if(strcmp(opt,FILE_LISTENING_PORT)==0)
	{
		g_p_sys_conf_info->airdisk_file_port= atoi(val);
	}else if(strcmp(opt,ROUTER_LISTENING_PORT)==0)
	{
		g_p_sys_conf_info->airdisk_router_port= atoi(val);
	}else if(strcmp(opt,INIT_LISTENING_PORT)==0)
	{
		g_p_sys_conf_info->airdisk_init_port= atoi(val);
	}else if(strcmp(opt,DATABASE_NAME)==0)
	{
		strcpy(g_p_sys_conf_info->db_name,val);
	}else if(strcmp(opt,DISK_UUID_NAME)==0)
	{
		strcpy(g_p_sys_conf_info->uuid_name,val);
	}else if(strcmp(opt,POWER_ACCESS)==0)
	{
		if(atoi(val) == 1)
		{
			g_p_sys_conf_info->fun_list_flag |= 0x0001;
		}
	}else if(strcmp(opt,WIFI_ACCESS)==0)
	{
		if(atoi(val) == 1)
		{
			g_p_sys_conf_info->fun_list_flag |= 0x0002;// 2
		}
	}
	else if(strcmp(opt,REMOTEAP_ACCESS)==0)
	{
		if(atoi(val) == 1)
		{
			g_p_sys_conf_info->fun_list_flag |= 0x0004;
		}
	}
	else if(strcmp(opt,FILE_TYPE_ACCESS)==0)
	{
		if(atoi(val) == 1)
		{
			g_p_sys_conf_info->fun_list_flag |= 0x0008;//8
		}
	}
	else if(strcmp(opt,BACKUP_ACCESS)==0)
	{
		if(atoi(val) == 1)
		{
			g_p_sys_conf_info->fun_list_flag |= 0x0010;//16
		}
	}
	else if(strcmp(opt,COPY_TO_ACCESS)==0)
	{
		if(atoi(val) == 1)
		{
			g_p_sys_conf_info->fun_list_flag |= 0x0020;//32
		}
	}
	else if(strcmp(opt,FILE_LIST_ACCESS)==0)
	{
		if(atoi(val) == 1)
		{
			g_p_sys_conf_info->fun_list_flag |= 0x0040;//64
		}
	}
	else if(strcmp(opt,PRODUCT_MODEL)==0)
	{
		strcpy(g_p_sys_conf_info->product_model,val);
	}
	else{
		DMCLOG_E("unknow opt:%s", opt);
	}
	
	DMCLOG_D("fun list flag = %d",g_p_sys_conf_info->fun_list_flag);
}


static void
initialize_config(const char *config_file)
{
	char			line[FILENAME_MAX], root[FILENAME_MAX],
					var[sizeof(line)], val[sizeof(line)];
	const char		*arg;
	size_t			i;
	FILE 			*fp;
	/* Second pass: load config file  */
	if (config_file != NULL && (fp = fopen(config_file, "r")) != NULL) {
		DMCLOG_D("init_ctx: config file %s", config_file);

		/* Loop through the lines in config file */
		while (fgets(line, sizeof(line), fp) != NULL) {

			/* Skip comments and empty lines */
			if (line[0] == '#' || line[0] == '\n')
				continue;

			/* Trim trailing newline character */
			line[strlen(line) - 1] = '\0';

			if (sscanf(line, "%s %[^#\n]", var, val) != 2)
				DMCLOG_D("init_ctx: bad line: [%s]",line);
			DMCLOG_D("var = %s,val = %s",var,val);
			set_option(var,val);
		}
		(void) fclose(fp);
	}
	/* Third pass: process command line args */
}


int child_fun()
{
    int rslt = 0;
	(void) signal(SIGTERM, signal_handler);
	(void) signal(SIGINT, signal_handler);
	InitializeCriticalSection(&power_info_global.mutex);
    /* 云通信模块发过来的消息的处理线程 */
	PTHREAD_T tid_discovery_server;
	PTHREAD_T tid_router_server;
	PTHREAD_T tid_file_server;
	#ifdef DB_TASK
	PTHREAD_T tid_db_server;
	#endif
	initialize_config(config_file);
	if (0 != (rslt = PTHREAD_CREATE(&tid_discovery_server, NULL, (void *)router_discovery_prcs_task, NULL)))
    { 
        DMCLOG_D("Create router discovery msg prcs thread failed!");
        rslt = -1;
        goto quit;
    }
	
	if (0 != (rslt = PTHREAD_CREATE(&tid_router_server, NULL, (void *)router_server_prcs_task, NULL)))
    { 
        DMCLOG_D("Create router server msg prcs thread failed!");
        rslt = -1;
        goto quit;
    }
	if (0 != (rslt = PTHREAD_CREATE(&tid_file_server, NULL, (void *)file_server_prcs_task, NULL)))
    { 
        DMCLOG_D("Create file server msg prcs thread failed!");
        rslt = -1;
        goto quit;
	}
	#ifdef DB_TASK
	if (0 != (rslt = PTHREAD_CREATE(&tid_db_server, NULL, (void *)db_server_prcs_task,NULL)))
    { 
        DMCLOG_D("Create db server msg prcs thread failed!");
        rslt = -1;
        goto quit;
    }
	#endif
	PTHREAD_DETACH(tid_discovery_server);
	PTHREAD_DETACH(tid_router_server);
	PTHREAD_DETACH(tid_file_server);
	#ifdef DB_TASK
	PTHREAD_DETACH(tid_db_server);
	#endif
    /* forever,可做异常退出或其他处理 */
   	dm_router_cycle();
quit:
	DestroyCriticalSection(&power_info_global.mutex);
    DMCLOG_D("----------------msg server main task quit!----------");
    return rslt;
}


void process_exit(int s)
{
    exit(0);
}


/*void child_fun()
{
    printf("child_fun. ppid %d\n",getppid());
    char *st = NULL;
    strcpy(st, "123");
}*/


void fork_child()
{
    pid_t child_process;
    int status;
    int signal_num;
    wait(&status);//等待子进程中断或终止，释放子进程资源，否则死掉的子进程会变成僵尸进程
  
    //如果子进程是由于某种信号退出的，捕获该信号
    if(WIFSIGNALED(status))
        signal_num = WTERMSIG(status);

    child_process = fork();
    if(child_process == 0)
    {
        printf("fork new child process.\n");
        child_fun();
    }
}


int main()
{
    pid_t child_process;
    int i = 0;
    while(1)
    {
        printf("fork new process.\n");
        child_process = fork();
        if(child_process > 0)
        {
            while(1)
            {
               //捕获子进程结束信号
                signal(SIGCHLD, fork_child);
                signal(SIGTERM, process_exit);
                pause();//主进程休眠，当有信号到来时被唤醒。
            }
        }
        else if(child_process == 0)
        {
            child_fun();
        }
    }
    return 0;
}


