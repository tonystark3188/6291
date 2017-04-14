/*
 * =============================================================================
 *
 *       Filename:  process_json.h
 *
 *    Description:  json process operation
 *
 *        Version:  1.0
 *        Created:  2015/7/2 11:00
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _FILE_JSON_H_
#define _FILE_JSON_H_

#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "defs.h"

#ifdef __cplusplus
extern "C"{
#endif

/*
#define FN_ADD_TASK 200
#define FN_REMOVE_TASK 201
#define FN_PAUSE_TASK 202
#define FN_TOWAIT_TASK 203
#define FN_START_TASK 204
#define FN_GET_ALL_TASK 205
#define FN_BY_STATUS_TASK 206
#define FN_GET_NOW_PRO_TASK 207
#define FN_GET_VERSION 208
#define FN_GET_ALBUM 209
#define FN_ADD_FOLLOW 210
#define FN_DEL_FOLLOW 211
#define FN_GET_ALL_FOLLOW 212
#define FN_LETV_LOGIN 213
#define FN_LETV_CLEAR 214
*/

//#define CMD_NUM 				6
#define CMD_ADD_TASK 			"task_add"
#define CMD_REMOVE_TASK 		"task_remove"
#define CMD_PAUSE_TASK 			"task_pause"
#define CMD_TOWAIT_TASK		 	"task_towait"
#define CMD_START_TASK 			"task_start"
#define CMD_GET_ALL_TASK 		"task_getall"
#define CMD_BY_STATUS_TASK 		"task_state"
#define CMD_GET_NOW_PRO_TASK 	"task_now"
#define CMD_GET_ALBUM 			"album_get"
#define CMD_SET_ALBUM 			"album_set"

#define CMD_ADD_FOLLOW 			"follow_add"
#define CMD_DEL_FOLLOW 			"follow_del"
#define CMD_GET_ALL_FOLLOW 		"follow_getall"
#define CMD_LETV_FILE_DOWNLOAD	"file_download"
#define CMD_LETV_GET_VIDEO_INFO	"task_getvideo"
#define CMD_LETV_GET_VERSION	"task_version"
#define CMD_LETV_TASK_UPDATE	"task_update"


#define CMD_LETV_CLEAR 			""

int errcode;
char errmsg[64];

int Letv_AddTask(struct conn *c);
int Parser_AddTask(struct conn *c);
int Letv_RemoveTask(struct conn *c);
int Parser_RemoveTask(struct conn *c);
int Letv_PauseTask(struct conn *c);
int Parser_PauseTask(struct conn *c);
int Letv_ToWaitTask(struct conn *c);
int Parser_ToWaitTask(struct conn *c);
int Letv_StartTask(struct conn *c);
int Parser_StartTask(struct conn *c);
int Letv_GetAllTask(struct conn *c);
int Parser_GetAllTask(struct conn *c);
int Letv_GetByStatusTask(struct conn *c);
int Parser_GetByStatusTask(struct conn *c);
int Letv_GetNowProTask(struct conn *c);
int Parser_GetNowProTask(struct conn *c);
int Letv_GetVersion(struct conn *c);
int Parser_GetVersion(struct conn *c);
int Parser_TaskUpdate(struct conn *c);
int Letv_TaskUpdate(struct conn *c);


int Letv_GetAlbum(struct conn *c);
int Parser_GetAlbum(struct conn *c);
int Letv_SetAlbum(struct conn *c);
int Parser_SetAlbum(struct conn *c);

int Letv_AddFollow(struct conn *c);
int Parser_AddFollow(struct conn *c);
int Letv_DelFollow(struct conn *c);
int Parser_DelFollow(struct conn *c);
int Letv_GetAllFollow(struct conn *c);
int Parser_GetAllFollow(struct conn *c);
int Letv_Login(struct conn *c);
int Parser_Login(struct conn *c);
int Letv_Clear(struct conn *c);
int Parser_Clear(struct conn *c);
int Letv_fileDownload(struct conn *c);
int fileDownload(struct conn *c);
int Letv_GetVideoInfo(struct conn *c);
int Parse_GetVideoInfo(struct conn *c);










#ifdef __cplusplus
}
#endif

#endif

