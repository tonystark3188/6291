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



// static char devPath[20]="/dev/ttyS1";

int init_uart(int baud_rate)
{
	int fd_uart;
	struct termios tio;
	char dev[20]={0};
	
	if((fd_uart=open("/dev/ttyS1",O_RDWR|O_NDELAY|O_NOCTTY))<0)
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

int main(int argc, char *argv[])
{
	unsigned char read_buf[16];
	int uart_fd;
	int uart_rcv_len;
	int i;
	uart_fd=init_uart(BAUD_RATE);
	while(1)
	{
		bzero(read_buf, 8);
		uart_rcv_len=uart_read(uart_fd, read_buf,0,10000,8);
		if(uart_rcv_len>0)
		{
			for(i = 0; i < uart_rcv_len; i++)
			{
				printf("0X%02X ",*(unsigned char *)(read_buf+i));
			}
			printf("\n");
		
		}

	}
}
#if 0
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
				printf("read_buf=%02X %02X %02X %02X %02X %02X %02X %02X\n", read_buf[0],
				read_buf[1],read_buf[2],read_buf[3],read_buf[4],read_buf[5],
				read_buf[6],read_buf[7]);
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


#endif

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


