#include "file_process.h"
#include "ppclib.h"

typedef int (*API_FUNC)(ClientTheadInfo *p_client_info);
typedef int (*PARSE_FUNC)(ClientTheadInfo *p_client_info);
typedef struct _file_tag_handle
{
    uint16_t tag;
    API_FUNC tagfun;
    PARSE_FUNC parsefun;

}file_tag_handle;

file_tag_handle all_file_handle[]=
{
    {FN_REGISTER_HIDISK,_DM_RegisterHidisk,parser_FileDelete},
    {FN_LOGIN_HIDISK,_DM_LoginHidisk,parser_FileDelete},
    {FN_LOGOUT_HIDISK,_DM_LogoutHidisk,parser_FileDelete},
    {FN_FILE_GET_LIST,DM_File_GetList,Parser_FileGetList},
    {FN_FILE_DOWNLOAD,_DM_File_Download,Parser_FileDownload},
    {FN_FILE_UPLOAD,_DM_File_Upload,Parser_FileUpload},
    {FN_FILE_MKDIR,DM_File_Mkdir,Parser_FileMkdir},
    {FN_FILE_RENAME,DM_File_Rename,Parser_FileRename},
    {FN_FILE_IS_EXIST,DM_File_Is_Exist,Parser_FileIsExist},
    {FN_FILE_GET_ATTR,DM_File_GetAttr,Parser_FileGetAttr},
    {FN_FILE_DELETE,DM_File_Delete,parser_FileDelete},
    {FN_FILE_DELETE_PRO,DM_File_Copy,parser_FileCopy},
    {FN_FILE_DELETE_HEAT,DM_File_Copy,parser_FileCopy},
    {FN_FILE_COPY,DM_File_Copy,parser_FileCopy},
    {FN_FILE_MOVE,DM_File_Move,parser_FileMove},
    {FN_FILE_UTIMENSAT, DM_File_Utimensat, parser_FileUtimensat},
    {FN_FILE_SYMLINK, DM_File_Symlink, Parser_FileSymlink},
    {FN_FILE_SYMLINK, DM_File_Link, Parser_FileLink},
    {FN_FILE_READLINK, DM_File_Readlink, Parser_FileReadlink},
    {FN_FILE_FTRUNCATE, DM_File_Ftruncate, Parser_FileFtruncate},
    {FN_FILE_FALLOCATE, DM_File_Fallocate, Parser_FileFallocate},
    {FN_FILE_STATVFS, DM_File_Statvfs, Parser_FileStatvfs},
};
#define FILE_TAGHANDLE_NUM (sizeof(all_file_handle)/sizeof(all_file_handle[0]))

static int json_to_string(ClientTheadInfo *p_client_info,JObj* response_json)
{
    int res_sz = 0;
    char *response_str = JSON_TO_STRING(response_json);
    if(response_str == NULL)
    {
        return -1;
    }
    res_sz = strlen(response_str);
    p_client_info->retstr = (char*)calloc(1,res_sz + 1);
    if(p_client_info->retstr == NULL)
    {
        return -1;
    }
    strcpy(p_client_info->retstr,response_str);
    DMCLOG_D("p_client_info->retstr = %s",p_client_info->retstr);
    return 0;
}

static int all_file_json_to_string(ClientTheadInfo *p_client_info,JObj* response_json)
{
    int res_sz = 0;
    char *response_str = JSON_TO_STRING(response_json);
    if(response_str == NULL)
    {
        return -1;
    }
    res_sz = strlen(response_str);
    p_client_info->send_buf = (char*)calloc(1,res_sz + 1);
    if(p_client_info->send_buf == NULL)
    {
        return -1;
    }
    strcpy(p_client_info->send_buf,response_str);
    DMCLOG_D("p_client_info->retstr = %s",p_client_info->send_buf);
    return 0;
}

int DM_File_GetList(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj* response_data_array = JSON_NEW_ARRAY();
    JObj* para_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(para_json, "path",JSON_NEW_OBJECT(p_client_info->path,string));
    JSON_ARRAY_ADD_OBJECT(response_data_array,para_json);
    JSON_ADD_OBJECT(send_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "token", JSON_NEW_OBJECT(p_client_info->token,int64));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}

int _DM_LoginHidisk(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj* response_data_array = JSON_NEW_ARRAY();
    JObj* para_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(para_json, "username",JSON_NEW_OBJECT(p_client_info->username,string));
    JSON_ADD_OBJECT(para_json, "password",JSON_NEW_OBJECT(p_client_info->password,string));
    JSON_ARRAY_ADD_OBJECT(response_data_array,para_json);
    JSON_ADD_OBJECT(send_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}

int _DM_RegisterHidisk(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj* response_data_array = JSON_NEW_ARRAY();
    JObj* para_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(para_json, "username",JSON_NEW_OBJECT(p_client_info->username,string));
    JSON_ADD_OBJECT(para_json, "password",JSON_NEW_OBJECT(p_client_info->password,string));
    JSON_ARRAY_ADD_OBJECT(response_data_array,para_json);
    JSON_ADD_OBJECT(send_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}

int _DM_LogoutHidisk(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "token",JSON_NEW_OBJECT(p_client_info->token,int64));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}


int _DM_File_Download(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj* response_data_array = JSON_NEW_ARRAY();
    JObj* para_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(para_json, "path",JSON_NEW_OBJECT(p_client_info->srcPath,string));
    JSON_ADD_OBJECT(para_json, "offset",JSON_NEW_OBJECT(p_client_info->offset,int64));
    JSON_ADD_OBJECT(para_json, "length",JSON_NEW_OBJECT(p_client_info->length,int64));
    
    JSON_ARRAY_ADD_OBJECT(response_data_array,para_json);
    JSON_ADD_OBJECT(send_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "token", JSON_NEW_OBJECT(p_client_info->token,int64));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}

int _DM_File_CheckUpload(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj* response_data_array = JSON_NEW_ARRAY();
    JObj* para_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(para_json, "path",JSON_NEW_OBJECT(p_client_info->desPath,string));
    JSON_ADD_OBJECT(para_json, "modifyTime",JSON_NEW_OBJECT(p_client_info->modifyTime,int64));
    JSON_ADD_OBJECT(para_json, "fileSize",JSON_NEW_OBJECT(p_client_info->file_length,int64));
    JSON_ARRAY_ADD_OBJECT(response_data_array,para_json);
    JSON_ADD_OBJECT(send_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "token", JSON_NEW_OBJECT(p_client_info->token,int64));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}

int _DM_File_Upload(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj* response_data_array = JSON_NEW_ARRAY();
    JObj* para_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(para_json, "path",JSON_NEW_OBJECT(p_client_info->desPath,string));
    JSON_ADD_OBJECT(para_json, "fileSize",JSON_NEW_OBJECT(p_client_info->file_length,int64));
    JSON_ADD_OBJECT(para_json, "modifyTime",JSON_NEW_OBJECT(p_client_info->modifyTime,int64));
    JSON_ADD_OBJECT(para_json, "offset",JSON_NEW_OBJECT(p_client_info->offset,int64));
    JSON_ADD_OBJECT(para_json, "length",JSON_NEW_OBJECT(p_client_info->length,int64));

    JSON_ARRAY_ADD_OBJECT(response_data_array,para_json);
    JSON_ADD_OBJECT(send_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "token", JSON_NEW_OBJECT(p_client_info->token,int64));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}

int DM_File_GetStorage(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "token", JSON_NEW_OBJECT(p_client_info->token,int64));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}


int DM_File_Mkdir(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj* response_data_array = JSON_NEW_ARRAY();
    JObj* para_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(para_json, "path",JSON_NEW_OBJECT(p_client_info->path,string));
    JSON_ARRAY_ADD_OBJECT(response_data_array,para_json);
    JSON_ADD_OBJECT(send_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "token", JSON_NEW_OBJECT(p_client_info->token,int64));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}

int DM_File_Rename(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj* response_data_array = JSON_NEW_ARRAY();
    JObj* para_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(para_json, "srcPath",JSON_NEW_OBJECT(p_client_info->srcPath,string));
    JSON_ADD_OBJECT(para_json, "desPath",JSON_NEW_OBJECT(p_client_info->desPath,string));
    JSON_ARRAY_ADD_OBJECT(response_data_array,para_json);
    JSON_ADD_OBJECT(send_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "token", JSON_NEW_OBJECT(p_client_info->token,int64));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}

int DM_File_Is_Exist(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj* response_data_array = JSON_NEW_ARRAY();
    JObj* para_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(para_json, "path",JSON_NEW_OBJECT(p_client_info->srcPath,string));
    JSON_ARRAY_ADD_OBJECT(response_data_array,para_json);
    JSON_ADD_OBJECT(send_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "token", JSON_NEW_OBJECT(p_client_info->token,int64));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}

int DM_File_GetAttr(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj* response_data_array = JSON_NEW_ARRAY();
    JObj* para_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(para_json, "path",JSON_NEW_OBJECT(p_client_info->srcPath,string));
    JSON_ARRAY_ADD_OBJECT(response_data_array,para_json);
    JSON_ADD_OBJECT(send_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "token", JSON_NEW_OBJECT(p_client_info->token,int64));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}

int DM_File_Delete(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj* response_data_array = JSON_NEW_ARRAY();
    JObj* para_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(para_json, "path",JSON_NEW_OBJECT(p_client_info->path,string));
    JSON_ARRAY_ADD_OBJECT(response_data_array,para_json);
    JSON_ADD_OBJECT(send_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "token", JSON_NEW_OBJECT(p_client_info->token,int64));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}

int DM_File_Copy(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj* response_data_array = JSON_NEW_ARRAY();
    JObj* para_json = JSON_NEW_EMPTY_OBJECT();
    ENTER_FUNC();
    JSON_ADD_OBJECT(para_json, "path",JSON_NEW_OBJECT(p_client_info->srcPath,string));
    if(p_client_info->desPath != NULL)
        JSON_ADD_OBJECT(para_json, "desPath",JSON_NEW_OBJECT(p_client_info->desPath,string));
    ENTER_FUNC();
    JSON_ARRAY_ADD_OBJECT(response_data_array,para_json);
    JSON_ADD_OBJECT(send_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "token", JSON_NEW_OBJECT(p_client_info->token,int64));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}

int DM_File_Move(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj* response_data_array = JSON_NEW_ARRAY();
    JObj* para_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(para_json, "path",JSON_NEW_OBJECT(p_client_info->srcPath,string));
    if(p_client_info->desPath != NULL)
        JSON_ADD_OBJECT(para_json, "desPath",JSON_NEW_OBJECT(p_client_info->desPath,string));
    JSON_ARRAY_ADD_OBJECT(response_data_array,para_json);
    JSON_ADD_OBJECT(send_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "token", JSON_NEW_OBJECT(p_client_info->token,int64));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}

int DM_File_Utimensat(ClientTheadInfo *p_client_info)
{
	int res = 0;
	if(p_client_info->path == NULL){
		return -1;
	}
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj* response_data_array = JSON_NEW_ARRAY();
    JObj* para_json = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(para_json, "path",JSON_NEW_OBJECT(p_client_info->path,string));
    JSON_ADD_OBJECT(para_json, "a_time_sec",JSON_NEW_OBJECT(p_client_info->a_time.tv_sec,int));
	JSON_ADD_OBJECT(para_json, "a_time_nsec",JSON_NEW_OBJECT(p_client_info->a_time.tv_nsec,int));
	JSON_ADD_OBJECT(para_json, "m_time_sec",JSON_NEW_OBJECT(p_client_info->m_time.tv_sec,int));
	JSON_ADD_OBJECT(para_json, "m_time_nsec",JSON_NEW_OBJECT(p_client_info->m_time.tv_nsec,int));
	JSON_ADD_OBJECT(para_json, "flags",JSON_NEW_OBJECT(p_client_info->status,int));
	JSON_ARRAY_ADD_OBJECT(response_data_array,para_json);
    JSON_ADD_OBJECT(send_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "token", JSON_NEW_OBJECT(p_client_info->token,int64));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}

int DM_File_Symlink(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj* response_data_array = JSON_NEW_ARRAY();
    JObj* para_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(para_json, "srcPath",JSON_NEW_OBJECT(p_client_info->srcPath,string));
    JSON_ADD_OBJECT(para_json, "desPath",JSON_NEW_OBJECT(p_client_info->desPath,string));
    JSON_ARRAY_ADD_OBJECT(response_data_array,para_json);
    JSON_ADD_OBJECT(send_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "token", JSON_NEW_OBJECT(p_client_info->token,int64));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}

int DM_File_Readlink(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj* response_data_array = JSON_NEW_ARRAY();
    JObj* para_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(para_json, "srcPath",JSON_NEW_OBJECT(p_client_info->srcPath,string));
    JSON_ARRAY_ADD_OBJECT(response_data_array,para_json);
    JSON_ADD_OBJECT(send_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "token", JSON_NEW_OBJECT(p_client_info->token,int64));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}

int DM_File_Link(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj* response_data_array = JSON_NEW_ARRAY();
    JObj* para_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(para_json, "srcPath",JSON_NEW_OBJECT(p_client_info->srcPath,string));
    JSON_ADD_OBJECT(para_json, "desPath",JSON_NEW_OBJECT(p_client_info->desPath,string));
    JSON_ARRAY_ADD_OBJECT(response_data_array,para_json);
    JSON_ADD_OBJECT(send_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "token", JSON_NEW_OBJECT(p_client_info->token,int64));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}

int DM_File_Ftruncate(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj* response_data_array = JSON_NEW_ARRAY();
    JObj* para_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(para_json, "path",JSON_NEW_OBJECT(p_client_info->path,string));
    JSON_ADD_OBJECT(para_json, "length",JSON_NEW_OBJECT(p_client_info->length,int64));
    JSON_ARRAY_ADD_OBJECT(response_data_array,para_json);
    JSON_ADD_OBJECT(send_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "token", JSON_NEW_OBJECT(p_client_info->token,int64));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}

int DM_File_Fallocate(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj* response_data_array = JSON_NEW_ARRAY();
    JObj* para_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(para_json, "path",JSON_NEW_OBJECT(p_client_info->path,string));	
    JSON_ADD_OBJECT(para_json, "mode",JSON_NEW_OBJECT(p_client_info->status,string));
    JSON_ADD_OBJECT(para_json, "offset",JSON_NEW_OBJECT(p_client_info->offset,string));
    JSON_ADD_OBJECT(para_json, "length",JSON_NEW_OBJECT(p_client_info->length,int64));
    JSON_ARRAY_ADD_OBJECT(response_data_array,para_json);
    JSON_ADD_OBJECT(send_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "token", JSON_NEW_OBJECT(p_client_info->token,int64));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}

int DM_File_Statvfs(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj* send_json = JSON_NEW_EMPTY_OBJECT();
    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    JObj* response_data_array = JSON_NEW_ARRAY();
    JObj* para_json = JSON_NEW_EMPTY_OBJECT();
    JSON_ADD_OBJECT(para_json, "path",JSON_NEW_OBJECT(p_client_info->path,string));	
    JSON_ARRAY_ADD_OBJECT(response_data_array,para_json);
    JSON_ADD_OBJECT(send_json, "data", response_data_array);
    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(p_client_info->cmd,int));
    JSON_ADD_OBJECT(header_json, "token", JSON_NEW_OBJECT(p_client_info->token,int64));
    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(p_client_info->seq,int));
    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(p_client_info->error,int));
    JSON_ADD_OBJECT(send_json, "header", header_json);
    res = json_to_string(p_client_info,send_json);
    if(res < 0)
    {
        JSON_PUT_OBJECT(send_json);
        return -1;
    }
    JSON_PUT_OBJECT(send_json);
    return 0;
}


int file_process(ClientTheadInfo *p_client_info)
{
	uint8_t i = 0;
	uint8_t switch_flag = 0;
	int ret = -1;
	for(i = 0; i<FILE_TAGHANDLE_NUM; i++)
	{
		if(p_client_info->cmd == all_file_handle[i].tag)
		{
	       	 ret = all_file_handle[i].tagfun(p_client_info);
		     switch_flag = 1;
		}
	}
	if(switch_flag == 0)
	{
	    strcpy(p_client_info->retstr,"input cmd is not finished!");
	}
	return ret;
}
int file_parse_header_json(ClientTheadInfo *p_client_info)
{
    int res = 0;
    p_client_info->r_json = JSON_PARSE(p_client_info->recv_buf);
    if(p_client_info->r_json == NULL)
    {
        DMCLOG_D("access NULL");
        res = -1;
        goto exit;
    }
    if(is_error(p_client_info->r_json))
    {
        DMCLOG_D("### error:post data is not a json string");
        res = -1;
        goto exit;
    }
    JObj *header_json = JSON_GET_OBJECT(p_client_info->r_json,"header");
    if(header_json == NULL)
    {
        DMCLOG_D("access NULL");
        res = -1;
        goto exit;
    }
    int cmd = JSON_GET_OBJECT_VALUE(JSON_GET_OBJECT(header_json,"cmd"),int);
    if(cmd == 115)//delete beat cmd
    {
        p_client_info->cmd = cmd;
    }else if(cmd == 139)
    {
        p_client_info->cmd = cmd;
    }
//    p_client_info->cmd = JSON_GET_OBJECT_VALUE(JSON_GET_OBJECT(header_json,"cmd"),int);//changed
    p_client_info->error = JSON_GET_OBJECT_VALUE(JSON_GET_OBJECT(header_json,"error"),int);
    JObj *token_json = JSON_GET_OBJECT(header_json,"token");
    if(token_json != NULL){
        p_client_info->token = JSON_GET_OBJECT_VALUE(token_json,int64);
    }
    JObj *seq_json = JSON_GET_OBJECT(header_json,"seq");
    if(seq_json != NULL)
        p_client_info->seq = JSON_GET_OBJECT_VALUE(seq_json,int);
    res = p_client_info->error;
exit:
    DMCLOG_D("res = %d,header.cmd = %d,p_client_info->error=%d",res,p_client_info->cmd,p_client_info->error);
    return res;
}

int Parser_FileGetList(ClientTheadInfo *p_client_info)
{
    int res = 0;
    unsigned i;
    JObj *data_json = JSON_GET_OBJECT(p_client_info->r_json,"data");
    if(data_json == NULL)
    {
        res = -1;
        goto EXIT;
    }
    
    JObj *filelist_json = JSON_GET_OBJECT(data_json,"filelist");
    if(filelist_json == NULL)
    {
        res = -1;
        goto EXIT;
    }
    char *name = NULL;
	int isFolder = 0;
    unsigned array_len = JSON_GET_ARRAY_LEN(filelist_json);
    //PPC_DIR *p_dir = (PPC_DIR *)p_client_info->p_dir;
    p_client_info->p_data = (struct dirent **)calloc(1,(array_len + 3)*sizeof(struct dirent *));

	//add . dir
	p_client_info->p_data[0] = (struct dirent *)calloc(1,sizeof(struct dirent));
	strcpy(p_client_info->p_data[0]->d_name,".");
	p_client_info->p_data[0]->d_type = DT_DIR;
	p_client_info->count++;

	//add .. dir
	p_client_info->p_data[1] = (struct dirent *)calloc(1,sizeof(struct dirent));
	strcpy(p_client_info->p_data[1]->d_name,"..");
	p_client_info->p_data[1]->d_type = DT_DIR;
	p_client_info->count++;

    for(i = 0;i < array_len;i++)
    {
        JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(filelist_json,i);
        if(para_json == NULL)
        {
            DMCLOG_D("access NULL");
            res = -1;
            goto EXIT;
        }
        JObj *name_json = JSON_GET_OBJECT(para_json,"name");
        if(name_json != NULL){
            name = JSON_GET_OBJECT_VALUE(name_json,string);
            p_client_info->p_data[p_client_info->count] = (struct dirent *)calloc(1,sizeof(struct dirent));
			DMCLOG_D("name: %s", name);
            strcpy(p_client_info->p_data[p_client_info->count]->d_name,name);
        }
		
		JObj *isFolder_json = JSON_GET_OBJECT(para_json,"name");
        if(isFolder_json != NULL){
			isFolder = JSON_GET_OBJECT_VALUE(isFolder_json,int);
			if(isFolder){
				p_client_info->p_data[p_client_info->count]->d_type = DT_DIR;
			}else{
				p_client_info->p_data[p_client_info->count]->d_type = DT_REG;
			}
		}
		p_client_info->count++;
    }
EXIT:
    return res;	
}
/*
 * 文件夹创建 cmd = 102
 */
int Parser_FileMkdir(ClientTheadInfo *p_client_info)
{
    return 0;
}
/*
 * 文件或文件夹重名 cmd = 103
 */
int Parser_FileRename(ClientTheadInfo *p_client_info)
{
    return 0;
}
/*
 * 判断文件或文件夹是否存在 cmd = 104
 */
int Parser_FileIsExist(ClientTheadInfo *p_client_info)
{
    int res = 0;
    JObj *data_json = JSON_GET_OBJECT(p_client_info->r_json,"data");
    if(data_json == NULL)
    {
        res = 0;
        goto EXIT;
    }
    JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
    if(para_json == NULL)
    {
        res = -1;
        goto EXIT;
    }
    JObj *status_json = JSON_GET_OBJECT(para_json,"status");
    if(status_json == NULL)
    {
        res = -1;
        goto EXIT;
    }
     p_client_info->status = JSON_GET_OBJECT_VALUE(status_json,int);
EXIT:
    return res;	
}
/*
 * 获取文件或文件夹属性 cmd = 105
 */
int Parser_FileGetAttr(ClientTheadInfo *p_client_info)
{
    int res = 0;
	_int64_t usize = 0;
    JObj *data_json = JSON_GET_OBJECT(p_client_info->r_json,"data");
    if(data_json == NULL)
    {
        res = -1;
        goto EXIT;
    }
    JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
    if(para_json == NULL)
    {
        res = -1;
        goto EXIT;
    }
    struct stat *st = (struct stat *)p_client_info->st;
    JObj *size_json = JSON_GET_OBJECT(para_json,"fileSize");
    if(size_json != NULL)
    {
        usize = JSON_GET_OBJECT_VALUE(size_json,int64);
        st->st_size = (_int64_t)usize;
        DMCLOG_D("p_client_info->file_length = %lld", st->st_size);
    }
    JObj *time_json = JSON_GET_OBJECT(para_json,"data");
    if(time_json != NULL)
    {
        st->st_mtime = JSON_GET_OBJECT_VALUE(time_json,int);
    }
    JObj *mode_json = JSON_GET_OBJECT(para_json,"mode");
    if(mode_json != NULL)
    {
        st->st_mode = JSON_GET_OBJECT_VALUE(mode_json,int);
    }

	JObj *nlink_json = JSON_GET_OBJECT(para_json,"nlink");
    if(nlink_json != NULL)
    {
        st->st_nlink = JSON_GET_OBJECT_VALUE(nlink_json,int);
    }
	JObj *ino_json = JSON_GET_OBJECT(para_json,"ino");
    if(nlink_json != NULL)
    {
        st->st_ino = JSON_GET_OBJECT_VALUE(ino_json,int);
    }
	JObj *dev_json = JSON_GET_OBJECT(para_json,"dev");
    if(dev_json != NULL)
    {
        st->st_dev = JSON_GET_OBJECT_VALUE(dev_json,int);
    }
	JObj *rdev_json = JSON_GET_OBJECT(para_json,"rdev");
    if(rdev_json != NULL)
    {
        st->st_rdev = JSON_GET_OBJECT_VALUE(rdev_json,int);
    }
	JObj *uid_json = JSON_GET_OBJECT(para_json,"uid");
    if(uid_json != NULL)
    {
        st->st_uid = JSON_GET_OBJECT_VALUE(uid_json,int);
    }
	JObj *gid_json = JSON_GET_OBJECT(para_json,"gid");
    if(gid_json != NULL)
    {
        st->st_gid = JSON_GET_OBJECT_VALUE(gid_json,int);
    }
	JObj *blksize_json = JSON_GET_OBJECT(para_json,"blksize");
    if(blksize_json != NULL)
    {
        st->st_blksize = JSON_GET_OBJECT_VALUE(blksize_json,int);
    }
	JObj *blocks_json = JSON_GET_OBJECT(para_json,"blocks");
    if(blocks_json != NULL)
    {
        st->st_blocks = JSON_GET_OBJECT_VALUE(blocks_json,int);
    }
	JObj *atime_json = JSON_GET_OBJECT(para_json,"atime");
    if(atime_json != NULL)
    {
        st->st_atime = JSON_GET_OBJECT_VALUE(atime_json,int);
    }
	JObj *ctime_json = JSON_GET_OBJECT(para_json,"ctime");
    if(ctime_json != NULL)
    {
        st->st_ctime = JSON_GET_OBJECT_VALUE(ctime_json,int);
    }
    
EXIT:
    return res;
}
/*
 *文件或文件夹删除 cmd = 109
 */
int parser_FileDelete(ClientTheadInfo *p_client_info)
{
    return 0;
}

int parser_FileCopy(ClientTheadInfo *p_client_info)
{
    return 0;
}

int parser_FileMove(ClientTheadInfo *p_client_info)
{
    return 0;
}

int parser_FileUtimensat(ClientTheadInfo *p_client_info)
{
    return 0;
}

/*
 * 文件下载 cmd = 106
 */
int Parser_FileDownload(ClientTheadInfo *p_client_info)
{
    int res = 0;
	_int64_t usize = 0;
    JObj *data_json = JSON_GET_OBJECT(p_client_info->r_json,"data");
    if(data_json == NULL)
    {
        DMCLOG_D("access NULL");
        res = -1;
        goto EXIT;
    }
    
    JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
    if(para_json == NULL)
    {
        DMCLOG_D("access NULL");
        res = -1;
        goto EXIT;
    }
    JObj *contentLength_json = JSON_GET_OBJECT(para_json,"contentLength");
    if(contentLength_json == NULL)
    {
        DMCLOG_D("access NULL");
        res = -1;
        goto EXIT;
    }
    p_client_info->content_len = JSON_GET_OBJECT_VALUE(contentLength_json,int64);
	if(p_client_info->content_len < 0){
		usize = JSON_GET_OBJECT_VALUE(contentLength_json, int);
		p_client_info->content_len = (_int64_t)usize;
	}
	DMCLOG_D("content_len: %lld", p_client_info->content_len);
EXIT:
    return res;
}
/*
 * 上传命令cmd = 107
 */
int Parser_FileUpload(ClientTheadInfo *p_client_info)
{
    int res = 0;
    _int64_t usize = 0;
    JObj *data_json = JSON_GET_OBJECT(p_client_info->r_json,"data");
    if(data_json == NULL)
    {
        DMCLOG_D("access NULL");
        res = -1;
        goto EXIT;
    }
    
    JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
    if(para_json == NULL)
    {
        DMCLOG_D("access NULL");
        res = -1;
        goto EXIT;
    }
    JObj *contentLength_json = JSON_GET_OBJECT(para_json,"contentLength");
    if(contentLength_json == NULL)
    {
        DMCLOG_D("access NULL");
        res = -1;
        goto EXIT;
    }
    p_client_info->content_len = JSON_GET_OBJECT_VALUE(contentLength_json,int64);
	DMCLOG_D("sizeof(size_t): %d", sizeof(size_t));
    if(p_client_info->content_len < 0){
        usize = JSON_GET_OBJECT_VALUE(contentLength_json, int);
        p_client_info->content_len = (_int64_t)usize;
    }
    DMCLOG_D("content_len: %lld", p_client_info->content_len);
EXIT:
    return res;
}

/*
 * 创建符号链接 
 */
int Parser_FileSymlink(ClientTheadInfo *p_client_info)
{
    return 0;
}

/*
 * 获取链接地址
 */
int Parser_FileReadlink(ClientTheadInfo *p_client_info)
{
	int res = 0;
    JObj *data_json = JSON_GET_OBJECT(p_client_info->r_json,"data");
    if(data_json == NULL){
        res = 0;
        goto EXIT;
    }
    JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
    if(para_json == NULL){
        res = -1;
        goto EXIT;
    }
    JObj *path_json = JSON_GET_OBJECT(para_json,"path");
    if(path_json == NULL){
        res = -1;
        goto EXIT;
    }

	char *link_path = NULL;
    link_path = JSON_GET_OBJECT_VALUE(path_json, string);
	if(link_path == NULL){
		res = -1;
        goto EXIT;
	}

	p_client_info->path = (char *)calloc(1, strlen(link_path));
	if(p_client_info->path == NULL){
		res = -1;
        goto EXIT;
	}

	strcpy(p_client_info->path, link_path);
EXIT:
    return res;	
}

/*
 * 创建硬链接 
 */
int Parser_FileLink(ClientTheadInfo *p_client_info)
{
    return 0;
}

/*
 * 文件截断
 */
int Parser_FileFtruncate(ClientTheadInfo *p_client_info)
{
    return 0;
}

/*
 * 文件空间预分配
 */
int Parser_FileFallocate(ClientTheadInfo *p_client_info)
{
    return 0;
}

/*
 * 获取文件系统信息
 */
int Parser_FileStatvfs(ClientTheadInfo *p_client_info)
{
	int res = 0;
    JObj *data_json = JSON_GET_OBJECT(p_client_info->r_json,"data");
    if(data_json == NULL){
        res = 0;
        goto EXIT;
    }
    JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
    if(para_json == NULL){
        res = -1;
        goto EXIT;
    }

    JObj *f_bsize_json = JSON_GET_OBJECT(para_json,"f_bsize");
	JObj *f_frsize_json = JSON_GET_OBJECT(para_json,"f_frsize");
	JObj *f_fsid_json = JSON_GET_OBJECT(para_json,"f_fsid");
	JObj *f_flag_json = JSON_GET_OBJECT(para_json,"f_flag");
	JObj *f_namemax_json = JSON_GET_OBJECT(para_json,"f_namemax");
	
	JObj *f_blocks_json = JSON_GET_OBJECT(para_json,"f_blocks");
	JObj *f_bfree_json = JSON_GET_OBJECT(para_json,"f_bfree");
	JObj *f_bavail_json = JSON_GET_OBJECT(para_json,"f_bavail");
	JObj *f_files_json = JSON_GET_OBJECT(para_json,"f_files");
	JObj *f_ffree_json = JSON_GET_OBJECT(para_json,"f_ffree");
	JObj *f_favail_json = JSON_GET_OBJECT(para_json,"f_favail");
	
	if(f_bsize_json==NULL || f_frsize_json==NULL || f_fsid_json==NULL || f_flag_json==NULL || 
		f_namemax_json==NULL || f_blocks_json==NULL || f_bfree_json==NULL || f_bavail_json==NULL ||
		f_files_json==NULL || f_ffree_json==NULL || f_favail_json==NULL){
        res = -1;
        goto EXIT;
    }

	p_client_info->statvfs_buf->f_bsize = JSON_GET_OBJECT_VALUE(f_bsize_json,int);
	p_client_info->statvfs_buf->f_frsize = JSON_GET_OBJECT_VALUE(f_frsize_json,int);
	p_client_info->statvfs_buf->f_fsid = JSON_GET_OBJECT_VALUE(f_fsid_json,int);
	p_client_info->statvfs_buf->f_flag = JSON_GET_OBJECT_VALUE(f_flag_json,int);
	p_client_info->statvfs_buf->f_namemax = JSON_GET_OBJECT_VALUE(f_namemax_json,int);
	
	p_client_info->statvfs_buf->f_blocks = JSON_GET_OBJECT_VALUE(f_blocks_json,int64);
	p_client_info->statvfs_buf->f_bfree = JSON_GET_OBJECT_VALUE(f_bfree_json,int64);
	p_client_info->statvfs_buf->f_bavail = JSON_GET_OBJECT_VALUE(f_bavail_json,int64);
	p_client_info->statvfs_buf->f_files = JSON_GET_OBJECT_VALUE(f_files_json,int64);
	p_client_info->statvfs_buf->f_ffree = JSON_GET_OBJECT_VALUE(f_ffree_json,int64);
	p_client_info->statvfs_buf->f_favail = JSON_GET_OBJECT_VALUE(f_favail_json,int64);

EXIT:
    return res;	
}


int file_parse_process(ClientTheadInfo *p_client_info)
{
    uint8_t i = 0;
    uint8_t switch_flag = 0;
    int ret = file_parse_header_json(p_client_info);
    if(ret != 0)
    {
        if(p_client_info->r_json != NULL)
            JSON_PUT_OBJECT(p_client_info->r_json);
        return ret;
    }
    for(i = 0; i<FILE_TAGHANDLE_NUM; i++)
    {
        if(p_client_info->cmd == all_file_handle[i].tag)
        {
            ret = all_file_handle[i].parsefun(p_client_info);
            switch_flag = 1;
            break;
        }
    }
    if(switch_flag == 0)
    {
        strcpy(p_client_info->retstr,"input cmd is not finished!");
    }
    if(p_client_info->r_json != NULL)
        JSON_PUT_OBJECT(p_client_info->r_json);
    return ret;
}
