#include "media_table.h"



static int get_media_id_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
	int *pid = (int *)data;

	if(FieldValue[0] != NULL)
	{
		*pid = strtoul(FieldValue[0], NULL, 10);
	}
	return 0;
}


error_t get_media_max_id(sqlite3 *database,uint32_t *max_id,char *media_table)
{
	error_t errcode;
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql,"SELECT MAX(ID) FROM %s",media_table);
	if((errcode = sqlite3_exec_busy_wait(database,sql,get_media_id_callback, max_id)) != SQLITE_OK)
	{
		DMCLOG_D("sqlite3_exec_busy_wait error");
        sqlite3_close(database);
        return errcode;
	}
	return errcode;
}

static inline void load_video_item(char **FieldName, char **FieldValue, int nFields, video_info_t *pvi)
{
    if(FieldValue[0] != NULL)
		pvi->index = strtoul(FieldValue[0], NULL, 10);
    if(FieldValue[1] != NULL)
    	pvi->width = atoi(FieldValue[1]);
    if(FieldValue[2] != NULL)
	    pvi->height = atoi(FieldValue[2]);
    if(FieldValue[3] != NULL)
	    pvi->bitrate = strtoul(FieldValue[3], NULL, 10);
    if(FieldValue[4] != NULL)
    	S_STRNCPY(pvi->encode_type,FieldValue[4], MAX_ENCODE_TYPE_SIZE);
    if(FieldValue[5] != NULL)
	    pvi->duration_time = strtoul(FieldValue[5], NULL, 10);
	
}

static inline void load_thum_item(char **FieldName, char **FieldValue, int nFields, thum_info_t *pvi)
{
    if(FieldValue[0] != NULL)
		pvi->index = strtoul(FieldValue[0], NULL, 10);
	 if(FieldValue[1] != NULL)
    	S_STRNCPY(pvi->small_path,FieldValue[1], MAX_FILE_PATH_LEN);
    if(FieldValue[2] != NULL)
    	S_STRNCPY(pvi->median_path,FieldValue[2], MAX_FILE_PATH_LEN);
	
}




static inline void load_audio_item(char **FieldName, char **FieldValue, int nFields, audio_info_t *pai)
{
	if(FieldValue[0] != NULL)
		pai->index  = strtoul(FieldValue[0], NULL, 10);
    if(FieldValue[1] != NULL)
	    pai->duration_time = strtoul(FieldValue[1], NULL, 10);
    if(FieldValue[2] != NULL)
	    S_STRNCPY(pai->song_name,   FieldValue[2], MAX_SONG_NAME_SIZE);
    if(FieldValue[3] != NULL)
	    S_STRNCPY(pai->album_name,  FieldValue[3], MAX_ALBUM_NAME_SIZE);
    if(FieldValue[4] != NULL)
	    S_STRNCPY(pai->artist_name, FieldValue[4], MAX_ARTIST_NAME_SIZE);
    if(FieldValue[5] != NULL)
	    S_STRNCPY(pai->encode_type, FieldValue[5], MAX_ENCODE_TYPE_SIZE);
    if(FieldValue[6] != NULL)
        pai->bitrate = strtoul(FieldValue[6], NULL, 10);

}


static inline void load_image_item(char **FieldName, char **FieldValue, int nFields, image_info_t *pii)
{
	if(FieldValue[0] != NULL)
	    pii->index = strtoul(FieldValue[0], NULL, 10);
    if(FieldValue[1] != NULL)
	    pii->width    =  atoi(FieldValue[1]);
    if(FieldValue[2] != NULL)
	    pii->height   =  atoi(FieldValue[2]);
    if(FieldValue[3] != NULL) 
	    pii->photo_time =  strtoull(FieldValue[3], NULL, 10);
    if(FieldValue[4] != NULL)
	    S_STRNCPY(pii->vendor_name, FieldValue[4], MAX_VENDOR_NAME_SIZE);
    if(FieldValue[5] != NULL)
	    S_STRNCPY(pii->camera_type, FieldValue[5], MAX_CAMERA_TYPE_NAME_SIZE);
    if(FieldValue[6] != NULL)
	    S_STRNCPY(pii->aperture_value, FieldValue[6], MAX_APETURE_VALUE_SIZE);
    if(FieldValue[7] != NULL)
		S_STRNCPY(pii->exposure_time, FieldValue[7], EXPOSURE_TIME_VALUE_SIZE);
    if(FieldValue[8] != NULL)
		S_STRNCPY(pii->iso_value, FieldValue[8], ISO_VALUE_VALUE_SIZE);
    if(FieldValue[9] != NULL)
		S_STRNCPY(pii->exposure_bias_val, FieldValue[9], EXPOSURE_BIAS_VALUE_SIZE);
    if(FieldValue[10] != NULL)
		S_STRNCPY(pii->focal_length, FieldValue[10], FOCAL_LENGTH);
    if(FieldValue[11] != NULL)
		S_STRNCPY(pii->max_apeture_val, FieldValue[11], MAX_APETURE_VALUE_SIZE);
    if(FieldValue[12] != NULL)
		S_STRNCPY(pii->meter_mode, FieldValue[12], METER_MODE_SIZE);
	if(FieldValue[13] != NULL) 
		S_STRNCPY(pii->flashlight_on, FieldValue[13], FLASHLIGHT_ON_SIZE);
}


static int  thum_query_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
    thum_info_t *pvi = (thum_info_t *)data;

	if(pvi == NULL)
	{
		return 1;
	}

	load_thum_item(FieldName, FieldValue, nFields, pvi);
	
	return 0;
	
}

static int  video_query_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
    video_info_t *pvi = (video_info_t *)data;

	if(pvi == NULL)
	{
		return 1;
	}

	load_video_item(FieldName, FieldValue, nFields, pvi);
	
	return 0;
	
}


static int  audio_query_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
    audio_info_t *pai = (audio_info_t *)data;

	if(pai == NULL)
	{
		return 1;
	}

	load_audio_item(FieldName, FieldValue, nFields, pai);
	
	return 0;
}

static int  image_query_callback(void *data, int nFields, char **FieldValue, char **FieldName)
{
    image_info_t *pii = (image_info_t *)data;

	if(pii == NULL)
	{
		return 1;
	}

	load_image_item(FieldName, FieldValue, nFields, pii);

	return 0;
}


error_t load_audio_query_cmd(sqlite3 *database,audio_info_t *paudio)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "SELECT * FROM %s where ID = '%d'", AUDIO_TABLE_NAME, paudio->index);
	return sqlite3_exec_busy_wait(database, sql, audio_query_callback, paudio);
}

error_t load_image_query_cmd(sqlite3 *database,image_info_t *pimage)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "SELECT * FROM %s where ID = '%d'", IMAGE_TABLE_NAME, pimage->index);
	return sqlite3_exec_busy_wait(database, sql, image_query_callback, pimage);
}

error_t load_video_query_cmd(sqlite3 *database,video_info_t *pvideo)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "SELECT * FROM %s where ID = '%d'", VIDEO_TABLE_NAME, pvideo->index);
	return sqlite3_exec_busy_wait(database, sql, video_query_callback, pvideo);
}

error_t load_thum_query_cmd(sqlite3 *database,thum_info_t *pthum)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "SELECT * FROM %s where ID = '%d'", THUM_TABLE_NAME, pthum->index);
	return sqlite3_exec_busy_wait(database, sql, thum_query_callback, pthum);
}



error_t load_audio_update_cmd(sqlite3 *database,audio_info_t *paudio)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "UPDATE %s SET DURATION='%lld',ALBUM='%s',ARTIST ='%s',ENCODE = '%s',BITRATE='%d' WHERE ID='%d'", \
		AUDIO_TABLE_NAME, paudio->duration_time,paudio->album_name,paudio->artist_name,paudio->encode_type,paudio->bitrate,paudio->index);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

error_t load_image_update_cmd(sqlite3 *database,image_info_t *pimage)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "UPDATE %s SET WIDTH='%d',HEIGHT='%d',TIME ='%ld',VENDOR_NAME = '%s',CAMERA_TYPE='%s',APERTURE_VALUE = '%s'\
		EXPOSURE_TIME='%s', ISO_VAL='%s', EXPOSURE_BIAS_VAL='%s', FOCAL_LENGTH='%s', MAX_APETURE_VAL='%s', METER_MODE ,FLASH='%s' WHERE ID='%d'", \
		IMAGE_TABLE_NAME, pimage->width,pimage->height,pimage->photo_time,pimage->vendor_name,pimage->camera_type,pimage->aperture_value\
		,pimage->exposure_time,pimage->iso_value,pimage->exposure_bias_val,pimage->focal_length,pimage->max_apeture_val,pimage->meter_mode,pimage->flashlight_on,
		pimage->index);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

error_t load_video_update_cmd(sqlite3 *database,video_info_t *pvideo)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
	sprintf(sql, "UPDATE %s SET WIDTH='%d',HEIGHT='%d',BITRATE ='%d',ENCODE = '%s',DURATION='%lld' WHERE ID='%d'", \
		VIDEO_TABLE_NAME,pvideo->width,pvideo->height,pvideo->bitrate,pvideo->encode_type,pvideo->duration_time,pvideo->index);
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

error_t load_video_insert_cmd(sqlite3 *database, video_info_t *pvi)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
    int n = snprintf(sql, SQL_CMD_WRITE_BUF_SIZE, "INSERT INTO %s(WIDTH,HEIGHT,BITRATE,ENCODE,DURATION) "\
			"VALUES(%ld,%ld,%ld,'%s',%lld)", VIDEO_TABLE_NAME,
			pvi->width, pvi->height, pvi->bitrate, pvi->encode_type,pvi->duration_time);
	if(n >= SQL_CMD_WRITE_BUF_SIZE)
	{
	    DMCLOG_E("load video insert cmd buffer overflow:%s",sql);
    	return EOUTBOUND;
	}

	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

error_t load_audio_insert_cmd(sqlite3 *database,audio_info_t *pai)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
    int n = snprintf(sql, SQL_CMD_WRITE_BUF_SIZE, "INSERT INTO %s(DURATION,NAME,ALBUM,ARTIST,ENCODE,BITRATE) "\
			"VALUES(%lld,'%s','%s','%s','%s',%ld)", AUDIO_TABLE_NAME, 
			pai->duration_time, pai->song_name, pai->album_name, pai->artist_name,
			pai->encode_type, pai->bitrate);

	if(n >= SQL_CMD_WRITE_BUF_SIZE)
	{
	    DMCLOG_E("load audio insert cmd buffer overflow:%s",sql);
    	return EOUTBOUND;
	}

	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}


error_t load_image_insert_cmd(sqlite3 *database, image_info_t *pii)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
    int n = snprintf(sql, SQL_CMD_WRITE_BUF_SIZE, "INSERT INTO %s(WIDTH,HEIGHT,TIME,VENDOR_NAME,CAMERA_TYPE,"\
		"APERTURE_VALUE,EXPOSURE_TIME,ISO_VAL,EXPOSURE_BIAS_VAL,FOCAL_LENGTH,"\
	    "MAX_APETURE_VAL,METER_MODE,FLASH) "\
		"VALUES(%d,%d,%ld,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",IMAGE_TABLE_NAME,
		pii->width, pii->height, pii->photo_time, pii->vendor_name,
			pii->camera_type, pii->aperture_value, pii->exposure_time, pii->iso_value,
			pii->exposure_bias_val, pii->focal_length, pii->max_apeture_val, pii->meter_mode,
			pii->flashlight_on);

	if(n >= SQL_CMD_WRITE_BUF_SIZE)
	{
	    DMCLOG_E("load image insert cmd buffer overflow:%s",sql);
    	return EOUTBOUND;
	}
	
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}

error_t load_thum_insert_cmd(sqlite3 *database, thum_info_t *pii)
{
	char sql[SQL_CMD_WRITE_BUF_SIZE] = {0};
    int n = snprintf(sql, SQL_CMD_WRITE_BUF_SIZE, "INSERT INTO %s(SMALL_PATH,MEDIAN_PATH)"\
		"VALUES('%s','%s')",THUM_TABLE_NAME,pii->small_path,pii->median_path);

	if(n >= SQL_CMD_WRITE_BUF_SIZE)
	{
	    DMCLOG_E("load small thum insert cmd buffer overflow:%s",sql);
    	return EOUTBOUND;
	}
	
	return sqlite3_exec_busy_wait(database, sql, NULL, NULL);
}







