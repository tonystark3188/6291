#include   <fcntl.h> 
#include   <unistd.h> 
#include     <stdio.h>
#include     <sys/types.h>  /**/
#include     <sys/stat.h>   /**/
#include     <termios.h>    /*PPSIX终端控制定义*/
#include     <errno.h>      /*错误号定义*/
#include     <stdlib.h>
#include     <string.h>

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "dirent.h"
#include <wchar.h>
#include <assert.h>

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


int   main() 
{
	int  fd;
	char c;
	int i;
	int readnum,nByte;
	int rtv;//存放返回值
	struct termios   opt,oldopt;
	char buf[100];
	struct timeval timeout;//超时设置参数结构体
		fd_set fs_read;//读状态数据集（这个名字不好取，只可意会吧）	
	fd=open("/dev/ttyATH0",O_RDWR);
	if(fd==NULL)
	{
	 return -1;		
	}

	tcgetattr( fd,&oldopt);
	tcgetattr( fd,&opt);

	cfsetispeed(&opt, B115200);//115200
	cfsetospeed(&opt, B115200);

	opt.c_cflag=0x1cb2;
	opt.c_iflag=0x1500;
	opt.c_oflag=0x5;
	opt.c_lflag=0xb3b;
	opt.c_cc[0]=3; 
	opt.c_cc[1]=0x1c; 
	opt.c_cc[2]=0x7f;
	opt.c_cc[3]=0x15; 
	opt.c_cc[4]=1; 
	opt.c_cc[5]=0; 
	opt.c_cc[6]=0; 
	opt.c_cc[7]=0; 
	opt.c_cc[8]=0x11; 
	opt.c_cc[9]=0x13; 
	opt.c_cc[10]=0x1a; 
	opt.c_cc[11]=0; 
	opt.c_cc[12]=0x12; 
	opt.c_cc[13]=0xf; 
	opt.c_cc[14]=0x17; 
	opt.c_cc[15]=0x16; 
	opt.c_cc[16]=0x4; 
	opt.c_cc[17]=0; 
	opt.c_cc[18]=0; 
	opt.c_cc[19]=0; 
	opt.c_cc[20]=0; 
	opt.c_cc[21]=0; 
	opt.c_cc[22]=0; 
	opt.c_cc[23]=0; 
	opt.c_cc[24]=0; 
	opt.c_cc[25]=0; 
	opt.c_cc[26]=0; 
	opt.c_cc[27]=0; 
	opt.c_cc[28]=0; 
	opt.c_cc[29]=0; 
	opt.c_cc[30]=0; 
	opt.c_cc[31]=0; 
	opt.c_cflag &= ~PARENB; //N
	opt.c_cflag &= ~INPCK;
	opt.c_cflag &= ~CSTOPB;//1
	opt.c_cflag &= ~CSIZE;
	opt.c_cflag |=  CS8  | CREAD;    //8
	
	opt.c_iflag &= ~(IXON | IXOFF | IXANY);
	opt.c_cc[VTIME] = 0; 
	opt.c_cc[VMIN] = 0;  
	opt.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  //Input
	opt.c_oflag  &= ~OPOST;   //Output
	
	tcflush(fd, TCIOFLUSH);
	tcsetattr(fd, TCSANOW, &opt);
//	write(fd,'\r',1);
//	write(fd,'\n',1);
//	write(fd,'5',1);
//	i=10;
	while(1)
	{
			memset(buf,0,100);
			readnum = read(fd,buf,100);
			if(readnum>0)
			{
	    		write(fd,buf,readnum);
				buf[0]='2';
				buf[1]='4';
				buf[2]='3';
				write(fd,buf,3);
	//			i--;
			}
	}
	tcsetattr(fd, TCSANOW, &oldopt);
	close(fd); 
	return 0; 
} 



#if 0
int   main() 
{
	int  fd;
	FILE *fwss=NULL;
	char c;
	int i;
	int readnum,nByte;
	int rtv;//存放返回值
	struct termios   opt,oldopt;
	char buf[100];
	char bufd[10];
	struct timeval timeout;//超时设置参数结构体
		fd_set fs_read;//读状态数据集（这个名字不好取，只可意会吧）	
#if 1
	fd=open("/dev/tty",O_RDWR);
	if(fd==NULL)
	{
//	printf("#######################\n\n");
	 return -1;		
	}

	tcgetattr( fd,&oldopt);
	tcgetattr( fd,&opt);

	cfsetispeed(&opt, B115200);//115200
	cfsetospeed(&opt, B115200);
	
	opt.c_cflag &= ~PARENB; //N
	opt.c_cflag &= ~INPCK;
	opt.c_cflag &= ~CSTOPB;//1
	opt.c_cflag &= ~CSIZE;
//	opt.c_cflag |= CS8;    //8
	opt.c_cflag |=  CS8  | CREAD;    //8
	
	opt.c_iflag &= ~(IXON | IXOFF | IXANY);
	opt.c_cc[VTIME] = 0; 
	opt.c_cc[VMIN] = 0;  
	opt.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  //Input
	opt.c_oflag  &= ~OPOST;   //Output
	
	tcflush(fd, TCIOFLUSH);
/*
	opt.c_cflag =0x1cb2;
	opt.c_iflag = 0x1500; 
	opt.c_lflag  = 0xb3b;  //Input
	opt.c_oflag  = 0x5;   //Output
	opt.c_line  = 0x0;   //Output

	opt.c_cc[0]=0x3;
	opt.c_cc[1]=0x1c;
	opt.c_cc[2]=0x7f;
	opt.c_cc[3]=0x15;
	opt.c_cc[4]=0x1;
	opt.c_cc[5]=0;
	opt.c_cc[6]=0;
	opt.c_cc[7]=0;
	opt.c_cc[8]=0x11;
	opt.c_cc[9]=0x13;
	opt.c_cc[10]=0x1a;
	opt.c_cc[11]=0;
	opt.c_cc[12]=0x12;
	opt.c_cc[13]=0xf;
	opt.c_cc[14]=0x17;
	opt.c_cc[15]=0x16;
	opt.c_cc[16]=0x4;
	opt.c_cc[17]=0;
	opt.c_cc[18]=0;
	opt.c_cc[19]=0;
	opt.c_cc[20]=0;
	opt.c_cc[21]=0;
	opt.c_cc[22]=0;
	opt.c_cc[23]=0;
	opt.c_cc[24]=0;
	opt.c_cc[25]=0;
	opt.c_cc[26]=0;
	opt.c_cc[27]=0;
	opt.c_cc[28]=0;
	opt.c_cc[29]=0;
	opt.c_cc[30]=0;
	opt.c_cc[31]=0;
*/
tcsetattr(fd, TCSANOW, &opt);

//	fcntl(fd, F_SETFL, FNDELAY); 
//	write(fd,"hello tty2\r\n ",12);
	i=10;
	while(i>0)
	{
//		FD_ZERO(&fs_read);//数据集清零
//		FD_SET(fd, &fs_read);//绑定描述符与数据集
//		rtv = select(fd + 1, &fs_read, NULL, NULL, &timeout);//查询改描述符关联设备是否存在数据可读
//		if(rtv)//存在可读数据，准备读取
		{
			memset(buf,0,100);
			
			readnum = read(fd,buf,100);
			if(readnum>0)
			{
//			bufd[i-1]=buf[0];
//			fwrite(buf,1,1, fwss);
//			tcflush(fd, TCIOFLUSH);
    		write(fd,buf,readnum);
			buf[0]='2';
			buf[1]='4';
			buf[2]='3';
			write(fd,buf,3);

//			tcflush(fd, TCIOFLUSH);
			i--;
			}
		}
	}
	
//	tcflush(fd, TCIOFLUSH);
	tcsetattr(fd, TCSANOW, &oldopt);        
	close(fd);
//	fwss=fopen("/mnt/disk-1/ddd","w");
//	if(fd==NULL)
//	{
//	 return -1; 	
//	}
//	fwrite(bufd,10,1, fwss);
//	fclose(fwss);

	/*
	close(fd);
	printf("c_iflag=%x \n",oldopt.c_iflag);
	printf("c_oflag=%x \n",oldopt.c_oflag);
	printf("c_cflag=%x \n",oldopt.c_cflag);
	printf("c_lflag=%x \n",oldopt.c_lflag);
	printf("c_line=%x \n",oldopt.c_line);
	for(i=0;i<NCCS;i++)
	{
		printf("c_cc[%d]=%x \n",i,oldopt.c_cc[i]);

	}
	*/
#endif	 
	return 0; 
} 
#endif
