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
#include "file_json.h"
#include "base.h"
#include "disk_manage.h"
#include "router_inotify.h"
#include "socket_uart.h"
#include "defs.h"
#include "router_defs.h"
#include "config.h"

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
	{FN_DM_LOGIN,dm_login,Parser_Login},
	{FN_DM_LOGOUT,dm_logout,Parser_Logout},
	{FN_GET_SERVICE_INFO,dm_get_service_info,Parser_GetServiceInfo},
	{FN_DEL_CLIENT_INFO,_dm_del_client_info,Parser_DelClientInfo},
	{FN_DISK_SCANNING,dm_disk_scanning,Parser_DiskScanning},
	{FN_DM_ROUTER_GET_OPTION,dm_router_get_option,Parser_RouterGetOption},
	{FN_DM_ROUTER_GET_FUNC_LIST,dm_router_get_func_list,Parser_RouterGetFuncList},
	{FN_DM_ROUTER_GET_STATUS_CHANGED,dm_router_get_status_changed,Parser_RouterGetStatusChanged},
	{FN_ROUTER_SET_STATUS_CHANGED_LISTENER,dm_router_set_status_changed_listener,Parser_RouterSetStatusListen},
	{FN_ROUTER_GET_WIFI_INFO,_dm_get_wifi_settings,Parser_RouterGetWifiSettings},
	{FN_ROUTER_SET_WIFI_INFO,_dm_set_wifi_settings,Parser_RouterSetWifiSettings},
	{FN_ROUTER_GET_REMOTE_AP_INFO,_dm_get_remote_ap_info,Parser_RouterGetRemoteInfo},
	{FN_ROUTER_SET_REMOTE_AP_INFO,_dm_set_remote_ap_info,Parser_RouterSetRemoteInfo},
	{FN_ROUTER_GET_CONNECT_MODE,_dm_get_wlan_con_mode,Parser_RouterGetWlanConMode},
	{FN_ROUTER_GET_WIRED_INFO,_dm_get_wired_con_mode,Parser_RouterGetWireConMode},
	{FN_ROUTER_SET_WIRED_INFO,_dm_set_wired_con_mode,Parser_RouterSetWireConMode},
	{FN_ROUTER_GET_REMOTE_AP_LIST,_dm_get_wlan_list,Parser_RouterGetWlanList},
	{FN_ROUTER_GET_POWER_INFO,_dm_get_power,Parser_RouterGetPower},
	{FN_ROUTER_GET_DISK_INFO,_handle_getStorageInfo,Parser_RouterGetStorageInfo},
	{FN_ROUTER_SET_DISK_FORMAT,_dm_format_disk,Parser_RouterFormatDisk},
	{FN_ROUTER_GET_FTP_INFO,_dm_get_ftp_settings,Parser_RouterGetFtpSettings},
	{FN_ROUTER_SET_FTP_INFO,_dm_set_ftp_settings,Parser_RouterSetFtpSettings},
	{FN_ROUTER_GET_SAMBA_INFO,_dm_get_smb_settings,Parser_RouterGetSMBSettings},
	{FN_ROUTER_SET_SAMBA_INFO,_dm_set_smb_settings,Parser_RouterSetSMBSettings},
	{FN_ROUTER_GET_DMS_INFO, _dm_get_dms_settings,Parser_RouterGetDMSSettings},
	{FN_ROUTER_SET_DMS_INFO, _dm_set_dms_settings,Parser_RouterSetDMSSettings},
	{FN_ROUTER_GET_DDNS_INFO, _dm_get_ddns_settings,Parser_RouterGetDDNSSettings},
	{FN_ROUTER_SET_DDNS_INFO, _dm_set_ddns_settings,Parser_RouterSetDDNSSettings},
	{FN_ROUTER_GET_WEBDAV_INFO, _dm_get_webdav_settings,Parser_RouterGetWebDavSettings},
	{FN_ROUTER_SET_WEBDAV_INFO, _dm_set_webdav_settings,Parser_RouterSetWebDavSettings},
	{FN_ROUTER_GET_ELC_LOCK_INFO, _dm_get_elec_lock,Parser_RouterGetElecSettings},
	{FN_ROUTER_SET_ELC_LOCK_INFO, _dm_set_elec_lock,Parser_RouterSetElecSettings},
	{FN_ROUTER_GET_3G_INFO, _dm_get_3g_access_info,Parser_RouterGet3gSettings},
	{FN_ROUTER_SET_3G_INFO, _dm_set_3g_access_info,Parser_RouterSet3gSettings},
	{FN_ROUTER_GET_CLIENT_STATUS, _dm_get_client_status,Parser_RouterGetClientStatus},
	{FN_ROUTER_SET_CLIENT_STATUS, _dm_set_client_status,Parser_RouterSetClientStatus},
	{FN_ROUTER_SET_UDISK_UPGRADE, _dm_udisk_upgrade,Parser_RouterUdiskUpgrade},
	{FN_ROUTER_GET_INTERNET_STATUS, _dm_get_internet_status,Parser_RouterGetInternetStatus},
	{FN_ROUTER_SET_TIME_SYNC, _dm_sync_time,Parser_RouterSyncTime},
	{FN_ROUTER_GET_FIRMWARE_VERSION, _dm_get_fw_version,Parser_RouterGetFwVersion},
	{FN_ROUTER_GET_OTA_INFO, _dm_get_ota_info,Parser_RouterGetOtaInfo}, 
	{FN_ROUTER_GET_VERSION_FLAG, _dm_get_version_flag,Parser_RouterGetVersionFlag},
	{FN_ROUTER_SET_VERSION_FLAG, _dm_set_version_flag,Parser_RouterSetVersionFlag},
	{FN_ROUTER_SET_UPLOAD_FIRMWARE, _dm_set_upload_fw, Parser_RouterSetUploadFw},
	{FN_ROUTER_SET_UPGRADE_FIRMWARE, _dm_set_upgrade_fw, Parser_RouterSetUpgradeFw},
	{FN_ROUTER_SET_FORGET_WIFI_INFO, _dm_set_forget_wifi_info, Parser_RouterSetForgetWifiInfo},
	{FN_FILE_GET_STORAGE,DM_FileGetStorage,Parser_FileGetStorage},
	{FN_FILE_GET_LIST,DM_FileGetList,Parser_FileGetList},
	{FN_FILE_MKDIR,DM_FileMkdir,Parser_FileMkdir},
	{FN_FILE_RENAME,DM_FileRename,Parser_FileRename},
	{FN_FILE_IS_EXIST,DM_FileIsExist,Parser_FileIsExist},
	{FN_FILE_GET_ATTR,DM_FileGetAttr,Parser_FileGetAttr},
	{FN_FILE_DOWNLOAD,dm_file_download,Parser_FileDownload},
	{FN_FILE_UPLOAD,dm_file_upload,Parser_FileUpload},
	{FN_FILE_CHECKUPLOAD,dm_file_check_upload,Parser_FileCheckUpload},
	{FN_FILE_DELETE,DM_FileDelete,parser_FileDelete},
    {FN_FILE_COPY,DM_FileCopy,parser_FileCopy},
    {FN_FILE_MOVE,DM_FileMove,parser_FileMove},
	{FN_FILE_GET_LSIT_BY_TYPE,_DM_FileGetListByType,Parser_FileGetListByType},
	{FN_FILE_GET_DIR_BY_TYPE,_DM_FileGetListByType,Parser_FileGetFilePathByType},
	{FN_FILE_GET_CLASS_INFO,DM_FileGetClassInfo,Parser_FileGetClassInfo},
	{FN_FILE_GET_LIST_BY_PATH,_DM_FileGetListByType,Parser_FileGetFileListByPath},
	{FN_FILE_DEL_LIST_BY_PATH,DM_FileDelListByPath,Parser_FileDelFileListByPath},
	{FN_FILE_GET_SCAN_STATUS,DM_FileGetScanStatus,Parser_FileGetScanStatus},
	{FN_FILE_GET_BACKUP_INFO,DM_FileGetBackupInfo,Parser_FileGetBackupInfo},
	{FN_FILE_BIND_BACKUP_DISK,DM_FileBindBackupDisk,Parser_FileBindBackupDisk},
	{FN_FILE_GET_BACKUP_FILE,DM_FileGetBackupFile,Parser_FileGetBackupFile},
	{FN_FILE_CHECK_BACKUP_FILE,DM_FileCheckBackupFile,Parser_FileCheckBackupFile},
	{FN_FILE_CHECK_BACKUP_FILE_LIST,DM_FileCheckBackupFileList,Parser_FileCheckBackupFileList},
	{FN_FILE_IS_BACKUP_EXISTED,DM_FileIsBackupExisted,Parser_FileIsBackupFile},
	{FN_FILE_RESET_BACKUP,DM_FileResetBackup,Parser_FileResetBackup},
};

#define FILE_TAGHANDLE_NUM (sizeof(all_file_handle)/sizeof(all_file_handle[0]))

/*
 * Try to open requested file, return 0 if OK, -1 if error.
 * If the file is given arguments using PATH_INFO mechanism,
 * initialize pathinfo pointer.
 */
static int
get_path_info(struct conn *c, char *path, struct stat *stp)
{
	char	*p, *e;
	if (my_stat(path, stp) == 0)
		return (0);
	return (-1);
}

static int 
dm_getFileList(const char *path,unsigned long offset,unsigned long length,JObj *response_para_json)
{
	struct dirent *entry;
	DIR *dir;
	unsigned i= 0;
	struct stat statbuf;
	dir = warn_opendir(path);
	if (dir == NULL) {
		return -1;	/* could not open the dir */
	}
	DMCLOG_D("start:offset = %lu,length = %lu,",offset,length);
	JObj *file_info_array = JSON_NEW_ARRAY();
	while ((entry = readdir(dir)) != NULL) {
		char *fullname;
		/* are we going to list the file- it may be . or .. or a hidden file */
		if (entry->d_name[0] == '.') {
			/*if ((!entry->d_name[1] || (entry->d_name[1] == '.' && !entry->d_name[2]))) {
				continue;
			}
			if(!strcmp(entry->d_name,get_sys_disk_uuid_name())||!strcmp(entry->d_name,get_sys_db_name()))
			{
				continue;
			}*/
			continue;
		}
		if(i >= offset&&i < length)
		{
			fullname = concat_path_file(path, entry->d_name);
			memset(&statbuf,0,sizeof(struct stat));
			if (lstat(fullname, &statbuf)) {
				printf(fullname);
				safe_close(fullname);
				break;
			}
			JObj *file_info = JSON_NEW_EMPTY_OBJECT();
			if(S_ISDIR(statbuf.st_mode))
			{
				JSON_ADD_OBJECT(file_info, "isFolder",JSON_NEW_OBJECT(1,int));
			}else if(S_ISREG(statbuf.st_mode))
			{
			 	JSON_ADD_OBJECT(file_info, "isFolder",JSON_NEW_OBJECT(0,int));
			}
			
			//DMCLOG_D("i = %d,fileName = %s,st_size = %zd",i,entry->d_name,statbuf.st_size);
			JSON_ADD_OBJECT(file_info, "size",JSON_NEW_OBJECT((unsigned long long)statbuf.st_size,int64));
			JSON_ADD_OBJECT(file_info, "data",JSON_NEW_OBJECT(statbuf.st_ctime,int));
			JSON_ADD_OBJECT(file_info, "name",JSON_NEW_OBJECT(entry->d_name,string));
			JSON_ADD_OBJECT(file_info,"path",JSON_NEW_OBJECT(fullname + strlen(DOCUMENT_ROOT) + 1,string));
			JSON_ARRAY_ADD_OBJECT (file_info_array,file_info);
			safe_free(fullname);
		}
		i++;
	}
	JSON_ADD_OBJECT(response_para_json, "filelist", file_info_array);
	closedir(dir);
	/* now that we know how many files there are
	 * allocate memory for an array to hold dnode pointers
	 */
	return 0;
}

int dm_file_download(struct conn *c)
{
	int res = 0;
	struct stat	st;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	if(c->error != 0)
	{
		goto EXIT;
	}
	if (get_path_info(c, c->src_path, &st) != 0) {
		c->error = FILE_IS_NOT_EXIST;
		goto EXIT;
	} else if (S_ISDIR(st.st_mode) && c->src_path[strlen(c->src_path) - 1] != '/') {
		c->error = SERVER_MOVED_LOCATION;
		goto EXIT;
	}
	DMCLOG_D("path = %s",c->src_path);
	if ((c->loc.chan.fd = my_open(c->src_path,
	    O_RDONLY | O_BINARY, 0644)) != -1) {
		get_file(c, &st);
	} else {
		c->error = SERVER_ERROR;
	}
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* contentLength_json = JSON_NEW_EMPTY_OBJECT();
	DMCLOG_D("DM_File_Download");
	JSON_ADD_OBJECT(contentLength_json, "contentLength",JSON_NEW_OBJECT(c->loc.content_len,int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,contentLength_json);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
EXIT:
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int dm_file_check_upload(struct conn *c)
{
	int res = 0;
	struct stat	st;
	struct stat statbuf; 
	unsigned i = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	if(c->error != 0)
		goto EXIT;
	if(my_stat(c->cfg_path, &st) == 0)
	{
		DMCLOG_D("cfg path is exist");
		c->status = 1;
		c->dn = NULL;
		res = read_list_from_file(c->cfg_path,&c->dn);
		//if(res < 0)
			//c->error == 108;// 获取未完成的文件起始地址信息失败
	}else{
		DMCLOG_D("cfg_path is not exist");
		c->status == 0;
	}
	 
	JObj* data_json = JSON_NEW_EMPTY_OBJECT();
	DMCLOG_D("file check upload,status = %d",c->status);
	if(c->status == 1)//存在未完成的tmp文件。
	{
		my_stat(c->tmp_path,&statbuf);  
    	unsigned long file_size = (unsigned long)statbuf.st_size;
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
		 if(p_dn->start > file_size)
	   	 {
			p_dn->start = file_size;
		 }
		 JSON_ADD_OBJECT(upload_record_json, "start", JSON_NEW_OBJECT(p_dn->start,int64));
		 JSON_ADD_OBJECT(upload_record_json, "end", JSON_NEW_OBJECT(p_dn->end,int64));
		 DMCLOG_D("i = %d,dn->index = %s",i,p_dn->index);
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
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}
int dm_file_upload_inotify(int cmd, char *buf)
{
	ENTER_FUNC();
	int res = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* status_json = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(status_json, "status",JSON_NEW_OBJECT(0,int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,status_json);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);	
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(0,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(0,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	char *response_str = JSON_TO_STRING(response_json);
	strcpy(buf,response_str);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
}

int dm_file_copy_inotify(int cmd, int error,char *buf)
{
    ENTER_FUNC();
    int res = 0;
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
    char *response_str = JSON_TO_STRING(response_json);
    strcpy(buf,response_str);
    if(res < 0)
    {
        JSON_PUT_OBJECT(response_json);
		
        return -1;
    }
    JSON_PUT_OBJECT(response_json);
    DMCLOG_D("buf = %s",buf);
    EXIT_FUNC();
}

int dm_router_upload_fw_inotify(int cmd, int status, char *buf)
{
	ENTER_FUNC();
	int res = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* status_json = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(status_json, "status",JSON_NEW_OBJECT(0, int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,status_json);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);	
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(0,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(status,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	char *response_str = JSON_TO_STRING(response_json);
	strcpy(buf,response_str);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
}


int dm_file_upload(struct conn *c)
{
	ENTER_FUNC();
	struct stat	st;
	DMCLOG_D("c->error = %d",c->error);
	struct stream *from;
	if(c->error != 0)
	{
		c->loc.flags |= FLAG_CLOSED;
		from = &c->loc;
		dm_file_upload_inotify(c->cmd, io_data(&from->io));
		goto EXIT;
	}
	if (c->rem.content_len == 0) {
		c->loc.flags |= FLAG_CLOSED;
		c->error = LENGTH_REQUIRED;
		from = &c->loc;
		dm_file_upload_inotify(c->cmd, io_data(&from->io));
		DMCLOG_D("c->error = %s",c->error);
		goto EXIT;
	} else if ((c->loc.chan.fd = my_open(c->tmp_path, O_WRONLY | O_CREAT, 0644)) == -1) {
		c->loc.flags |= FLAG_CLOSED;
		c->error = CREATE_FILE_ERROR;
		DMCLOG_D("c->error = %d,errno = %d",CREATE_FILE_ERROR,errno);
		from = &c->loc;
		dm_file_upload_inotify(c->cmd, io_data(&from->io));
		goto EXIT;
	} else {
		DMCLOG_D("PUT file [%s]", c->tmp_path);
		c->loc.io_class = &io_file;
		//c->loc.flags |= FLAG_W | FLAG_ALWAYS_READY ;
		c->loc.flags |= FLAG_W;
		DMCLOG_D("upload c->offset = %lu",c->offset);
		(void) lseek(c->loc.chan.fd, c->offset, SEEK_SET);
		if(my_stat(c->cfg_path, &st) != 0)
		{
			//如果记录文件块起始地址的配置文件不存在，则将信息写入
			int i = 0;
			int percent = c->fileSize/THREAD_COUNT;
			c->dn = NULL;
			for(; i < THREAD_COUNT;i++)
		    {
		        if(i == THREAD_COUNT - 1)
					add_record_to_list(&c->dn,i,i * percent,c->fileSize);
		        else
					add_record_to_list(&c->dn,i,i * percent,(i + 1)* percent);
		    }
			display_record_dnode(c->dn);
			write_list_to_file(c->cfg_path,c->dn);
			destory_record_list(c->dn);
			gettimeofday(&c->tstart, NULL);
		}
	}
	#if 0
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* status_json = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(status_json, "status",JSON_NEW_OBJECT(c->status,int));
	JSON_ARRAY_ADD_OBJECT (response_data_array,status_json);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
EXIT:	
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	#endif
EXIT:	
	EXIT_FUNC();
	return 0;
}

/*
 * 获取磁盘信息 cmd = 100
 */
int DM_FileGetStorage(struct conn *c)
{
	ENTER_FUNC();
	JObj *response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj* data_json=JSON_NEW_EMPTY_OBJECT();
	if(c->error != 0)
	{
		goto EXIT;
	}
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj *disk_info_array = JSON_NEW_ARRAY();
	int res = 0;
	int i = 0;
	DMCLOG_D("c->error = %d",c->error);
	if(c->error != 0)
		goto EXIT;
	all_disk_t mAll_disk_t;
	memset(&mAll_disk_t,0,sizeof(all_disk_t));
	mAll_disk_t.storage_dir = STORAGE_BOARD;
	res = dm_get_storage(&mAll_disk_t);
	if(res != 0)
	{
		c->error = ERROR_GET_DISK_INFO;
		goto EXIT;
	}
	
 	DMCLOG_D("drive_count = %d",mAll_disk_t.count);
#ifdef MCU_COMMUNICATE_SUPPORT
	if(0 != dm_get_storage_dir(&mAll_disk_t.storage_dir))
		mAll_disk_t.storage_dir = STORAGE_BOARD;
#endif	

	for(i = 0;i < mAll_disk_t.count;i++)
	{
		JObj* drive_info = JSON_NEW_EMPTY_OBJECT();
		JSON_ADD_OBJECT(drive_info, "displayName",JSON_NEW_OBJECT(mAll_disk_t.disk[i].name,string));
		JSON_ADD_OBJECT(drive_info, "path",JSON_NEW_OBJECT(mAll_disk_t.disk[i].path,string));
		JSON_ADD_OBJECT(drive_info, "size",JSON_NEW_OBJECT(mAll_disk_t.disk[i].total_size,int64));
		JSON_ADD_OBJECT(drive_info, "avSize",JSON_NEW_OBJECT(mAll_disk_t.disk[i].free_size,int64));
		JSON_ADD_OBJECT(drive_info, "diskType",JSON_NEW_OBJECT(mAll_disk_t.disk[i].type,int));
		JSON_ADD_OBJECT(drive_info, "auth",JSON_NEW_OBJECT(mAll_disk_t.disk[i].auth,int));
		JSON_ARRAY_ADD_OBJECT (disk_info_array,drive_info);	
	}
	JSON_ADD_OBJECT(data_json, "diskDir",JSON_NEW_OBJECT(mAll_disk_t.storage_dir, int));
	JSON_ADD_OBJECT(data_json, "diskInfoList", disk_info_array);
	JSON_ARRAY_ADD_OBJECT (response_data_array, data_json);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
EXIT:
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
	return 0;
}

int _DM_FileGetList(struct conn *c)
{
	ENTER_FUNC();
	struct stat	st;
	DMCLOG_D("c->error = %d",c->error);
	if(c->error != 0)
	{
		c->loc.flags |= FLAG_CLOSED;
		goto EXIT;
	}
	if(my_stat(c->src_path, &st) != 0 || S_ISDIR(st.st_mode) == 0)
	{
		c->loc.flags |= FLAG_CLOSED;
		c->error = ERROR_GET_FILE_LIST;
		goto EXIT;
	}

	c->loc.chan.dir.path = c->src_path;
	DMCLOG_D("dir.path = %s",c->src_path);
	get_dir(c);
EXIT:	
	EXIT_FUNC();
	return 0;
}


/*
 * 获取文件列表 cmd = 101
 */
int DM_FileGetList(struct conn *c)
{
	int res = 0;
	int count = 0;
	if(c->pageNum < 0)
	{
		return _DM_FileGetList(c);	
	}
	JObj *response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	if(c->error != 0)
	{
		goto EXIT;
	}
	
	if(c->error != 0)
		goto EXIT;
	struct file_list file_list_t;
	memset(&file_list_t,0,sizeof(struct file_list));
	if((res = is_file_exist(c->src_path)) != 0)
	{
		c->error = FILE_IS_NOT_EXIST;
		goto EXIT;
	}
	res = dm_getDirInfo(c->src_path,&file_list_t);
	DMCLOG_D("path = %s",c->src_path);
	if(res < 0)
	{
		c->error = ERROR_GET_FILE_LIST;
		goto EXIT;
	}
	if(c->pageNum >= file_list_t.totalPage)
	{
		DMCLOG_D("c->pageNum = %d",c->pageNum);
		c->error = ERROR_GET_FILE_NULL;
		goto EXIT;
	}
	c->offset = c->pageNum*PAGESIZE;
	if(c->pageNum == file_list_t.totalPage - 1)
	{
		c->length = file_list_t.totalCount;
	}
	else
	{
		c->length = (c->pageNum + 1)*PAGESIZE;
	}
	JObj *response_para_json=JSON_NEW_EMPTY_OBJECT();
	DMCLOG_D("c->offset = %lu,c->length = %lu",c->offset,c->length);
	 res = dm_getFileList(c->src_path,c->offset,c->length,response_para_json);
	if(res != 0)
	{
		c->error = ERROR_GET_FILE_LIST;
		goto EXIT;
	}
    count = c->length - c->offset;
	JSON_ADD_OBJECT(response_para_json, "count", JSON_NEW_OBJECT(count,int));
	JSON_ADD_OBJECT(response_para_json, "totalCount", JSON_NEW_OBJECT(file_list_t.totalCount,int));
	JSON_ADD_OBJECT(response_para_json, "totalPage", JSON_NEW_OBJECT(file_list_t.totalPage,int));
	JSON_ADD_OBJECT(response_para_json, "pageSize", JSON_NEW_OBJECT(file_list_t.pageSize,int));
	JSON_ADD_OBJECT(response_json, "data", response_para_json);
EXIT:
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	c->loc.flags |= FLAG_CLOSED;
	return 0;
}

/*
 * 文件夹创建 cmd = 102
 */
int DM_FileMkdir(struct conn *c)
{
	ENTER_FUNC();
	int res = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	if(c->error != 0)
	{
		goto EXIT;
	}
	DMCLOG_D("DM_FileMkdir");
	if((res = make_directory(c->src_path)) != RET_SUCCESS)
    {
        DMCLOG_D("make_dirs(%s) failed, ret(0x%x)", c->src_path, res);
        c->error = ERROR_MKDIR;   
		goto EXIT;
    }
	#ifdef DB_TASK
	char uuid[32];
	memset(uuid,0,32);
	res = read_mark_file(c->disk_root,uuid);
	if(res < 0)
	{
        DMCLOG_D("make_dirs(%s) failed, ret(0x%x)", c->src_path, res);
        c->error = ERROR_GET_DISK_INFO;  
		goto EXIT;
    }
	int bIsRegularFile = 0;
	res = handle_file_insert_cmd(c->file_uuid,c->src_path,bIsRegularFile,uuid);
	if(res < 0)
	{
        DMCLOG_D("make_dirs(%s) failed, ret(0x%x)", c->src_path, res);
        c->error = DM_ERROR_DB_MKDIR;  
		rm(c->src_path);
		goto EXIT;
    }
	#endif
EXIT:
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
	return 0;
}
/*
 * 文件或文件夹重名 cmd = 103
 */
int DM_FileRename(struct conn *c)
{
	int res = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	if(c->error != 0)
	{
		goto EXIT;
	}
	DMCLOG_D("DM_FileRename");
	if((res = is_file_exist(c->src_path)) != 0)
	{
		c->error = FILE_IS_NOT_EXIST;
		goto EXIT;
	}
	if((res = dm_rename(c->src_path, c->des_path)) != RET_SUCCESS)
    {
        DMCLOG_D("rename_(%s) failed, ret(0x%x)", c->des_path, res);
        c->error = ERROR_RENAME;
		goto EXIT;
    }
	#ifdef DB_TASK
	char uuid[32];
	memset(uuid,0,32);
	res = read_mark_file(c->disk_root,uuid);
	if(res < 0)
	{
        c->error = ERROR_GET_DISK_INFO;  
		goto EXIT;
    }
	DMCLOG_D("c->src_path = %s,uuid = %s",c->src_path,uuid);
	res = handle_file_rename_cmd(c->src_path,c->des_path,uuid);
	if(res < 0)
	{
        DMCLOG_D("db rename (%s) failed, ret(0x%x)", c->src_path, res);
        c->error = DM_ERROR_DB_RENAME;   
		goto EXIT;
    }
	#endif
EXIT:
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}
/*
 * 判断文件或文件夹是否存在 cmd = 104
 */
int DM_FileIsExist(struct conn *c)
{
	int res = 0;
	JObj* response_json = JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	if(c->error != 0)
	{
		goto EXIT;
	}
	DMCLOG_D("DM_FileIsExist");
	 if((res = is_file_exist(c->src_path)) != RET_SUCCESS)
    {
        DMCLOG_D("access(%s) failed, ret(0x%x)", c->src_path, res);
        c->error = FILE_IS_NOT_EXIST;
    }	
EXIT:
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	return 0;
}
/*
 * 获取文件或文件夹属性 cmd = 105
 */
int DM_FileGetAttr(struct conn *c)
{
	JObj *response_json=JSON_NEW_EMPTY_OBJECT();
	JObj *header_json = JSON_NEW_EMPTY_OBJECT();
	if(c->error != 0)
	{
		goto EXIT;
	}
	int res = 0;
	unsigned long total_size = 0;
	unsigned nfiles = 0;
	if((res = is_file_exist(c->src_path)) != 0)
	{
		c->error = FILE_IS_NOT_EXIST;
		goto EXIT;
	}
	res = copy_compute(c->src_path,&total_size,&nfiles);
    if(res < 0)
    {
        DMCLOG_D("get file attr error");
		c->error = ERROR_GET_FILE_ATTR;
		goto EXIT;
    }
	struct file_dnode* dn = dm_get_file_attr(c->src_path);
	if(res != 0)
	{
		DMCLOG_D("get file attr error");
		c->error = ERROR_GET_FILE_ATTR;
		goto EXIT;
	}
    DMCLOG_D("filePath = %s,utime = %lu,total_size = %lu,total_count = %u",dn->name,dn->date,total_size,nfiles);
	JObj *response_data_array = JSON_NEW_ARRAY();
	JObj* file_info = JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(file_info, "fileSize",JSON_NEW_OBJECT(total_size,int64));
	JSON_ADD_OBJECT(file_info, "totalCount",JSON_NEW_OBJECT(nfiles,int));
	JSON_ADD_OBJECT(file_info, "data",JSON_NEW_OBJECT(dn->date,int));
	JSON_ADD_OBJECT(file_info, "zuth",JSON_NEW_OBJECT(0,int));
	JSON_ARRAY_ADD_OBJECT(response_data_array,file_info);
	JSON_ADD_OBJECT(response_json, "data", response_data_array);
	safe_free(dn);
EXIT:
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	safe_free(dn);
	JSON_PUT_OBJECT(response_json);
	return 0;
}

int del_file_inotify(const char *path,copy_info_t *pInfo)
{
    struct stat path_stat;
	if(*pInfo->flags & FLAG_CLOSED) 
	{
		DMCLOG_E("quit the del task");
		return 0;
	}
    if(lstat(path,&path_stat) < 0)
    {
        return -1;
    }

    //directory
    if(S_ISDIR(path_stat.st_mode))
    {
        DIR *dp;
        struct dirent *d;
        //int status = 0;

        dp = opendir(path);
        if (dp == NULL)
        {
    	    return -1;
        }

        while((d = readdir(dp)) != NULL)
        {
            char *new_path;

            if(strcmp(d->d_name, ".") == 0
              || strcmp(d->d_name, "..") == 0)
            {
                continue;
            }
 
            new_path = concat_path_file(path, d->d_name);
            if(del_file_inotify(new_path,pInfo) < 0)
            {
                printf("remove file %s failed,errno = %d\n", new_path,errno);
                free(new_path);
                closedir(dp);
                return -1;
            }
            free(new_path);
        } 
        closedir(dp);

        if (rmdir(path) < 0)
        {
	    	return -1;
        }
		pInfo->cur_nfiles++;
        return 0; 
    }
    //regular file
    if (unlink(path) < 0) 
    {
        printf("unlink file %s failed,errno = %d\n", path,errno);
        return -1;
    }
	
	char file_uuid[32];
	memset(file_uuid,0,32);
    int ret = handle_get_file_uuid_cmd(path,file_uuid,pInfo->disk_uuid);
	if(ret == 0)
	{
		if((ret = handle_backfile_delete_cmd(file_uuid,pInfo->disk_uuid)) != RET_SUCCESS)
		{
			DMCLOG_D("del back file error");
			return -1;
		}
	}
	#if 0
	ret = handle_query_file_cmd(path,pInfo->disk_uuid);
	if(ret == 0)
	{
		ret = handle_file_generic_delete_cmd(path,pInfo->disk_uuid);
		if(ret < 0)
		{
	        DMCLOG_D("db rm(%s) failed, ret(0x%x)", path, ret);
			return -1;
	    }
	}
	#endif
    pInfo->cur_nfiles++;
	if(pInfo->cur_nfiles/10&&pInfo->cur_nfiles%10 == 0)
	{
		pInfo->status = 1;
		if(copy_handle_inotify(pInfo) < 0)
		{
			return -1;
		}
	}
    return 0;
}

static void del_inotify_func(void *self)
{
    ENTER_FUNC();
    copy_info_t *pInfo = (copy_info_t *)self;
    char *source_file = pInfo->src_path;
    int res = 0;
	int error = 0;
	char buf[1024];
    memset(buf,0,1024);
    unsigned long total_size = 0;
    DMCLOG_D("source_file = %s",source_file);
    res = get_info_from_hd_list(pInfo->session,pInfo->ip,&(pInfo->port));
    if(res < 0)
    {
    	error = ERROR_FILE_DELETE;
		pInfo->status = -1;
        EXIT_FUNC();
		goto EXIT;
    }
    //1:计算需要复制的文件或者文件夹信息,并通知反向推送服务进行信息发送
    pInfo->status = 0;//复制操作计算中
    copy_handle_inotify(pInfo);
    res = copy_compute(source_file,&pInfo->total_size,&pInfo->nfiles);
    if(res < 0)
    {
        error = ERROR_FILE_DELETE;
		pInfo->status = -1;
        EXIT_FUNC();
		goto EXIT;
    }
	 DMCLOG_D("nfiles = %u",pInfo->nfiles);
    //2:复制每一个文件到指定的文件夹路径，并通过反向推送服务反馈实时传输的文件信息
    if(del_file_inotify((const char*)source_file,pInfo) < 0)
    {
        error = ERROR_FILE_DELETE;
		pInfo->status = -1;
        EXIT_FUNC();
		goto EXIT;
    }
  	
    
	res = handle_file_delete_cmd(pInfo->src_path,pInfo->disk_uuid);
	if(res < 0)
	{
        DMCLOG_D("db rm(%s) failed, ret(0x%x)", pInfo->src_path, res);
      	pInfo->status = -1;
		 error = ERROR_FILE_DELETE;
        EXIT_FUNC();
		goto EXIT;
    }
	pInfo->status = 2;
EXIT:
	copy_handle_inotify(pInfo);
	//if(*(struct conn **)pInfo->con != NULL)
	{
		dm_file_copy_inotify(pInfo->cmd,error,buf);    
	    DM_MsgSend(pInfo->sock,buf,strlen(buf));
	    *(pInfo->flags) |= FLAG_CLOSED;
	    *(pInfo->flags) &= ~(FLAG_R | FLAG_W | FLAG_ALWAYS_READY);
	}
    
    safe_free(source_file);
    safe_free(self);
    EXIT_FUNC();
    return ;
}

int create_del_task(struct conn *c,const char *uuid)
{
    ENTER_FUNC();
    PTHREAD_T tid_del_task;
    int rslt = 0;
    copy_info_t *pInfo = (copy_info_t *)calloc(1,sizeof(copy_info_t));
    pInfo->flags = &c->rem.flags;
    pInfo->sock = c->rem.chan.sock;
    pInfo->cmd = c->cmd;
    strcpy(pInfo->session,c->session);
	if(uuid != NULL)
		strcpy(pInfo->disk_uuid,uuid);
    pInfo->seq = c->del_seq;
    pInfo->src_path = (char *)calloc(1,strlen(c->src_path) + 1);
    strcpy(pInfo->src_path,c->src_path);
    if (0 != (rslt = PTHREAD_CREATE(&tid_del_task, NULL, (void *)del_inotify_func,pInfo)))
    {
        DMCLOG_D("del task failed!");
        rslt = -1;
        return rslt;
    }
    PTHREAD_DETACH(tid_del_task);
    EXIT_FUNC();
    return rslt;
}

int DM_FileDelete(struct conn *c)
{
	ENTER_FUNC();
	int res = 0;
	JObj* response_json;
	JObj* header_json;
	if(c->error != 0)
	{
		goto EXIT;
	}
	if((res = is_file_exist(c->src_path)) != 0)
	{
		c->error = FILE_IS_NOT_EXIST;
		goto EXIT;
	}
#ifdef DB_TASK
	char uuid[32];
	memset(uuid,0,32);
	res = read_mark_file(c->disk_root,uuid);
	if(res < 0)
	{
        c->error = ERROR_GET_DISK_INFO;  
		goto EXIT;
    }
	if(!*uuid )
	{
		DMCLOG_D("uuid is null");
		c->error = DM_ERROR_DB_DELETE; 
		goto EXIT;
	}
#endif
	if(c->del_seq != 0)
	{
		c->loc.flags |= FLAG_W;
		res = create_del_task(c,uuid);
		if(res < 0)
		{
		    DMCLOG_D("delete(%s) failed", c->src_path);
		    c->error = ERROR_FILE_DELETE;
		    return -1;
		}
		return 0;
	}
	if((res = db_backup_file_rm(c->src_path,uuid)) != RET_SUCCESS)
    {
        DMCLOG_D("delete_(%s) failed from backup db, res(%d)", c->src_path, res);
        c->error = ERROR_FILE_DELETE;
		goto EXIT;
    }

	res = handle_file_delete_cmd(c->src_path,uuid);
	if(res < 0)
	{
        DMCLOG_D("db rm(%s) failed, ret(0x%x)", c->src_path, res);
        c->error = DM_ERROR_DB_DELETE; 
		goto EXIT;
    }
	if((res = rm(c->src_path)) != RET_SUCCESS)
    {
        DMCLOG_D("delete_(%s) failed, res(%d)", c->src_path, res);
        c->error = ERROR_FILE_DELETE;
		goto EXIT;
    }

EXIT:
	response_json = JSON_NEW_EMPTY_OBJECT();
	header_json=JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	c->loc.flags |= FLAG_CLOSED;
	EXIT_FUNC();
	return 0;
}
int copy_file_inotify(const char *source, const char *dest,copy_info_t *pInfo)
{
    struct stat source_stat;
    struct stat dest_stat;
    char dest_exist = 0;
    int retval = 1;
	int enRet = 0;
	int bIsRegularFile = 0;
	if(*pInfo->flags & FLAG_CLOSED) 
	{
		DMCLOG_E("quit the del task");
		return 0;
	}
    if(stat(source, &source_stat) < 0)
    {
        return 0;
    }
    if(stat(dest, &dest_stat) < 0)
    {
        if(errno != ENOENT)
        {
            return 0;
        }
    }
    else
    {
        if (source_stat.st_dev == dest_stat.st_dev
            && source_stat.st_ino == dest_stat.st_ino)
        {
            printf("%s and %s are the same file\n", source, dest);
            return 0;
        }
        dest_exist = 1;
    }
    
    //copy dir
    if(S_ISDIR(source_stat.st_mode))
    {
        DIR *dp;
        struct dirent *d;
        mode_t saved_umask = 0;
        printf("dest_exist = %d\n",dest_exist);
        if(dest_exist)
        {
            if(!S_ISDIR(dest_stat.st_mode))
            {
                printf("target %s is not a directory\n", dest);
            }
        }
        else //create dest dir
        {
            mode_t mode;
            saved_umask = umask(0);
            
            mode = source_stat.st_mode;
            mode |= S_IRWXU;
            if (mkdir(dest, mode) < 0)
            {
                umask(saved_umask);
                return 0;
            }
			bIsRegularFile = 0;
			enRet = handle_file_insert_cmd(pInfo->file_uuid,dest,bIsRegularFile,pInfo->disk_uuid);
			if(enRet < 0)
			{
		        DMCLOG_D("make_dirs(%s) failed, enRet(0x%x)",dest, enRet);
				rm(dest);
				return 0;
		    }
            umask(saved_umask);
			pInfo->cur_nfiles++;
        }
        
        dp = opendir(source);
        if(dp == NULL)
        {
            return 0;
        }
        
        while((d = readdir(dp)) != NULL)
        {
            char *new_source, *new_dest;
            if(strcmp(d->d_name, ".") == 0
               || strcmp(d->d_name, "..") == 0)
            {
                continue;
            }
            
            new_source = concat_path_file(source, d->d_name);
            if(new_source == NULL)
            {
                continue;
            }
            new_dest = concat_path_file(dest, d->d_name);
            if(new_dest == NULL)
            {
                continue;
            }
//            copy_file_inotify(new_source, new_dest,pInfo);
            if(copy_file_inotify(new_source, new_dest,pInfo) == 0)
            {
                printf("copy %s to %s failed\n", new_source, new_dest);
                free(new_source);
                free(new_dest);
                closedir(dp);
                return 0;
            }
            free(new_source);
            free(new_dest);
        }
        closedir(dp);
    }
    
    //copy regular file
    if(S_ISREG(source_stat.st_mode))
    {
        int src_fd;
        int dst_fd;
        mode_t new_mode;
        
        src_fd = open(source, O_RDONLY, 0666);
        if(src_fd < 0)
        {
            printf("cannot open source file:%s\n", source);
            return 0;
        }
        new_mode = source_stat.st_mode;
        if (!S_ISREG(source_stat.st_mode))
        {
            new_mode = 0666;
        }
        dst_fd = open(dest, O_WRONLY|O_CREAT, new_mode);
        if(dst_fd < 0)
        {
            printf("cannot open dest file:%s,errno = %d\n", dest,errno);
            return 0;
        }
        
        if(bb_copyfd_eof(src_fd, dst_fd) == -1)
        {
            retval = 0;
        }
        if(close(dst_fd) < 0)
        {
            retval = 0;
        }
        close(src_fd);
        if (!S_ISREG(source_stat.st_mode))
            return retval;
		bIsRegularFile = 1;
		enRet = handle_file_insert_cmd(pInfo->file_uuid,dest,bIsRegularFile,pInfo->disk_uuid);
		if(enRet < 0)
		{
	        DMCLOG_D("insert file(%s) failed, enRet(0x%x)", dest, enRet);
			rm(dest);
			return 0;
	    }
		pInfo->status = 1;
	    pInfo->cur_name = (char *)malloc(strlen(bb_basename(dest)) + 1);
	    strcpy(pInfo->cur_name,bb_basename(dest));
	    pInfo->cur_nfiles++;
	    pInfo->cur_progress += (unsigned long)source_stat.st_size;
	    enRet = copy_handle_inotify(pInfo);
		if(enRet < 0)
		{
			retval = 0;
		}
	    safe_free(pInfo->cur_name);
    }
    return retval;
    
}

static void copy_inotify_func(void *self)
{
    ENTER_FUNC();
    copy_info_t *pInfo = (copy_info_t *)self;
    char *source_file = pInfo->src_path;
    char *dest_file = pInfo->des_path;
    struct stat source_stat;
    struct stat dest_stat;
    const char *last;
    const char *dest;
    int s_flag;
    int d_flag;
    int malloc = 1;
    int res = 0;
	int error = 0;
	char buf[1024];
    memset(buf,0,1024);
    DMCLOG_D("source_file = %s,dest_file = %s",source_file,dest_file);
    res = get_info_from_hd_list(pInfo->session,pInfo->ip,&(pInfo->port));
    if(res < 0)
    {
    	EXIT_FUNC();
		error = ERROR_FILE_COPY;
		pInfo->status = -1;
    	goto EXIT;
    }
    //1:计算需要复制的文件或者文件夹信息,并通知反向推送服务进行信息发送
    pInfo->status = 0;//复制操作计算中
    copy_handle_inotify(pInfo);
    res = copy_compute(source_file,&pInfo->total_size,&pInfo->nfiles);
    if(res < 0)
    {
    	error = ERROR_FILE_COPY;
		pInfo->status = -1;
        EXIT_FUNC();
		goto EXIT;
    }
    DMCLOG_D("total_size = %lu,nfiles = %u",pInfo->total_size,pInfo->nfiles);
    
    //2:复制每一个文件到指定的文件夹路径，并通过反向推送服务反馈实时传输的文件信息
    
    last = dest_file;
    s_flag = cp_mv_stat(source_file, &source_stat);
    if(s_flag <= 0)
    {
       	error = ERROR_FILE_COPY;
		pInfo->status = -1;
        goto EXIT ;
    }
    d_flag = cp_mv_stat(dest_file, &dest_stat);
    if(d_flag < 0)
    {
    	error = ERROR_FILE_COPY;
		pInfo->status = -1;
    	EXIT_FUNC();
    	goto EXIT;
    }
    
    if( !((s_flag | d_flag)&2)
       || ((s_flag & 2) && !d_flag))
    {
        dest = dest_file;
        malloc = 0;
        goto do_copy;
    }
//    copy_handle_inotify(copy_info_t *pInfo);
    //strcpy(source,source_file);
    dest = concat_path_file(last, bb_get_last_path_component_strip(source_file));
do_copy:
    if(copy_file_inotify((const char*)source_file, dest,pInfo) == 0)
    {
        if(malloc)
        {
            free((void*)dest);
        }
		pInfo->status = -1;
		error = ERROR_FILE_COPY;
        EXIT_FUNC();
		goto EXIT;
    }
    
    if(malloc)
    {
        free((void*)dest);
    }
    pInfo->status = 2;
EXIT:
	copy_handle_inotify(pInfo);
	dm_file_copy_inotify(pInfo->cmd,error,buf);    
    DM_MsgSend(pInfo->sock,buf,strlen(buf));
    *(pInfo->flags) |= FLAG_CLOSED;
    *(pInfo->flags) &= ~(FLAG_R | FLAG_W | FLAG_ALWAYS_READY);
	
    safe_free(source_file);
    safe_free(dest_file);
    safe_free(self);
    EXIT_FUNC();
    return ;
}
int create_copy_task(struct conn *c,char *uuid)
{
    ENTER_FUNC();
    PTHREAD_T tid_copy_task;
    int rslt = 0;
    copy_info_t *pInfo = (copy_info_t *)calloc(1,sizeof(copy_info_t));
    pInfo->flags = &c->rem.flags;
    pInfo->sock = c->rem.chan.sock;
    pInfo->cmd = c->cmd;
    strcpy(pInfo->session,c->session);
    pInfo->seq = c->copy_seq;
	if(uuid != NULL)
		strcpy(pInfo->disk_uuid,uuid);
    pInfo->src_path = (char *)calloc(1,strlen(c->src_path) + 1);
    strcpy(pInfo->src_path,c->src_path);
    pInfo->des_path = (char *)calloc(1,strlen(c->des_path) + 1);
    strcpy(pInfo->des_path,c->des_path);
    if (0 != (rslt = PTHREAD_CREATE(&tid_copy_task, NULL, (void *)copy_inotify_func,pInfo)))
    {
        DMCLOG_D("Create db server msg prcs thread failed!");
        rslt = -1;
        return rslt;
    }
    PTHREAD_DETACH(tid_copy_task);
    EXIT_FUNC();
    return rslt;
}


int DM_FileCopy(struct conn *c)
{
    ENTER_FUNC();
    int res = 0;
//    JObj* response_json = JSON_NEW_EMPTY_OBJECT();
//    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    if(c->error != 0)
    {
        goto EXIT;
    }
	char uuid[32];
	memset(uuid,0,32);
	res = read_mark_file(c->disk_root,uuid);
	if(res < 0)
	{
        c->error = ERROR_GET_DISK_INFO;  
		goto EXIT;
    }
	if(!*uuid )
	{
		DMCLOG_D("uuid is null");
		c->error = DM_ERROR_DB_COPY; 
		goto EXIT;
	}
    if((res = is_file_exist(c->src_path)) != 0)
    {
        c->error = FILE_IS_NOT_EXIST;
        goto EXIT;
    }
    c->loc.flags |= FLAG_W;
    res = create_copy_task(c,uuid);
    if(res < 0)
    {
        DMCLOG_D("copy(%s) failed, des path(%s)", c->src_path, c->des_path);
        c->error = ERROR_FILE_COPY;
        goto EXIT;
    }
    
EXIT:
    EXIT_FUNC();
    return 0;
}
int DM_FileMove(struct conn *c)
{
    ENTER_FUNC();
    int res = 0;
//    JObj* response_json = JSON_NEW_EMPTY_OBJECT();
//    JObj* header_json=JSON_NEW_EMPTY_OBJECT();
    if(c->error != 0)
    {
        goto EXIT;
    }
    if((res = is_file_exist(c->src_path)) != 0)
    {
        c->error = FILE_IS_NOT_EXIST;
        goto EXIT;
    }
#ifdef DB_TASK
    char uuid[32];
    memset(uuid,0,32);
    res = read_mark_file(c->disk_root,uuid);
	if(res < 0)
	{
        c->error = ERROR_GET_DISK_INFO;  
		goto EXIT;
    }
    if(!*uuid )
    {
        DMCLOG_D("uuid is null");
        c->error = DM_ERROR_DB_DELETE;
        goto EXIT;
    }
    if((res = db_backup_file_rm(c->src_path,uuid)) != RET_SUCCESS)
    {
        DMCLOG_D("delete_(%s) failed from backup db, res(%d)", c->src_path, res);
        c->error = ERROR_FILE_DELETE;
        goto EXIT;
    }
    
    res = handle_file_delete_cmd(c->src_path,uuid);
    if(res < 0)
    {
        DMCLOG_D("db rm(%s) failed, ret(0x%x)", c->src_path, res);
        c->error = DM_ERROR_DB_DELETE;
        goto EXIT;
    }
#endif
    if((res = rm(c->src_path)) != RET_SUCCESS)
    {
        DMCLOG_D("delete_(%s) failed, res(%d)", c->src_path, res);
        c->error = ERROR_FILE_DELETE;
        goto EXIT;
    }
    
EXIT:
//    JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
//    JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
//    JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
//    JSON_ADD_OBJECT(response_json, "header", header_json);
//    res = file_json_to_string(c,response_json);
//    if(res < 0)
//    {
//        JSON_PUT_OBJECT(response_json);
//        return -1;
//    }
//    JSON_PUT_OBJECT(response_json);
    EXIT_FUNC();
    return 0;
}

int _DM_FileGetListByType(struct conn *c)
{
	ENTER_FUNC();
	if(c->error != 0)
		goto EXIT;
	get_type(c);
	return 0;
EXIT:
	c->loc.flags |= FLAG_CLOSED;
	EXIT_FUNC();
	return 0;
}

/*int DM_FileGetListByType(struct conn *c)
{
	int res = 0;
	ENTER_FUNC();
	if(c->pageNum < 0)
	{
		return _DM_FileGetListByType(c);
	}
	JObj *response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_para_json=JSON_NEW_EMPTY_OBJECT();
	
	int i = 0;
	int count = 0;
	char uuid[32];
	if(c->error != 0)
		goto EXIT;
	struct file_list file_list_t;
	memset(&file_list_t,0,sizeof(struct file_list));
	all_disk_t mAll_disk_t;
	int sort_mode = 3;
	memset(&mAll_disk_t,0,sizeof(all_disk_t));
	res = dm_get_storage(&mAll_disk_t);
	if(res != 0)
	{
		c->error = ERROR_GET_DISK_INFO;
		goto EXIT;
	}
 	DMCLOG_D("drive_count = %d",mAll_disk_t.count);
	file_list_t.file_type = c->fileType;
	
	for(i = 0;i < mAll_disk_t.count;i++)
	{
		memset(uuid,0,32);
		res = read_mark_file(mAll_disk_t.disk[i].path,uuid);
		if(res < 0)
		{
			DMCLOG_D("get disk uuid error");
			goto EXIT;
		}
		res = dm_getTypeInfo(c->fileType,&file_list_t,uuid);
		if(res != 0)
		{
			DMCLOG_D("get file list count from db error");
			goto EXIT;
		}
		if(c->pageNum >= file_list_t.totalPage )
		{
			DMCLOG_D("c->pageNum = %d",c->pageNum);
			c->error = ERROR_GET_FILE_LIST;
			goto EXIT;
		}
		c->offset = c->pageNum*PAGESIZE;
		if(c->pageNum == file_list_t.totalPage - 1)
		{
			c->length = file_list_t.totalCount;
		}
		else
		{
			c->length = (c->pageNum + 1)*PAGESIZE;
		}
		
		DMCLOG_D("c->offset = %lu,c->length = %lu",c->offset,c->length);
		count = c->length - c->offset;
		res = handle_get_file_list_cmd(c->fileType,c->offset,PAGESIZE,sort_mode,response_para_json,uuid);
		if(res != 0)
		{
			c->error = ERROR_GET_FILE_LIST;
			goto EXIT;
		}
		
	}
	JSON_ADD_OBJECT(response_para_json, "count", JSON_NEW_OBJECT(count,int));
	JSON_ADD_OBJECT(response_para_json, "totalCount", JSON_NEW_OBJECT(file_list_t.totalCount,int));
	JSON_ADD_OBJECT(response_para_json, "totalPage", JSON_NEW_OBJECT(file_list_t.totalPage,int));
	JSON_ADD_OBJECT(response_para_json, "pageSize", JSON_NEW_OBJECT(file_list_t.pageSize,int));
EXIT:
	JSON_ADD_OBJECT(response_json, "data", response_para_json);
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
	return 0;
}*/
int DM_FileGetClassInfo(struct conn *c)
{
	ENTER_FUNC();
	JObj *response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_para_json=JSON_NEW_EMPTY_OBJECT();
	int res = 0;
	int i = 0;
	int count = 0;
	if(c->error != 0)
		goto EXIT;
	struct file_list file_list_t;
	memset(&file_list_t,0,sizeof(struct file_list));
	all_disk_t mAll_disk_t;
	int sort_mode = 3;
	uint64_t result_size;
	char uuid[32];
	memset(&mAll_disk_t,0,sizeof(all_disk_t));
	res = dm_get_storage(&mAll_disk_t);
	if(res != 0)
	{
		c->error = ERROR_GET_DISK_INFO;
		goto EXIT;
	}
 	DMCLOG_D("drive_count = %d",mAll_disk_t.count);
	file_list_t.file_type = c->fileType;
	
	for(i = 0;i < mAll_disk_t.count;i++)
	{
		/*firstly,get the disk uuid according to .dmdiskuuid;
	    secondly,query the file_table_name from file_table;
	    thirdly,query the count by type from the file_table_name*/
	    memset(uuid,0,32);
		res = read_mark_file(mAll_disk_t.disk[i].path,uuid);
		if(res < 0)
		{
			DMCLOG_D("get disk uuid error");
			goto EXIT;
		}
		/*query the file list by type*/
		res = dm_getTypeInfo(c->cmd,c->fileType,&file_list_t,uuid);
		if(res != 0)
		{
			DMCLOG_D("get file list count from db error");
			goto EXIT;
		}
		c->offset = 0;
		c->length = file_list_t.totalCount;
		
		DMCLOG_D("c->offset = %lu,c->length = %lu",c->offset,c->length);
		res = handle_get_file_size_cmd(c->fileType,c->offset,c->length,sort_mode,response_para_json,uuid);
		if(res != 0)
		{
			c->error = ERROR_GET_FILE_LIST;
			goto EXIT;
		}
	}
EXIT:
	JSON_ADD_OBJECT(response_json, "data", response_para_json);
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
	return 0;
}
#if 0
int DM_FileGetPathInfoByType(struct conn *c)
{
	ENTER_FUNC();
	JObj *response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_para_json=JSON_NEW_EMPTY_OBJECT();
	int res = 0;
	int i = 0;
	int count = 0;
	char uuid[32];
	if(c->error != 0)
		goto EXIT;
	struct file_list file_list_t;
	memset(&file_list_t,0,sizeof(struct file_list));
	all_disk_t mAll_disk_t;
	int sort_mode = 3;
	memset(&mAll_disk_t,0,sizeof(all_disk_t));
	res = dm_get_storage(&mAll_disk_t);
	if(res != 0)
	{
		c->error = ERROR_GET_DISK_INFO;
		goto EXIT;
	}
 	DMCLOG_D("drive_count = %d",mAll_disk_t.count);
	file_list_t.file_type = c->fileType;
	
	for(i = 0;i < mAll_disk_t.count;i++)
	{
		/*firstly,get the disk uuid according to .dmdiskuuid;
	    secondly,query the file_table_name from file_table;
	    thirdly,query the count by type from the file_table_name*/
		memset(uuid,0,32);
		res = read_mark_file(mAll_disk_t.disk[i].path,uuid);
		if(res < 0)
		{
			DMCLOG_D("get disk uuid error");
			goto EXIT;
		}
		res = dm_getTypeDirInfo(c->fileType,&file_list_t,uuid);
		if(res != 0)
		{
			DMCLOG_D("get file list count from db error");
			goto EXIT;
		}
		if(c->pageNum >= file_list_t.totalPage )
		{
			DMCLOG_D("c->pageNum = %d",c->pageNum);
			c->error = ERROR_GET_FILE_LIST;
			goto EXIT;
		}
		c->offset = c->pageNum*PAGESIZE;
		if(c->pageNum == file_list_t.totalPage - 1)
		{
			c->length = file_list_t.totalCount;
		}
		else
		{
			c->length = (c->pageNum + 1)*PAGESIZE;
		}
		
		DMCLOG_D("c->offset = %lu,c->length = %lu",c->offset,c->length);
		count = c->length - c->offset;
		/*query the file list by type*/
		res = handle_get_dir_list_cmd(c->fileType,c->offset,PAGESIZE,sort_mode,response_para_json,uuid);
		if(res != 0)
		{
			c->error = ERROR_GET_FILE_LIST;
			goto EXIT;
		}
		
	}
	JSON_ADD_OBJECT(response_para_json, "count", JSON_NEW_OBJECT(count,int));
	JSON_ADD_OBJECT(response_para_json, "totalCount", JSON_NEW_OBJECT(file_list_t.totalCount,int));
	JSON_ADD_OBJECT(response_para_json, "totalPage", JSON_NEW_OBJECT(file_list_t.totalPage,int));
	JSON_ADD_OBJECT(response_para_json, "pageSize", JSON_NEW_OBJECT(8,int));
EXIT:
	JSON_ADD_OBJECT(response_json, "data", response_para_json);
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
	return 0;
}
int DM_FileGetListByPath(struct conn *c)
{
	ENTER_FUNC();
	JObj *response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_para_json=JSON_NEW_EMPTY_OBJECT();
	int res = 0;
	int i = 0;
	int count = 0;
	char uuid[32];
	if(c->error != 0)
		goto EXIT;
	struct file_list file_list_t;
	memset(&file_list_t,0,sizeof(struct file_list));
	all_disk_t mAll_disk_t;
	int sort_mode = 3;
	memset(&mAll_disk_t,0,sizeof(all_disk_t));
	res = dm_get_storage(&mAll_disk_t);
	if(res != 0)
	{
		c->error = ERROR_GET_DISK_INFO;
		goto EXIT;
	}
 	DMCLOG_D("drive_count = %d",mAll_disk_t.count);
	file_list_t.file_type = c->fileType;
	
	for(i = 0;i < mAll_disk_t.count;i++)
	{
		/*firstly,get the disk uuid according to .dmdiskuuid;
	    secondly,query the file_table_name from file_table;
	    thirdly,query the count by type from the file_table_name*/
		memset(uuid,0,32);
		res = read_mark_file(mAll_disk_t.disk[i].path,uuid);
		if(res < 0)
		{
			DMCLOG_D("get disk uuid error");
			goto EXIT;
		}
		res = dm_getTypeInfoByPath(c->fileType,c->src_path + strlen(DOCUMENT_ROOT), &file_list_t,uuid);
		if(res != 0)
		{
			DMCLOG_D("get file list count from db error");
			goto EXIT;
		}
		if(c->pageNum >= file_list_t.totalPage )
		{
			DMCLOG_D("c->pageNum = %d",c->pageNum);
			c->error = ERROR_GET_FILE_LIST;
			goto EXIT;
		}
		c->offset = c->pageNum*PAGESIZE;
		if(c->pageNum == file_list_t.totalPage - 1)
		{
			c->length = file_list_t.totalCount;
		}
		else
		{
			c->length = (c->pageNum + 1)*PAGESIZE;
		}
		
		DMCLOG_D("c->offset = %lu,c->length = %lu",c->offset,c->length);
		count = c->length - c->offset;
		/*query the file list by type*/
		res = handle_get_file_list_by_path_cmd(c->fileType,c->offset,PAGESIZE,sort_mode,c->src_path + strlen(DOCUMENT_ROOT),response_para_json,uuid);
		if(res != 0)
		{
			c->error = ERROR_GET_FILE_LIST;
			goto EXIT;
		}
		
	}
	JSON_ADD_OBJECT(response_para_json, "count", JSON_NEW_OBJECT(count,int));
	JSON_ADD_OBJECT(response_para_json, "totalCount", JSON_NEW_OBJECT(file_list_t.totalCount,int));
	JSON_ADD_OBJECT(response_para_json, "totalSize", JSON_NEW_OBJECT(file_list_t.totalSize,int64));
	JSON_ADD_OBJECT(response_para_json, "totalPage", JSON_NEW_OBJECT(file_list_t.totalPage,int));
	JSON_ADD_OBJECT(response_para_json, "pageSize", JSON_NEW_OBJECT(file_list_t.pageSize,int));
EXIT:
	JSON_ADD_OBJECT(response_json, "data", response_para_json);
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
	return 0;
}
#endif

int DM_FileDelListByPath(struct conn *c)
{
	ENTER_FUNC();
	JObj *response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_para_json=JSON_NEW_EMPTY_OBJECT();
	int res = 0;
	int i = 0;
	char uuid[32];
	if(c->error != 0)
		goto EXIT;
	struct file_list file_list_t;
	memset(&file_list_t,0,sizeof(struct file_list));
	all_disk_t mAll_disk_t;
	int sort_mode = 3;
	memset(&mAll_disk_t,0,sizeof(all_disk_t));
	res = dm_get_storage(&mAll_disk_t);
	if(res != 0)
	{
		c->error = ERROR_GET_DISK_INFO;
		goto EXIT;
	}
 	DMCLOG_D("drive_count = %d",mAll_disk_t.count);
	file_list_t.file_type = c->fileType;
	
	for(i = 0;i < mAll_disk_t.count;i++)
	{
		/*firstly,get the disk uuid according to .dmdiskuuid;
	    secondly,query the file_table_name from file_table;
	    thirdly,query the count by type from the file_table_name*/
		memset(uuid,0,32);
		res = read_mark_file(mAll_disk_t.disk[i].path,uuid);
		if(res < 0)
		{
			DMCLOG_D("get disk uuid error");
			goto EXIT;
		}
		res = dm_getTypeInfoByPath(c->fileType,c->src_path + + strlen(DOCUMENT_ROOT), &file_list_t,uuid);
		if(res != 0)
		{
			DMCLOG_D("get file list count from db error");
			goto EXIT;
		}
		/*query the file list by type*/
		c->offset = 0;
		c->length = file_list_t.totalCount;
		DMCLOG_D("c->offset = %lu,c->length = %lu",c->offset,c->length);
		res = handle_del_file_list_by_path_cmd(c->fileType,c->offset,c->length,sort_mode,c->src_path + strlen(DOCUMENT_ROOT),uuid);
		if(res != 0)
		{
			c->error = ERROR_GET_FILE_LIST;
			goto EXIT;
		}
		
	}
EXIT:
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
	return 0;
}


int DM_FileGetScanStatus(struct conn *c)
{
	ENTER_FUNC();
	JObj *response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int res = 0;
	int status;
	if(c->error != 0)
		goto EXIT;
	JObj *response_para_json=JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(response_para_json, "status", JSON_NEW_OBJECT(status,int));
	JSON_ADD_OBJECT(response_json, "data", response_para_json);
EXIT:
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
	return 0;
}
extern struct disk_node *disk_dn;
int DM_FileGetBackupInfo(struct conn *c)
{
	ENTER_FUNC();
	JObj *response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	all_disk_t mAll_disk_t;
	int res = 0,i = 0;
	int status;
	if(c->error != 0)
		goto EXIT;
	char disk_uuid[32];
	memset(disk_uuid,0,32);
	char device_uuid[64];
	char device_name[64];
	memset(device_uuid,0,64);
	memset(device_name,0,64);
	res = get_devuuid_from_usr_table(c->session,device_uuid,device_name);
	if(res < 0)
	{
        DMCLOG_E("db get_deviceuuid_from_usr_table(%s) failed, ret(0x%x)", device_uuid, res);
        c->error = DM_ERROR_DB_USER_TABLE;   
		goto EXIT;
    }
	DMCLOG_D("device_uuid = %s,device_name = %s",device_uuid,device_name);
	JObj *response_para_json=JSON_NEW_EMPTY_OBJECT();
	struct disk_node *disk_info = NULL;
	res = get_bind_disk_uuid_from_db(device_uuid,disk_uuid);
	if(res == 0)
	{
		
		disk_info = get_disk_node(disk_uuid);
		if(disk_info != NULL)
		{
			char *disk_name = bb_basename(disk_info->path);
			memset(&mAll_disk_t,0,sizeof(all_disk_t));
			res = dm_get_storage(&mAll_disk_t);
			if(res != 0)
			{
				c->error = ERROR_GET_DISK_INFO;
				goto EXIT;
			}
		 	DMCLOG_D("drive_count = %d",mAll_disk_t.count);
			for(i = 0;i < mAll_disk_t.count;i++)
			{
				if(!strcmp(mAll_disk_t.disk[i].name,disk_name))
				{
					status = 2;/*device have bind with disk and the disk is mounted*/
					JObj *bindInfo_json = JSON_NEW_EMPTY_OBJECT();
					JSON_ADD_OBJECT(bindInfo_json, "disk_name", JSON_NEW_OBJECT(disk_name,string));
					JSON_ADD_OBJECT(bindInfo_json, "diskSize", JSON_NEW_OBJECT(mAll_disk_t.disk[i].total_size,int64));
					JSON_ADD_OBJECT(bindInfo_json, "diskAvSize", JSON_NEW_OBJECT(mAll_disk_t.disk[i].free_size,int64));
					JSON_ADD_OBJECT(bindInfo_json, "deviceSize", JSON_NEW_OBJECT(0,int64));
					JSON_ADD_OBJECT(bindInfo_json, "entireDeviceSize", JSON_NEW_OBJECT(0,int64));
					JSON_ADD_OBJECT(response_para_json, "BindDiskInfo", bindInfo_json);
					break;
				}
			}
		}else{
			status = 1;/*device have bind with disk ,but the disk is unmounted*/
		}	
	}else{
		status = 0;/*device have not bind with disk*/
		res = dm_get_storage(&mAll_disk_t);
		if(res != 0)
		{
			c->error = ERROR_GET_DISK_INFO;
			goto EXIT;
		}
	 	DMCLOG_D("drive_count = %d",mAll_disk_t.count);
		for(i = 0;i < mAll_disk_t.count;i++)
		{
			if(i == 0)
			{
				memset(disk_uuid,0,32);
				DMCLOG_D("path = %s",mAll_disk_t.disk[i].path);
				res = read_mark_file(mAll_disk_t.disk[i].path,disk_uuid);
				if(res < 0)
				{
			        c->error = ERROR_GET_DISK_INFO;  
					goto EXIT;
			    }
				DMCLOG_D("device_uuid = %s,device_name = %s,disk_uuid = %s",device_uuid,device_name,disk_uuid);
				res = bind_devuuid_to_diskuuid(device_uuid,device_name,disk_uuid);
				if(res < 0)
				{
			        DMCLOG_D("db bind_deviceUuid_to_diskUuid(%s) failed, ret(0x%x)", disk_uuid, res);
			        c->error = DM_ERROR_DB_DEV_TABLE;   
					goto EXIT;
			    }
				status = 2;/*device have bind with disk and the disk is mounted*/
				DMCLOG_D("disk_uuid = %s",disk_uuid);
			}
		}
	}
	
	JSON_ADD_OBJECT(response_para_json, "BindStatus", JSON_NEW_OBJECT(status,int));
	JSON_ADD_OBJECT(response_json, "data", response_para_json);
EXIT:
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
	return 0;
}
int DM_FileBindBackupDisk(struct conn *c)
{
	ENTER_FUNC();
	int res = 0;
	JObj *response_json=JSON_NEW_EMPTY_OBJECT();
	JObj *header_json=JSON_NEW_EMPTY_OBJECT();
	if(c->error != 0)
		goto EXIT;
#ifdef DB_TASK
	char disk_uuid[32];
	memset(disk_uuid,0,32);
	res = read_mark_file(c->disk_root,disk_uuid);
	if(res < 0)
	{
        c->error = ERROR_GET_DISK_INFO;  
		goto EXIT;
    }
	char device_uuid[64];
	char device_name[64];
	memset(device_uuid,0,64);
	memset(device_name,0,64);
	res = get_devuuid_from_usr_table(c->session,device_uuid,device_name);
	if(res < 0)
	{
        DMCLOG_D("db get_deviceuuid_from_usr_table(%s) failed, ret(0x%x)", device_uuid, res);
        c->error = DM_ERROR_DB_USER_TABLE;   
		goto EXIT;
    }
	DMCLOG_D("device_uuid = %s,device_name = %s,disk_uuid = %s",device_uuid,device_name,disk_uuid);
	res = bind_devuuid_to_diskuuid(device_uuid,device_name,disk_uuid);
	if(res < 0)
	{
        DMCLOG_D("db bind_deviceUuid_to_diskUuid(%s) failed, ret(0x%x)", disk_uuid, res);
        c->error = DM_ERROR_DB_DEV_TABLE;   
		goto EXIT;
    }
	DMCLOG_D("disk_uuid = %s",disk_uuid);
#endif
EXIT:
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
	return 0;
}

int DM_FileGetBackupFile(struct conn *c)
{
	ENTER_FUNC();
	int res = 0;
	int status;
    JObj *response_json;
    JObj* header_json;
	struct stat	st;
	if(c->error != 0)
    {
        response_json=JSON_NEW_EMPTY_OBJECT();
        header_json=JSON_NEW_EMPTY_OBJECT();
        DMCLOG_D("c->error = %d",c->error);
        goto EXIT;
    }
	if (c->rem.content_len == 0) {
		c->error = LENGTH_REQUIRED;
        response_json=JSON_NEW_EMPTY_OBJECT();
        header_json=JSON_NEW_EMPTY_OBJECT();
        DMCLOG_D("c->error = %d",c->error);
		goto EXIT;
	} else if ((c->loc.chan.fd = my_open(c->tmp_path, O_WRONLY | O_CREAT, 0644)) == -1) {
		c->error = CREATE_FILE_ERROR;
        response_json=JSON_NEW_EMPTY_OBJECT();
        header_json=JSON_NEW_EMPTY_OBJECT();
        DMCLOG_D("c->error = %d",c->error);
		goto EXIT;
	} else {
		DMCLOG_D("PUT file [%s]", c->des_path);
		c->loc.io_class = &io_file;
		//c->loc.flags |= FLAG_W | FLAG_ALWAYS_READY ;
		c->loc.flags |= FLAG_W;
		DMCLOG_D("upload c->offset = %lu",c->offset);
		(void) lseek(c->loc.chan.fd, c->offset, SEEK_SET);
		if(my_stat(c->cfg_path, &st) != 0)
		{
			//如果记录文件块起始地址的配置文件不存在，则将信息写入
			int i = 0;
			int percent = c->fileSize/THREAD_COUNT;
			c->dn = NULL;
			for(; i < THREAD_COUNT;i++)
		    {
		        if(i == THREAD_COUNT - 1)
					add_record_to_list(&c->dn,i,i * percent,c->fileSize);
		        else
					add_record_to_list(&c->dn,i,i * percent,(i + 1)* percent);
		    }
			display_record_dnode(c->dn);
			write_list_to_file(c->cfg_path,c->dn);
			destory_record_list(c->dn);
			gettimeofday(&c->tstart, NULL);
		}
		goto FINISH;
	}
EXIT:
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
FINISH:
	EXIT_FUNC();
	return 0;
}
int DM_FileCheckBackupFileList(struct conn *c)
{
	ENTER_FUNC();
	JObj *response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_para_json = JSON_NEW_EMPTY_OBJECT();
	int res = 0;
	int status;
	if(c->error != 0)
		goto EXIT;
	char device_uuid[64];
	char device_name[64];
	memset(device_uuid,0,64);
	memset(device_name,0,64);
	EnterCriticalSection(&c->ctx->mutex);
	res = get_devuuid_from_usr_table(c->session,device_uuid,device_name);
	if(res < 0)
	{
        DMCLOG_D("db get_deviceuuid_from_usr_table(%s) failed, ret(0x%x)", device_uuid, res);
        c->error = DM_ERROR_DB_USER_TABLE;   
		goto EXIT;
    }
	char disk_uuid[32];
	memset(disk_uuid,0,32);
	res = get_bind_disk_uuid_from_db(device_uuid,disk_uuid);
	if(res < 0)
	{
        DMCLOG_D("db get_bind_disk_uuid_from_db(%s) failed, ret(0x%x)", disk_uuid, res);
        c->error = DM_ERROR_DB_HDISK_TABLE;   
		goto EXIT;
    }
	
	DMCLOG_D("disk_uuid = %s",disk_uuid);
	strcpy(c->flist->device_uuid,device_uuid);
	DMCLOG_D("c->flist->device_uuid = %s",c->flist->device_uuid);
	res = handle_file_list_uuid_exist_cmd(c->flist,response_para_json,disk_uuid);
	if(res != 0)
	{
		DMCLOG_D("db handle_file_list_uuid_exist_cmd(%s) failed, ret(0x%x)", disk_uuid, res);
		goto EXIT;
	}
EXIT:
	LeaveCriticalSection(&c->ctx->mutex);
	JSON_ADD_OBJECT(response_json, "data", response_para_json);
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	
	EXIT_FUNC();
	return 0;
}
int DM_FileCheckBackupFile(struct conn *c)
{
	ENTER_FUNC();
	JObj *response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj* data_json = JSON_NEW_EMPTY_OBJECT();
	int res = 0,status;
	int i;
	struct stat	st;
	if(c->error != 0)
		goto EXIT;
	char device_uuid[64];
	char device_name[64];
	memset(device_uuid,0,64);
	memset(device_name,0,64);
	res = get_devuuid_from_usr_table(c->session,device_uuid,device_name);
	if(res < 0)
	{
        DMCLOG_D("db get_deviceuuid_from_usr_table(%s) failed, ret(0x%x)", device_uuid, res);
        c->error = DM_ERROR_DB_USER_TABLE;   
		goto EXIT;
    }
	char disk_uuid[32];
	memset(disk_uuid,0,32);
	res = get_bind_disk_uuid_from_db(device_uuid,disk_uuid);
	if(res < 0)
	{
        DMCLOG_D("db get_bind_disk_uuid_from_db(%s) failed, ret(0x%x)", disk_uuid, res);
        c->error = DM_ERROR_DB_HDISK_TABLE;   
		goto EXIT;
    }
	DMCLOG_D("disk_uuid = %s",disk_uuid);
	char *file_path;
	res = handle_file_uuid_exist_cmd(c->file_uuid,device_uuid,disk_uuid,&file_path);
	if(res < 0)
	{
		DMCLOG_D("db handle_file_uuid_exist_cmd(%s) failed, ret(0x%x)", disk_uuid, res);
        c->status = 0;//文件不存在
		goto EXIT;
	}
	c->cfg_path = (char *)malloc(file_path + strlen(CFG_PATH_NAME) + 1);
	if(c->cfg_path == NULL)
	{
		c->error = SERVER_OUT_MEMORY;;
		res = -1;
		goto EXIT;
	}
	sprintf(c->cfg_path,"%s%s",file_path,CFG_PATH_NAME);
	DMCLOG_D("cfg_path = %s",c->cfg_path);
	if(my_stat(c->cfg_path, &st) == 0)
	{
		DMCLOG_D("cfg path is exist");
		c->status = 1;//临时文件存在
		c->dn = NULL;
		res = read_list_from_file(c->cfg_path,&c->dn);
		if(res < 0)
			c->status = 0;
			goto EXIT;
	}else{
		DMCLOG_D("cfg_path is not exist");
		c->status = 0;
	}
	DMCLOG_D("access1 c->status = %d",c->status);
	if(c->status == 1)//存在未完成的tmp文件。
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
		 DMCLOG_D("i = %d,dn->index = %s",i,p_dn->index);
		 JSON_ARRAY_ADD_OBJECT (response_data_array,upload_record_json);
		 p_dn = p_dn->dn_next;
		}
		JSON_ADD_OBJECT(data_json, "count", JSON_NEW_OBJECT(i,int));
		JSON_ADD_OBJECT(data_json, "incompleteInfo", response_data_array);
	}
	destory_record_list(c->dn);
EXIT:
	JSON_ADD_OBJECT(data_json, "status", JSON_NEW_OBJECT(c->status,int));
	JSON_ADD_OBJECT(response_json, "data", data_json);
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
	return 0;
}

int DM_FileIsBackupExisted(struct conn *c)
{
	ENTER_FUNC();
	JObj *response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	JObj *response_para_json = NULL;
	int res = 0;
	int status;
	if(c->error != 0)
		goto EXIT;
	char device_uuid[64];
	char device_name[64];
	memset(device_uuid,0,64);
	memset(device_name,0,64);
	EnterCriticalSection(&c->ctx->mutex);
	res = get_devuuid_from_usr_table(c->session,device_uuid,device_name);
	if(res < 0)
	{
        DMCLOG_D("db get_deviceuuid_from_usr_table(%s) failed, ret(0x%x)", device_uuid, res);
        c->error = DM_ERROR_DB_USER_TABLE;   
		goto EXIT;
    }
	char disk_uuid[32];
	memset(disk_uuid,0,32);
	res = get_bind_disk_uuid_from_db(device_uuid,disk_uuid);
	if(res < 0)
	{
        DMCLOG_D("db get_bind_disk_uuid_from_db(%s) failed, ret(0x%x)", disk_uuid, res);
        c->error = DM_ERROR_DB_HDISK_TABLE;   
		goto EXIT;
    }
	
	DMCLOG_D("disk_uuid = %s",disk_uuid);
	char *file_path = NULL;
	res = handle_file_uuid_exist_cmd(c->file_uuid,device_uuid,disk_uuid,&file_path);
	if(res < 0)
	{
		DMCLOG_D("db handle_file_uuid_exist_cmd(%s) failed, ret(0x%x)", disk_uuid, res);
        c->status = 1;   //file is not exist in backup db
		goto STATUS;
	}else
	{
		c->status = 0; //file is exist and file status == 0
	}
STATUS:
	response_para_json=JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(response_para_json, "status", JSON_NEW_OBJECT(c->status,int));
	JSON_ADD_OBJECT(response_json, "data", response_para_json);

EXIT:
	LeaveCriticalSection(&c->ctx->mutex);
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
	return 0;
}
int DM_FileResetBackup(struct conn *c)
{
	ENTER_FUNC();
	JObj *response_json=JSON_NEW_EMPTY_OBJECT();
	JObj* header_json=JSON_NEW_EMPTY_OBJECT();
	int res = 0;
	int status;
	if(c->error != 0)
		goto EXIT;
	JObj *response_para_json=JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(response_para_json, "status", JSON_NEW_OBJECT(status,int));
	JSON_ADD_OBJECT(response_json, "data", response_para_json);
EXIT:
	JSON_ADD_OBJECT(header_json, "cmd", JSON_NEW_OBJECT(c->cmd,int));
	JSON_ADD_OBJECT(header_json, "seq", JSON_NEW_OBJECT(c->seq,int));
	JSON_ADD_OBJECT(header_json, "error", JSON_NEW_OBJECT(c->error,int));
	JSON_ADD_OBJECT(response_json, "header", header_json);
	res = file_json_to_string(c,response_json);
	if(res < 0)
	{
		JSON_PUT_OBJECT(response_json);
		return -1;
	}
	JSON_PUT_OBJECT(response_json);
	EXIT_FUNC();
	return 0;
}


int file_process(struct conn *c)
{ 
	uint8_t i = 0;
	uint8_t switch_flag = 0;
	int ret = 0;
	DMCLOG_D("c->cmd = %d",c->cmd);
	for(i = 0; i<FILE_TAGHANDLE_NUM; i++)
	{
		if(c->cmd == all_file_handle[i].tag)
		{
	       	 ret = all_file_handle[i].tagfun(c);
		     switch_flag = 1;
		}
	}
	if(switch_flag == 0)
    {
        c->error = REQUEST_FORMAT_ERROR;//命令无法识别
		DMCLOG_D("cmd not found");
    }else if(ret < 0)
    {
		c->error = REQUEST_FORMAT_ERROR;//命令的格式错误
	}	
	return ret;
}

int file_parse_header_json(struct conn *c)
{
	ENTER_FUNC();
	int ret = -1;
	c->r_json = JSON_PARSE(c->rem.io.buf);
	if(c->r_json == NULL)
	{
		DMCLOG_D("access NULL");
		c->error = REQUEST_FORMAT_ERROR;
		ret = -1;
		goto EXIT;
	}
	if(is_error(c->r_json))
	{
		DMCLOG_D("### error:post data is not a json string");
		c->error = REQUEST_FORMAT_ERROR;
		ret = -1;
		goto EXIT;
	}
	
	JObj *header_json = JSON_GET_OBJECT(c->r_json,"header");
	if(header_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		ret = -1;
		goto EXIT;
	}
	JObj *cmd_json = JSON_GET_OBJECT(header_json,"cmd");
	if(cmd_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		ret = -1;
		goto EXIT;
	}
	c->cmd = JSON_GET_OBJECT_VALUE(cmd_json,int);
	JObj *session_json = JSON_GET_OBJECT(header_json,"session");
	JObj *seq_json = JSON_GET_OBJECT(header_json,"seq");

	if(seq_json == NULL)
	{
		c->error = REQUEST_FORMAT_ERROR;
		ret = -1;
		goto EXIT;
	}
	c->seq = JSON_GET_OBJECT_VALUE(seq_json,int);
	if(session_json != NULL)
		strcpy(c->session,JSON_GET_OBJECT_VALUE(session_json,string));
	DMCLOG_D("c->cmd = %d",c->cmd);

	EXIT_FUNC();
	return 0;
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return ret;
}

/*
 * 文件下载 cmd = 106
 */
int Parser_FileDownload(struct conn *c)
{
	int res = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
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
	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	JObj *offset_json = JSON_GET_OBJECT(para_json,"offset");
	JObj *length_json = JSON_GET_OBJECT(para_json,"length");
	DMCLOG_D("c->cmd = %d",c->cmd);
	if(path_json == NULL||offset_json == NULL||length_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	char *uri_src = JSON_GET_OBJECT_VALUE(path_json,string);
	if(uri_src == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
		
	}
	DMCLOG_D("uri_src = %s",uri_src);
	if (strlen(uri_src) + strlen(DOCUMENT_ROOT) >= URI_MAX) {
		res = -1;
		c->error = PATH_TOO_LONGTH;
		goto EXIT;
	}
	c->src_path = (char *)malloc(strlen(uri_src) + strlen(DOCUMENT_ROOT) + 2);
	if(c->src_path == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = SERVER_OUT_MEMORY;
		res = -1;
		goto EXIT;
	}
    if(*uri_src == '/')
    {
        sprintf(c->src_path,"%s%s",DOCUMENT_ROOT,uri_src);
    }else{
        sprintf(c->src_path,"%s/%s",DOCUMENT_ROOT,uri_src);
    }
	c->offset = JSON_GET_OBJECT_VALUE(offset_json,int64);
	c->length = JSON_GET_OBJECT_VALUE(length_json,int64);
EXIT:
	if(c->r_json != NULL)
			JSON_PUT_OBJECT(c->r_json);
	return res;	
}

/*
 * 上传检查 cmd = 108
 */
int Parser_FileCheckUpload(struct conn *c)
{
	int res = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	JObj *fileSize_json = JSON_GET_OBJECT(para_json,"fileSize");
	JObj *modifyTime_json = JSON_GET_OBJECT(para_json,"modifyTime");
	
	DMCLOG_D("c->cmd = %d",c->cmd);
	if(path_json == NULL||fileSize_json == NULL||modifyTime_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	char *uri_src = JSON_GET_OBJECT_VALUE(path_json,string);
	if(uri_src == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
		
	}
	DMCLOG_D("uri_src = %s",uri_src);
	if (strlen(uri_src) + strlen(DOCUMENT_ROOT) >= URI_MAX) {
		res = -1;
		c->error = PATH_TOO_LONGTH;
		goto EXIT;
	}
	c->src_path = (char *)malloc(strlen(uri_src) + strlen(DOCUMENT_ROOT) + 2);
	if(c->src_path == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = c->error = SERVER_OUT_MEMORY;;
		res = -1;
		goto EXIT;
	}
    if(*uri_src == '/')
    {
        sprintf(c->src_path,"%s%s",DOCUMENT_ROOT,uri_src);
    }else{
        sprintf(c->src_path,"%s/%s",DOCUMENT_ROOT,uri_src);
    }
	c->fileSize = JSON_GET_OBJECT_VALUE(fileSize_json,int64);
	c->modifyTime = JSON_GET_OBJECT_VALUE(modifyTime_json,int64);
	c->cfg_path = (char *)malloc(strlen(c->src_path)+strlen(CFG_PATH_NAME)+1);
	if(c->cfg_path == NULL)
	{
		c->error = c->error = SERVER_OUT_MEMORY;;
		res = -1;
		goto EXIT;
	}
	sprintf(c->cfg_path,"%s%s",c->src_path,CFG_PATH_NAME);
	DMCLOG_D("cfg_path = %s",c->cfg_path);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return res;	
}
/*
 * 上传命令cmd = 107
 */
int Parser_FileUpload(struct conn *c)
{
	int res = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	JObj *offset_json = JSON_GET_OBJECT(para_json,"offset");
	JObj *length_json = JSON_GET_OBJECT(para_json,"length");
	JObj *fileSize_json = JSON_GET_OBJECT(para_json,"fileSize");
	JObj *modifyTime_json = JSON_GET_OBJECT(para_json,"modifyTime");
	if(path_json == NULL||offset_json == NULL||length_json == NULL||fileSize_json == NULL||modifyTime_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	int res_sz = 0;
	char *uri_src = JSON_GET_OBJECT_VALUE(path_json,string);
	if(uri_src == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
		
	}
	DMCLOG_D("uri_src = %s",uri_src);
	if (strlen(uri_src) + strlen(DOCUMENT_ROOT) >= URI_MAX) {
		res = -1;
		c->error = PATH_TOO_LONGTH;
		goto EXIT;
	}
	char disk_name[64];
	memset(disk_name,0,64);
	char *tmp = strchr(uri_src,'/');
	memcpy(disk_name,uri_src,tmp - uri_src);
	DMCLOG_D("disk_name = %s",disk_name);
	c->disk_root = (char *)malloc(strlen(DOCUMENT_ROOT) + strlen(disk_name) + 2 );
	if(c->disk_root == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = SERVER_OUT_MEMORY;
		res = -1;
		goto EXIT;
	}
	sprintf(c->disk_root,"%s/%s",DOCUMENT_ROOT,disk_name);
	DMCLOG_D("disk_root = %s",c->disk_root);
	char disk_uuid[64];
	memset(disk_uuid,0,64);
	res = read_mark_file(c->disk_root,disk_uuid);
	if(res < 0)
	{
        c->error = ERROR_GET_DISK_INFO;  
		goto EXIT;
    }
	c->disk_uuid = (char *)malloc(strlen(disk_uuid) + 1);
	strcpy(c->disk_uuid,disk_uuid);
	c->des_path = (char *)malloc(strlen(uri_src) + strlen(DOCUMENT_ROOT) + 2);
	if(c->des_path == NULL)
	{
		DMCLOG_D("malloc error");
		res = -1;
		c->error = SERVER_OUT_MEMORY;
		goto EXIT;
	}
    if(*uri_src == '/')
    {
        sprintf(c->des_path,"%s%s",DOCUMENT_ROOT,uri_src);
    }else{
        sprintf(c->des_path,"%s/%s",DOCUMENT_ROOT,uri_src);
    }
	c->fileSize= JSON_GET_OBJECT_VALUE(fileSize_json,int64);
	c->modifyTime = JSON_GET_OBJECT_VALUE(modifyTime_json,int64);
	c->offset = JSON_GET_OBJECT_VALUE(offset_json,int64);
	c->length = JSON_GET_OBJECT_VALUE(length_json,int64);
	c->cfg_path = (char *)malloc(strlen(c->des_path)+strlen(CFG_PATH_NAME)+1);
	if(c->cfg_path == NULL)
	{
		DMCLOG_D("malloc error");
		res = -1;
		c->error = SERVER_OUT_MEMORY;
		goto EXIT;
	}
	sprintf(c->cfg_path,"%s%s",c->des_path,CFG_PATH_NAME);
	DMCLOG_D("cfg_path = %s",c->cfg_path);
	c->tmp_path = (char *)malloc(strlen(c->des_path)+strlen(TMP_PATH_NAME)+1);
	if(c->tmp_path == NULL)
	{
		DMCLOG_D("malloc error");
		res = -1;
		c->error = SERVER_OUT_MEMORY;
		goto EXIT;
	}
	sprintf(c->tmp_path,"%s%s",c->des_path,TMP_PATH_NAME);
	DMCLOG_D("tmp_path = %s",c->tmp_path);
	c->rem.content_len = c->length - c->offset;
	tmp = strrchr(c->tmp_path,'/');
	*tmp = '\0';
	res = make_dirs(c->tmp_path, 666);
	if(res != 0)
	{
		DMCLOG_D("make dirs error,errno = %d",errno);
		c->error = ERROR_MKDIR;   
		goto EXIT;
	}
	*tmp = '/';
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return res;	
}

int Parser_FileGetStorage(struct conn *c)
{
	ENTER_FUNC();
	int res = 0;
	/*JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = 0;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}*/
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	EXIT_FUNC();
	return res;	
}

int Parser_FileGetList(struct conn *c)
{
	int res = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	JObj *pageNum_json = JSON_GET_OBJECT(para_json,"pageNum");
	if(path_json == NULL||pageNum_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	int res_sz = 0;
	char *uri_src = JSON_GET_OBJECT_VALUE(path_json,string);
	if(uri_src == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	DMCLOG_D("uri = %s",uri_src);
	if (strlen(uri_src) + strlen(DOCUMENT_ROOT) >= URI_MAX) {
		res = -1;
		c->error = PATH_TOO_LONGTH;
		goto EXIT;
	}
	c->src_path = (char *)malloc(strlen(uri_src) + strlen(DOCUMENT_ROOT) + 2);
	if(c->src_path == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = SERVER_OUT_MEMORY;
		res = -1;
		goto EXIT;
	}
    if(*uri_src == '/')
    {
        sprintf(c->src_path,"%s%s",DOCUMENT_ROOT,uri_src);
    }else{
        sprintf(c->src_path,"%s/%s",DOCUMENT_ROOT,uri_src);
    }
	DMCLOG_D("decide_what_to_do: cmd:%d,src_path = %s", c->cmd,c->src_path);
	c->pageNum = JSON_GET_OBJECT_VALUE(pageNum_json,int);
	/*if(c->pageNum < 0)
	{
		DMCLOG_D("pageNum is not valide");
		c->error = REQUEST_FORMAT_ERROR;
		res = -1;
	}*/
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return res;	
}
/*
 * 文件夹创建 cmd = 102
 */
int Parser_FileMkdir(struct conn *c)
{
	int res = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	if(path_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	char *uri_src = JSON_GET_OBJECT_VALUE(path_json,string);
	if(uri_src == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	DMCLOG_D("uri = %s",uri_src);
	char disk_name[64];
	memset(disk_name,0,64);
    get_disk_name(uri_src, disk_name);
	DMCLOG_D("disk_name = %s",disk_name);
	c->disk_root = (char *)malloc(strlen(DOCUMENT_ROOT) + strlen(disk_name) + 2);
	if(c->disk_root == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = SERVER_OUT_MEMORY;
		res = -1;
		goto EXIT;
	}
	sprintf(c->disk_root,"%s/%s",DOCUMENT_ROOT,disk_name);
	DMCLOG_D("c->disk_root = %s",c->disk_root);
	if (strlen(uri_src) + strlen(DOCUMENT_ROOT) >= URI_MAX) {
		res = -1;
		c->error = PATH_TOO_LONGTH;
		goto EXIT;
	}
	c->src_path = (char *)malloc(strlen(uri_src) + strlen(DOCUMENT_ROOT) + 2);
	if(c->src_path == NULL)
	{
		DMCLOG_D("malloc error");
		res = -1;
		c->error = SERVER_OUT_MEMORY;
		goto EXIT;
	}
    if(*uri_src == '/')
    {
        sprintf(c->src_path,"%s%s",DOCUMENT_ROOT,uri_src);
    }else{
        sprintf(c->src_path,"%s/%s",DOCUMENT_ROOT,uri_src);
    }
    
	DMCLOG_D("decide_what_to_do: cmd:%d,src_path = %s", c->cmd,c->src_path);
    char *tmp = strrchr(c->src_path,'/');
    *tmp = '\0';
    res = make_dirs(c->src_path, 666);
    if(res != 0)
    {
        DMCLOG_D("make dirs error,errno = %d",errno);
        c->error = ERROR_MKDIR;
        goto EXIT;
    }
    *tmp = '/';
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return res;	
}
/*
 * 文件或文件夹重名 cmd = 103
 */
int Parser_FileRename(struct conn *c)
{
	int res = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *src_path_json = JSON_GET_OBJECT(para_json,"srcPath");
	JObj *des_path_json = JSON_GET_OBJECT(para_json,"desPath");
	if(src_path_json == NULL||des_path_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	int res_sz = 0;
	char *uri_src = JSON_GET_OBJECT_VALUE(src_path_json,string);
	if(uri_src == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	DMCLOG_D("uri_src = %s",uri_src);
	char disk_name[64];
	memset(disk_name,0,64);
    get_disk_name(uri_src,disk_name);
	c->disk_root = (char *)malloc(strlen(DOCUMENT_ROOT) + strlen(disk_name)+ 2 );
	if(c->disk_root == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = SERVER_OUT_MEMORY;
		res = -1;
		goto EXIT;
	}
	sprintf(c->disk_root,"%s/%s",DOCUMENT_ROOT,disk_name);
	DMCLOG_D("c->disk_root = %s",c->disk_root);
	if (strlen(uri_src) + strlen(DOCUMENT_ROOT) >= URI_MAX) {
		res = -1;
		c->error = PATH_TOO_LONGTH;
		goto EXIT;
	}
	c->src_path = (char *)malloc(strlen(uri_src) + strlen(DOCUMENT_ROOT) + 2);
	if(c->src_path == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = SERVER_OUT_MEMORY;
		res = -1;
		goto EXIT;
	}
    if(*uri_src == '/')
    {
        sprintf(c->src_path,"%s%s",DOCUMENT_ROOT,uri_src);
    }else{
        sprintf(c->src_path,"%s/%s",DOCUMENT_ROOT,uri_src);
    }
	char *uri_des = JSON_GET_OBJECT_VALUE(des_path_json,string);
	DMCLOG_D("uri_des = %s",uri_des);
	if (strlen(uri_des) + strlen(DOCUMENT_ROOT) >= URI_MAX) {
			res = -1;
			c->error = PATH_TOO_LONGTH;
			goto EXIT;
	}
	c->des_path = (char *)malloc(strlen(uri_des) + strlen(DOCUMENT_ROOT) + 2);
	if(c->des_path == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = SERVER_OUT_MEMORY;
		res = -1;
		goto EXIT;
	}
    if(*uri_src == '/')
    {
        sprintf(c->des_path,"%s%s",DOCUMENT_ROOT,uri_des);
    }else{
        sprintf(c->des_path,"%s/%s",DOCUMENT_ROOT,uri_des);
    }
	DMCLOG_D("c->des_path = %s",c->des_path);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return res;	
}
/*
 * 判断文件或文件夹是否存在 cmd = 104
 */
int Parser_FileIsExist(struct conn *c)
{
	int res = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	if(path_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	char *uri_src = JSON_GET_OBJECT_VALUE(path_json,string);
	if(uri_src == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	DMCLOG_D("uri_src = %s",uri_src);
	if (strlen(uri_src) + strlen(DOCUMENT_ROOT) >= URI_MAX) {
		res = -1;
		c->error = PATH_TOO_LONGTH;
		goto EXIT;
	}
	c->src_path = (char *)malloc(strlen(uri_src) + strlen(DOCUMENT_ROOT) + 2);
	if(c->src_path == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = SERVER_OUT_MEMORY;
		res = -1;
		goto EXIT;
	}
    if(*uri_src == '/')
    {
        sprintf(c->src_path,"%s%s",DOCUMENT_ROOT,uri_src);
    }else{
        sprintf(c->src_path,"%s/%s",DOCUMENT_ROOT,uri_src);
    }
EXIT:
	if(c->r_json != NULL)
			JSON_PUT_OBJECT(c->r_json);
	return res;	
}
/*
 * 获取文件或文件夹属性 cmd = 105
 */
int Parser_FileGetAttr(struct conn *c)
{
	int res = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	if(path_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	char *uri_src = JSON_GET_OBJECT_VALUE(path_json,string);
	if(uri_src == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	DMCLOG_D("uri_src = %s",uri_src);
	if (strlen(uri_src) + strlen(DOCUMENT_ROOT) >= URI_MAX) {
		res = -1;
		c->error = PATH_TOO_LONGTH;
		goto EXIT;
	}
	c->src_path = (char *)malloc(strlen(uri_src) + strlen(DOCUMENT_ROOT) + 2);
	if(c->src_path == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = SERVER_OUT_MEMORY;
		res = -1;
		goto EXIT;
	}
    if(*uri_src == '/')
    {
        sprintf(c->src_path,"%s%s",DOCUMENT_ROOT,uri_src);
    }else{
        sprintf(c->src_path,"%s/%s",DOCUMENT_ROOT,uri_src);
    }
EXIT:
	if(c->r_json != NULL)
			JSON_PUT_OBJECT(c->r_json);
	return res;	
}
/*
 *文件或文件夹删除 cmd = 109
 */
int parser_FileDelete(struct conn *c)
{
	int res = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	if(path_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *seq_json = JSON_GET_OBJECT(para_json,"seq");
    if(seq_json != NULL)
    {
    	c->del_seq = JSON_GET_OBJECT_VALUE(seq_json,int);
    }
    
	char *uri_src = JSON_GET_OBJECT_VALUE(path_json,string);
	if(uri_src == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	DMCLOG_D("uri_src = %s",uri_src);
	if (strlen(uri_src) + strlen(DOCUMENT_ROOT) >= URI_MAX) {
		res = -1;
		c->error = PATH_TOO_LONGTH;
		goto EXIT;
	}
	char disk_name[64];
	memset(disk_name,0,64);
    get_disk_name(uri_src,disk_name);
	DMCLOG_D("disk_name = %s",disk_name);
	c->disk_root = (char *)malloc(strlen(DOCUMENT_ROOT) + strlen(disk_name) + 2 );
	if(c->disk_root == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = SERVER_OUT_MEMORY;
		res = -1;
		goto EXIT;
	}
	sprintf(c->disk_root,"%s/%s",DOCUMENT_ROOT,disk_name);
	DMCLOG_D("disk_root = %s",c->disk_root);
	c->src_path = (char *)malloc(strlen(uri_src) + strlen(DOCUMENT_ROOT) + 2);
	if(c->src_path == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = SERVER_OUT_MEMORY;
		res = -1;
		goto EXIT;
	}
    if(*uri_src == '/')
    {
        sprintf(c->src_path,"%s%s",DOCUMENT_ROOT,uri_src);
    }else{
        sprintf(c->src_path,"%s/%s",DOCUMENT_ROOT,uri_src);
    }
	
	DMCLOG_D("c->src_path = %s",c->src_path);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return res;	
}
int parser_FileCopy(struct conn *c)
{
    int res = 0;
    JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
    if(data_json == NULL)
    {
        res = -1;
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
    if(para_json == NULL)
    {
        res = -1;
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    
    JObj *seq_json = JSON_GET_OBJECT(para_json,"seq");
    if(seq_json == NULL)
    {
        res = -1;
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    c->copy_seq = JSON_GET_OBJECT_VALUE(seq_json,int);
    JObj *src_path_json = JSON_GET_OBJECT(para_json,"path");
    if(src_path_json == NULL)
    {
        res = -1;
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    char *srcPath = JSON_GET_OBJECT_VALUE(src_path_json,string);
    if(srcPath == NULL)
    {
        res = -1;
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    DMCLOG_D("srcPath = %s",srcPath);
    if (strlen(srcPath) + strlen(DOCUMENT_ROOT) >= URI_MAX) {
        res = -1;
        c->error = PATH_TOO_LONGTH;
        goto EXIT;
    }
    c->src_path = (char *)malloc(strlen(srcPath) + strlen(DOCUMENT_ROOT) + 2);
    if(c->src_path == NULL)
    {
        DMCLOG_D("malloc error");
        c->error = SERVER_OUT_MEMORY;
        res = -1;
        goto EXIT;
    }
    if(*srcPath == '/')
    {
        sprintf(c->src_path,"%s%s",DOCUMENT_ROOT,srcPath);
    }else{
        sprintf(c->src_path,"%s/%s",DOCUMENT_ROOT,srcPath);
    }
    DMCLOG_D("c->src_path = %s",c->src_path);
    
    JObj *des_path_json = JSON_GET_OBJECT(para_json,"desPath");
    if(des_path_json == NULL)
    {
        res = -1;
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    char *desPath = JSON_GET_OBJECT_VALUE(des_path_json,string);
    if(desPath == NULL)
    {
        res = -1;
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    DMCLOG_D("desPath = %s",desPath);
    if (strlen(desPath) + strlen(DOCUMENT_ROOT) >= URI_MAX) {
        res = -1;
        c->error = PATH_TOO_LONGTH;
        goto EXIT;
    }
    c->des_path = (char *)malloc(strlen(desPath) + strlen(DOCUMENT_ROOT) + 2);
    if(c->des_path == NULL)
    {
        DMCLOG_D("malloc error");
        c->error = SERVER_OUT_MEMORY;
        res = -1;
        goto EXIT;
    }
    if(*desPath == '/')
    {
        sprintf(c->des_path,"%s%s",DOCUMENT_ROOT,desPath);
    }else{
        sprintf(c->des_path,"%s/%s",DOCUMENT_ROOT,desPath);
    }
    DMCLOG_D("c->des_path = %s",c->des_path);
    char disk_name[64];
    memset(disk_name,0,64);
    get_disk_name(desPath,disk_name);
    DMCLOG_D("disk_name = %s",disk_name);
    c->disk_root = (char *)malloc(strlen(DOCUMENT_ROOT) + strlen(disk_name) + 2 );
    if(c->disk_root == NULL)
    {
        DMCLOG_D("malloc error");
        c->error = SERVER_OUT_MEMORY;
        res = -1;
        goto EXIT;
    }
    sprintf(c->disk_root,"%s/%s",DOCUMENT_ROOT,disk_name);
    DMCLOG_D("disk_root = %s",c->disk_root);
EXIT:
    if(c->r_json != NULL)
        JSON_PUT_OBJECT(c->r_json);
    return res;	
}
int parser_FileMove(struct conn *c)
{
    int res = 0;
    JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
    if(data_json == NULL)
    {
        res = -1;
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
    if(para_json == NULL)
    {
        res = -1;
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    JObj *src_path_json = JSON_GET_OBJECT(para_json,"srcPath");
    if(src_path_json == NULL)
    {
        res = -1;
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    char *srcPath = JSON_GET_OBJECT_VALUE(src_path_json,string);
    if(srcPath == NULL)
    {
        res = -1;
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    DMCLOG_D("srcPath = %s",srcPath);
    if (strlen(srcPath) + strlen(DOCUMENT_ROOT) >= URI_MAX) {
        res = -1;
        c->error = PATH_TOO_LONGTH;
        goto EXIT;
    }
    c->src_path = (char *)malloc(strlen(srcPath) + strlen(DOCUMENT_ROOT) + 2);
    if(c->src_path == NULL)
    {
        DMCLOG_D("malloc error");
        c->error = SERVER_OUT_MEMORY;
        res = -1;
        goto EXIT;
    }
    if(*srcPath == '/')
    {
        sprintf(c->src_path,"%s%s",DOCUMENT_ROOT,srcPath);
    }else{
        sprintf(c->src_path,"%s/%s",DOCUMENT_ROOT,srcPath);
    }
    DMCLOG_D("c->src_path = %s",c->src_path);
    
    JObj *des_path_json = JSON_GET_OBJECT(para_json,"desPath");
    if(des_path_json == NULL)
    {
        res = -1;
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    char *desPath = JSON_GET_OBJECT_VALUE(des_path_json,string);
    if(desPath == NULL)
    {
        res = -1;
        c->error = REQUEST_FORMAT_ERROR;
        goto EXIT;
    }
    DMCLOG_D("desPath = %s",desPath);
    if (strlen(desPath) + strlen(DOCUMENT_ROOT) >= URI_MAX) {
        res = -1;
        c->error = PATH_TOO_LONGTH;
        goto EXIT;
    }
    c->des_path = (char *)malloc(strlen(desPath) + strlen(DOCUMENT_ROOT) + 2);
    if(c->des_path == NULL)
    {
        DMCLOG_D("malloc error");
        c->error = SERVER_OUT_MEMORY;
        res = -1;
        goto EXIT;
    }
    if(*desPath == '/')
    {
        sprintf(c->des_path,"%s%s",DOCUMENT_ROOT,desPath);
    }else{
        sprintf(c->des_path,"%s/%s",DOCUMENT_ROOT,desPath);
    }
    DMCLOG_D("c->des_path = %s",c->des_path);
    char disk_name[64];
    memset(disk_name,0,64);
    get_disk_name(desPath,disk_name);
    c->disk_root = (char *)malloc(strlen(DOCUMENT_ROOT) + strlen(disk_name) + 2 );
    if(c->disk_root == NULL)
    {
        DMCLOG_D("malloc error");
        c->error = SERVER_OUT_MEMORY;
        res = -1;
        goto EXIT;
    }
    sprintf(c->disk_root,"%s/%s",DOCUMENT_ROOT,disk_name);
    DMCLOG_D("disk_root = %s",c->disk_root);
EXIT:
    if(c->r_json != NULL)
        JSON_PUT_OBJECT(c->r_json);
    return res;
}
int Parser_FileGetListByType(struct conn *c)
{
	int res = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *fileType_json = JSON_GET_OBJECT(para_json,"fileType");
	//JObj *pageNum_json = JSON_GET_OBJECT(para_json,"pageNum");
	if(fileType_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	c->fileType = JSON_GET_OBJECT_VALUE(fileType_json,int);
	if(c->fileType <=  0)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	//c->pageNum = JSON_GET_OBJECT_VALUE(pageNum_json,int);
	/*if(c->pageNum < 0)
	{
		DMCLOG_D("pageNum is not valide");
		c->error = REQUEST_FORMAT_ERROR;
		res = -1;
	}*/
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return res;	
}
int Parser_FileGetClassInfo(struct conn *c)
{
	int res = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *fileType_json = JSON_GET_OBJECT(para_json,"fileType");
	if(fileType_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	c->fileType = JSON_GET_OBJECT_VALUE(fileType_json,int);
	if(c->fileType <=  0)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return res;	
}
int Parser_FileGetFilePathByType(struct conn *c)
{
	int res = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *fileType_json = JSON_GET_OBJECT(para_json,"fileType");
	if(fileType_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	c->fileType = JSON_GET_OBJECT_VALUE(fileType_json,int);
	if(c->fileType <=  0)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return res;	
}
int Parser_FileGetFileListByPath(struct conn *c)
{
	int res = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *fileType_json = JSON_GET_OBJECT(para_json,"fileType");
	if(fileType_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	c->fileType = JSON_GET_OBJECT_VALUE(fileType_json,int);
	if(c->fileType <=  0)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *fileNum_json = JSON_GET_OBJECT(para_json,"fileNum");
	if(fileNum_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	c->fileNum = JSON_GET_OBJECT_VALUE(fileNum_json,int);
	if(c->fileNum < 0)
	{
		DMCLOG_D("fileNum is not valide");
		c->error = REQUEST_FORMAT_ERROR;
		res = -1;
	}
	JObj *filePath_json = JSON_GET_OBJECT(para_json,"path");
	if(filePath_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	char *uri_src = JSON_GET_OBJECT_VALUE(filePath_json,string);
	if(uri_src == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	DMCLOG_D("uri_src = %s",uri_src);
	if (strlen(uri_src) + strlen(DOCUMENT_ROOT) >= URI_MAX) {
		res = -1;
		c->error = PATH_TOO_LONGTH;
		goto EXIT;
	}
	char disk_name[64];
	memset(disk_name,0,64);
    get_disk_name(uri_src,disk_name);
    DMCLOG_D("disk_name = %s",disk_name);
	c->disk_root = (char *)malloc(strlen(DOCUMENT_ROOT) + strlen(disk_name) + 2 );
	if(c->disk_root == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = SERVER_OUT_MEMORY;
		res = -1;
		goto EXIT;
	}
	sprintf(c->disk_root,"%s/%s",DOCUMENT_ROOT,disk_name);
	DMCLOG_D("disk_root = %s",c->disk_root);
	c->src_path = (char *)malloc(strlen(uri_src) + strlen(DOCUMENT_ROOT) + 2);
	if(c->src_path == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = SERVER_OUT_MEMORY;
		res = -1;
		goto EXIT;
	}
    if(*uri_src == '/')
    {
        sprintf(c->src_path,"%s%s",DOCUMENT_ROOT,uri_src);
    }else{
        sprintf(c->src_path,"%s/%s",DOCUMENT_ROOT,uri_src);
    }
	DMCLOG_D("c->src_path = %s",c->src_path);
	/*JObj *oderMode_json = JSON_GET_OBJECT(para_json,"oderMode");
	if(oderMode_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	c->statusFlag= JSON_GET_OBJECT_VALUE(oderMode_json,int);*/
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return res;	
}
int Parser_FileDelFileListByPath(struct conn *c)
{
	int res = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *fileType_json = JSON_GET_OBJECT(para_json,"fileType");
	if(fileType_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	c->fileType = JSON_GET_OBJECT_VALUE(fileType_json,int);
	if(c->fileType <=  0)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *filePath_json = JSON_GET_OBJECT(para_json,"path");
	if(filePath_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	char *uri_src = JSON_GET_OBJECT_VALUE(filePath_json,string);
	if(uri_src == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	DMCLOG_D("uri_src = %s",uri_src);
	if (strlen(uri_src) + strlen(DOCUMENT_ROOT) >= URI_MAX) {
		res = -1;
		c->error = PATH_TOO_LONGTH;
		goto EXIT;
	}
	char disk_name[64];
	memset(disk_name,0,64);
    get_disk_name(uri_src,disk_name);
    DMCLOG_D("disk_name = %s",disk_name);
	c->disk_root = (char *)malloc(strlen(DOCUMENT_ROOT) + strlen(disk_name) + 2 );
	if(c->disk_root == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = SERVER_OUT_MEMORY;
		res = -1;
		goto EXIT;
	}
	sprintf(c->disk_root,"%s/%s",DOCUMENT_ROOT,disk_name);
	DMCLOG_D("disk_root = %s",c->disk_root);
	c->src_path = (char *)malloc(strlen(uri_src) + strlen(DOCUMENT_ROOT) + 2);
	if(c->src_path == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = SERVER_OUT_MEMORY;
		res = -1;
		goto EXIT;
	}
    if(*uri_src == '/')
    {
        sprintf(c->src_path,"%s%s",DOCUMENT_ROOT,uri_src);
    }else{
        sprintf(c->src_path,"%s/%s",DOCUMENT_ROOT,uri_src);
    }
	DMCLOG_D("c->src_path = %s",c->src_path);
	/*JObj *oderMode_json = JSON_GET_OBJECT(para_json,"oderMode");
	if(oderMode_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	c->statusFlag= JSON_GET_OBJECT_VALUE(oderMode_json,int);*/
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return res;	
}

int Parser_FileGetScanStatus(struct conn *c)
{
	ENTER_FUNC();
	int res = 0;
	/*JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = 0;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}*/
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	EXIT_FUNC();
	return res;	
}
int Parser_FileGetBackupInfo(struct conn *c)
{
	ENTER_FUNC();
	int res = 0;
	/*JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = 0;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}*/
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	EXIT_FUNC();
	return res;	
}
int Parser_FileBindBackupDisk(struct conn *c)
{
	int res = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *diskName_json = JSON_GET_OBJECT(para_json,"diskName");
	if(diskName_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	char *disk_name = JSON_GET_OBJECT_VALUE(diskName_json,string);
	if(disk_name == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	#if 0
	c->disk_name = (char *)malloc(strlen(disk_name) + 1);
	if(c->disk_name == NULL)
	{
		DMCLOG_D("malloc error");
		res = -1;
		c->error = SERVER_OUT_MEMORY;
		goto EXIT;
	}
	strcpy(c->disk_name,disk_name);
	#endif
	DMCLOG_D("disk_name = %s",disk_name);
	c->disk_root = (char *)malloc(strlen(DOCUMENT_ROOT) + strlen(disk_name) + 2 );
	if(c->disk_root == NULL)
	{
		DMCLOG_D("malloc error");
		c->error = SERVER_OUT_MEMORY;
		res = -1;
		goto EXIT;
	}
	sprintf(c->disk_root,"%s/%s",DOCUMENT_ROOT,disk_name);
	DMCLOG_D("disk_root = %s",c->disk_root);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);		
	return res;	
}

int Parser_FileGetBackupFile(struct conn *c)
{
	int res = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		res = -1;
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
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	c->fileSize= JSON_GET_OBJECT_VALUE(fileSize_json,int64);
	c->modifyTime = JSON_GET_OBJECT_VALUE(modifyTime_json,int64);
	c->offset = JSON_GET_OBJECT_VALUE(offset_json,int64);
	c->length = JSON_GET_OBJECT_VALUE(length_json,int64);
	c->rem.content_len = c->length - c->offset;

	char *fileUuid = JSON_GET_OBJECT_VALUE(fileUuid_json,string);
	char *dir_name = JSON_GET_OBJECT_VALUE(dirName_json,string);
	char *src_path = JSON_GET_OBJECT_VALUE(path_json,string);
	if(src_path == NULL || dir_name == NULL || fileUuid == NULL)
	{
		res = -1;
		DMCLOG_D("access");
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
		
	c->file_uuid= (char *)malloc(strlen(fileUuid) + 1);
	if(c->file_uuid == NULL)
	{
		DMCLOG_D("malloc error");
		res = -1;
		c->error = SERVER_OUT_MEMORY;
		goto EXIT;
	}
	strcpy(c->file_uuid,fileUuid);
	char device_uuid[64];
	char device_name[64];
	memset(device_uuid,0,64);
	memset(device_name,0,64);
	res = get_devuuid_from_usr_table(c->session,device_uuid,device_name);
	if(res < 0)
	{
		DMCLOG_D("get_devuuid_from_usr_table error");
		c->error = DM_ERROR_DB_DEV_TABLE;
		goto EXIT;
	}
	c->deviceUuid = (char *)malloc(strlen(device_uuid) + 1);
	strcpy(c->deviceUuid,device_uuid);
	char disk_uuid[64];
	memset(disk_uuid,0,64);
	res = get_bind_disk_uuid_from_db(device_uuid,disk_uuid);
	if(res < 0)
	{
		DMCLOG_D("get_devuuid_from_usr_table error");
		c->error = DM_ERROR_DB_HDISK_TABLE;
		goto EXIT;
	}
	DMCLOG_D("disk_uuid = %s",disk_uuid);
	c->disk_uuid = (char *)malloc(strlen(disk_uuid) + 1);
	strcpy(c->disk_uuid,disk_uuid);
	struct disk_node *disk_info = get_disk_node(disk_uuid);
	
	if(disk_info != NULL)
	{
		c->des_path = (char *)malloc(strlen(disk_info->path) + strlen(device_name) + strlen(dir_name) + strlen(bb_basename(src_path)) + 4 + 3);
		sprintf(c->des_path,"%s/%s/%s/%s",disk_info->path,device_name,dir_name,bb_basename(src_path));
	}else{
		c->error = DM_ERROR_DB_BACKUP_TABLE;
		goto EXIT;
	}
	c->cfg_path = (char *)malloc(strlen(c->des_path)+strlen(CFG_PATH_NAME)+1);
	if(c->cfg_path == NULL)
	{
		DMCLOG_D("malloc error");
		res = -1;
		c->error = SERVER_OUT_MEMORY;
		goto EXIT;
	}
	sprintf(c->cfg_path,"%s%s",c->des_path,CFG_PATH_NAME);
	DMCLOG_D("cfg_path = %s",c->cfg_path);
	c->tmp_path = (char *)malloc(strlen(c->des_path)+strlen(TMP_PATH_NAME)+1);
	if(c->tmp_path == NULL)
	{
		DMCLOG_D("malloc error");
		res = -1;
		c->error = SERVER_OUT_MEMORY;
		goto EXIT;
	}
	sprintf(c->tmp_path,"%s%s",c->des_path,TMP_PATH_NAME);
	DMCLOG_D("tmp_path = %s,disk_uuid = %s",c->tmp_path,c->disk_uuid);
	res = handle_backfile_insert_cmd(c,1);//start backup file
	if(res < 0)
	{
        DMCLOG_D("backup file(%s) failed, ret(0x%x)", c->file_uuid, res);
        c->error = DM_ERROR_DB_BACKUP_TABLE;
        goto EXIT;
    }
	char *tmp = strrchr(c->tmp_path,'/');
	*tmp = '\0';
	res = make_dirs(c->tmp_path, 666);
	if(res != 0)
	{
		DMCLOG_D("make dirs error");
		c->error = ERROR_MKDIR;   
		goto EXIT;
	}
	*tmp = '/';
	DMCLOG_D("c->tmp_path = %s",c->tmp_path);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return res;	
}
int Parser_FileCheckBackupFileList(struct conn *c)
{
	ENTER_FUNC();
	int res = 0;
	unsigned long i;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *backupfilelist_json = JSON_GET_OBJECT(data_json,"backupFileListInfo");
	if(backupfilelist_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	int array_len = JSON_GET_ARRAY_LEN(backupfilelist_json);
	if(array_len == 0)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	DMCLOG_D("array_len = %d",array_len);
	c->flist = (file_uuid_list_t *)malloc(sizeof(file_uuid_list_t));
	c->flist->result_cnt = 0;
	dl_list_init(&c->flist->head);
	for(i = 0;i < array_len;i++)
	{
		JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(backupfilelist_json,i);
		if(para_json == NULL)
		{
			DMCLOG_D("access NULL");
			res = -1;
			c->error = REQUEST_FORMAT_ERROR;
			goto EXIT;
		}
		JObj *fileUuid_json = JSON_GET_OBJECT(para_json,"fileUuid");
		
		if(fileUuid_json == NULL)
		{
			DMCLOG_D("access NULL");
			res = -1;
			c->error = REQUEST_FORMAT_ERROR;
			goto EXIT;
		}
		char *fileUuid = JSON_GET_OBJECT_VALUE(fileUuid_json,string);
		DMCLOG_D("add i = %d,file_uuid = %s",i,fileUuid);
		file_uuid_t *fdi = (file_uuid_t *)malloc(sizeof(file_uuid_t));
		if(fdi == NULL)
		{
			res = -1;
			c->error = SERVER_OUT_MEMORY;
			goto EXIT;
		}
		fdi->backupFlag = 0;
		strcpy(fdi->file_uuid,fileUuid);
		dl_list_add_tail(&c->flist->head, &fdi->next);
		c->flist->result_cnt++;
	}
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	EXIT_FUNC();
	return res;
}

int Parser_FileCheckBackupFile(struct conn *c)
{
	int res = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *fileUuid_json = JSON_GET_OBJECT(para_json,"fileUuid");
	JObj *path_json = JSON_GET_OBJECT(para_json,"path");
	JObj *fileSize_json = JSON_GET_OBJECT(para_json,"fileSize");
	JObj *modifyTime_json = JSON_GET_OBJECT(para_json,"modifyTime");
	
	DMCLOG_D("c->cmd = %d",c->cmd);
	if(fileSize_json == NULL||modifyTime_json == NULL||fileUuid_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	char *fileUuid = JSON_GET_OBJECT_VALUE(fileUuid_json,string);
	c->fileSize = JSON_GET_OBJECT_VALUE(fileSize_json,int64);
	c->modifyTime = JSON_GET_OBJECT_VALUE(modifyTime_json,int);
	c->file_uuid= (char *)malloc(strlen(fileUuid) + 1);
	if(c->file_uuid == NULL)
	{
		DMCLOG_D("malloc error");
		res = -1;
		c->error = SERVER_OUT_MEMORY;
		goto EXIT;
	}
	strcpy(c->file_uuid,fileUuid);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return res;	
}

int Parser_FileIsBackupFile(struct conn *c)
{
	int res = 0;
	JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *para_json = JSON_GET_ARRAY_MEMBER_BY_ID(data_json,0);
	if(para_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	JObj *uuid_json = JSON_GET_OBJECT(para_json,"fileUuid");
	if(uuid_json == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	char *file_uuid = JSON_GET_OBJECT_VALUE(uuid_json,string);
	if(file_uuid == NULL)
	{
		res = -1;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}
	DMCLOG_D("file_uuid = %s",file_uuid);
	c->file_uuid = (char *)malloc(strlen(file_uuid) + 1);
	if(c->file_uuid == NULL)
	{
		DMCLOG_D("malloc error");
		res = -1;
		c->error = SERVER_OUT_MEMORY;
		goto EXIT;
	}
	strcpy(c->file_uuid,file_uuid);
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	return res;		
}

int Parser_FileResetBackup(struct conn *c)
{
	ENTER_FUNC();
	int res = 0;
	/*JObj *data_json = JSON_GET_OBJECT(c->r_json,"data");
	if(data_json == NULL)
	{
		res = 0;
		c->error = REQUEST_FORMAT_ERROR;
		goto EXIT;
	}*/
EXIT:
	if(c->r_json != NULL)
		JSON_PUT_OBJECT(c->r_json);
	EXIT_FUNC();
	return res;	
}
int file_parse_process(struct conn *c)
{
	//TODO 需添加解析json文件的出错机制
    uint8_t i = 0;
    uint8_t switch_flag = 0;
    int ret = 0;
	ret = file_parse_header_json(c);
	if(ret != 0){
		return ret;
	}
    for(i = 0; i< FILE_TAGHANDLE_NUM; i++)
    {
        if(c->cmd == all_file_handle[i].tag)
        {
        	DMCLOG_D("c->cmd = %d",c->cmd);
            ret = all_file_handle[i].parsefun(c);
            switch_flag = 1;
        }
    }
    if(switch_flag == 0)
    {
        c->error = INVALIDE_COMMAND;//命令无法识别
		DMCLOG_D("cmd not found");
    }else if(ret < 0)
    {
		c->error = REQUEST_FORMAT_ERROR;//命令的格式错误
	}	
    return ret;
}


int verify_firmware()
{
	FILE *fw_fp = NULL;
	long fw_len = 0;
	int f_size = 0;
	char fw_tmp[256];
	char op_fw_header[32]="\0";
	char product_model[33]="\0";
	int ret = 0;
	char path[128];
	char cmd[128];
	memset(path, 0, sizeof(path));
	sprintf(path, "%s%s", FW_FILE, TMP_PATH_NAME);
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
			sprintf(cmd, "cp %s%s /tmp/fwupgrade_1.gz", FW_FILE, TMP_PATH_NAME);
			system(cmd);
			system("gzip -d /tmp/fwupgrade_1.gz");
			if( (fw_fp=fopen("/tmp/fwupgrade_1", "rb"))==NULL)    // read binary
			{
				DMCLOG_E("can't open /tmp/fwupgrade_1");
				rm("/tmp/fwupgrade_1.gz");
				memset(path, 0, sizeof(path));
				sprintf(path ,"%s%s", FW_FILE, TMP_PATH_NAME);
				rm(path);
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
			rm("/tmp/fwupgrade_1.gz");
			rm("/tmp/fwupgrade_1");
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



