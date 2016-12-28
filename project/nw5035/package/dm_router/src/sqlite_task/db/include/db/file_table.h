/*
 * =============================================================================
 *
 *       Filename: file_table.h
 *
 *    Description: file table related data structure definition.
 *
 *        Version:  1.0
 *        Created:  2014/09/10 
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Tao Sheng (), vanbreaker90@gmail.com
 *   Organization:  
 *
 * =============================================================================
 */


#ifndef FILE_TABLE_H
#define FILE_TABLE_H

#ifdef __cplusplus
extern "C"{
#endif


#include "base.h"
#include "hidisk_errno.h"
#include "db/db_table.h"
#include "file_opr.h"
#include <stdbool.h>
#include <stdint.h>

#define  INVALID_FILE_ID    ((uint32_t)-1)




#define DELETE_BY_USER_ID 0
#define FREEZE_USER_FILE  1

#define  FILE_TABLE_NAME  "file_table"
#define  FILE_AUDIO_VIEW  "audio_view"
#define  FILE_IMAGE_VIEW  "image_view"
#define  FILE_VIDEO_VIEW  "video_view"
#define  FILE_DOCUM_VIEW  "docum_view" 

#define  FILE_BACKUP_VIEW "backup_file_view"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


//file type define
typedef enum
{
    TYPE_ALL = 0,     //only used when search file of all of the types above
   	TYPE_VIDEO = 1,
    TYPE_AUDIO = 2,
    TYPE_IMAGE = 3,
    TYPE_DOCUMENT = 4,
    TYPE_ARCHIVE = 5,
    TYPE_DIR = 6, 
    TYPE_UNKNOWN = 0x80,
}file_type_t;

//1：视频，2：音频，3：图片，4：文档

//file  searching list type define
typedef enum
{
	LIST_TYPE,   //search files of a certain type.
	LIST_DIR,    //search files in  a certain directory.
	LIST_SHARE, //search files that are shared with users
	LIST_GROUP, //search files in a certain group
	LIST_TRASH, //search files that are in the trash
}file_search_list_t;



typedef enum
{
	NAME_ASC, 
	NAME_DSC,
	TIME_ASC,
	TIME_DSC,
	SIZE_ASC,
	SIZE_DSC
}file_sort_mode_t;


typedef enum
{
// modify by wenhao
	STATE_NOT_READY = 0,
	STATE_NORMAL = 1,
	STATE_VISIBLE_IN_TRASH = 0x10,
	STATE_UNVISIBLE_IN_TRASH = 0x11
}file_state_t;


#define MAX_MIME_TYPE_SIZE     32


typedef struct FileInfoTable
{
	off_t 		file_size;//文件大小
	unsigned 	create_time;//创建时间
	unsigned 	modify_time;//修改时间
	unsigned 	access_time;
	char 		isFolder;//是否是目录文件
	char *		name;	//文件名
	char *		dir;
	char *		path;   //name
	char 		mime_type[MAX_MIME_TYPE_SIZE];
	int  		file_type;		   // see file_type_t
	uint32_t 	parent_id; 
	uint32_t	index;
	char 		file_uuid[64];
	unsigned	file_count;
	bool		attr;
	bool		parent_attr;
	struct dl_list next;
}file_info_t;



typedef struct FileIdTable
{
	char isFolder;//是否是目录文件
	char *name;	//文件名	   // see file_type_t 
	uint32_t index;
	char file_uuid[64];
	struct dl_list next;
}file_id_t;




typedef enum
{
    FILE_TABLE_UPDATE_STATE       = 0x01,
    FILE_TABLE_UPDATE_ACCESS_TIME = 0x02,
	FILE_TABLE_UPDATE_MODIFY_TIME = 0x04,
	FILE_TABLE_UPDATE_ALBUM_HIDE  = 0x08,
	FILE_TABLE_UPDATE_THUMB_POS   = 0x10,
	FILE_TABLE_UPDATE_FAVORITE    = 0x20,
	FILE_TABLE_UPDATE_SIZE        = 0x40,
	FILE_TABLE_UPDATE_HIDE        = 0x80
}file_update_cmd_t;


typedef struct
{
	file_info_t file_info;
	uint32_t    cmd;
}file_update_t;


#define UPDATE_SHARE_STATE   0x01
#define UPDATE_THUMB_NO      0x02
#define UPDATE_THUMB_POS     0x04
#define UPDATE_FAVORITE_FLAG 0x08



static inline file_type_t get_file_type(file_info_t *pfi)
{
    if(pfi->file_type < TYPE_UNKNOWN)
    {
	    return pfi->file_type;
	}
	else
	{
	    return TYPE_UNKNOWN;
	}
}

error_t file_table_init(char *name);
int file_query_callback(void *data, int nFields, char **FieldValue, char **FieldName);
int file_path_query_callback(void **data, int nFields, char **FieldValue, char **FieldName);
int file_attr_query_callback(void *data, int nFields, char **FieldValue, char **FieldName);
void register_file_table_ops(void);

#ifdef __cplusplus
}
#endif


#endif
