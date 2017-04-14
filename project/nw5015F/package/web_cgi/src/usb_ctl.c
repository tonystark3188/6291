#include <errno.h>
#include <stdlib.h>
#include <stdio.h>          
#include <ctype.h>
#include <time.h>           
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>      
#include <setjmp.h>

#include <netinet/in.h>
#include <linux/wireless.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <sys/mman.h>
#include <sys/types.h>        
#include <sys/socket.h>      
#include <sys/stat.h>         


#define USB_CONNECT			0x61
#define USB_DISCONNECT		0x60

#define GET_SYS_BTN		0x30

#define JZ_GPIO "/proc/jz_gpio"

#define power_gotoload_ctrl  4

#define power_reset_ctrl  3

#define test_led_on  9

#define SHORT_PRESS 	1
#define LONG_PRESS 		2


void usage()
{
	printf("control usb bus:\n");
	printf("\tusb_ctl on : usb connect\n");
	printf("\tusb_ctl off : usb disconnect\n");

}

int main(int argc, char** argv)
{

	int fh=NULL;
	if( argc < 2)
	{
		usage();
		exit(0);
	}
	fh=open(JZ_GPIO, O_RDWR);
	if(fh== NULL)
	{
		printf("can not open jz_gpio.\n");
		goto error;
	}
	if ( !strcmp(argv[1], "on") )
	{
		if(ioctl(fh, USB_CONNECT, 0) < 0)
		{
			printf("usb_connect error \n\n");
		}

	}
	else if ( !strcmp(argv[1], "off") )
	{
		if(ioctl(fh, USB_DISCONNECT, 0) < 0)
		{
			printf("usb_disconnect error \n\n");
		}
	}
	close(fh);
	return 0;
error:
		return -1;	

}

