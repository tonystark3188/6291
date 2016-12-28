/**
 * 高级功能模块
 * v 0.0.2
 */
#ifndef _AFVFS_H_
#define _AFVFS_H_

//#include "errorvfs.h"
#include "db_table.h"
#include "media_manage.h"

typedef struct
{
	/**
	 * 用户名
	 */
	char* username;
	
	/**
	 * 使用容量（逻辑）
	 */
	long used;

	/**
	 * 总容量（逻辑）
	 */
	long total;

//	/**
//	 * 物理使用容量
//	 */
//	long used_physical;
//
//	/**
//	 * 物理总容量
//	 */
//	long total_physical;

	uint8_t isAdmin;

} VFS_USER_INFO;


typedef image_info_t IMAGE_MEDIA_INFO;
typedef audio_info_t AUDIO_MEDIA_INFO;
typedef video_info_t VIDEO_MEDIA_INFO;
/**
 * 获取用户列表信息
 */
ERROR_CODE_VFS afvfs_get_user_list(VFS_USER_INFO** user_info,const char * token);

/**
 * 获取媒体文件小尺寸缩略图.该种缩略图主要用于列表或网格样式展示。
 */
ERROR_CODE_VFS afvfs_get_thumbnail_small(const char *path,FILE **fp,const char * token);

/**
 * 获取媒体文件中等尺寸缩略图.该种缩略图主要用于移动设备或小屏设备在大图浏览时展示的图片。
 */
ERROR_CODE_VFS afvfs_get_thumbnail_median(const char *path,FILE **fp,const char * token);


/**
 * 获取图片媒体文件信息
 */
ERROR_CODE_VFS afvfs_get_image_media_info(const char *path,IMAGE_MEDIA_INFO * image_media_info,const char * token);
/**
 * 获取音频媒体文件信息
 */
ERROR_CODE_VFS afvfs_get_audio_media_info(const char *path,AUDIO_MEDIA_INFO * audio_media_info,const char * token);
/**
 * 获取视频媒体文件信息
 */
ERROR_CODE_VFS afvfs_get_video_media_info(const char *path,VIDEO_MEDIA_INFO * video_media_info,const char * token);




////////////// 媒体文件处理部分///////////////////

/**
 * 当有文件新增的时候会回调已注册的回调函数，提交给上层进行任务处理。
 */
ERROR_CODE_VFS afvfs_set_on_new_file_handler(const char *new_file_real_path,
										const int meida_type,
										ON_NEW_FILE_HANDLE func,
										unsigned int pri);


/**
 * 获取一个未使用的文件路径，可以获取需要保存缩略图或裁剪过后的视频的目标路径。
 * 高级模块获取到真实文件路径的时候需要对该路径进行标记，如果最终文件没有完成或操作失败的时候需要
 * 可以清理该路径遗留下来的文件。
 */
ERROR_CODE_VFS afvfs_get_new_file_path(char pathBuf[32]);


/**
 * 设置媒体文件缩略图.该种缩略图主要用于列表或网格样式展示。
 */
ERROR_CODE_VFS afvfs_set_thumbnail_small(const char *real_path,const char *thumbnail_real_path);

/**
 * 设置媒体文件中等缩略图.该种缩略图主要用于移动设备或小屏设备在大图浏览时展示的图片。
 */
ERROR_CODE_VFS afvfs_set_thumbnail_median(const char *real_path,const char *thumbnail_real_path);


/**
 * 设置图片媒体文件信息
 */
ERROR_CODE_VFS afvfs_set_image_media_info(const char *real_path,IMAGE_MEDIA_INFO * image_media_info);
/**
 * 设置音频媒体文件信息
 */
ERROR_CODE_VFS afvfs_set_audio_media_info(const char *real_path,AUDIO_MEDIA_INFO * audio_media_info);
/**
 * 设置视频媒体文件信息
 */
ERROR_CODE_VFS afvfs_set_video_media_info(const char *real_path,VIDEO_MEDIA_INFO * video_media_info);




#endif

