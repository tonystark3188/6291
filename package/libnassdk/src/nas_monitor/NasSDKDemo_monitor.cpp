#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <json-c/json.h>
#include <fcntl.h>


static char path[] = "/tmp/mnt/USB-disk-1";


//return value : -1 error,0 no storage,1 have storage
static int find_mount_point(char *path)
{
	FILE *fp = fopen("/proc/mounts", "r");
	char line[256];
	int ret = 0;
	if(!fp)
		return -1;

	while (fgets(line, sizeof(line), fp))
	{
		if(strstr(line,path))
		{
			ret = 1;
			break;
		}
	}
	fclose(fp);
	return ret;
	
}

int main(int argc, char* argv[])
{
	int ret = -1;
	int nas_status = 0;  //   1--run   0--no run
	int nodisk_cnt = -1;
	int disk_cnt = -1;
	while (1) 
	{
		ret = find_mount_point(path);
		if((1 == ret) && (0 == nas_status))
		{
			disk_cnt++;
			if(3 == disk_cnt)
			{
				disk_cnt = -1;
				nas_status = 1;	
				printf("tx_nas init!!!\n");
				system("/usr/sbin/nasdemo >/dev/null &");
				
			}
		}
		else if((0 == ret) && (1 == nas_status))
		{
			nodisk_cnt++;
			if(5 == nodisk_cnt)
			{
				nodisk_cnt = -1;
				nas_status = 0;
				printf("tx_nas uninit!!!\n");
				system("killall -9 nasdemo");
				sleep(5);
			}
		}
		else
		{
			nodisk_cnt = -1;
			disk_cnt = -1;
		}
		sleep(1);
	}
	
	
}

