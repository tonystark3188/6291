#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sys/ioctl.h>
#include <linux/wireless.h>
#include "uci_for_cgi.h"
#include "my_debug.h"
#include "msg.h"

#define GET_WIFI_MODE_BTN		0x30

#define JZ_GPIO "/proc/jz_gpio"


#define TRUE	0
#define FALSE	1

int main()
{
	int fd;
	unsigned char btn_status;
	int time_count=0;
	int ret = 1;
	fd=open(JZ_GPIO,O_RDWR);
	if(fd==-1)
		return -1;

	ioctl(fd, GET_WIFI_MODE_BTN, &btn_status);
	if(btn_status==0)
		ret = TRUE;
	else
		ret = FALSE;
	
	close(fd);
	if(ret)	printf("2g");
	else printf("5g");
	return ret;

}


