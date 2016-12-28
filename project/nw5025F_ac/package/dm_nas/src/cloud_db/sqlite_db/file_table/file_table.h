/*
 * =============================================================================
 *
 *       Filename: file_table.h
 *
 *    Description: file table related data structure definition.
 *
 *        Version:  1.0
 *        Created:  2016/10/21 
 *       Revision:  none
 *       Compiler:  gcc
 *
 *          Author:  Oliver <henry.jin@dmsys.com>
 *   Organization:  longsys
 *
 * =============================================================================
 */
#ifndef FILE_TABLE_H
#define FILE_TABLE_H


#ifdef __cplusplus
extern "C"{
#endif

#include "db_base.h"
#include "media_table.h"

#define  INVALID_FILE_ID    ((uint32_t)-1)
#define  FILE_TABLE_NAME  	"file_table"


typedef struct VfileInfoTable
{
	char 	bucket_name[MAX_BUCKET_NAME_LEN];
	int		id;
	char	name[MAX_FILE_NAME_LEN];
	char	gb_name[MAX_FILE_NAME_LEN];
	char	path[MAX_FILE_PATH_LEN];
	char	des_path[MAX_FILE_PATH_LEN];
	int		parent_id;
	uint8_t isDir;
	int		type;
	int		real_file_id;
	char	real_name[MAX_REAL_NAME_LEN];
	char	real_path[MAX_REAL_PATH_LEN];
	size_t	size;
	long	atime;
	long	ctime;
	long	mtime;
	char	uuid[MAX_FILE_UUID_LEN];
	int 	link;
	int 	media_index;
	int 	thum_index;
	int 	backupFlag;
	media_info_t media_info;
	struct dl_list next;
}v_file_info_t,file_info_t;




error_t load_max_real_id(sqlite3 *database,uint32_t *max_id);

error_t load_file_insert_cmd(sqlite3 *database,file_info_t *pfile);

error_t load_file_update_cmd(sqlite3 *database,file_info_t *pfile);

error_t load_file_update_media_index_cmd(sqlite3 *database,int real_id,int media_index);

error_t load_file_update_thum_index_cmd(sqlite3 *database,int real_file_id,int thum_index);

error_t load_file_info_cmd(sqlite3 *database, file_info_t *pfile);

error_t load_file_delete_cmd(sqlite3 *database,int id);

error_t load_file_des_link_cmd(sqlite3 *database,int id);

error_t load_file_asc_link_cmd(sqlite3 *database,int id,int link);

error_t load_uuid_exist_cmd(sqlite3 *database,char *uuid);

error_t load_file_info_by_path_cmd(sqlite3 *database, char *real_path,int *real_id);











#ifdef __cplusplus
}
#endif



#endif
