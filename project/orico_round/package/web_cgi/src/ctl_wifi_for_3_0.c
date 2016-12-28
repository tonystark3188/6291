#include <errno.h>
#include <stdlib.h>
#include <stdio.h>          
#include <ctype.h> 
#include <unistd.h>
#include <fcntl.h>


#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/types.h>  

#define PC_STATUS 2

#define rtl_encryp_control "/proc/wifidisk_ioctl"
#define LED_CONTROL "/proc/led_upgrade_ioctl"

#define LED_OFF				0

int wifi_led_off()
{
	int fh;
	int fd1;
	fd1=open("/proc/led_net_ioctl",0);
	if(fd1<0)
	{
		printf("can't open led_net_ioctl\n");
		return -1;
	}
	ioctl(fd1,0);
	close(fd1);

	fh=open(LED_CONTROL, O_RDWR);
	if(fh<0)
	{
		printf("fh null .\n");
		return -1;
	}
	ioctl(fh,LED_OFF);
	close(fh);
	return 0;
}

int main(int argc, char** argv)
{

		int fh=NULL;
		int cmd;
		unsigned char pc_or_dc=0;
//		cmd=atoi(argv[1]);
		fh=open(rtl_encryp_control, O_RDWR);
		if(fh== NULL)
		{
			printf("fh null .\n");
			goto error;
		}
//		printf("tt=%s\n",argv[1]);
		ioctl(fh,PC_STATUS,&pc_or_dc);
		if(pc_or_dc==1)
		{
			system("wifi down");
			wifi_led_off();
			printf("udisk is connected to pc, shutdown the wifi.\n");
		}
		
		
		close(fh);
		return 0;
error:
		return -1;

}

