/*
 * =============================================================================
 *
 *       Filename:  server.c
 *
 *    Description:  hidisk server module.
 *
 *        Version:  1.0
 *        Created:  2015/3/19 10:54
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 515296288jf@163.com
 *   Organization:  
 *
 * =============================================================================
 */
#include "msg.h"

//Maximum 13 bytes
#define PRODUCT_MODEL  "letv"

//#define FW_FILE "/tmp/fwupgrade"
#define FW_FILE "/tmp/mnt/USB-disk-1/ota/update.bin"
#define FW_DL_FLIE "/tmp/mnt/USB-disk-1/ota/update.bin"
#define FW_LEN  16384000
int updateSysVal(const char *para,const char *val){
	char set_str[128]={0};
	char tmp[128]={0};
	sprintf(set_str,"sed -e \'/%s/d\' -i /tmp/state/status",para);
	system(set_str);
	memset(set_str,0,sizeof(set_str));
	
	sprintf(set_str,"echo \'%s=%s\' >> /tmp/state/status",para, val);
	system(set_str);
}

void main()
{
	FILE *fw_fp;
	int fw_len = 0;
	int f_size = 0;
	char ret_buf[2048];
	char sid[SID_LEN]="\0";
	char fw_sid[SID_LEN]="\0";
	char *web_str=NULL;
	int ret=0;
	char uci_option_str[128]="\0";
	char cmd[128];
	char tmp_str[1024];
	char *header = NULL;
	char op_fw_header[32]="\0";
	char product_model[32]="\0";
	
	memset(ret_buf, 0, 2048);
	
	ctx=uci_alloc_context();
	
	memset(uci_option_str,'\0',128);
	memset(fw_sid, 0, SID_LEN);
	strcpy(uci_option_str,"system.@system[0].sid");			//name
	uci_get_option_value(uci_option_str,fw_sid);

	printf("Content-type:text/html\r\n\r\n");

	if((web_str=GetStringFromWeb())==NULL)
	{
		fprintf(stdout,"can't get string from web\n");
		exit(1);
	}
	processString(web_str,SID,sid);		
	p_debug("fw_sid=(%s)",fw_sid);
	p_debug("sid=(%s)",sid);
	
	if(!strcmp(sid,fw_sid)){//是管理员
		//memset(cmd, 0, 128);
		//sprintf(cmd, "rm -f %s", FW_FILE);
		//system(cmd);
		//p_debug("cmd = %s", cmd);
		system("sync");
		sleep(1);
		system("echo 3 > /proc/sys/vm/drop_caches");
		sleep(1);
		//memset(cmd, 0, 128);
		//sprintf(cmd, "cp -f %s %s", FW_DL_FLIE, FW_FILE);
		//system(cmd);
		//p_debug("cmd = %s", cmd);
		
		if( (fw_fp=fopen(FW_FILE,"r"))==NULL){    //read,binary
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"Illegal Firmware\"}");
			goto exit;
		}
		
		memset(tmp_str, 0, 1024);
		f_size=fread(tmp_str, 1, 1024, fw_fp);
		if(f_size != 1024){
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"Illegal Firmware1\"}");
			fclose(fw_fp);
			goto exit;
		}
		
		header = tmp_str;
		memset(op_fw_header, 0, 32);
		strncpy(op_fw_header, header+37, 7);
		if(strcmp(op_fw_header, "OpenWrt")!=0)
		{
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"Illegal Firmware2\"}");
			fclose(fw_fp);
			goto exit;
		}
		
		memset(product_model, 0, 32);
		strncpy(product_model, header+0x33, 13);
		product_model[13] = '\0';
		if( strcmp(product_model,PRODUCT_MODEL)!=0 )
		{
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"Illegal Firmware3\"}");
			fclose(fw_fp);
			goto exit;
		}
		fseek(fw_fp,0,SEEK_END);
		fw_len = ftell(fw_fp);
		//if(fw_len != FW_LEN)
		if(0)			
		{	
			sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"Illegal Firmware4\"}");
			fclose(fw_fp);
			goto exit;
		}
		fclose(fw_fp);
		
		sprintf(ret_buf,"%s","{\"status\":1,\"data\":{}}");	
		p_debug("%s",ret_buf);
		printf("%s",ret_buf);
		fflush(stdout);
		free(web_str);
		uci_free_context(ctx);
		updateSysVal("start_upgrade","1");
		/*
		system("dd if=/tmp/mnt/USB-disk-1/ota/update.bin of=/tmp/image1_fw bs=1k conv=notrunc count=7936");
		system("dd if=/tmp/mnt/USB-disk-1/ota/update.bin of=/tmp/image1_md5 bs=1k skip=7936 conv=notrunc count=64");
		system("dd if=/tmp/mnt/USB-disk-1/ota/update.bin of=/tmp/image2_fw bs=1k skip=8000 conv=notrunc count=7936");
		system("dd if=/tmp/mnt/USB-disk-1/ota/update.bin of=/tmp/image2_md5 bs=1k skip=15936 conv=notrunc count=64");
		system("sync");
		system("rm /tmp/mnt/USB-disk-1/ota/update.bin");
		system("mcu_control -s 6");
		system("sysupgrade -d 1 /tmp/image1_fw &");
		*/
		return ;
	}
	else{
		sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"Not Admin\"}");
		goto exit;
	} 
	
exit:
	p_debug("%s",ret_buf);
	printf("%s",ret_buf);
	fflush(stdout);
	free(web_str);
	uci_free_context(ctx);
	return ;
}


