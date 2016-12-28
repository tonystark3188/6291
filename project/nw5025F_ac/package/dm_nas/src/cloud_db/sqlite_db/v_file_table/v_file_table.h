/*
 * =============================================================================
 *
 *       Filename: v_file_table.h
 *
 *    Description: v_file table related data structure definition.
 *
 *        Version:  1.0
 *        Created:  2016/10/19 
 *       Revision:  none
 *       Compiler:  gcc
 *
 *          Author:  Oliver <henry.jin@dmsys.com>
 *   Organization:  longsys
 *
 * =============================================================================
 */
#ifndef V_FILE_TABLE_H
#define V_FILE_TABLE_H


#ifdef __cplusplus
extern "C"{
#endif

#include "db_base.h"
#include "file_table.h"


#define  INVALID_V_FILE_ID    ((uint32_t)-1)
#define  V_FILE_TABLE_NAME  "v_file_table"

#define	MAX_FILE_NAME_LEN 	512
#define MAX_PASSWORD_LEN	32


typedef int (*real_remove)(char *path);
typedef int (*real_stat)(char *path,struct stat *st);




typedef enum
{
    V_FILE_TABLE_UPDATE_FILE_INFO    = 0x01,
    V_FILE_TABLE_UPDATE_FILE_LIST    = 0x02,
    V_FILE_TABLE_UPDATE_FILE_RENAME  = 0x03,
    V_FILE_TABLE_UPDATE_FILE_COPY	 = 0x04,
    V_FILE_TABLE_UPDATE_FILE_MOVE	 = 0x05,
    V_FILE_TABLE_UPDATE_IMAGE_INFO	 = 0x06,
    V_FILE_TABLE_UPDATE_AUDIO_INFO	 = 0x07,
    V_FILE_TABLE_UPDATE_VIDEO_INFO	 = 0x08,
    V_FILE_TABLE_UPDATE_THUM_INFO	 = 0x09,
}v_file_update_cmd_t;

typedef enum
{
    V_FILE_TABLE_QUERY_INFO    		= 0x01,
    V_FILE_TABLE_QUERY_LIST    		= 0x02,
    V_FILE_TABLE_QUERY_INFO_BY_UUID = 0x03,
    V_FILE_TABLE_QUERY_LIST_BY_UUID = 0x04,
    V_FILE_TABLE_SEARCH_QUERY_LIST  = 0x05,
    V_FILE_TABLE_QUERY_INFO_BY_PATH = 0x06,
    V_FILE_TABLE_QUERY_IMAGE_INFO	 = 0x07,
    V_FILE_TABLE_QUERY_AUDIO_INFO	 = 0x08,
    V_FILE_TABLE_QUERY_VIDEO_INFO	 = 0x09,
    V_FILE_TABLE_QUERY_THUM_INFO	 = 0x0A,
}v_file_query_cmd_t;

typedef enum
{
    V_FILE_TABLE_INSERT_INFO    		= 0x01,
    V_FILE_TABLE_INSERT_LIST    		= 0x02,
}v_file_insert_cmd_t;


typedef enum
{
    V_FILE_TABLE_DELETE_INFO    				= 0x01,
    V_FILE_TABLE_DELETE_TYPE_BY_PATH    		= 0x02,
}v_file_delete_cmd_t;


typedef enum
{
	/**
	 * 所有文件
	 */
	ALL = 0,
	/**
	 * 视频
	 */
	VIDEO,
	/**
	 * 音频
	 */
	AUDIO,
	/**
	 * 图片
	 */
	IMAGE,
	/**
	 * 图片
	 */
	DOCU

} Category;
typedef enum
{
	/**
	 * 时间升序
	 */
	TIME_ASC,
	/**
	 * 时间降序
	 */
	TIME_DES,
	/**
	 * 大小升序
	 */
	SIZE_ASC,
	/**
	 * 大小降序
	 */
	SIZE_DES,
	/**
	 * 名字升序
	 */
	NAME_ASC,
	/**
	 * 名字降序
	 */
	NAME_DES,
	/**
	 * 类型升序
	 */
	TYPE_ASC,
	/**
	 * 类型降序
	 */
	TYPE_DES,
	/**
	 * 拼音升序
	 */
	PINYIN_ASC,
	/**
	 * 拼音降序
	 */
	PINYIN_DES
} SortType;


typedef struct
{
	/**
	 * 文件大小
	 */
	size_t size;
	/**
	 * 文件名
	 */
	char* name;
	/**
	 * 是否是文件
	 */
	unsigned char is_dir;
	/**
	 * 访问时间
	 */
	long atime;
	/**
	 * 创建时间
	 */
	long ctime;
	/**
	 * 修改时间
	 */
	long mtime;

	
} FILE_ATTRIBUTE;


typedef struct
{
	/**
	 * 文件总数
	 */
	int total;
	/**
	 * 数量
	 */
	int len;

	/**
	 * authority 信息
	 */
	FILE_ATTRIBUTE file_attribute_list[];
} FILE_LIST;


typedef struct VfileTableList
{
	char		bucket_name[MAX_BUCKET_NAME_LEN];
	sqlite3*	database;
	uint8_t  	isDir;
	char		path[MAX_FILE_PATH_LEN];
	int		 	startIndex;//真实文件索引值（从0开始）
	int 	 	len;//需要从startIndex的文件条数
	Category 	category;
	SortType 	sortType;	
	int 		parent_id;
	/*
	*	total file count;
	*/
	int			total;
	/**
	 * file list info
	 */
	int			file_type;
	int 		cmd;
	char 		search_str[64];
	struct dl_list head; //list head for result
}v_file_list_t;
typedef struct
{
	v_file_info_t 			v_file_info;
	char					bucket_name[MAX_BUCKET_NAME_LEN];
	v_file_insert_cmd_t  	cmd;
	real_remove			 	remove;
	real_stat				stat;
}v_file_insert_t;


typedef struct
{
	v_file_info_t 			v_file_info;
	v_file_info_t 			v_des_info;
	char					bucket_name[MAX_BUCKET_NAME_LEN];
	char					des_bucket_name[MAX_BUCKET_NAME_LEN];
	v_file_update_cmd_t 	cmd;
	real_remove			 	remove;
	real_stat				stat;
}v_file_update_t;

typedef struct
{
	v_file_list_t 		v_file_list;
	v_file_info_t 		v_file_info;
	char				bucket_name[MAX_BUCKET_NAME_LEN];
	v_file_query_cmd_t  cmd;
	real_remove			remove;
}v_file_query_t;

typedef struct
{
	v_file_info_t 		 v_file_info;
	char				 bucket_name[MAX_BUCKET_NAME_LEN];
	v_file_delete_cmd_t  cmd;
	real_remove			 remove;
}v_file_delete_t;

void register_v_file_table_ops(void);


#ifdef __cplusplus
}
#endif



#endif
