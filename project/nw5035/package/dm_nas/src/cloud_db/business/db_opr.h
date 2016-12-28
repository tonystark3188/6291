/*
 * =============================================================================
 *
 *       Filename:  get_file_list.h
 *
 *    Description:  handle get file list cmd.
 *
 *        Version:  1.0
 *        Created:  2015/10/29 17:55:17
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver ()
 *   Organization:  
 *
 * =============================================================================
 */
#ifndef _GET_FILE_LIST_CMD_H_
#define _GET_FILE_LIST_CMD_H_

#include "base.h"
#include "db_table.h"

#ifdef __cplusplus
extern "C"{
#endif


/**
 *	insert item to user table when register
 */
ERROR_CODE_VFS handle_user_table_insert(_In_ char *username,_In_ char *password,_Out_ int *user_id);


/*
*	insert item to bucket table when register
*/
ERROR_CODE_VFS handle_bucket_table_insert(_In_ char *bucket_name,_In_ int creat_user_id,_Out_ int *bucket_id);


/*
*	insert item to authority table when register
*/
ERROR_CODE_VFS handle_authority_table_insert(_In_ int bucket_id,_In_ int user_id,_In_ int autority,_Out_ int *autority_id);

/*
*	query item from user table when login
*/
ERROR_CODE_VFS handle_user_table_query(_In_ char *username,_Out_ char *password,_Out_ int *user_id);

/*
*	query item from bucket table when login
*/
ERROR_CODE_VFS handle_bucket_table_query(_Out_ char *bucket_name,_In_ int creat_user_id,_Out_ int *bucket_id);

/*
*	query item from authority table when login
*/
ERROR_CODE_VFS handle_authority_table_query(_In_ int bucket_id,_In_ int user_id,_Out_ int *autority,_Out_ int *autority_id);

/*
*	query list from bucket table when login
*/
char **handle_bucket_table_list_query();


/************************************************upload start**************************************************************/
/*
*	query item from v file table when upload
*/
ERROR_CODE_VFS handle_v_file_table_query(_In_ char *bucket_name,v_file_info_t *file_info);


/*
*	insert item to v file table when upload
*/
ERROR_CODE_VFS handle_v_file_table_insert(_In_ char *bucket_name,v_file_info_t *v_file_info);

/************************************************upload end**************************************************************/

/*
*	update item to v file table when upload
*/
ERROR_CODE_VFS handle_v_file_table_update(_In_ char *bucket_name,int cmd ,real_remove remove,v_file_info_t *v_file_info);

ERROR_CODE_VFS handle_media_update(int cmd,v_file_info_t *v_file_info);

ERROR_CODE_VFS handle_media_query(int cmd,v_file_info_t *v_file_info);

/*
*	query list from v file table
*/
int handle_v_file_table_list_query(_In_ char *bucket_name,v_file_list_t *plist);

ERROR_CODE_VFS handle_v_file_table_delete(_In_ char *bucket_name,int cmd,real_remove remove,v_file_info_t *v_file_info);

/*
*	query item is or not exist in the file table by uuid
*	return 0:the uuid is exist;
*		   !0:the uuid is not exist
*/
ERROR_CODE_VFS handle_file_uuid_exist_query(_In_ char *bucket_name,_In_ char *uuid);

/*
*	query item list are or not exist in the file table by uuid
*	return 0:success
*		   !0:error
*/
ERROR_CODE_VFS handle_uuid_list_exist_query(_In_ char *bucket_name,struct dl_list *head);


/*
*	query item from v file table
*/
ERROR_CODE_VFS _handle_v_file_table_query(v_file_query_t *v_file_query);

/*
*	insert item to v file table
*/
ERROR_CODE_VFS _handle_v_file_table_insert(v_file_insert_t *v_file_insert);

/*
*	update item to another bucket
*/
ERROR_CODE_VFS _handle_v_file_table_update(v_file_update_t *v_file_update);

/*
*	delete item to v file table 
*/
ERROR_CODE_VFS _handle_v_file_table_delete(v_file_delete_t *v_file_delete);



#ifdef __cplusplus
}
#endif

#endif
