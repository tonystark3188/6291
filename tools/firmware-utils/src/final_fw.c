#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define FLASH_SIZE 16
#define FW_LEN 0xfd0000

#if FLASH_SIZE==16 
#define boot_file "./uboot"		//uboot文件名为uboot
#define linux_file "./fw"		//升级文件名为fw
#define art_file "./art"		//校准数据文件名为art
#elif FLASH_SIZE==8
#define boot_file "./uboot"		//uboot文件名为uboot
#define linux_file "./fw"		//升级文件名为fw
#define art_file "./art"		//校准数据文件名为art
#elif FLASH_SIZE==4
#define boot_file "./tuboot.bin"
#define roofs_file "./ap121-2.6.31-squashfs"
#define linux_file "./vmlinux.lzma.uImage"
#define art_file "./ART.img.litter"
#endif 

#if FLASH_SIZE==16
#define BOOT_START 0
#define LINUX_START 0x20000
#define ART_START 0xff0000
#elif FLASH_SIZE==8
#define BOOT_START 0
#define LINUX_START 0x20000
#define ART_START 0x7f0000
#elif FLASH_SIZE==4
#define BOOT_START 0
#define ROOFS_START 0x50000
#define LINUX_START 0x300000
#define ART_START 0x3f0000
#endif 
int main(int argc,char *argv[])
{
	FILE *stin;
	FILE *stout;
	int size;
	unsigned char *buf;

	printf("generate the final fw.\n");
	stout = fopen(argv[3], "wb");

	if(stout == NULL)
	{
		printf("can not open the file.\n");
		return 1;
	}
		

	stin = fopen(argv[1], "rb");
	if(stin == 0)
	{
		printf("no uImage file!\n");
		return 1;
	}
	fseek(stin, 0, SEEK_END);
	size = ftell(stin);
	fseek(stin, 0, SEEK_SET);

	buf = (unsigned char *)malloc(0x3c0000);
	memset(buf,0xff,0x3c0000);
	fread(buf, 1, size, stin);
	fseek(stout, 0, SEEK_SET);
	fwrite(buf, 1, 0x3c0000, stout);
	fclose(stin);
	free(buf);



	stin = fopen(argv[2], "rb");
	if(stin == 0)
	{
		printf("no rootfs file!\n");
		return 1;
	}
	fseek(stin, 0, SEEK_END);
	size = ftell(stin);
	fseek(stin, 0, SEEK_SET);

	buf = (unsigned char *)malloc(0xc00000);
	memset(buf,0xff,0xc00000);
	fread(buf, 1, size, stin);
	fseek(stout, 0x3c0000, SEEK_SET);
	fwrite(buf, 1, 0xc00000, stout);
	fclose(stin);
	free(buf);

	fclose(stout);
	return 0;
}
