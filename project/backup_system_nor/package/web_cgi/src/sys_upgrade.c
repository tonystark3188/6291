#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#define FW_DEV "/dev/mmcblk0"
#define FW_FILE "/tmp/fwupgrade"
#define FW_LEN (53*1024*1024)
#define BUF_MAX 0x80000

#define MBYTE(n) (n*1024*1024)
#define KBYTE(n) (n*1024)

#define BUFF_SIZE 4096




void logstr(char *str)
{		
#if 0
	int fw_fp;
	int f_size;
	if( (fw_fp=fopen("/tmp/feng.txt","a+"))==NULL)    // write and read,binary
	{
		exit(1);
	}		
	
	f_size=fwrite(str,1,strlen(str),fw_fp);

	fclose(fw_fp);
#endif
}

int main(int argc, char const *argv[])
{
	int fw_fd=-1;
	int dev_fd=-1;
	unsigned char buff[BUFF_SIZE];

	fw_fd = open(FW_FILE,O_RDONLY);
	if(fw_fd < 0)
	{
		printf("open %s failed.\n",FW_FILE);
		return -1;
	}

	dev_fd = open(FW_DEV,O_RDWR);
	if(dev_fd < 0)
	{
		printf("open %s failed.\n",FW_DEV);
		close(fw_fd);
		return -1;
	}

	unsigned int kernel_len = MBYTE(5);
	unsigned int write_size;
	unsigned int read_size;
	lseek(dev_fd, MBYTE(3),SEEK_SET);
	while(kernel_len > 0)
	{
		read_size=read(fw_fd,buff,BUFF_SIZE);
		if(read_size != BUFF_SIZE)
		{
			//set boot_flag =3 or reupdate
			system("nvram_cfg set boot_flag=3");
			goto error_close;
		}
		write_size=write(dev_fd,buff,BUFF_SIZE);
		if(write_size != BUFF_SIZE)
		{
			//set boot_flag =3 or reupdate
			system("nvram_cfg set boot_flag=3");
			goto error_close;
		}
		kernel_len = kernel_len - BUFF_SIZE;

	}

	unsigned int rootfs_len = MBYTE(48);
	lseek(dev_fd, MBYTE(13),SEEK_SET);
	while(rootfs_len > 0)
	{
		read_size=read(fw_fd,buff,BUFF_SIZE);
		if(read_size != BUFF_SIZE)
		{
			//set boot_flag =3 or reupdate
			system("nvram_cfg set boot_flag=3");
			goto error_close;
		}
		write_size=write(dev_fd,buff,BUFF_SIZE);
		if(write_size != BUFF_SIZE)
		{
			//set boot_flag =3 or reupdate
			system("nvram_cfg set boot_flag=3");
			goto error_close;
		}
		rootfs_len = rootfs_len - BUFF_SIZE;
	}

	system("sync");
	system("sync");
	system("sync");
	close(fw_fd);
	close(dev_fd);

	return 0;

error_close:
	close(fw_fd);
	close(dev_fd);
	return -1;
}
