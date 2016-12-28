#ifndef __CAMERA_H
#define __CAMERA_H
#include <gphoto2/gphoto2-list.h>

#include "file_json.h"
#include "camera_detect.h"
#include "commucation.h"
#include "entryinfo.h"
enum EntityType{
	ET_FOLDER = 0,
	ET_FILE = 1,
};

enum ADDEXIFFLAG {
	EXIF_GETEXIF = 0,	//add all available exif data to json
	EXIF_GETFILELIST= 1,	//only add those exif-data that listfile command need
};

#define DEBG(level, format, arg...)

/*
enum EntityType{
	ET_FOLDER = 0,
	ET_FILE = 1,
};


#define SDKVERSION "1.0.0"



#define MAX_FILE_PATH 512



struct CameraInfo {
	char manufacturer[30];
	char model[20];
	char version[20];
	char serial_number[30];
	char capture_format[20];
	char storage_type[20];
	char access_cap[20];
	char max_space[30];
	char free_space[30];	
};

struct EntryInfoList {
	struct EntryInfo* eis;
	int count;
	int malloc_cnt;
};

struct EntryInfo {
	char path[512];
};



struct APCameraFileInfo {
	char type[64];
	int permissions;
	unsigned long long size;
	unsigned long long mtime;
	unsigned int width;
	unsigned int height;
	
};

struct ExifInfo {
	char shoot_date[32];
	char exposure_time[20];
	char iso_speed_ratings[20];
	char exposure_bias_value[32];
	char manufacturer[30];
	char modle[20];
	char fnmuber[20];
	char focal_length[20];
	char metering_mode[20];
};
struct APCameraFile {
	unsigned char* data;
	unsigned long long size;
};


enum APCamFileType {
	APCAMFT_NORMAL = 0,
	APCAMFT_THUMB = 1
};


int list_files(char* folder, int recursive, cJSON* filearray);
int list_folders(char* folder, int recursive, cJSON* folderarray);
struct APCameraFile *get_camera_file_normal(char* path, int *errcode);
int get_camera_file_thumb(char* path, struct common_args *pargs, struct my_buffer* p_buffer);
int free_entry_info(struct EntryInfo * eis);
int get_exif(char* path, struct ExifInfo* exif);
int free_camera_file(struct APCameraFile* camfile);
int get_camera_file_info(char* path, struct APCameraFileInfo* fileinfo);
int init_airphoto_sdk (char **envp);
int get_camera_info(struct CameraInfo* caminfo);
int read_camera_file_normal(char* path, unsigned long long offset, unsigned long long *size, unsigned char*	data);
int read_camera_file_thumb(char* path, unsigned long long offset, unsigned long long *size, unsigned char* data);
int free_entrylist(struct EntryInfoList * eil);
int delete_file(char* path);
int check_airphoto(int* status);
int detect_camera ();
*/

int cam_list_folders(char* folder, int recursive, JObj *data_json);
int cam_list_files(char* folder, int recursive, JObj *data_json);
int cam_get_exif(char* path, JObj* p_jdata);
int cam_get_cam_info(JObj *data_json);
int cam_del_file(char* path);
int cam_detect_camera ();
int cam_read_file(char* path, unsigned long long offset, unsigned long long size, unsigned char* data);
int cam_get_thumb_file(char* path, unsigned char** thumb_data);
int cam_get_file_len(char* path);




#endif
