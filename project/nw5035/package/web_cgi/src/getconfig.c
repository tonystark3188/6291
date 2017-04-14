#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 

#define CONFIG_DEV "/dev/mtdblock2"
#define MAC_ADDR 0xe021
#define SN_ADDR  0xe039
#define SID_ADDR 	0x2c2d 
#define CODE_ADDR	0x2c5a
#define CODE_ADDR	0x2c5a
#define IMAGE1_STABLE_ADDR	0xd5
#define UBOOT_CHECK_ADDR	0x10FA
#define UBOOT_CHECK_FIRMWARE_ADDR 0X1000



#define MAC_STR		"f_mac"
#define SN_STR		"f_sn"
#define CODE_STR 	"app_code"
#define SID_STR		"app_sid"
#define IS_IMAGE1_STABLE		"Image1Stable"
#define UBOOT_CHECK_FLAG		"Reset"
#define UBOOT_CHECK_FIRMWARE		"check"


static void read_mtd(unsigned char *buffer,FILE *fp,unsigned long offset,unsigned long len)
{
	fseek(fp,offset,SEEK_SET);
	fread(buffer,len,1,fp);
}

static void write_mtd(unsigned char *buffer,FILE *fp,unsigned long offset,unsigned long len)
{
	fseek(fp,offset,SEEK_SET);
	fwrite(buffer,len,1,fp);
}

int main(int argc, char *argv[]){
	if(argc < 2)
	{
		//usage();
		exit(0);
	}
	FILE *fp_mtd=fopen(CONFIG_DEV,"rb+");
	if(fp_mtd==NULL)
	{
		#if 1
		printf("open /dev/mtdblock2 failed\n");
		#endif
		exit(1);
	}
	//else {printf("open success.");}

	if(!strcmp(argv[1],"get"))
	{
	
		unsigned char arg_str[64]={0};
		memset(arg_str,0,sizeof(arg_str));
		if(argv[2]==NULL)
			return 1;
		if(!strcmp(argv[2],"mac"))
		{
			read_mtd(arg_str,fp_mtd,MAC_ADDR,24);
			if(!strncmp(MAC_STR,arg_str,strlen(MAC_STR)))
			{
				printf("%s",arg_str+strlen(MAC_STR)+1);
				printf("\n");
			}
		}
		else if(!strcmp(argv[2],"sn"))
		{
			read_mtd(arg_str,fp_mtd,SN_ADDR,21);
			if(!strncmp(SN_STR,arg_str,strlen(SN_STR)))
			{
				printf("%s",arg_str+strlen(SN_STR)+1);
				printf("\n");
			}
		}

		else if(!strcmp(argv[2],"sid"))
		{
			read_mtd(arg_str,fp_mtd,SID_ADDR,sizeof(arg_str));
			if(!strncmp(SID_STR,arg_str,strlen(SID_STR)))
			{
				printf("%s",arg_str+strlen(SID_STR)+1);
				printf("\n");
			}
		}
		else if(!strcmp(argv[2],"code"))
		{
			read_mtd(arg_str,fp_mtd,CODE_ADDR,sizeof(arg_str));
			//printf("arg_str=%s\n",arg_str);
			if(!strncmp(CODE_STR,arg_str,strlen(CODE_STR)))
			{
				printf("%s",arg_str+strlen(CODE_STR)+1);
				printf("\n");
			}
		}
		else if(!strcmp(argv[2],"Reset"))
		{
			read_mtd(arg_str,fp_mtd,UBOOT_CHECK_ADDR,sizeof(arg_str));
			//printf("arg_str=%s\n",arg_str);
			if(!strncmp(UBOOT_CHECK_FLAG,arg_str,strlen(UBOOT_CHECK_FLAG)))
			{
				printf("%s",arg_str+strlen(UBOOT_CHECK_FLAG)+1);
				printf("\n");
			}
		}
		else if(!strcmp(argv[2],IS_IMAGE1_STABLE))
		{
			read_mtd(arg_str,fp_mtd,IMAGE1_STABLE_ADDR,sizeof(arg_str));
			//printf("arg_str=%s\n",arg_str);
			if(!strncmp(IS_IMAGE1_STABLE,arg_str,strlen(IS_IMAGE1_STABLE)))
			{
				printf("%s",arg_str+strlen(IS_IMAGE1_STABLE)+1);
				printf("\n");
			}
		}
	}
	else if(!strcmp(argv[1],"set"))
	{
		unsigned char arg_str[64]={0};
		if(!strcmp(argv[2],IS_IMAGE1_STABLE)){
			sprintf(arg_str,"%s=%s",argv[2],argv[3]);
			printf("%s\n",arg_str);
			write_mtd(arg_str,fp_mtd,IMAGE1_STABLE_ADDR,strlen(arg_str));
		}
		if(!strcmp(argv[2],UBOOT_CHECK_FLAG)){
			sprintf(arg_str,"%s=%s",argv[2],argv[3]);
			printf("%s\n",arg_str);
			write_mtd(arg_str,fp_mtd,UBOOT_CHECK_ADDR,strlen(arg_str));
		}
		if(!strcmp(argv[2],UBOOT_CHECK_FIRMWARE)){
			if(!strcmp(argv[3],"y")){
				//CE7A0461
				long check=0XCE7A0461;
				arg_str[0]=0XCE;
				arg_str[1]=0X7A;
				arg_str[2]=0X04;
				arg_str[3]=0X61;
				printf("check=%x\n",check);
				
				write_mtd(arg_str,fp_mtd,UBOOT_CHECK_FIRMWARE_ADDR,4);	
			}
			else if (!strcmp(argv[3],"n")){
				//8292B688
				long check=0X8292B688;
				arg_str[0]=0X82;
				arg_str[1]=0X92;
				arg_str[2]=0XB6;
				arg_str[3]=0X88;
				printf("check=%x\n",check);
				write_mtd(arg_str,fp_mtd,UBOOT_CHECK_FIRMWARE_ADDR,4);	
			}
			
		}
	}
	fclose(fp_mtd);
}
