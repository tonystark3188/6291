#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>


#include "camera.h"
#include "gp-params.h"
#include <libexif/exif-data.h>
#include <gphoto2/gphoto2-port-info-list.h>
#include <gphoto2/gphoto2-port-log.h>
#include <sys/time.h>
#include "file_json.h"
#include "debuglog.h"

static pthread_mutex_t g_camera_mutex;
#define CAMERA_LOCK	pthread_mutex_lock(&g_camera_mutex)
#define CAMERA_UNLOCK	pthread_mutex_unlock(&g_camera_mutex)
#define CAMERA_LINIT	pthread_mutex_init(&g_camera_mutex, NULL)

static int get_files_range(char* folder, JObj* filearray, int start_index, int end_index, int* total_num);



/*static struct timeval glob_tv_zero = { 0, 0 };
static void debug_func (GPLogLevel level, const char *domain, const char *str, void *data)
{
	struct timeval tv;
	long sec, usec;
	FILE *logfile = (data != NULL)?(FILE *)data:stderr;

	gettimeofday (&tv,NULL);
	sec = tv.tv_sec  - glob_tv_zero.tv_sec;
	usec = tv.tv_usec - glob_tv_zero.tv_usec;
	if (usec < 0) {sec--; usec += 1000000L;}
	fprintf (logfile, "%li.%06li %-28s(%i): %s\n", sec, usec, domain, level, str);
}
*/
static int g_initsdk_flag = 0;
static struct _GPParams g_gp_params;
int init_airphoto_sdk (char **envp)
{
	if (g_initsdk_flag == 0) {
		CAMERA_LINIT;
		init_cam_detect_mutex();
	}
	g_initsdk_flag = 1;
	struct _GPParams *p = &g_gp_params;
	if (!p)
		return -1;

	memset (p, 0, sizeof (GPParams));

	p->folder = strdup ("/");
	if (!p->folder) {
		fprintf (stderr, "Not enough memory.");
		fputc ('\n', stderr);
		exit (1);
	}

	gp_camera_new (&p->camera);

	p->cols = 79;
	p->flags = FLAGS_RECURSE;

	/* Create a context. Report progress only if users will see it. */
	p->context = gp_context_new ();
	//p->debug_func_id = gp_log_add_func (GP_LOG_ALL, debug_func, (void *) stdout);
	p->_abilities_list = NULL;

	p->debug_func_id = -1;

	p->envp = envp;
	return 0;
}


static int extract_content(CameraText *a, char* b, char* str)
{
	char *c;
	c = strstr((char*)a, b);
	int i = 0;
	if (c != NULL) 
		c = strstr(c, ": ");		
	else 
		return -1;
	c= c + 2;
	while(c[i] != '\n' && i < 99) {
		str[i] = c[i];
		++i;
	}
	str[i] = '\0';
	return 0;
}

static int add_camera_info_to_json(CameraText *text, JObj *data_json)
{
	char str[100] = {0};
//	JObj *j_para = NULL;

	extract_content(text, "Manufacturer", str);
	JSON_ADD_OBJECT(data_json, "Manufacturer",JSON_NEW_OBJECT(str,string));

	extract_content(text, "Model", str);
	JSON_ADD_OBJECT(data_json, "Model",JSON_NEW_OBJECT(str,string));

	extract_content(text, "Version", str);
	JSON_ADD_OBJECT(data_json, "Version",JSON_NEW_OBJECT(str,string));

	extract_content(text, "Serial Number", str);
	JSON_ADD_OBJECT(data_json, "Serial Number",JSON_NEW_OBJECT(str,string));

	extract_content(text, "Capture Formats", str);
	JSON_ADD_OBJECT(data_json, "Capture Formats",JSON_NEW_OBJECT(str,string));


	extract_content(text, "Storage Type", str);
	JSON_ADD_OBJECT(data_json, "Storage Type",JSON_NEW_OBJECT(str,string));

	extract_content(text, "Access Capability", str);
	JSON_ADD_OBJECT(data_json, "Access Capability",JSON_NEW_OBJECT(str,string));

	extract_content(text, "Maximum Capability", str);
	JSON_ADD_OBJECT(data_json, "Maximum Capability",JSON_NEW_OBJECT(str,string));

	extract_content(text, "Free Space", str);
	JSON_ADD_OBJECT(data_json, "Free Space",JSON_NEW_OBJECT(str,string));

	return 0;
}


/*****************************************
* 参数:
struct camera_info* caminfo   --相机信息的结构体的指针
* 返回: 
成功，返回0 ;失败，返回错误码
* 说明：
获取相机信息，可用于检测相机是否已连接。
*****************************************/
int cam_get_cam_info(JObj *data_json)
{
	int ret = 0;
	CameraText text;
	
	CAMERA_LOCK;
	DMCLOG_D ("get camera info from camera ...");	
	CK00 (ret = gp_camera_get_summary(g_gp_params.camera, &text, g_gp_params.context), err_no);
	CK02 (ret = add_camera_info_to_json(&text, data_json),err_no);
	
	CAMERA_UNLOCK;
err_no:
	return ret;
}

static int get_name_offset(char *path, int* offset)
{
	int ret = 0;
//	debug(0, "spliting path: name and folder\n");
	int i  =0;
	for (i = strlen(path)-1; i>=0; --i) {
		if (path[i] == '/') 
			break;
	}
	if (i < 0||i == strlen(path)-1) {
	//	debug(0, "[is not a valid path]\n");
		ret = -1;
		goto err_no;
	}
	*offset = i+1;
	return ret;
err_no:
	*offset = 0;
	return ret;
}

static int split_path_folder(char*path, char **folder, char** name)
{
	int offset = 0;
	int	ret = 0;
	CK00 (ret = get_name_offset(path, &offset), err_no);
	*name = strdup(path+offset);
	*folder = strdup(path);
	(*folder)[offset] = '\0';	
	//DMCLOG_D("folder = %s,name = %s", *folder, *name);
	return 0;
err_no:
	return ret;
}

//ppfile must be free by caller if succeed
static int open_and_read_file(char* path, CameraFile** ppfile, const char **data, unsigned long *size, CameraFileType type )
{	
	int ret = 0;
	
	char* folder = NULL;
	char* name = NULL;
	CK00 (ret = split_path_folder(path, &folder, &name), err_no);
	
	CK00 (ret = gp_file_new (ppfile), err_filenew_path);
	
	CameraFile* p_file = *ppfile;
	CK00 (ret = gp_camera_file_get (g_gp_params.camera, folder, name,
			 type, p_file, g_gp_params.context), err_fileget_path);
	
	CK00 (ret = gp_file_get_data_and_size (p_file, data, size), err_filedata_file);

	free(name);
	free(folder);

	return 0;

err_filedata_file:
	//DEBG (0, "[error_msg][gp_file_get_data_and_size error!!!][error_code=%d]\n", ret);
	goto err_file;		

err_fileget_path:
	//DEBG (0, "[error_msg][gp_camera_file_get error!!!][error_code=%d]\n", ret);
	goto err_path;	

err_filenew_path:
	//DEBG (0, "[error_msg][gp_file_new error!!!][error_code=%d]\n", ret);
	goto err_path;

err_file:
	gp_file_unref(p_file);
err_path:
	free(name);
	free(folder);
err_no:
	return ret;
}

#define EXIFINFO_SIZE 128

static inline int fill_exif_info2(ExifEntry *e, JObj* p_jdata)
{
	const char* tag = exif_tag_get_name (e->tag);
	char* exifval = NULL;
	CK20(exifval = (char*)malloc(EXIFINFO_SIZE), err_malloc);
	exif_entry_get_value (e, exifval, EXIFINFO_SIZE-1);
	JSON_ADD_OBJECT(p_jdata,tag,JSON_NEW_OBJECT(exifval,string));
	DMCLOG_D("find new exif info,tag = %s,val = %s", tag, exifval);	
	safe_free(exifval);
	return 0;
err_malloc:
	DMCLOG_E("out of memory");
	return -1;
}
static int fill_exif_info(ExifData *ed, JObj* p_jdata, enum ADDEXIFFLAG exifflag)
{
	int i, k=0;
	ExifByteOrder byteOrder;
	ExifEntry *exifEntry;
	int orientation = 0;
	byteOrder = exif_data_get_byte_order(ed);
	exifEntry = exif_data_get_entry(ed, EXIF_TAG_ORIENTATION);
	if (exifEntry)
		orientation = exif_get_short(exifEntry->data, byteOrder);
	JSON_ADD_OBJECT(p_jdata,"Orient",JSON_NEW_OBJECT(orientation,int));
	switch(exifflag) {
		case EXIF_GETFILELIST:  			
			goto out;
		case EXIF_GETEXIF:
			break;
		default:
			DMCLOG_E("no such type:%d", exifflag);
			goto err_no;
	}
	ExifContent *content;
//	int size = 0;
	for (i = 0; i < EXIF_IFD_COUNT; i++) {
		content = ed->ifd[i];
		if (content) {
	 		ExifEntry *e;
		    unsigned int j;
		    for (j = 0; j < content->count; j++) {
	            e = content->entries[j];
				CK00 (fill_exif_info2(e, p_jdata), err_no);
			}
			
		}
	}
out: 
	return 0;

err_no:
	return -1;			
}

static int do_get_exif(char* path, JObj* p_jdata, enum ADDEXIFFLAG exifflag)
{
	int ret = 0;
	CK20 (path, err_argunull);
	CK20 (p_jdata, err_argunull);
	//DMCLOG_D("get exif data from camera,file_path = %s]", path);
	
	CameraFile *file;
	const char *data;
	unsigned long size = 0;
	ExifData *ed;

	CK02 (ret = open_and_read_file(path, &file, &data, &size, GP_FILE_TYPE_EXIF), err_no);
	CK20 (ed = exif_data_new_from_data ((unsigned char *)data, size), err_exifnew_file);
	CK02 (ret = fill_exif_info(ed, p_jdata, exifflag), err_ed);
	
	exif_data_unref(ed);
	gp_file_unref(file);
	return 0;

err_exifnew_file:
	DMCLOG_E ("exif_data_new_from_data error");
	goto err_file;
err_argunull:
	DMCLOG_E ("parameter is null");
	goto err_no;
err_ed:
	exif_data_unref(ed);
err_file:
	gp_file_unref(file);
err_no:
	return -1;
}

static char* cam_makepath(char* folder, const char* filename)
{
	char* new_path = NULL;
	int len = strlen(folder)+strlen(filename)+5;
	CK20 (new_path = (char*)malloc(len), err_maloc_no);
	if (folder[strlen(folder)-1] == '/')
		snprintf(new_path, len, "%s%s", folder, filename);
	else
		snprintf(new_path, len, "%s/%s", folder, filename);
	return new_path;
err_maloc_no:
	DMCLOG_E("out of memeory");
	return NULL;
}

static int add_folder_tolist(char* folder, CameraList *list, JObj* filearray)
{
	const char* tmp_name = NULL;
	int ret = 0, count = 0;
	CK00 (ret = count = gp_list_count (list), err_listc_no);
	int i = 0;
	for (i = 0; i < count; ++i) {
		CK00 (ret = gp_list_get_name (list, i, &tmp_name), err_gname_no);

		JObj* jfolder = JSON_NEW_EMPTY_OBJECT();
		JSON_ARRAY_ADD_OBJECT(filearray, jfolder);
		JSON_ADD_OBJECT(jfolder,"type",JSON_NEW_OBJECT("folder",string));
		JSON_ADD_OBJECT(jfolder,"time",JSON_NEW_OBJECT(0,int64));
		JSON_ADD_OBJECT(jfolder,"size",JSON_NEW_OBJECT(0,int64));
		
		char* tmp_path = NULL;
		CK22 (tmp_path = cam_makepath(folder, tmp_name), err_no);

		//DMCLOG_D("new_folder=%s", tmp_path);
		JSON_ADD_OBJECT(jfolder,"path",JSON_NEW_OBJECT(tmp_path,string));

		JObj* son_array = JSON_NEW_ARRAY();
		JSON_ADD_OBJECT(jfolder,"son_list",son_array);
/*		JObj* file = JSON_NEW_EMPTY_OBJECT();
		JSON_ARRAY_ADD_OBJECT(son_array, file);
		JSON_ADD_OBJECT(file,"type",JSON_NEW_OBJECT("a",string));
		JSON_ADD_OBJECT(file,"time",JSON_NEW_OBJECT(1,int64));
		JSON_ADD_OBJECT(file,"size",JSON_NEW_OBJECT(2,int64));*/
		int total_num = 0;
		get_files_range(tmp_path, son_array, 0, 3, &total_num);
		JSON_ADD_OBJECT(jfolder,"total_num",JSON_NEW_OBJECT(total_num,int64));
		
		safe_free(tmp_path);
	} 
	return 0;
	
err_listc_no:
	DMCLOG_D("gp_list_count, errno=%d", ret);
	goto err_no;
err_gname_no:
	DMCLOG_D("gp_list_get_name, errno=%d", ret);
	goto err_no;
err_no:
	return -1;
	
}


static int add_files_tolist(char* folder, CameraList *list, JObj* filearray, int start_index, int end_index, int* total_num)
{
	const char* tmp_name = NULL;
	int ret = 0, count = 0;
	CK00 (ret = count = gp_list_count (list), err_listc_no);
	CameraFileInfo info;
	memset(&info, 0, sizeof(CameraFileInfo)); 
	int i = 0;
	if (total_num)
		*total_num = count;
	end_index = end_index==0?count:end_index;
	for (i = start_index; i < count; ++i) {
		if (i > end_index)
			break;
		CK00 (ret = gp_list_get_name (list, i, &tmp_name), err_gname_no);
		CK00 (ret = gp_camera_file_get_info (g_gp_params.camera, folder, tmp_name, &info, g_gp_params.context), err_no);

		JObj* file = JSON_NEW_EMPTY_OBJECT();
		JSON_ARRAY_ADD_OBJECT(filearray, file);
		JSON_ADD_OBJECT(file,"type",JSON_NEW_OBJECT(info.file.type,string));
		JSON_ADD_OBJECT(file,"time",JSON_NEW_OBJECT(info.file.mtime,int64));
		JSON_ADD_OBJECT(file,"size",JSON_NEW_OBJECT(info.file.size,int64));
		
		char* tmp_path = NULL;
		CK22 (tmp_path = cam_makepath(folder, tmp_name), err_no);
		//DMCLOG_D("new_files=%s", tmp_path);
		JSON_ADD_OBJECT(file,"path",JSON_NEW_OBJECT(tmp_path,string));
		do_get_exif(tmp_path, file, EXIF_GETFILELIST);
		safe_free(tmp_path);

	} 
	return 0;
	
err_listc_no:
	DMCLOG_D("gp_list_count, errno=%d", ret);
	goto err_no;
err_gname_no:
	DMCLOG_D("gp_list_get_name, errno=%d", ret);
	goto err_no;
//err_mem_no:
	DMCLOG_D("out of memory");
	goto err_no;
err_no:
	return ret;
	
}

typedef int (*func)(char*, void*);
static int get_files_range(char* folder, JObj* filearray, int start_index, int end_index, int* total_num)
{
	//DEBG(1, "[log][checking folder=%s ...]\n", folder);
	int ret=0;
	CameraList *list = NULL;	
	CK00 (ret = gp_list_new (&list), err_listnew_no);
	CK00 (ret = gp_camera_folder_list_files (g_gp_params.camera, folder, list, g_gp_params.context), err_listfi_list);
	CK02 (ret = add_files_tolist(folder, list, filearray, start_index, end_index, total_num), err_list);
	gp_list_free (list);
	return 0;
	
err_listnew_no:
	//DEBG(0, "[error][gp_list_new error][errno=%d]\n", ret);
	goto err_no;
err_listfi_list:
	//DEBG(0, "[error][gp_camera_folder_list_files error][errno=%d]\n", ret);
	goto err_list;
err_list:
	if (list)
		gp_list_free (list);
err_no:
	return ret; 
}
static int get_all_files(char* folder, JObj* filearray)
{
	DMCLOG_D("searching folder %s ...]", folder);
	return get_files_range(folder, filearray, 0, 0, NULL);
}
static int get_all_folders(char* folder, JObj* folderarray)
{
	DMCLOG_D("searching folder %s ...]", folder);
	int ret=0;
	CameraList *list = NULL;	
	CK00 (ret = gp_list_new (&list), err_listnew_no);
	CK00 (ret = gp_camera_folder_list_folders (g_gp_params.camera, folder, list, g_gp_params.context), err_listfo_list);
	CK02 (ret = add_folder_tolist(folder, list, folderarray), err_list);
	gp_list_free (list);
	return 0;
	
err_listnew_no:
	//DEBG(0, "[error][gp_list_new error][errno=%d]\n", ret);
	goto err_no;
err_listfo_list:
	//DEBG(0, "[error][gp_camera_folder_list_folders error][errno=%d]\n", ret);
	goto err_list;
err_list:
	if (list)
		gp_list_free (list);
err_no:
	return ret; 
}

static int foreach_folder(char* folder, JObj* array, func func)
{
	CameraList *list = NULL;	
	int ret = 0, count=0;
	const char* tmp_name = NULL;
	char* new_folder = NULL;
	CK00 (ret = func(folder, (void*)array), err_no);
	CK00 (ret = gp_list_new (&list), err_listnew_no);
	CK00 (ret = gp_camera_folder_list_folders (g_gp_params.camera, folder, list, g_gp_params.context), err_listfo_list);
	CK00 (ret = gp_list_count (list), err_listc_list);
	

	count = ret;
	int i = 0;
	for (i = 0; i < count; ++i) {
		CK00 (ret= gp_list_get_name (list, i, &tmp_name), err_gname_list);
		CK22 (new_folder = cam_makepath(folder,tmp_name), err_list);
		CK02 (ret = foreach_folder(new_folder, array, func), err_newfo);		
		safe_free(new_folder);
		
	}
	if (list)
		gp_list_free (list);

	return 0;
err_listnew_no:
	DMCLOG_E("gp_list_new,retcode = %d", ret);
	goto err_no;
err_listfo_list:
	DMCLOG_E("gp_camera_folder_list_folders,retcode = %d", ret);
	goto err_list;
err_listc_list:
	DMCLOG_E("gp_list_count,retcode = %d", ret);
	goto err_list;
err_gname_list:
	DMCLOG_E("gp_list_get_name,retcode = %d", ret);
	goto err_list;

err_newfo:
	safe_free(new_folder);
err_list:
	if (list)
		gp_list_free (list);
err_no:
	return -1; 
	
}

int cam_list_folders(char* folder, int recursive, JObj *data_json)
{
	CAMERA_LOCK;
	int ret = 0;
	JObj *array = JSON_NEW_ARRAY();
	JSON_ADD_OBJECT(data_json,"list",array);
	
	if (recursive)
		CK00 (ret = foreach_folder(folder, array, (func)get_all_folders), err_no);
	else
		CK00 (ret = get_all_folders(folder, array), err_no);
err_no:
	CAMERA_UNLOCK;
	return ret;
}

int cam_list_files(char* folder, int recursive, JObj *data_json)
{
	CAMERA_LOCK;
	int ret = 0;
	JObj *array = JSON_NEW_ARRAY();
	JSON_ADD_OBJECT(data_json,"list",array);
	if (recursive)
		CK00 (ret = foreach_folder(folder, array, (func)get_all_files), err_no);
	else
		CK00 (ret = get_all_files(folder, array), err_no);
err_no:
	CAMERA_UNLOCK;
	return ret;
}


static void gp_params_exit (GPParams *p)
{
	if (!p)
		return;

	if (p->_abilities_list)
		gp_abilities_list_free (p->_abilities_list);
	if (p->camera)
		gp_camera_unref (p->camera);
	if (p->folder)
		free (p->folder);
	if (p->filename)
		free (p->filename);
	if (p->context)
		gp_context_unref (p->context);
	if (p->hook_script)
		free (p->hook_script);
	if (p->portinfo_list)
		gp_port_info_list_free (p->portinfo_list);
	memset (p, 0, sizeof (GPParams));
}


static void _get_portinfo_list (GPParams *p) {
	int count, result;
	GPPortInfoList *list = NULL;

	if (p->portinfo_list)
		return;

	if (gp_port_info_list_new (&list) < GP_OK)
		return;
	result = gp_port_info_list_load (list);
	if (result < 0) {
		gp_port_info_list_free (list);
		return;
	}
	count = gp_port_info_list_count (list);
	if (count < 0) {
		gp_port_info_list_free (list);
		return;
	}
	p->portinfo_list = list;
	return;
}



static CameraAbilitiesList *
gp_params_abilities_list (GPParams *p)
{
	/* If p == NULL, the behaviour of this function is as undefined as
	 * the expression p->abilities_list would have been. */
	if (p->_abilities_list == NULL) {
		gp_abilities_list_new (&p->_abilities_list);
		gp_abilities_list_load (p->_abilities_list, p->context);
	}
	//DEBG (0, "zzz\n");
	return p->_abilities_list;
}
static int print_portinfo()
{
	int count = 0;
	struct _GPPortInfo *pinfo = NULL;
	char* xname ,*xpath  = 0;
	GPPortType type;
	count = gp_port_info_list_count (g_gp_params.portinfo_list);
	int i = 0;
//	DMCLOG_D("count = %d,iolib_count = %d", g_gp_params.portinfo_list->count, g_gp_params.portinfo_list->iolib_count);
	for (;i < count; ++i) {
		gp_port_info_list_get_info(g_gp_params.portinfo_list, i, &pinfo);
		gp_port_info_get_name (pinfo, &xname);
		gp_port_info_get_path (pinfo, &xpath);
		gp_port_info_get_type(pinfo, &type);
		DMCLOG_D("index = %d,porttype = %d,name = %s,path = %s", i, type, xname,
												xpath);
	}
	return 0;
}
int cam_detect_camera ()
{
	CAMERA_LOCK;
	int x, count;
        CameraList *list;
        const char *name = NULL, *value = NULL;
	int ret = 0;
	GPParams *p = &g_gp_params;
	
	gp_params_exit(p);
	init_airphoto_sdk(NULL);
	
	_get_portinfo_list (p);
	//print_portinfo();
	//count = gp_port_info_list_count (p->portinfo_list);
	CK00 (ret = gp_list_new (&list), err_no);
    gp_abilities_list_detect (gp_params_abilities_list(p), p->portinfo_list, list, p->context);


    CK00 (ret = count = gp_list_count (list), err_list);

        printf(("%-30s %-16s\n"), ("Model"), ("Port"));
        printf(("----------------------------------------------------------\n"));
        for (x = 0; x < count; x++) {
                (gp_list_get_name  (list, x, &name), list);
                (gp_list_get_value (list, x, &value), list);
                printf(("%-30s %-16s\n"), name, value);
        }
	gp_list_free (list);
	
	CAMERA_UNLOCK;
    return count;
err_list:
	gp_list_free (list);	
err_no:
	CAMERA_UNLOCK;
	return ret;
}

#if 0

int free_entrylist(struct EntryInfoList * eil)
{
	if (eil) {
		if (eil->eis) {
			free(eil->eis);
			eil->eis = NULL;
		}
		free(eil);
	}
	return 0;
}

#endif
/*
static char* exifstruct_match_tag(JObj* p_jdata, const char* tag, int* size)
{
	//int i = 0;
	if (strcmp(tag,"DateTime") == 0) {
		*size = sizeof(exif->shoot_date);
		return exif->shoot_date;
	} 
	if (strcmp(tag,"Make") == 0) {
		*size = sizeof(exif->manufacturer);
		return exif->manufacturer;
	} 
	if (strcmp(tag,"Model") == 0) {
		*size = sizeof(exif->modle);
		return exif->modle;
	} 
	if (strcmp(tag,"ExposureTime") == 0) {
		*size = sizeof(exif->exposure_time);
		return exif->exposure_time;
	} 
	if (strcmp(tag,"FNumber") == 0) {
		*size = sizeof(exif->fnmuber);
		return exif->fnmuber;
	} 
	if (strcmp(tag,"ISOSpeedRatings") == 0) {
		*size = sizeof(exif->iso_speed_ratings);
		return exif->iso_speed_ratings;
	} 
	if (strcmp(tag,"ExposureBiasValue") == 0) {
		*size = sizeof(exif->exposure_bias_value);
		return exif->exposure_bias_value;
	} 
	if (strcmp(tag,"FocalLength") == 0) {
		*size = sizeof(exif->focal_length);
		return exif->focal_length;
	} 
	if (strcmp(tag,"MeteringMode") == 0) {
		*size = sizeof(exif->metering_mode);
		return exif->metering_mode;
	} 
	return NULL;
}
*/
/*
DM_ROUTER][1451007224.391260][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = Make,val = Canon
[DM_ROUTER][1451007224.391317][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = Model,val = Canon EOS 700D
[DM_ROUTER][1451007224.391345][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = Orientation,val = top - left
[DM_ROUTER][1451007224.391559][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = XResolution,val = 72.00
[DM_ROUTER][1451007224.391593][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = YResolution,val = 72.00
[DM_ROUTER][1451007224.391622][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = ResolutionUnit,val = Inch
[DM_ROUTER][1451007224.391640][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = DateTime,val = 2015:12:23 10:07:28
[DM_ROUTER][1451007224.391659][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = Artist,val = 
[DM_ROUTER][1451007224.391679][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = YCbCrPositioning,val = co-sited
[DM_ROUTER][1451007224.391703][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = Copyright,val = [None] (Photographer) -  (Editor)
[DM_ROUTER][1451007224.391727][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = ExposureTime,val = 1/50 sec.
[DM_ROUTER][1451007224.391753][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = FNumber,val = f/4.0
[DM_ROUTER][1451007224.391773][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = ExposureProgram,val = Not defined
[DM_ROUTER][1451007224.391793][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = ISOSpeedRatings,val = 320
[DM_ROUTER][1451007224.391812][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = ExifVersion,val = Unknown Exif Version
[DM_ROUTER][1451007224.391831][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = DateTimeOriginal,val = 2015:12:23 10:07:28
[DM_ROUTER][1451007224.391850][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = DateTimeDigitized,val = 2015:12:23 10:07:28
[DM_ROUTER][1451007224.391873][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = ComponentsConfiguration,val = Y Cb Cr -
[DM_ROUTER][1451007224.391973][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = ShutterSpeedValue,val = 5.62 EV (1/49 sec.)
[DM_ROUTER][1451007224.392005][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = ApertureValue,val = 4.00 EV (f/4.0)
[DM_ROUTER][1451007224.392026][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = ExposureBiasValue,val = 0.00 EV
[DM_ROUTER][1451007224.392047][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = MeteringMode,val = Pattern
[DM_ROUTER][1451007224.392073][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = Flash,val = Flash did not fire, compulsory flash mode
[DM_ROUTER][1451007224.392096][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = FocalLength,val = 31.0 mm
[DM_ROUTER][1451007224.392116][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = MakerNote,val = 7856 bytes undefined data
[DM_ROUTER][1451007224.392136][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = UserComment,val = 
[DM_ROUTER][1451007224.392154][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = SubsecTime,val = 82
[DM_ROUTER][1451007224.392171][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = SubSecTimeOriginal,val = 82
[DM_ROUTER][1451007224.392189][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = SubSecTimeDigitized,val = 82
[DM_ROUTER][1451007224.392209][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = FlashPixVersion,val = FlashPix Version 1.0
[DM_ROUTER][1451007224.392229][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = ColorSpace,val = sRGB
[DM_ROUTER][1451007224.392248][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = PixelXDimension,val = 5184
[DM_ROUTER][1451007224.392266][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = PixelYDimension,val = 3456
[DM_ROUTER][1451007224.392287][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = FocalPlaneXResolution,val = 5798.66
[DM_ROUTER][1451007224.392308][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = FocalPlaneYResolution,val = 5788.94
[DM_ROUTER][1451007224.392328][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = FocalPlaneResolutionUnit,val = Inch
[DM_ROUTER][1451007224.392347][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = CustomRendered,val = Normal process
[DM_ROUTER][1451007224.394930][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = ExposureMode,val = Auto exposure
[DM_ROUTER][1451007224.394962][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = WhiteBalance,val = Auto white balance
[DM_ROUTER][1451007224.394984][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = SceneCaptureType,val = Standard
[DM_ROUTER][1451007224.395009][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = GPSVersionID,val = 2.3.0.0
[DM_ROUTER][1451007224.395029][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = InteroperabilityIndex,val = R98
[DM_ROUTER][1451007224.395048][./camera/camera.c:664][fill_exif_info][DEBUG] find new exif info,tag = InteroperabilityVersion,val = 0100
*/


int cam_get_exif(char* path, JObj* p_jdata)
{
	CAMERA_LOCK;
	int ret = do_get_exif(path, p_jdata, EXIF_GETEXIF);
	CAMERA_UNLOCK;
	return ret;
}


static int read_camera_file(CameraFileType filetype, char* path, unsigned long long offset, 
		unsigned long long size, unsigned char*	data)
{
	int ret = 0;
	CK20 (path, err_paranul_no);
	CK20 (data, err_paranul_no);
	char* folder = NULL;
	char* name = NULL;
		
	DMCLOG_D("get file info from camera,file_path = %s,offset = %llu,size_to_read = %llu", path, offset, size);

	CK00 (ret = split_path_folder(path, &folder, &name), err_no);
	CK00 (ret = gp_camera_file_read(g_gp_params.camera, folder, name, filetype, offset, 
									(char*)data, &size, g_gp_params.context), err_path);
	
	safe_free(folder);
	safe_free(name);
	return size;
	
err_paranul_no:
	DMCLOG_E("parameter is null");
	goto err_no;	
err_path:
	safe_free(folder);
	safe_free(name);
err_no:
	return -1;
}

int cam_read_file(char* path, unsigned long long offset, 
		unsigned long long size, unsigned char* data)
{
	CAMERA_LOCK;
	int ret = read_camera_file(GP_FILE_TYPE_NORMAL, path, offset, size, data);	
	CAMERA_UNLOCK;
	return  ret;
}
int cam_get_file_len(char* path)
{
	CAMERA_LOCK;
	int ret = 0;
	CameraFileInfo info;
	char* folder = NULL;
	char* name = NULL;
	CK00 (ret = split_path_folder(path, &folder, &name), err_no);
	CK00 (ret = gp_camera_file_get_info (g_gp_params.camera, folder, name, &info, g_gp_params.context), err_no);
	safe_free(folder);
	safe_free(name);
	CAMERA_UNLOCK;
	return info.file.size;
err_no:
	CAMERA_UNLOCK;
	return -1;
}

static int get_camera_file(char* path, unsigned char** thumb_data, CameraFileType filetype)
{
	int ret = 0;
	CK20 (path, err_argunull);

	CameraFile *file = NULL;
	const char *data = NULL;
	unsigned long size = 0;
	
	CK00 (ret = open_and_read_file(path, &file, &data, &size, filetype), err_no);
 
	CK20 (*thumb_data = (unsigned char*)malloc(size+1), err_malloc_file);
	memcpy(*thumb_data, data, size);
		
	gp_file_unref(file);
	return size;
	
	goto err_no;	
err_argunull:		
	DMCLOG_E("parameter is null");
	goto err_no;
err_malloc_file:
	DMCLOG_E("out of memory");
	goto err_file;
err_file:
	gp_file_unref(file);
err_no:
	return -1;
	
}

int cam_get_thumb_file(char* path, unsigned char** thumb_data)
{
	CAMERA_LOCK;
	int ret = get_camera_file(path, thumb_data, GP_FILE_TYPE_PREVIEW);
	CAMERA_UNLOCK;
	return ret;
}

int cam_del_file(char* path)
{
	CK20 (path, err_argunull);
	CAMERA_LOCK;
	char* folder = NULL;
	char* name = NULL;
	int ret = 0;
	DMCLOG_D("delete camera file,file_path = %s", path);

	CK00 (ret = split_path_folder(path, &folder, &name), err_no);
	ret = gp_camera_file_delete(g_gp_params.camera, folder, name, g_gp_params.context);
	if (ret < 0) 
		DMCLOG_E("gp_camera_file_delete failed,errno = %d", ret);
	
	safe_free(folder);
	safe_free(name);
err_no:
	CAMERA_UNLOCK;

	return ret;
err_argunull:
	DMCLOG_E("arugment path is null");
	return -1;
}

#define FILEPERPAGE 40

int cam_list_files_by_page(char* path, int pageNum, JObj* data_json)
{
	
	CAMERA_LOCK;
	DMCLOG_D("list files by page,path = %s,pageNum = %d,data = %d", path, pageNum, data_json);
	int ret = 0;
	JObj *array = JSON_NEW_ARRAY();
	JSON_ADD_OBJECT(data_json,"list",array);
	int total_num = 0;
	get_files_range(path, array, pageNum*FILEPERPAGE, (pageNum+1)*FILEPERPAGE-1, &total_num);
	JSON_ADD_OBJECT(data_json,"curPage",JSON_NEW_OBJECT(pageNum,int));
	JSON_ADD_OBJECT(data_json,"totalPage",JSON_NEW_OBJECT(total_num/FILEPERPAGE+1,int));
	JSON_ADD_OBJECT(data_json,"pageSize",JSON_NEW_OBJECT(FILEPERPAGE,int));
err_no:
	CAMERA_UNLOCK;
	return ret;
}

#if 0

static int get_camera_file(char* path, CameraFileType filetype, struct common_args *pargs, struct my_buffer* p_buffer)
{
	int ret = GP_OK;
	CK20 (path, err_argunull);
	//DEBG (1, "[log][get thumb data from camera ...][file_path=%s]\n", path);

	CameraFile *file = NULL;
	const char *data = NULL;
	unsigned long size = 0;
	
	CK00 (ret = open_and_read_file(path, &file, &data, &size, filetype), err_openread_no);
 

	CK00 (ret = put_buffer_32(p_buffer, (unsigned int)size), err_file);
	CK00 (ret = put_buffer_stream(p_buffer, (unsigned char*)data, size), err_file);
		/*

	struct APCameraFile *p_fileinfo;
	CK20 (p_fileinfo = (struct APCameraFile *)malloc(sizeof (struct APCameraFile)), err_cmalloc_file);
	CK20 (p_fileinfo->data = (unsigned char*)malloc(size+1), err_cmalloc_pfileinfo);
	p_fileinfo->size = size;
	memcpy(fileinfo->data, data, size);

	*errcode = ret;*/
	gp_file_unref(file);
	return ret;
	
err_openread_no:
	//DEBG (0, "[error_msg][open_and_read_file error][errorcode=%d]\n", ret);
	goto err_no;	
	/*
err_cmalloc_file:
	ret = GP_ERROR_NO_MEMORY;
	//DEBG (0, "[error_msg][cmalloc error][%s]\n", strerror(errno) );
	goto err_file;
err_cmalloc_pfileinfo:
	ret = GP_ERROR_NO_MEMORY;
	//DEBG (0, "[error_msg][cmalloc error][%s]\n", strerror(errno) );
	goto err_pfileinfo;

err_pfileinfo:
	free(fileinfo);
*/
err_argunull:		
	ret = GP_ERROR_BAD_PARAMETERS;
	//DEBG (0 ,"[error_msg][parameter is null!!]\n");
	goto err_no;

err_file:
	gp_file_unref(file);
err_no:
	return ret;
	
}

int get_camera_file_thumb(char* path, struct common_args *pargs, struct my_buffer* p_buffer)
{
	return get_camera_file(path, GP_FILE_TYPE_PREVIEW, pargs, p_buffer);
}
/*
struct APCameraFile *get_camera_file_normal(char* path, int *errcode)
{
	return get_camera_file(path, GP_FILE_TYPE_NORMAL, errcode);
}
*/
int free_camera_file(struct APCameraFile* camfile)
{
	if (camfile) {
		if (camfile->data)
			free(camfile->data);
		free(camfile);
	}
	return 0;
}






int cam_get_fileinfo(char* path, JObj* p_jdata)
{
	#if 0
	DMCLOG_D("camera get file information");
	int ret = 0;
	char* folder = NULL;
	char* name = NULL;
	CameraFileInfo info;
	memset(&info, 0, sizeof(CameraFileInfo)); 
	
	CK20 (path, err_argunull);

	
	CK00 (ret = split_path_folder(path, &folder, &name), err_no);
	CK00 (ret = gp_camera_file_get_info (g_gp_params.camera, folder, name, &info, g_gp_params.context), err_fileinfo_path);

	JSON_ADD_OBJECT(p_jdata,"type",JSON_NEW_OBJECT(info.file.type, string));
	JSON_ADD_OBJECT(p_jdata,"perm",JSON_NEW_OBJECT(info.file.permissions, int));
	JSON_ADD_OBJECT(p_jdata,"size",JSON_NEW_OBJECT(info.file.permissions, int));
	JSON_ADD_OBJECT(p_jdata,"width",JSON_NEW_OBJECT(info.file.permissions, int));
	JSON_ADD_OBJECT(p_jdata,"height",JSON_NEW_OBJECT(info.file.permissions, int));
	JSON_ADD_OBJECT(p_jdata,"mtime",JSON_NEW_OBJECT(info.file.permissions, int));

	snprintf(fileinfo->type, sizeof(fileinfo->type), "%s", info.file.type); 
	fileinfo->permissions = info.file.permissions;
	fileinfo->size = info.file.size;
	fileinfo->width = info.file.width;
	fileinfo->height = info.file.height;
	fileinfo->mtime = info.file.mtime;

	free(folder);
	free(name);
	return ret;

err_argunull: ;
	DMCLOG_E("parameter is null!!");
	goto err_no;
err_fileinfo_path:
	//DEBG (0, "[error_msg][gp_camera_file_get_info error!!!][error_code=%d]\n", ret);
	goto err_path;
err_path:
	free(folder);
	free(name);
err_no:
	return -1;
	#endif
	return 0;
}



#endif

