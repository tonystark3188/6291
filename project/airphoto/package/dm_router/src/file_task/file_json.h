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






#define ERROR_GET_WIFI_SETTINGS 24
#define ERROR_SET_WIFI_SETTINGS 25
#define ERRPR_GET_REMOTEAP_INFO 26
#define ERROR_GET_WLAN_CONNECTION_MODE 27
#define ERROR_GET_WIRED_CONNECTION_MODE 28
#define ERROR_SET_WIRED_CONNECTION_MODE 29
#define ERROR_GET_AP_LSIT 30
#define ERROR_GET_POWER 31

#define ERROR_FORMAT_DISK 32
#define ERROR_GET_FTP_SETTINGS 33
#define ERROR_SET_FTP_SETTINGS 34
#define ERROR_GET_SMB_SETTINGS 35
#define ERROR_SET_SMB_SETTINGS 36
#define ERROR_GET_DMS_SETTINGS 37
#define ERROR_SET_DMS_SETTINGS 38
#define ERROR_GET_DDNS_SETTINGS 39
#define ERROR_SET_DDNS_SETTINGS 40

#define ERROR_GET_WEBDAV_SETTINGS 41
#define ERROR_SET_WEBDAV_SETTINGS 42
#define ERROR_GET_ELEC_LOCK 43
#define ERROR_SET_ELEC_LOCK 44
#define ERROR_GET_3G_INFO 45
#define ERROR_SET_3G_INFO 46
#define ERROR_GET_CLIENT_STATUS 47
#define ERROR_SET_CLIENT_STATUS 48
#define ERROR_UPGRADE_FW 49



#define ERROR_LOGIN 1
#define ERROR_LOGOUT 2
#define ERROR_GET_SERVICE_LIST 3
#define ERROR_GET_OPTION 4
#define ERROR_DEL_CLIENT_INFO 5
#define ERROR_GET_STATUS_CHANGED 21
#define ERROR_SET_STATUS_CHANGED 22


#define ERROR_GET_WIFI_SETTINGS 24
#define ERROR_SET_WIFI_SETTINGS 25
#define ERRPR_GET_REMOTEAP_INFO 26
#define ERROR_GET_WLAN_CONNECTION_MODE 27
#define ERROR_GET_WIRED_CONNECTION_MODE 28
#define ERROR_SET_WIRED_CONNECTION_MODE 29
#define ERROR_GET_AP_LSIT 30
#define ERROR_GET_POWER 31

#define ERROR_FORMAT_DISK 32
#define ERROR_GET_FTP_SETTINGS 33
#define ERROR_SET_FTP_SETTINGS 34
#define ERROR_GET_SMB_SETTINGS 35
#define ERROR_SET_SMB_SETTINGS 36
#define ERROR_GET_DMS_SETTINGS 37
#define ERROR_SET_DMS_SETTINGS 38
#define ERROR_GET_DDNS_SETTINGS 39
#define ERROR_SET_DDNS_SETTINGS 40

#define ERROR_GET_WEBDAV_SETTINGS 41
#define ERROR_SET_WEBDAV_SETTINGS 42
#define ERROR_GET_ELEC_LOCK 43
#define ERROR_SET_ELEC_LOCK 44
#define ERROR_GET_3G_INFO 45
#define ERROR_SET_3G_INFO 46
#define ERROR_GET_CLIENT_STATUS 47
#define ERROR_SET_CLIENT_STATUS 48
#define ERROR_UPGRADE_FW 49




#define FN_DM_GET_VERSION 1
#define FN_DM_LOGIN 2
#define FN_DM_LOGOUT 3
#define FN_GET_SERVICE_INFO 4
#define FN_DEL_CLIENT_INFO 5
#define FN_DISK_SCANNING 6

#define FN_DM_ROUTER_GET_FUNC_LIST 15
#define	FN_DM_ROUTER_GET_STATUS_CHANGED 16
#define	FN_ROUTER_SET_STATUS_CHANGED_LISTENER 17
#define FN_ROUTER_NOTIFY_STATUS_CHANGED 18
#define FN_ROUTER_RESPONSE_STATUS_CHANGED 19

#define FN_DM_ROUTER_GET_OPTION 20
#define FN_ROUTER_GET_WIFI_INFO			21
#define FN_ROUTER_SET_WIFI_INFO 		22
#define FN_ROUTER_GET_REMOTE_AP_INFO	23
#define FN_ROUTER_SET_REMOTE_AP_INFO	24
#define FN_ROUTER_GET_CONNECT_MODE		25
#define FN_ROUTER_GET_WIRED_INFO		26
#define FN_ROUTER_SET_WIRED_INFO		27
#define FN_ROUTER_GET_REMOTE_AP_LIST	28
#define FN_ROUTER_GET_POWER_INFO		29
#define FN_ROUTER_GET_DISK_INFO			30
#define FN_ROUTER_SET_DISK_FORMAT		31
#define FN_ROUTER_GET_FTP_INFO			32
#define FN_ROUTER_SET_FTP_INFO			33
#define FN_ROUTER_GET_SAMBA_INFO		34
#define FN_ROUTER_SET_SAMBA_INFO		35
#define FN_ROUTER_GET_DMS_INFO			36
#define FN_ROUTER_SET_DMS_INFO			37
#define FN_ROUTER_GET_DDNS_INFO			38
#define FN_ROUTER_SET_DDNS_INFO			39
#define FN_ROUTER_GET_WEBDAV_INFO 		40
#define FN_ROUTER_SET_WEBDAV_INFO		41
#define FN_ROUTER_GET_ELC_LOCK_INFO		42
#define FN_ROUTER_SET_ELC_LOCK_INFO		43
#define FN_ROUTER_GET_3G_INFO			44
#define FN_ROUTER_SET_3G_INFO			45
#define FN_ROUTER_GET_CLIENT_STATUS		46
#define FN_ROUTER_SET_CLIENT_STATUS		47
#define FN_ROUTER_SET_UDISK_UPGRADE		48
#define FN_ROUTER_GET_INTERNET_STATUS	49
#define FN_ROUTER_SET_TIME_SYNC			50
#define FN_ROUTER_SET_SYSTEM_SYNC		51
#define FN_ROUTER_GET_FIRMWARE_VERSION	52
#define FN_ROUTER_SET_RESET				53
#define FN_ROUTER_GET_OTA_INFO			54
#define FN_ROUTER_GET_VERSION_FLAG		55
#define FN_ROUTER_SET_VERSION_FLAG		56
//#define FN_ROUTER_GET_PRIVATE_DISK_INFO 54

#define FN_FILE_GET_STORAGE 100
#define FN_FILE_GET_LIST 101
#define FN_FILE_MKDIR 102
#define FN_FILE_RENAME 103
#define FN_FILE_IS_EXIST 104
#define FN_FILE_GET_ATTR 105
#define FN_FILE_DOWNLOAD 106
#define FN_FILE_UPLOAD 107
#define FN_FILE_CHECKUPLOAD 108
#define FN_FILE_DELETE 109
#define FN_FILE_GET_LSIT_BY_TYPE 120
#define FN_FILE_GET_CLASS_INFO 121

#define FN_FILE_GET_PATH_INFO_BY_TYPE 136
#define FN_FILE_GET_LIST_BY_PATH 137

#define FN_FILE_GET_SCAN_STATUS 122
#define FN_FILE_GET_BACKUP_INFO 130
#define FN_FILE_BIND_BACKUP_DISK 131
#define FN_FILE_GET_BACKUP_FILE 132
#define FN_FILE_CHECK_BACKUP_FILE 133
#define FN_FILE_IS_BACKUP_EXISTED 134
#define FN_FILE_RESET_BACKUP 135
#define FN_FILE_CHECK_BACKUP_FILE_LIST 138

#define FN_CAM_GET_INFO 139
#define FN_CAM_LIST_FILE 140
#define FN_CAM_LIST_FOLDER 141
#define FN_CAM_GET_EXIF 142
#define FN_CAM_GET_THUMB 143
#define FN_CAM_GET_FILEINFO 144
#define FN_CAM_READ_FILE 145
#define FN_CAM_DOWNLOAD_FILE 146
#define FN_CAM_DEL_FILE 147
#define FN_CAM_DETECT 148
#define FN_CAM_LIST_FILE_BYPAGE 149













/**********************************file client module****************************************************/

/*
 * 文件下载 cmd = 106
 */
int dm_file_download(struct conn *c);
int dm_file_check_upload(struct conn *c);

int dm_file_upload(struct conn *c);
/*
 * 获取磁盘信息 cmd = 100
 */
int DM_FileGetStorage(struct conn *c);
/*
 * 获取文件列表 cmd = 101
 */
int DM_FileGetList(struct conn *c);
/*
 * 文件夹创建 cmd = 102
 */
int DM_FileMkdir(struct conn *c);
/*
 * 文件或文件夹重名 cmd = 103
 */
int DM_FileRename(struct conn *c);
/*
 * 判断文件或文件夹是否存在 cmd = 104
 */
int DM_FileIsExist(struct conn *c);
/*
 * 获取文件或文件夹属性 cmd = 105
 */
int DM_FileGetAttr(struct conn *c);

/*
 *文件或文件夹删除 cmd = 109
 */
int DM_FileDelete(struct conn *c);
int DM_FileGetListByType(struct conn *c);
int DM_FileGetClassInfo(struct conn *c);
int DM_FileGetPathInfoByType(struct conn *c);
int DM_FileGetListByPath(struct conn *c);

int DM_FileGetScanStatus(struct conn *c);
int DM_FileGetBackupInfo(struct conn *c);

int dm_disk_scanning(struct conn *c);
int DM_FileGetBackupInfo(struct conn *c);
int DM_FileBindBackupDisk(struct conn *c);
int DM_FileGetBackupFile(struct conn *c);

int DM_FileCheckBackupFile(struct conn *c);
int DM_FileCheckBackupFileList(struct conn *c);


int DM_FileIsBackupExisted(struct conn *c);
int DM_FileResetBackup(struct conn *c);

int dm_file_upload_inotify(char *buf);

int DM_GetCamInfo(struct conn *c);
int DM_CamListFile(struct conn *c);
int DM_CamListFolder(struct conn *c);
int DM_CamGetExif(struct conn *c);
int DM_CamGetThumb(struct conn *c);
int DM_CamGetFileInfo(struct conn *c);
int DM_CamReadFile(struct conn *c);
int DM_CamDownloadFile(struct conn *c);
int DM_CamDelFile(struct conn *c);
int DM_CamDetect(struct conn *c);
int DM_CamListFileByPage(struct conn *c);








/**********************************file client module****************************************************/
/*
 * 文件下载 cmd = 106
 */
int Parser_FileDownload(struct conn *c);

/*
 * 上传检查 cmd = 108
 */
int Parser_FileCheckUpload(struct conn *c);
/*
 * 上传命令cmd = 107
 */
int Parser_FileUpload(struct conn *c);

/*
 * 获取磁盘信息 cmd = 100
 */
int Parser_FileGetStorage(struct conn *c);
/*
 * 获取文件列表 cmd = 101
 */
int Parser_FileGetList(struct conn *c);
/*
 * 文件夹创建 cmd = 102
 */
int Parser_FileMkdir(struct conn *c);
/*
 * 文件或文件夹重名 cmd = 103
 */
int Parser_FileRename(struct conn *c);
/*
 * 判断文件或文件夹是否存在 cmd = 104
 */
int Parser_FileIsExist(struct conn *c);
/*
 * 获取文件或文件夹属性 cmd = 105
 */
int Parser_FileGetAttr(struct conn *c);
/*
 *文件或文件夹删除 cmd = 109
 */
int parser_FileDelete(struct conn *c);

int Parser_FileGetListByType(struct conn *c);
int Parser_FileGetClassInfo(struct conn *c);
int Parser_FileGetScanStatus(struct conn *c);
int Parser_FileGetBackupInfo(struct conn *c);
int Parser_FileGetFilePathByType(struct conn *c);
int Parser_FileGetFileListByPath(struct conn *c);

int Parser_DiskScanning(struct conn *c);

int Parser_FileGetBackupInfo(struct conn *c);
int Parser_FileBindBackupDisk(struct conn *c);

int Parser_FileGetBackupFile(struct conn *c);
int Parser_FileCheckBackupFile(struct conn *c);

int Parser_FileCheckBackupFileList(struct conn *c);

int Parser_FileIsBackupFile(struct conn *c);

int Parser_FileResetBackup(struct conn *c);

int Parser_GetCamInfo(struct conn *c);
int Parser_CamListFile(struct conn *c);
int Parser_CamListFolder(struct conn *c);
int Parser_CamGetExif(struct conn *c);
int Parser_CamGetThumb(struct conn *c);
int Parser_CamGetFileInfo(struct conn *c);
int Parser_CamReadFile(struct conn *c);
int Parser_CamDownloadFile(struct conn *c);
int Parser_CamDelFile(struct conn *c);
int Parser_Camdetect(struct conn *c);
int Parser_CamListFileByPage(struct conn *c);






/*############################## Enums ######################################*/

/*############################## Structs #####################################*/



/*############################## Macros ######################################*/
extern int dm_get_version(struct conn *c);
extern int dm_login(struct conn *c);
extern int dm_logout(struct conn *c);
extern int dm_get_service_info(struct conn *c);
extern int _dm_del_client_info(struct conn *c);
/*##############################router module start#####################################*/
extern int dm_router_get_func_list(struct conn *c);

extern int dm_router_get_option(struct conn *c);
extern int dm_router_get_status_changed(struct conn *c);
extern int dm_router_set_status_changed_listener(struct conn *c);


/*##############################router module end#####################################*/




extern int Parser_GetVersion(struct conn *c);
extern int Parser_Login(struct conn *c);
extern int Parser_Logout(struct conn *c);
extern int Parser_GetServiceInfo(struct conn *c);
extern int Parser_DelClientInfo(struct conn *c);
extern int Parser_RouterGetOption(struct conn *c);
extern int Parser_RouterGetFuncList(struct conn *c);

extern int Parser_RouterGetStatusChanged(struct conn *c);
extern int Parser_RouterSetStatusListen(struct conn *c);

extern int _handle_getStorageInfo(struct conn *c);
extern int _dm_get_wifi_settings(struct conn *c);
extern int _dm_set_wifi_settings(struct conn *c);
extern int _dm_get_remote_ap_info(struct conn *c);
extern int _dm_set_remote_ap_info(struct conn *c);
extern int _dm_get_wlan_con_mode(struct conn *c);
extern int _dm_set_wired_con_mode(struct conn *c);
extern int _dm_get_wired_con_mode(struct conn *c);
extern int _dm_get_wlan_list(struct conn *c);
extern int _dm_get_power(struct conn *c);
extern int _dm_format_disk(struct conn *c);

//4.12	获取FTP信息  cmd = 0x020B
extern int _dm_get_ftp_settings(struct conn *c);

//4.13	设置FTP信息  cmd = 0x020C 
extern int _dm_set_ftp_settings(struct conn *c);
//4.14	获取samba配置  cmd = 0x020D
extern int _dm_get_smb_settings(struct conn *c);
//4.15	设置samba信息  cmd = 0x020E
extern int _dm_set_smb_settings(struct conn *c);

//4.16	获取DMS配置  cmd = 0x020F
extern int _dm_get_dms_settings(struct conn *c);
//4.17	设置DMS信息  cmd = 0x0210
extern int _dm_set_dms_settings(struct conn *c);

//4.18	获取DDNS配置  cmd = 0x0211
extern int _dm_get_ddns_settings(struct conn *c);

//4.19	设置DDNS信息  cmd = 0x0212
extern int _dm_set_ddns_settings(struct conn *c);
//4.20	获取WebDAV配置  cmd = 0x0213
extern int _dm_get_webdav_settings(struct conn *c);
//4.21	设置WebDAV信息  cmd = 0x0214
extern int _dm_set_webdav_settings(struct conn *c);
//4.22	获取电子锁信息  cmd = 0x0215
extern int _dm_get_elec_lock(struct conn *c);
//4.23	设置电子锁信息  cmd = 0x0216
extern int _dm_set_elec_lock(struct conn *c);

//4.24	获取3G接入信息  cmd = 0x0217
extern int _dm_get_3g_access_info(struct conn *c);
//4.25	设置3G接入信息  cmd = 0x0218
extern int _dm_set_3g_access_info(struct conn *c);

//4.26	获取client功能状态  cmd = 0x0219
int _dm_get_client_status(struct conn *c);
//4.27	设置client功能状态  cmd = 0x021A
extern int _dm_set_client_status(struct conn *c);

//4.28	执行从U盘升级  cmd = 0x021B
extern int _dm_upgrade_fw(struct conn *c);
//4.29	获取是否已经连上互联网 cmd = 0x021C
extern int _dm_get_internet_status(struct conn *c);
//4.30	同步时间  cmd = 0x021D
extern int _dm_sync_time(struct conn *c);

extern int _dm_get_ota_info(struct conn *c);

extern int _dm_get_version_flag(struct conn *c);

extern int _dm_set_version_flag(struct conn *c);

extern int _dm_get_fw_version(struct conn *c);
extern int Parser_RouterGetWifiSettings(struct conn *c);
extern int Parser_RouterSetWifiSettings(struct conn *c);
extern int Parser_RouterGetRemoteInfo(struct conn *c);
extern int Parser_RouterSetRemoteInfo(struct conn *c);
extern int Parser_RouterGetWlanConMode(struct conn *c);
extern int Parser_RouterGetWireConMode(struct conn *c);
extern int Parser_RouterSetWireConMode(struct conn *c);
extern int Parser_RouterGetWlanList(struct conn *c);
extern int Parser_RouterGetPower(struct conn *c);
extern int Parser_RouterGetStorageInfo(struct conn *c);
extern int Parser_RouterFormatDisk(struct conn *c);
extern int Parser_RouterGetFtpSettings(struct conn *c);
extern int Parser_RouterSetFtpSettings(struct conn *c);
extern int Parser_RouterGetSMBSettings(struct conn *c);
extern int Parser_RouterSetSMBSettings(struct conn *c);
extern int Parser_RouterGetDMSSettings(struct conn *c);
extern int Parser_RouterSetDMSSettings(struct conn *c);
extern int Parser_RouterGetDDNSSettings(struct conn *c);
extern int Parser_RouterSetDDNSSettings(struct conn *c);
extern int Parser_RouterGetWebDavSettings(struct conn *c);
extern int Parser_RouterSetWebDavSettings(struct conn *c);
extern int Parser_RouterGetElecSettings(struct conn *c);
extern int Parser_RouterSetElecSettings(struct conn *c);
extern int Parser_RouterGet3gSettings(struct conn *c);
extern int Parser_RouterSet3gSettings(struct conn *c);
extern int Parser_RouterGetClientStatus(struct conn *c);
extern int Parser_RouterSetClientStatus(struct conn *c);
extern int Parser_RouterUpgradeFw(struct conn *c);
extern int Parser_RouterGetInternetStatus(struct conn *c);
extern int Parser_RouterSyncTime(struct conn *c);
extern int Parser_RouterGetFwVersion(struct conn *c);
extern int Parser_RouterGetOtaInfo(struct conn *c);
extern int Parser_RouterGetVersionFlag(struct conn *c);
extern int Parser_RouterSetVersionFlag(struct conn *c);







#ifdef __cplusplus
}
#endif

#endif

