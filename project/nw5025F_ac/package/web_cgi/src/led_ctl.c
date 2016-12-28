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

#define LED_ON 				0x11
#define LED_OFF				0x10

#define JZ_GPIO "/proc/jz_gpio"

#define power_gotoload_ctrl  4

#define power_reset_ctrl  3

#define test_led_on  9



int main(int argc, char** argv)
{

		int fh=NULL;
		int cmd=atoi(argv[1]);
		fh=open(JZ_GPIO, O_RDWR);
		if(fh== NULL)
		{
			printf("fh null .\n");
			goto error;
		}
//		printf("tt=%s\n",argv[1]);
		
		if(cmd==1)
			ioctl(fh, LED_ON, 0);
		else
			ioctl(fh,LED_OFF,1);
		

		
		
		close(fh);
		return 0;
error:
		return -1;

}

