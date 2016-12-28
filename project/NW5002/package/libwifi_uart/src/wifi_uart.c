#include <unistd.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <errno.h>
#include <fcntl.h>
#include <limits.h>      
#include <setjmp.h>
#include <sys/mount.h>
#include <fcntl.h> 
#include <sys/types.h>  
#include <sys/stat.h>   
#include <termios.h>    
#include <sys/ioctl.h>   
#include "wifi_uart.h"

typedef struct{
  unsigned char Sync_byte;
  unsigned char Packet_start;
  unsigned char write_or_read;
  unsigned char addr;
  unsigned char data_h;
  unsigned char data_l;
  unsigned char checksum;
} UART_send_w;

typedef struct{
  unsigned char Sync_byte;
  unsigned char Packet_start;
  unsigned char write_or_read;
  unsigned char addr;
  unsigned char data_h;
  unsigned char data_l;
  unsigned char checksum;
} UART_send_r;

typedef struct{
	unsigned char g_s19_buf[0x4000];
	unsigned short g_addr_buf[0x2000];
	unsigned char g_len_buf[0x4000];
	int run_time;
} UART_s19_data;


 
//UART_send_w uart_send_write;
//UART_send_r uart_send_read;
unsigned char ack_head[2] = {0xce,0xaa};
//static int fd_uart;
static char filePath[256]="/tmp/stm8.s19";
static char devPath[20]="/dev/ttyS1";

unsigned char checksum(unsigned char *buf, unsigned char len)
{
	unsigned char tmp = 0, i;
	for(i=0; i< len; i++)
		tmp += buf[i];

	return tmp;
}


//return :0 - no ack head
//		1 - have ack head
int have_ack_head(const unsigned char *s1)
{
	if(*s1 == ack_head[0] && *(s1+1) == ack_head[1]) {
		return 1;
	}
	return 0;
}

int init_uart(int baud_rate)
{
	int fd_uart;
	struct termios tio;
	char dev[20]={0};
	sprintf(dev, devPath);
	if((fd_uart=open(dev,O_RDWR|O_NDELAY|O_NOCTTY))<0)
	{
	  printf("could not open %s\n", dev);
	  return -1;
	}
	else
	{
	  printf("open %s success\n", dev);
	}
	tio.c_cflag=baud_rate|CS8|CREAD|CLOCAL;
	tio.c_cflag&=~HUPCL;
	tio.c_lflag=0;
	tio.c_iflag=IGNPAR;
	tio.c_oflag=0;
	tio.c_cc[VTIME]=0;
	tio.c_cc[VMIN]=0;
	tcflush(fd_uart,TCIFLUSH);
	tcsetattr(fd_uart,TCSANOW,&tio);
//	fcntl(fd,F_SETFL,FNDELAY);
	fcntl(fd_uart,F_SETFL,0);
	return fd_uart;
}


int close_uart(int fd_uart)
{
	if(fd_uart)
	{
		close(fd_uart);
	}
}


int uart_write(int fd_uart, unsigned char *buf, int leng)
{		
	if(write(fd_uart,buf,leng)!=leng)
	{
		return -1;
	}
	return 0;
}

int uart_read(int fd_uart, unsigned char *buf, int sec, int usec, int leng)
{
	int ret;
	int cur_leng = 0;
	unsigned char tmp;
	fd_set fd_read;
	struct timeval tv;
	tv.tv_sec = sec;//set the rcv wait time
	tv.tv_usec = usec;//100000us = 0.1s
	while(1)
	{
	   FD_ZERO(&fd_read);
	   FD_SET(fd_uart,&fd_read);
	   ret = select(fd_uart+1,&fd_read,NULL,NULL,&tv);
	   if(ret < 0)
	   {
	  	  printf("select error\n");
	  	  break;
	   }
	   else if(ret)
	   {
			if(cur_leng >= leng)
				return cur_leng;
		    ret= read(fd_uart,&tmp,1);
			if(ret > 0)
			{
				cur_leng++;
				buf[cur_leng-1] = tmp;
			}
	   }
	   else
	   {
    		//printf("select time out\n");
	   		break;
	   }
	}
	if(cur_leng > 0)
		return cur_leng;
	return -1;  
}


int stm8_write(int fd_uart, unsigned char addr, unsigned short data)
{
	int i;
	int write_try_times = 0;
	int read_try_times = 0;
	unsigned char *send_data;
	unsigned char *p;
	unsigned char read_buf[16];
	int uart_rcv_len = 0;
	int haved_rcv_len = 0;
	int num_cur;
	unsigned char sum_cheak;
	int ok_ack = -1;
	UART_send_w uart_send_write;
	//printf(">>>>>>>>>>>>>>>>stm8_write begin\n");
	uart_send_write.Sync_byte=0xcf;
	uart_send_write.Packet_start=0x55;
	uart_send_write.write_or_read=0;
	uart_send_write.addr=addr;
	uart_send_write.data_h=(data & 0x00ff);
	uart_send_write.data_l=(data>>8);
	uart_send_write.checksum=uart_send_write.addr+uart_send_write.data_h+uart_send_write.data_l+uart_send_write.write_or_read;
	send_data=(unsigned char *)&uart_send_write;

	for(write_try_times = 0; write_try_times < 3; write_try_times++)
	{
		uart_write(fd_uart, send_data,sizeof(UART_send_w));
		//uart_write(fd_uart, send_data,sizeof(UART_send_w));
		debug("uart_send_write:");
		for(i = 0;i < sizeof(UART_send_w);i++)
		{
			debug("0X%02X ",send_data[i]);
		}
		debug("\n");
		for(read_try_times = 0; read_try_times < 3; read_try_times++)
		{
			//usleep(1000);
			bzero(read_buf, 8);
			uart_rcv_len=uart_read(fd_uart, read_buf,0,10000,8);
			if (uart_rcv_len < 8) {
				printf("recv len=%d ,too short!!!\n", uart_rcv_len);
				continue;
			}
			debug("receive data :");
			for(i = 0; i < uart_rcv_len; i++){
				debug("0X%02X ",*(unsigned char *)(read_buf+i));
			}
			debug("\n");	

			if(have_ack_head(read_buf) == 0) {
				printf("has no ack head, wrong data\n");
				break;
			}
			if (read_buf[2] != 0) {
				printf("it's not write ack, something is wrong\n");
				break;
			}
			if (read_buf[3] != addr) {
				printf("it's not receive ack, write address:%d responsee address:%d\n", addr, read_buf[3]);
				break;
			}
			if (read_buf[4] != 1) {
				printf("excute read common failed\n");
				break;
			}
			unsigned char cs= checksum(read_buf+2, 5);
			if (read_buf[7] != cs) {
				printf("failed, checksum unmatch:%02x!=%02x\n", read_buf[7], cs);
				break;
			}
			//*pdata = *(read_buf+5) << 8 | *(read_buf+6);
			ok_ack = 1;
			break;
		}
		if(1 == ok_ack)
			break;
	}
	
	if(1 == ok_ack)
		return 1; //stm8 write success
	else
		return 0; //stm8 write fail
}

//return : 0 -read failed
//		 > 0 succeed
//		< 0 inner error
int stm8_read(int fd_uart, unsigned char addr,unsigned short *pdata)
{
	int i;
	int write_try_times = 0;
	int read_try_times = 0;
	unsigned char *send_data;
	unsigned char *p;
	unsigned char read_buf[16];
	int uart_rcv_len;
	int num_cur;
	unsigned char sum_cheak;
	int ok_ack = -1;
	UART_send_r uart_send_read;
	//printf(">>>>>>>>>>>>>>>>stm8_read begin\n");
	uart_send_read.Sync_byte=0xcf;
	uart_send_read.Packet_start=0x55;
	uart_send_read.write_or_read=0x01;
	uart_send_read.addr = addr;
	uart_send_read.checksum = uart_send_read.addr+ uart_send_read.write_or_read;
	send_data=(unsigned char *)&uart_send_read;
	for(write_try_times = 0; write_try_times < 3; write_try_times++)
	{
		uart_write(fd_uart, send_data,sizeof(UART_send_r));
		//uart_write(fd_uart, send_data,sizeof(UART_send_r));
		debug("uart_send_read:");
		for(i = 0;i < sizeof(UART_send_r);i++)
		{
			debug("0X%02X ",send_data[i]);
		}
		debug("\n");
		for(read_try_times = 0; read_try_times < 3; read_try_times++)
		{
			//usleep(100000);
			bzero(read_buf, 8);
			uart_rcv_len=uart_read(fd_uart, read_buf,0,10000,8);
			if (uart_rcv_len < 8) {
				printf("recv len=%d ,too short!!!\n", uart_rcv_len);
				continue;
			}
				
			debug("receive data :");
			for(i = 0; i < uart_rcv_len; i++)
			{
				debug("0X%02X ",*(unsigned char *)(read_buf+i));
			}
			debug("\n");
			if(have_ack_head(read_buf) == 0) {
				printf("has no ack head, wrong data\n");
				break;
			}
			if (read_buf[2] != 1) {
				printf("it's not read ack, something is wrong\n");
				break;
			}
			if (read_buf[3] != addr) {
				printf("it's not receive ack, read address:%d responsee address:%d\n", addr, read_buf[3]);
				break;
			}
			if (read_buf[4] != 1) {
				printf("excute read common failed\n");
				break;
			}
			unsigned char cs= checksum(read_buf+2, 5);
			if (read_buf[7] != cs) {
				printf("failed, checksum unmatch:%02x!=%02x\n", read_buf[7], cs);
				break;
			}
			*pdata = *(read_buf+6) << 8 | *(read_buf+5);
			ok_ack = 1;
			break;
		}
		if(1 == ok_ack)
			break;
	}
	
	if(1 == ok_ack)
		return 1; //stm8 read success
	else
		return 0; //stm8 read fail
}




unsigned char char_to_int(unsigned char data_h,unsigned char data_l)
{
	unsigned char t,var;
	var=0;
	if (data_h>='A' && data_h <='F')
	 t = data_h-55;//a-f之间的ascii与对应数值相差55如'A'为65,65-55即为A
	else
	 t = data_h-48;
	var = t<<4;
	if (data_l>='A' && data_l <='F')
	 t = data_l-55;//a-f之间的ascii与对应数值相差55如'A'为65,65-55即为A
	else
	 t = data_l-48;
	var |= t;
	return var;
}
int check_sum(unsigned char *data,unsigned char leng)
{
	unsigned char sum;
	int i;
	sum=0;
	for(i=0;i<leng;i++)
	{
		sum=sum+data[i];
	}
	//printf("sum=%x\n\n",sum);
	if((sum & 0xff) == 0xff)
		return 0;
	else
		return -1;
}

void s19_Analysis(unsigned char *data, UART_s19_data *s19_data)
{
	unsigned char data_tmp[256];
	unsigned char adder[3];
	unsigned char data_leng;
	unsigned short upload_addr = 0;
	unsigned int g_addr=0;
	int i,j,k;
	i=0;
	//printf("%s\n\n",data);
	if(data[i]!='s')
	{
		if(data[i+1]=='7'||data[i+1]=='8'||data[i+1]=='9')
		{
			return;
		}
		data_leng=char_to_int(data[i+2],data[i+3]);
		//printf("data_leng=%x\n\n",data_leng);
		switch(data[i+1])
		{
			case '0':
			//	i=(data_leng*2)+4;
				break;
			case '1':
				printf("\n");
				for(j=0;j<((data_leng+1)*2);j=j+2)
				{
					data_tmp[j/2]=char_to_int(data[j+2],data[j+2+1]);
					//printf("%x ",data_tmp[j/2]);
				}
				printf("\n");
				if(check_sum(data_tmp,(data_leng+1))<0)
				{
					printf("check_sum error \n\n");
				}
				//printf("check_sum ok \n\n");
				upload_addr = (data_tmp[1]<<8) | (data_tmp[2]);
				g_addr=upload_addr-0x8000;
				s19_data->g_addr_buf[s19_data->run_time] = g_addr;
				s19_data->g_len_buf[g_addr] = data_leng-3;
				s19_data->run_time++;
				//stm8_write(UPLOAD_ADDR,upload_addr);
				for(k=0;k<(data_leng-3);k++)
				{
					s19_data->g_s19_buf[g_addr+k]=data_tmp[3+k];
					//stm8_write(UPLOAD_DATA,((unsigned short)(data_tmp[3+k]) << 8));
				}
				debug("\n");
				break;	
			case '2':		
				break;
			case '3':		
				break;
			case '4':	
				break;	
			case '5':	
				break;
			case '6':	
				break;
		}
	}
		

}


void s19_Write(int fd_uart, UART_s19_data *s19_data)
{
	unsigned short m_addr,m_len;
	unsigned short tmp_addr;
	int i=0, j=0;
	debug("\n");
	debug(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>0x80\n");
	for(i = 0; i < s19_data->run_time; i++)
	{
		m_addr = s19_data->g_addr_buf[i];
		if(m_addr > 0x80)
		{	
			debug("addr:%x\n", m_addr+0x8000);
			tmp_addr = (((m_addr+0x8000) >> 8) | ((m_addr+0x8000) << 8));
			//debug("tmp_addr:%x\n", tmp_addr);
			stm8_write(fd_uart, UPLOAD_ADDR, tmp_addr);
			//stm8_write(fd_uart, UPLOAD_ADDR, m_addr+0x8000);
			m_len = s19_data->g_len_buf[m_addr];
			debug("data: ");
			for(j = 0; j < m_len; j++)
			{
				//usleep(5000);
				debug("%x ", s19_data->g_s19_buf[m_addr+j]);
				stm8_write(fd_uart, UPLOAD_DATA, s19_data->g_s19_buf[m_addr++]);
			}
			debug("\n");
		}
	}
	debug("<<<<<<<<<<<<<<<<<<<<<<<<<<<=0x80\n");
	for(i = 0; i < s19_data->run_time; i++)
	{
		m_addr = s19_data->g_addr_buf[i];
		if(m_addr <= 0x80)
		{
			debug("addr:%x\n", m_addr+0x8000);
			tmp_addr = (((m_addr+0x8000) >> 8) | ((m_addr+0x8000) << 8));
			//debug("tmp_addr:%x\n", tmp_addr);
			stm8_write(fd_uart, UPLOAD_ADDR, tmp_addr);
			//stm8_write(fd_uart, UPLOAD_ADDR, m_addr+0x8000);
			m_len = s19_data->g_len_buf[m_addr];
			debug("data: ");
			for(j = 0; j < m_len; j++)
			{
				//usleep(5000);
				debug("%x ", s19_data->g_s19_buf[m_addr+j]);	
				stm8_write(fd_uart, UPLOAD_DATA, s19_data->g_s19_buf[m_addr++]);
			}
			debug("\n");
		}
	}

	stm8_write(fd_uart, UPLOAD_END_FLAG,0x8080);
	debug("end write %x\n", UPLOAD_END_FLAG);
}


int stm8_upload(int fd_uart)
{
	FILE *fp=NULL;
	unsigned short i,j;
	unsigned char linebuf[250];
	UART_s19_data g_s19_data;
	fp = fopen(filePath, "rb");
	if (fp == NULL)
	{
		printf("open file error\n");
		goto error;
	}
	bzero(&g_s19_data, sizeof(UART_s19_data));
	while(fgets(linebuf,250,fp))
	{
		s19_Analysis(linebuf, &g_s19_data);
	}
	fclose(fp);
	
	s19_Write(fd_uart, &g_s19_data);
	
	return 0;
error:
	return -1;

}
#if 0

void usage()
{
	printf("usage   : wifi_uart <num> <value>\n");
	printf("<num:>\n");
	printf("0   : upload stm8\n");
	printf("1   : get firmware\n");
	printf("2   : get power percent\n");
	printf("3   : get button\n");
	printf("4   : get charg\n");
	printf("5   : get storage power\n");
	printf("6   : get pc detect\n");
	printf("7   : set reset\n");
	printf("8   : set poweroff\n");
	printf("9   : set storage power\n");
	printf("<value:>\n");
	printf("0 or 1 for set cmd\n");
}


int main(int argc, char **argv) 
{ 
	unsigned short tmp = 0;
	int value;
	int baud_rate = BAUD_RATE;
	int fd_uart;
	
	if(argc <= 1)
	{
		usage();
		return -1;
	}

	debug("start\n");
	fd_uart = init_uart(baud_rate);
	
	if(!strcmp(argv[1],"0")) 
	{
		debug("stm8 upload\n");
		stm8_upload(fd_uart);
	}else if(!strcmp(argv[1],"1"))  //ok
	{
		stm8_read(fd_uart,CMD_GET_FIRMWARE_VERSION, &tmp);
		debug("tmp = %02x\n", tmp);	
	}
	else if(!strcmp(argv[1],"2"))  //ok all 10%
	{
		stm8_read(fd_uart,CMD_GET_POWER_PERCENT, &tmp);
		debug("tmp = %02x\n", tmp);	
	}else if(!strcmp(argv[1],"3"))  //can't test
	{
		stm8_read(fd_uart,CMD_GET_BUTTON_STATUS, &tmp);
		debug("tmp = %02x\n", tmp);	
	}else if(!strcmp(argv[1],"4"))   //have problem
	{
		stm8_read(fd_uart,CMD_GET_CHARGE_STATUS, &tmp);
		debug("tmp = %02x\n", tmp);	
	}else if(!strcmp(argv[1],"5"))  //have problem
	{
		stm8_read(fd_uart,CMD_GET_STORAGE_POWER, &tmp);
		debug("tmp = %02x\n", tmp);	
	}else if(!strcmp(argv[1],"6"))  //ok
	{
		stm8_read(fd_uart,CMD_GET_PC_DETECT_STATUS, &tmp);
		debug("tmp = %02x\n", tmp);	
	}else if(!strcmp(argv[1],"7"))  //ok
	{
		value = atoi(argv[2]);
		debug("set reset = %d\n", value);
		stm8_write(fd_uart,CMD_SET_RESET, (unsigned short)value);
	}else if(!strcmp(argv[1],"8")) //ok
	{
		value = atoi(argv[2]);
		debug("set pweroff = %d\n", value);
		stm8_write(fd_uart,CMD_SET_POWEROFF, (unsigned short)value);
	}else if(!strcmp(argv[1],"9")) //have problem
	{
		value = atoi(argv[2]);
		debug("set STORAGE_POWER = %d\n", value);
		stm8_write(fd_uart,CMD_SET_STORAGE_POWER, (unsigned short)value);
	}else
	{
		debug("invalid argument");
	}
	close_uart(fd_uart);
	return 0;	 
}
#endif


