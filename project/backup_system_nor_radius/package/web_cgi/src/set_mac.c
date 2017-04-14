#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>


#define MAC_NAME "macaddr"
#define BUFSIZE 1000

char * GetStringFromWeb()
{
	char method[10];
	char *outstring;
	int string_length = 0;
	
	strcpy(method,getenv("REQUEST_METHOD"));
	
	if(! strcmp(method,"POST"))
	{
		//fprintf(stdout,"method is post<br>");
		string_length=atoi(getenv("CONTENT_LENGTH"));
		if(string_length!=0)
		{
			outstring=malloc(sizeof(char)*string_length+1);
			fread(outstring,sizeof(char),string_length,stdin);
			//fprintf(stdout,"%s<br>",outstring);
		}
	}

	else if(! strcmp(method,"GET"))
	{
		//fprintf(stdout,"method is get<br>");
		if(NULL != getenv("QUERY_STRING")){
			string_length=strlen(getenv("QUERY_STRING"));
			outstring=malloc(sizeof(char)*string_length+1);
			strcpy(outstring,getenv("QUERY_STRING"));
			string_length=strlen(outstring);
		}
		//fprintf(stdout,"%s<br>",outstring);
	}
	
	if(string_length==0)
	{
		return NULL;
	}
	return outstring;
	
	
}

int processString(char *string,char *name,char *value)
{
	char *ret_value=value;
	char *p_name;
	
	if(string==NULL || strlen(string)==0)
	{
		#if debug
		printf("the string is wrong. ");
		#endif
		return -1;
	}
	
	if((p_name=strstr(string,name))==NULL)
	{
		#if debug
		printf("the name is not found. ");
		#endif
		return -1;
	}
	p_name+=strlen(name)+1;
	if((*p_name)=='&' || !(*p_name) )
	{
		#if debug
		printf("the value is empty. ");
		#endif
		*value='\0';
		return 0;
	}
	while( *p_name != '&' && *p_name)
	{
		if(*p_name =='+')
		{
			*ret_value=' ';
		}
		else
		{
			*ret_value=*p_name;
		}
		p_name++;
		ret_value++;
	}
	*ret_value='\0';
	return 0;
}

int main(int argc,char *argv[])
{
	char mac[32]="\0";
	char *web_str=NULL;
	char set_str[64]="\0";
	FILE *fp;
	char *cmd = "cfg get mac";
	char buf[BUFSIZE]="\0";
	printf("Content-type:text/html\r\n\r\n");
	if((web_str=GetStringFromWeb())==NULL)
	{
		fprintf(stdout,"can't get string from web\n");
		exit(1);
	}
	processString(web_str,MAC_NAME,mac);
	memset(set_str,0,sizeof(set_str));
	sprintf(set_str,"cfg set mac=%s",mac);
	system(set_str);
	if((fp=popen(cmd,"r"))==NULL)
		perror("popen");
	while((fgets(buf,BUFSIZE,fp))!=NULL)
		printf("%s\n",buf);
	pclose(fp);
	if(strstr(buf,mac)!=NULL)
	{
		printf("successful :)\n");
	}
	else
	{
		printf("failed :(\n");
	}
	//printf("successful :)");
	free(web_str);
	return 0;
}
