/*
 * =============================================================================
 *
 *       Filename:  my_json.h
 *
 *    Description:  Just wrapper of json library.
 *
 *        Version:  1.0
 *        Created:  2014/10/27 15:54:19
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 515296288jf@163.com
 *   Organization:  
 *
 * =============================================================================
 */
#ifndef _FILE_PROCESS_H_
#define _FILE_PROCESS_H_

#include "base.h"
     
#ifdef __cplusplus
extern "C"{
#endif
    typedef struct _ClientThreadInfo
    {
        int client_fd;  // for tcp or cgi_forward udp
        // for recv
        char *recv_buf;
        // for send response
        JObj *r_json;
        JObj *s_json;
        uint32_t cmd;
        uint32_t seq;
        int error;
        char *retstr;
        uint32_t time_out;
        char ver[32];
        char *username;
        char *password;
        int client_port;
        int statusFlag; /* ×￠2áμ?±ê????*/
        int tcp_server_port;
        char ip[32];
        //file download
        char *path;
        unsigned modifyTime;
        int count;
        char file_buf[16384];
        int local_fd;
        const char *srcPath;
        const char *desPath;
        char *send_buf;
        char *token;
        void  *st;
        off_t  offset;
        off_t   length;
        size_t content_len;
        off_t file_length;
        int status;
        void *p_dir;
        
    }ClientTheadInfo;
    
#define FN_LOGIN_HIDISK         2
#define FN_LOGOUT_HIDISK        3
#define FN_GET_SERVICE_LIST     4
#define FN_REGISTER_HIDISK      8
#define FN_FILE_GET_LIST        101
#define FN_FILE_MKDIR           102
#define FN_FILE_RENAME          103
#define FN_FILE_IS_EXIST        104
#define FN_FILE_GET_ATTR        105
#define FN_FILE_DOWNLOAD        106
#define FN_FILE_UPLOAD          107
#define FN_FILE_CHECKUPLOAD     108
#define FN_FILE_DELETE          109
#define FN_FILE_DELETE_PRO      139
#define FN_FILE_DELETE_HEAT     115
#define FN_FILE_DELETE_ALL      139
#define FN_FILE_COPY            110
#define FN_FILE_MOVE            111




int file_process(ClientTheadInfo *p_client_info);
    
int _DM_LoginHidisk(ClientTheadInfo *p_client_info);

int _DM_RegisterHidisk(ClientTheadInfo *p_client_info);

int _DM_LogoutHidisk(ClientTheadInfo *p_client_info);
    int DM_File_GetList(ClientTheadInfo *p_client_info);
/*
 * 创建文件夹 cmd = 102
 */
int DM_File_Mkdir(ClientTheadInfo *p_client_info);
/*
 * 文件或文件夹重命名 cmd = 103
 */
int DM_File_Rename(ClientTheadInfo *p_client_info);
/*
 * 判断文件是否存在 cmd = 104
 */
int DM_File_Is_Exist(ClientTheadInfo *p_client_info);
/*
 *获取文件属性 cmd = 105
 */
int DM_File_GetAttr(ClientTheadInfo *p_client_info);

int _DM_File_Download(ClientTheadInfo *p_client_info);
int _DM_File_Upload(ClientTheadInfo *p_client_info);
    
/*
 *删除文件夹或者文件 cmd = 109
 */
int DM_File_Delete(ClientTheadInfo *p_client_info);
int DM_File_Copy(ClientTheadInfo *p_client_info);
int DM_File_Del_All(ClientTheadInfo *p_client_info);
int DM_File_Move(ClientTheadInfo *p_client_info);

int file_parse_process(ClientTheadInfo *p_client_info);
int upload_parse_json(ClientTheadInfo *p_client_info);
int Parser_FileGetList(ClientTheadInfo *p_client_info);
/*
 * 文件夹创建 cmd = 102
 */
int Parser_FileMkdir(ClientTheadInfo *p_client_info);
/*
 * 文件或文件夹重名 cmd = 103
 */
int Parser_FileRename(ClientTheadInfo *p_client_info);
/*
 * 判断文件或文件夹是否存在 cmd = 104
 */
int Parser_FileIsExist(ClientTheadInfo *p_client_info);
/*
 * 获取文件或文件夹属性 cmd = 105
 */
int Parser_FileGetAttr(ClientTheadInfo *p_client_info);
/*
 *文件或文件夹删除 cmd = 109
 */
int parser_FileDelete(ClientTheadInfo *p_client_info);
/*
 *文件或文件夹复制 cmd = 110
 */
int parser_FileCopy(ClientTheadInfo *p_client_info);
/*
 *文件或文件夹移动 cmd = 111
 */
int parser_FileMove(ClientTheadInfo *p_client_info);
    /*
     * 文件下载 cmd = 106
     */
    int Parser_FileDownload(ClientTheadInfo *p_client_info);
    /*
     * 上传命令cmd = 107
     */
    int Parser_FileUpload(ClientTheadInfo *p_client_info);

#ifdef __cplusplus
}
#endif


#endif

