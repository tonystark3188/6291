#include "gp_buffer.h"
#include "stdlib.h"
#include "debuglog.h"
#include <sys/time.h>
#include <stdio.h>

#define DEBG(level, format, arg...)

int my_free(void **buf)
{
	if (*buf) {
		free(*buf);
		*buf = NULL;
	}
	return 0;
}

int print_buffer(unsigned char* stream, int len)
{
	int i = 0;
	int printlen = len>200?200:len;
	DEBG (3, "[log][streamlen=%d][", len);
	for (i = 0; i < printlen; ++i)
		DEBG (3,"%02x ", stream[i]);
	if (printlen < len)
		DEBG (3, "...");		
	DEBG (3, "]\n");
	return 0;
}

int print_mtime(char* msg)
{
	struct timeval tv = {0};
	gettimeofday(&tv, NULL);
	DEBG(1, c_CharColor_yellow"%s: %lld\n"c_Print_Ctrl_Off, msg, ((long long)tv.tv_sec*1000+tv.tv_usec/1000));
	return 0;
}

