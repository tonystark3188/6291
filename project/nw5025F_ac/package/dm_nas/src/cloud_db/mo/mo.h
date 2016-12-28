/*
 * =============================================================================
 *
 *       Filename:  mq.h
 *
 *    Description:  message object
 *
 *        Version:  1.0
 *        Created:  2016/10/24 13:13:41
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author: Oliver (), henry.jin@dmsys.com
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _MESSAGE_OBJECT_H_
#define _MESSAGE_OBJECT_H_

#include "base.h"
#include "sm.h"
#include "db_table.h"



#ifdef __cplusplus
extern "C"{
#endif

typedef enum
{
    //file table query msg define
	MSG_TYPE_DB_QUERY = 0x200,

	MSG_TYPE_DB_WRITE = 0x300,
	MSG_DB_INSERT_USER,
	MSG_DB_UPDATE_USER,
	MSG_DB_DELETE_USER,
	MSG_DB_QUERY_USER,
	
	MSG_DB_INSERT_DISK,
	MSG_DB_DELETE_DISK,
	MSG_DB_UPDATE_DISK,
	MSG_DB_QUERY_DISK,

	MSG_DB_INSERT_AUTHORITY,
	MSG_DB_DELETE_AUTHORITY,
	MSG_DB_UPDATE_AUTHORITY,
	MSG_DB_QUERY_AUTHORITY,

	MSG_DB_INSERT_BUCKET,
	MSG_DB_DELETE_BUCKET,
	MSG_DB_UPDATE_BUCKET,
	MSG_DB_QUERY_BUCKET,
	
	MSG_DB_INSERT_VFILE,
	MSG_DB_DELETE_VFILE,
	MSG_DB_UPDATE_VFILE,
	MSG_DB_QUERY_VFILE,

	MSG_DB_INSERT_FILE,
	MSG_DB_DELETE_FILE,
	MSG_DB_UPDATE_FILE,
	MSG_DB_QUERY_FILE,

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

