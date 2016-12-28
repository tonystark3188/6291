/************************************************************************
#
#  Copyright (c) 2014-2016  I-MOVE(SHENTHEN) Co., Ltd.
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


/*############################## Includes ####################################*/
#include "base.h"
#include "my_debug.h"


/*############################## Global Variable #############################*/

/*############################## Functions ###################################*/

void sighand(int signo)
{
    DMCLOG_D("Thread %u in signal handler", (unsigned int )pthread_self());  
}

int main(int argc, char *argv[])
{
	struct sigaction actions;	
	memset(&actions, 0, sizeof(actions));	 
	sigemptyset(&actions.sa_mask);		 
	actions.sa_flags = 0;   
	actions.sa_handler = sighand;  
	sigaction(SIGALRM, &actions, NULL); 
	if(_udp_listen_clients() < 0){
		DMCLOG_E("udp listen server error!");
	}
    DMCLOG_D("dm discovery main task quit!");
    return 0;
}
