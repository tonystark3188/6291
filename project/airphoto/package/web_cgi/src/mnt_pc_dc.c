#include <errno.h>
#include <stdlib.h>
#include <stdio.h>          
#include <ctype.h> 
#include <unistd.h>
#include <fcntl.h>


#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/types.h>  


#define rtl_encryp_control "/proc/wifidisk_ioctl"





int main(int argc, char** argv)
{

		int fh=NULL;
		int cmd;
		unsigned int pc_or_dc=0;
//		cmd=atoi(argv[1]);
		fh=open(rtl_encryp_control, O_RDWR);
		if(fh== NULL)
		{
			printf("fh null .\n");
			goto error;
		}
//		printf("tt=%s\n",argv[1]);
		ioctl(fh,2,&pc_or_dc);
		if(pc_or_dc==1)
		{
			ioctl(fh,1);
			printf("mount to system.\n");
		}
		else
		{
			ioctl(fh,0);
			printf("mount to pc.\n");
		}
#if 0
		if(ioctl(fh,cmd,&pc_or_dc) < 0)
		{
			printf("turn on led error \n\n");
		}
		else
		{
			printf("ok\n");
			if(cmd==2)
				printf("status=%d\n",pc_or_dc);

		}
#endif
		
		
		close(fh);
		return 0;
error:
		return -1;

}

