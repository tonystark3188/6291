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
#include "uci_for_cgi.h"

extern char *optarg;
#define FACTORYCONFIGPATH "/factory/"
#define DISK_DEV		"/dev/mmcblk0"


#define KBYTE			(1024LL)
#define MBYTE			((KBYTE)*(KBYTE))
#define GBYTE			((MBYTE)*(KBYTE))

#define P4_OFFSET		(70*1024*2)    //sector , 512 bytes per sector, p4 offset is 70MB

#define LINUX_FS		0x83
#define FAT_FS			0x0b

#define ARRAY_SIZE(x)		((sizeof(x))/(sizeof(x[0])))

struct mbr_tab_item {
	unsigned long	offset;
	unsigned long	size;
	unsigned char	type;
};

struct mbr_tab_item	tab_item[4];



static void usage(void)
{
   
	printf("usage: disk mp tool\n");
	
	printf("	read	: display partition info \n");
	printf("	write	: write partition 4 sector number to mbr\n");

	printf("\n");

	exit(1);
}

int diskplay_partition_info()
{
	int	fd = -1;
	int i=0;
	unsigned char	block[512];
	memset(tab_item, 0, ARRAY_SIZE(tab_item));
	memset(block, 0, ARRAY_SIZE(block));

	fd = open(DISK_DEV,O_RDONLY);
	if(fd < 0)
	{
		printf("open %s failed.\n",DISK_DEV);
		exit(1);
	}
	if(read(fd,block,512) != 512)
	{
		printf("write %s failed.\n",DISK_DEV);
		close(fd);
	}
	/*
	memcpy(block+0x1c6,&tab_item[0].offset,sizeof(unsigned int));
	memcpy(block+0x1d6,&tab_item[1].offset,sizeof(unsigned int));
	memcpy(block+0x1e6,&tab_item[2].offset,sizeof(unsigned int));
	memcpy(block+0x1f6,&tab_item[3].offset,sizeof(unsigned int));

	memcpy(block+0x1ca,&tab_item[0].size,sizeof(unsigned int));
	memcpy(block+0x1da,&tab_item[1].size,sizeof(unsigned int));
	memcpy(block+0x1ea,&tab_item[2].size,sizeof(unsigned int));
	memcpy(block+0x1fa,&tab_item[3].size,sizeof(unsigned int));

	memcpy(block+0x1c2,&tab_item[0].type,sizeof(unsigned char));
	memcpy(block+0x1d2,&tab_item[1].type,sizeof(unsigned char));
	memcpy(block+0x1e2,&tab_item[2].type,sizeof(unsigned char));
	memcpy(block+0x1f2,&tab_item[3].type,sizeof(unsigned char));
	*/
	memcpy(&tab_item[0].offset,block+0x1c6,sizeof(unsigned int));
	memcpy(&tab_item[1].offset,block+0x1d6,sizeof(unsigned int));
	memcpy(&tab_item[2].offset,block+0x1e6,sizeof(unsigned int));
	memcpy(&tab_item[3].offset,block+0x1f6,sizeof(unsigned int));

	memcpy(&tab_item[0].size,block+0x1ca,sizeof(unsigned int));
	memcpy(&tab_item[1].size,block+0x1da,sizeof(unsigned int));
	memcpy(&tab_item[2].size,block+0x1ea,sizeof(unsigned int));
	memcpy(&tab_item[3].size,block+0x1fa,sizeof(unsigned int));

	memcpy(&tab_item[0].type,block+0x1c2,sizeof(unsigned char));
	memcpy(&tab_item[1].type,block+0x1d2,sizeof(unsigned char));
	memcpy(&tab_item[2].type,block+0x1e2,sizeof(unsigned char));
	memcpy(&tab_item[3].type,block+0x1f2,sizeof(unsigned char));
	printf("partition\ttype\toffset\t\tsize\n");
	for(i=0;i<4;i++)
	{
		//printf("%d\t\t%02x\t\t%08x %d MB\t\t%08x %d MB\n",i+1,tab_item[i].type,tab_item[i].offset,(tab_item[i].offset*512)/MBYTE,tab_item[i].size,(tab_item[i].size*512)/MBYTE);
		printf("%d\t\t%02x\t%08x(%d MB)\t%08x(%d MB)\n",i+1,tab_item[i].type,tab_item[i].offset,
				(tab_item[i].offset)/(1024*2),tab_item[i].size,(tab_item[i].size)/(1024*2));

	}
	printf("\n");

	close(fd);

	return 0;
}
int write_sector_num_to_p4(unsigned long disk_size)
{
	unsigned int p4_sector= disk_size;
	int	fd = -1;
	fd = open(DISK_DEV,O_RDWR);
	if(fd < 0)
	{
		printf("open %s failed.\n",DISK_DEV);
		return -1;
	}
	lseek(fd, 0x1fa,SEEK_SET);
	if(write(fd,&p4_sector,sizeof(unsigned int)) != sizeof(unsigned int))
	{
		printf("write %s failed.\n",DISK_DEV);
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}
int check_mp_status(int *i_mp_sta)
{
	char uci_option_str[64]="\0";
	char mp_sta[16]="\0";
	memset(uci_option_str,0,64);
	ctx=uci_alloc_context();
	uci_set_confdir(ctx,FACTORYCONFIGPATH);
	strcpy(uci_option_str,"diskconfig.@mp[0].mpsta");
	uci_get_option_value(uci_option_str,mp_sta);
	if(mp_sta[0]=='0')
		*i_mp_sta=0;
	else if(mp_sta[0]=='1')
		*i_mp_sta=1;
	printf("mp_status= %d\n",*i_mp_sta);
	uci_free_context(ctx);
	return 0;
}

int set_mp_flag()
{
	system("uci -c/factory set diskconfig.@mp[0].mpsta=1");
	system("uci -c/factory commit diskconfig");
	return 0;
}

int get_disk_size(unsigned long *disk_size)
{
	char buffer[512]; 
	FILE *read_fp; 
	int chars_read; 
	int ret=0; 
	memset( buffer, 0, 512 ); 
	read_fp = popen("fdisk -l | grep \"Disk /dev/mmcblk0\" | awk '{print $7}'", "r");
	if(read_fp!=NULL)
	{
		chars_read = fread(buffer, sizeof(char), 512-1, read_fp); 
		if (chars_read > 0)   	//connect !
		{ 
			printf("fdisk result= %s\n",buffer);
			// *disk_size=atol(buffer);
			*disk_size=strtoul(buffer,NULL,0);
			printf("%lu\n",*disk_size);
			printf("disk_size=%lu sectors,%d MB\n",*disk_size,(*disk_size)/(1024*2));
			*disk_size= *disk_size - P4_OFFSET;
			printf("p4_size=%08x(%lu) sectors,%d MB\n",*disk_size,*disk_size,(*disk_size)/(1024*2));
			ret = 1; 
		} 
		else 					//disconnect !
		{ 
			ret = 0; 
		} 
		pclose(read_fp); 
	}
	
	// printf("disk_size=%lu sectors, %08x\n",*disk_size/512,*disk_size/512);

	return ret;
}


int main(int argc, char **argv)
{
	int ret = 0;
	int mp_status=-1;
	unsigned long p4_size=0;
	if(argc < 2 || !strcmp(argv[1],"-h"))
	{
		usage();
		exit(0);
	}
	if(!strcmp(argv[1],"read"))
	{
		diskplay_partition_info();
	}
	else if(!strcmp(argv[1],"write"))
	{
		check_mp_status(&mp_status);
		if(mp_status==0)
		{
			if(get_disk_size(&p4_size)==1)
			{
				if(write_sector_num_to_p4(p4_size)==0)
				{
					set_mp_flag();
					usleep(10000);
				}
			}
		}
	}
	else
	{
		usage();
	}
				
	return 0;
}
