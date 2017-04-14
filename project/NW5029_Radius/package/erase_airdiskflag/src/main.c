#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <syslog.h>

     
#define MAX_TRY   (5)

int main(void) 
{
	FILE *fp = NULL;
	char flag[16];
	size_t size;
	int cnt = 0;
	fp = fopen("/dev/mtdblock8","rb+");
	if(fp == NULL)
	{
		fprintf(stderr,"fopen /dev/mtdblock8 error!!!");
		syslog(LOG_ERR,"fopen /dev/mtdblock8 error!!!");
		return -1;
	}
	while(cnt < MAX_TRY)
	{
		cnt++;
		fseek(fp, 0L, SEEK_SET);
		memset(flag,0xFF,sizeof(flag));
		size = fwrite(flag,1,16,fp);
		if(size == 16)
		{
			break;
		}
		
	}
	fclose(fp);
	if(cnt == MAX_TRY)
	{
		fprintf(stderr,"fwrite /dev/mtdblock8 error!!!");
		syslog(LOG_ERR,"fwrite /dev/mtdblock8 error!!!");
		return -1;
	}
	return 0;
	
}

