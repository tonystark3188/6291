#include <errno.h>
#include <stdlib.h>
#include <stdio.h>          
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

#define LED_ON 				0x11
#define LED_OFF				0x10

#define GET_SYS_BTN		0x30

#define JZ_GPIO "/proc/jz_gpio"

#define power_gotoload_ctrl  4

#define power_reset_ctrl  3

#define test_led_on  9

#define SHORT_PRESS 	1
#define LONG_PRESS 		2

int check_button()
{
	int fd;
	unsigned char btn_status;
	int time_count=0;
	int ret;
	fd=open(JZ_GPIO,O_RDWR);
	if(fd==-1)
		return -1;

	ioctl(fd, GET_SYS_BTN, &btn_status);
	if(btn_status==1)
	{
		close(fd);
		return 0;
	}
	usleep(10000); //按键消抖
	do {
		sleep(1);
		time_count++;
		ioctl(fd, GET_SYS_BTN, &btn_status);


	} while(btn_status!=1);
	usleep(10000); //按键消抖
	if(time_count > 3)
		ret = LONG_PRESS;
	else
		ret = SHORT_PRESS;

	close(fd);
	return ret;

}


int main(int argc, char** argv)
{

	int button_status;

	while(1)
	{
		button_status = check_button();
		if( button_status == SHORT_PRESS )
		{
			system("echo 0 > /sys/class/leds/wifi\:led/brightness");
		}
		else if(button_status == LONG_PRESS)
		{
			system("echo 1 > /sys/class/leds/wifi\:led/brightness");
		}
		usleep(100000);

	}

}

