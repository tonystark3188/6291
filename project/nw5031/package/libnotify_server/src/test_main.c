#include <stdio.h>
#include "notify_server.h"
#include "my_debug.h"

int main(int argc, char **argv) 
{
	int ret = -1;
	int release_flag = 0;
	int status = 0;
	if(argc <= 1){
		DMCLOG_E("invalid argument");
		return -1;
	}
		
	release_flag = atoi(argv[1]);
	DMCLOG_D("release_flag = %d", release_flag);
	ret = notify_server_release_disk(release_flag, &status);
	if(ret == 0){
		DMCLOG_D("notify success");
		DMCLOG_D("status = %d", status);
	}
	else{
		DMCLOG_D("notify fail,ret = %d",ret);
	}
	return 0;
}

