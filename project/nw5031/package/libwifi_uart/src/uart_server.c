#include <stdio.h>
#include "wifi_uart.h"

int main(int argc, char **argv) 
{
	int ret = -1;
	ret = SocketUartServerStart();
	if(ret < 0)
	{
		debug("start server error\n");
	}
	return ret;
}

