#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>



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
	unsigned char sys_flag;
};

static void usage(void)
{
   
	printf("usage: set system flag\n");
	printf("	auto	: auto set the first system flag \n");
	//printf("	first	: set the first system flag \n");
	//printf("	second	: set the second system flag\n");

	printf("\n");

	exit(1);
}



int main(int argc, char **argv)
{
	int	fd = -1;
	struct private_data *p_data;
	unsigned char buf[NVRAM_LEN];

	if(argc < 2 || !strcmp(argv[1],"-h"))
	{
		usage();
		exit(0);
	}
	if(!strcmp(argv[1],"auto"))
	{
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
			
		if(p_data->boot_flag > 3)
		{
			p_data->boot_flag = 0;
		}
		else
		{
			p_data->sys_flag = !p_data->sys_flag;
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
	}
	else
	{
		usage();
		exit(0);
	}
	return 0;

}
