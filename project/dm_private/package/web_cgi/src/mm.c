#include <errno.h>
#include <stdlib.h>             
#include <ctype.h>
#include <time.h>           
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>      
#include <setjmp.h>

#include <netinet/in.h>
#include <linux/wireless.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <sys/mman.h>
#include <sys/types.h>        
#include <sys/socket.h>      
#include <sys/stat.h>
/*
int htoi(const char *s)
{
	if( !s )return 0;

	if( *s == '0' )
	{
		s++;
		if( *s == 'x' || *s == 'X' )s++;
	}

	int n = 0;
	while( *s )
	{
		n <<= 4;
		if( *s <= '9' )
			n |= ( *s & 0xf );
		else
			n |= ( (*s & 0xf) + 9 );
		s++;
	}
	return n;
}

int main(int argc, char** argv)
{
	unsigned int addree;
	unsigned int num;
	unsigned int i;
	unsigned int map_base;
	int fbb;

	
	if(argc<3)
	{
		printf("arg error\n");
	}
	addree=htoi(argv[1]);
	printf("argv[1]=%s addree=%x\n",argv[1],addree);
	num=htoi(argv[2]);
	printf("num=%x\n",num);
//	*((volatile unsigned int *)addree)=num;
	fbb=open("/dev/mem",O_RDWR | O_SYNC);
	map_base=(char *)mmap(0,1024*128,PROT_READ|PROT_WRITE,MAP_SHARED,fbb,0x18114000);
	printf("map_base=%x\n",map_base);
	*(volatile unsigned int *)(map_base+(addree-0xb8114000))=num;
	close(fbb);

	
}
*/
#if 1
#define AR9300_OTP_BASE			0x14000
#define AR9300_OTP_STATUS		0x15f18
#define AR9300_OTP_STATUS_TYPE		0x7
#define AR9300_OTP_STATUS_VALID		0x4
#define AR9300_OTP_STATUS_ACCESS_BUSY	0x2
#define AR9300_OTP_STATUS_SM_BUSY	0x1
#define AR9300_OTP_READ_DATA		0x15f1c

#define OTP_MEM_START_ADDRESS     0x14000
#define OTP_STATUS0_OTP_SM_BUSY   0x00015f18
#define OTP_STATUS1_EFUSE_READ_DATA 0x00015f1c

#define OTP_LDO_CONTROL_ENABLE    0x00015f24
#define OTP_LDO_STATUS_POWER_ON   0x00015f2c
#define OTP_INTF0_EFUSE_WR_ENABLE_REG_V 0x00015f00

/*
int ar9300OtpWrite(struct ath_hal *ah, u_int off, u_int32_t data)
{

		// Power on LDO
		OS_REG_WRITE(ah,OTP_LDO_CONTROL_ENABLE, 0x1);
		OS_DELAY(1000);			// was 10000
		if (!(OS_REG_READ(ah,OTP_LDO_STATUS_POWER_ON)&0x1)){
			return AH_FALSE;
		}
		OS_REG_WRITE(ah,OTP_INTF0_EFUSE_WR_ENABLE_REG_V, 0x10AD079);
		OS_DELAY(1000);			// was 1000000
		OS_REG_WRITE(ah,OTP_MEM_START_ADDRESS+(off*4),data); // OTP is 32 bit addressable

		while((OS_REG_READ(ah,OTP_STATUS0_OTP_SM_BUSY))&0x1) { 
			  OS_DELAY(1000) ;	// was 100000
			}
		OS_DELAY(1000);			// was 100000

		// Power Off LDO
		OS_REG_WRITE(ah,OTP_LDO_CONTROL_ENABLE, 0x0);
		OS_DELAY(1000);		// was 10000
		if ((OS_REG_READ(ah,OTP_LDO_STATUS_POWER_ON)&0x1)){
			return AH_FALSE;
		}
		OS_DELAY(1000);			// was 10000
		return AH_TRUE;
}
*/

int ath9k_hw_wait(unsigned int reg,unsigned int mask,unsigned int val,unsigned int timeout)
{
	int i;
	unsigned int data;

	for (i = 0; i<(timeout/20);i++) {
			one_int_read(0xb8115f18,&data);
		if (data== val)
			return 0;
		usleep(20);
	}
	return -1;
}





	#define otp_adder 0xb8114030
	int get_Product_mac(unsigned char *mac)
	{
		char buffer[128];
		FILE *read_fp; 
		int chars_read; 
		int ret=-1; 
		if(mac==NULL)
		{
			printf("mac NULL error \n");
			return -1;
		}
		memset( buffer, 0, sizeof(buffer) ); 
		read_fp = popen("cfg get mac","r");
		if(read_fp!=NULL)
		{
			chars_read = fread(buffer, sizeof(char), sizeof(buffer)-1, read_fp); 
			if (chars_read > 0) 	
			{ 
				printf("%s",buffer);
				if((buffer[0]=='m')&&(buffer[1]=='a')&&(buffer[2]=='c')&&(buffer[3]=='='))
				{
					memcpy(mac,&buffer[4],12);
					ret=0;
				}else
				{
					ret=-1;
				}
			} 
			else					
			{ 
				ret = -1; 
			} 
			pclose(read_fp); 
		}
		return ret;
	}

void one_int_write(unsigned int addree,unsigned int num)
{
	unsigned int map_base;
	int fbb;
	fbb=open("/dev/mem",O_RDWR | O_SYNC);
	map_base=(char *)mmap(0,1024*128,PROT_READ|PROT_WRITE,MAP_SHARED,fbb,0x18114000);
	printf("map_base=%x ,num=%x\n",map_base,num);
	*(volatile unsigned int *)(map_base+(addree-0xb8114000))=num;
	close(fbb);
}

void one_int_read(unsigned int addree,unsigned int *num)
{
	unsigned int map_base;
	int fbb;
	fbb=open("/dev/mem",O_RDWR | O_SYNC);
	map_base=(char *)mmap(0,1024*128,PROT_READ|PROT_WRITE,MAP_SHARED,fbb,0x18114000);
	*num=*(volatile unsigned int *)(map_base+(addree-0xb8114000));
	printf("read map_base=%x num=%x \n",map_base,*num);
	close(fbb);
}




int one_write_time(unsigned int addree,unsigned int num)
{
	unsigned int tmpint;
	unsigned int data;
	one_int_write(0xb8115f24,0x1);
	usleep(40000);
	one_int_read(0xb8115f2c,&tmpint);
	usleep(40000);
	one_int_write(0xb8115f00,0x010ad079);
	usleep(40000);
	one_int_write(addree,num);
	usleep(40000);
	do{
		one_int_read(0xb8115f18,&data);
	}while(data&0x1);
	one_int_write(0xb8115f24,0x0);
	usleep(40000);
}

int one_read_time(unsigned int addree,unsigned int *num)
{
	unsigned int tmpint;
	one_int_read(addree,&tmpint);
	if (ath9k_hw_wait(AR9300_OTP_STATUS, AR9300_OTP_STATUS_TYPE,
			   AR9300_OTP_STATUS_VALID, 1000)<0)
		return -1;	
//	one_int_read(0xb8115f18,&tmpint);
	one_int_read(0xb8115f1c,num);
	return 0; 
}


int write_buffer_otp(unsigned int addree,unsigned char *buff,unsigned int len)
{
	int i;
	unsigned int data,addr,comp_data;
	addr=addree&0xfffffffc;
	for(i=0;i<len;i=i+3)
	{
		data=((buff[i]<<24)|(buff[i+1]<<16)|(buff[i+2]<<8)|0);
//		data=((buff[i]<<8)|(buff[i+1]<<16)|(buff[i+2]<<24)|0);
		if(one_read_time(addr,&comp_data)<0)
		{
			return -1;
		}
		if(comp_data!=0)
		{
			printf("otp Already write %x \n",comp_data);
			return -1;
		}
		one_write_time(addr,data);
		if(one_read_time(addr,&comp_data)<0)
		{
			return -1;
		}
/*		
		if((data&0xffffff00)!=(comp_data&0xffffff00))
		{
			printf("write addr %x error data %x comp_data %x\n",addr,data,comp_data);
			return -1;
		}
*/		
		addr=addr+4;
	}
	return 0;
}

int read_buffer_otp(unsigned int addree,unsigned char *buff,unsigned int len)
{
	int i;
	unsigned int data,addr;
	addr=addree&0xfffffffc;
	for(i=0;i<len;i=i+3)
	{
		if(one_read_time(addr,&data)<0)
		{
			return -1;
		}
		buff[i]=((data&0xff000000)>>24);
		buff[i+1]=((data&0x00ff0000)>>16);
		buff[i+2]=((data&0x0000ff00)>>8);
/*		
		buff[i+2]=((data&0xff000000)>>24);
		buff[i+1]=((data&0x00ff0000)>>16);
		buff[i]=((data&0x0000ff00)>>8);
*/
		addr=addr+4;
	}
	return 0;
}

unsigned char swim_chang(unsigned char cahrchar)
{
	unsigned char tmp,tmp1;
	tmp=cahrchar>>4;
	tmp1=cahrchar<<4;
	return (tmp|tmp1);
}

int encrypt_write()
{
	unsigned char mac[20];
	unsigned char buff[32];
	memset(mac,0,20);
	if(get_Product_mac(mac)<0)
	{
		printf("get product error\n");
		return -1;
	}
	
	buff[0]=swim_chang(mac[0]);
	buff[1]=swim_chang(mac[1]);
	buff[4]=swim_chang(mac[2]);
	buff[5]=swim_chang(mac[3]);
	buff[8]=swim_chang(mac[4]);
	buff[9]=swim_chang(mac[5]);
	buff[12]=swim_chang(mac[6]);
	buff[13]=swim_chang(mac[7]);
	buff[16]=swim_chang(mac[8]);
	buff[17]=swim_chang(mac[9]);
	buff[20]=swim_chang(mac[10]);
	buff[21]=swim_chang(mac[11]);
//	memset(buff,0xaa,32);	
//	printf("buff[0]=%x buff[1]=%x buff[2]=%x buff[3]=%x,buff[4]=%x buff[5]=%x\n",buff[0],buff[4],buff[5],buff[8],buff[12],buff[20]);
	if(write_buffer_otp(otp_adder,buff,32)<0)
	{
		printf("write otp error\n");
		return -1;
	}
	printf("write otp ok\n");
	return 0;

}

int encrypt_read()
{
	unsigned char mac[20];
	unsigned char buff[32];
	unsigned char mac_product[20];
	memset(mac,0,20);
	memset(mac_product,0,20);
	if(read_buffer_otp(otp_adder,buff,32)<0)
	{
		return -1;
	}
	mac[0]=swim_chang(buff[0]);
	mac[1]=swim_chang(buff[1]);
	mac[2]=swim_chang(buff[4]);
	mac[3]=swim_chang(buff[5]);
	mac[4]=swim_chang(buff[8]);
	mac[5]=swim_chang(buff[9]);
	mac[6]=swim_chang(buff[12]);
	mac[7]=swim_chang(buff[13]);
	mac[8]=swim_chang(buff[16]);
	mac[9]=swim_chang(buff[17]);
	mac[10]=swim_chang(buff[20]);
	mac[11]=swim_chang(buff[21]);
	
	if(get_Product_mac(mac_product)<0)
	{
		printf("get product error\n");
		return -1;
	}
	if(!strcmp(mac_product,mac))
	{
		return 0;
	}else
		{
		return -1;
	}

}



int main(int argc, char** argv)
{
	int ret=-1;
	ret=-1;
	if(!strcmp(argv[1],"w"))
	{
		if(!strcmp(argv[2],"r"))
		{
			if(!strcmp(argv[3],"i"))
			{
				if(!strcmp(argv[4],"t"))
				{
					if(!strcmp(argv[5],"e"))
					{
						printf("encrypt_write\n");
						if(encrypt_write()==0)
						{
							ret=0;
						}
					}
				}
			}
		}

	}else
	{
		if(!strcmp(argv[1],"r"))
		{
			if(!strcmp(argv[2],"e"))
			{
				if(!strcmp(argv[3],"a"))
				{
				
					if(!strcmp(argv[4],"d"))
					{
						printf("encrypt_read\n");
						if(encrypt_read()==0)
						{
							ret=0;
						}
					}
				}
			}
		}
	}
	printf("ret=%d\n",ret);
	return ret;

}
#endif



