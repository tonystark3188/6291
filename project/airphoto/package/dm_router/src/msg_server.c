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
#include "msg_server.h"
#include "camera.h"

int	exit_flag;
static void
signal_handler(int sig_num)
{
	switch (sig_num) {
	case SIGPIPE:
		DMCLOG_E("receive pipe signal");			
		break;
#ifndef _WIN32
	case SIGCHLD:
		while (waitpid(-1, &sig_num, WNOHANG) > 0) ;
		break;
#endif /* !_WIN32 */
	default:
		DMCLOG_E("exit_flag = %d",sig_num);
		exit_flag = sig_num;
		break;
	}
}

/*****************************************router module start*******************************************************************/
void router_server_prcs_task(void)
{
	ENTER_FUNC();
	int res = 0;
	res = mkdir(INOTIFY_DIR_PATH, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	DMCLOG_D("res = %d", res);
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
	DMCLOG_D("dm_server started on port(s) %d,%d,%d", INIT_PORT,ROUTER_PORT,FILE_PORT);
	while (exit_flag == 0)
		shttpd_poll(ctx, 5000);
	shttpd_fini(ctx);
	DMCLOG_D("exit succ");
	return (0);
	
}

int signal_setup()
{
	(void) signal(SIGTERM, signal_handler);
	(void) signal(SIGINT, signal_handler);
	(void) signal(SIGPIPE, signal_handler);

	signal(SIG_UHOTPLUG, CamD_usb_hotplug_event);
	return 0;
}

int main(int argc, char *argv[])
{
    int rslt = 0;
	signal_setup();
    /* 云通信模块发过来的消息的处理线程 */
	PTHREAD_T tid_router_server;
	PTHREAD_T tid_file_server;
	#ifdef DB_TASK
	PTHREAD_T tid_db_server;
	#endif
	
	init_airphoto_sdk (NULL);
	create_longconnect_thread();

	#ifdef ROUTER_TASK
	if (0 != (rslt = PTHREAD_CREATE(&tid_router_server, NULL, (void *)router_server_prcs_task, NULL)))
    { 
       DMCLOG_D("Create router server msg prcs thread failed!");
        rslt = -1;
        goto quit;
    }
	#endif
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
	
	#ifdef ROUTER_TASK	
	PTHREAD_DETACH(tid_router_server);
	#endif
	PTHREAD_DETACH(tid_file_server);
	#ifdef DB_TASK
	PTHREAD_DETACH(tid_db_server);
	#endif
    /* forever,可做异常退出或其他处理 */
   dm_router_cycle();
quit:
    DMCLOG_D("----------------msg server main task quit!----------");
    return rslt;
}
