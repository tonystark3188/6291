/************************************************************************
 #
 #  Copyright (c) 2015-2016  longsys(SHENTHEN) Co., Ltd.
 #  All Rights Reserved
 #
 #  author: Oliver
 #  create date: 2016-7-7
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
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include "my_debug.h"

#include "dm_mqtt.h"
/*############################## Global Variable #############################*/

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
            sleep(3);
            exit(1);
    }
}
int main()
{
	ENTER_FUNC();
    /*(void) signal(SIGTERM, signal_handler);
    (void) signal(SIGINT, signal_handler);
    (void) signal(SIGSEGV, signal_handler);
    (void) signal(SIGBUS, signal_handler);
    (void) signal(SIGTRAP, signal_handler);
    (void) signal(SIGABRT, signal_handler);
    (void) signal(SIGPIPE,SIG_IGN);*/
    exit_flag = 0;
    dm_mqtt_start();
    do{
        sleep(1);
    }while(exit_flag == 0);
    dm_mqtt_destroy();
	EXIT_FUNC(); 
}


