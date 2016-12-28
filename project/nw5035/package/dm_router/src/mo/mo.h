/*
 * =============================================================================
 *
 *       Filename:  mq.h
 *
 *    Description:  message object
 *
 *        Version:  1.0
 *        Created:  2014/7/25 13:13:41
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ye Wenhao (), wenhaoye@126.com
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _MESSAGE_OBJECT_H_
#define _MESSAGE_OBJECT_H_

#include "base.h"
#include "list.h"
#include "sm/sm.h"
#include "db/nas_db.h"



#ifdef __cplusplus
extern "C"{
#endif


typedef enum
{
	MSG_DEFAULT = 0,

	MSG_TYPE_HB = 0x100,
	MSG_TYPE_HB_RES = 0x102,

    //file table query msg define
	MSG_TYPE_DB_QUERY = 0x200,
	MSG_DB_QUERY_EXIT,
	MSG_DB_QUERY_EXIT_RES,
	MSG_DB_FILE_ITEM_QUERY_START,
	MSG_DB_FILE_ITEM_QUERY_START_RES,
	MSG_DB_QUERY_PATH,
	MSG_DB_QUERY_PATH_RES,
	MSG_DB_QUERY_BATCH_PATH, // for getAbsolutPathArray cmd
	MSG_DB_QUERY_BATCH_PATH_RES,
	MSG_DB_QUERY_FILE_INFO,     // for getFileDetails cmd
	MSG_DB_QUERY_FILE_INFO_RES,
	MSG_DB_QUERY_FILE_BY_UUID,     // for getFileDetails cmd
	MSG_DB_QUERY_FILE_BY_UUID_RES,
	MSG_DB_QUERY_FILE_BY_NAME,
	MSG_DB_QUERY_FILE_BY_NAME_RES,
	MSG_DB_QUERY_FILE_LIST_CNT,
	MSG_DB_QUERY_FILE_LIST_CNT_RES,
	MSG_DB_QUERY_FILE_DIR_LIST_CNT,
	MSG_DB_QUERY_FILE_DIR_LIST_CNT_RES,
	MSG_DB_QUERY_FILE_LIST_CNT_BY_PATH,
	MSG_DB_QUERY_FILE_LIST_CNT__BY_PATH_RES,
	MSG_DB_QUERY_FILE_LIST,     // for getFilList cmd
	MSG_DB_QUERY_FILE_LIST_RES,
	MSG_DB_QUERY_DIR_LIST,     // for getFilList cmd
	MSG_DB_QUERY_DIR_LIST_RES,
	MSG_DB_QUERY_FILE_LIST_BY_PATH,     // for getFilList cmd
	MSG_DB_QUERY_FILE_LIST_BY_PATH_RES,
	MSG_DB_QUERY_HASH_CODE_FILE_LIST, // query by hash code
	MSG_DB_QUERY_HASH_CODE_FILE_LIST_RES,
	MSG_DB_QUERY_GROUP_LIST,    // for getFileGroup cmd
	MSG_DB_QUERY_GROUP_LIST_RES,
	MSG_DB_QUERY_PHOTO_LIST_WITHOUT_THUMB,
	MSG_DB_QUERY_PHOTO_LIST_WITHOUT_THUMB_RES,
	MSG_DB_QUERY_MAX_THUMB_NO,
	MSG_DB_QUERY_MAX_THUMB_NO_RES,
	MSG_DB_FILE_ITEM_QUERY_END,
	MSG_DB_FILE_ITEM_QUERY_END_RES,

	//user table query msg define	
	MSG_DB_USER_INFO_QUERY_START,
	MSG_DB_USER_INFO_QUERY_START_RES,
	MSG_DB_QUERY_USER_INFO_BY_ID,
	MSG_DB_QUERY_USER_INFO_BY_ID_RES,
	MSG_DB_QUERY_USER_INFO_BY_NAME,
	MSG_DB_QUERY_USER_INFO_BY_NAME_RES,
	MSG_DB_QUERY_USER_LIST,
	MSG_DB_QUERY_USER_LIST_RES,
	MSG_DB_USER_INFO_QUERY_END,
	MSG_DB_USER_INFO_QUERY_END_RES,

	//device table query msg define
	MSG_DB_QUERY_DEVICE_BY_USER,
	MSG_DB_QUERY_DEVICE_BY_USER_RES,
	MSG_DB_QUERY_DEVICE_BY_USER_MAC,
	MSG_DB_QUERY_DEVICE_BY_USER_MAC_RES,
	MSG_DB_QUERY_DEVICE_BY_USER_MAC_NAME,
	MSG_DB_QUERY_DEVICE_BY_USER_MAC_NAME_RES,

	//hdisk table query msg define
	MSG_DB_QUERY_HDISK_INFO,    // for getDiskCapacity cmd
	MSG_DB_QUERY_HDISK_INFO_RES,

	//version table query msg define
	MSG_DB_QUERY_VERSION_INFO,
	MSG_DB_QUERY_VERSION_INFO_RES,

	//backup file table query msg define
	MSG_DB_QUERY_BACKUP_LIST,
	MSG_DB_QUERY_BACKUP_LIST_RES,

	MSG_DB_QUERY_BACKUP_DEVICE_SIZE,
	MSG_DB_QUERY_BACKUP_DEVICE_SIZE_RES,

	MSG_DB_QUERY_BACKUP,
	MSG_DB_QUERY_BACKUP_RES,

	MSG_TYPE_DB_WRITE = 0x300,
	MSG_DB_WRITE_EXIT,
	MSG_DB_WRITE_EXIT_RES,
	MSG_DB_FILE_ITEM_WRITE_START,
	MSG_DB_FILE_ITEM_WRITE_START_RES,
	MSG_DB_COMMIT,
	MSG_DB_COMMIT_RES,
	MSG_DB_FILE_SINGLE_ADD,  // for uploadFileBegin cmd
	MSG_DB_FILE_SINGLE_ADD_RES,
	MSG_DB_FILE_BATCH_ADD,
	MSG_DB_FILE_BATCH_ADD_RES,
	MSG_DB_FILE_UPDATE,
	MSG_DB_FILE_UPDATE_RES,
	MSG_DB_FILE_COPY,
	MSG_DB_FILE_COPY_RES,
	MSG_DB_FILE_MOVE,
	MSG_DB_FILE_MOVE_RES,
	MSG_DB_FILE_DELETE,     // for deleteFile cmd 
	MSG_DB_FILE_DELETE_RES,
	MSG_DB_FILE_DEL_LIST_BY_PATH,     // for getFilList cmd
	MSG_DB_FILE_DEL_LIST_BY_PATH_RES,
	MSG_DB_FILE_GENERIC_DELETE,     // for delete File list by path cmd 
	MSG_DB_FILE_GENERIC_DELETE_RES,
	MSG_DB_FILE_RECYCLE,     // for recoverDeleteFile cmd
	MSG_DB_FILE_RECYCLE_RES,
	MSG_DB_FILE_RENAME,
	MSG_DB_FILE_RENAME_RES,
	MSG_DB_SHARE_FILE,  // for setShareState cmd
	MSG_DB_SHARE_FILE_RES,
	MSG_DB_UPDATE_FILE,
	MSG_DB_UPDATE_FILE_RES,
	MSG_DB_REINDEX,
	MSG_DB_REINDEX_RES,
	MSG_DB_WRITE_END,
	MSG_DB_WRITE_END_RES,

    MSG_DB_USER_INFO_WRITE_START,
    MSG_DB_USER_INFO_WRITE_START_RES,
	MSG_DB_USER_INFO_ADD,
	MSG_DB_USER_INFO_ADD_RES,
	MSG_DB_USER_INFO_UPDATE,    // for changePassword cmd
	MSG_DB_USER_INFO_UPDATE_RES,
	MSG_DB_USER_INFO_DEL,
	MSG_DB_USER_INFO_DEL_RES,
	MSG_DB_USER_INFO_WRITE_END,
    MSG_DB_USER_INFO_WRITE_END_RES,

	MSG_DB_DEVICE_INFO_ADD,
	MSG_DB_DEVICE_INFO_ADD_RES,

	MSG_DB_HDISK_INFO_ADD,
	MSG_DB_HDISK_INFO_ADD_RES,
	MSG_DB_HDISK_INFO_UPDATE,
	MSG_DB_HDISK_INFO_UPDATE_RES,

	MSG_DB_VERSION_ADD,
	MSG_DB_VERSION_ADD_RES,
	MSG_DB_VERSION_UPDATE,
	MSG_DB_VERSION_UPDATE_RES,

	MSG_DB_BACKUP_FILE_ADD,
	MSG_DB_BACKUP_FILE_ADD_RES,
	MSG_DB_BACKUP_FILE_DEL,
	MSG_DB_BACKUP_FILE_DEL_RES,
	MSG_DB_BACKUP_FILE_SPECIAL_DEL,
	MSG_DB_BACKUP_FILE_SPECIAL_DEL_RES,
	MSG_DB_BACKUP_FILE_UPDATE,
	MSG_DB_BACKUP_FILE_UPDATE_RES,
	
	MSG_DB_BACKUP,
	MSG_DB_BACKUP_RES,

	MSG_DB_CLEAN_UP_FILE,
	MSG_DB_CLEAN_UP_FILE_RES,
	MSG_DB_CLEAN_UP_BKP_FILE,
	MSG_DB_CLEAN_UP_BKP_FILE_RES,
	MSG_DB_SCANNING_DISK,
	MSG_TYPE_FLASH = 0x400,
	MSG_FLASH_READ,
	MSG_FLASH_READ_RES,
	MSG_FLASH_WRITE,
	MSG_FLASH_WRITE_RES,
	MSG_FLASH_CHECK,
	MSG_FLASH_CHECK_RES,

	MSG_TYPE_DISK = 0x500,
	MSG_DISK_READ,
	MSG_DISK_READ_RES,
	MSG_DISK_WRITE,
	MSG_DISK_WRITE_RES,
	MSG_DISK_CHECK,
	MSG_DISK_CHECK_RES,

    MSG_TYPE_CGI = 0x600,
    MSG_CGI_REQ,
    MSG_CGI_REQ_RES,

    MSG_TYPE_THUMB = 0X700,
	MSG_THUMB_EXIT,
	MSG_THUMB_EXIT_RES,
    MSG_THUMB_REQ,
    MSG_THUMB_REQ_RES,


	MSG_TYPE_SHELL_CMD = 0X800,
	MSG_SHELL_CMD_EXIT,
	MSG_SHELL_CMD_EXIT_RES,
	MSG_SHELL_CMD_REQ,
	MSG_SHELL_CMD_REQ_RES,

    // Notice: all msg type must define before here!!!
    MSG_TYPE_TEST = 0x900,
    MSG_TEST_READ,
    MSG_TEST_READ_RES,
    MSG_TEST_WRITE,
    MSG_TEST_WRITE_RES
    	
}MSG_TYPE;

typedef enum 
{
	MSG_NO_RETURN = 0,
	MSG_NORMAL_RETURN = 1,
	MSG_SYN_RETURN = 2,

}MSG_RET_FLAG;

typedef struct _MsgObj
{
    // private member
	struct dl_list head; // linked to next MsgObj, used by mq.
    void *parent_ptr;    // save parent ptr for free.

    //public member
	MSG_TYPE m_type; // Must set
	//MQObj *m_from;
	void *m_from;   // If we want to get response no sync, set it!

	MSG_RET_FLAG ret_flag; // Must set
	//sem_t *m_syn;  // If we want to get response sync, set it!
	SemObj *m_syn;  // If we want to get response sync, set it!

}MsgObj;

#define get_db_opr_obj(p_msg) (p_msg->msg_data.db_obj)

typedef struct _Message
{
	MsgObj msg_header;
	union
	{
		DB_OprObj db_obj;
		int test_data;
	}msg_data;
#ifdef MEM_DEBUG
	struct dl_list debug_next;
	time_t crt_time;
#endif

}Message;



Message *new_message(void);
Message *new_message_no_wait(void);
int free_message(Message **pp_msg);
Message *create_sync_message(void);
int free_sync_message( Message **pp_msg);


#ifdef __cplusplus
}
#endif


#endif

