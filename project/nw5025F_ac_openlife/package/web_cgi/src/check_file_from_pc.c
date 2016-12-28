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
	FILE *fp=fopen(path,"r");
	if(!fp)
	{
		sprintf(retstr,"can not open %s",path);
		return 0;
	}
	char tmp_ret[N]={0};
	while(fgets(whole_str,N,fp))
	{
		bzero(tmp_ret,N);
		del_n(whole_str);
		separate_str(whole_str,param_str,value_str);
		#if 0
		if(!strcmp(param_str,"ssid"))
		{
			if(strlen(whole_str)>=32)
			{
				sprintf(retstr,"ssid too long : %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"ssid check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		if(!strcmp(param_str,"mac"))
		{
			upper2lower(value_str,strlen(value_str));
			upper2lower(whole_str,strlen(whole_str));
			if(strlen(value_str)!=12)
			{
				sprintf(retstr,"mac length wrong : %s",whole_str);
				return 0;
			}
			if(check_mac(value_str))
			{
				sprintf(retstr,"mac format wrong : %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"mac check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		#endif
		if(!strcmp(param_str,"encryption"))
		{
			if(strcmp(value_str,"0") && strcmp(value_str,"1") && strcmp(value_str,"2") && strcmp(value_str,"4") && strcmp(value_str,"6"))
			{
				sprintf(retstr,"encryption value wrong should be 0,1,2,4,6: %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"encryption check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		if(!strcmp(param_str,"password"))
		{
			char tmp_encry[N]={0};
			get_cfg_str("encryption",tmp_encry);
			if(!strstr(tmp_encry,"0"))
			{
				if(strlen(value_str) < 8)
				{
					sprintf(retstr,"password too short : %s",whole_str);
					return 0;
				}
				get_cfg_str(param_str,tmp_ret);
				if(strcmp(value_str,tmp_ret))
				{
					sprintf(retstr,"password check failed : %s != %s",value_str,tmp_ret);
					return 0;				
				}
			}
		}
		if(!strcmp(param_str,"ip"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"ip format wrong : %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"ip check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		if(!strcmp(param_str,"dhcp_start"))
		{
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"dhcp_start check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		if(!strcmp(param_str,"dhcp_end"))
		{
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"dhcp_end check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		if(!strcmp(param_str,"wpa_cipher"))
		{
			char tmp_encry[N]={0};
			get_cfg_str("encryption",tmp_encry);
			if(!strstr(tmp_encry,"0"))
			{
				if(strcmp(value_str,"1") && strcmp(value_str,"2") && strcmp(value_str,"3"))
				{
					sprintf(retstr,"encryption value wrong should be 1,2,3: %s",whole_str);
					return 0;
				}
				get_cfg_str(param_str,tmp_ret);
				if(strcmp(value_str,tmp_ret))
				{
					sprintf(retstr,"encryption check failed : %s != %s",value_str,tmp_ret);
					return 0;				
				}
			}
		}
		#if 0 
		if(!strcmp(param_str,"airplay_name"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"airplay_name too long : %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"airplay_name check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		if(!strcmp(param_str,"dlna_name"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"dlna_name too long : %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"dlna_name check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		#endif
		if(!strcmp(param_str,"fw_version"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"fw_version too long : %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"fw_version check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		if(!strcmp(param_str,"model_name"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"model_name too long : %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"model_name check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		if(!strcmp(param_str,"model_number"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"model_number too long : %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"model_number check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		if(!strcmp(param_str,"model_description"))
		{
			if(strlen(whole_str) >= 64)
			{
				sprintf(retstr,"model_description too long : %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"model_description check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		if(!strcmp(param_str,"manufacturer"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"manufacturer too long : %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"manufacturer check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		if(!strcmp(param_str,"url_manufacturer"))
		{
			if(strlen(whole_str) >= 64)
			{
				sprintf(retstr,"url_manufacturer too long : %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"url_manufacturer check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		if(!strcmp(param_str,"dms_name"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"dms_name too long : %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"dms_name check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		if(!strcmp(param_str,"dms_enable"))
		{
			if(strcmp(value_str,"0") && strcmp(value_str,"1"))
			{
				sprintf(retstr,"dms_enable should be 0 or 1 : %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"dms_enable check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		if(!strcmp(param_str,"smb_usr_name"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"smb_usr_name too long : %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"smb_usr_name check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		if(!strcmp(param_str,"smb_usr_pwd"))
		{
			char tmp_smb_pwd[N]={0};
			get_cfg_str("smb_guest_ok",tmp_smb_pwd);
			if(!strstr(tmp_smb_pwd,"yes"))
			{
				if(strlen(whole_str) >= 32)
				{
					sprintf(retstr,"smb_usr_pwd too long : %s",whole_str);
					return 0;
				}
				if(strlen(value_str) < 8)
				{
					sprintf(retstr,"smb_usr_pwd too short : %s",whole_str);
					return 0;
				}
				else{
					get_cfg_str(param_str,tmp_ret);
					if(strcmp(value_str,tmp_ret))
					{
						sprintf(retstr,"smb_usr_pwd check failed : %s != %s",value_str,tmp_ret);
						return 0;				
					}
				}
			}
		}
		if(!strcmp(param_str,"smb_guest_ok"))
		{
			if(strcmp(value_str,"yes") && strcmp(value_str,"no"))
			{
				sprintf(retstr,"smb_guest_ok should be yes or no : %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"smb_guest_ok check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		if(!strcmp(param_str,"smb_enabled"))
		{
			if(strcmp(value_str,"1") && strcmp(value_str,"0"))
			{
				sprintf(retstr,"smb_enabled should be 1 or 0 : %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"smb_enabled check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		if(!strcmp(param_str,"host_name"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"host_name too long : %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"host_name check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		if(!strcmp(param_str,"wifi_module"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"wifi_module too long : %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"wifi_module check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		if(!strcmp(param_str,"radio_band"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"radio_band too long : %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"radio_band check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		if(!strcmp(param_str,"flag"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"flag too long : %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"flag check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		if(!strcmp(param_str,"version_flag"))
		{
			if(strlen(whole_str) >= 32)
			{
				sprintf(retstr,"version_flag too long : %s",whole_str);
				return 0;
			}
			get_cfg_str(param_str,tmp_ret);
			if(strcmp(value_str,tmp_ret))
			{
				sprintf(retstr,"version_flag check failed : %s != %s",value_str,tmp_ret);
				return 0;				
			}
		}
		bzero(whole_str,N);
		bzero(param_str,N);
		bzero(value_str,N);
	}
	sprintf(retstr,"check success");
	return 1;
}