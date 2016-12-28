/*
 * =============================================================================
 *
 *       Filename:  file_json.c
 *
 *    Description:  process json data
 *
 *        Version:  1.0
 *        Created:  2015/04/03 14:33
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver, 
 *   Organization:  
 *
 * =============================================================================*/
#include <stdio.h>
#include <utime.h> 
#include <time.h>
#include <sys/sysinfo.h>
#include "file_json.h"
#include "base.h"
#include "router_inotify.h"
#include "socket_uart.h"
#include "router_defs.h"
#include "config.h"
#include "util.h"
#include "db_prcs.h"


typedef int (*API_FUNC)(struct conn *c);
typedef int (*PARSE_FUN)(struct conn *c);


typedef struct _file_tag_handle
{
 uint16_t tag;
 API_FUNC tagfun;
 PARSE_FUN parsefun;
}file_tag_handle;

file_tag_handle all_file_handle[]=
{
	{FN_DM_GET_VERSION,dm_get_version,Parser_GetVersion},
	{FN_DM_REGISTER,dm_register,Parser_register},
	{FN_DM_LOGIN,dm_login,Parser_Login},
	{FN_DM_LOGOUT,dm_logout,Parser_Logout},
	{FN_DEL_CLIENT_INFO,dm_del_client_info,Parser_DelClientInfo},
	
	{FN_FILE_GET_STORAGE,DM_FileGetStorage,Parser_FileGetStorage},
	{FN_FILE_GET_LIST,DM_FileGetList,Parser_FileGetList},
	{FN_FILE_MKDIR,DM_FileMkdir,Parser_FileMkdir},
	{FN_FILE_RENAME,DM_FileRename,Parser_FileRename},
	{FN_FILE_IS_EXIST,DM_FileIsExist,Parser_FileIsExist},
	{FN_FILE_GET_ATTR,DM_FileGetAttr,Parser_FileIsExist},
	{FN_FILE_DOWNLOAD,DM_FileDownload,Parser_FileDownload},
	{FN_FILE_UPLOAD,DM_FileUpload,Parser_FileUpload},
	{FN_FILE_CHECKUPLOAD,DM_FileCheckUpload,Parser_FileCheckUpload},
	{FN_FILE_DELETE,DM_FileDelete,parser_FileDelete},
    {FN_FILE_COPY,DM_FileCopy,parser_FileCopy},
    {FN_FILE_MOVE,DM_FileMove,parser_FileMove},
    {FN_FILE_SEARCH,DM_FileSearch,parser_FileSearch},
	{FN_FILE_UTIMENSAT,DM_FileUtimensat,parser_FileUtimensat},
	{FN_FILE_SYMLINK,DM_FileSymlink,parser_FileSymlink},
	{FN_FILE_LINK,DM_FileLink,parser_FileLink},
	{FN_FILE_READLINK,DM_FileReadlink,parser_FileReadlink},
	{FN_FILE_STATVFS,DM_FileStatvfs,parser_FileStatvfs},
	//{FN_FILE_FTRUNCATE,DM_FileFtruncate,parser_FileFtruncate},
	//{FN_FILE_FALLOCATE,DM_FileFallocate,parser_FileFallocate},
	
    
	{FN_FILE_GET_LSIT_BY_TYPE,DM_FileGetListByType,Parser_FileGetListByType},
	{FN_FILE_GET_DIR_BY_TYPE,DM_FileGetListByType,Parser_FileGetFilePathByType},
	{FN_FILE_GET_LIST_BY_PATH,DM_FileGetListByType,Parser_FileGetFileListByPath},
	{FN_FILE_GET_ALBUM_LIST,DM_FileGetListByType,Parser_FileGetFilePathByType},
	{FN_FILE_DEL_LIST_BY_PATH,DM_FileDelListByPath,Parser_FileDelFileListByPath},
	{FN_FILE_GET_BACKUP_FILE,DM_FileGetBackupFile,Parser_FileGetBackupFile},
	{FN_FILE_CHECK_BACKUP_FILE,DM_FileCheckBackupFile,Parser_FileCheckBackupFile},
	{FN_FILE_CHECK_BACKUP_FILE_LIST,DM_FileCheckBackupFileList,Parser_FileCheckBackupFileList},
	{FN_FILE_IS_BACKUP_EXISTED,DM_FileIsBackupExisted,Parser_FileIsBackupFile},

	{FN_DM_ROUTER_GET_FUNC_LIST,dm_router_get_func_list,Parser_RouterGetFuncList},
	{FN_ROUTER_SET_PASSWORD, _dm_set_password, Parser_RouterSetPassword},
	{FN_ROUTER_RESET_PASSWORD, _dm_reset_password, Parser_RouterResetPassword},
	{FN_ROUTER_GET_PWD_STATUS, _dm_get_pwd_status, Parser_RouterResetPassword},
	{FN_ROUTER_SET_ADD_NETWORK, _dm_set_add_network, Parser_RouterSetAddNetwork},
	{FN_ROUTER_GET_WIFI_INFO,_dm_get_wifi_settings,Parser_RouterGetWifiSettings},
	{FN_ROUTER_SET_WIFI_INFO,_dm_set_wifi_settings,Parser_RouterSetWifiSettings},
	{FN_ROUTER_GET_REMOTE_AP_INFO,_dm_get_remote_ap_info,Parser_RouterGetRemoteInfo},
	{FN_ROUTER_SET_REMOTE_AP_INFO,_dm_set_remote_ap_info,Parser_RouterSetRemoteInfo},
	{FN_ROUTER_GET_CONNECT_MODE,_dm_get_wlan_con_mode,Parser_RouterGetWlanConMode},
	{FN_ROUTER_GET_WIRED_INFO,_dm_get_wired_con_mode,Parser_RouterGetWireConMode},
	{FN_ROUTER_SET_WIRED_INFO,_dm_set_wired_con_mode,Parser_RouterSetWireConMode},
	{FN_ROUTER_GET_REMOTE_AP_LIST,_dm_get_wlan_list,Parser_RouterGetWlanList},
	{FN_ROUTER_GET_POWER_INFO,_dm_get_power,Parser_RouterGetPower},
	{FN_ROUTER_GET_DISK_INFO,DM_FileGetStorage,Parser_FileGetStorage},
	{FN_ROUTER_GET_CLIENT_STATUS, _dm_get_client_status,Parser_RouterGetClientStatus},
	{FN_ROUTER_SET_CLIENT_STATUS, _dm_set_client_status,Parser_RouterSetClientStatus},
	{FN_ROUTER_SET_UDISK_UPGRADE, _dm_udisk_upgrade,Parser_RouterUdiskUpgrade},
	{FN_ROUTER_SET_TIME_SYNC, _dm_sync_time,Parser_RouterSyncTime},
	{FN_ROUTER_SET_SYSTEM_SYNC, _dm_sync_system, Parser_RouterSyncSystem},
	{FN_ROUTER_GET_FIRMWARE_VERSION, _dm_get_fw_version,Parser_RouterGetFwVersion},
	{FN_ROUTER_GET_OTA_INFO, _dm_get_ota_info,Parser_RouterGetOtaInfo}, 
	{FN_ROUTER_GET_VERSION_FLAG, _dm_get_version_flag,Parser_RouterGetVersionFlag},
	{FN_ROUTER_SET_VERSION_FLAG, _dm_set_version_flag,Parser_RouterSetVersionFlag},
	{FN_ROUTER_SET_UPLOAD_FIRMWARE, _dm_set_upload_fw, Parser_RouterSetUploadFw},
	{FN_ROUTER_SET_UPGRADE_FIRMWARE, _dm_set_upgrade_fw, Parser_RouterSetUpgradeFw},
	{FN_ROUTER_SET_FORGET_WIFI_INFO, _dm_set_forget_wifi_info, Parser_RouterSetForgetWifiInfo},
	{FN_ROUTER_GET_DISK_DIRECTION, _dm_get_disk_direction, Parser_RouterGetDiskDirection},
	{FN_ROUTER_SET_DISK_DIRECTION, _dm_set_disk_direction, Parser_RouterSetDiskDirection},
	{FN_ROUTER_GET_WIFI_TYPE, _dm_get_wifi_type, Parser_RouterGetWifiType},
	{FN_ROUTER_SET_WIFI_TYPE, _dm_set_wifi_type, Parser_RouterSetWifiType},
};

#define FILE_TAGHANDLE_NUM (sizeof(all_file_handle)/sizeof(all_file_handle[0]))

int dm_get_version(struct conn *c)
{
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	strcpy(c->ver,get_sys_dm_router_version());
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* ver_info = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(ver_info, "ver",JSON_NEW_OBJECT(c->ver,string));
	JSON_ARRAY_ADD_OBJECT (response_data_array,ver_info);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int dm_login(struct conn *c)
{
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();

	int res = _add_dev_to_list(c->client_ip,c->client_port,c->statusFlag);
	if(res < 0)
	{
		c->error = ERROR_SET_STATUS_CHANGED;
	}
	
	// 0:get version
	strcpy(c->ver,get_sys_dm_router_version());
	JObj* ver_info = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(ver_info, "ver",JSON_NEW_OBJECT(c->ver,string));
	JSON_ARRAY_ADD_OBJECT (response_data_array,ver_info);

	// 1:get status changed
	c->statusFlag = dm_get_status_changed();
	if(c->statusFlag < 0)
	{
		c->error = ERROR_GET_STATUS_CHANGED;
		goto EXIT;
	}
	JObj* status_info = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(status_info, "statusFlag",JSON_NEW_OBJECT(c->statusFlag,int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,status_info);

	// 2:get function list
	JObj* func_list = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(func_list, "funcListFlag",JSON_NEW_OBJECT(get_func_list_flag(),int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,func_list);

	//3:get database seq
	JObj* database_sign = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(database_sign, "databaseSign",JSON_NEW_OBJECT(get_database_sign(),int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,database_sign);

	// 4 :get ota info
	dm_ota_info m_ota_info;
	memset(&m_ota_info,0,sizeof(dm_ota_info));
	int ret = dm_get_ota_info(&m_ota_info);
	if(ret >= 0)
	{
		JObj* ota_json = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(ota_json, "customCode",JSON_NEW_OBJECT(m_ota_info.customCode,string));
		JSON_ADD_OBJECT(ota_json, "versionCode",JSON_NEW_OBJECT(m_ota_info.versionCode,string));
		JSON_ADD_OBJECT(ota_json, "version_flag",JSON_NEW_OBJECT(m_ota_info.version_flag,int));
		JSON_ARRAY_ADD_OBJECT (response_data_array,ota_json);
	}

EXIT:
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int dm_register(struct conn *c)
{
	int ret = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	int user_id = 0;
	ret = handle_user_table_insert(c->username,c->password,&user_id);
	if(ret != RET_SUCCESS)
	{
		DMCLOG_E("user table insert error");
		c->error = ret;
		goto EXIT;
	}
	
	char bucket_name[32] = {0};
	sprintf(bucket_name,"v_file_table_%d",user_id);
	ret = v_file_table_create(bucket_name);
	if(ret != RET_SUCCESS)
	{
		DMCLOG_E("v file table create error");
		c->error = ret;
		goto EXIT;
	}

	int bucket_id = 0;
	ret = handle_bucket_table_insert(bucket_name,user_id,&bucket_id);
	if(ret != RET_SUCCESS)
	{
		DMCLOG_E("buncket table insert error");
		c->error = ret;
		goto EXIT;
	}
	int autority = 7;
	int autority_id = 0;
	ret = handle_authority_table_insert(bucket_id,user_id,autority,&autority_id);
	if(ret != RET_SUCCESS)
	{
		DMCLOG_E("authority table insert error");
		c->error = ret;
		goto EXIT;
	}
	
	v_file_insert_t v_file_insert;
	memset(&v_file_insert,0,sizeof(v_file_insert_t));
	v_file_insert.cmd = V_FILE_TABLE_INSERT_INFO;
	v_file_insert.v_file_info.isDir = 1;
	S_STRNCPY(v_file_insert.bucket_name,bucket_name,MAX_BUCKET_NAME_LEN);
	sprintf(v_file_insert.v_file_info.path,"/%s/public",bucket_name);
	time_t now; //ʵ����time_t�ṹ    
	struct tm *timenow; //ʵ����tm�ṹָ��    
	time(&now);
	//time������ȡ���ڵ�ʱ��(���ʱ�׼ʱ��Ǳ���ʱ��)��Ȼ��ֵ��now    
	v_file_insert.v_file_info.atime = now;
	v_file_insert.v_file_info.ctime = now;
	v_file_insert.v_file_info.mtime = now;
	
	v_file_insert.v_file_info.isDir = 1;
	if(!*v_file_insert.v_file_info.uuid)
	{
		strcpy(v_file_insert.v_file_info.uuid,"13141314");
	}
	ret = _handle_v_file_table_insert(&v_file_insert);
	if(ret != 0)
	{
		DMCLOG_E("mkdir root file table error");
		c->error = ret;
		goto EXIT;
	}
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}


int dm_logout(struct conn *c)
{
	ENTER_FUNC();
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
	return 0;
}

int dm_del_client_info(struct conn *c)
{
	ENTER_FUNC();
	JObj* response_json=JSON_NEW_EMPTY_OBJECT();
	int res = _del_dev_from_list_by_ip(c->client_ip);
	if(res < 0)
	{
		DMCLOG_D("del client info error");
		c->error = ERROR_DEL_CLIENT_INFO;
		goto EXIT;
	}
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
	return 0;
}

int dm_router_get_func_list(struct conn *c)
{
	int res = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* status_info = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(status_info, "statusFlag",JSON_NEW_OBJECT(get_func_list_flag(),int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,status_info);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int DM_FileDownload(struct conn *c)
{
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	struct stat st;
	BucketObject *sObject = build_bucket_object(c->src_path,c->token);
	if(sObject == NULL)
	{
		DMCLOG_E("buile bucket object error");
		c->error = ERROR_FILE_COPY;
		goto EXIT;
	}
	
	if (_bfavfs_fstat(sObject, &st,(char *)c->token) != 0) {
		c->error = FILE_IS_NOT_EXIST;
		goto EXIT;
	
	} 
	//ppc_open 接口有可能打开的是目录
	#if 0
	else if (S_ISDIR(st.st_mode)&& c->src_path[strlen(c->src_path) - 1] != '/') {
		c->error = SERVER_MOVED_LOCATION;
		goto EXIT;
	}
	#endif
	DMCLOG_D("path = %s",c->src_path);
	if ((c->loc.chan.fd = _bfavfs_fopen(sObject,"r",(char *)c->token)) != NULL) {
		get_file(c, &st);
	} else {
		c->error = SERVER_ERROR;
	}
		
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* contentLength_json = JSON_NEW_EMPTY_OBJECT();
	DMCLOG_D("DM_File_Download, c->loc.content_len = %lld", c->loc.content_len);
	JSON_ADD_OBJECT(contentLength_json, "contentLength",JSON_NEW_OBJECT(c->loc.content_len,int64));
	JSON_ARRAY_ADD_OBJECT (response_data_array,contentLength_json);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	safe_free(sObject);
	return 0;
}

int DM_FileCheckUpload(struct conn *c)
{
	int res = 0;
	struct stat	st;
	unsigned i = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	BucketObject *sObject = build_bucket_object(c->cfg_path,c->token);
	if(sObject == NULL)
	{
		DMCLOG_E("buile bucket object error");
		c->error = ERROR_FILE_COPY;
		goto EXIT;
	}
	if(_bfavfs_fstat(sObject, &st,c->token) == 0)
	{
		DMCLOG_D("cfg path is exist");
		c->status = 1;
		c->dn = NULL;
		res = read_list_from_file(c->cfg_path,&c->dn,c->token);
		if(res < 0)
		{
			c->error = 108;
			goto EXIT;
		}
	}else{
		DMCLOG_D("cfg_path is not exist");
		c->status == 0;
	}
	 
	JObj* data_json = JSON_NEW_EMPTY_OBJECT();
	DMCLOG_D("file check upload,status = %d",c->status);
	if(c->status == 1)
	{
		JObj *response_data_array = JSON_NEW_ARRAY();
		struct record_dnode *p_dn = c->dn;
		for(i = 0;/*count - detected via !dn below*/;i++)
		{
			 if(!p_dn)
			 {
			 	DMCLOG_D("link is NULL");
				break;
			 }
			 JObj *upload_record_json =  JSON_NEW_EMPTY_OBJECT();
			 JSON_ADD_OBJECT(upload_record_json, "start", JSON_NEW_OBJECT(p_dn->start,int64));
			 JSON_ADD_OBJECT(upload_record_json, "end", JSON_NEW_OBJECT(p_dn->end,int64));
			 DMCLOG_D("i = %d,dn->index = %d",i,p_dn->index);
			 JSON_ARRAY_ADD_OBJECT (response_data_array,upload_record_json);
			 p_dn = p_dn->dn_next;
		}
		JSON_ADD_OBJECT(data_json, "count", JSON_NEW_OBJECT(i,int));
		JSON_ADD_OBJECT(data_json, "incompleteInfo", response_data_array);
	}
	destory_record_list(c->dn);
	JSON_ADD_OBJECT(data_json, "status", JSON_NEW_OBJECT(c->status,int));
	JSON_ADD_OBJECT(response_json, "data", data_json);
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int DM_FileUpload(struct conn *c)
{
	ENTER_FUNC();
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	BucketObject *dObject = NULL;
	BucketObject *sObject = build_bucket_object(c->tmp_path,c->token);
	if(sObject == NULL)
	{
		DMCLOG_E("buile bucket object error");
		c->error = ERROR_FILE_UPLOAD_CHECK;
		goto EXIT;
	}
	sObject->offset = c->offset;
	if ((c->loc.chan.vf = _bfavfs_fopen(sObject,"a",(char *)c->token)) == NULL) {
		c->loc.flags |= FLAG_CLOSED;
		c->error = CREATE_FILE_ERROR;
		goto EXIT;
	} else {
		c->loc.io_class = &io_file;
		c->loc.flags |= FLAG_W;
		DMCLOG_D("upload c->offset = %lld",c->offset);
		int res = bfavfs_fseek(c->loc.chan.vf, c->offset, SEEK_SET);
		if(res != 0)
		{
			DMCLOG_E("fseek %s error[%d]",c->tmp_path,errno);
			c->loc.flags |= FLAG_CLOSED;
			c->error = CREATE_FILE_ERROR;
			goto EXIT;
		}
		struct stat st;
		BucketObject *dObject = build_bucket_object(c->cfg_path,c->token);
	    if(dObject == NULL)
		{
			DMCLOG_E("buile bucket object error");
			c->error = ERROR_FILE_UPLOAD_CHECK;
			goto EXIT;
		}
		if(_bfavfs_fstat(dObject, &st,(char *)c->token) != 0)
		{
			int i = 0;
			off_t percent = c->fileSize/THREAD_COUNT;
			c->dn = NULL;
			for(; i < THREAD_COUNT;i++)
		    {
		        if(i == THREAD_COUNT - 1)
					add_record_to_list(&c->dn,i,i * percent,c->fileSize);
		        else
					add_record_to_list(&c->dn,i,i * percent,(i + 1)* percent);
		    }
		}else{
			c->dn = NULL;
			if(read_list_from_file(c->cfg_path,&c->dn,c->token) < 0)
			{
				c->error = ERROR_FILE_UPLOAD_CHECK;
				goto EXIT;
			}
		}
		DMCLOG_D("cfg_path = %s",c->cfg_path);
		if((c->record_fd = _bfavfs_fopen(dObject,"w+",(char *)c->token))== NULL)
		{
			DMCLOG_E("open file error[errno = %d]",errno);
			c->error = ERROR_FILE_UPLOAD_CHECK;
			goto EXIT;
		}
	}
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* contentLength_json = JSON_NEW_EMPTY_OBJECT();
	//c->rem.content_len = 350364;
	DMCLOG_D("DM_FileUpload, c->rem.content_len = %lld", c->rem.content_len);
	JSON_ADD_OBJECT(contentLength_json, "contentLength",JSON_NEW_OBJECT(c->rem.content_len,int64));
	JSON_ARRAY_ADD_OBJECT (response_data_array,contentLength_json);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	safe_free(sObject);
	safe_free(dObject);
	return 0;
}

/*
 *  cmd = 100  get storage info
 */
int DM_FileGetStorage(struct conn *c)
{
	ENTER_FUNC();
	JObj *response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* data_json=JSON_NEW_EMPTY_OBJECT();
	JObj *disk_info_array = JSON_NEW_ARRAY();
	int i = 0;
	all_disk_t mAll_disk_t;
	memset(&mAll_disk_t,0,sizeof(all_disk_t));
	int res = dm_get_storage(&mAll_disk_t);
	if(res != 0)
	{
		c->error = ERROR_GET_DISK_INFO;
		goto EXIT;
	}
	token_dnode_t *token_dnode = (token_dnode_t *)c->token;
	res = handle_user_table_query(c->username,c->password,&token_dnode->user_id);
	if(res != 0)
	{
		DMCLOG_E("user is not exist");
		c->error = USERNAME_NOT_FOUND;
		goto EXIT;
	}
 	DMCLOG_D("drive_count = %d",mAll_disk_t.count);
	for(i = 0;i < mAll_disk_t.count;i++)
	{
		JObj* drive_info = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(drive_info, "displayName",JSON_NEW_OBJECT(c->username,string));
		JSON_ADD_OBJECT(drive_info, "path",JSON_NEW_OBJECT("/",string));
		JSON_ADD_OBJECT(drive_info, "size",JSON_NEW_OBJECT(mAll_disk_t.disk[i].total_size,int64));
		JSON_ADD_OBJECT(drive_info, "avSize",JSON_NEW_OBJECT(mAll_disk_t.disk[i].free_size,int64));
		JSON_ADD_OBJECT(drive_info, "diskType",JSON_NEW_OBJECT(mAll_disk_t.disk[i].type,int));
		JSON_ADD_OBJECT(drive_info, "auth",JSON_NEW_OBJECT(mAll_disk_t.disk[i].auth,int));
		JSON_ADD_OBJECT(drive_info, "fsType",JSON_NEW_OBJECT(mAll_disk_t.disk[i].fstype,string));
		JSON_ARRAY_ADD_OBJECT (disk_info_array,drive_info);	
	}
	JSON_ADD_OBJECT(data_json, "diskInfoList", disk_info_array);
	JSON_ADD_OBJECT(response_json, "data", data_json);
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
	return 0;
}

/*
 * 获取文件列表 cmd = 101
 */
int DM_FileGetList(struct conn *c)
{
	ENTER_FUNC();
	struct stat st;
	BucketObject *sObject = build_bucket_object(c->src_path,c->token);
	if(sObject == NULL)
	{
		DMCLOG_E("buile bucket object error");
		c->error = ERROR_GET_FILE_LIST;
		goto EXIT;
	}
	
	if(_bfavfs_fstat(sObject, &st,(char *)c->token) != 0 || S_ISDIR(st.st_mode) != 1)
	{
		DMCLOG_D("S_ISDIR(st.st_mode) = %d",S_ISDIR(st.st_mode));
		c->loc.flags |= FLAG_CLOSED;
		c->error = ERROR_GET_FILE_LIST;
		goto EXIT;
	}

	DMCLOG_D("src_path = %s",c->src_path);
	get_type(c);
EXIT:	
	EXIT_FUNC();
	return 0;
}

/*
 * 文件夹创建 cmd = 102
 */
int DM_FileMkdir(struct conn *c)
{
	int res = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	BucketObject *sObject = build_bucket_object(c->src_path,c->token);
	if(sObject == NULL)
	{
		DMCLOG_E("buile bucket object error");
		c->error = ERROR_MKDIR;
		goto EXIT;
	}
	if((res = _bfavfs_mkdir(sObject,(char *)c->token)) != RET_SUCCESS)
    {
        DMCLOG_D("mkdir(%s) failed", c->src_path);
        c->error = ERROR_MKDIR;
		goto EXIT;
    }
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	safe_free(sObject);
	return 0;
}
/*
 * 文件或文件夹重名 cmd = 103
 */
int DM_FileRename(struct conn *c)
{
	int res = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	struct stat st;
	BucketObject *sObject = build_bucket_object(c->src_path,c->token);
	if(sObject == NULL)
	{
		DMCLOG_E("buile bucket object error");
		c->error = ERROR_FILE_COPY;
		goto EXIT;
	}

	BucketObject *dObject = build_bucket_object(c->des_path,c->token);
	if(dObject == NULL)
	{
		DMCLOG_E("buile bucket object error");
		c->error = ERROR_FILE_COPY;
		goto EXIT;
	}
	if (_bfavfs_fstat(sObject, &st,(char *)c->token) != 0) {
		c->error = FILE_IS_NOT_EXIST;
		goto EXIT;
	} 
	
	if((res = _bfavfs_frename( sObject,dObject,(char *)c->token)) != RET_SUCCESS)
    {
        DMCLOG_D("rename_(%s) failed, ret(0x%x)", c->des_path, res);
        c->error = ERROR_RENAME;
		goto EXIT;
    }
	
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	safe_free(sObject);
	safe_free(dObject);
	return 0;
}
/*
 * cmd = 104
 */
int DM_FileIsExist(struct conn *c)
{
	int res = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	struct stat st;
	
	BucketObject *sObject = build_bucket_object(c->src_path,c->token);
	if(sObject == NULL)
	{
		DMCLOG_E("buile bucket object error");
		c->error = FILE_IS_NOT_EXIST;
		goto EXIT;
	}
	if(_bfavfs_fstat(sObject, &st,(char *)c->token) != 0) {
		c->error = FILE_IS_NOT_EXIST;
		goto EXIT;
	} 
	
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	safe_free(sObject);
	return 0;
}

int DM_FileGetAttr(struct conn *c)
{
	int res = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	struct stat st;
	memset(&st,0,sizeof(struct stat));
	BucketObject *sObject = build_bucket_object(c->src_path,c->token);
	if(sObject == NULL)
	{
		DMCLOG_E("buile bucket object error");
		c->error = FILE_IS_NOT_EXIST;
		goto EXIT;
	}
	if(_bfavfs_fstat(sObject, &st,(char *)c->token) != 0) {
		c->error = FILE_IS_NOT_EXIST;
		goto EXIT;
	} 
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* file_info = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(file_info, "fileSize",JSON_NEW_OBJECT(st.st_size,int64));
	JSON_ADD_OBJECT(file_info, "data",JSON_NEW_OBJECT(st.st_mtime,int));
	JSON_ADD_OBJECT(file_info, "mode",JSON_NEW_OBJECT(st.st_mode,int));
	
	JSON_ADD_OBJECT(file_info, "nlink",JSON_NEW_OBJECT(st.st_nlink,int));
	JSON_ADD_OBJECT(file_info, "ino",JSON_NEW_OBJECT(st.st_ino,int));	
	JSON_ADD_OBJECT(file_info, "dev",JSON_NEW_OBJECT(st.st_dev,int));
	JSON_ADD_OBJECT(file_info, "uid",JSON_NEW_OBJECT(st.st_uid,int));
	JSON_ADD_OBJECT(file_info, "gid",JSON_NEW_OBJECT(st.st_gid,int));
	JSON_ADD_OBJECT(file_info, "rdev",JSON_NEW_OBJECT(st.st_rdev,int));
	JSON_ADD_OBJECT(file_info, "blksize",JSON_NEW_OBJECT(st.st_blksize,int));
	JSON_ADD_OBJECT(file_info, "blocks",JSON_NEW_OBJECT(st.st_blocks,int));
	JSON_ADD_OBJECT(file_info, "atime",JSON_NEW_OBJECT(st.st_atime,int));
	JSON_ADD_OBJECT(file_info, "ctime",JSON_NEW_OBJECT(st.st_ctime,int));
	
	JSON_ARRAY_ADD_OBJECT(response_data_array,file_info);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	safe_free(sObject);
	return 0;
}


int DM_FileDelete(struct conn *c)
{
	int res = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	struct stat st;
	BucketObject *sObject = build_bucket_object(c->src_path,c->token);
	if(sObject == NULL)
	{
		DMCLOG_E("buile bucket object error");
		c->error = FILE_IS_NOT_EXIST;
		goto EXIT;
	}
	if (_bfavfs_fstat(sObject, &st,(char *)c->token) != 0) {
		c->error = FILE_IS_NOT_EXIST;
		goto EXIT;
	} 
	
	if((res = _bfavfs_remove(sObject,(char *)c->token)) != RET_SUCCESS)
    {
        DMCLOG_D("delete(%s) failed", c->src_path);
        c->error = ERROR_FILE_DELETE;
		goto EXIT;
    }
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	safe_free(sObject);
	return 0;
}


int DM_FileCopy(struct conn *c)
{
    int res = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	struct stat st;
	BucketObject *sObject = build_bucket_object(c->src_path,c->token);
	if(sObject == NULL)
	{
		DMCLOG_E("buile bucket object error");
		c->error = ERROR_FILE_COPY;
		goto EXIT;
	}

	BucketObject *dObject = build_bucket_object(c->des_path,c->token);
	if(dObject == NULL)
	{
		DMCLOG_E("buile bucket object error");
		c->error = ERROR_FILE_COPY;
		goto EXIT;
	}
	if (_bfavfs_fstat(sObject, &st,(char *)c->token) != 0||_bfavfs_fstat(dObject, &st,(char *)c->token) != 0) {
		c->error = FILE_IS_NOT_EXIST;
		goto EXIT;
	} 
		
	if((res = _bfavfs_fcopy(sObject, dObject,(char *)c->token)) != RET_SUCCESS)
    {
        DMCLOG_D("copy(%s) failed, ret(0x%x)", c->des_path, res);
        c->error = ERROR_FILE_COPY;
		goto EXIT;
    }
	
EXIT:
	safe_free(dObject);
	safe_free(sObject);
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
    return 0;
}

int DM_FileMove(struct conn *c)
{
	int res = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	struct stat st;
	BucketObject *sObject = build_bucket_object(c->src_path,c->token);
	if(sObject == NULL)
	{
		DMCLOG_E("buile bucket object error");
		c->error = ERROR_FILE_MOVE;
		goto EXIT;
	}

	BucketObject *dObject = build_bucket_object(c->des_path,c->token);
	if(dObject == NULL)
	{
		DMCLOG_E("buile bucket object error");
		c->error = ERROR_FILE_MOVE;
		goto EXIT;
	}
	if (_bfavfs_fstat(sObject, &st,(char *)c->token) != 0||_bfavfs_fstat(dObject, &st,(char *)c->token) != 0) {
		c->error = FILE_IS_NOT_EXIST;
		goto EXIT;
	} 
	
	if((res = _bfavfs_fmove(sObject, dObject,(char *)c->token)) != RET_SUCCESS)
    {
        DMCLOG_D("move(%s) failed, ret(0x%x)", c->des_path, res);
        c->error = ERROR_FILE_MOVE;
		goto EXIT;
    }
	
EXIT:
	safe_free(dObject);
	safe_free(sObject);
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
    return 0;
}

int DM_FileSearch(struct conn *c)
{
	return get_type(c);
}

int DM_FileGetListByType(struct conn *c)
{
	return get_type(c);
}

int DM_FileDelListByPath(struct conn *c)
{
	int res = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	struct stat st;
	BucketObject *sObject = build_bucket_object(c->src_path,c->token);
	if(sObject == NULL)
	{
		DMCLOG_E("buile bucket object error");
		c->error = FILE_IS_NOT_EXIST;
		goto EXIT;
	}
	if (_bfavfs_fstat(sObject, &st,(char *)c->token) != 0) {
		c->error = FILE_IS_NOT_EXIST;
		goto EXIT;
	} 
	sObject->file_type = c->fileType;
	if((res = _bfavfs_remove(sObject,(char *)c->token)) != RET_SUCCESS)
    {
        DMCLOG_D("delete file type(%d) by %s failed",c->fileType, c->src_path);
        c->error = ERROR_FILE_DELETE;
		goto EXIT;
    }
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	safe_free(sObject);
	return 0;
}

int DM_FileGetBackupFile(struct conn *c)
{
	ENTER_FUNC();
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	int res = 0;
	struct stat	st;
	BucketObject *cObject = NULL;
	BucketObject *sObject = build_bucket_object(c->tmp_path,c->token);
	if(sObject == NULL)
	{
		DMCLOG_E("buile bucket object error");
		c->error = FILE_IS_NOT_EXIST;
		goto EXIT;
	}
	sObject->offset = c->offset;
	if (c->rem.content_len == 0) {
		c->error = LENGTH_REQUIRED;
		goto EXIT;
	} else if ((c->loc.chan.vf = _bfavfs_fopen(sObject,"a", c->token)) == NULL) {
		c->error = CREATE_FILE_ERROR;
		goto EXIT;
	} else {
		DMCLOG_D("backup file [%s]", c->des_path);
		S_STRNCPY(c->loc.chan.vf->uuid,c->file_uuid,MAX_FILE_UUID_LEN);
		c->loc.io_class = &io_file;
		c->loc.flags |= FLAG_W;
		DMCLOG_D("backup c->offset = %lld",c->offset);
		c->offset = 0;
		(void) bfavfs_fseek(c->loc.chan.vf, c->offset, SEEK_SET);
		BucketObject *cObject = build_bucket_object(c->cfg_path,c->token);
		if(cObject == NULL)
		{
			DMCLOG_E("buile bucket object error");
			c->error = FILE_IS_NOT_EXIST;
			goto EXIT;
		}
		if(_bfavfs_fstat(cObject, &st,c->token) != 0)
		{
			int i = 0;
			off_t percent = c->fileSize/THREAD_COUNT;
			c->dn = NULL;
			for(; i < THREAD_COUNT;i++)
		    {
		        if(i == THREAD_COUNT - 1)
					add_record_to_list(&c->dn,i,i * percent,c->fileSize);
		        else
					add_record_to_list(&c->dn,i,i * percent,(i + 1)* percent);
		    }
			display_record_dnode(c->dn);
		}else{
			c->dn = NULL;
			if(read_list_from_file(c->cfg_path,&c->dn,c->token) < 0)
			{
				c->error = ERROR_FILE_UPLOAD_CHECK;
				goto EXIT;
			}
		}
		if((c->record_fd = _bfavfs_fopen(cObject,"w+",c->token))== NULL)
		{
			DMCLOG_D("open file error[errno = %d]",errno);
			c->error = ERROR_FILE_UPLOAD_CHECK;
			goto EXIT;
		}
	}
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* contentLength_json = JSON_NEW_EMPTY_OBJECT();
	DMCLOG_D("DM_FileUpload, c->rem.content_len = %lld", c->rem.content_len);
	JSON_ADD_OBJECT(contentLength_json, "contentLength",JSON_NEW_OBJECT(c->rem.content_len,int64));
	JSON_ARRAY_ADD_OBJECT (response_data_array,contentLength_json);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	safe_free(sObject);
	safe_free(cObject);
	return 0;
}
int DM_FileCheckBackupFileList(struct conn *c)
{
	ENTER_FUNC();
	JObj *response_json=JSON_NEW_EMPTY_OBJECT();
	v_file_info_t *p_file_info;
	v_file_info_t *n;
	BucketObject *bObject = (BucketObject *)calloc(1,sizeof(BucketObject));
	assert(bObject != NULL);
	if(c->vlist.total > 0)
    {
    	bObject->head = &c->vlist.head;
    	int res = _bfavfs_exist(bObject,c->token);
		if(res != 0)
		{
			DMCLOG_E("query uuid list error,res = %d",res);
			goto EXIT;
		}
		
		JObj *response_para_json = JSON_NEW_EMPTY_OBJECT();
        JObj *file_info_array = JSON_NEW_ARRAY();
    	dl_list_for_each(p_file_info, &(c->vlist.head), v_file_info_t, next)
    	{
    		if(p_file_info->backupFlag == 1)
    		{
				JObj *file_info = JSON_NEW_EMPTY_OBJECT();
				JSON_ADD_OBJECT(file_info, "file_uuid",JSON_NEW_OBJECT(p_file_info->uuid,string));
				DMCLOG_D("file_uuid = %s",p_file_info->uuid);
				JSON_ARRAY_ADD_OBJECT (file_info_array,file_info);
			}
			
    	}
		JSON_ADD_OBJECT(response_para_json, "backupFileListInfo", file_info_array);
		JSON_ADD_OBJECT(response_json, "data", response_para_json);
    }
    dl_list_for_each_safe(p_file_info, n, &c->vlist.head, v_file_info_t, next)
    {
        dl_list_del(&p_file_info->next);
		safe_free(p_file_info);
        
    }
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	safe_free(bObject);
	EXIT_FUNC();
	return 0;
}
int DM_FileCheckBackupFile(struct conn *c)
{
	ENTER_FUNC();
	JObj *response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* data_json = JSON_NEW_EMPTY_OBJECT();
	int i;

	struct stat st;
	BucketObject *sObject = build_bucket_object(c->cfg_path,c->token);
	if(sObject == NULL)
	{
		DMCLOG_E("buile bucket object error");
		c->error = FILE_IS_NOT_EXIST;
		goto EXIT;
	}
	if(_bfavfs_fstat(sObject, &st,c->token) == 0)
	{
		DMCLOG_D("cfg path is exist");
		c->status = 1;//cfg is exsit  and app need to upload continue by cfg info
		c->dn = NULL;
		int res = read_list_from_file(c->cfg_path,&c->dn,c->token);
		if(res < 0)
		{
			c->status = 0;
			goto EXIT;
		}
		JObj *response_data_array = JSON_NEW_ARRAY();
		struct record_dnode *p_dn = c->dn;
		for(i = 0;/*count - detected via !dn below*/;i++)
		{
			 if(!p_dn)
			 {
			 	DMCLOG_D("link is NULL");
				break;
			 }
			 JObj *upload_record_json =  JSON_NEW_EMPTY_OBJECT();
			 JSON_ADD_OBJECT(upload_record_json, "start", JSON_NEW_OBJECT(p_dn->start,int64));
			 JSON_ADD_OBJECT(upload_record_json, "end", JSON_NEW_OBJECT(p_dn->end,int64));
			 DMCLOG_D("i = %d,dn->index = %d",i,p_dn->index);
			 JSON_ARRAY_ADD_OBJECT (response_data_array,upload_record_json);
			 p_dn = p_dn->dn_next;
		}
		JSON_ADD_OBJECT(data_json, "count", JSON_NEW_OBJECT(i,int));
		JSON_ADD_OBJECT(data_json, "incompleteInfo", response_data_array);
		destory_record_list(c->dn);
	}else{
		DMCLOG_D("cfg_path is not exist");
		c->status = 0;
	}
	DMCLOG_D("c->status = %d",c->status);
	
EXIT:
	JSON_ADD_OBJECT(data_json, "status", JSON_NEW_OBJECT(c->status,int));
	JSON_ADD_OBJECT(response_json, "data", data_json);
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	safe_free(sObject);
	EXIT_FUNC();
	return 0;
}

int DM_FileIsBackupExisted(struct conn *c)
{
	ENTER_FUNC();
	JObj *response_json=JSON_NEW_EMPTY_OBJECT();
	BucketObject *bObject = (BucketObject *)calloc(1,sizeof(BucketObject));
	assert(bObject != NULL);
	S_STRNCPY(bObject->file_uuid,c->file_uuid,MAX_FILE_UUID_LEN);
	
	int res = _bfavfs_exist(bObject,c->token);
	if(res != 0)
	{
		DMCLOG_D("file uuid is not exist (%s)", c->file_uuid);
		c->error = FILE_IS_NOT_EXIST;//the file is not exist in file table
	}
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
	return 0;
}

char *DM_FileInotify(int cmd ,int error)
{
	ENTER_FUNC();
	char *buf = NULL;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* status_json = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(status_json, "status",JSON_NEW_OBJECT(0,int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,status_json);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);	
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(0,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	const char *response_str = JSON_TO_STRING(response_json);
	if(response_str == NULL)
	{
		return NULL;
	}
	buf = (char *)calloc(1,strlen(response_str) + 1);
	assert(buf != NULL);
	strcpy(buf,response_str);
	JSON_PUT_OBJECT(response_json);
	return buf;
	EXIT_FUNC();
	
}

int DM_FileUtimensat(struct conn *c)
{
	int res = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	BucketObject *sObject = build_bucket_object(c->src_path,c->token);
	if(sObject == NULL){
		DMCLOG_E("buile bucket object error");
		c->error = ERROR_SET_FILE_TIME;
		goto EXIT;
	}

	struct timespec times[2];
	memcpy(&times[0], &c->a_time, sizeof(struct timespec));
	memcpy(&times[1], &c->m_time, sizeof(struct timespec));
	DMCLOG_D("times[0].tv_sec: %lu, times[0].tv_nsec: %lu, times[1].tv_sec: %lu, times[1].tv_nsec: %lu", 
		times[0].tv_sec, times[0].tv_nsec, times[1].tv_sec, times[1].tv_nsec);
	if (_bfavfs_utimensat(sObject, times, c->status, (char *)c->token) != 0) {
		c->error = ERROR_SET_FILE_TIME;
		goto EXIT;
	} 
	
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	safe_free(sObject);
	return 0;
}

int DM_FileSymlink(struct conn *c)
{
	int res = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	BucketObject *sObject = build_bucket_object(c->src_path,c->token);
	if(sObject == NULL){
		DMCLOG_E("buile bucket object error");
		c->error = ERROR_SET_FILE_LINK;
		goto EXIT;
	}

	BucketObject *dObject = build_bucket_object(c->des_path,c->token);
	if(dObject == NULL){
		DMCLOG_E("buile bucket object error");
		c->error = ERROR_SET_FILE_LINK;
		goto EXIT;
	}
		
	if (_bfavfs_symlink(sObject, dObject, (char *)c->token) != 0) {
		c->error = ERROR_SET_FILE_LINK;
		goto EXIT;
	} 
	
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	safe_free(sObject);
	safe_free(dObject);
	return 0;
}

int DM_FileLink(struct conn *c)
{
	int res = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	BucketObject *sObject = build_bucket_object(c->src_path,c->token);
	if(sObject == NULL){
		DMCLOG_E("buile bucket object error");
		c->error = ERROR_SET_FILE_LINK;
		goto EXIT;
	}

	BucketObject *dObject = build_bucket_object(c->des_path,c->token);
	if(dObject == NULL){
		DMCLOG_E("buile bucket object error");
		c->error = ERROR_SET_FILE_LINK;
		goto EXIT;
	}
		
	if (_bfavfs_link(sObject, dObject, (char *)c->token) != 0) {
		c->error = ERROR_SET_FILE_LINK;
		goto EXIT;
	} 
	
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	safe_free(sObject);
	safe_free(dObject);
	return 0;
}

int DM_FileReadlink(struct conn *c)
{
	int res = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	BucketObject *sObject = build_bucket_object(c->src_path,c->token);
	if(sObject == NULL){
		DMCLOG_E("buile bucket object error");
		c->error = ERROR_SET_FILE_LINK;
		goto EXIT;
	}

	c->des_path = (char *)calloc(1, 4096);
	if(c->des_path == NULL){
		c->error = ERROR_SET_FILE_LINK;
		goto EXIT;
	}

	if (_bfavfs_readlink(sObject, c->des_path, 4096, (char *)c->token) != 0) {
		c->error = ERROR_SET_FILE_LINK;
		goto EXIT;
	} 

	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* data_json = JSON_NEW_EMPTY_OBJECT();
	DMCLOG_D("c->des_path: %s", c->des_path);
	JSON_ADD_OBJECT(data_json, "path",JSON_NEW_OBJECT(c->des_path,string));
	JSON_ARRAY_ADD_OBJECT (response_data_array,data_json);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	safe_free(sObject);
	return 0;
}

int DM_FileStatvfs(struct conn *c)
{
	int res = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	BucketObject *sObject = build_bucket_object(c->src_path,c->token);
	if(sObject == NULL){
		DMCLOG_E("buile bucket object error");
		c->error = ERROR_STATVFS;
		goto EXIT;
	}

	DMCLOG_D("c->src_path: %s", c->src_path);
	if(_bfavfs_statvfs(sObject, &c->statvfs_buf, (char *)c->token) != 0) {
		DMCLOG_E("_bfavfs_statvfs fail");
		c->error = ERROR_STATVFS;
		goto EXIT;
	} 

	DMCLOG_D("f_bsize: %d, f_blocks: %lld", (int)c->statvfs_buf.f_bsize, (long long)c->statvfs_buf.f_blocks);

	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* data_json = JSON_NEW_EMPTY_OBJECT();
	
	JSON_ADD_OBJECT(data_json, "f_bsize",JSON_NEW_OBJECT(c->statvfs_buf.f_bsize, int));	
	JSON_ADD_OBJECT(data_json, "f_frsize",JSON_NEW_OBJECT(c->statvfs_buf.f_frsize, int));	
	JSON_ADD_OBJECT(data_json, "f_fsid",JSON_NEW_OBJECT(c->statvfs_buf.f_fsid, int));
	JSON_ADD_OBJECT(data_json, "f_flag",JSON_NEW_OBJECT(c->statvfs_buf.f_flag, int)); 
	JSON_ADD_OBJECT(data_json, "f_namemax",JSON_NEW_OBJECT(c->statvfs_buf.f_namemax, int)); 

	JSON_ADD_OBJECT(data_json, "f_blocks",JSON_NEW_OBJECT(c->statvfs_buf.f_blocks, int64));
	JSON_ADD_OBJECT(data_json, "f_bfree",JSON_NEW_OBJECT(c->statvfs_buf.f_bfree, int64));	
	JSON_ADD_OBJECT(data_json, "f_bavail",JSON_NEW_OBJECT(c->statvfs_buf.f_bavail, int64));	
	JSON_ADD_OBJECT(data_json, "f_files",JSON_NEW_OBJECT(c->statvfs_buf.f_files, int64));
	JSON_ADD_OBJECT(data_json, "f_ffree",JSON_NEW_OBJECT(c->statvfs_buf.f_ffree, int64)); 
	JSON_ADD_OBJECT(data_json, "f_favail",JSON_NEW_OBJECT(c->statvfs_buf.f_favail, int64)); 

	JSON_ARRAY_ADD_OBJECT (response_data_array,data_json);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
EXIT:
	file_json_to_string(c,response_json);
	JSON_PUT_OBJECT(response_json);
	safe_free(sObject);
	return 0;
}

void file_func_process(struct conn *c)
{ 
	uint8_t i = 0;
	for(i = 0; i<FILE_TAGHANDLE_NUM; i++)
	{
		if(c->cmd == all_file_handle[i].tag)
		{
	       	 all_file_handle[i].tagfun(c);
			 break;
		}
	}	
	return ;
}

void file_parse_header_json(struct conn *c)
{
	c->r_json = JSON_PARSE(c->rem.io.buf);
	if(c->r_json == NULL)
	{
		DMCLOG_D("access NULL");
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	if(is_error(c->r_json))
	{
		DMCLOG_D("### error:post data is not a json string");
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	
	JObj *header_json = JSON_GET_OBJECT(c->r_json,"header");
	if(header_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *cmd_json = JSON_GET_OBJECT(header_json,"cmd");
	if(cmd_json != NULL)
	{
		c->cmd = JSON_GET_OBJECT_VALUE(cmd_json,int);
	}
	
	JObj *seq_json = JSON_GET_OBJECT(header_json,"seq");
	if(seq_json != NULL)
	{
		c->seq = JSON_GET_OBJECT_VALUE(seq_json,int);
	}
	
	JObj *token_json = JSON_GET_OBJECT(header_json,"token");
	if(token_json != NULL)
	{
		c->token = JSON_GET_OBJECT_VALUE(token_json,int);
	}
	return;
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return;
}

int Parser_GetVersion(struct conn *c)
{
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;
}

int Parser_register(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	JObj *username_json = JSON_GET_OBJECT(para_json,"username");
	JObj *password_json = JSON_GET_OBJECT(para_json,"password");
	if(username_json == NULL||password_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	S_STRNCPY(c->username,JSON_GET_OBJECT_VALUE(username_json,string),64);
	S_STRNCPY(c->password,JSON_GET_OBJECT_VALUE(password_json,string),64);
	if(!*c->username||!*c->password)
	{
		DMCLOG_E("para is null");
		c->error = DM_ERROR_CMD_PARAMETER;
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;
}

int Parser_Login(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	JObj *username_json = JSON_GET_OBJECT(para_json,"username");
	JObj *password_json = JSON_GET_OBJECT(para_json,"password");
	if(username_json == NULL||password_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	S_STRNCPY(c->username,JSON_GET_OBJECT_VALUE(username_json,string),64);
	S_STRNCPY(c->password,JSON_GET_OBJECT_VALUE(password_json,string),64);
	if(!*c->username||!*c->password)
	{
		DMCLOG_E("para is null");
		c->error = DM_ERROR_CMD_PARAMETER;
	}
	JObj *port_json = JSON_GET_OBJECT(para_json,"port");
	if(port_json != NULL)
	{
		c->client_port = JSON_GET_OBJECT_VALUE(port_json,int);
	}
	JObj *statusFlag_json = JSON_GET_OBJECT(para_json,"statusFlag");
	if(statusFlag_json != NULL)
	{
		c->statusFlag = JSON_GET_OBJECT_VALUE(statusFlag_json,int);
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;
}
int Parser_Logout(struct conn *c)
{
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;
}

int Parser_DelClientInfo(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	JObj *ip_json = JSON_GET_OBJECT(para_json,"ip");
	if(ip_json == NULL)
	{
		c->error = INVALIDE_COMMAND;
		goto EXIT;
	}
	strcpy(c->client_ip, JSON_GET_OBJECT_VALUE(ip_json,string));
	DMCLOG_D("cleint ip = %s",c->client_ip);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;
}

int Parser_RouterGetFuncList(struct conn *c)
{
	return 0;
}


/*
 * cmd = 106
 */
int Parser_FileDownload(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	JObj *offset_json = JSON_GET_OBJECT(para_json,"offset");
	JObj *length_json = JSON_GET_OBJECT(para_json,"length");

	if(path_json == NULL||offset_json == NULL||length_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	const char *uri_src = JSON_GET_OBJECT_VALUE(path_json,string);
	if(uri_src == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
		
	}
	DMCLOG_D("uri_src = %s",uri_src);
	c->src_path = (char *)calloc(1,strlen(uri_src) + 1);
	assert(c->src_path != NULL);
    if(*uri_src == '/')
    {
        sprintf(c->src_path,"%s",uri_src + 1);
    }else{
        sprintf(c->src_path,"%s",uri_src);
    }
	c->offset = JSON_GET_OBJECT_VALUE(offset_json,int64);
	c->length = JSON_GET_OBJECT_VALUE(length_json,int64);
	
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;	
}

/*
 * 上传检查 cmd = 108
 */
int Parser_FileCheckUpload(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	JObj *fileSize_json = JSON_GET_OBJECT(para_json,"fileSize");
	JObj *modifyTime_json = JSON_GET_OBJECT(para_json,"modifyTime");

	if(path_json == NULL||fileSize_json == NULL||modifyTime_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	const char *uri_src = JSON_GET_OBJECT_VALUE(path_json,string);
	if(uri_src == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	DMCLOG_D("uri_src = %s",uri_src);
	c->des_path = (char *)calloc(1,strlen(uri_src) + 1);
	assert(c->des_path != NULL);
    if(*uri_src == '/')
    {
        sprintf(c->des_path,"%s",uri_src + 1);
    }else{
        sprintf(c->des_path,"%s",uri_src);
    }
	c->fileSize = JSON_GET_OBJECT_VALUE(fileSize_json,int64);
	c->modifyTime = JSON_GET_OBJECT_VALUE(modifyTime_json,int64);
	c->cfg_path = (char *)malloc(strlen(c->des_path)+strlen(CFG_PATH_NAME)+1);
	assert(c->cfg_path != NULL);
	sprintf(c->cfg_path,"%s%s",c->des_path,CFG_PATH_NAME);
	DMCLOG_D("cfg_path = %s",c->cfg_path);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;	
}
/*
 * 上传命令cmd = 107
 */
int Parser_FileUpload(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	JObj *offset_json = JSON_GET_OBJECT(para_json,"offset");
	JObj *length_json = JSON_GET_OBJECT(para_json,"length");
	JObj *fileSize_json = JSON_GET_OBJECT(para_json,"fileSize");
	JObj *modifyTime_json = JSON_GET_OBJECT(para_json,"modifyTime");
	if(path_json == NULL||offset_json == NULL||length_json == NULL||fileSize_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	
	const char *uri_src = JSON_GET_OBJECT_VALUE(path_json,string);
	if(uri_src == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
		
	}
	DMCLOG_D("uri_src = %s",uri_src);

	c->des_path = (char *)calloc(1,strlen(uri_src) + 1);
	assert(c->des_path != NULL);
    if(*uri_src == '/')
    {	
    	if(*(uri_src + 1) == '/')
    	{
			sprintf(c->des_path,"%s",uri_src + 2);
		}else{
			sprintf(c->des_path,"%s",uri_src + 1);
		}
    }else{
        sprintf(c->des_path,"%s",uri_src);
    }
	
	c->fileSize= JSON_GET_OBJECT_VALUE(fileSize_json,int64);
	if(modifyTime_json != NULL)
		c->modifyTime = JSON_GET_OBJECT_VALUE(modifyTime_json,int64);
	c->offset = JSON_GET_OBJECT_VALUE(offset_json,int64);
	c->length = JSON_GET_OBJECT_VALUE(length_json,int64);
	
	c->cfg_path = (char *)calloc(1,strlen(c->des_path)+strlen(CFG_PATH_NAME)+1);
	assert(c->cfg_path != NULL);
	sprintf(c->cfg_path,"%s%s",c->des_path,CFG_PATH_NAME);
	DMCLOG_D("cfg_path = %s",c->cfg_path);
	c->tmp_path = (char *)calloc(1,strlen(c->des_path)+strlen(TMP_PATH_NAME)+1);
	assert(c->tmp_path);
	sprintf(c->tmp_path,"%s%s",c->des_path,TMP_PATH_NAME);
	DMCLOG_D("tmp_path = %s",c->tmp_path);
	c->rem.content_len = c->length - c->offset;

EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;	
}


int Parser_FileGetStorage(struct conn *c)
{
	ENTER_FUNC();
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	EXIT_FUNC();
	return 0;	
}

int Parser_FileGetList(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}

	JObj *fileType_json = JSON_GET_OBJECT(para_json,"fileType");
	JObj *sortType_json = JSON_GET_OBJECT(para_json,"sortType");
	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	JObj *offset_json = JSON_GET_OBJECT(para_json,"offset");
	JObj *length_json = JSON_GET_OBJECT(para_json,"length");

	if(fileType_json != NULL)
	{
		c->fileType = JSON_GET_OBJECT_VALUE(fileType_json,int);
	}

	if(sortType_json != NULL)
	{
		c->sortType = JSON_GET_OBJECT_VALUE(sortType_json,int);
	}
	
	if(path_json != NULL)
	{
		const char *uri_src = JSON_GET_OBJECT_VALUE(path_json,string);
		if(uri_src == NULL)
		{
			c->error = REQUEST_FORMAT_ERROR;
			goto EXIT;
		}
		DMCLOG_D("uri = %s",uri_src);
		c->src_path = (char *)calloc(1,strlen(uri_src) + 1);
		assert(c->src_path != NULL);
	    if(*uri_src == '/')
	    {
	        sprintf(c->src_path,"%s",uri_src + 1);
	    }else{
	        sprintf(c->src_path,"%s",uri_src);
	    }
		DMCLOG_D("src_path = %s",c->src_path);
		char *lc = get_last_char(c->src_path, '/');
		if(lc != NULL){
			*lc = 0;
		}
		DMCLOG_D("c->src_path = %s",c->src_path);
	}

	if(offset_json != NULL)
		c->offset = JSON_GET_OBJECT_VALUE(offset_json,int);

	if(length_json != NULL)
		c->length = JSON_GET_OBJECT_VALUE(length_json,int);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;	
}
/*
 * 文件夹创建 cmd = 102
 */
int Parser_FileMkdir(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	
	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	if(path_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	const char *uri_src = JSON_GET_OBJECT_VALUE(path_json,string);
	if(uri_src == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	DMCLOG_D("uri = %s",uri_src);
	c->src_path = (char *)calloc(1,strlen(uri_src) + 1);
	assert(c->src_path != NULL);
    if(*uri_src == '/')
    {
    	if(*(uri_src + 1) == '/')
    	{
			sprintf(c->src_path,"%s",uri_src + 2);
		}else{
			sprintf(c->src_path,"%s",uri_src + 1);
		}
        
    }else{
        sprintf(c->src_path,"%s",uri_src);
    }
    
	DMCLOG_D("src_path = %s",c->src_path);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;	
}
/*
 * 文件或文件夹重名 cmd = 103
 */
int Parser_FileRename(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;

		goto EXIT;
	}

	JObj *src_path_json = JSON_GET_OBJECT(para_json,"srcPath");
	JObj *des_path_json = JSON_GET_OBJECT(para_json,"desPath");
	if(src_path_json == NULL||des_path_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	const char *uri_src = JSON_GET_OBJECT_VALUE(src_path_json,string);
	if(uri_src == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	c->src_path = (char *)calloc(1,strlen(uri_src) + 1);
	assert(c->src_path != NULL);
    if(*uri_src == '/')
    {
        sprintf(c->src_path,"%s",uri_src +1);
    }else{
        sprintf(c->src_path,"%s",uri_src);
    }
	uri_src = JSON_GET_OBJECT_VALUE(des_path_json,string);
	if(uri_src == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	c->des_path = (char *)calloc(1,strlen(uri_src) + 1);
	assert(c->des_path != NULL);
    if(*uri_src == '/')
    {
        sprintf(c->des_path,"%s",uri_src +1);
    }else{
        sprintf(c->des_path,"%s",uri_src);
    }
	DMCLOG_D("c->des_path = %s",c->des_path);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;	
}

/*
 * 判断文件或文件夹是否存在 cmd = 104
 */
int Parser_FileIsExist(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	
	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	if(path_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	const char *uri_src = JSON_GET_OBJECT_VALUE(path_json,string);
	if(uri_src == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	DMCLOG_D("uri_src = %s",uri_src);
	c->src_path = (char *)calloc(1,strlen(uri_src) + 1);
	assert(c->src_path != NULL);
    if(*uri_src == '/')
    {
        sprintf(c->src_path,"%s",uri_src + 1);
    }else{
        sprintf(c->src_path,"%s",uri_src);
    }
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;	
}
/*
 *文件或文件夹删除 cmd = 109
 */
int parser_FileDelete(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}

	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	if(path_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
    
	const char *uri_src = JSON_GET_OBJECT_VALUE(path_json,string);
	if(uri_src == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	DMCLOG_D("uri_src = %s",uri_src);
	c->src_path = (char *)calloc(1,strlen(uri_src) + 1);
	assert(c->src_path != NULL);
    if(*uri_src == '/')
    {
        sprintf(c->src_path,"%s",uri_src + 1);
    }else{
        sprintf(c->src_path,"%s",uri_src);
    }
	
	DMCLOG_D("c->src_path = %s",c->src_path);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;	
}
int parser_FileCopy(struct conn *c)
{
    JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
    if(data_json == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
    if(para_json == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
	
    JObj *src_path_json = JSON_GET_OBJECT(para_json,"path");
    if(src_path_json == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    const char *srcPath = JSON_GET_OBJECT_VALUE(src_path_json,string);
    if(srcPath == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    DMCLOG_D("srcPath = %s",srcPath);

    c->src_path = (char *)calloc(1,strlen(srcPath) + 1);
    assert(c->src_path != NULL);
    if(*srcPath == '/')
    {
        sprintf(c->src_path,"%s",srcPath + 1);
    }else{
        sprintf(c->src_path,"%s",srcPath);
    }
    DMCLOG_D("c->src_path = %s",c->src_path);
    
    JObj *des_path_json = JSON_GET_OBJECT(para_json,"desPath");
    if(des_path_json == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    const char *desPath = JSON_GET_OBJECT_VALUE(des_path_json,string);
    if(desPath == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    DMCLOG_D("desPath = %s",desPath);

    c->des_path = (char *)calloc(1,strlen(desPath) + 1);
    assert(c->des_path != NULL);
    if(*desPath == '/')
    {
        sprintf(c->des_path,"%s",desPath + 1);
    }else{
        sprintf(c->des_path,"%s",desPath);
    }
    DMCLOG_D("c->des_path = %s",c->des_path);
	char *lc = get_last_char(c->des_path, '/');
	if(lc != NULL){
		*lc = 0;
	}
	DMCLOG_D("c->des_path = %s",c->des_path);
EXIT:
    if(c->r_json != NULL)
        JSON_PUT_OBJECT(c->r_json);
    return 0;	
}
int parser_FileMove(struct conn *c)
{
    JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
    if(data_json == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
    if(para_json == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }

    JObj *src_path_json = JSON_GET_OBJECT(para_json,"path");
    if(src_path_json == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    const char *srcPath = JSON_GET_OBJECT_VALUE(src_path_json,string);
    if(srcPath == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    DMCLOG_D("srcPath = %s",srcPath);
    c->src_path = (char *)calloc(1,strlen(srcPath) + 1);
    assert(c->src_path != NULL);
    if(*srcPath == '/')
    {
        sprintf(c->src_path,"%s",srcPath + 1);
    }else{
        sprintf(c->src_path,"%s",srcPath);
    }
    DMCLOG_D("c->src_path = %s",c->src_path);
    
    JObj *des_path_json = JSON_GET_OBJECT(para_json,"desPath");
    if(des_path_json == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    const char *desPath = JSON_GET_OBJECT_VALUE(des_path_json,string);
    if(desPath == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    DMCLOG_D("desPath = %s",desPath);
    c->des_path = (char *)calloc(1,strlen(desPath) + 1);
    assert(c->des_path != NULL);
    if(*desPath == '/')
    {
        sprintf(c->des_path,"%s",desPath + 1);
    }else{
        sprintf(c->des_path,"%s",desPath);
    }
    DMCLOG_D("c->des_path = %s",c->des_path);
EXIT:
    if(c->r_json != NULL)
        JSON_PUT_OBJECT(c->r_json);
    return 0;	
}

int parser_FileSearch(struct conn *c)
{
    JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
    if(data_json == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
    if(para_json == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    
    JObj *seq_json = JSON_GET_OBJECT(para_json,"seq");
    if(seq_json == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    c->seq = JSON_GET_OBJECT_VALUE(seq_json,int);

	/* get the string of search */
	JObj *search_string_json = JSON_GET_OBJECT(para_json,"searchString");
    if(search_string_json == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    const char *searchString = JSON_GET_OBJECT_VALUE(search_string_json,string);
    if(searchString == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    DMCLOG_D("searchString = %s",searchString);

	c->search_string = (char *)calloc(1, strlen(searchString) + 1);
	assert(c->search_string != NULL);
	strcpy(c->search_string, searchString);

	/* get the path of search */
	JObj *des_path_json = JSON_GET_OBJECT(para_json,"searchPath");
    if(des_path_json == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    const char *desPath = JSON_GET_OBJECT_VALUE(des_path_json,string);
    if(desPath == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    DMCLOG_D("desPath = %s",desPath);
    c->des_path = (char *)calloc(1,strlen(desPath) + 1);
    assert(c->des_path != NULL);
    if(*desPath == '/')
    {
        sprintf(c->des_path,"%s",desPath + 1);
    }else{
        sprintf(c->des_path,"%s",desPath);
    }
    DMCLOG_D("c->des_path = %s",c->des_path);
EXIT:
    if(c->r_json != NULL)
        JSON_PUT_OBJECT(c->r_json);
    return 0;	
}

int Parser_FileGetListByType(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *fileType_json = JSON_GET_OBJECT(para_json,"fileType");
	JObj *sortType_json = JSON_GET_OBJECT(para_json,"sortType");
	if(fileType_json != NULL)
	{
		c->fileType = JSON_GET_OBJECT_VALUE(fileType_json,int);
	}
	
	if(sortType_json != NULL)
	{
		c->sortType = JSON_GET_OBJECT_VALUE(sortType_json,int);
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;	
}

int Parser_FileGetFilePathByType(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *fileType_json = JSON_GET_OBJECT(para_json,"fileType");
	if(fileType_json != NULL)
	{
		c->fileType = JSON_GET_OBJECT_VALUE(fileType_json,int);
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;	
}
int Parser_FileGetFileListByPath(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *fileType_json = JSON_GET_OBJECT(para_json,"fileType");
	if(fileType_json != NULL)
	{
		c->fileType = JSON_GET_OBJECT_VALUE(fileType_json,int);
	}
	JObj *filePath_json = JSON_GET_OBJECT(para_json,"path");
	if(filePath_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	const char *uri_src = JSON_GET_OBJECT_VALUE(filePath_json,string);
	if(uri_src == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	DMCLOG_D("uri_src = %s",uri_src);

	c->src_path = (char *)calloc(1,strlen(uri_src) + 1);
	assert(c->src_path != NULL);
    if(*uri_src == '/')
    {
        sprintf(c->src_path,"%s",uri_src + 1);
    }else{
        sprintf(c->src_path,"%s",uri_src);
    }
    DMCLOG_D("c->src_path = %s",c->src_path);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;	
}

int Parser_FileDelFileListByPath(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *fileType_json = JSON_GET_OBJECT(para_json,"fileType");
	if(fileType_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	c->fileType = JSON_GET_OBJECT_VALUE(fileType_json,int);
	if(c->fileType <=  0)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *filePath_json = JSON_GET_OBJECT(para_json,"path");
	if(filePath_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	const char *uri_src = JSON_GET_OBJECT_VALUE(filePath_json,string);
	if(uri_src == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	DMCLOG_D("uri_src = %s",uri_src);

	c->src_path = (char *)calloc(1,strlen(uri_src) + 1);
	assert(c->src_path != NULL);
    if(*uri_src == '/')
    {
        sprintf(c->src_path,"%s",uri_src + 1);
    }else{
        sprintf(c->src_path,"%s",uri_src);
    }
    DMCLOG_D("c->src_path = %s",c->src_path);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;	
}

int Parser_FileGetBackupFile(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	JObj *offset_json = JSON_GET_OBJECT(para_json,"offset");
	JObj *length_json = JSON_GET_OBJECT(para_json,"length");
	JObj *fileSize_json = JSON_GET_OBJECT(para_json,"fileSize");
	JObj *modifyTime_json = JSON_GET_OBJECT(para_json,"modifyTime");
	JObj *fileUuid_json = JSON_GET_OBJECT(para_json,"fileUuid");
	JObj *dirName_json = JSON_GET_OBJECT(para_json,"dirName");
	if(path_json == NULL||offset_json == NULL||length_json == NULL||fileSize_json == NULL||modifyTime_json == NULL || fileUuid_json == NULL || dirName_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	c->fileSize= JSON_GET_OBJECT_VALUE(fileSize_json,int64);
	c->modifyTime = JSON_GET_OBJECT_VALUE(modifyTime_json,int);
	c->offset = JSON_GET_OBJECT_VALUE(offset_json,int64);
	c->length = JSON_GET_OBJECT_VALUE(length_json,int64);
	const char *fileUuid = JSON_GET_OBJECT_VALUE(fileUuid_json,string);
	const char *dir_name = JSON_GET_OBJECT_VALUE(dirName_json,string);
	if(dir_name == NULL || fileUuid == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	
	S_STRNCPY(c->file_uuid, fileUuid, FILE_UUID_LEN);	
	c->rem.content_len = c->length - c->offset;
	const char *uri_src = JSON_GET_OBJECT_VALUE(path_json,string);
	if(uri_src == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
		
	}
	c->des_path = (char *)calloc(1,strlen(dir_name) + strlen(bb_basename(uri_src)) + 2);
	assert(c->des_path != NULL);
    sprintf(c->des_path,"%s/%s",dir_name,bb_basename(uri_src));

	c->cfg_path = (char *)calloc(1,strlen(c->des_path)+strlen(CFG_PATH_NAME)+1);
	assert(c->cfg_path != NULL);
	sprintf(c->cfg_path,"%s%s",c->des_path,CFG_PATH_NAME);
	DMCLOG_D("cfg_path = %s",c->cfg_path);
	c->tmp_path = (char *)calloc(1,strlen(c->des_path)+strlen(TMP_PATH_NAME)+1);
	assert(c->tmp_path != NULL);
	sprintf(c->tmp_path,"%s%s",c->des_path,TMP_PATH_NAME);
	char *tmp = strrchr(c->tmp_path,'/');
	*tmp = '\0';
	BucketObject *sObject = build_bucket_object(c->tmp_path,c->token);
	if(sObject == NULL)
	{
		DMCLOG_E("buile bucket object error");
		c->error = ERROR_MKDIR;
		goto EXIT;
	}
	if(_bfavfs_mkdir(sObject,(char *)c->token) != RET_SUCCESS)
    {
        DMCLOG_D("mkdir(%s) failed", c->src_path);
        c->error = ERROR_MKDIR;
		goto EXIT;
    }
	*tmp = '/';
EXIT:
	safe_free(sObject);
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;
}

int Parser_FileCheckBackupFileList(struct conn *c)
{
	ENTER_FUNC();
	unsigned long i;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *backupfilelist_json = JSON_GET_OBJECT(data_json,"backupFileListInfo");
	if(backupfilelist_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	int array_len = JSON_GET_ARRAY_LEN(backupfilelist_json);
	if(array_len == 0)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	DMCLOG_D("array_len = %d",array_len);
	c->vlist.total= 0;
	dl_list_init(&c->vlist.head);
	for(i = 0;i < array_len;i++)
	{
		JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(backupfilelist_json,i);
		if(para_json == NULL)
		{
			DMCLOG_D("access NULL");
			c->error = REQUEST_FORMAT_ERROR;
			goto EXIT;
		}
		JObj *fileUuid_json = JSON_GET_OBJECT(para_json,"fileUuid");
		
		if(fileUuid_json == NULL)
		{
			DMCLOG_D("access NULL");
			c->error = REQUEST_FORMAT_ERROR;
			goto EXIT;
		}
		const char *fileUuid = JSON_GET_OBJECT_VALUE(fileUuid_json,string);
		v_file_info_t *fdi = (v_file_info_t *)calloc(1,sizeof(v_file_info_t));
		assert(fdi != NULL);
		fdi->backupFlag = 0;
		S_STRNCPY(fdi->uuid,fileUuid,MAX_FILE_UUID_LEN);
		dl_list_add_tail(&c->vlist.head, &fdi->next);
		c->vlist.total++;
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	EXIT_FUNC();
	return 0;
}

int Parser_FileCheckBackupFile(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	
	JObj *fileUuid_json = JSON_GET_OBJECT(para_json,"fileUuid");
	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	JObj *fileSize_json = JSON_GET_OBJECT(para_json,"fileSize");
		
	if(fileSize_json == NULL||fileUuid_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	const char *fileUuid = JSON_GET_OBJECT_VALUE(fileUuid_json,string);
	c->fileSize = JSON_GET_OBJECT_VALUE(fileSize_json,int64);

	S_STRNCPY(c->file_uuid,fileUuid,FILE_UUID_LEN);
	if(path_json != NULL)
	{
		const char *uri_src = JSON_GET_OBJECT_VALUE(path_json,string);
		if(uri_src == NULL)
		{
			c->error = REQUEST_FORMAT_ERROR;
			goto EXIT;
		}
		DMCLOG_D("uri = %s",uri_src);
		c->src_path = (char *)calloc(1,strlen(uri_src) + 1);
		assert(c->src_path != NULL);
	    if(*uri_src == '/')
	    {
	        sprintf(c->src_path,"%s",uri_src + 1);
	    }else{
	        sprintf(c->src_path,"%s",uri_src);
	    }
		DMCLOG_D("src_path = %s",c->src_path);
		c->cfg_path = (char *)calloc(1,strlen(c->src_path) + strlen(CFG_PATH_NAME) + 1);
		assert(c->cfg_path);
		sprintf(c->cfg_path,"%s%s",c->src_path,CFG_PATH_NAME);
		DMCLOG_D("cfg_path = %s",c->cfg_path);
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;	
}

int Parser_FileIsBackupFile(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *uuid_json = JSON_GET_OBJECT(para_json,"fileUuid");
	if(uuid_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	const char *file_uuid = JSON_GET_OBJECT_VALUE(uuid_json,string);
	if(file_uuid == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	DMCLOG_D("file_uuid = %s",file_uuid);
	S_STRNCPY(c->file_uuid,file_uuid,FILE_UUID_LEN);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;		
}

int parser_FileUtimensat(struct conn *c)
{
    JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
    if(data_json == NULL){
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
    if(para_json == NULL){
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }

	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	if(path_json == NULL){
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
    
	const char *uri_src = JSON_GET_OBJECT_VALUE(path_json,string);
	if(uri_src == NULL){
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	DMCLOG_D("uri_src = %s",uri_src);
	c->src_path = (char *)calloc(1,strlen(uri_src) + 1);
	if(c->src_path == NULL){
		c->error = DM_ERROR_ALLOCATE_MEM;
		goto EXIT;
	}
	
    if(*uri_src == '/'){
        sprintf(c->src_path,"%s",uri_src + 1);
    }else{
        sprintf(c->src_path,"%s",uri_src);
    }
	DMCLOG_D("c->src_path = %s",c->src_path);

	JObj *a_time_sec_json = JSON_GET_OBJECT(para_json,"a_time_sec");
	JObj *a_time_nsec_json = JSON_GET_OBJECT(para_json,"a_time_nsec");
	JObj *m_time_sec_json = JSON_GET_OBJECT(para_json,"m_time_sec");
	JObj *m_time_nsec_json = JSON_GET_OBJECT(para_json,"m_time_nsec");
	JObj *flags_json = JSON_GET_OBJECT(para_json,"flags");

	if(a_time_sec_json==NULL || a_time_nsec_json==NULL || m_time_sec_json==NULL || 
		m_time_nsec_json==NULL || flags_json==NULL){
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}

	c->a_time.tv_sec = JSON_GET_OBJECT_VALUE(a_time_sec_json, int);
	c->a_time.tv_nsec = JSON_GET_OBJECT_VALUE(a_time_nsec_json, int);
	c->m_time.tv_sec = JSON_GET_OBJECT_VALUE(m_time_sec_json, int);
	c->m_time.tv_nsec = JSON_GET_OBJECT_VALUE(m_time_nsec_json, int);
	c->status = JSON_GET_OBJECT_VALUE(flags_json, int);
	DMCLOG_D("a_time.sec: %d, a_time.nsec: %d, m_time.sec: %d, m_time.nsec: %d, c->status: %d", 
		c->a_time.tv_sec, c->a_time.tv_nsec, c->m_time.tv_sec, c->m_time.tv_nsec, c->status);
	
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;
}

int parser_FileSymlink(struct conn *c)
{
    JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
    if(data_json == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
    if(para_json == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
	
    JObj *src_path_json = JSON_GET_OBJECT(para_json,"srcPath");
    if(src_path_json == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    const char *srcPath = JSON_GET_OBJECT_VALUE(src_path_json,string);
    if(srcPath == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    DMCLOG_D("srcPath = %s",srcPath);

    c->src_path = (char *)calloc(1,strlen(srcPath) + 1);
    assert(c->src_path != NULL);
    if(*srcPath == '/')
    {
        sprintf(c->src_path,"%s",srcPath + 1);
    }else{
        sprintf(c->src_path,"%s",srcPath);
    }
    DMCLOG_D("c->src_path = %s",c->src_path);
    
    JObj *des_path_json = JSON_GET_OBJECT(para_json,"desPath");
    if(des_path_json == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    const char *desPath = JSON_GET_OBJECT_VALUE(des_path_json,string);
    if(desPath == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    DMCLOG_D("desPath = %s",desPath);

    c->des_path = (char *)calloc(1,strlen(desPath) + 1);
    assert(c->des_path != NULL);
    if(*desPath == '/')
    {
        sprintf(c->des_path,"%s",desPath + 1);
    }else{
        sprintf(c->des_path,"%s",desPath);
    }
    DMCLOG_D("c->des_path = %s",c->des_path);
	char *lc = get_last_char(c->des_path, '/');
	if(lc != NULL){
		*lc = 0;
	}
	DMCLOG_D("c->des_path = %s",c->des_path);
EXIT:
    if(c->r_json != NULL)
        JSON_PUT_OBJECT(c->r_json);
    return 0;	
}


int parser_FileLink(struct conn *c)
{
    JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
    if(data_json == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
    if(para_json == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
	
    JObj *src_path_json = JSON_GET_OBJECT(para_json,"srcPath");
    if(src_path_json == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    const char *srcPath = JSON_GET_OBJECT_VALUE(src_path_json,string);
    if(srcPath == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    DMCLOG_D("srcPath = %s",srcPath);

    c->src_path = (char *)calloc(1,strlen(srcPath) + 1);
    assert(c->src_path != NULL);
    if(*srcPath == '/')
    {
        sprintf(c->src_path,"%s",srcPath + 1);
    }else{
        sprintf(c->src_path,"%s",srcPath);
    }
    DMCLOG_D("c->src_path = %s",c->src_path);
    
    JObj *des_path_json = JSON_GET_OBJECT(para_json,"desPath");
    if(des_path_json == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    const char *desPath = JSON_GET_OBJECT_VALUE(des_path_json,string);
    if(desPath == NULL)
    {
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    DMCLOG_D("desPath = %s",desPath);

    c->des_path = (char *)calloc(1,strlen(desPath) + 1);
    assert(c->des_path != NULL);
    if(*desPath == '/')
    {
        sprintf(c->des_path,"%s",desPath + 1);
    }else{
        sprintf(c->des_path,"%s",desPath);
    }
    DMCLOG_D("c->des_path = %s",c->des_path);
	char *lc = get_last_char(c->des_path, '/');
	if(lc != NULL){
		*lc = 0;
	}
	DMCLOG_D("c->des_path = %s",c->des_path);
EXIT:
    if(c->r_json != NULL)
        JSON_PUT_OBJECT(c->r_json);
    return 0;	
}

int parser_FileReadlink(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}

	JObj *path_json = JSON_GET_OBJECT(para_json,"srcPath");
	if(path_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
    
	const char *uri_src = JSON_GET_OBJECT_VALUE(path_json,string);
	if(uri_src == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	DMCLOG_D("uri_src = %s",uri_src);
	c->src_path = (char *)calloc(1,strlen(uri_src) + 1);
	assert(c->src_path != NULL);
    if(*uri_src == '/')
    {
        sprintf(c->src_path,"%s",uri_src + 1);
    }else{
        sprintf(c->src_path,"%s",uri_src);
    }
	
	DMCLOG_D("c->src_path = %s",c->src_path);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;	
}

int parser_FileStatvfs(struct conn *c)
{
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}

	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	if(path_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
    
	const char *uri_src = JSON_GET_OBJECT_VALUE(path_json,string);
	if(uri_src == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	DMCLOG_D("uri_src = %s",uri_src);
	c->src_path = (char *)calloc(1,strlen(uri_src) + 1);
	assert(c->src_path != NULL);
    if(*uri_src == '/')
    {
        sprintf(c->src_path,"%s",uri_src + 1);
    }else{
        sprintf(c->src_path,"%s",uri_src);
    }
	
	DMCLOG_D("c->src_path = %s",c->src_path);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return 0;	
}

void file_parse_process(struct conn *c)
{
    uint8_t i = 0;
    uint8_t switch_flag = 0;
	file_parse_header_json(c);
	if(c->error != 0){
		return;
	}
    for(i = 0; i< FILE_TAGHANDLE_NUM; i++)
    {
        if(c->cmd == all_file_handle[i].tag)
        {
            all_file_handle[i].parsefun(c);
            switch_flag = 1;
			break;
        }
    }
    if(switch_flag == 0)
    {
        c->error = INVALIDE_COMMAND;
    }
	
	if(c->cmd != FN_ROUTER_GET_OTA_INFO
		&&c->cmd != FN_ROUTER_SET_STATUS_CHANGED_LISTENER
		&&c->cmd != FN_DM_ROUTER_GET_FUNC_LIST
		&&c->cmd != FN_ROUTER_GET_PWD_STATUS
		&&c->cmd != FN_DEL_CLIENT_INFO)
	{
		if(c->ctx->token_process != NULL)
		{
			int ret = c->ctx->token_process(c);
			if(ret != 0)
			{
				c->error = ERROR_SESSION_PROCESS;
			}
		}
	}
    return;
}


int verify_firmware()
{
	FILE *fw_fp = NULL;
	FILE *md5_fp = NULL;
	long fw_len = 0;
	int f_size = 0;
	char fw_tmp[256];
	char op_fw_header[32]="\0";
	char product_model[33]="\0";
	int ret = 0;
	char path[128];
	char cmd[128];
	char line_buff[256]="\0";
	char md5_str1[40]="\0";
	char md5_str2[40]="\0";
	char *p_stok_line=line_buff;
	char *p_stok_md5=NULL;
	memset(path, 0, sizeof(path));
	sprintf(path, "%s%s", FW_FILE, TMP_PATH_NAME);
	DMCLOG_D("update file : %s%s\n",FW_FILE, TMP_PATH_NAME);
	if( (fw_fp=fopen(path, "rb"))==NULL)    // read binary
	{
		DMCLOG_E("can't open %s%s", FW_FILE, TMP_PATH_NAME);
		return DM_ERROR_FW_FILE_CTL;
	}
	#if defined(SUPPORT_OPENWRT_PLATFORM)
		#if defined(OPENWRT_X1000)
			system("sync");
			system("echo 3 >/proc/sys/vm/drop_caches");
			system("sync");

			fclose(fw_fp);
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "mv %s%s /tmp/fwupgrade.tar.gz", FW_FILE, TMP_PATH_NAME);
			system(cmd);
			system("tar -zxf /tmp/fwupgrade.tar.gz -C /tmp");
			system("mv /tmp/6291-update-fw.bin /tmp/fwupgrade");
			if( (fw_fp=fopen("/tmp/fwupgrade", "rb"))==NULL)    // read binary
			{
				DMCLOG_E("can't open /tmp/fwupgrade");
				rm("/tmp/fwupgrade.tar.gz");
				memset(path, 0, sizeof(path));
				sprintf(path ,"%s%s", FW_FILE, TMP_PATH_NAME);
				rm(path);
				return DM_ERROR_FW_FILE_CTL;
			}
			system("md5sum /tmp/fwupgrade >/tmp/fwupgrade.md5");

			if( (md5_fp=fopen("/tmp/6291-update-fw.bin.md5","rb"))==NULL)
			{
				system("rm -f /tmp/6291-update-fw.bin.md5");
				DMCLOG_E("can't open /tmp/6291-update-fw.bin.md5");
				return DM_ERROR_FW_FILE_CTL;
			}
			fgets(line_buff,256,md5_fp);
			p_stok_line=line_buff;
			p_stok_md5=strtok(p_stok_line, " ");
			strcpy(md5_str1,p_stok_md5);
			DMCLOG_D("md5_str1 = %s\n",md5_str1);
			fclose(md5_fp);
			memset(line_buff,0,256);

			if( (md5_fp=fopen("/tmp/fwupgrade.md5","rb"))==NULL)
			{
				system("rm -f /tmp/fwupgrade.md5");
				DMCLOG_E("can't open /tmp/fwupgrade.md5");
				return DM_ERROR_FW_FILE_CTL;
			}
			fgets(line_buff,256,md5_fp);
			p_stok_line=line_buff;
			p_stok_md5=strtok(p_stok_line, " ");
			strcpy(md5_str2,p_stok_md5);
			DMCLOG_D("md5_str2 = %s\n",md5_str2);
			fclose(md5_fp);
			if(strcmp(md5_str1,md5_str2)!=0)
			{
				DMCLOG_E("fw md5sum error");
				return DM_ERROR_FW_FILE_CTL;
			}



		#endif
	#endif

	fseek(fw_fp, 0, SEEK_SET);
	memset(fw_tmp, 0, sizeof(fw_tmp));
	f_size=fread(fw_tmp,1,sizeof(fw_tmp),fw_fp);
	if(f_size != sizeof(fw_tmp))
	{
		DMCLOG_E("can't read %s%s,len = %d", FW_FILE, TMP_PATH_NAME, f_size);
		ret = DM_ERROR_FW_FILE_CTL;
		goto EXIT;
	}

	#if defined(SUPPORT_OPENWRT_PLATFORM)
		#if defined(OPENWRT_MT7628)
			memset(op_fw_header, 0, sizeof(op_fw_header));
			strncpy(op_fw_header, fw_tmp+0x25, 7);
			if( strcmp(op_fw_header,"OpenWrt")!=0 )
			{
				DMCLOG_E("fw verify error, header error!");
				ret = DM_ERROR_FW_HEADER;
				goto EXIT;
			}

			memset(product_model, 0, sizeof(product_model));
			strncpy(product_model, fw_tmp+0x33, 13);
			if(strcmp(product_model, get_sys_product_model())!=0 )
			{
				DMCLOG_E("fw verify error, expect model : %s, real model : %s!", get_sys_product_model(), product_model);
				ret = DM_ERROR_FW_PROJECT_NUMBER;
				goto EXIT;
			}	

			fseek(fw_fp,0,SEEK_END);
			fw_len = ftell(fw_fp);
			if(fw_len < FW_LEN)
			{			
				DMCLOG_E("fw verify error, expect len : %d, real len = %d!", FW_LEN, fw_len);
				ret = DM_ERROR_FW_LENGTH;
				goto EXIT;
			}
		#elif defined(OPENWRT_X1000)
			memset(product_model, 0, sizeof(product_model));
			strncpy(product_model, fw_tmp+0x20, 32);
			if(strcmp(product_model,get_sys_product_model())!=0 )
			{
				DMCLOG_E("fw verify error, expect model : %s, real model : %s!", get_sys_product_model(), product_model);
				ret = DM_ERROR_FW_PROJECT_NUMBER;
				goto EXIT;
			}
		
		#endif
	#endif

	fclose(fw_fp);
	return 0;
EXIT:
	fclose(fw_fp);
	#if defined(SUPPORT_OPENWRT_PLATFORM)
		#if defined(OPENWRT_X1000)
			rm("/tmp/fwupgrade.tar.gz");
			rm("/tmp/fwupgrade");
			memset(path, 0, sizeof(path));
			sprintf(path ,"%s%s", FW_FILE, TMP_PATH_NAME);
			rm(path);
		#else
			memset(path, 0, sizeof(path));
			sprintf(path ,"%s%s", FW_FILE, TMP_PATH_NAME);
			rm(path);
		#endif
	#endif
	return ret;
}



