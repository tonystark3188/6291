#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "msg.h"

int main()
{
	char time[64]={0};
	char *value=NULL;
	char *zone=NULL;
	char *zone_tmp=NULL;
	char zone_str[32]="\0";
	char zone_cur[32]="\0";
	char str_sp[64]="\0";
	struct tm time_tm;
    struct timeval time_tv;
    time_t timep;
	struct timezone time_tz;
	int hour;
	int minutes;
	

	char *web_str=NULL;
	char uci_option_str[UCI_BUF_LEN]="\0";
	char ret_buf[RET_BUF_LEN]={0};
	
	memset(zone_str,0,sizeof(zone_str));
	memset(uci_option_str,0,sizeof(uci_option_str));
	memset(zone_cur,0,sizeof(zone_cur));
	printf("Content-type:text/plain\r\n\r\n");
	if((web_str=GetStringFromWeb())==NULL)
	{
		sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":4,\"errorMessage\":\"Can not get any parameters.\"}");
		printf("%s",ret_buf);
		fflush(stdout);
		uci_free_context(ctx);
		return ;
	
	}
	processString(web_str,"time",time); 
	p_debug("time=%s",time);
	value=urlDecode(time);
	
	ctx=uci_alloc_context();
		
	strcpy(zone_str,"UTC-8");
	strcpy(uci_option_str,"system.@system[0].timezone");  //status
	uci_get_option_value(uci_option_str,zone_cur);
	memset(uci_option_str,'\0',64);
	//Time_Sync_zone
	if(value !=NULL)
	{
		if(sscanf(value, "%d-%d-%d %d:%d:%d", &time_tm.tm_year, &time_tm.tm_mon, &time_tm.tm_mday, &time_tm.tm_hour, &time_tm.tm_min, &time_tm.tm_sec)!=6){
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":4,\"errorMessage\":\"parameters error.\"}");
			printf("%s",ret_buf);
			fflush(stdout);
			free(web_str);
			uci_free_context(ctx);
			return ;
		}else{
		
			time_tm.tm_year -= 1900;
			time_tm.tm_mon -= 1;
			time_tm.tm_wday = 0;
			time_tm.tm_yday = 0;
			time_tm.tm_isdst = 0;
			
			timep = mktime(&time_tm);
			time_tv.tv_sec = timep;
			time_tv.tv_usec = 0;
			time_tz.tz_dsttime=0;
			
			
			
			{
				
				
				
				hour=8;
				minutes=0;
				time_tz.tz_minuteswest=hour*60+minutes;
				
			}
			
			
			
			if(strcmp(zone_cur,zone_str)!=0)
			{
				sprintf(str_sp,"system.@system[0].timezone=%s",zone_str);
				uci_set_option_value(str_sp);
				memset(str_sp,0,64);
				sprintf(str_sp,"echo \"%s\">/tmp/TZ",zone_str);
				system(str_sp);
				memset(str_sp,0,64);
			}
			settimeofday(&time_tv, &time_tz);
		}
	}
	
	sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":0,\"errorMessage\":\"success\"}");
	printf("%s",ret_buf);
	fflush(stdout);
	system("uci commit");
	free(value);
	free(web_str);
	uci_free_context(ctx);	
	return 0;
}

