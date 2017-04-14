/************************************************************************
#
#  Copyright (c) 2014-2016  DAMAI(SHENTHEN) Co., Ltd.
#  All Rights Reserved
#
#  author: Oliver
#  create date: 2015-8-20
# 
# Unless you and longsys execute a separate written software license 
# agreement governing use of this software, this software is licensed 
# to you under the terms of the GNU General Public License version 2 
# (the "GPL"), with the following added to such license:
# 
#    As a special exception, the copyright holders of this software give 
#    you permission to link this software with independent modules, and 
#    to copy and distribute the resulting executable under terms of your 
#    choice, provided that you also meet, for each linked independent 
#    module, the terms and conditions of the license of that module. 
#    An independent module is a module which is not derived from this
#    software.  The special exception does not apply to any modifications 
#    of the software.  
# 
# Not withstanding the above, under no circumstances may you combine 
# this software in any way with any other longsys software provided 
# under a license other than the GPL, without longsys's express prior 
# written consent. 
#
#
*************************************************************************/


/*############################## Includes ####################################*/
#include "config.h"
#include "defs.h"
#include "base.h"
#include "router_task.h"
#include "../http_client/http_client.h"
#include "../http_client/http_letv.h"
#include "uci_for_cgi.h"
#include <pthread.h>
#include <io.h>

/*############################## Global Variable #############################*/


int	exit_flag;
	
int status_downloading=0;
int downloading_vid=0;
int is_query_album=0;
int backup_time=0;
static void
signal_handler(int sig_num)
{
	switch (sig_num) {
#ifndef _WIN32
	case SIGCHLD:
		while (waitpid(-1, &sig_num, WNOHANG) > 0) ;
		break;
#endif /* !_WIN32 */
	case SIGTERM:
		p_debug("SIGTERM:get killed");
		break;
	default:
		p_debug("exit_flag = %d",sig_num);
		exit_flag = sig_num;
		break;
	}
}
void handle_query_album_task(void *arg){//查询追剧
	p_debug("access handle_query_album_task");
	int i;
	int j=0;
	http_tcpclient	t_client;
	char tmp_path[256];
	int ret = 0;
	memset(&t_client, '\0', sizeof(http_tcpclient));
	struct album_node *album;
	album=album_dn;
	is_query_album=1;


/*	t_client.dn = ;
	struct task_dnode *pdn = (struct task_dnode *)arg;
	t_client.dn=pdn;
	t_client.dn->download_status=DOWNLOADING;
*/	/* get request url info */
//	t_client.dn->pid = "";
	//strcpy(t_client.album_id,"10007684");
//	while(1){
		//cur=album_dn;
		for(i=0;;i++){
			if(!album){
				break;
			}else if(album->status != REMOVE){
				j++;
				t_client.adn=album;
				//strcpy(t_client.adn->pid,album->pid);

				if(!strcmp(t_client.adn->album_img_path,"")){
					//if(0){

					p_debug("download pid coverImg=%s",album->pid);
					memset(tmp_path, 0, 256);
					sprintf(tmp_path, "/tmp/mnt/USB-disk-1/hack/%s.jpg",t_client.adn->pid);
					strcpy(t_client.tmp_path,tmp_path);

					/* get request url info */
					ret = letv_client_get_source(&t_client,REQ_QUERY_ALBUM_INFO);
					if(ret < 0)
					{
						p_debug("get source error");
						//goto exit;
					}
					
					ret=letv_client_image_download(&t_client,ALBUM_IMAGE);

					if(ret>=0)sprintf(t_client.adn->album_img_path, "/USB-disk-1/hack/%s.jpg",t_client.adn->pid);
					else sprintf(t_client.adn->album_img_path, "%s","");
				}

				ret = letv_client_get_source(&t_client,REQ_QUERY_ALBUM);
				if(ret < 0)
				{
					p_debug("get album source error");//query next
					//safe_free(task_cur);// memory leak
					//goto exit;
				}
				
			}
			p_debug("status===%d",album->status);
			album=album->dn_next;
			sleep(1);
		}
		if(j>0){
			updateSysVal("isFollow","true");
		}else{
			updateSysVal("isFollow","false");
		}
		is_query_album=0;
		flag_new_follow_add=0;
//	sleep(60);
//	}
exit:
	p_debug("exit handle_query_album_task task");
	return ;

}
void handle_download_task(void *arg)
{
	p_debug("access handle_download_task");
//	return ;
	int i;
	http_tcpclient	t_client;
	char tmp_path[256];
	int ret = 0;
	int fd;
	memset(&t_client, '\0', sizeof(http_tcpclient));
//	t_client.dn = ;
	struct task_dnode *pdn = (struct task_dnode *)arg;
	t_client.dn=pdn;
	

	/* byte-range if downloaded_size > 0 */
	p_debug("dn[%s].vid_url=(%s),isAutoAdd=%d",t_client.dn->vid,t_client.dn->vid_url,t_client.dn->isAutoAdd);
	if((!strcmp(t_client.dn->vid_url,""))&&(t_client.dn->isAutoAdd==1)){//追剧下载链接失效
		if(flag_new_follow_add==0)	flag_new_follow_add=1;//需要重新查询追剧地址
		goto exit;
	}
	if((t_client.dn->total_img_size==0)||(t_client.dn->download_img_size < t_client.dn->total_img_size)||(t_client.dn->downloaded_size == 0||(!strcmp(t_client.dn->vid_url,""))||(t_client.dn->download_status==RETRY_DOWNLOAD))&&(t_client.dn->isAutoAdd==0))
	{//手动添加的视频，从这里获得视频地址

		ret = letv_client_get_source(&t_client,REQ_DOWNLOAD);

		if(ret < 0)
		{
			p_debug("get source error");
			goto exit;
		}

		/* download image file */
		memset(tmp_path, 0, 256);
		sprintf(tmp_path, "/tmp/mnt/USB-disk-1/hack/%s.jpg",t_client.dn->vid);
		fd=access(tmp_path,R_OK);
		if(fd != 0)//not exsit,then download
		{
			strcpy(t_client.tmp_path,tmp_path);
			p_debug(t_client.tmp_path);
		/*
			char *p_img_path = (char *)calloc(1, strlen(tmp_path)+1);
			if(p_img_path == NULL)
			{
				ret = DM_ERROR_ALLOCATE_MEM;
				goto exit;
			}	
			strcpy(p_img_path, tmp_path); 	
			*/
			//strcpy(t_client.dn->img_path,tmp_path);
			sprintf(t_client.dn->img_path,"/tmp/mnt/USB-disk-1/hack/%s.jpg",t_client.dn->vid);
			p_debug(t_client.dn->img_path);
			ret = letv_client_image_download(&t_client,VIDEO_IMAGE);
			if(ret < 0)
			{
				p_debug("download image file error");
				//goto exit;

			}
			else 
			{


			struct album_node AlbumNode;
			memset(&AlbumNode, '\0', sizeof(AlbumNode));
			strcpy(AlbumNode.pid,t_client.dn->pid);
			t_client.adn=&AlbumNode;
			
			sprintf(tmp_path, "/tmp/mnt/USB-disk-1/hack/%s.jpg",t_client.dn->pid);
			strcpy(t_client.tmp_path,tmp_path);

			/* get request url info */
			ret = letv_client_get_source(&t_client,REQ_QUERY_ALBUM_INFO);
			if(ret < 0)
			{
				p_debug("get source error");
				goto exit;
			}
			
			ret=letv_client_image_download(&t_client,ALBUM_IMAGE);

			#if 0
			p_debug("t_client.dn->pid=%s",t_client.dn->pid);
				if(IsInt(t_client.dn->pid)){
					memset(tmp_path, 0, 256);
					sprintf(tmp_path, "/tmp/mnt/USB-disk-1/hack/%s.jpg",t_client.dn->pid);
					fd=access(tmp_path,R_OK);

					if(fd !=0 ){
						/*
						sprintf(t_client.dn->img_path,"/tmp/mnt/USB-disk-1/hack/%s.jpg",t_client.dn->pid);
						p_debug(t_client.dn->img_path);
						ret = letv_client_image_download(&t_client,REQ_DOWNLOAD);
						if(ret < 0)
						{
							p_debug("download image file error");
							//goto exit;

						}
						*/
						char cmd[256]={0};
						sprintf(cmd,"cp /tmp/mnt/USB-disk-1/hack/%s.jpg /tmp/mnt/USB-disk-1/hack/%s.jpg",t_client.dn->vid,t_client.dn->pid);
						p_debug(cmd);
						system(cmd);
					}
				}
				#endif
			}
		}


	}
	/* download video file and save download list*/
	ret = letv_client_get_redirect_url(&t_client);
	if(ret < 0)
	{
		p_debug("get redirect_url error");
		goto exit;

	}
	//p_debug("rerrrrrrrrrr=%s",t_client.dn->vid_re_url);
	//p_debug("uuuuuuuuuuuuuuuuuuuu=%s",t_client.dn->vid_url);
	memset(tmp_path, 0, 256);
	sprintf(tmp_path, "/tmp/mnt/USB-disk-1/hack/%s.mp4",t_client.dn->vid);

	strcpy(t_client.tmp_path,tmp_path);

	sprintf(t_client.dn->vid_path,"/tmp/mnt/USB-disk-1/hack/%s.mp4",t_client.dn->vid);
	if(t_client.dn->download_status != PAUSE)
		t_client.dn->download_status=DOWNLOADING;
	ret = letv_client_video_download(&t_client,REQ_DOWNLOAD);
	if(ret < 0)
	{
		p_debug("download video file error");
		goto exit;

	}else{
/*		(*pdn)->download_status=DONE;//finished downloading
		*pdn=(*pdn)->dn_next;
		for(i=0;;i++){//start the next vid  downloading
			if(!*pdn)//tasklist is empty
				break;
			if((*pdn)->download_status==WAITING)
				{
				(*pdn)->download_status=DOWNLOADING;
				break;
			}
			*pdn = (*pdn)->dn_next;
		}
*/		p_debug("Download Video ID %s Success.",t_client.dn->vid);
	}
exit:
	status_downloading=0;//if dowload error, then download next.
	p_debug("exit download task");
	return ;
}


int update_upgrade_info_file(int new_flag, fw_version_info *p_version_info)
{
	char cmd[512];

	if(new_flag==1)
	{		
		memset(cmd, 0, 512);
		sprintf(cmd,"sed -e \'/^%s=.*/d\' -i %s", INFO_NEXT_VERSION_CODE, FILE_UPGRADE_INFO_PATH);
		system(cmd);
		memset(cmd, 0, 512);
		sprintf(cmd,"echo \'%s=%s\' >> %s", INFO_NEXT_VERSION_CODE, p_version_info->nextVersionCode, FILE_UPGRADE_INFO_PATH);
		system(cmd);

		memset(cmd, 0, 512);
		sprintf(cmd,"sed -e \'/^%s=.*/d\' -i %s", INFO_NEXT_VERSION_NAME, FILE_UPGRADE_INFO_PATH);
		system(cmd);
		memset(cmd, 0, 512);
		sprintf(cmd,"echo \'%s=%s\' >> %s", INFO_NEXT_VERSION_NAME, p_version_info->nextVersionName, FILE_UPGRADE_INFO_PATH);
		system(cmd);

		memset(cmd, 0, 512);
		sprintf(cmd,"sed -e \'/^%s=.*/d\' -i %s", INFO_NEXT_FEATURE, FILE_UPGRADE_INFO_PATH);
		system(cmd);
		memset(cmd, 0, 512);
		sprintf(cmd,"echo \'%s=%s\' >> %s", INFO_NEXT_FEATURE, p_version_info->nextFeature, FILE_UPGRADE_INFO_PATH);
		system(cmd);

		memset(cmd, 0, 512);
		sprintf(cmd,"sed -e \'/^%s=.*/d\' -i %s", INFO_IS_FORCE, FILE_UPGRADE_INFO_PATH);
		system(cmd);
		memset(cmd, 0, 512);
		sprintf(cmd,"echo \'%s=%s\' >> %s", INFO_IS_FORCE, p_version_info->isForce, FILE_UPGRADE_INFO_PATH);
		system(cmd);

		memset(cmd, 0, 512);
		sprintf(cmd,"sed -e \'/^%s=.*/d\' -i %s", INFO_HAS_NEW, FILE_UPGRADE_INFO_PATH);
		system(cmd);
		memset(cmd, 0, 512);
		sprintf(cmd,"echo \'%s=%s\' >> %s", INFO_HAS_NEW, "true", FILE_UPGRADE_INFO_PATH);
		system(cmd);
	}
	else if(new_flag==2){
		memset(cmd, 0, 512);
		sprintf(cmd,"sed -e \'/^%s=.*/d\' -i %s", INFO_NEXT_VERSION_CODE, FILE_UPGRADE_INFO_PATH);
		system(cmd);
		memset(cmd, 0, 512);
		sprintf(cmd,"echo \'%s=%s\' >> %s", INFO_NEXT_VERSION_CODE, p_version_info->nextVersionCode, FILE_UPGRADE_INFO_PATH);
		system(cmd);

		memset(cmd, 0, 512);
		sprintf(cmd,"sed -e \'/^%s=.*/d\' -i %s", INFO_NEXT_VERSION_NAME, FILE_UPGRADE_INFO_PATH);
		system(cmd);
		memset(cmd, 0, 512);
		sprintf(cmd,"echo \'%s=%s\' >> %s", INFO_NEXT_VERSION_NAME, p_version_info->nextVersionName, FILE_UPGRADE_INFO_PATH);
		system(cmd);

		memset(cmd, 0, 512);
		sprintf(cmd,"sed -e \'/^%s=.*/d\' -i %s", INFO_NEXT_FEATURE, FILE_UPGRADE_INFO_PATH);
		system(cmd);
		memset(cmd, 0, 512);
		sprintf(cmd,"echo \'%s=%s\' >> %s", INFO_NEXT_FEATURE, p_version_info->nextFeature, FILE_UPGRADE_INFO_PATH);
		system(cmd);

		memset(cmd, 0, 512);
		sprintf(cmd,"sed -e \'/^%s=.*/d\' -i %s", INFO_IS_FORCE, FILE_UPGRADE_INFO_PATH);
		system(cmd);
		memset(cmd, 0, 512);
		sprintf(cmd,"echo \'%s=%s\' >> %s", INFO_IS_FORCE, p_version_info->isForce, FILE_UPGRADE_INFO_PATH);
		system(cmd);
		
		memset(cmd, 0, 512);
		sprintf(cmd,"sed -e \'/^%s=.*/d\' -i %s", INFO_HAS_NEW, FILE_UPGRADE_INFO_PATH);
		system(cmd);
		memset(cmd, 0, 512);
		sprintf(cmd,"echo \'%s=%s\' >> %s", INFO_HAS_NEW, "false", FILE_UPGRADE_INFO_PATH);
		system(cmd);


	}
	else if(new_flag==0)
	{
		memset(cmd, 0, 512);
		sprintf(cmd,"sed -e \'/^%s=.*/d\' -i %s", INFO_HAS_NEW, FILE_UPGRADE_INFO_PATH);
		system(cmd);
		memset(cmd, 0, 512);
		sprintf(cmd,"echo \'%s=%s\' >> %s", INFO_HAS_NEW, "false", FILE_UPGRADE_INFO_PATH);
		system(cmd);
	}

	return 0;
}

#define FW_LENGTH 16384000

//0:no download;1:have download
int check_upgrade_info_file(fw_version_info *p_version_info)
{
	int ret = 0;
	char has_new[16];
	char version_code[16];
	int has_fw_file= 0;
	memset(&has_new, 0, 16);
	if(access(DIR_UPGRADE_PATH, F_OK) == 0){

	}
	else{
		mkdir(DIR_UPGRADE_PATH, 0777);
	}
		
	ret = get_conf_str(INFO_HAS_NEW, has_new, FILE_UPGRADE_INFO_PATH);
	if(ret == 0)
	{
		p_debug("has_new = %s", has_new);
		if(strcmp(has_new, "true"))
		{
			if(access(FILE_UPGRADE_PATH, F_OK) == 0){
				if(remove(FILE_UPGRADE_PATH) ==0)
					p_debug("remove upgrade file success");
			}
			return 0;
		}
	}	

	memset(&version_code, 0, 16);
	ret = get_conf_str(INFO_NEXT_VERSION_CODE, version_code, FILE_UPGRADE_INFO_PATH);
	if(ret == 0)
	{
		
		p_debug("version_code = %s, nextVersionCode = %s", version_code, p_version_info->nextVersionCode);
		if(atoi(p_version_info->nextVersionCode) > atoi(version_code))//如果有更新的固件，先删除已下载的，再下载最新固件
		{
			
			if(access(FILE_UPGRADE_PATH, F_OK) == 0){
				if(remove(FILE_UPGRADE_PATH) ==0)
					p_debug("remove upgrade file success");
			}
			
			update_upgrade_info_file(0, p_version_info);// 如果有更新的，先设置为未有更新，下载后再更新为更新版本。
			return 0;
		}
		if(atoi(p_version_info->nextVersionCode) == atoi(version_code)){//如果是正在下载的版本，则断点续传
			int fd=open(FILE_UPGRADE_PATH,O_RDONLY,0644);
			if(fd>=0){
				long long fsize=lseek(fd,0,SEEK_END);
				close(fd);
				if(fsize==FW_LENGTH) //already finished download.
					return 1;
				else
					return 0;
			}else 
				return 0;
		}
	}	

	has_fw_file = access(FILE_UPGRADE_PATH, 0);
	if(has_fw_file == -1)
	{
		p_debug("no file");
		return 0;
	}

	return 1;
}



void handle_check_firmware_task()
{
	p_debug("handle_check_firmware_task");
	int ret = 0;
	int has_dl = 0;
	http_tcpclient	t_client;
	fw_version_info	version_info;
	char tmp_path[256];

	memset(&version_info, 0, sizeof(fw_version_info));
	memset(&t_client, '\0', sizeof(http_tcpclient));
	while(ret = letv_client_check_firmware(&t_client, &version_info)<0)
	{
		p_debug("wait for check firmware.");
		sleep(5);
	}

	p_debug("check firmware success");
	

	if(atoi(version_info.nextVersionCode) > atoi(version_info.curVersionCode)){
		p_debug("have new version url = %s", version_info.nextUpdateUrl);
		has_dl = check_upgrade_info_file(&version_info);
		if(has_dl == 0){
			p_debug("have  new fw to download");
			if(strlen(version_info.nextUpdateUrl) > 0)
			{
				strcpy(t_client.fw_update_url, version_info.nextUpdateUrl);	
				ret = letv_client_get_firmware_redirect_url(&t_client);
				if(ret < 0){
					p_debug("get firmware redirect url error");
					return ;
				}
				else{
					p_debug("get firmware redirect url success");
				}							
				strcpy(tmp_path, FILE_UPGRADE_PATH);
				strcpy(t_client.tmp_path,tmp_path);
				ret = letv_client_firmware_download(&t_client);
				if(ret < 0){
					p_debug("download firmware error");
					update_upgrade_info_file(2, &version_info);
				}
				else{
					p_debug("download firmware success");
					update_upgrade_info_file(1, &version_info);
					//updateSysVal("net","true");
				}
			}
			else
			{
				p_debug("url is null");
			}
		}
		else{
			p_debug("have download fw");
		}
	}
	else{
		p_debug("have no new version");
		update_upgrade_info_file(0, &version_info);
	}

exit:
	p_debug("exit check firmware task");
	return ;
}



void handle_report_status_task()
{
#if 1
	p_debug("access handle_reported_task");
	int ret = 0;
	http_tcpclient	t_client;

	if(report_status_err_time>=3){// 5 minutes reconnect
		
		p_debug("internet connection is not good, can't connect to the letv server for %ds.",report_status_err_time*60);

		//updateSysVal("net","false");
		//system("wifi_connect &");//rescan and connect
	}

	memset(&t_client, 0, sizeof(http_tcpclient));
	ret = letv_client_report_status(&t_client);
	if(ret < 0)
	{
		p_debug("report status error");
		report_status_err_time++;
		goto exit;

	}else{

		//updateSysVal("net","true");
		report_status_err_time=0;
		
		p_debug("report status success");
	}

exit:
	p_debug("exit report status task");
	return ;
#endif
}


int LETV_album_process()
{
	PTHREAD_T pt_album_id;
	//pthread_rwlock_trywrdlock(&task_dn); 
	//while(1)
		{
		if(PTHREAD_CREATE(&pt_album_id,NULL, (void *)handle_query_album_task,NULL) != 0)
	    {
	        p_debug("pthread_create error");
			return -1;
	    }
		PTHREAD_DETACH(pt_album_id);
		sleep(5);
	}
	return 0;
}

int download_cover(){
	p_debug("access download_cover");
	//struct task_dnode *dn = task_dn;
	http_tcpclient	t_client;
	//p_debug("sizeof(long long ): %d\n", (int) sizeof(long long));
	char tmp_path[256]={0};
	int ret = 0;
	int fd;
	int i,j;
	char net[8]={0};
	while(1){
		sleep(1);
		j++;
		if(j>=30 || (download_img_first==1))
		{
			j=0;
			get_conf_str("net",net,"/tmp/state/status");
			if(strcmp(net,"true")){
				sleep(2);
				continue;
			}
			struct task_dnode *dn = task_dn;
			for(i = 0;;i++)
				{
					if(!dn)//tasklist is empty
						break;

					if(dn->isDeleted==1||(dn->download_status==PAUSE))//task is deleted.
					{
						dn=dn->dn_next;
						continue;
					}

					p_debug("total=%lld,dl=%lld,t=%lld",dn->total_img_size,dn->download_img_size,dn->total_size);
					if((dn->total_img_size==0)||(dn->download_img_size<dn->total_img_size))//not downloaded 
					{

						memset(&t_client, '\0', sizeof(http_tcpclient));
						//struct task_dnode *pdn = task_dn;
						t_client.dn=dn;
						/* download image file */
						memset(tmp_path, 0, 256);
						sprintf(tmp_path, "/tmp/mnt/USB-disk-1/hack/%s.jpg",t_client.dn->vid);
						/*fd=access(tmp_path,R_OK);
						if(fd==0){
							FILE *fp=fopen(fp,O_CREAT|O_WRONLY,0644);
							long long fsize= lseek(fp, 0, SEEK_END);
							
							lseek(pclient->local_fd,fsize,SEEK_SET);

						}*/
						//if(fd != 0)//not exsit,then download
						//{
							strcpy(t_client.tmp_path,tmp_path);
							p_debug(t_client.tmp_path);
				
							//strcpy(t_client.dn->img_path,tmp_path);
							sprintf(t_client.dn->img_path,"/tmp/mnt/USB-disk-1/hack/%s.jpg",t_client.dn->vid);
							p_debug(t_client.dn->img_path);
							download_img_first=1;
							if(!strcmp(dn->img_url,""))//
							{
								ret = letv_client_get_source(&t_client,VIDEO_IMAGE);
								if(ret < 0)
								{
									p_debug("get source error");
									continue;
									//goto exit;
								}
							}
							
							ret = letv_client_image_download(&t_client,VIDEO_IMAGE);
							if(ret < 0)
							{
								p_debug("download video image file error");
							}
									
							p_debug("ffffffffffffff=%lld",dn->total_size);
							if(0){//如果为空要先查大小
//							if(dn->total_size==0){//如果为空要先查大小
								if(!strcmp(dn->vid_url,""))
									ret = letv_client_get_source(&t_client,REQ_DOWNLOAD);

								if(!strcmp(dn->vid_re_url,""))// 为空
								 letv_client_get_redirect_url(&t_client);
							
									letv_client_video_download(&t_client,GET_VIDEO_SIZE);
							}
							
											
						}
						
						memset(&t_client, '\0', sizeof(http_tcpclient));
						//struct task_dnode *pdn = task_dn;
						t_client.dn=dn;
						sprintf(tmp_path, "/tmp/mnt/USB-disk-1/hack/%s.jpg",t_client.dn->pid);
						fd=access(tmp_path,R_OK);
						if(fd != 0)//not exsit,then download
						{
							struct album_node AlbumNode;
							memset(&AlbumNode, '\0', sizeof(AlbumNode));
							strcpy(AlbumNode.pid,t_client.dn->pid);
							t_client.adn=&AlbumNode;
							strcpy(t_client.tmp_path,tmp_path);
							if(!strcmp(AlbumNode.coverImgUrl,"")){
								ret = letv_client_get_source(&t_client,REQ_QUERY_ALBUM_INFO);
								if(ret < 0)
								{
									p_debug("get album source error");
									dn=dn->dn_next;
									continue;
									//goto exit;
								}
							}
							/* get request url info */
							p_debug("t_client.tmp_path=%s",t_client.tmp_path);
									
							ret=letv_client_image_download(&t_client,ALBUM_IMAGE);
							if(ret < 0)
							{
								p_debug("download album image file error");
								//goto exit;
							}
						}	
					
					dn=dn->dn_next;
				}
		download_img_first=0;

		//sleep(30);
		}

	}
exit:
	p_debug("exit download_cover");
	return;
}
char retry_vid[VID_LEN]={0};

int LETV_task_process()
{
	int ret=0;
	int i=0;
	int report_status_time = 60;//1s*10
	int query_album_time = 3530;//delay 70 seconds
	char net[8]={0};
	p_debug("access LETV_task_process");
	PTHREAD_T download_ptid;
	PTHREAD_T report_ptid;
	PTHREAD_T query_ptid;
	struct task_dnode *dn = task_dn;

	//query_album_time=0;
	char uci_option_str[64]="\0";
	char wifimode[8]="\0";
	//char retry_vid[VID_LEN]={0};
	int retry_times=0;
//	while(strcmp(net,"true")||){
	//	get_conf_str("net",net,"/tmp/state/status");
	//	sleep(1);
	//}

	while(1){
		ctx=uci_alloc_context();
		memset(uci_option_str,'\0',64); 
		strcpy(uci_option_str,"system.@system[0].wifimode"); 	
		uci_get_option_value(uci_option_str,wifimode);

		get_conf_str("net",net,"/tmp/state/status");
		
		if(ctx)uci_free_context(ctx);
		if(!strcmp(wifimode,"sta")||(strcmp(net,"true")))
			{
			sleep(2);
			if(!strcmp(wifimode,"sta"))updateSysVal("net","false");
			continue;
		}
		
		#if 1
		dn = task_dn;
		//pthread_rwlock_trywrlock(&task_dn);
		if(status_downloading==0&&(download_img_first==0)){
			for(i = 0;;i++)
			{
				if(!dn)//tasklist is empty
					break;
				//p_debug("dn[%d]->pid[%s],vid[%s],status=%d,loaded=%lld,total=%lld",i,(dn)->pid,(dn)->vid,(dn)->download_status,dn->downloaded_size,dn->total_size);
					if(((dn)->download_status==DOWNLOADING))
					{

						if(!strcmp(retry_vid,(dn)->vid)) retry_times++;
						if(retry_times==10) {
							retry_times=0;
							(dn)->download_status=DOWNLOAD_URL_ERROR;
							break;
						}
						
						status_downloading=1;	
						updateSysVal("vid",dn->vid);
						char pr[8]="\0";
						sprintf(pr,"%.0f",dn->percent);
						updateSysVal("percent",pr);
						char progress[32]="\0";
						char totalSize[32]="\0";
						sprintf(progress,"%lld",dn->downloaded_size);
						sprintf(totalSize,"%lld",dn->total_size);						
						updateSysVal("totalSize",totalSize);
						updateSysVal("progress",progress);
						updateSysVal("speed","");
						
						
						get_conf_str("net",net,"/tmp/state/status");
						if(strcmp(net,"true")==0){//wait for internet..
							//updateSysVal("isDownloading","true");
							//system("mcu_control -s 4");
							//updateSysVal("led_status","4");
						}						
						downloading_vid=1;
						if(dn->download_status == PAUSE)
							{
							status_downloading=0;
							downloading_vid=0;
							updateSysVal("vid","0");
							updateSysVal("percent","0");
							updateSysVal("isDownloading","false");
							//system("mcu_control -s 1");
							system("pwm_control 1 1 0;pwm_control 1 0 0");
							updateSysVal("led_status","1");
							break;
						}
						strcpy(retry_vid,dn->vid);
						if(PTHREAD_CREATE(&download_ptid,NULL, (void *)handle_download_task,dn) != 0)
						{
							//safe_free(task_dn);	 //have problem
							p_debug("download task pthread create error");
							updateSysVal("vid","");
							updateSysVal("percent","");
							updateSysVal("isDownloading","false");
//							system("mcu_control -s 1");
							system("pwm_control 1 1 0;pwm_control 1 0 0");
							updateSysVal("led_status","1");

							status_downloading=0;
							downloading_vid=0;
							return -1;
						}
						PTHREAD_DETACH(download_ptid);

						break;

					}
				dn = (dn)->dn_next;
			}
		}
		if(status_downloading == 0&&(download_img_first==0))
		{
			dn = task_dn;
			for(i = 0;;i++)
			{
				if(!dn)//tasklist is empty
					break;
				//p_debug("dn[%d]->pid[%s],vid[%s],status=%d,loaded=%lld,total=%lld",i,(dn)->pid,(dn)->vid,(dn)->download_status,dn->downloaded_size,dn->total_size);
	/**/			if(((dn)->download_status==DOWNLOADING||(dn)->download_status==WAITING))//find the one that is waiting or a new downloading
					{

						p_debug("find the one tasklist[%d] pid[%s] vid[%s] that is waiting",i,(dn)->pid,(dn)->vid);

						if(!strcmp(retry_vid,(dn)->vid)) retry_times++;
						if(retry_times==10) {
							retry_times=0;
							(dn)->download_status=DOWNLOAD_URL_ERROR;
							break;
						}
						status_downloading=1;
						updateSysVal("vid",dn->vid);
						char pr[8]="\0";
						sprintf(pr,"%.0f",dn->percent);
						updateSysVal("percent",pr);
						char progress[32]="\0";
						char totalSize[32]="\0";
						sprintf(progress,"%lld",dn->downloaded_size);
						sprintf(totalSize,"%lld",dn->total_size);						
						updateSysVal("totalSize",totalSize);
						updateSysVal("progress",progress);
						updateSysVal("speed","0");
						
						get_conf_str("net",net,"/tmp/state/status");
						if(strcmp(net,"true")==0){//wait for internet..
							//updateSysVal("isDownloading","true");
							//system("mcu_control -s 4");
							//updateSysVal("led_status","4");
						}	


						downloading_vid=1;
						if(dn->download_status == PAUSE)
							{
							status_downloading=0;
							downloading_vid=0;
							updateSysVal("vid","0");
							updateSysVal("percent","0");
							updateSysVal("isDownloading","false");
//							system("mcu_control -s 1");
							system("pwm_control 1 1 0;pwm_control 1 0 0");
							updateSysVal("led_status","1");							
							break;
						}
						strcpy(retry_vid,dn->vid);
					    if(PTHREAD_CREATE(&download_ptid,NULL, (void *)handle_download_task,dn) != 0)
					    {
					        //safe_free(task_dn);    //have problem
					        p_debug("download task pthread create error");
							updateSysVal("vid","0");
							updateSysVal("percent","0");
							updateSysVal("isDownloading","false");
//							system("mcu_control -s 1");
							system("pwm_control 1 1 0;pwm_control 1 0 0");
							updateSysVal("led_status","1");

					        status_downloading=0;
							downloading_vid=0;
							return -1;
					    }
						PTHREAD_DETACH(download_ptid);
					break;
				}
				dn = (dn)->dn_next;
			}	

		}
		if(status_downloading == 0&&(download_img_first==0))
		{
			dn = task_dn;
			for(i = 0;;i++)
			{
				if(!dn)//tasklist is empty
					break;
				//p_debug("dn[%d]->pid[%s],vid[%s],status=%d,loaded=%lld,total=%lld",i,(dn)->pid,(dn)->vid,(dn)->download_status,dn->downloaded_size,dn->total_size);
	/**/			if(((dn)->download_status == RETRY_DOWNLOAD))//
					{

						p_debug("find the one tasklist[%d] pid[%s] vid[%s] that need retry",i,(dn)->pid,(dn)->vid);

						if(!strcmp(retry_vid,(dn)->vid)) retry_times++;
						if(retry_times==10) {
							retry_times=0;
							(dn)->download_status=DOWNLOAD_URL_ERROR;
							break;
						}
						status_downloading=1;
						updateSysVal("vid",dn->vid);
						char pr[8]="\0";
						sprintf(pr,"%.0f",dn->percent);
						updateSysVal("percent",pr);
						char progress[32]="\0";
						char totalSize[32]="\0";
						sprintf(progress,"%lld",dn->downloaded_size);
						sprintf(totalSize,"%lld",dn->total_size);						
						updateSysVal("totalSize",totalSize);
						updateSysVal("progress",progress);
						updateSysVal("speed","");
						

						get_conf_str("net",net,"/tmp/state/status");
						if(strcmp(net,"true")==0){//wait for internet..
							//updateSysVal("isDownloading","true");
							//system("mcu_control -s 4");
							//updateSysVal("led_status","4");
						}				
					

						downloading_vid=1;
						//dn->download_status=DOWNLOADING;
						if(dn->download_status == PAUSE)
							{
							status_downloading=0;
							downloading_vid=0;
							updateSysVal("vid","0");
							updateSysVal("percent","0");
							updateSysVal("isDownloading","false");
//							system("mcu_control -s 1");
							system("pwm_control 1 1 0;pwm_control 1 0 0");
							updateSysVal("led_status","1");
							break;
						}
						strcpy(retry_vid,dn->vid);
					    if(PTHREAD_CREATE(&download_ptid,NULL, (void *)handle_download_task,dn) != 0)
					    {
					        //safe_free(task_dn);    //have problem
					        p_debug("download task pthread create error");
							updateSysVal("vid","");
							updateSysVal("percent","");
							updateSysVal("isDownloading","false");
//							system("mcu_control -s 1");
							system("pwm_control 1 1 0;pwm_control 1 0 0");
							updateSysVal("led_status","1");

					        status_downloading=0;
							downloading_vid=0;
							return -1;
					    }
						PTHREAD_DETACH(download_ptid);
					break;
				}
				dn = (dn)->dn_next;
			}	

		}
		//pthread_rwlock_unlock(&task_dn);
		p_debug("status_downloading=%d",status_downloading);
		p_debug("download_img_first=%d",download_img_first);		
		//display_task_dnode(task_dn);
		if((status_downloading == 0) && (downloading_vid != 0)){
		//if((status_downloading == 0)){			
			updateSysVal("vid","0");
			updateSysVal("percent","0");
			updateSysVal("progress","0");			
			updateSysVal("totalSize","0");			
			updateSysVal("isDownloading","false");			
			downloading_vid=0;
//			system("mcu_control -s 1");//led on
			system("pwm_control 1 1 0;pwm_control 1 0 0");
			updateSysVal("led_status","1");			
		}
		if(status_downloading==1){
			//display_task_dnode(task_dn);
		}
		#endif
		#if 1
		/*sleep(1);*/
		if(report_status_time >= 60){
			report_status_time = 0;
			if(PTHREAD_CREATE(&report_ptid,NULL, (void *)handle_report_status_task,NULL) != 0){
				p_debug("pthread_create error");
				return -1;
			}
			PTHREAD_DETACH(report_ptid);
		}
		else{
			report_status_time++;
		}	
		#endif
		sleep(1);
		backup_time++;
		if(backup_time>120){
			backup_time=0;
			//is_backup=1;
			updateSysVal("is_backup","1");
			system("cp -f /tmp/mnt/USB-disk-1/hack/.tasklist /tmp/mnt/USB-disk-1/hack/.tasklist.bak");
			updateSysVal("is_backup","0");
			//is_backup=0;
		}
		if(query_album_time>=3600||flag_new_follow_add){
			query_album_time=0;
			if(flag_new_follow_add)sleep(1);
			
			if(is_query_album == 0){
				if(PTHREAD_CREATE(&query_ptid,NULL, (void *)handle_query_album_task,NULL) != 0)
			    {
			        p_debug("pthread_create query_ptid error");
					return -1;
			    }
				PTHREAD_DETACH(query_ptid);
			}
		}else{
			query_album_time++;
		}
		
	}
	p_debug("access LETV_task_process outout");
    return 0;

}

int del_removed_album(){

	struct album_node *album=album_dn;
	int i,ret=0;
	for(i=0;;i++){
		if(!album)break;
		if(album->status==REMOVE){
			ret=del_album_from_list(album_dn,album);
			if(ret==0)return ret;
			else{
				p_debug("del album [%s] from list error",album->pid);
			}
		}
		album=album->dn_next;
	}


}
int checkUdisk()
{
	char buffer[256]; 
	FILE *read_fp; 
	int chars_read; 
	memset( buffer, 0, 256 ); 
	char tmpStr[30];
//	printf("int size=%d\n",sizeof(int));

	sprintf(tmpStr,"mount |grep USB-disk");
//	printf("tmpStr=%s\n",tmpStr);

	read_fp=popen(tmpStr,"r");
	if(read_fp!=NULL)
		{
		chars_read = fread(buffer, sizeof(char), 256-1, read_fp); 
		if (chars_read > 0) 
		{ 
			pclose(read_fp); 
			p_debug("Disk is mounted.");
			return 1;
		} 
		else 
		{ 
			pclose(read_fp); 
			p_debug("Disk is not mounted.");
			return 0; 
		} 

	}
}

#if 1

int getMac(){
	FILE *read_fp = NULL;
	read_fp = fopen("/etc/mac.txt", "r");
	if(read_fp != NULL)
	{
		memset(tmp_mac,0,32);
		fgets(tmp_mac, 18, read_fp);
		return 0;
	}
	else
		return -1;
	//p_debug("tmp_mac=%s",tmp_mac);
}
int get_file_ext(char * filename,char* format){
	char *ptr;
	char ext[16]="\0";
	int i=1;
	//p_debug("filename=%s\n",filename);
	ptr=strrchr(filename,'.');
	if(ptr != NULL ){
		//printf("ptr=%c\n",*ptr);
		while((*(ptr+i) != '\0')&&(i<16)){
			ext[i-1]=*(ptr+i);
			//printf("ext[%d]=%c\n",i-1,ext[i-1]);
			i++;
		}
	}
	ext[i]='\0';
	//printf("ext[]=%s\n",ext);
	sprintf(format,"%s",ext);
	//p_debug(".ext=%s\n",format);
}

int filter_dot(const struct dirent *ent)
{
	char format[16]={0};
	get_file_ext(ent->d_name,format);
	//p_debug("format=%s",format);
	return (!strcmp(format,"vid"));
	//return (ent->d_name[0] != '.');
}	

int scanTaskToList(){
	p_debug("access scanTaskToList");
	
	struct dirent **namelist;
	int i=0;
	int k=0;
	struct task_dnode *cur;
	FILE *record_fd;
	char path[256]={0};
	int total = scandir(LETV_DOWNLOAD_DIR_PATH, &namelist, filter_dot, alphasort);
	if(total < 0)
	{	
		//perror("scandir");
		return -2;
	}
	for(i = 0; i < total; i++){
		//p_debug("d_name=%s",namelist[i]->d_name);
		memset(path,0,strlen(path));
		sprintf(path,"%s/%s",LETV_DOWNLOAD_DIR_PATH,namelist[i]->d_name);
		if((record_fd = fopen(path,"r"))==NULL)//改动：路径
		{
			 p_debug("task file does not exist [errno:%d]",errno);
			 //return -1;
			 continue;
		}

		cur = (struct album_node *)malloc(sizeof(*cur));
		memset(cur,0,sizeof(*cur));		
		int enRet = fread(cur,sizeof(struct task_dnode),1,record_fd);
		 if(enRet <= 0)
	     {	
	     	if(cur!=NULL)free(cur);
			p_debug("fread error,enRet = %d,errno = %d",enRet,errno);
			break;
		 }
	
		if(!cur)
		{
			p_debug("malloc task_dn error");
			fclose(record_fd);
			return -2;
		}
		fclose(record_fd);
		if(IsInt(cur->vid)){
			add_task_to_list(&task_dn,cur,3);//3= scan add
		}else 
			safe_free(cur);
		
	}
}

int main(int argc, char *argv[])
{
    int rslt = 0;
	int ret=0;	
	char net[8]={0};
	report_status_err_time=0;
	download_img_first=0;
	printf("##################################");
	p_debug("letv download server v0.0.1");
	printf("##################################");

	while (checkUdisk()==0){
		usleep(100000);
		p_debug("Waiting for disk");
	}
	//sleep(3);
	if(getMac()!=0){
		p_debug("get the mac address failed.");
		strcpy(tmp_mac,"84:5D:D7:01:01:01");
		//return;//copy to tmp_mac
	}
	
	pthread_rwlock_init(&album_list_lock,NULL);
	pthread_rwlock_init(&task_list_lock,NULL);

	(void) signal(SIGTERM, signal_handler);
	(void) signal(SIGINT, signal_handler);


	if(access(LETV_DOWNLOAD_DIR_PATH,R_OK)!=0){// 不存在

		if (mkdir(LETV_DOWNLOAD_DIR_PATH,0777))//如果不存在就用mkdir函数来创建 
        { 
            p_debug("Create Directory %s failed!!!,exit",LETV_DOWNLOAD_DIR_PATH); 
			return 0;
        } 
		p_debug("Download dir is empty, create a new one %s",LETV_DOWNLOAD_DIR_PATH);
	}
	album_dn=NULL;
	//album_dn->dn_next=NULL;
	if(access(LETV_TASKLIST_FILE_PATH_BACKUP,R_OK)==0){//存在
		system("cp -f /tmp/mnt/USB-disk-1/hack/.tasklist.bak /tmp/mnt/USB-disk-1/hack/.tasklist");
		usleep(100000);
	}
	ret=read_album_list_from_file(LETV_ALBUMLIST_FILE_PATH,&album_dn);
	display_album_dnode(album_dn);
	
	if(ret<0){//the first time start the device, albumlist is not exist.
		p_debug("#######read albumlist file error.######");
		return;
	}

	task_dn=NULL;
	//task_dn->dn_next=NULL;

	ret = read_list_from_file(LETV_TASKLIST_FILE_PATH,&task_dn);

	scanTaskToList();
	
	if(ret<0){//the first time start the device, tasklist is not exist.
		p_debug("#######read tasklist file error.######");
		return;
	}

	display_task_dnode(task_dn);
	/**/


	struct shttpd_ctx	*ctx;
	ctx = shttpd_init();
	open_listening_ports(ctx);
	p_debug("dm_server started on port(s) %d", INIT_PORT);

	//LETV_album_process();
	PTHREAD_T upgrade_ptid;
	if(PTHREAD_CREATE(&upgrade_ptid,NULL, (void *)handle_check_firmware_task,NULL) != 0)
	{
		//safe_free(task_dn);	 //have problem
		destory_task_list(task_dn);
		destory_album_list(album_dn);
		p_debug("check firmware pthread create error");
		return -1;
	}
	PTHREAD_DETACH(upgrade_ptid);
	

	sleep(1);

	PTHREAD_T ptid;
	if(PTHREAD_CREATE(&ptid,NULL, (void *)LETV_task_process,NULL) != 0)
	{
		//safe_free(task_dn);	 //have problem
		destory_task_list(task_dn);
		destory_album_list(album_dn);
		p_debug("pthread_create error");
		return -1;
	}
	PTHREAD_DETACH(ptid);
/*
	PTHREAD_T cover_pid;
	if(PTHREAD_CREATE(&cover_pid,NULL, (void *)download_cover,NULL) != 0)
	{
		//safe_free(task_dn);	 //have problem
		destory_task_list(task_dn);
		destory_album_list(album_dn);
		p_debug("pthread_create error");
		return -1;
	}
	PTHREAD_DETACH(cover_pid);
*/
	
	while (exit_flag == 0)
		shttpd_poll(ctx, 5000);
	shttpd_fini(ctx);
	p_debug("exit succ");
quit:
    p_debug("----------------msg server main task quit!----------");
	destory_task_list(task_dn);
	destory_album_list(album_dn);

	pthread_rwlock_destroy(&album_list_lock);
	pthread_rwlock_destroy(&task_list_lock);
    return rslt;
}
#endif
#if 0

int main(int argc, char *argv[])
{
	struct task_dnode my_task_dn;
	memset(&my_task_dn, 0, sizeof(struct task_dnode));
	p_debug("start");
	//strcpy(my_task_dn.vid, "23828813");
	strcpy(my_task_dn.vid, "23828804");
	strcpy(my_task_dn.pid, "10007684");
	_handle_file_json_req(&my_task_dn);
	while(1)
	{
		sleep(3);
	}
	return 0;
}
#endif


