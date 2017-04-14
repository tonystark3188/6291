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
//#include "server.h"
//#include "hidisk_udp_server.h"
//#include "usr_manage.h"
#include "msg.h"

#define STR_VERSION_CODE		"versionCode"
#define STR_VERSION_NAME		"versionName"
#define INFO_HAS_NEW			"hasNew"
#define INFO_NEXT_VERSION_CODE	"nextVersionCode"
#define INFO_NEXT_VERSION_NAME	"nextVersionName"
#define INFO_NEXT_FEATURE		"nextFeature"
#define INFO_NEXT_UPDATE_URL	"nextUpdateUrl"
#define INFO_IS_FORCE			"isForce"

#define FILE_VERSION_INFO_PATH		"/etc/fw_version.conf"
#define FILE_UPGRADE_INFO_PATH		"/etc/fw_upgrade.conf"

typedef struct _fw_version_info{
	char curVersionCode[16];
	char curVersionName[32];
	char nextVersionCode[16];
	char nextVersionName[32];
	char hasNew[16];
	char nextFeature[512];
	char nextUpdateUrl[1024];
	char isForce[16];
	char errorMsg[128];
}fw_version_info;


/* return -1:error;0:get value success;1:no value */
int get_conf_str2(char *var, char *dest, char *file_path)
{
	char tmp[512];

	if((var == NULL) || (dest == NULL) || (file_path == NULL))
	{
		//p_debug("argument invalid \n", file_path);
		return -1;
	}
	
	FILE *fp=fopen(file_path, "r");
	if(NULL == fp)
	{
		//p_debug("open %s failed \n", file_path);
		return -1;
	}
	
	bzero(tmp,512);
	while(fgets(tmp,512,fp)!=NULL)
	{
		if('\n'==tmp[strlen(tmp)-1])
		{
			tmp[strlen(tmp)-1]=0;
		}
		//p_debug("get string from %s:%s\n", file_path, tmp);
		if(!strncmp(var,tmp,strlen(var)))
		{	
			bzero(dest, strlen(dest));
			strcpy(dest, tmp+strlen(var)+1);
			fclose(fp);
			return 0;
		}
		bzero(tmp, 512);
	}
	fclose(fp);
	return 1;
}


int get_fw_version(fw_version_info *p_version_info)
{
	int ret = 0;

	if(p_version_info == NULL){
		return -1;
	}

	ret = get_conf_str2(STR_VERSION_CODE, p_version_info->curVersionCode, FILE_VERSION_INFO_PATH);
	if(ret == 0){
		//p_debug("ret = %d, version_code = %s", ret, p_version_info->curVersionCode);
	}
	else{
		return -1;
	}

	ret = get_conf_str2(STR_VERSION_NAME, p_version_info->curVersionName, FILE_VERSION_INFO_PATH);
	if(ret == 0){
		//p_debug("ret = %d, version_code = %s", ret, p_version_info->curVersionName);
	}
	else{
		return -1;
	}
	
	return 0;
}

int get_fw_upgrade(fw_version_info *p_version_info)
{
	int ret = 0;
	if(p_version_info == NULL){
		return -1;
	}

	ret = get_conf_str2(INFO_HAS_NEW, p_version_info->hasNew, FILE_UPGRADE_INFO_PATH);
	if(ret == 0){
		//p_debug("ret = %d, version_code = %s", ret, p_version_info->curVersionCode);
	}
	else{
		return -1;
	}

	if(0 == strcmp(p_version_info->hasNew, "true"))
	{
		ret = get_conf_str2(INFO_NEXT_VERSION_CODE, p_version_info->nextVersionCode, FILE_UPGRADE_INFO_PATH);
		if(ret == 0){
			//p_debug("ret = %d, version_code = %s", ret, p_version_info->curVersionCode);
		}
		else{
			return -1;
		}

		ret = get_conf_str2(INFO_NEXT_VERSION_NAME, p_version_info->nextVersionName, FILE_UPGRADE_INFO_PATH);
		if(ret == 0){
			//p_debug("ret = %d, version_code = %s", ret, p_version_info->curVersionCode);
		}
		else{
			return -1;
		}

		ret = get_conf_str2(INFO_NEXT_FEATURE, p_version_info->nextFeature, FILE_UPGRADE_INFO_PATH);
		if(ret == 0){
			//p_debug("ret = %d, version_code = %s", ret, p_version_info->curVersionCode);
		}
		else{
			return -1;
		}

		ret = get_conf_str2(INFO_IS_FORCE, p_version_info->isForce, FILE_UPGRADE_INFO_PATH);
		if(ret == 0){
			//p_debug("ret = %d, version_code = %s", ret, p_version_info->curVersionCode);
		}
		else{
			return -1;
		}
	}
	return 0;
}


void main()
{
		char ret_buf[2048];
		char sid[SID_LEN]="\0";
		char fw_sid[SID_LEN]="\0";
		char *web_str=NULL;
		int ret=0;
		char uci_option_str[UCI_BUF_LEN]="\0";
		fw_version_info version_info;

		memset(&version_info, 0, sizeof(fw_version_info));
		memset(ret_buf, 0, 2048);
		
		ctx=uci_alloc_context();

		memset(uci_option_str,'\0',UCI_BUF_LEN);
		memset(fw_sid, 0 , SID_LEN);
		strcpy(uci_option_str,"system.@system[0].sid");			//name
		uci_get_option_value(uci_option_str,fw_sid);
		
		printf("Content-type:text/plain\r\n\r\n");

		if((web_str=GetStringFromWeb())==NULL)
		{
			fprintf(stdout,"can't get string from web\n");
			exit(1);
		}
		processString(web_str,SID,sid);		
	
		if(!strcmp(sid,fw_sid)){//是管理员

			ret = get_fw_version(&version_info);
			if(ret < 0){
				sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"get fw version error\"}");
				goto exit;
			}

			ret = get_fw_upgrade(&version_info);
			if(ret < 0){
				sprintf(ret_buf,"%s","{\"status\":0,\"data\":{},\"errorCode\":1,\"errorMessage\":\"get fw upgrade error\"}");
				goto exit;
			}
			
			sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"curVersion\":\"%s\",\"hasNew\":\"%s\",\"nextVersion\":\"%s\",\"nextFeature\":\"%s\",\"isForce\":\"%s\"}}", \
				version_info.curVersionName, version_info.hasNew, version_info.nextVersionName, version_info.nextFeature, version_info.isForce);
		}else{
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


