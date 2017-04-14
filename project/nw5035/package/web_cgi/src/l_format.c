#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>


#include <mntent.h>
#include <sys/vfs.h>
#include <dirent.h>

#include <sys/stat.h>

#include <locale.h>

#include "msg.h"
// /etc/config/system sid
#define SID "sid"

#define ROOT_PATH "/tmp/mnt/USB-disk-1/hack"

void updateVer(int flag){
	char set_str[128]={0};
	char tmp[128]={0};
	
	if(flag==0){

		sprintf(set_str,"sed -e \'/%s/d\' -i /tmp/state/status","unfinishedVer");
		system(set_str);
		memset(set_str,0,sizeof(set_str));
		
		sprintf(set_str,"echo \'%s=%d\' >> /tmp/state/status","unfinishedVer", time(NULL));
		system(set_str);
	
	}else if(flag==1){
		sprintf(set_str,"sed -e \'/%s/d\' -i /tmp/state/status","completedVer");
		system(set_str);
		memset(set_str,0,sizeof(set_str));
		
		sprintf(set_str,"echo \'%s=%d\' >> /tmp/state/status","completedVer", time(NULL));
		system(set_str);

	}else {
		sprintf(set_str,"sed -e \'/%s/d\' -i /tmp/state/status","unfinishedVer");
		system(set_str);
		memset(set_str,0,sizeof(set_str));
		
		sprintf(set_str,"echo \'%s=%d\' >> /tmp/state/status","unfinishedVer", time(NULL));
		system(set_str);
		
		sprintf(set_str,"sed -e \'/%s/d\' -i /tmp/state/status","completedVer");
		system(set_str);
		memset(set_str,0,sizeof(set_str));
		
		sprintf(set_str,"echo \'%s=%d\' >> /tmp/state/status","completedVer", time(NULL));
		system(set_str);
	
		
	}


}

int updateSysVal(const char *para,const char *val){
	char set_str[128]={0};
	char tmp[128]={0};
	sprintf(set_str,"sed -e \'/%s/d\' -i /tmp/state/status",para);
	system(set_str);
	memset(set_str,0,sizeof(set_str));
	
	sprintf(set_str,"echo \'%s=%s\' >> /tmp/state/status",para, val);
	system(set_str);
}

int format()
{
	int fd;
	char real_file_path[64]={0};
#if 1
	sprintf(real_file_path,"rm -rf %s >/dev/null","/tmp/mnt/USB-disk-1/ota/");
	system(real_file_path);

	sprintf(real_file_path,"rm -rf %s >/dev/null","/tmp/mnt/USB-disk-1/public/");
	system(real_file_path);
	
	memset(real_file_path,0,64);
	sprintf(real_file_path,"rm -rf %s >/dev/null",ROOT_PATH);
	system("/etc/init.d/dm_letv stop >/dev/null");
	system(real_file_path);
//	system("mcu_control -s 1 >/dev/null");//
	system("pwm_control 1  1 0;pwm_control 1 0 0 >/dev/null");
	updateSysVal("led_status","1");//
	updateVer(2);

	fd=access(real_file_path,0);
	
	//if( remove(real_file_path) == 0 )
	if(fd<0){	
		p_debug("Remove file %s OK \n",real_file_path);
		//fprintf(stdout,"Content-type:text/html\r\n\r\n");
		//fprintf(stdout,"<?xml version=\"1.0\" encoding=\"utf-8\"?>");
		//fprintf(stdout,"<%s><Return status=\"true\" ></Return></%s>\r\n",SETSYSSTR,SETSYSSTR);
		//fflush(stdout);		
		
		//sleep(1);
		system("/etc/init.d/dm_letv start >/dev/null");
		system("/etc/init.d/samba restart >/dev/null");
		//system("uci set system.@system[0].unfinishedVer=\"0\"");
		//system("uci set system.@system[0].finishedVer=\"0\"");
		//system("uci commit");
	}else
#endif
	{
		system("fuser -Km /tmp/mnt/USB-disk-1/ >/dev/null");
		system("block umount >/dev/null");
		system("mkfs.vfat /dev/sda1 >/dev/null");
		//sleep(5);
		system("block mount >/dev/null");
		fd=access(real_file_path,0);
		if(fd<0){	
			
			system("/etc/init.d/dm_letv start >/dev/null");
			p_debug("Remove file %s OK \n",real_file_path);
			//fprintf(stdout,"Content-type:text/html\r\n\r\n");
			//fprintf(stdout,"<?xml version=\"1.0\" encoding=\"utf-8\"?>");
			//fprintf(stdout,"<%s><Return status=\"true\" ></Return></%s>\r\n",SETSYSSTR,SETSYSSTR);
			//fflush(stdout);	
		}else
		{
			p_debug("Remove file %s fail \n",real_file_path);	
			//fprintf(stdout,"Content-type:text/html\r\n\r\n");
			//fprintf(stdout,"<?xml version=\"1.0\" encoding=\"utf-8\"?>");
			//fprintf(stdout,"<%s><Return status=\"false\" ></Return></%s>\r\n",SETSYSSTR,SETSYSSTR);
			//fflush(stdout);
			return -1;
		}
		//return -1;
	}

	return 0;


}


void main()
{
		char ret_buf[2048];
		char fw_sid[SID_LEN]="\0";
		char app_sid[SID_LEN]="\0";
	
		char *web_str=NULL;


		char uci_option_str[UCI_BUF_LEN]="\0";
		ctx=uci_alloc_context();

		strcpy(uci_option_str,"system.@system[0].sid");			//name
		uci_get_option_value(uci_option_str,fw_sid);
		memset(uci_option_str,'\0',UCI_BUF_LEN);

		printf("Content-type:text/plain\r\n\r\n");

		if((web_str=GetStringFromWeb())==NULL)
		{
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":4,\"errorMessage\":\"Can not get any parameters.\"}");
			printf("%s",ret_buf);
			fflush(stdout);
			uci_free_context(ctx);
			return ;
		}
		processString(web_str,SID,app_sid);
		
		if(!strcmp(fw_sid,app_sid))//是管理员
		{
			if(format()<0){
				sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":10,\"errorMessage\":\"Format disk error.\"}");
			};
			sprintf(ret_buf,"%s","{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{}}");
			
		}else {//访客
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"Not Admin\"}");
			
		}
		
		printf("%s",ret_buf);
		fflush(stdout);
		free(web_str);
		uci_free_context(ctx);
		return ;
}

