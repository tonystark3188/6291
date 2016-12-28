#include <stdio.h>
#include "base.h"
#include "notify_server.h"
#include "my_debug.h"

void usage()
{
	printf("usage   : notify_server_test <cmd>\n");
	printf("<cmd>\n");
	printf("cmd 1   : mount on pc release\n");
	printf("cmd 2   : mount on pc unrelease\n");
	printf("cmd 3   : udisk extract release\n");
	printf("cmd 4	: udisk extract unrelease\n");
}


int main(int argc, char **argv) 
{
	int ret = -1;
	int cmd = 0;
	int status = 0;
	if(argc < 2){
		goto error;
	}
		
	cmd = atoi(argv[1]);
	switch(cmd){
		case 1:
			ret = notify_server_release_disk(RELEASE_FLAG_RELEASE, &status);
			if(ret == 0){
				DMCLOG_D("notify success");
				DMCLOG_D("status = %d", status);
			}
			else{
				DMCLOG_D("notify fail,ret = %d",ret);
			}
			break;
		case 2:
			ret = notify_server_release_disk(RELEASE_FLAG_UNRELEASE, &status);
			if(ret == 0){
				DMCLOG_D("notify success");
				DMCLOG_D("status = %d", status);
			}
			else{
				DMCLOG_D("notify fail,ret = %d",ret);
			}
			break;
		case 3:
			if(argc < 3){
				goto error;
			}
			char action_node[16];
			memset(action_node, 0, sizeof(action_node));
			strcpy(action_node, argv[2]);
			ret = udisk_extract_notify_server_release_disk(RELEASE_FLAG_RELEASE, action_node, &status);
			if(ret == 0){
				DMCLOG_D("notify success");
				DMCLOG_D("status = %d", status);
			}
			else{
				DMCLOG_D("notify fail,ret = %d",ret);
			}
			
			break;
		case 4:
			if(argc < 3){
				goto error;
			}
			memset(action_node, 0, sizeof(action_node));
			strcpy(action_node, argv[2]);
			ret = udisk_extract_notify_server_release_disk(RELEASE_FLAG_UNRELEASE, action_node, &status);
			if(ret == 0){
				DMCLOG_D("notify success");
				DMCLOG_D("status = %d", status);
			}
			else{
				DMCLOG_D("notify fail,ret = %d",ret);
			}
			
			break;	
	}

	return 0;
error:
	DMCLOG_E("invalid argument");
	usage();
	return -1;
}

