#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<unistd.h>
#include <ctype.h>
#include "cfg_api.h"
#define N 256
#define Debug_cfg 0
static int check_mac(char *mac_addr)
{
	int i=0;
	if(strlen(mac_addr)!=12)
	{
		#if Debug_cfg
		printf("the mac address must be 12 charactors\n");
		#endif
		return 1;
	}
	for(i=0;i<12;i++)
	{
		if(!isxdigit(mac_addr[i]))
		{
			#if Debug_cfg
			printf("wrong charactor\n");
			#endif
			return 1;
		}

	}
	return 0;
}
static void upper2lower(char *str,int len)
{
	int i;
	for(i=0;i<len;i++)
	{
		if(str[i]>='A' && str[i]<='Z')
		{
			str[i]=str[i]-'A'+'a';
		}
	}
}
static void separate_str(char *whole_str,char *param_str,char *value_str)
{
	int found_equ=0;
	int i=0;
	int j,k;
	j=0;
	k=0;
	while(whole_str[i])
	{
		if('=' == whole_str[i])
			found_equ=1;
		else if(!found_equ)
			param_str[j++]=whole_str[i];
		else
			value_str[k++]=whole_str[i];
		i++;
	}
} 
int parse_file(char *path,char *retstr)
{
	char whole_str[N]={0};
	char param_str[N]={0};
	char value_str[N]={0};
	char sys_str[N]={0};
	sprintf(sys_str,"sed -e \'s/\r$//g\' -i %s",path);
	system(sys_str);
	FILE *fp=fopen(path,"r");
	if(!fp)
	{
		sprintf(retstr,"<ParseFile msg=\"can not open %s\"></ParseFile>",path);
		return 0;
	}
	char tmp_encrpytion[N]={0};
	char tmp_passwd[N]={0};
	int is_encryption_none=0;
	int is_smb_password_empty=0;
	char tmp_smb_usr_pwd[N]={0};
	while(fgets(whole_str,N,fp))
	{
		del_n(whole_str);
		separate_str(whole_str,param_str,value_str);
		//printf("get str : %s\n",whole_str);
		//printf("param:%s 	,	 value:%s\n\n",param_str,value_str);
		if(!strcmp(param_str,"ssid"))
		{
			if(strlen(whole_str)>=32)
			{
				sprintf(retstr,"<ParseFile msg=\"ssid too long : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"ssid set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		if(!strcmp(param_str,"mac"))
		{
			upper2lower(value_str,strlen(value_str));
			upper2lower(whole_str,strlen(whole_str));
			if(strlen(value_str)!=12)
			{
				sprintf(retstr,"<ParseFile msg=\"mac length wrong : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(check_mac(value_str))
			{
				sprintf(retstr,"<ParseFile msg=\"mac format wrong : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"mac set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		if(!strcmp(param_str,"encryption"))
		{
			if(strcmp(value_str,"0") && strcmp(value_str,"1") && strcmp(value_str,"2") && strcmp(value_str,"4") && strcmp(value_str,"6"))
			{
				sprintf(retstr,"<ParseFile msg=\"encryption value wrong should be 0,1,2,4,6: %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"encryption set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		if(!strcmp(param_str,"password"))
		{
			strcpy(tmp_passwd,whole_str);
		/*
			if(strlen(value_str) < 8)
			{
				sprintf(retstr,"<ParseFile msg=\"password too short : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"password set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}*/
		}
		if(!strcmp(param_str,"ip"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"<ParseFile msg=\"ip format wrong : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"ip set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		if(!strcmp(param_str,"dhcp_start"))
		{
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"dhcp_start set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		if(!strcmp(param_str,"dhcp_end"))
		{
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"dhcp_end set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		if(!strcmp(param_str,"wpa_cipher"))
		{
			if(strcmp(value_str,"1") && strcmp(value_str,"2") && strcmp(value_str,"3"))
			{
				sprintf(retstr,"<ParseFile msg=\"encryption value wrong should be 1,2,3: %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"encryption set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		#if 0
		if(!strcmp(param_str,"airplay_name"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"<ParseFile msg=\"airplay_name too long : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"airplay_name set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		if(!strcmp(param_str,"dlna_name"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"<ParseFile msg=\"dlna_name too long : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"dlna_name set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		#endif 
		if(!strcmp(param_str,"fw_version"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"<ParseFile msg=\"fw_version too long : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"fw_version set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		if(!strcmp(param_str,"model_name"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"<ParseFile msg=\"model_name too long : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"model_name set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		if(!strcmp(param_str,"model_number"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"<ParseFile msg=\"model_number too long : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"model_number set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		if(!strcmp(param_str,"model_description"))
		{
			if(strlen(whole_str) >= 64)
			{
				sprintf(retstr,"<ParseFile msg=\"model_description too long : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"model_description set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		if(!strcmp(param_str,"manufacturer"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"<ParseFile msg=\"manufacturer too long : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"manufacturer set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		if(!strcmp(param_str,"url_manufacturer"))
		{
			if(strlen(whole_str) >= 64)
			{
				sprintf(retstr,"<ParseFile msg=\"url_manufacturer too long : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"url_manufacturer set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		if(!strcmp(param_str,"dms_name"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"<ParseFile msg=\"dms_name too long : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"dms_name set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		if(!strcmp(param_str,"dms_enable"))
		{
			if(strcmp(value_str,"0") && strcmp(value_str,"1"))
			{
				sprintf(retstr,"<ParseFile msg=\"dms_enable should be 0 or 1 : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"dms_enable set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		if(!strcmp(param_str,"smb_usr_name"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"<ParseFile msg=\"smb_usr_name too long : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"smb_usr_name set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		if(!strcmp(param_str,"smb_usr_pwd"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"<ParseFile msg=\"smb_usr_pwd too long : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(strlen(value_str) < 8)
			{
				is_smb_password_empty=1;
				strcpy(tmp_smb_usr_pwd,whole_str);
				//sprintf(retstr,"<ParseFile msg=\"smb_usr_pwd too short : %s\"></ParseFile>",whole_str);
				//return 0;
			}
			else{
				if(!cfg_set_and_check_str(whole_str))
				{
					sprintf(retstr,"<ParseFile msg=\"smb_usr_name set failed : %s\"></ParseFile>",whole_str);
					return 0;				
				}
			}
		}
		if(!strcmp(param_str,"smb_guest_ok"))
		{
			if(strcmp(value_str,"yes") && strcmp(value_str,"no"))
			{
				sprintf(retstr,"<ParseFile msg=\"smb_guest_ok should be yes or no : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"smb_guest_ok set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		if(!strcmp(param_str,"smb_enabled"))
		{
			if(strcmp(value_str,"1") && strcmp(value_str,"0"))
			{
				sprintf(retstr,"<ParseFile msg=\"smb_enabled should be 1 or 0 : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"smb_enabled set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		if(!strcmp(param_str,"host_name"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"<ParseFile msg=\"host_name too long : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"host_name set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		if(!strcmp(param_str,"wifi_module"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"<ParseFile msg=\"wifi_module too long : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"wifi_module set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		if(!strcmp(param_str,"radio_band"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"<ParseFile msg=\"radio_band too long : %s\"></ParseFile>",whole_str);
				return 0;
			}
			if(!cfg_set_and_check_str(whole_str))
			{
				sprintf(retstr,"<ParseFile msg=\"radio_band set failed : %s\"></ParseFile>",whole_str);
				return 0;				
			}
		}
		bzero(whole_str,N);
		bzero(param_str,N);
		bzero(value_str,N);
	}
	get_cfg_str("encryption",tmp_encrpytion);
	if(!strcmp(tmp_encrpytion,"0"))
	{
		is_encryption_none=1;
	}
	if(strlen(tmp_passwd)>1 && (is_encryption_none != 1))
	{
		if(strlen(tmp_passwd + 9) < 8 )
		{
			sprintf(retstr,"<ParseFile msg=\"password too short : %s\"></ParseFile>",tmp_passwd);
			return 0;
		}
		if(!cfg_set_and_check_str(tmp_passwd))
		{
			sprintf(retstr,"<ParseFile msg=\"password set failed : %s\"></ParseFile>",tmp_passwd);
			return 0;				
		}		
	}
	if(is_smb_password_empty)
	{
		char tmp_guest_ok[N]={0};
		get_cfg_str("smb_guest_ok",tmp_guest_ok);
		if(strcmp(tmp_guest_ok,"yes"))
		{
				sprintf(retstr,"<ParseFile msg=\"smb_usr_pwd too short : %s\"></ParseFile>",tmp_smb_usr_pwd);
				return 0;			
		}
	}
	sprintf(retstr,"<ParseFile msg=\"success\"></ParseFile>");
	return 1;
}