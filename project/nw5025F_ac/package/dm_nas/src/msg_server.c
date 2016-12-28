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
#include "base.h"
#include "defs.h"
#include "db_prcs.h"
#include "router_inotify.h"
#include "task_base.h"
#include "mq.h"


static const char	*config_file = CONFIG;

#define FOCK_TIME 5
#define random(x) (rand()%x)

int	exit_flag;
int	safe_exit_flag;

int listen_fd = -1;



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
		exit_flag = sig_num;
		DMCLOG_D("exit_flag = %d",exit_flag);
		//safe_close(listen_fd);
		sleep(2);
		exit(1);
		//break;
	}
}

/*****************************************router module start*******************************************************************/
void router_discovery_prcs_task(void)
{
	if(_udp_listen_clients() < 0){
		DMCLOG_E("udp listen server error!");
	}
    return;
}


/*****************************************router module start*******************************************************************/
void router_server_prcs_task(void)
{
	int res = 0;
	res = mkdir(INOTIFY_DIR_PATH, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	char *inotify_path = INOTIFY_DIR_PATH;
	int pnTimeOut = 1000;
	res = notify_router_para(inotify_path,pnTimeOut);
    return;
}

void file_server_prcs_task(void)
{	
	struct shttpd_ctx	*ctx;
	ctx = shttpd_init();
	open_listening_ports(ctx);
	while (exit_flag == 0)
		shttpd_poll(ctx, 2000);

	DMCLOG_D("quit shttpd poll");
	
	shttpd_fini(ctx);
	safe_close(listen_fd);
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
	}else if(strcmp(opt,SESSION_WATCH_TIME)==0)
	{
		g_p_sys_conf_info->session_watch_time = atoi(val);
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
	else if(strcmp(opt,SAFE_EXIT_ACCESS)==0)
	{
		if(atoi(val) == 1)
		{
			g_p_sys_conf_info->fun_list_flag |= 0x0080;//128
		}
	}
	else if(strcmp(opt,PASSWORD_ACCESS)==0)
	{
		if(atoi(val) == 1)
		{
			g_p_sys_conf_info->fun_list_flag |= 0x0100;//256
		}
	}
	else if(strcmp(opt,FILE_SEARCH_ACCESS)==0)
	{
		if(atoi(val) == 1)
		{
			g_p_sys_conf_info->fun_list_flag |= 0x0400;//1024
		}
	}else if(strcmp(opt,FILE_HIDE_ACCESS)==0)
	{
		if(atoi(val) == 1)
		{
			g_p_sys_conf_info->fun_list_flag |= 1<<12;
		}
	}
	else if(strcmp(opt,ADD_NETWORK_ACCESS)==0)
	{
		if(atoi(val) == 1)
		{
			g_p_sys_conf_info->fun_list_flag |= 0x2000;//8192
		}
	}
	else if(strcmp(opt,PC_MOUNT_ACCESS)==0)
	{
		if(atoi(val) == 1)
		{
			g_p_sys_conf_info->fun_list_flag |= 0x4000;//16384
		}
	}
	else if(strcmp(opt,PRODUCT_MODEL)==0)
	{
		strcpy(g_p_sys_conf_info->product_model,val);
	}
	else{
		DMCLOG_E("unknow opt:%s", opt);
	}
}


static void
initialize_config(const char *config_file)
{
	char			line[128], root[128],
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
			set_option(var,val);
		}
		extern SystemConfigInfo *g_p_sys_conf_info;
		g_p_sys_conf_info->database_sign = random(1000);
		(void) fclose(fp);
	}
	/* Third pass: process command line args */
}



int child_fun()
{
    int rslt = 0;
	(void) signal(SIGTERM, signal_handler);
	(void) signal(SIGINT, signal_handler);
	(void) signal(SIGSEGV,signal_handler);
	(void) signal(SIGABRT,signal_handler);
	(void) signal(SIGPIPE,SIG_IGN);

	initialize_config(config_file);
	_dev_list_init();
	PTHREAD_T tid_discovery_server;
	PTHREAD_T tid_router_server;
	PTHREAD_T tid_file_server;
	PTHREAD_T tid_db_server;
	exit_flag = 0;
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
	
	if (0 != (rslt = PTHREAD_CREATE(&tid_db_server, NULL, (void *)db_prcs_task,NULL)))
    { 
        DMCLOG_D("Create db server msg prcs thread failed!");
        rslt = -1;
        goto quit;
    }
	PTHREAD_DETACH(tid_file_server);
	PTHREAD_DETACH(tid_discovery_server);
	PTHREAD_DETACH(tid_router_server);
	PTHREAD_DETACH(tid_db_server);
    /* forever,可做异常退出或其他处理 */
   do{
		sleep(1);
   }while(exit_flag == 0);
quit:
	_destory_dev_list();
    DMCLOG_D("----------------msg server main task quit!----------");
    return rslt;
}


void process_exit(int s)
{
    exit(0);
}

int main()
{
	child_fun();
    return 0;
}


