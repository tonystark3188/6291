#ifndef _MEDIA_INFO_H_
#define _MEDIA_INFO_H_

#include <stdio.h>
#include <stdint.h>
#define LIB_VERSION "1.1.0"

typedef struct
{
	char ApertureValue[32];
	char MaxApertureValue[32];
	char CameraType[32];
	char ExposureBiasValue[32];
	char ExposureTime[32];
	char Flash[64];
	char FocalLength[32];
	char MeteringMode[32];
	char DateTime[32];
	char Make[32];
	char ISOSpeedRatings[16];
	int YResolution;
	int XResolution;
} image_media_info_t;
typedef struct
{
	char album[64];
	char artist[32];
	int bit_rate;
	int64_t duration;
	char encode[32];
	char name[128];
} audio_media_info_t;
typedef struct
{
	int width;
	int height;
	int bit_rate;
	char encode[32];
	int64_t duration;
} video_media_info_t;


int get_video_media_info(const char *path,video_media_info_t** ppVideoMediaInfo);
int get_audio_media_info(const char *path,audio_media_info_t** ppAudioMediaInfo);
int get_image_media_info(const char *path,image_media_info_t** ppImageMediaInfo);

void free_video_media_info(video_media_info_t** ppVideoMediaInfo);
void free_audio_media_info(audio_media_info_t** ppAudioMediaInfo);
void free_image_media_info(image_media_info_t** ppImageMediaInfo);
char* get_lib_version();


#endif
