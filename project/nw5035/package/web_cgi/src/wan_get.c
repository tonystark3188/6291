
#include "msg.h"

void main()
{
		char ret_buf[RET_BUF_LEN];
		char code[CODE_LEN]="\0";
		char sid[SID_LEN]="\0";
		char fw_sid[SID_LEN]="\0";

		char ssid[33]="\0";
		char key[33]="\0";
		char type[32]="\0";
		char pid[32]="\0";
		char tmp_buf[256]="\0";
		char tmp_vid[32]="\0";
		char ap_sum[32]="\0";
		int i,j,k;
		char *web_str=NULL;
		int ret=0;
		int sum=0;

		char uci_option_str[UCI_BUF_LEN]="\0";
		ctx=uci_alloc_context();
		
		strcpy(uci_option_str,"system.@system[0].sid");			//name
		uci_get_option_value(uci_option_str,fw_sid);
		memset(uci_option_str,'\0',UCI_BUF_LEN);
		
		p_debug("fw_sid=%s\n",fw_sid);	


	
		printf("Content-type:text/plain\r\n\r\n");

		if((web_str=GetStringFromWeb())==NULL)
		{
			sprintf(ret_buf,"%s","{\"status\":0,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{},\"errorCode\":4,\"errorMessage\":\"Can not get any parameters.\"}");
			printf("%s",ret_buf);
			fflush(stdout);
			uci_free_context(ctx);
			return ;

		}
		processString(web_str,SID,sid);		
		p_debug("sid=%s\n",sid);		
		processString(web_str,CODE,code);
		p_debug("code=%s\n",code);		

			
		if(!strcmp(sid,fw_sid)){//是管理员
			strcpy(uci_option_str,"remoteAPlist.ap.sum");			//sum
			uci_get_option_value(uci_option_str,ap_sum);
			memset(uci_option_str,'\0',UCI_BUF_LEN);
			sum=atoi(ap_sum);
			p_debug("sum=%d\n",sum);
			sprintf(ret_buf,"%s","{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"ssidList\":[");
			if(sum!=0){
				for(i=0;i<sum;i++){
					sprintf(uci_option_str,"remoteAPlist.@ap[%d].ssid",i);			//name
					uci_get_option_value(uci_option_str,ssid);
					memset(uci_option_str,'\0',UCI_BUF_LEN);			
					p_debug("i=%d,ssid=%s\n",i,ssid);

					sprintf(uci_option_str,"remoteAPlist.@ap[%d].key",i);			//name
					uci_get_option_value(uci_option_str,key);
					memset(uci_option_str,'\0',UCI_BUF_LEN);			
					p_debug("i=%d,key=%s\n",i,key);
					if(i==(sum-1))
						sprintf(buf,"{\"ssid\":\"%s\",\"key\":\"%s\"}",ssid,key);
					else
						sprintf(buf,"{\"ssid\":\"%s\",\"key\":\"%s\"},",ssid,key);

					strcat(ret_buf,buf);	
				}
			}
			
			strcat(ret_buf,"]},\"result\":\"0\"}");
		}else{
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"Not Admin\"}");
		} 

		printf("%s",ret_buf);
		fflush(stdout);
		free(web_str);
		uci_free_context(ctx);
		return ;
}


