#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<strings.h>
#define N 128
void del_n(char *str)
{
	int i=0;
	while(str[i])
	{
		if('\n' == str[i])
		{
			str[i]=0;
			if(i>0 && str[i-1]=='\r')
				str[i-1]=0;
		}
		i++;
	}
}
void set_cfg_str(char *str)
{
	char set_str[N]={0};
	sprintf(set_str,"cfg set \'%s\'",str);
	system(set_str);
}
//1,ok,0,param not set or wrong param
int get_cfg_str(char *param,char *ret_str)
{
	char get_str[N]={0};
	char tmp[N]={0};
	FILE *fp;
	sprintf(get_str,"cfg get \'%s\'",param);
	fp=popen(get_str,"r");
	fgets(tmp,N,fp);
	pclose(fp);
	del_n(tmp);
	if(strlen(tmp)<=1)
		return 0;
	else
	{
		strcpy(ret_str,tmp+strlen(param)+1);
		return 1;
	}
}
char *get_cfg_str_malloc(char *param)
{
	char get_str[N]={0};
	char ret_str[N]={0};
	char *ret_p;
	char tmp[N]={0};
	FILE *fp;
	sprintf(get_str,"cfg get \'%s\'",param);
	fp=popen(get_str,"r");
	fgets(tmp,N,fp);
	pclose(fp);
	del_n(tmp);
	if(strlen(tmp)<=1)
		return NULL;
	else
	{
		strcpy(ret_str,tmp+strlen(param)+1);
		ret_p=malloc(strlen(ret_str)+1);
		bzero(ret_p,strlen(ret_str)+1);
		strcpy(ret_p,ret_str);
		return ret_p;
	}
}
//1,set success;0,fail
int cfg_set_and_check_str(char *str)
{
	char tmp[N]={0};
	char param[N]={0};
	int i=0;
	while('=' != str[i])
	{
		param[i]=str[i];
		i++;
	}
	set_cfg_str(str);
	get_cfg_str(param,tmp);
	if(!strcmp(tmp,str+strlen(param)+1))
		return 1;
	else
		return 0;
}
