#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "cgiget.h"
#define FW_DEV "/dev/mmcblk0"
#define FW_FILE "/tmp/fwupgrade"
#define FW_LEN (53*1024*1024)
#define BUF_MAX 0x80000

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

	FILE *fw_fp=NULL;
	FILE *fp_dev=NULL;
	unsigned char * fw_ptr;
	int f_size=0;
	int fd;
	char *cgistr;
	int nor_seek;
	int file_seek;
	int nor_size;
	int len;
	char teststr[100];
	fprintf(stdout,"Content-type:text/html\r\n\r\n");
	
	cgistr = GetStringFromWeb();  
	// if((fw_fp = fopen(FW_FILE,"rb")) == NULL)    //read,binary
	// {
	// 	//printf("open /dev/mtdblock5 failed\n");
	// 	logstr("1 open fwupgrade failed.");
	// 	return 0;
	// }
#if 1
	system("sync");
	system("echo 3 >/proc/sys/vm/drop_caches");
	//system("echo none > /sys/class/leds/longsys\:green\:led/trigger");
	//system("echo timer > /sys/class/leds/longsys\:blue\:led/trigger");
	//sleep(1);
	// system("block umount");
	// usleep(200000);
#endif
	// system("ifconfig wlan0 down");
	// system("/etc/init.d/samba stop");
	// system("killall uart_server");
	// system("killall dnsmasq");
	// system("killall wpa_supplicant");
	// system("killall udhcpc");
	// system("killall ushare");
	// system("killall check_shair.sh");
	// system("killall newshair");
	// system("killall avahi-publish-service");
	// system("killall check_reset");
	system("/etc/init.d/dm_router stop");
#if 0
	unsigned long filesize;
	FILE *fp=NULL;
	fp=fopen(FW_FILE,"r");
	if(fp==NULL)
		exit(1);
	fseek(fp, 0L, SEEK_END);
	filesize = ftell(fp);
	fclose(fp);

	unsigned char pad_ff[1024];
	unsigned long write_len=0;
	int ret_size;
	FILE *mmc_fp;
	memset(pad_ff,0xff,sizeof(pad_ff));
	mmc_fp=fopen(FW_DEV,"wb");
	if(mmc_fp==NULL)
	{
		printf("can not open device\n");
		exit(1);
	}
		
	fseek(mmc_fp,(3*1024*1024),SEEK_SET);
	while(write_len<FW_LEN)
	{
		ret_size=fwrite(pad_ff,1,1024,mmc_fp);
		if(ret_size!=1024)
		{
			printf("write error\n");
			break;
		}
		
		write_len += ret_size;
	}
	fclose(mmc_fp);


#endif
	//system("mv /tmp/fwupgrade /tmp/fwupgrade.gz");
	//system("gzip -d /tmp/fwupgrade.gz");

	// system("dd if=/tmp/fwupgrade of=/dev/mmcblk0 bs=1M count=5 seek=3");
	// system("sync");
	// system("dd if=/tmp/fwupgrade of=/dev/mmcblk0 bs=1M skip=5 seek=8");
	// system("sync");
	system("sysupgrade /tmp/fwupgrade &");


	// system("dd if=/tmp/fwupgrade of=/dev/mmcblk0 bs=1M seek=3");

	//return 0;

#if 0
	system("ifdown -a");
	system("killall perl");
	system("killall checknren");
	system("killall nrender");
	system("killall minidlna");
	system("killall miniupnpd");
	system("killall wifi_save");
//	system("echo 3 > /proc/sys/vm/drop_caches");
//added led blink during upgrading
	int fd1;
	fd1=open("/proc/led_net_ioctl",0);
	if(fd1<0)
	{
		printf("can't open led_net_ioctl\n");
		exit(1);
	}
	ioctl(fd1,0);
	close(fd1);
    fd = open("/proc/led_upgrade_ioctl", 0);
    if(fd < 0)
    {
        printf("Open Led Device Faild! Quit!/n");
        exit(1);
    } 
    ioctl(fd,2);
 
	fw_ptr=(unsigned char *)malloc(BUF_MAX);
	if( (fp_dev=fopen(FW_DEV,"wb"))==NULL)    // write and read,binary
	{
		//printf("open /dev/mtdblock5 failed\n");
		logstr("start 1221\n");
		return 0;
	}
	memset(fw_ptr,0,512);
	f_size=fwrite(fw_ptr,1,512,fp_dev);
	if(f_size!=512)
	{
		//printf("write zero 512 failed\n");
		logstr("2\n");
		return 0;
	}
	file_seek=512;
	nor_seek = 512;
	logstr("start 3\n");
	for(len=0; len<FW_LEN-512;)
	{
		fseek(fp_dev, nor_seek, SEEK_SET);
		fseek(fw_fp, file_seek, SEEK_SET);
		f_size = fread(fw_ptr,1,BUF_MAX,fw_fp);
		nor_size = fwrite(fw_ptr, 1, f_size, fp_dev);
		if(nor_size != f_size)
		{
			logstr("3\n");
			return 0;
		}
		if(nor_size != BUF_MAX)
			break;
		len += nor_size;
		nor_seek += nor_size;
		file_seek += nor_size;
		sprintf(teststr, "len %x, nor_size %x, nor_seek %x\n", len, nor_size, nor_seek);
		logstr(teststr);
	}
	fseek(fp_dev,0,SEEK_SET);
	fseek(fw_fp, 0, SEEK_SET);
	fread(fw_ptr, 1, 512, fw_fp);
	f_size=0;
	f_size=fwrite(fw_ptr,1,512,fp_dev);
	if(f_size!=512)
	{
		logstr("write 512 failed\n");
		logstr("4\n");
		return 0;
	}
	
	free(fw_ptr);
	fclose(fp_dev);
	fclose(fw_fp);
#endif
#if 0
	system("sync");


	if(cgistr==NULL || (!strcmp(cgistr+5,"reboot")))
	{
			free(cgistr);
			// ioctl(fd,0);
			// close(fd);
		//	logstr("reboot -f\n");
			system("reboot -f");
			system("sleep 5");
			system("echo b 2>/dev/null >/proc/sysrq-trigger");
	}else{
		free(cgistr);
		// ioctl(fd,0);
		// close(fd);
		//logstr("reboot -f\n");
		system("halt");
	}
#endif
	return 0;
}
