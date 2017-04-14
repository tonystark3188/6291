#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//#include "msg.h"

#define BUFSIZ 512

int letvIsAlive(){


	char buffer[BUFSIZ]; 
	FILE *read_fp; 
	int chars_read; 
	int count=0; 
	memset( buffer, 0, BUFSIZ ); 
	char tmpStr[64];

	sprintf(tmpStr,"%s","ps aux |grep httpd |wc -l");

	read_fp=popen(tmpStr,"r");
	if(read_fp!=NULL)
	{
		chars_read = fread(buffer, sizeof(char), BUFSIZ-1, read_fp); 
		if (chars_read > 0){
			//p_debug("buffer==%s==",buffer);
			//count=atoi(buffer);
			count=buffer[0]-'0';
			count=count-2;
			fprintf(stdout,"count=%d",count);
			fflush(stdout);
			pclose(read_fp);
			//return count;
		}else {
			printf("chars_read=%d",chars_read);
			pclose(read_fp);
			//return 0;
		}

	}else {
		printf("read fp error");
		//return 0;
	}

	//sprintf(retstr,"<Users count=\"%d\"/>",count);
	//p_debug(retstr);
	return count;

}


void main()
{
	while(1)
		{
		if(letvIsAlive()<3){
			//system("httpd &");
			//system("/etc/init.d/mysqld restart");			
		}else {
			//system("killall dm_letv");
			
		}
		sleep(1);
	}

}
