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


#define NVRAM_BASE_ADDR	0x40000
#define NVRAM_LEN 512

#define ARRAY_SIZE(x)		((sizeof(x))/(sizeof(x[0])))

struct private_data
{
	unsigned char wifimode;
	unsigned char mpsta;
	unsigned char boot_flag;
};

char *nvram_parameter[] = {
	[0] = "wifimode",         [1] = "mpsta",
	[2] = "boot_flag",     [3] = NULL, 
};

static void show_parameter()
{
	int i;
	while(nvram_parameter[i])
	{
        printf("%s  ",nvram_parameter[i]);
        /*
		if((i)&&(i%4 == 0))
			printf("\n");
		*/
		i++;
	}
	printf("\n");
}

static void usage(void)
{
   
	printf("usage: nvram config tools\n");
	printf("nvram parameters: \n");
	show_parameter();
	printf("	get	: nvram_cfg get wifimode\n");
	printf("	set	: nvram_cfg set wifimode=2g\n");
	printf("	show: show nvram data\n");
	printf("	reset: reset nvram data\n");

	printf("\n");

	exit(1);
}

static int nvram_show()
{
	int	fd = -1;
	int i=0;
	struct private_data *p_data;
	unsigned char buf[NVRAM_LEN];
	

	fd = open(DISK_DEV,O_RDONLY);
	if(fd < 0)
	{
		printf("open %s failed.\n",DISK_DEV);
		exit(1);
	}
	lseek(fd, NVRAM_BASE_ADDR,SEEK_SET);
	if(read(fd,buf,NVRAM_LEN) != NVRAM_LEN)
	{
		printf("read %s failed.\n",DISK_DEV);
		close(fd);
		return -1;
	}
	p_data=(struct private_data *) buf;

	show_parameter();
	
	printf("%02x %02x %02x\n", p_data->wifimode,p_data->mpsta,p_data->boot_flag);

	close(fd);

	return 0;
}

int nvram_get(char *para)
{
	int	fd = -1;
	int i=0;
	struct private_data *p_data;
	unsigned char buf[NVRAM_LEN];

	fd = open(DISK_DEV,O_RDONLY);
	if(fd < 0)
	{
		printf("open %s failed.\n",DISK_DEV);
		exit(1);
	}
	lseek(fd, NVRAM_BASE_ADDR,SEEK_SET);
	if(read(fd,buf,NVRAM_LEN) != NVRAM_LEN)
	{
		printf("read %s failed.\n",DISK_DEV);
		close(fd);
		return -1;
	}
	p_data=(struct private_data *) buf;
	
	if(!strcmp(para,nvram_parameter[0]))
		printf("%02x",p_data->wifimode);
	else if(!strcmp(para,nvram_parameter[1]))
		printf("%02x",p_data->mpsta);
	else if(!strcmp(para,nvram_parameter[2]))
		printf("%02x",p_data->boot_flag);
	close(fd);
	printf("\n");

	return 0;
}
int nvram_set(char *para)
{
	int	fd = -1;
	unsigned char i_mode;
	struct private_data *p_data;
	unsigned char buf[NVRAM_LEN];
	fd = open(DISK_DEV,O_RDONLY);
	if(fd < 0)
	{
		printf("open %s failed.\n",DISK_DEV);
		exit(1);
	}
	lseek(fd, NVRAM_BASE_ADDR,SEEK_SET);
	if(read(fd,buf,NVRAM_LEN) != NVRAM_LEN)
	{
		printf("read %s failed.\n",DISK_DEV);
		close(fd);
		return -1;
	}
	p_data=(struct private_data *) buf;
	close(fd);

	if(!strncmp(nvram_parameter[0],para,strlen(nvram_parameter[0])))
	{
		if( !strcmp(para+strlen(nvram_parameter[0])+1,"2g" ) )
			p_data->wifimode = 2;
		else if( !strcmp(para+strlen(nvram_parameter[0])+1,"5g" ) )
			p_data->wifimode = 5;
	}
	else if(!strncmp(nvram_parameter[1],para,strlen(nvram_parameter[1])))
	{
		p_data->mpsta = atoi(para+strlen(nvram_parameter[1])+1);
	}
	else if(!strncmp(nvram_parameter[2],para,strlen(nvram_parameter[2])))
	{
		p_data->boot_flag = atoi(para+strlen(nvram_parameter[2])+1);
	}


	fd = open(DISK_DEV,O_RDWR);
	if(fd < 0)
	{
		printf("open %s failed.\n",DISK_DEV);
		return -1;
	}
	lseek(fd, NVRAM_BASE_ADDR,SEEK_SET);
	if(write(fd,buf,NVRAM_LEN) != NVRAM_LEN)
	{
		printf("write %s failed.\n",DISK_DEV);
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

int nvram_reset()
{
	int	fd = -1;
	
	unsigned char buf[NVRAM_LEN];
	memset(buf,0xff,NVRAM_LEN);
	fd = open(DISK_DEV,O_RDWR);
	if(fd < 0)
	{
		printf("open %s failed.\n",DISK_DEV);
		return -1;
	}
	lseek(fd, NVRAM_BASE_ADDR,SEEK_SET);
	if(write(fd,buf,NVRAM_LEN) != NVRAM_LEN)
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
	if(!strcmp(argv[1],"get"))
	{
		nvram_get(argv[2]);
	}
	else if(!strcmp(argv[1],"set") )
	{
		nvram_set(argv[2]);
	}
	else if(!strcmp(argv[1],"show") )
	{
		nvram_show();
	}
	else if(!strcmp(argv[1],"reset") )
	{
		nvram_reset();
	}
	else
		usage();
				
	return 0;
}
