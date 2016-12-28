#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include "uci_for_cgi.h"
#define FW_DEV "/dev/mtdblock5"

#define FW_LEN 8126464

#define MNT_PATH "/tmp/mnt"

#define TRUE 0
#define FALSE -1

char path[64]="\0";
int disk_num=0;
#if 0
int check_fw_file()
{
	int i=0;
	char j='a';
	char fw_path[64]="\0";
	FILE *fw_fp;
	int hasFW=0;
	memset(fw_path,0,sizeof(fw_path));
	//sprintf(fw_path,"/tmp/mnt/disk-%d/$$update$$.bin",i); 
	for(i=0;i<10;i++)
	{
		for(j='a';j<'z'+1;j++)
		{
			sprintf(fw_path,"/tmp/mnt/disk-%c%d/$$update$$.bin",j,i);

			if( (fw_fp=fopen(fw_path,"rb")) != NULL )
			{
				hasFW=1;
				strcpy(path,fw_path);
				fclose(fw_fp);
				goto flash;
			}
		}
		
	}
flash:
	if(hasFW)
	{
/*		i=0;
		while(fw_path[i]!='-' && fw_path[i]!='\0')
		{
			i++;
		}
		i++;
		disk_num=fw_path[i]-'0';
*/
		return TRUE;
	}
	else
		return FALSE;
}
#endif
int check_fw_file()
{
	struct dirent* ent = NULL;
	DIR *pDir;
	char dir[128];
	char fw_path[64];
	struct stat statbuf;
	int hasFW=FALSE;
	FILE *fw_fp;
	memset(dir,0,sizeof(dir));
	memset(fw_path,0,sizeof(fw_path));
	if( (pDir=opendir(MNT_PATH))==NULL )
	{
		fprintf( stderr, "Cannot open directory:%s\n", MNT_PATH );
		return FALSE;
	}
	while( (ent=readdir(pDir))!=NULL )
	{
		snprintf( dir, 128 ,"%s/%s", MNT_PATH, ent->d_name );
		//printf("%s\n",dir);
		lstat(dir, &statbuf);
		if( S_ISDIR(statbuf.st_mode) )  //is a dir
		{
			if(strcmp( ".",ent->d_name) == 0 || strcmp( "..",ent->d_name) == 0) 
				continue;
			sprintf(fw_path,"%s/$$update$$.bin",dir);
			if( (fw_fp=fopen(fw_path,"rb")) != NULL )
			{
				hasFW=TRUE;
				strcpy(path,fw_path);
				fclose(fw_fp);
				break;
			}
		}
	}
	
	closedir(pDir);
	return hasFW;
}
int main(int argc, char const *argv[])
{
	FILE *fw_fp=NULL;
	FILE *fp_dev=NULL;
	unsigned char * fw_ptr;
	int f_size=0;
	int fd;
	int fd1;
	int hasfw=0;
	int i;
	char uci_option_str[64]="\0";
	char str_sp[64]="\0";
	char updatefw[8]="\0";
	char op_fw_header[32]="\0";
	char cmd_line[128]="\0";
	//ctx=uci_alloc_context();
	//sleep(20);
	memset(path,0,sizeof(path));
	//memset(uci_option_str,0,sizeof(uci_option_str));
	memset(str_sp,0,sizeof(str_sp));
	memset(updatefw,0,sizeof(updatefw));
	memset(op_fw_header,0,sizeof(op_fw_header));
	memset(cmd_line,0,sizeof(cmd_line));
	
	//strcpy(uci_option_str,"system.@system[0].updatefw"); 
	//uci_get_option_value(uci_option_str,updatefw);
	//memset(uci_option_str,0,sizeof(uci_option_str));
	//sleep(10);
	for(i=0;i<30;i++)
	{
		hasfw=check_fw_file();
		if(hasfw==TRUE)
			break;
		sleep(1);
	}
	//printf("%s\n",path);
	if(hasfw==FALSE)
	{
		printf("\nno fw file or update has done!\n");
		goto exit;
	}
	
	system("ifdown -a");
	system("killall perl");
	system("killall checknren");
	system("killall nrender");
	system("killall lighttpd");
	system("killall hostapd");	
	system("killall wpa_supplicant");
	system("echo 3 > /proc/sys/vm/drop_caches");
	
	fd = open("/proc/led_upgrade_ioctl", 0);
    if(fd < 0)
    {
        printf("Open Led Device Faild! Quit!/n");
        exit(1);
    }
	fd1 = open("/proc/led_net_ioctl", 0);
    if(fd1 < 0)
    {
        printf("Open Led Net Device Faild! Quit!/n");
        exit(1);
    } 
	
	
	
	if((fw_fp = fopen(path,"rb")) == NULL)    //read,binary
	{
		//printf("open /dev/mtdblock5 failed\n");
		printf("1 open fwupgrade failed.");
		goto exit;
	}
	
	
	//added led blink during upgrading
    
    
 	//close(fd);
	
	fw_ptr=(unsigned char *)malloc(FW_LEN);
	f_size=fread(fw_ptr,1,FW_LEN,fw_fp);
	strncpy(op_fw_header,fw_ptr+4,7);
	if( strcmp(op_fw_header,"OpenWrt")!=0 )
	{
		printf("update.bin is wrong\n");
		free(fw_ptr);
		fclose(fw_fp);
		goto exit;
	}
	/* code 
	system("mtd -r write /tmp/fwupgrade firmware");*/
	if( (fp_dev=fopen(FW_DEV,"wb"))==NULL)    // write and read,binary
	{
		//printf("open /dev/mtdblock5 failed\n");
		printf("1");
		goto exit;
	}
	//system("ifdown -a");
	system("killall -9 wanlan");
	system("killall -9 hostapd");
	system("ifconfig wlan0 down");
	system("ifconfig wlan1 down");
	system("ifconfig eth0 down");
	system("ifconfig eth2 down");
	system("ifconfig lo down");
	ioctl(fd1,0);
	ioctl(fd,2);

	unsigned char *zero_str=NULL;
	zero_str=(unsigned char *)malloc(512);
	memset(zero_str,0,512);
	f_size=fwrite(zero_str,1,512,fp_dev);
	if(f_size!=512)
	{
		//printf("write zero 512 failed\n");
		printf("2");
		goto error_exit;
	}
	f_size=0;
	f_size=fwrite(fw_ptr+512,1,8126464-512,fp_dev);

	if(f_size!=8126464-512)
	{
		//printf("write 8126464-512 failed\n");
		printf("3");
		goto error_exit;
	}


	fseek(fp_dev,0,SEEK_SET);
	f_size=0;
	f_size=fwrite(fw_ptr,1,512,fp_dev);
	if(f_size!=512)
	{
		//printf("write 512 failed\n");
		printf("4");
		goto error_exit;
	}
	
	free(fw_ptr);
	free(zero_str);
	fclose(fp_dev);
	fclose(fw_fp);
	//uci_free_context(ctx);
	sprintf(cmd_line,"mv '%s' '%s.bak'",path,path);
	system(cmd_line);
	system("sync");
	//system("reboot -f");
	ioctl(fd,0);
	close(fd);
	close(fd1);
	return 0;
error_exit:
	ioctl(fd,1);
	
exit:
	//uci_free_context(ctx);
	close(fd);
	close(fd1);
	return -1;
}
