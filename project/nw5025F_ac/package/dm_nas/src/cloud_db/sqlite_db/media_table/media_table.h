#ifndef MEDIA_TABLE_H
#define MEDIA_TABLE_H

#include "db_base.h"

#define VIDEO_TABLE_NAME "video_table"
#define AUDIO_TABLE_NAME "audio_table"
#define IMAGE_TABLE_NAME "image_table"
#define THUM_TABLE_NAME  "thumbnail_table"


#define MAX_ENCODE_TYPE_SIZE    8

typedef struct  VideoMediaInfoTable
{
	int index;
	int width;		// in pixels
	int height;	// in pixels
	int bitrate; // in bps 
	char encode_type[MAX_ENCODE_TYPE_SIZE];
	long long duration_time; // in seconds
}video_info_t;


#define MAX_VENDOR_NAME_SIZE        32
#define MAX_CAMERA_TYPE_NAME_SIZE   32
#define MAX_APETURE_VALUE_SIZE      32
#define EXPOSURE_TIME_VALUE_SIZE	32
#define ISO_VALUE_VALUE_SIZE		16
#define EXPOSURE_BIAS_VALUE_SIZE    32
#define FOCAL_LENGTH				32
#define METER_MODE_SIZE				32
#define FLASHLIGHT_ON_SIZE			64

typedef struct PhotoMediaInfoTable
{
	int index;
	int width;
	int height;
	long photo_time;
	char vendor_name[MAX_VENDOR_NAME_SIZE];
	char camera_type[MAX_CAMERA_TYPE_NAME_SIZE];
	char aperture_value[MAX_APETURE_VALUE_SIZE];
	char exposure_time[EXPOSURE_TIME_VALUE_SIZE];
	char iso_value[ISO_VALUE_VALUE_SIZE];
	char exposure_bias_val[EXPOSURE_BIAS_VALUE_SIZE];
	char focal_length[FOCAL_LENGTH];
	char max_apeture_val[MAX_APETURE_VALUE_SIZE];
	char meter_mode[METER_MODE_SIZE];
	char flashlight_on[FLASHLIGHT_ON_SIZE];
}image_info_t;

typedef struct thumMediaInfoTable
{
	int 	index;
	char	small_path[MAX_FILE_PATH_LEN];
	char	median_path[MAX_FILE_PATH_LEN];
}thum_info_t;



#define MAX_SONG_NAME_SIZE      128
#define MAX_ALBUM_NAME_SIZE     128
#define MAX_ARTIST_NAME_SIZE    128

typedef struct AudioMediaInfoTable
{
	int index;
	long long duration_time; // in seconds
	int  bitrate; // in bps
	char song_name[MAX_SONG_NAME_SIZE];
	char album_name[MAX_ALBUM_NAME_SIZE];
	char artist_name[MAX_ARTIST_NAME_SIZE];
	char encode_type[MAX_ENCODE_TYPE_SIZE];	
}audio_info_t;


typedef union
{
    video_info_t video_info;
	audio_info_t audio_info;
	image_info_t image_info; 
	thum_info_t  thum_info;
}media_info_t;

error_t get_media_max_id(sqlite3 *database,uint32_t *max_id,char *media_table);

error_t load_audio_update_cmd(sqlite3 *database,audio_info_t *paudio);

error_t load_image_update_cmd(sqlite3 *database,image_info_t *pimage);

error_t load_video_update_cmd(sqlite3 *database,video_info_t *pvideo);

error_t load_video_insert_cmd(sqlite3 *database, video_info_t *pvi);

error_t load_audio_insert_cmd(sqlite3 *database,audio_info_t *pai);

error_t load_image_insert_cmd(sqlite3 *database, image_info_t *pii);

error_t load_image_query_cmd(sqlite3 *database,image_info_t *pimage);

error_t load_video_query_cmd(sqlite3 *database,video_info_t *pvideo);

error_t load_audio_query_cmd(sqlite3 *database,audio_info_t *paudio);

error_t load_thum_query_cmd(sqlite3 *database,thum_info_t *pthum);

error_t load_thum_insert_cmd(sqlite3 *database, thum_info_t *pii);



#endif
