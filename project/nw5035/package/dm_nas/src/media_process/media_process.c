#include "media_process.h"
#include "media_manage.h"
#include "resize_img.h"
#include "thpool.h"
#include "snapshot.h"
#include "rfsvfs.h"
#include "media_info.h"
#include "media_table.h"

static threadpool media_prc_thpool = NULL;

#define DISK_FULLPATH   	 "/tmp/mnt/SD-disk-1"
#define DISK_FULLPATH_LEN    (64)

static void image_media_info_t_convert(image_media_info_t *in,image_info_t *out)
{
	if(!in || !out)
	{
		DMCLOG_E("param error");
		return;
	}
	out->height = in->YResolution;
	out->width = in->XResolution;
	strncpy(out->vendor_name,in->Make,sizeof(out->vendor_name)-1);
	strncpy(out->camera_type,in->CameraType,sizeof(out->camera_type)-1);
	strncpy(out->aperture_value,in->ApertureValue,sizeof(out->aperture_value)-1);
	strncpy(out->exposure_time,in->ExposureTime,sizeof(out->exposure_time)-1);
	strncpy(out->iso_value,in->ISOSpeedRatings,sizeof(out->iso_value)-1);
	strncpy(out->exposure_bias_val,in->ExposureBiasValue,sizeof(out->exposure_bias_val)-1);
	strncpy(out->focal_length,in->FocalLength,sizeof(out->focal_length)-1);
	strncpy(out->max_apeture_val,in->MaxApertureValue,sizeof(out->max_apeture_val)-1);
	strncpy(out->meter_mode,in->MeteringMode,sizeof(out->meter_mode)-1);
	strncpy(out->flashlight_on,in->Flash,sizeof(out->flashlight_on)-1);
	
}

static void audio_media_info_t_convert(audio_media_info_t *in,audio_info_t *out)
{
	if(!in || !out)
	{
		DMCLOG_E("param error");
		return;
	}
	out->duration_time = in->duration;
	out->bitrate = in->bit_rate;
	strncpy(out->album_name,in->album,sizeof(out->album_name)-1);
	strncpy(out->artist_name,in->artist,sizeof(out->artist_name)-1);
	strncpy(out->encode_type,in->encode,sizeof(out->encode_type)-1);
	strncpy(out->song_name,in->name,sizeof(out->song_name)-1);
}

static void video_media_info_t_convert(video_media_info_t *in,video_info_t *out)
{
	if(!in || !out)
	{
		DMCLOG_E("param error");
		return;
	}
	out->bitrate = in->bit_rate;
	out->duration_time = in->duration;
	out->height = in->height;
	out->width = in->width;
	strncpy(out->encode_type,in->encode,sizeof(out->encode_type)-1);
}


static int image_prc(const char *file_path)
{	
	int ret = -1;
	char *middle_image_realpath = NULL;
	char *small_image_realpath = NULL;
	ERROR_CODE_VFS vfs_ret;
	unsigned char quality = 60;
	image_media_info_t *pImageMediaIn = NULL;
	image_info_t ImageMediaOut;
	char middle_image_path[RFSVFS_PATH_LENGTH_DEFAULT+1];
	char small_image_path[RFSVFS_PATH_LENGTH_DEFAULT+1];
	ImgReduceRequest *request_array[3] = {0};
	ImgReduceRequest request_middle, request_small;	
	memset(&request_middle, 0, sizeof(ImgReduceRequest));
	memset(&request_small, 0, sizeof(ImgReduceRequest));
	memset(middle_image_path,0,sizeof(middle_image_path));
	memset(small_image_path,0,sizeof(small_image_path));
	DMCLOG_D("file_path:%s",file_path);
	char *real_path = calloc(1,strlen(file_path)+DISK_FULLPATH_LEN);
	if(!real_path)
	{
		DMCLOG_E("real_path calloc fail");
		goto exit;
	}
	strcpy(real_path,DISK_FULLPATH);
	strcat(real_path,file_path);
	DMCLOG_D("real_path:%s",real_path);
	if(afvfs_get_new_file_path(middle_image_path) != 0)
	{
		DMCLOG_E("afvfs_get_new_file_path fail");
		goto exit;
	}
	if(afvfs_get_new_file_path(small_image_path) != 0)
	{
		DMCLOG_E("afvfs_get_new_file_path fail");
		goto exit;
	}
	DMCLOG_D("middle_image_path:%s",middle_image_path);
	DMCLOG_D("small_image_path:%s",small_image_path);
	middle_image_realpath = calloc(1,strlen(middle_image_path)+DISK_FULLPATH_LEN);
	small_image_realpath = calloc(1,strlen(small_image_path)+DISK_FULLPATH_LEN);
	if(!middle_image_realpath || !small_image_realpath)
	{
		DMCLOG_E("calloc fail");
		goto exit;
	}
	strcpy(middle_image_realpath,DISK_FULLPATH);
	strcat(middle_image_realpath,middle_image_path);
	strcpy(small_image_realpath,DISK_FULLPATH);
	strcat(small_image_realpath,small_image_path);
	DMCLOG_D("middle_image_realpath:%s",middle_image_realpath);
	DMCLOG_D("small_image_realpath:%s",small_image_realpath);
	request_middle.width = 640;
	request_middle.height = 480;
	request_middle.is_accurate = 1;
	request_middle.scale = 0;
	request_middle.quality = quality;
	strncpy(request_middle.img_name,middle_image_realpath,sizeof(request_middle.img_name));
	request_small.width = 320;
	request_small.height = 240;
	request_small.is_accurate = 1;
	request_small.scale = 0;
	request_small.quality = quality;
	strncpy(request_small.img_name,small_image_realpath,sizeof(request_small.img_name));
	request_array[0] = &request_middle;
	request_array[1] = &request_small;
	if(reduce_image(real_path, &request_array[0]) != 0)
	{
		DMCLOG_E("reduce_image fail (%s)",file_path);
		goto exit;
	}
	vfs_ret = afvfs_set_thumbnail_small(file_path,small_image_path);
	if(vfs_ret != 0)
	{
		DMCLOG_E("afvfs_set_thumbnail_small fail (%d)",vfs_ret);
		goto exit;
	}
	vfs_ret = afvfs_set_thumbnail_median(file_path,middle_image_path);
	if(vfs_ret != 0)
	{
		DMCLOG_E("afvfs_set_thumbnail_median fail (%d)",vfs_ret);
		goto exit;
	}
	if (get_image_media_info(real_path, &pImageMediaIn) == 0) 
	{
		DMCLOG_D("ApertureValue = %s", pImageMediaIn->ApertureValue);
		DMCLOG_D("AMaxApertureValue = %s",pImageMediaIn->MaxApertureValue);
		DMCLOG_D("CameraType = %s",pImageMediaIn->CameraType);
		DMCLOG_D("ExposureBiasValue = %s",pImageMediaIn->ExposureBiasValue);
		DMCLOG_D("ExposureTime = %s",pImageMediaIn->ExposureTime);
		DMCLOG_D("Flash = %s",pImageMediaIn->Flash);
		DMCLOG_D("FocalLength = %s",pImageMediaIn->FocalLength);
		DMCLOG_D("MeteringMode = %s",pImageMediaIn->MeteringMode);
		DMCLOG_D("DateTime = %s",pImageMediaIn->DateTime);
		DMCLOG_D("Make = %s",pImageMediaIn->Make);
		DMCLOG_D("ISOSpeedRatings = %s",pImageMediaIn->ISOSpeedRatings);
		DMCLOG_D("XResolution = %d",pImageMediaIn->XResolution);
		DMCLOG_D("YResolution = %d",pImageMediaIn->YResolution);
		memset(&ImageMediaOut,0,sizeof(image_info_t));
		image_media_info_t_convert(pImageMediaIn,&ImageMediaOut);
		if(afvfs_set_image_media_info(file_path,&ImageMediaOut) != 0)
		{
			DMCLOG_E("afvfs_set_image_media_info fail");
			free_image_media_info(&pImageMediaIn);
			goto exit;
		}
		free_image_media_info(&pImageMediaIn);
	} 
	else 
	{
		DMCLOG_E("can't get image media info");
		goto exit;
	}
	ret = 0;
exit:
	safe_free(real_path);
	safe_free(small_image_realpath);
	safe_free(middle_image_realpath);
	return ret;
}


static int audio_prc(const char *file_path)
{	
	int ret = -1;
	ERROR_CODE_VFS vfs_ret;
	OptionDmContext dm_context;
	audio_media_info_t *pAudioMediaIn = NULL;
	audio_info_t AudioMediaOut;
	char image_path[RFSVFS_PATH_LENGTH_DEFAULT+1];
	char *output_real_path = NULL;
	char *input_real_path = calloc(1,strlen(file_path)+DISK_FULLPATH_LEN);
	if(!input_real_path)
	{
		DMCLOG_E("input_real_path calloc fail");
		goto exit;
	}
	strcpy(input_real_path,DISK_FULLPATH);
	strcat(input_real_path,file_path);
	DMCLOG_D("input_real_path:%s",input_real_path);
	memset(image_path,0,sizeof(image_path));
	if(afvfs_get_new_file_path(image_path) != 0)
	{
		DMCLOG_E("afvfs_get_new_file_path fail");
		goto exit;
	}
	DMCLOG_D("image_path:%s",image_path);
	output_real_path = calloc(1,strlen(image_path)+DISK_FULLPATH_LEN);
	if(!output_real_path)
	{
		DMCLOG_E("output_real_path calloc fail");
		goto exit;
	}
	strcpy(output_real_path,DISK_FULLPATH);
	strcat(output_real_path,image_path);
	DMCLOG_D("output_real_path:%s",output_real_path);
	memset(&dm_context, 0, sizeof(OptionDmContext));
	dm_context.input_file_name = input_real_path;
	dm_context.output_file_name = output_real_path;
	if(media_snapshot(&dm_context) < 0)
	{
		DMCLOG_E("ffmpeg_creat_thumbnail fail");
		goto exit;
	}
	vfs_ret = afvfs_set_thumbnail_median(file_path,image_path);
	if(vfs_ret != 0)
	{
		DMCLOG_E("afvfs_set_thumbnail_median fail (%d)",vfs_ret);
		goto exit;
	}
	if (get_audio_media_info(input_real_path, &pAudioMediaIn) == 0) 
	{
		DMCLOG_D("title=%s\nartist=%s\nalbum=%s\ncodec=%s\nbit_rate=%d\nduration:%"PRId64"", 
			pAudioMediaIn->name, pAudioMediaIn->artist, pAudioMediaIn->album, 
			pAudioMediaIn->encode, pAudioMediaIn->bit_rate, pAudioMediaIn->duration);
		memset(&AudioMediaOut,0,sizeof(audio_info_t));
		audio_media_info_t_convert(pAudioMediaIn,&AudioMediaOut);
		if(afvfs_set_audio_media_info(file_path,&AudioMediaOut) != 0)
		{
			DMCLOG_E("afvfs_set_audio_media_info fail");
			free_audio_media_info(&pAudioMediaIn);
			goto exit;
		}
		free_audio_media_info(&pAudioMediaIn);
	}
	else
	{
		DMCLOG_E("can't get audio media info");
		goto exit;
	}
	ret = 0;
exit:
	safe_free(input_real_path);
	safe_free(output_real_path);
	return ret;
}

static int video_prc(const char *file_path)
{	
	int ret = -1;
	ERROR_CODE_VFS vfs_ret;
	OptionDmContext dm_context;
	video_media_info_t *pVideoMediaIn = NULL;
	video_info_t VideoMediaOut;
	char image_path[RFSVFS_PATH_LENGTH_DEFAULT+1];
	char *output_real_path = NULL;
	char *input_real_path = calloc(1,strlen(file_path)+DISK_FULLPATH_LEN);
	if(!input_real_path)
	{
		DMCLOG_E("input_real_path calloc fail");
		goto exit;
	}
	strcpy(input_real_path,DISK_FULLPATH);
	strcat(input_real_path,file_path);
	DMCLOG_D("input_real_path:%s",input_real_path);
	memset(image_path,0,sizeof(image_path));
	if(afvfs_get_new_file_path(image_path) != 0)
	{
		DMCLOG_E("afvfs_get_new_file_path fail");
		goto exit;
	}
	DMCLOG_D("image_path:%s",image_path);
	output_real_path = calloc(1,strlen(image_path)+DISK_FULLPATH_LEN);
	if(!output_real_path)
	{
		DMCLOG_E("output_real_path calloc fail");
		goto exit;
	}
	strcpy(output_real_path,DISK_FULLPATH);
	strcat(output_real_path,image_path);
	DMCLOG_D("output_real_path:%s",output_real_path);
	memset(&dm_context, 0, sizeof(OptionDmContext));
	strcpy(dm_context.seek_proportion, "20");
	dm_context.input_file_name = input_real_path;
	dm_context.output_file_name = output_real_path;
	if(media_snapshot(&dm_context) < 0)
	{
		DMCLOG_E("ffmpeg_creat_thumbnail fail");
		goto exit;
	}
	vfs_ret = afvfs_set_thumbnail_median(file_path,image_path);
	if(vfs_ret != 0)
	{
		DMCLOG_E("afvfs_set_thumbnail_median fail (%d)",vfs_ret);
		goto exit;
	}
	if (get_video_media_info(input_real_path, &pVideoMediaIn) == 0) 
	{
		DMCLOG_D("weight:%d height:%d bit_rate:%d codec_name:%s duration:%"PRId64"", 
			pVideoMediaIn->width, pVideoMediaIn->height, pVideoMediaIn->bit_rate,
			pVideoMediaIn->encode, pVideoMediaIn->duration);
		memset(&VideoMediaOut,0,sizeof(video_info_t));
		video_media_info_t_convert(pVideoMediaIn,&VideoMediaOut);
		if(afvfs_set_video_media_info(file_path,&VideoMediaOut) != 0)
		{
			DMCLOG_E("afvfs_set_video_media_info fail");
			free_video_media_info(&pVideoMediaIn);
			goto exit;
		}
		free_video_media_info(&pVideoMediaIn);
	}
	else 
	{
		DMCLOG_E("can't get video media info");
		goto exit;
	}
	ret = 0;
exit:
	safe_free(input_real_path);
	safe_free(output_real_path);
	return ret;
}



//media process api
static int media_prc(const char *file_path,const int media_type)
{
	int ret = -1;
	switch(media_type)
	{
		case TYPE_IMAGE:
			DMCLOG_D("media_type is image");
			if(image_prc(file_path) != 0)
			{
				DMCLOG_E("image_prc fail (%s)",file_path);
				goto exit;
			}
			break;
		case TYPE_AUDIO:
			DMCLOG_D("media_type is audio");
			if(audio_prc(file_path) != 0)
			{
				DMCLOG_E("audio_prc fail (%s)",file_path);
				goto exit;
			}
			break;
		case TYPE_VIDEO:
			DMCLOG_D("media_type is video");
			if(video_prc(file_path) != 0)
			{
				DMCLOG_E("video_prc fail (%s)",file_path);
				goto exit;
			}
			break;
		default:
			DMCLOG_E("unknown media type: (%d)!", media_type);
			goto exit;
	}
	ret = 0;
exit:
	return ret;
}

int media_prc_thpool_cb(void* arg_p)
{
	int ret = -1;
	DMCLOG_D("Thread #%u working on media_prc_thpool_cb", (int)pthread_self());
	media_dnode_t *pdata = (media_dnode_t *)arg_p;
	if(!pdata)
	{
		DMCLOG_E("pdata is null");
		goto exit;
	}
	if(media_prc(pdata->path,pdata->media_type) != 0)
	{
		DMCLOG_E("media_prc fail");
		goto exit;
	}
	ret = 0;
exit:
	media_dnode_free(pdata);
	return ret;
}


int media_prc_thpool_init(void)
{
	int ret = -1;
	media_prc_thpool = thpool_init(MAX_MEDIA_PROCESS_THREAD_NUM);
	if(media_prc_thpool == NULL)
	{
		DMCLOG_E("thpool_init fail");
		goto exit;
	}
	ret = 0;
exit:
	return ret;
}


void media_prc_thpool_destroy(void)
{
	if(media_prc_thpool != NULL)
	{
		thpool_destroy(media_prc_thpool);
	}
	media_prc_thpool = NULL;
	return;
}

int media_prc_thpool_add_work(int (*function_p)(void*), void* arg_p)
{
	int ret = -1;
	if(media_prc_thpool == NULL)
	{
		DMCLOG_E("media_prc_thpool uninit");
		goto exit;
	}
	if(thpool_add_work(media_prc_thpool,function_p,arg_p) != 0)
	{
		DMCLOG_E("thpool_add_work fail");
		goto exit;
	}
	ret = 0;
exit:
	return ret;
}


