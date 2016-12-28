#include <errno.h>
#include <stdlib.h>             
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


#define rtl_encryp_control "/proc/rtl_encryp_control"

#define power_gotoload_ctrl  4

#define power_reset_ctrl  3

#define get_Firmware_Edition 5



int main(int argc, char** argv)
{
		unsigned char stat=0;
		unsigned char tmpStatus=0;//normal
		int fh=NULL;
		fh=open(rtl_encryp_control, O_RDWR);
		if(fh== NULL)
		{
			printf("open device failed.\n");
			goto error;
		}
		if(ioctl(fh,get_Firmware_Edition,&stat)<0)
		{
			printf("connect error\n");
			close(fh);
			return;
		}
		tmpStatus=(stat&0xf0)>>4;
		if((15>tmpStatus)&&(tmpStatus>0))
		{
			printf("ok\n");
		}
		close(fh);
		return 0;
error:
			return -1;

}

