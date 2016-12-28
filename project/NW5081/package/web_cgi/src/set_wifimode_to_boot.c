#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>


#define DISK_DEV		"/dev/mmcblk0"


#define KBYTE			(1024LL)
#define MBYTE			((KBYTE)*(KBYTE))
#define GBYTE			((MBYTE)*(KBYTE))

#define P4_OFFSET		(70*1024*2)    //sector , 512 bytes per sector, p4 offset is 70MB

#define WIFI_MODE_ADDR	0x40000


#define ARRAY_SIZE(x)		((sizeof(x))/(sizeof(x[0])))


static void usage(void)
{
   
	printf("usage: set wifi mode to boot\n");
	
	printf("	2g	: 2.4G \n");
	printf("	5g	: 5G\n");
	printf("	read: read wifi mode from boot\n");

	printf("\n");

	exit(1);
}

int read_wifi_mode()
{
	int	fd = -1;
	int i=0;
	unsigned char	wifimode;
	

	fd = open(DISK_DEV,O_RDONLY);
	if(fd < 0)
	{
		printf("open %s failed.\n",DISK_DEV);
		exit(1);
	}
	lseek(fd, WIFI_MODE_ADDR,SEEK_SET);
	if(read(fd,&wifimode,1) != 1)
	{
		printf("read %s failed.\n",DISK_DEV);
		close(fd);
	}
	
	printf("%x\n", wifimode);

	close(fd);

	return 0;
}
int write_wifimode_to_boot(char *mode_str)
{
	int	fd = -1;
	unsigned char i_mode;
	fd = open(DISK_DEV,O_RDWR);
	if(fd < 0)
	{
		printf("open %s failed.\n",DISK_DEV);
		return -1;
	}
	if(!strcmp(mode_str,"2g"))
		i_mode=2;
	else if (!strcmp(mode_str,"5g"))
	{
		i_mode=5;
	}
	else
	{
		printf("unsupported wifi mode\n");
		close(fd);
		return -1;
	}
	lseek(fd, WIFI_MODE_ADDR,SEEK_SET);
	if(write(fd,&i_mode,sizeof(unsigned char)) != sizeof(unsigned char))
	{
		printf("write %s failed.\n",DISK_DEV);
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}




int main(int argc, char **argv)
{
	int ret = 0;
	
	if(argc < 2 || !strcmp(argv[1],"-h"))
	{
		usage();
		exit(0);
	}
	if(!strcmp(argv[1],"read"))
	{
		read_wifi_mode();
	}
	else if(!strcmp(argv[1],"2g") || !strcmp(argv[1],"5g"))
	{
		write_wifimode_to_boot(argv[1]);
	}
	else
	{
		usage();
	}
				
	return 0;
}
