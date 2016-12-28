//
//  dmdownload.c
//  DMDOWNLOAD
//
//  Created by Oliver on 16/6/27.
//  Copyright © 2016年 Oliver. All rights reserved.
//

#include "dmdownload.h"
#include "ghttp.h"
#include "my_debug.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int download_bt_file(char* url , char* save_file_path)
{
	ENTER_FUNC();
    ghttp_request*		request = NULL;
    ghttp_status		status;
    ghttp_current_status download_progress;
    char*				buf = 0;
    int					bytes_read=0 ;
	int 				bytes_write = 0;
    FILE*				fp=NULL;

    fp = fopen( save_file_path , "wb");
	if(fp == NULL)
	{
		DMCLOG_E("path is not exist:%s",save_file_path);
		return -1;
	}
    
    request = ghttp_request_new();
    if(ghttp_set_uri(request, url) == -1) {
        if (fp != NULL) {
            fclose(fp);
        }
        ghttp_request_destroy(request);
        return -1;
    }
    if(ghttp_set_type(request, ghttp_type_get) == -1) {
        if (fp != NULL) {
            fclose(fp);
        }
        ghttp_request_destroy(request); 
        return -1;
    }
    
    ghttp_set_sync(request, ghttp_async);
    ghttp_prepare(request);
    
    do{
        status = ghttp_process(request);
        if(status == ghttp_error)  {
            DMCLOG_D("download file failed errno = %d",errno);
			if (fp != NULL) {
            	fclose(fp);
			}
            ghttp_close(request);
            ghttp_request_destroy(request);
			EXIT_FUNC();
            return -1;
        }
        // A solution
        //download_progress = ghttp_get_status(request);
		//DMCLOG_D("bytes_read = %d,bytes_total = %d",download_progress.bytes_read,download_progress.bytes_total);
    } while( status == ghttp_not_done );
    
    bytes_read = ghttp_get_body_len(request);
    buf = ghttp_get_body(request);

	DMCLOG_D("bytes_read = %d",bytes_read);
    
    fwrite( buf ,bytes_read , 1 , fp);
	DMCLOG_D("bytes_read = %d",bytes_read);
    fclose(fp);
	DMCLOG_D("bytes_read = %d",bytes_read);
	
    ghttp_close(request);
	DMCLOG_D("bytes_read = %d",bytes_read);
    ghttp_request_destroy(request);
    EXIT_FUNC();
    return 0;
}

/**
 *  功能：添加下载任务
 *  权限：开放权限
 *
 *  @param url:输入，资源地址；task_id:输出，任务唯一标识符
 *
 *  @return 0为成功，非0为异常。
 */
DMDOWNLOAD_RESULT download_add_task(void *self)
{
    int res = DMDOWNLOAD_SUCCES;
    DownloadTaskInfo *dTaskInfo = (DownloadTaskInfo *)self;
    
    if(dTaskInfo == NULL||dTaskInfo->add_task_cb == NULL)
    {
        return DMDOWNLOAD_PARA_ERROR;
    }
    
    MDownloadTask *fdi = NULL;
    dl_list_for_each(fdi, &dTaskInfo->t_head, MDownloadTask, node)
    {
        if((res = dTaskInfo->add_task_cb(fdi->url,fdi->task_id)) != DMDOWNLOAD_SUCCES)
        {
            DMCLOG_E("add task error");
            return res;
        }
    }
    return res;
}


/**
 *  功能：控制下载任务
 *  权限：开放权限
 *
 *  @param ctrl 0 :下载任务，1:暂停，2:等待，3:删除
 *         dTaskInfo->dTaskList 需要控制的任务链表
 *
 *  @return 0为成功，非0为异常。
 */
DMDOWNLOAD_RESULT download_ctr_task(void *self)
{
	ENTER_FUNC();
    int res = DMDOWNLOAD_SUCCES;
    DownloadTaskInfo *dTaskInfo = (DownloadTaskInfo *)self;
    if(dTaskInfo == NULL)
    {
        return DMDOWNLOAD_PARA_ERROR;
    }
    if(dTaskInfo->ctr_task_cb == NULL)
    {
        return DMDOWNLOAD_PARA_ERROR;
    }
    MDownloadTask *fdi = NULL;
    dl_list_for_each(fdi, &dTaskInfo->t_head, MDownloadTask, node)
    {
    	DMCLOG_D("fdi->task_id = %s",fdi->task_id);
        if(*fdi->task_id)//task_id存在 代表着要控制此task_id
        {
            if((res = dTaskInfo->ctr_task_cb(fdi->ctrl,fdi->task_id)) != DMDOWNLOAD_SUCCES)
            {
                DMCLOG_E("ctrl task error");
                return res;
            }
        }else{//task_id不存在，代表着要添加此任务
        	if(fdi->type == 1)//BT file,need to download from server
        	{
    			char *tmp = strstr(fdi->url,"mediaId");
				char *fileId = (char *)calloc(1,strlen(tmp + strlen("mediaId") +  1) + 1);
				strcpy(fileId,tmp + strlen("mediaId") +  1);
				DMCLOG_D("fileId = %s",fileId);
				
				char *bt_download_path = (char *)calloc(1,strlen(BT_ROOT_PATH) + strlen(fileId) + 32);
				sprintf(bt_download_path,"%s/%s.torrent",BT_ROOT_PATH,fileId);
				DMCLOG_D("bt_download_path = %s",bt_download_path);
				res = download_bt_file(fdi->url,bt_download_path);
				if(res != 0)
				{
					if(errno == 32)
					{
						res = download_bt_file(fdi->url,bt_download_path);
						if(res != 0)
						{
							DMCLOG_D("download bt file error");
							safe_free(fileId);
							safe_free(bt_download_path);
							return DMDOWNLOAD_BT_FILE_ERROR;
						}
		
					}else{
						DMCLOG_D("download bt file error");
						safe_free(fileId);
						safe_free(bt_download_path);
						return DMDOWNLOAD_BT_FILE_ERROR;
					}
					
				}

				char *bt_file_http_path = (char *)calloc(1,strlen(bt_download_path) + strlen(LOCAL_PATH) + 1);
				if(bt_file_http_path == NULL)
				{
					safe_free(fileId);
					safe_free(bt_download_path);
					return -1;
				}
				sprintf(bt_file_http_path,"%s%s",LOCAL_PATH,bt_download_path + strlen(BT_ROOT_PATH));
				DMCLOG_D("bt_file_http_path = %s",bt_file_http_path);
				if((res = dTaskInfo->add_task_cb(bt_file_http_path,fdi->task_id)) != DMDOWNLOAD_SUCCES)
	            {
	                DMCLOG_E("add task error");
					safe_free(fileId);
					safe_free(bt_download_path);
					safe_free(bt_file_http_path);
					return res;
	            }
				safe_free(fileId);
				safe_free(bt_download_path);
				safe_free(bt_file_http_path);
				
			}else{
				if((res = dTaskInfo->add_task_cb(fdi->url,fdi->task_id)) != DMDOWNLOAD_SUCCES)
	            {
	                DMCLOG_E("add task error");
					return res;
	            }
			}
			
            
			DMCLOG_D("taskid=%s",fdi->task_id);
        }
        
    }
	EXIT_FUNC();
    return res;
}

/**
 *  功能：控制全部下载任务
 *  权限：开放权限
 *
 *  @param dTaskInfo->ctrl 0 :下载任务，1:暂停，2:等待，3:删除
 *
 *  @return 0为成功，非0为异常。
 */
DMDOWNLOAD_RESULT download_ctr_all_task(void *self)
{
    int res = DMDOWNLOAD_SUCCES;
    DownloadTaskInfo *dTaskInfo = (DownloadTaskInfo *)self;
    if(dTaskInfo == NULL)
    {
        return DMDOWNLOAD_PARA_ERROR;
    }
    if(dTaskInfo->ctr_all_task_cb == NULL)
    {
        return DMDOWNLOAD_PARA_ERROR;
    }
    if((res = dTaskInfo->ctr_all_task_cb(dTaskInfo->ctrl)) != DMDOWNLOAD_SUCCES)
    {
        DMCLOG_E("ctrl all task error");
        return res;
    }
    return res;
}

/**
 * func:ctrl download or upload speed
 *
 *  @param dTaskInfo->upload_speed,dTaskInfo->download_speed
 *
 *  @return 0:succ ,-1:failed
 */
DMDOWNLOAD_RESULT download_ctr_speed(void *self)
{
    int res = DMDOWNLOAD_SUCCES;
    DownloadTaskInfo *dTaskInfo = (DownloadTaskInfo *)self;
    if(dTaskInfo == NULL)
    {
        return DMDOWNLOAD_PARA_ERROR;
    }
    if(dTaskInfo->ctr_speed_cb == NULL)
    {
        return DMDOWNLOAD_PARA_ERROR;
    }
    if((res = dTaskInfo->ctr_speed_cb(&dTaskInfo->dGlobalStatus)) != DMDOWNLOAD_SUCCES)
    {
        DMCLOG_E("ctrl all task error");
        return res;
    }
    return res;
}
/**
 *  功能：获取下载任务列表
 *  权限：开放权限
 *
 *  @param status 0 :下载中的任务，1:暂停中的任务，2:等待中的任务，3:已完成的任务，4下载失败的任务
 *
 *  @return 0为成功，非0为异常。
 */
DMDOWNLOAD_RESULT download_query_task(void *self)
{
    int res = DMDOWNLOAD_SUCCES;
    int status = 0;
    DownloadTaskInfo *dTaskInfo = (DownloadTaskInfo *)self;
    if(dTaskInfo == NULL||dTaskInfo->query_task_by_status_cb == NULL)
    {
    	DMCLOG_E("para is null");
        return DMDOWNLOAD_PARA_ERROR;
    }
    for(status = 0;status < 5;status++)
    {
        if((res = dTaskInfo->query_task_by_status_cb(status,&dTaskInfo->t_head)) != DMDOWNLOAD_SUCCES)
        {
            DMCLOG_E("query task error");
            return res;
        }
    }
    return res;
}
/**
 *  功能：获取下载目录列表
 *  权限：开放权限
 *
 *  @param
 *
 *  @return DownloadTaskInfo->dDirList:目录列表，非空为成功，NULL为异常。
 */
DMDOWNLOAD_RESULT download_query_dir(void *self)
{
    int res = DMDOWNLOAD_SUCCES;
    DownloadTaskInfo *dTaskInfo = (DownloadTaskInfo *)self;
    if(dTaskInfo == NULL||dTaskInfo->query_dir_cb == NULL)
    {
        return DMDOWNLOAD_PARA_ERROR;
    }
    if((res = dTaskInfo->query_dir_cb(&dTaskInfo->d_head)) != DMDOWNLOAD_SUCCES)
    {
        DMCLOG_E("query dir error");
        return res;
    }
    return res;
}


/**
 *  功能：获取下载模块全局状况
 *  权限：开放权限
 *
 *  @param
 *
 *  @return 0为成功，非0为异常。
 */
DMDOWNLOAD_RESULT download_query_global(void *self)
{
    int res = DMDOWNLOAD_SUCCES;
    DownloadTaskInfo *dTaskInfo = (DownloadTaskInfo *)self;
    
    if(dTaskInfo == NULL||dTaskInfo->query_global_cb == NULL)
    {
        return DMDOWNLOAD_PARA_ERROR;
    }

    if((res = dTaskInfo->query_global_cb(&dTaskInfo->dGlobalStatus)) != DMDOWNLOAD_SUCCES)
    {
        DMCLOG_E("add task error");
        return res;
    }

    return res;
}