#include <strings.h>
#include <string.h>
#include<stdio.h>
#include<stdlib.h>
#define FW_VERSION "fw_version"
#define MODEL_NAME "model_name"
#define MODEL_NUMBER "model_number"
#define MODEL_DESCRIPTION "model_description"

#define MANUFACTURER "manufacturer"
#define MANUFACTURER_URL "url_manufacturer"
char *get_conf_str(char *var)
{
	FILE *fp=fopen("/etc/nrender.conf","r");
	if(NULL == fp)
	{
		//printf("open /etc/config/nrender.conf failed \n");
		return 0;
	}
	char tmp[128];
	char *ret_str;
	bzero(tmp,128);
	while(fgets(tmp,128,fp)!=NULL)
	{
		if('\n'==tmp[strlen(tmp)-1])
		{
			tmp[strlen(tmp)-1]=0;
		}
		//printf("get string from /etc/config/nrender.conf:%s\n",tmp);
		if(!strncmp(var,tmp,strlen(var)))
		{
			ret_str = malloc(strlen(tmp)-strlen(var));
			if(!ret_str)
			{
				fclose(fp);
				return 0;
			}
			bzero(ret_str,strlen(tmp)-strlen(var));
			strcpy(ret_str,tmp+strlen(var)+1);
			//printf("ret string:%s\n",ret_str);
			fclose(fp);
			return ret_str;
		}
		
	}
	fclose(fp);
	return 0;
}