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

#define cheak_encryp  2




int main(int argc, char** argv)
{
		int stat=0;
		int fh=NULL;
		int bool_key;
		bool_key=0;
		while(1)
		{
			fh=open(rtl_encryp_control, O_RDWR);
			if(fh== NULL)
			{
				printf("open device failed.\n");
				stat==-1;
			}else
			{
				if(ioctl(fh,cheak_encryp,&stat)<0)
				{
					printf("connect error\n");
					stat==-1;
				}
				close(fh);
				fh=NULL;
			}
			printf("stat=%d\n",stat);
			if(stat!=2)
			{
				if(bool_key==0)
				{
					bool_key=1;
					system("wifi down >/dev/null");
				}else
					{
					system("wifi up");
					bool_key=0;
				}
			}
			sleep(5);
			
		}

}
