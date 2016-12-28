#include "db_opr.h"
#include "token_manage.h"
#include "afvfs.h"
#include "rfsvfs.h"


/**
 * 获取用户列表信息
 */
ERROR_CODE_VFS afvfs_get_user_list(VFS_USER_INFO** user_info,const char * token)
{
	token_dnode_t *token_dnode = (token_dnode_t *)token;
	return 0;
}

/**
 * 获取媒体文件小尺寸缩略图.该种缩略图主要用于列表或网格样式展示。
 */
ERROR_CODE_VFS afvfs_get_thumbnail_small(const char *path,FILE **fp,const char * token)
{
	if(path == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	token_dnode_t *token_dnode = (token_dnode_t *)token;
	v_file_info_t v_file_info;
	memset(&v_file_info,0,sizeof(v_file_info_t));
	char db_path[MAX_FILE_PATH_LEN] = {0};
	if(*path)
	{
		sprintf(db_path,"/%s/%s",token_dnode->bucket_name,path);
	}else{
		sprintf(db_path,"/%s",token_dnode->bucket_name);
	}
	S_STRNCPY(v_file_info.path,db_path,MAX_FILE_PATH_LEN);
	
	int res = handle_media_query(V_FILE_TABLE_QUERY_THUM_INFO,&v_file_info);
	if(res != 0)
	{
		DMCLOG_E("query thum info from  v file table error");
		return res;
	}

	char thumbnail_real_path[MAX_FILE_PATH_LEN] = {0};
	
	S_STRNCPY(thumbnail_real_path,v_file_info.media_info.thum_info.median_path,MAX_FILE_PATH_LEN);
	DMCLOG_D("thumbnail_real_path = %s",thumbnail_real_path);

	*fp = rfsvfs_fopen(thumbnail_real_path, "r");
	if(*fp == NULL)
	{
		DMCLOG_E("fopen %s error[%d]",thumbnail_real_path,errno);
		return -1;
	}
	return res;
}

/**
 * 获取媒体文件中等尺寸缩略图.该种缩略图主要用于移动设备或小屏设备在大图浏览时展示的图片。
 */
ERROR_CODE_VFS afvfs_get_thumbnail_median(const char *path,FILE **fp,const char * token)
{
	if(path == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	token_dnode_t *token_dnode = (token_dnode_t *)token;
	v_file_info_t v_file_info;
	memset(&v_file_info,0,sizeof(v_file_info_t));
	char db_path[MAX_FILE_PATH_LEN] = {0};
	if(*path)
	{
		sprintf(db_path,"/%s/%s",token_dnode->bucket_name,path);
	}else{
		sprintf(db_path,"/%s",token_dnode->bucket_name);
	}
	S_STRNCPY(v_file_info.path,db_path,MAX_FILE_PATH_LEN);
	
	int res = handle_media_query(V_FILE_TABLE_QUERY_THUM_INFO,&v_file_info);
	if(res != 0)
	{
		DMCLOG_E("query thum info from  v file table error");
		return res;
	}

	char thumbnail_real_path[MAX_FILE_PATH_LEN] = {0};
	
	S_STRNCPY(thumbnail_real_path,v_file_info.media_info.thum_info.small_path,MAX_FILE_PATH_LEN);
	DMCLOG_D("thumbnail_real_path = %s",thumbnail_real_path);

	*fp = rfsvfs_fopen(thumbnail_real_path, "r");
	if(*fp == NULL)
	{
		DMCLOG_E("fopen %s error[%d]",thumbnail_real_path,errno);
		return -1;
	}
	return res;
}


/**
 * 获取图片媒体文件信息
 */
ERROR_CODE_VFS afvfs_get_image_media_info(const char *path,IMAGE_MEDIA_INFO * image_media_info,const char * token)
{
	if(path == NULL || image_media_info == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	token_dnode_t *token_dnode = (token_dnode_t *)token;
	v_file_info_t v_file_info;
	memset(&v_file_info,0,sizeof(v_file_info_t));
	char db_path[MAX_FILE_PATH_LEN] = {0};
	if(*path)
	{
		sprintf(db_path,"/%s/%s",token_dnode->bucket_name,path);
	}else{
		sprintf(db_path,"/%s",token_dnode->bucket_name);
	}
	S_STRNCPY(v_file_info.path,db_path,MAX_FILE_PATH_LEN);
	
	int res = handle_media_update(V_FILE_TABLE_QUERY_IMAGE_INFO,&v_file_info);
	if(res != 0)
	{
		DMCLOG_E("query image info from  v file table error");
		return res;
	}

	memcpy(image_media_info,&v_file_info.media_info.image_info,sizeof(image_info_t));
	return res;
}
/**
 * 获取音频媒体文件信息
 */
ERROR_CODE_VFS afvfs_get_audio_media_info(const char *path,AUDIO_MEDIA_INFO * audio_media_info,const char * token)
{
	if(path == NULL || audio_media_info == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	token_dnode_t *token_dnode = (token_dnode_t *)token;
	v_file_info_t v_file_info;
	memset(&v_file_info,0,sizeof(v_file_info_t));
	char db_path[MAX_FILE_PATH_LEN] = {0};
	if(*path)
	{
		sprintf(db_path,"/%s/%s",token_dnode->bucket_name,path);
	}else{
		sprintf(db_path,"/%s",token_dnode->bucket_name);
	}
	S_STRNCPY(v_file_info.path,db_path,MAX_FILE_PATH_LEN);
	
	int res = handle_media_query(V_FILE_TABLE_QUERY_AUDIO_INFO,&v_file_info);
	if(res != 0)
	{
		DMCLOG_E("query audio info from v file table error");
		return res;
	}

	memcpy(audio_media_info,&v_file_info.media_info.audio_info,sizeof(audio_info_t));
	return res;
}
/**
 * 获取视频媒体文件信息
 */
ERROR_CODE_VFS afvfs_get_video_media_info(const char *path,VIDEO_MEDIA_INFO * video_media_info,const char * token)
{
	if(path == NULL || video_media_info == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	token_dnode_t *token_dnode = (token_dnode_t *)token;
	v_file_info_t v_file_info;
	memset(&v_file_info,0,sizeof(v_file_info_t));
	char db_path[MAX_FILE_PATH_LEN] = {0};
	if(*path)
	{
		sprintf(db_path,"/%s/%s",token_dnode->bucket_name,path);
	}else{
		sprintf(db_path,"/%s",token_dnode->bucket_name);
	}
	S_STRNCPY(v_file_info.path,db_path,MAX_FILE_PATH_LEN);
	
	int res = handle_media_query(V_FILE_TABLE_QUERY_VIDEO_INFO,&v_file_info);
	if(res != 0)
	{
		DMCLOG_E("query audio info from v file table error");
		return res;
	}
	memcpy(video_media_info,&v_file_info.media_info.video_info,sizeof(video_info_t));
	return res;
}




////////////// 媒体文件处理部分///////////////////

/**
 * 当有文件新增的时候会回调已注册的回调函数，提交给上层进行任务处理。
 */
ERROR_CODE_VFS afvfs_set_on_new_file_handler(const char *new_file_real_path,
							const int meida_type,
							ON_NEW_FILE_HANDLE func,
							unsigned int pri)
{ 
	return add_media_to_list(new_file_real_path,meida_type,func,pri);
}


/**
 * 获取一个未使用的文件路径，可以获取需要保存缩略图或裁剪过后的视频的目标路径。
 * 高级模块获取到真实文件路径的时候需要对该路径进行标记，如果最终文件没有完成或操作失败的时候需要
 * 可以清理该路径遗留下来的文件。
 */
ERROR_CODE_VFS afvfs_get_new_file_path(char pathBuf[32])
{
	return rfsvfs_get_new_file_path(pathBuf);
}


/**
 * 设置媒体文件缩略图.该种缩略图主要用于列表或网格样式展示。
 */
ERROR_CODE_VFS afvfs_set_thumbnail_small(const char *real_path,const char *thumbnail_real_path)
{
	if(real_path == NULL || thumbnail_real_path == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	v_file_info_t v_file_info;
	memset(&v_file_info,0,sizeof(v_file_info_t));
	S_STRNCPY(v_file_info.real_path,real_path,MAX_FILE_PATH_LEN);
	S_STRNCPY(&v_file_info.media_info.thum_info.small_path,thumbnail_real_path,MAX_FILE_PATH_LEN);
	int res = handle_media_update(V_FILE_TABLE_UPDATE_THUM_INFO,&v_file_info);
	if(res != 0)
	{
		DMCLOG_E("update median thumbnail error");
		return res;
	}
	return res;
}

/**
 * 设置媒体文件中等缩略图.该种缩略图主要用于移动设备或小屏设备在大图浏览时展示的图片。
 */
ERROR_CODE_VFS afvfs_set_thumbnail_median(const char *real_path,const char *thumbnail_real_path)
{
	if(real_path == NULL || thumbnail_real_path == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	v_file_info_t v_file_info;
	memset(&v_file_info,0,sizeof(v_file_info_t));
	S_STRNCPY(v_file_info.real_path,real_path,MAX_FILE_PATH_LEN);
	S_STRNCPY(&v_file_info.media_info.thum_info.median_path,thumbnail_real_path,MAX_FILE_PATH_LEN);
	int res = handle_media_update(V_FILE_TABLE_UPDATE_THUM_INFO,&v_file_info);
	if(res != 0)
	{
		DMCLOG_E("update median thumbnail error");
		return res;
	}
	return res;
}

/**
 * 设置图片媒体文件信息
 */
ERROR_CODE_VFS afvfs_set_image_media_info(const char *real_path,IMAGE_MEDIA_INFO * image_media_info)
{
	if(real_path == NULL || image_media_info == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	v_file_info_t v_file_info;
	memset(&v_file_info,0,sizeof(v_file_info_t));
	S_STRNCPY(v_file_info.real_path,real_path,MAX_FILE_PATH_LEN);
	memcpy(&v_file_info.media_info.image_info,image_media_info,sizeof(image_info_t));
	int res = handle_media_update(V_FILE_TABLE_UPDATE_IMAGE_INFO,&v_file_info);
	if(res != 0)
	{
		DMCLOG_E("update v file table error");
		return res;
	}
	return res;
}
/**
 * 设置音频媒体文件信息
 */
ERROR_CODE_VFS afvfs_set_audio_media_info(const char *real_path,AUDIO_MEDIA_INFO * audio_media_info)
{
	if(real_path == NULL || audio_media_info == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	v_file_info_t v_file_info;
	memset(&v_file_info,0,sizeof(v_file_info_t));
	S_STRNCPY(v_file_info.real_path,real_path,MAX_FILE_PATH_LEN);
	memcpy(&v_file_info.media_info.audio_info,audio_media_info,sizeof(audio_info_t));
	int res = handle_media_update(V_FILE_TABLE_UPDATE_AUDIO_INFO,&v_file_info);
	if(res != 0)
	{
		DMCLOG_E("update v file table error");
		return res;
	}
	return res;
}
/**
 * 设置视频媒体文件信息
 */
ERROR_CODE_VFS afvfs_set_video_media_info(const char *real_path,VIDEO_MEDIA_INFO * video_media_info)
{
	if(real_path == NULL || video_media_info == NULL)
	{
		DMCLOG_E("para is null");
		return -1;
	}
	v_file_info_t v_file_info;
	memset(&v_file_info,0,sizeof(v_file_info_t));
	S_STRNCPY(v_file_info.real_path,real_path,MAX_FILE_PATH_LEN);
	memcpy(&v_file_info.media_info.video_info,video_media_info,sizeof(video_info_t));
	int res = handle_media_update(V_FILE_TABLE_UPDATE_VIDEO_INFO,&v_file_info);
	if(res != 0)
	{
		DMCLOG_E("update v file table error");
		return res;
	}
	return res;
}

