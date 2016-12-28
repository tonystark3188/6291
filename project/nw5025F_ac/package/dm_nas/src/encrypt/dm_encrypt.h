/************************************************************************/
/* qqiot wifi store sdk, http:/*iot.open.qq.com/						*/
/************************************************************************/

#ifndef __DM_ENCRYPT_H__
#define __DM_ENCRYPT_H__

#include "base.h"
/* md5加密，长度32字节
结果保存在out，out大小应当超过32字节，注意没有0结尾 
*/
void dm_md5(_In_ const void *src, int src_len, _Out_ char *out);

/* 生成utoken ，长度32字节
结果保存在utoken，大小应该超过32字节，注意没有0结尾
*/
void dm_gen_utoken(_Out_ char* utoken, _In_ const char *uin);

/* 生成session ，长度16字节
结果保存在session，大小应该超过17字节，有0结尾
*/
void _dm_gen_session(_Out_ char* session_id, _In_ const char *username, _In_ const char *password, _In_ int time);

/*
* put the session info to db for backup
*/
int _dm_put_session(_In_ char* session_id,_In_ const char* username);

/*
* check the session wheather or not have ability to process the cmd,return 0:can not,1:can
*/
int dm_check_session_cmd(_In_ char* sessionid, _In_ int cmd);
void _dm_gen_uuid(_Out_ char* uuid, _In_ const char *pid, _In_ const char *vid, _In_ unsigned long total_size,_In_ unsigned long free_size);
#endif