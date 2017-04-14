#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "http_letv.h"
#include "http_client.h"
#include "md5.h"
#include <pthread.h>
#include "uci_for_cgi.h"

int updateSysVal(const char *para,const char *val){
	char set_str[128]={0};
	char tmp[128]={0};
	sprintf(set_str,"sed -e \'/%s/d\' -i /tmp/state/status",para);
	system(set_str);
	memset(set_str,0,sizeof(set_str));
	
	sprintf(set_str,"echo \'%s=%s\' >> /tmp/state/status",para, val);
	system(set_str);
#if 0
	char uci_option_str[64]="\0";
	ctx=uci_alloc_context();
	uci_add_delta_path(ctx,"/tmp/state");
	uci_set_confdir(ctx,"/tmp/state");
	memset(uci_option_str,'\0',64); 
	sprintf(uci_option_str,"status.@downloading[0].%s=%s",para,val);		
	uci_set_option_value(uci_option_str);

	//system("uci commit");
	uci_free_context(ctx);
#endif
}

int compose_page(letv_http_client *p_letv_client, char **page, int req_type)
{
	char url_argv[URL_ARGV_LEN_256]="\0";
	int url_flag = 0;
	if(page == NULL)
		return -1;
	if(req_type==REQ_DOWNLOAD||(req_type==VIDEO_IMAGE))
		sprintf(page, "%s", LETV_SERVER_DOWNLOAD_PAGE);
	else if(req_type==REQ_QUERY_ALBUM)
		sprintf(page, "%s", LETV_SERVER_QUERY_PAGE);
	else if(req_type == REQ_REPORT_STATUS)
		sprintf(page, "%s", LETV_SERVER_REPORT_PATH);
	else if(req_type == REQ_QUERY_ALBUM_INFO)
		sprintf(page, "%s", LETV_SERVER_QUERY_ALBUM_INFO);
	else if(req_type == REQ_CHECK_FIRMWARE)
		sprintf(page, "%s", LETV_SERVER_CHECK_FIRMWARE);
	else{
		p_debug("unknown type!!");
		return -1;
	}
	
	if((req_type == REQ_DOWNLOAD)||(req_type==VIDEO_IMAGE) || (req_type == REQ_REPORT_STATUS)||(req_type == REQ_QUERY_ALBUM)||(req_type == REQ_QUERY_ALBUM_INFO)|| (req_type == REQ_CHECK_FIRMWARE)){
		if(strcmp(p_letv_client->authorize_code, "") != 0){
			memset(url_argv, '\0', URL_ARGV_LEN_256);
			if(url_flag)
				sprintf(url_argv, "&%s=%s", STR_AUTHORIZE_CODE, p_letv_client->authorize_code);
			else
				sprintf(url_argv, "?%s=%s", STR_AUTHORIZE_CODE, p_letv_client->authorize_code);
			strcat(page, url_argv);
			url_flag = 1;
		}
	}

	if((req_type == REQ_DOWNLOAD)||(req_type==VIDEO_IMAGE) || (req_type == REQ_REPORT_STATUS)||(req_type == REQ_QUERY_ALBUM)||(req_type == REQ_QUERY_ALBUM_INFO)|| (req_type == REQ_CHECK_FIRMWARE)){
	if(strcmp(p_letv_client->dev_type, "") != 0){
		memset(url_argv, '\0', URL_ARGV_LEN_256);
			if(url_flag)
				sprintf(url_argv, "&%s=%s", STR_DEV_TYPE, p_letv_client->dev_type);
			else
				sprintf(url_argv, "?%s=%s", STR_DEV_TYPE, p_letv_client->dev_type);
			strcat(page, url_argv);
			url_flag = 1;
		}
	}
#if 0
	if((req_type == REQ_DOWNLOAD)||(req_type == REQ_QUERY_ALBUM)||(req_type == REQ_QUERY_ALBUM_INFO)){
		if(strcmp(p_letv_client->vtype, "") != 0){
			memset(url_argv, '\0', URL_ARGV_LEN_256);
			if(url_flag)
				sprintf(url_argv, "&%s=%s", STR_VTYPE, p_letv_client->vtype);
			else
				sprintf(url_argv, "?%s=%s", STR_VTYPE, p_letv_client->vtype);
			strcat(page, url_argv);
			url_flag = 1;
		}
	}
#endif
	if((req_type == REQ_DOWNLOAD)||(req_type==VIDEO_IMAGE)||(req_type == REQ_QUERY_ALBUM)){
		if(strcmp(p_letv_client->vid, "") != 0){
			memset(url_argv, '\0', URL_ARGV_LEN_256);
			if(url_flag)
				sprintf(url_argv, "&%s=%s", STR_VID, p_letv_client->vid);
			else
				sprintf(url_argv, "?%s=%s", STR_VID, p_letv_client->vid);
			strcat(page, url_argv);
			url_flag = 1;
		}
	}

	if((req_type == REQ_DOWNLOAD)||(req_type==VIDEO_IMAGE)||(req_type == REQ_QUERY_ALBUM)||(req_type == REQ_QUERY_ALBUM_INFO)){
		if(strcmp(p_letv_client->pid, "") != 0){
			memset(url_argv, '\0', URL_ARGV_LEN_256);
			if(url_flag)
				sprintf(url_argv, "&%s=%s", STR_PID, p_letv_client->pid);
			else
				sprintf(url_argv, "?%s=%s", STR_PID, p_letv_client->pid);
			strcat(page, url_argv);
			url_flag = 1;
		}
	}

	if((req_type == REQ_DOWNLOAD)||(req_type==VIDEO_IMAGE)||(req_type == REQ_QUERY_ALBUM)){
		if(strcmp(p_letv_client->ext, "") != 0){
			memset(url_argv, '\0', URL_ARGV_LEN_256);
			if(url_flag)
				sprintf(url_argv, "&%s=%s", STR_EXT, p_letv_client->ext);
			else
				sprintf(url_argv, "?%s=%s", STR_EXT, p_letv_client->ext);
			strcat(page, url_argv);
			url_flag = 1;

		}
	}

	if((req_type == REQ_DOWNLOAD)||(req_type==VIDEO_IMAGE) || (req_type == REQ_REPORT_STATUS)||(req_type == REQ_QUERY_ALBUM)||(req_type == REQ_QUERY_ALBUM_INFO)||(req_type == REQ_CHECK_FIRMWARE)){
		if(strcmp(p_letv_client->ip, "") != 0){
			memset(url_argv, '\0', URL_ARGV_LEN_256);
			if(url_flag)
				sprintf(url_argv, "&%s=%s", STR_IP, p_letv_client->ip);
			else
				sprintf(url_argv, "?%s=%s", STR_IP, p_letv_client->ip);
			strcat(page, url_argv);
			url_flag = 1;
		}
	}

	if((req_type == REQ_DOWNLOAD) ||(req_type==VIDEO_IMAGE)|| (req_type == REQ_REPORT_STATUS)||(req_type == REQ_QUERY_ALBUM)||(req_type == REQ_QUERY_ALBUM_INFO)||(req_type == REQ_CHECK_FIRMWARE)){
		if(strcmp(p_letv_client->mac_id, "") != 0){
			memset(url_argv, '\0', URL_ARGV_LEN_256);
			if(url_flag)
				sprintf(url_argv, "&%s=%s", STR_MAC_ID, p_letv_client->mac_id);
			else
				sprintf(url_argv, "?%s=%s", STR_MAC_ID, p_letv_client->mac_id);
			strcat(page, url_argv);
			url_flag = 1;
		}
	}

	if((req_type == REQ_DOWNLOAD)||(req_type==VIDEO_IMAGE)||(req_type == REQ_QUERY_ALBUM)||(req_type == REQ_QUERY_ALBUM_INFO)){
		if(strcmp(p_letv_client->signature, "") != 0){
			memset(url_argv, '\0', URL_ARGV_LEN_256);
			if(url_flag)
				sprintf(url_argv, "&%s=%s", STR_SIGNATURE, p_letv_client->signature);
			else
				sprintf(url_argv, "?%s=%s", STR_SIGNATURE, p_letv_client->signature);
			strcat(page, url_argv);
			url_flag = 1;
		}
	}

	if(req_type == REQ_REPORT_STATUS){
		if(strcmp(p_letv_client->params, "") != 0){
			memset(url_argv, '\0', URL_ARGV_LEN_256);
			if(url_flag)
				sprintf(url_argv, "&%s=%s", STR_PARAMS, p_letv_client->params);
			else
				sprintf(url_argv, "?%s=%s", STR_PARAMS, p_letv_client->params);
			strcat(page, url_argv);
			url_flag = 1;
		}
	}
	
	if(req_type == REQ_CHECK_FIRMWARE){
		if(strcmp(p_letv_client->version_code, "") != 0){
			memset(url_argv, '\0', URL_ARGV_LEN_256);
			if(url_flag)
				sprintf(url_argv, "&%s=%s", STR_VERSION_CODE, p_letv_client->version_code);
			else
				sprintf(url_argv, "?%s=%s", STR_VERSION_CODE, p_letv_client->version_code);
			strcat(page, url_argv);
			url_flag = 1;
		}
	}

	if(req_type == REQ_CHECK_FIRMWARE){
		if(strcmp(p_letv_client->version_name, "") != 0){
			memset(url_argv, '\0', URL_ARGV_LEN_256);
			if(url_flag)
				sprintf(url_argv, "&%s=%s", STR_VERSION_NAME, p_letv_client->version_name);
			else
				sprintf(url_argv, "?%s=%s", STR_VERSION_NAME, p_letv_client->version_name);
			strcat(page, url_argv);
			url_flag = 1;
		}
	}

	if(req_type == REQ_CHECK_FIRMWARE){
		if(strcmp(p_letv_client->sign_term, "") != 0){
			memset(url_argv, '\0', URL_ARGV_LEN_256);
			if(url_flag)
				sprintf(url_argv, "&%s=%s", STR_SIGN_TERM, p_letv_client->sign_term);
			else
				sprintf(url_argv, "?%s=%s", STR_SIGN_TERM, p_letv_client->sign_term);
			strcat(page, url_argv);
			url_flag = 1;
		}
	}

	if(req_type==REQ_QUERY_ALBUM||req_type==REQ_REPORT_STATUS||req_type==REQ_CHECK_FIRMWARE) p_debug("page=%s",page);
	if(req_type==REQ_DOWNLOAD||(req_type==VIDEO_IMAGE)) p_debug("page=%s",page);

	return 0;
}


int parse_source_info_json(http_tcpclient *pclient, char *p_buff,int type)
{
	int i,j,ret = 0; 
	char state[16]="\0";
	unsigned long len=0;
	char tmp_vid[VID_LEN]="\0";
	p_debug("p_buff = %s", p_buff);
	len=strlen(p_buff);
	p_debug("len=%lu",len);
	if(type == REQ_DOWNLOAD)p_debug("33333333333=%lu",pclient->dn->downloaded_size);

	char *p_buff2= (char*)malloc((len+1)*sizeof(char));
	if(p_buff2==NULL)//如果malloc失败，可以得到一些log
	{
		p_debug("malloc error");
		ret = -1;
		pclient->errorCode=DOWNLOAD_URL_ERROR;
		strcpy(pclient->error_msg,"malloc error");
		return ret;
	}
	p_debug("len=%lu",sizeof(p_buff2));

	memset(p_buff2,0,len+1);
	p_debug("len=%lu",len);
	for(i=0,j=0;i<strlen(p_buff);i++){
		if(p_buff[i] != '\r')
			{
			p_buff2[j]=p_buff[i];
			//i++;
			j++;
		}
		else if(p_buff[i] == '\r'){
//			if(len)
			//p_debug("p_buff0[%d]=%x",i,p_buff[i]);
			i=i+2;//skip '\n'
			while(p_buff[i] != '\n'){
				//p_debug("p_buff1[%d]=%x",i,p_buff[i]);
				i++;
				
			}
			//p_debug("p_buff2[%d]=%x",i,p_buff[i]);
			//p_debug("p_buff3[%d]=%x",i+1,p_buff[i+1]);
		}
	}
	p_buff2[j]='\0';
	
	//free(p_buff);p_buff=NULL;
	p_debug("p_buff2 = %s,strlen=%lu", p_buff2,strlen(p_buff2));
/**/

	//strstr(p_buff,"\r\n");
	JObj *r_json = JSON_PARSE(p_buff2);
	if(r_json == NULL)
	{
		p_debug("access NULL");
		safe_free(p_buff2);
		ret = -1;
		pclient->errorCode=SERVER_ERROR;
		strcpy(pclient->error_msg,"server returned null");
		goto EXIT;
	}

	p_debug("111");
	JObj *state_json = JSON_GET_OBJECT(r_json,"state");
	if(state_json == NULL)
	{
		ret = -1;
		pclient->errorCode=DOWNLOAD_URL_ERROR;
		updateVer(0);
		strcpy(pclient->error_msg,"server return state error");
		goto EXIT;
	}
	memset(state, 0, 16);	
	strcpy(state, JSON_GET_OBJECT_VALUE(state_json,string));

	if(!strcmp(state, "error"))
	{
		p_debug("222");
		JObj *msg_json = JSON_GET_OBJECT(r_json,"msg");
		if(msg_json == NULL)
		{
			ret = -1;
			pclient->errorCode=DOWNLOAD_URL_ERROR;
			updateVer(0);
			strcpy(pclient->error_msg,"server return msg error");
			goto EXIT;
		
		}
		if(type==REQ_DOWNLOAD){
			memset(pclient->dn->vid_url,0,sizeof(pclient->dn->vid_url));

			pclient->errorCode=DOWNLOAD_URL_ERROR;
			update_task_status_to_list(pclient->dn->pid,pclient->dn->vid,DOWNLOAD_URL_ERROR);
			updateVer(0);
		}		
		strcpy(pclient->error_msg, JSON_GET_OBJECT_VALUE(msg_json,string));
		ret=-1;
		goto EXIT;
	}
	else if(!strcmp(state, "pay")){
			if(type==REQ_DOWNLOAD||(type==VIDEO_IMAGE))
				pclient->dn->download_status=PAUSE;
			
			pclient->errorCode=SERVER_ERROR;
			updateVer(0);
			strcpy(pclient->error_msg,"收费视频!");
			p_debug(pclient->error_msg);
			ret=-2;
			goto EXIT;
	}
	else
	{	
		if(type == REQ_DOWNLOAD)p_debug("444444444=%lld",pclient->dn->downloaded_size);
		p_debug("333");
		JObj *data_json = JSON_GET_OBJECT(r_json,"data");
		if(data_json == NULL)
		{
			ret = -1;
			pclient->errorCode=DOWNLOAD_URL_ERROR;
			updateVer(0);
			strcpy(pclient->error_msg,"parse data error");
			goto EXIT;
		}

		if((type == VIDEO_IMAGE)){
			JObj *image_url_json = JSON_GET_OBJECT(data_json,"coverImgUrl");
            if(image_url_json == NULL)
            {
                    ret = -1;
                    pclient->errorCode=DOWNLOAD_URL_ERROR;
                    strcpy(pclient->error_msg,"parse coverImgUrl error");
                    goto EXIT;
            }
			strcpy(pclient->dn->img_url, JSON_GET_OBJECT_VALUE(image_url_json, string));
			
			JObj *info_json = JSON_GET_OBJECT(data_json,"info");
			if(info_json == NULL)
			{
				ret = -1;
				pclient->errorCode=DOWNLOAD_URL_ERROR;
				updateVer(0);
				strcpy(pclient->error_msg,"parse video info error");
				goto EXIT;
			}

			sprintf(pclient->dn->info,"%s",JSON_GET_OBJECT_VALUE(info_json, string));
			//pclient->dn->info = p_info;
			if(type == REQ_DOWNLOAD)p_debug("888888=%lld",pclient->dn->downloaded_size);
			if(type == REQ_DOWNLOAD)p_debug("REQ_DOWNLOAD info=%s,strlen=%d",pclient->dn->info,strlen(pclient->dn->info));

			p_debug("888");
	
		}
		if(type == REQ_DOWNLOAD){
			
			JObj *image_url_json = JSON_GET_OBJECT(data_json,"coverImgUrl");
			if(image_url_json == NULL)
			{
				ret = -1;
				pclient->errorCode=DOWNLOAD_URL_ERROR;
				strcpy(pclient->error_msg,"parse coverImgUrl error");
				goto EXIT;
			}
			if(type == REQ_DOWNLOAD)p_debug("555555555=%lld",pclient->dn->downloaded_size);
			strcpy(pclient->dn->img_url, JSON_GET_OBJECT_VALUE(image_url_json, string));

			p_debug("444");

			JObj *video_url_json = JSON_GET_OBJECT(data_json,"url");
			if(video_url_json == NULL)
			{
				ret = -1;
				pclient->errorCode=DOWNLOAD_URL_ERROR;
				updateVer(0);
				strcpy(pclient->error_msg,"parse video url error");
				goto EXIT;
			}

			p_debug("5555");
			if(type == REQ_DOWNLOAD)p_debug("66666666666=%lld",pclient->dn->downloaded_size);
			if(pclient->dn->isAutoAdd==0)
				strcpy(pclient->dn->vid_url, JSON_GET_OBJECT_VALUE(video_url_json, string));		
			else if(!strcmp(pclient->dn->vid_url,""))// 否则自动追剧添加的，下载出错后无法再下载。
				strcpy(pclient->dn->vid_url, JSON_GET_OBJECT_VALUE(video_url_json, string));		
			
			//strcpy(p_video_url, "http://14.22.1.164/161/40/44/letv-uts/14/ver_00_22-1003231504-avc-476238-aac-32047-127840-8265462-781550b5f80af6c414c990da44bf14d7-1445479817156.mp4?crypt=76aa7f2e51600&b=516&nlh=3072&nlt=45&bf=8000&p2p=1&video_type=mp4&termid=0&tss=no&geo=CN-19-246-1&platid=5&splatid=500&its=0&qos=5&fcheck=0&proxy=2006169252,236015216,3683272586&uid=1946578901.rp&keyitem=GOw_33YJAAbXYE-cnQwpfLlv_b2zAkYctFVqe5bsXQpaGNn3T1-vhw..&ntm=1447089600&nkey=9e2e934c12c3111a03673425b5a41109&nkey2=2f5fa386993d4c58dbfdb11221c95530&mltag=1&mmsid=36269663&tm=1447053276&key=8036f46ef0de7d6e62a3473ff1229ff2&playid=2&vtype=13&cvid=915062003682&payff=0&errc=0&gn=1026&buss=5&cips=116.6.111.213");
			//pclient->dn->vid_url = p_video_url;
			p_debug("1pppppppp1=%s,%s",pclient->dn->vid,pclient->dn->vid_url);

			JObj *info_json = JSON_GET_OBJECT(data_json,"info");
			if(info_json == NULL)
			{
				ret = -1;
				pclient->errorCode=DOWNLOAD_URL_ERROR;
				updateVer(0);
				strcpy(pclient->error_msg,"parse video info error");
				goto EXIT;
			}
			if(type == REQ_DOWNLOAD)p_debug("77777777777=%lld",pclient->dn->downloaded_size);
			if(type == REQ_DOWNLOAD)p_debug("REQ_DOWNLOAD info=%s",pclient->dn->info);

			sprintf(pclient->dn->info,"%s",JSON_GET_OBJECT_VALUE(info_json, string));
			//pclient->dn->info = p_info;
			if(type == REQ_DOWNLOAD)p_debug("888888=%lld",pclient->dn->downloaded_size);
			if(type == REQ_DOWNLOAD)p_debug("REQ_DOWNLOAD info=%s,strlen=%d",pclient->dn->info,strlen(pclient->dn->info));

			p_debug("888");
	/*		safe_free(p_info);p_info=NULL;
			p_debug("9999");
			safe_free(p_video_url);
			p_debug("101010");
			safe_free(p_image_url); 
		*/	
		} else 
		if(type == REQ_QUERY_ALBUM){
			JObj *medi_array_obj;//=JSON_NEW_EMPTY_OBJECT();
			//struct task_dnode *cur;
			//should malloc a task node for pclient. *******
			//cur = xzalloc(sizeof(*cur));
			//pclient->dn=cur;
			p_debug("data_json=%s",JSON_TO_STRING(data_json));
			JObj *list_json=JSON_GET_OBJECT(data_json,"list");
			JObj *params_json=JSON_GET_OBJECT(data_json,"params");
			JObj *isEnd_json=JSON_GET_OBJECT(data_json,"isEnd");
			int isEnd=JSON_GET_OBJECT_VALUE(isEnd_json,int);
			if(isEnd==1)
				{
				pclient->adn->isEnd=1;
			}else 
				pclient->adn->isEnd=0;
			int data_len=JSON_GET_ARRAY_LEN(list_json);
			int i=data_len;	
			p_debug("album,vid_array_len=%d",data_len);

			//if(data_len==0)safe_free(pclient->dn);//memory leak
			int j=0;
			while(j<i) {
				// get the i-th object in medi_array
				struct task_dnode *task_cur;
				task_cur = xzalloc(sizeof(*task_cur));
				if(!task_cur)
				{
					p_debug("xzalloc error");
					goto EXIT;
				}
				memset(task_cur,0,sizeof(*task_cur));

				pclient->dn=task_cur;

				medi_array_obj = json_object_array_get_idx(list_json, j);
				JObj *vid_json = JSON_GET_OBJECT(medi_array_obj,"vid");
				JObj *url_json = JSON_GET_OBJECT(medi_array_obj,"url");
				JObj *info_json = JSON_GET_OBJECT(medi_array_obj,"info");
				JObj *coverImgUrl_json = JSON_GET_OBJECT(medi_array_obj,"coverImgUrl");
				if((vid_json != NULL)&&(url_json != NULL)){
					strcpy(pclient->dn->vid,JSON_GET_OBJECT_VALUE(vid_json, string));	
					memset(pclient->dn->vid_url,0,sizeof(pclient->dn->vid_url));
					strcpy(pclient->dn->vid_url,JSON_GET_OBJECT_VALUE(url_json, string));	
					strcpy(pclient->dn->info,JSON_GET_OBJECT_VALUE(info_json, string));						
					memset(pclient->dn->img_url,0,sizeof(pclient->dn->img_url));
					strcpy(pclient->dn->img_url,JSON_GET_OBJECT_VALUE(coverImgUrl_json, string));						
					p_debug("2pppppppp2=%s,%s",pclient->dn->vid,pclient->dn->vid_url);
					strcpy(pclient->dn->pid,pclient->adn->pid);	
					
					//pclient->dn->isDeleted=0;
					p_debug("add task vid=%s,pid=%s,vid_url=%s",pclient->dn->vid,pclient->dn->pid,pclient->dn->vid_url);
					

					//pclient->dn->downloaded_size=0;
					//pclient->dn->download_status=WAITING;
					pthread_rwlock_trywrlock(&task_list_lock); 
					ret=add_task_to_list(&task_dn,pclient->dn,2);
					pthread_rwlock_unlock(&task_list_lock);
					p_debug("save to list");
					updateVer(0);
				}
				j++;
			}
			//JSON_PUT_OBJECT(medi_array_obj);
/**/
		}else if(type == REQ_QUERY_ALBUM_INFO){
		p_debug("REQ_QUERY_ALBUM_INFO parse_source_info_json");

				JObj *pid_json = JSON_GET_OBJECT(data_json,"pid");
				if(pid_json == NULL)
				{
					ret = -1;
					pclient->errorCode=DOWNLOAD_URL_ERROR;
					updateVer(0);
					strcpy(pclient->error_msg,"parse pid error");
					goto EXIT;
				}
				sprintf(pclient->adn->pid,"%s",JSON_GET_OBJECT_VALUE(pid_json,string));
				p_debug("REQ_QUERY_ALBUM_INFO pid=%s",pclient->adn->pid);
				
				JObj *coverImgUrl_json=JSON_GET_OBJECT(data_json,"coverImgUrl");
				if(pid_json == NULL)
				{
					ret = -1;
					pclient->errorCode=DOWNLOAD_URL_ERROR;
					updateVer(0);
					strcpy(pclient->error_msg,"parse coverImgUrl error");
					goto EXIT;
				}
				sprintf(pclient->adn->coverImgUrl,"%s",JSON_GET_OBJECT_VALUE(coverImgUrl_json,string));
				p_debug("REQ_QUERY_ALBUM_INFO img=%s",pclient->adn->coverImgUrl);

				JObj *info_json=JSON_GET_OBJECT(data_json,"info");
				if(pid_json == NULL)
				{
					ret = -1;
					pclient->errorCode=DOWNLOAD_URL_ERROR;
					updateVer(0);
					strcpy(pclient->error_msg,"parse album info error");
					goto EXIT;
				}
				p_debug("info_json=%s",JSON_TO_STRING(info_json));
				sprintf(pclient->adn->info,"%s",JSON_GET_OBJECT_VALUE(info_json,string));
				
				p_debug("REQ_QUERY_ALBUM_INFO info=%s,strlen=%d",pclient->adn->info,strlen(pclient->adn->info));
				ret=update_album_to_list(album_dn,pclient->adn);
				if(ret<0){
					p_debug("write pid=%s to list error.",pclient->adn->pid);
				}

		}
			
				
	}
	p_debug("leave parse_source_info_json");
	return ret;
	
EXIT:
	p_debug("leave parse_source_info_json");
	if(r_json != NULL)JSON_PUT_OBJECT(r_json);//memory leak
	safe_free(p_buff2);
	return ret;
}


int letv_client_get_signature(letv_http_client *p_letv_client, char *src)
{
	p_debug("access letv_client_get_signature");
	int i = 0;
	yasm_md5_context context;
	unsigned char checksum[16]="\0";;
	char tmp[64]="\0";;
	if(p_letv_client == NULL || src == NULL)
	{
		return -1;
	}
	
	memset(checksum, 0, 16);
    yasm_md5_init (&context);
	yasm_md5_update (&context, src, strlen (src));
	yasm_md5_final (checksum, &context);
	memset(tmp, 0, 64);
	for(i = 0; i < 16; i++)
	{
		//p_debug ("%02x", (unsigned int) checksum[i]);
 		sprintf(tmp+2*i, "%02x", (unsigned int) checksum[i]);
	}
	strcpy(p_letv_client->signature, tmp);
	//p_debug("666 dest = %s", p_letv_client->signature);
	return 0;
}


int letv_client_get_mac_id(letv_http_client *p_letv_client)
{
/*	FILE *read_fp = NULL;
	char tmp_mac[32]="\0";
	read_fp = popen("hexdump -s 4 -n 6 -C /dev/mtd3 | head -n 1 | sed 's/\ \ /:/g' | cut -d: -f 2 | sed 's/\ /:/g' | tr \"[a-z]\" \"[A-Z]\"", "r");
	if(read_fp != NULL)
	{
		memset(tmp_mac,0,32);
		fgets(tmp_mac, 32-1, read_fp);
	}
	else
		return -1;
*/	
	p_debug("tmp_mac=%s",tmp_mac);
	strncpy(p_letv_client->mac_id, tmp_mac, 17);
	p_letv_client->mac_id[17]='\0';
	p_debug("mac_id=%s",p_letv_client->mac_id);
	//pclose(read_fp);
	return 0;
}

/* return -1:error;0:get value success;1:no value */
int get_conf_str(char *var, char *dest, char *file_path)
{
	char tmp[512]="\0";

	if((var == NULL) || (dest == NULL) || (file_path == NULL))
	{
		p_debug("argument invalid \n", file_path);
	}
	
	FILE *fp=fopen(file_path, "r");
	if(NULL == fp)
	{
		p_debug("open %s failed \n", file_path);
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


int letv_client_get_version(letv_http_client *p_letv_client)
{
	int ret = 0;
	ret = get_conf_str(STR_VERSION_CODE, p_letv_client->version_code, FILE_VERSION_INFO_PATH);
	if(ret == 0)
	{
		p_debug("ret = %d, version_code = %s", ret, p_letv_client->version_code);
	}

	ret = get_conf_str(STR_VERSION_NAME, p_letv_client->version_name, FILE_VERSION_INFO_PATH);
	if(ret == 0)
	{
		p_debug("ret = %d, version_code = %s", ret, p_letv_client->version_name);
	}
	
	strcpy(p_letv_client->sign_term, LETV_SIGN_TERM);
	return 0;
}

void replace_space(char *tmp_buf,int leng)
{
	int i;
	for(i=leng;i>0;i--)
	{
		if(tmp_buf[i-1] == ' ')
		{
			tmp_buf[i-1]='-';
		}
	}
}
int letv_client_compose_params(letv_http_client *p_letv_client)
{
	//ssid
	char osCode[8]={0};
	char osVer[32]={0};
	char osModel[32]={0};
	char total[32]={0};	
	char used[32]={0};		
	char sn[32]={0};	
	char userPhone[32]={0};	
	char client_ssid[32]={0};
	//osCode,android 10000,ios 20000,pc 30000
	get_conf_str2(osCode,"osCode");
	//osModel
	get_conf_str2(osModel,"osModel");
	get_conf_str2(osVer,"osVer");

	//capacity
	get_conf_str2(total,"tot_s");
	get_conf_str2(used,"used_s");
	get_conf_str2(client_ssid,"client_ssid");
	//sn
	get_conf_str2(sn,"sn");

	//userPhone
	get_conf_str2(userPhone,"userPhone");

	letv_client_get_version(p_letv_client);
	sprintf(p_letv_client->params, "{\"power\":89,\"ssid\":\"%s\",\"ver\":\"%s\",\"osCode\":\"%s\",\"osVer\":\"%s\",\"osModel\":\"%s\",\"sn\":\"%s\",\"userPhone\":\"%s\",\"capacity\":{\"total\":\"%s\",\"used\":\"%s\"}}" \
		,client_ssid,p_letv_client->version_name,osCode,osVer,osModel,sn,userPhone,total,used);
	replace_space(p_letv_client->params,strlen(p_letv_client->params));
	//p_debug("p_letv_client->params=%s",p_letv_client->params);
	return 0;
}

int letv_client_get_source(http_tcpclient *pclient,int type)
{
	p_debug("access letv_client_get_source");
	p_debug("type=%d",type);
	char page[BUFFER_SIZE]="\0";
	char str[512]="\0";
	int	host_port;
	int n;
	int ret = 0;
	//char	*request = "p1=hello";
	char	*response = NULL;
	char	*request = "get_source";//"<input name='filepath' type='file' />";
	char host_ip[HOST_IP_LEN_256];
	//char    *pHost;
	int time_out = 0;
	letv_http_client letv_client;
	
	memset(&letv_client, 0, sizeof(letv_http_client));
	strcpy(letv_client.authorize_code, LETV_AUTHORIZECODE);
	strcpy(letv_client.dev_type, LETV_DEV_TYPE);
	//strcpy(letv_client.mac_id, "08:60:6e:81:87:23");
	//strcpy(letv_client.signature, "5068596b2b4953ae1ccc4a800f90f687");
	//strcpy(page, LETV_SERVER_PAGE);
	if(type == REQ_DOWNLOAD||type==VIDEO_IMAGE){ 
		sprintf(letv_client.vid,"%s", pclient->dn->vid);
		sprintf(letv_client.ext,"%s", pclient->dn->ext);
	}

	if(type == REQ_QUERY_ALBUM)p_debug("query ablum id=%s",pclient->adn->pid);

	if((type == REQ_QUERY_ALBUM)){
		if(!strcmp(pclient->adn->pid,"")){
			return -1;
		}
		strcpy(letv_client.pid, pclient->adn->pid);
		if(!strcmp(pclient->adn->ext,"")){
			p_debug("ext is null");
			return -1;
		}
		p_debug("pclient->adn->ext=%s",pclient->adn->ext);
		memset(letv_client.ext,0,256);
		strncpy(letv_client.ext,pclient->adn->ext,EXT_LEN);
//		p_debug("ttttttttttt,letv_client.ext=%s",letv_client.ext);
		
	}else if(type == REQ_QUERY_ALBUM_INFO)
	{
		if(!strcmp(pclient->adn->pid,"")){
			p_debug("pid is null");
			return -1;
		}
		strcpy(letv_client.pid, pclient->adn->pid);
	}
	else if(type == REQ_DOWNLOAD||type==VIDEO_IMAGE){
		if(!strcmp( pclient->dn->pid,""))sprintf(letv_client.pid, "-%s", letv_client.vid);
		else sprintf(letv_client.pid, "%s", pclient->dn->pid);
	}

	if(letv_client_get_mac_id(&letv_client) < 0){
		p_debug("get mac id error");
		return -1;
	}
	else{
		p_debug("letv_client.mac_id = %s", letv_client.mac_id);
	}
		
	memset(str, 0, sizeof(str));
	if(type == REQ_DOWNLOAD||type==VIDEO_IMAGE){
		strcpy(str, letv_client.vid);
		strcat(str, letv_client.pid);
	}else if((type == REQ_QUERY_ALBUM)||(type == REQ_QUERY_ALBUM_INFO)){
		strcpy(str, letv_client.pid);
	}
	strcat(str, letv_client.mac_id);
	strcat(str, letv_client.authorize_code);
	p_debug("str = %s", str);
	if(letv_client_get_signature(&letv_client, str) < 0){
		p_debug("get signature error");
		return -1;
	}
	else{
		p_debug("letv_client.signature = %s", letv_client.signature);
	}

	if(compose_page(&letv_client, &page,type) < 0)
	{
		p_debug("compose page error");
		return -1;
	}
	
	memset(host_ip, 0, sizeof(host_ip));
	strcpy(host_ip, LETV_SERVER_IP);
	host_port = LETV_SERVER_PORT;
	time_out = HTTP_GET_INFO_TIME_OUT; 
	
	if ((n = http_tcpclient_create(pclient, host_ip, host_port,time_out)) < 0) {
		p_debug("Create socket error.n = %d\n",n);
		if(type == REQ_DOWNLOAD)
			//system("mcu_control -s 1");
			system("pwm_control 1  1 0;pwm_control 1 0 0");
		return -1;
		//goto EXIT;
	}

	if ((n = http_tcpclient_conn(pclient)) < 0) {
		p_debug("Connet srv error.\n");
		ret = -1;
		if(type == REQ_DOWNLOAD)
	//		system("mcu_control -s 1");		
			system("pwm_control 1  1 0;pwm_control 1 0 0");
		goto EXIT;
	}
	//updateSysVal("net","true");
/*
	if (http_get_info(pclient, page, "", &response) < 0) {
		p_debug("GET !\n");
		ret = -1;
		goto EXIT;
	}
	else{
		p_debug("GET:%d:%s\n",strlen(response),response);
	}
*/
	if(type == REQ_DOWNLOAD)p_debug("0000000000=%lld",pclient->dn->downloaded_size);

	if (http_post(pclient,page, request, &response)) {
		ret=-1;
		if(type == REQ_DOWNLOAD)pclient->dn->download_status=RETRY_DOWNLOAD;
		p_debug("http_post return error!");
		goto EXIT;
	}
	else
		p_debug("POST SUCCESS:%d:%s\n",strlen(response),response);
	//p_debug("111111111=%lu",pclient->dn->downloaded_size);
	//if(type==REQ_QUERY_ALBUM)
	//	{}
	//	else

	if(type == REQ_DOWNLOAD)p_debug("111111111111111=%lld",pclient->dn->downloaded_size);
	ret=parse_source_info_json(pclient, response,type);
		
	if( ret< 0)
	{
		p_debug("PARSE json ERROR!");
		ret = -1;
		goto EXIT;
	}
	else if(ret==0)
	{
	
	//	p_debug("pclient->dn->img_url = %s", pclient->dn->img_url);
	//	p_debug("pclient->dn->vid_url = %s", pclient->dn->vid_url);
	//	p_debug("pclient->dn->info = %s", pclient->dn->info);	
		p_debug("PARSE json SUCCESS!");	
	}	
	//if(type == REQ_DOWNLOAD)p_debug("2222222222222=%lld",pclient->dn->downloaded_size);

EXIT:	
	p_debug("PARSE ERROR!2");
	safe_free(response);
	p_debug("PARSE ERROR!3");
	http_tcpclient_close(pclient);
	p_debug("PARSE ERROR!4,ret=%d",ret);
	return ret;	
}

int letv_client_parse_url(char *url, char *host, int *port, char *path)
{
	const char *parseptr1 = NULL;
    const char *parseptr2 = NULL;
	char *port_str[8];
	if(host == NULL || port == NULL || path == NULL)
		return -1;
	
	parseptr2 = url;
	//parseptr2 = "http://123.125.89.22:8080/206/4/19/letv-uts/14/ver_00_22-1046642232-avc-197554-aac-32000-2797960-83351657-8794fae54c2880c95a4f2e88fdacefa6-1464514379186.mp4?crypt=10aa7f2e23800&b=238&nlh=4096&nlt=60&bf=8000&p2p=1&video_type=mp4&termid=0&tss=no&platid=2&splatid=203&its=0&qos=2&fcheck=0&mltag=6&proxy=2016684210,2016685017,467476889&uid=171582080.rp&keyitem=GOw_33YJAAbXYE-cnQwpfLlv_b2zAkYctFVqe5bsXQpaGNn3T1-vhw..&ntm=1465059000&nkey=920609b26e5dc3021604fadc5b186023&nkey2=f9b9f6f54c5b8e2ad8527bc177d3ac4e&geo=CN-1-9-666&mmsid=57776507&tm=1465022672&key=ee5c145f9ca0ead5b5084bb440af9608&playid=2&vtype=21&cvid=1707476259080&payff=0&errc=0&gn=706&vrtmcd=102&buss=6&cips=10.58.34.128";
	parseptr1 = strchr(parseptr2, ':');
	if(parseptr1 == NULL)
	{
		p_debug("parseptr1 url error");
		return -1;
	}
	parseptr1 = parseptr1 + 3;
	parseptr2 = parseptr1;
	parseptr1 = strchr(parseptr2, ':');
	p_debug("parseptr1 = %s, parseptr2 = %s", parseptr1, parseptr2);
	if(parseptr1 == NULL)
	{
		parseptr1 = strchr(parseptr2, '/');
		if(parseptr1 == NULL)
		{
			p_debug("pares url error1");
			return -1;
		}
		memcpy(host, parseptr2, parseptr1-parseptr2);
		*port = 80;
		p_debug("host = %s, port = %d", host, *port);
	}
	else
	{
		memcpy(host, parseptr2, parseptr1-parseptr2);
		p_debug("host = %s", host);
		parseptr2 = parseptr1;
		parseptr1 = strchr(parseptr1, '/');
		if(parseptr1 == NULL)
		{
			p_debug("pares url error2");
			return -1;
		}
		memset(port_str, 0, 8);
		memcpy(port_str, parseptr2+1, parseptr1-parseptr2);
		*port = atoi(port_str);
		p_debug("port = %d", *port);
	}

	parseptr2 = parseptr1;
	parseptr1 = parseptr1 + strlen(parseptr2);
	p_debug("parseptr1 = %s, parseptr2 = %s", parseptr1, parseptr2);
	if(parseptr1 == NULL)
	{
		p_debug("pares url error");
		return -1;
	}
	memcpy(path, parseptr2, parseptr1-parseptr2);
	p_debug("path = %s", path);
	return 0;
}


int letv_client_image_download(http_tcpclient *pclient,int type)
{
	char	page[BUFFER_SIZE]="\0";
	int		n, port;
	char	*request = "image_download";
	char	*response=NULL;
	int     i = 0;
	//char    url[BUFFER_SIZE];
	char    host[BUFFER_SIZE_1024]="\0";
	int 	time_out = 0;
	int 	ret=0;
    memset(host, 0, sizeof(host));
	memset(page, 0, sizeof(page));

	p_debug("type=%d",type);
	char url[URL_LEN]="\0";
	if(type==VIDEO_IMAGE)strcpy(url,pclient->dn->img_url);
	else if(type==ALBUM_IMAGE){
		strcpy(url,pclient->adn->coverImgUrl);
	}
    if ((n = letv_client_parse_url(url, host, &port, page)) < 0)
    {
		p_debug("parse url error");
		return;
	}
 
	time_out = HTTP_DOWNLOAD_TIME_OUT;
	if ((n = http_tcpclient_create(pclient, host, port,time_out)) < 0) {
		p_debug("Create socket error.\n");
		return -1;
	}
	if ((n = http_tcpclient_conn(pclient)) < 0) {
		p_debug("Connet srv error.\n");
		ret = -1;
		goto exit;
	}

	if (http_get(pclient, page, "", &response,type)<0) {
		p_debug("GET IMAGE!\n");
		ret=-1;
		goto exit;
	}
	else{
		p_debug("free response.1");
		updateVer(0);
	}
exit:
	safe_free(response);
	http_tcpclient_close(pclient);
	p_debug("leave letv_client_image_download");
	return 0;
}

int update_follow_finished_time(struct task_dnode *dn){
	int i=0;
	struct album_node *adn=album_dn;
	for(i=0;;i++){
		if(!adn)
			break;
		if(!strcmp(adn->pid,dn->pid)){
			adn->update_time=time(NULL);
			p_debug("adn->update_time=%d",adn->update_time);
		}
		adn=adn->dn_next;
	}
}

int letv_client_video_download(http_tcpclient *pclient,int type)
{
	char	page[BUFFER_SIZE]="\0";
	int	n, port;
	char	*request = "video_download";
	char	*response = NULL;
	int     i = 0;
	char    host[BUFFER_SIZE_1024]="\0";
	int 	time_out = 0;
	int 	ret=0;
    memset(host, 0, sizeof(host));
    memset(page, 0, sizeof(page));
    p_debug("vid_url=%s",pclient->dn->vid_url);	
	p_debug("vid_re_url=%s",pclient->dn->vid_re_url);	
    if ((n = letv_client_parse_url(pclient->dn->vid_re_url, host, &port, page)) < 0)
    {
		p_debug("parse url error");
		return -2;
	}

 	p_debug("page = %s", page);
	time_out = HTTP_DOWNLOAD_TIME_OUT;
	if ((n = http_tcpclient_create(pclient, host, port,time_out)) < 0) {
		p_debug("Create socket error.\n");
		return -3;
	}
	if ((n = http_tcpclient_conn(pclient)) < 0) {
		p_debug("Connet srv error.\n");
		ret =-4;
		goto  exit;
	}
	if(type==GET_VIDEO_SIZE)
	{
		http_get(pclient,page,"",&response,type);
		goto exit;
	}
	if (http_get(pclient, page, "", &response,REQ_DOWNLOAD)) {
		p_debug("GET !\n");
		ret =-1;
		goto  exit;
	}
	else{
		p_debug("get success");
		updateSysVal("net","true");
		updateVer(3);
		//printf("GET:%d:%s\n",strlen(response),response);
	}
	
exit:
	if((pclient->dn->total_size == pclient->dn->downloaded_size)&&(pclient->dn->total_size != 0))
		{

			char encrypt_file[256]={0};
			sprintf(encrypt_file,"encrypt %s/%s.mp4",LETV_DOWNLOAD_DIR_PATH,pclient->dn->vid);
			system(encrypt_file);
			sleep(5);
			pclient->dn->update_task_time=time(NULL);
			pclient->dn->percent=100;
			//pclient->dn->finish_task_time=time(NULL);
			updateSysVal("vid",pclient->dn->vid);
			updateSysVal("percent","0");			
			updateSysVal("progress","");
			updateSysVal("totalSize","");

			

			p_debug("VID=%s download percent=100",pclient->dn->vid);
//			display_task_dnode(task_dn);

			pclient->dn->download_status=DONE;
			update_follow_finished_time(pclient->dn);

			update_task_to_list(pclient->dn);

			updateVer(3);
	}
	
	safe_free(response);
	p_debug("111");
	http_tcpclient_close(pclient);
	p_debug("222");
	return ret;
}

int letv_client_parse_redirect_url(char *buff, char *re_url)
{
	char *p = NULL, *start = NULL, *recv_buff = NULL;
	int recvnum;
	p = strstr(buff, "302 Moved");
	if(p != NULL){
		p = strstr(p, "Location:");
		if(p == NULL){
			p = strstr(buff, "302 Moved");
			p = strstr(p, "location:");
			if(p == NULL)
			return -1;
		}
		
		p = strchr(p, ' ');//location: http://www.xxx.com
		if(p == NULL)
			return -1;
		p++;
		start = p;
		p_debug("start = %s", start);
		p = strstr(p, "\r\n");
		if(p == NULL)
			return -1;
		p_debug("end = %s", p);
		recvnum = (p-start);
		recv_buff = (char *)calloc(1, recvnum);
		if(recv_buff == NULL)
			return -1;
		memcpy(recv_buff, start, recvnum);
		memset(re_url,0,strlen(re_url));
		strcpy(re_url,recv_buff);
		p_debug("recvnum = %d, *lpbuff = %s", recvnum, re_url);
		safe_free(recv_buff);
		return recvnum;
	}
	else {
		p_debug("NO 302 reponse.");	
		if(strstr(buff, "424 GSLB")!=NULL)
			return -5;
		if(strstr(buff, "504 Gateway Time-out")!=NULL)
			return -5;

		return -1;
	}
}

int letv_client_get_redirect_url(http_tcpclient *pclient)
{
	p_debug("access letv_client_get_redirect_url");
	char	page[BUFFER_SIZE]="\0";
	int	n, port;
	char	*request = "get_redirect_url";
	char	*response=NULL;
	int     i = 0;
	char    url[BUFFER_SIZE]="\0";
	char    host[BUFFER_SIZE_1024]="\0";
	int 	time_out = 0;
	int ret=0;
    memset(host, 0, sizeof(host));
    memset(page, 0, sizeof(page));
	p_debug("page size=%d,url.len=%d,",sizeof(page),strlen(pclient->dn->vid_url));
    if ((n = letv_client_parse_url(pclient->dn->vid_url, host, &port, page)) < 0)
    {
		p_debug("parse url error");
		return -1;
	}
 
	time_out = HTTP_GET_INFO_TIME_OUT;
	if ((n = http_tcpclient_create(pclient, host, port,time_out)) < 0) {
		p_debug("Create socket error =%d.\n",n);
		return -1;
	}
	//p_debug("-----------------------------------------------");
	if ((n = http_tcpclient_conn(pclient)) < 0) {
		p_debug("Connet srv error.\n");
		//p_debug("-----------------------------------------------");

		ret = -3;
		goto EXIT;
	}
	if ((n = http_get_re_url(pclient, page, "", &response)) < 0) {
		p_debug("http_get_re_url error !\n");
		pclient->dn->download_status=RETRY_DOWNLOAD;
		ret=-4;
		goto EXIT;
	}
	else{
		p_debug("re_url response[%d]=%s\n",strlen(response),response);
	}

	if((ret=letv_client_parse_redirect_url(response, pclient->dn->vid_re_url)) <0 ){
		p_debug("PARSE ERROR!");
		//p_debug("ret=%d",ret);
		if(ret == -5)//424 GLSB response  追剧的链接失效
		{
			if(pclient->dn->isAutoAdd==1)
				if(flag_new_follow_add==0)flag_new_follow_add=1;//需要重新查询追剧地址
			else 
				memset(pclient->dn->vid_url,0,sizeof(pclient->dn->vid_url));//手动添加的需要更新URL地址
		}
		ret = -5;
		
		p_debug("pclient->dn->vid_url=%s",pclient->dn->vid_url);

		pclient->dn->download_status=RETRY_DOWNLOAD;
		goto EXIT;
	}
	else{
		p_debug("pclient->dn->vid_re_url = %s", pclient->dn->vid_re_url);
	}
EXIT:
	safe_free(response);
	//p_debug("pclient->dn->vid_re_url +++++++++= %s", pclient->dn->vid_re_url);
	http_tcpclient_close(pclient);
	return ret;
}


int letv_client_query_drama()
{
	
}

int letv_client_get_drama_info()
{
	
}

int letv_client_parse_version_info(char *p_buff, fw_version_info *p_version_info)
{
//p_debug("p_buff=%s",p_buff);
//p_debug("access letv_client_parse_version_info");
	int ret = 0;
	char state[16]="\0";
	JObj *r_json = JSON_PARSE(p_buff);
	if(r_json == NULL)
	{
		p_debug("access NULL");
		ret = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}

	JObj *state_json = JSON_GET_OBJECT(r_json,"state");
	if(state_json == NULL)
	{
		ret = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	memset(state, 0, 16);	
	strcpy(state, JSON_GET_OBJECT_VALUE(state_json,string));

	if(!strcmp(state, "error"))
	{ 
		JObj *msg_json = JSON_GET_OBJECT(r_json,"msg");
		if(msg_json == NULL)
		{
			ret = REQUEST_FORMAT_ERROR;
			goto EXIT;
		}
		strcpy(p_version_info->errorMsg, JSON_GET_OBJECT_VALUE(msg_json,string));
		ret = DM_ERROR_SERVER_EXCEPTION;
		goto EXIT;
	}
	else
	{ 
		JObj *data_json = JSON_GET_OBJECT(r_json,"data");
		if(data_json == NULL)
		{
			ret = REQUEST_FORMAT_ERROR;
			goto EXIT;
		}

		JObj *version_code_json = JSON_GET_OBJECT(data_json,"versionCode");
		if(version_code_json == NULL)
		{
			ret = REQUEST_FORMAT_ERROR;
			goto EXIT;
		}
		else
			strcpy(p_version_info->nextVersionCode, JSON_GET_OBJECT_VALUE(version_code_json,string));
	
		JObj *version_name_json = JSON_GET_OBJECT(data_json,"versionName");
		if(version_name_json == NULL)
		{
			ret = REQUEST_FORMAT_ERROR;
			goto EXIT;
		}
		else
			strcpy(p_version_info->nextVersionName, JSON_GET_OBJECT_VALUE(version_name_json,string));	

		
		JObj *update_url_json = JSON_GET_OBJECT(data_json,"updateUrl");
		if(update_url_json == NULL)
		{
			ret = REQUEST_FORMAT_ERROR;
			goto EXIT;
		}
		else
			strcpy(p_version_info->nextUpdateUrl, JSON_GET_OBJECT_VALUE(update_url_json,string));	

		
		JObj *is_force_json = JSON_GET_OBJECT(data_json,"isForce");
		if(is_force_json == NULL)
		{
			ret = REQUEST_FORMAT_ERROR;
			goto EXIT;
		}
		else{
			
			if(!strcmp(JSON_GET_OBJECT_VALUE(is_force_json,string),"1"))
				strcpy(p_version_info->isForce, "true");	
			else 
				strcpy(p_version_info->isForce, "false");
		}
		
		JObj *feature_json = JSON_GET_OBJECT(data_json,"feature");
		if(feature_json == NULL)
		{
			p_debug("no feature");
			//ret = REQUEST_FORMAT_ERROR;
			//goto EXIT;
		}
		else
			strcpy(p_version_info->nextFeature, JSON_GET_OBJECT_VALUE(feature_json,string));	

	}

EXIT:	
	return ret;
}

int letv_client_check_firmware(http_tcpclient *pclient, fw_version_info *p_version_info)
{
	int ret = 0;
	int	n = 0;
	int	host_port = 0;
	int time_out = 0;
	char page[BUFFER_SIZE]="\0";
	char host_ip[HOST_IP_LEN_256]="\0";
	char	*response = NULL;
	char	*request = "check firmware";
	letv_http_client letv_client;

	memset(&letv_client, 0, sizeof(letv_http_client));
	strcpy(letv_client.authorize_code, LETV_AUTHORIZECODE);
	strcpy(letv_client.dev_type, LETV_DEV_TYPE);
	strcpy(letv_client.mac_id, tmp_mac);
	//p_debug("tmp_mac=%s.",tmp_mac);
	if(letv_client_get_version(&letv_client) < 0)
	{
		p_debug("get mac id error");
		return -1;
	}
	else
	{
		//p_debug("version_code = %s", letv_client.version_code);
		//p_debug("version_name = %s", letv_client.version_name);
		//p_debug("sign_term = %s", letv_client.sign_term);
	}


	if(compose_page(&letv_client, &page, REQ_CHECK_FIRMWARE) < 0)
	{
		p_debug("compose page error");
		return -1;
	}

	memset(host_ip, 0, sizeof(host_ip));
	strcpy(host_ip, LETV_SERVER_IP);
	host_port = LETV_SERVER_PORT;
	time_out = HTTP_GET_INFO_TIME_OUT; 
//p_debug("\nhttp://%s:%d/%s\n",LETV_SERVER_IP,LETV_SERVER_PORT,page);
	if ((http_tcpclient_create(pclient, host_ip, host_port,time_out)) < 0) {
		p_debug("Create socket error.n = %d\n",n);
		return -1;
	}

	if ((http_tcpclient_conn(pclient)) < 0) {
		p_debug("Connet srv error.\n");
		ret = -1;
		goto EXIT;
	}

	if (http_post(pclient, page, request, &response)) {
		p_debug("POST !");
		goto EXIT;
	}
	else
		p_debug("POST:%d:%s\n",strlen(response),response);

	strcpy(p_version_info->curVersionCode, letv_client.version_code);
	strcpy(p_version_info->curVersionName, letv_client.version_name);
	if(letv_client_parse_version_info(response, p_version_info) < 0)
	{
		p_debug("parse version info error");
		goto EXIT;
	}

EXIT:	
	safe_free(response);
	http_tcpclient_close(pclient);
	return ret;	
}

int letv_client_get_firmware_redirect_url(http_tcpclient *pclient)
{
	int ret = 0;
	char	page[BUFFER_SIZE]="\0";
	int	n, port;
	char	*request = "get_firmware_redirect";
	char	*response=NULL;
	int     i = 0;
	char    url[BUFFER_SIZE]="\0";
	char    host[BUFFER_SIZE_1024]="\0";
	int 	time_out = 0;

    memset(host, 0, sizeof(host));
    memset(page, 0, sizeof(page));
    if ((n = letv_client_parse_url(pclient->fw_update_url, host, &port, page)) < 0)
    {
		p_debug("parse url error");
		return -1;
	}
 
	time_out = HTTP_GET_INFO_TIME_OUT;
	if ((n = http_tcpclient_create(pclient, host, port,time_out)) < 0) {
		p_debug("Create socket error.\n");
		return -1;
	}
	if ((n = http_tcpclient_conn(pclient)) < 0) {
		p_debug("Connet srv error.\n");
		ret = -1;
		goto EXIT;
	}
	if ((n = http_get_re_url(pclient, page, "", &response)) < 0) {
		p_debug("GET !\n");
		ret = -1;
		goto EXIT;
	}
	else{
		p_debug("GET:%d:%s\n",strlen(response),response);
	}

	if(ret=letv_client_parse_redirect_url(response, pclient->fw_update_re_url) <0 ){
		p_debug("PARSE ERROR!");
		if(ret == -3)//424 GLSB response
			memset(pclient->fw_update_url,0,sizeof(pclient->fw_update_url));
		//ret = -1;
		goto EXIT;
	}
	else{
		p_debug("pclient->dn->fw_update_re_url = %s", pclient->fw_update_re_url);
	}
EXIT:
	safe_free(response);
	http_tcpclient_close(pclient);
	return 0;
}


int letv_client_firmware_download(http_tcpclient *pclient)
{
	char	page[BUFFER_SIZE]="\0";
	int	n, port;
	char	*request = "firmware download";
	char	*response=NULL;
	int     i = 0;
	char    url[BUFFER_SIZE]="\0";
	char    host[BUFFER_SIZE_1024]="\0";
	int 	time_out = 0;
	int 	ret=0;
    memset(host, 0, sizeof(host));
    memset(page, 0, sizeof(page));
    if ((n = letv_client_parse_url(pclient->fw_update_re_url, host, &port, page)) < 0)
    {
		p_debug("parse url error");
		return -1;
	}
 
	time_out = HTTP_DOWNLOAD_TIME_OUT;
	if ((n = http_tcpclient_create(pclient, host, port,time_out)) < 0) {
		p_debug("Create socket error.\n");
		return -1;
	}
	
	if ((n = http_tcpclient_conn(pclient)) < 0) {
		p_debug("Connet srv error.\n");
		ret = -1;
		goto exit;
	}
	updateSysVal("net","true");
	if (http_get(pclient, page, "", &response,FIRMWARE)<0) {
		p_debug("GET !\n");
		ret = -1;
		goto exit;
	}
	else{
		p_debug("free response.1");

	}
exit:
	safe_free(response);
	http_tcpclient_close(pclient);
	p_debug("leave letv client firmware download");
	return ret;
}


int letv_client_report_status(http_tcpclient *pclient)
{
	char page[BUFFER_SIZE]="\0";
	int	host_port;
	int ret = 0;
	int	n = 0;
	char	*response = NULL;
	char	*request = "report status";//"<input name='filepath' type='file' />";
	char host_ip[HOST_IP_LEN_256]="\0";
	int time_out = 0;
	letv_http_client letv_client;

	memset(&letv_client, 0, sizeof(letv_http_client));
	strcpy(letv_client.authorize_code, LETV_AUTHORIZECODE);
	strcpy(letv_client.dev_type, LETV_DEV_TYPE);
	/* for example */
	strcpy(letv_client.ip, "10.10.10.254");

	if(letv_client_get_mac_id(&letv_client) < 0){
		p_debug("get mac id error");
		return -1;
	}
	else{
		p_debug("letv_client.mac_id = %s", letv_client.mac_id);
	}

	if(letv_client_compose_params(&letv_client) < 0){
		p_debug("compose params error");
		return -1;
	}
	else{
		p_debug("letv_client.params = %s", letv_client.params);
	}

	if(compose_page(&letv_client, &page, REQ_REPORT_STATUS) < 0)
	{
		p_debug("compose page error");
		return -1;
	}

	memset(host_ip, 0, sizeof(host_ip));
	strcpy(host_ip, LETV_SERVER_IP);
	host_port = LETV_SERVER_PORT;
	time_out = HTTP_REPORT_TIME_OUT; 

	if ((n = http_tcpclient_create(pclient, host_ip, host_port,time_out)) < 0) {
		p_debug("Create socket error.n = %d\n",n);
		return -1;
	}

	if ((n = http_tcpclient_conn(pclient)) < 0) {
		p_debug("Connet srv error.\n");
		ret = -1;
		goto EXIT;
	}

	if (http_post(pclient, page, request, &response)<0) {
		p_debug("report status post error!");
		ret = -1;
		goto EXIT;
	}
	else
		p_debug("report status post:%d:%s\n",strlen(response),response);


EXIT:	
	safe_free(response);
	http_tcpclient_close(pclient);
	return ret;	
}



