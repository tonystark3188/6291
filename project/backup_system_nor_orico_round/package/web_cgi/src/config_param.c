#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
extern int parse_file(char *path,char *retstr);
static void printf_essentials(char *str)
{
	int i=0;
	int j=0;
	int times=0;
	char tmp[64]={0};
	while(str[i])
	{
		if('\"' == str[i])
			times++;
		else
		{
			if(1==times)
			tmp[j++]=str[i];
		}
		i++;
	}
	fprintf(stdout,"%s",tmp);
}
int main(int argc, const char *argv[])
{
	FILE *fp;
	int fw_len;
	fprintf(stdout,"Content-type:text/html\r\n\r\n");
	fp=fopen("/tmp/default.ini","wb+");
	if(!fp)
	{
		printf("open file failed!!! <br>");
		fprintf(stdout,"</body>\n");
		fprintf(stdout,"\n</html>\n");
		exit(1);
	}
	fw_len=atoi(getenv("CONTENT_LENGTH"));

	char *buff=malloc(fw_len+1);
	int bytes=fread(buff,1,fw_len,stdin);
	if(!strstr(buff,"fw_version"))
	{
		printf("no fw_version!!! \r\n");
		exit(1);
	}
	if(fwrite(buff,1,bytes,fp)!=bytes)
	{
		printf("write error!!! \r\n");
		exit(1);	
	}
	fclose(fp);
	free(buff);
	char msg[1024]={0};
	parse_file("/tmp/default.ini",msg);
	printf_essentials(msg);
	fprintf(stdout,"\r\n");
	return 0;
}
