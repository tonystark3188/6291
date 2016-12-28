/*
 * =============================================================================
 *
 *       Filename:  hidisk_errno.h
 *
 *    Description:  AirNas errno define module
 *
 *        Version:  1.0
 *        Created:  2015/3/19 11:48
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver
 *   Organization:  
 *
 * =============================================================================
 */
#ifndef _MY_AIRNAS_ERRNO_H_
#define _MY_AIRNAS_ERRNO_H_
	 
#ifdef __cplusplus
	 extern "C"{
#endif
	 


typedef enum ERROR_CODE
{
	ACTION_SUCCESS = 0,
	ERROR_LOGOUT = 2,
	ERROR_GET_STATUS_CHANGED = 21,
	ERROR_SET_STATUS_CHANGED = 22,
	ERROR_UPGRADE_FW = 49,
	REQUEST_ERROR = 10000,//请求失败	
	INVALIDE_COMMAND  = 10001,/*命令无效*/
	REQUEST_FORMAT_ERROR = 10002,/*json数据格式非法*/
	DM_ERROR_CMD_PARAMETER = 10003,/*JSON参数错误*/
	//DM_ERROR_TARGET_NOT_FOUND = 10004,/**/
	LENGTH_REQUIRED = 10005,/*上传需指定长度*/
	PATH_TOO_LONGTH = 10006,/*文件路径过长*/
	DM_ERROR_DICONNECTED =  10007,//网络连接断开
	DM_ERROR_UNAUTHORIZED = 10100,/*无权限*/
	DM_ERROR_PERMISSION = 10101,/*不被允许访问*/
	DM_ERROR_SERVER_BUSY = 10102,/*服务器繁忙*/
	DM_ERROR_SERVER_TIMEOUT = 10103,/*请求服务器超时*/
	SERVER_OUT_MEMORY = 10104,/*内存不够*/
	DM_SRROR_SERVER_FS = 10105,/*文件系统错误*/
	DM_ERROR_ALLOCATE_MEM  = 10106,/*申请内存错误*/
	SERVER_ERROR = 10107,/*系统错误*/
	ERROR_DEL_CLIENT_INFO = 10108,/*客户端信息删除失败*/
	ERROR_GET_SERVICE_LIST = 10109,/*获取服务列表失败*/
	SERVER_MOVED_LOCATION = 10200,/*移动操作失败*/
	CREATE_FILE_ERROR = 10201,/*创建文件失败*/
	FILE_IS_NOT_EXIST = 10202,/*指定文件不存在*/
	ERROR_GET_FILE_LIST= 10203,/*获取文件列表失败*/
	ERROR_MKDIR=  10204,/*创建文件夹失败*/
	ERROR_RENAME = 10205,/*更改文件名失败*/
	ERROR_GET_FILE_ATTR = 10206,/*获取文件属性失败*/
	ERROR_FILE_DOWNLOAD = 10207,/*下载动作失败*/
	ERROR_FILE_UPLOAD_CHECK = 10208,/*上传检查失败*/
	ERROR_FILE_UPLOAD = 10209,/*上传动作失败*/
	ERROR_FILE_DELETE = 10210,/*删除文件夹或文件失败*/
	ERROR_GET_DISK_INFO= 10211,/*获取磁盘信息失败*/
	ERROR_GET_FILE_NULL= 10212,/*文件夹为空*/

	ERROR_CHECKUP_BACKUP_FILE = 10213,/*检查备份文件信息失败*/
	ERROR_BACKUP_FILE = 10214,/*备份文件信息失败*/
    ERROR_FILE_COPY = 10215,/*复制文件或者文件夹失败*/
    ERROR_FILE_OUT_OF_SPACE = 10216,/*当前操作的盘符空间不足*/
    ERROR_FILE_MOVE = 10217,/*移动文件或者文件夹失败*/
    FILE_IS_EXIST = 10218,/*目标文件已存在*/
    ERROR_LOGIN = 10219,/*登录失败，需要重新输入密码*/
    ERROR_PASSWORD_IS_EXIST = 10220,/*the root password is exist*/
	ERROR_PASSWORD_SET = 10221,/*set password error*/
	ERROR_PASSWORD_RESET = 10222,/*reset password error*/
	ERROR_SESSION_PROCESS = 10223,/*should login again*/
	ERROR_PASSWORD_IS_NOT_EXIST = 10224,/*the root password is not exist*/
    
    ERROR_GET_DISK_INFO= 10305,/*获取盘符信息失败*/
    ERROR_GET_FILE_LIST= 10306,/*获取文件列表失败*/
    ERROR_MKDIR=  10307,/*创建文件夹失败*/
    ERROR_RENAME = 10308,/*文件重命名失败*/
    ERROR_IS_NOT_EXIST = 10309,/* the file is not exist*/
    ERROR_FILE_DOWNLOAD = 10311,/*download file error*/
    ERROR_FILE_UPLOAD_CHECK = 10312,/*文件上传检查失败*/
    ERROR_FILE_UPLOAD = 10313,/*文件上传失败*/
    ERROR_FILE_DELETE = 10314,/*文件删除失败*/
    ERROR_FILE_LOCKED = 10315,/*当前文件被锁住*/
    
    DM_ERROR_UCI = 10251,/*UCI ¥ÌŒÛ*/
    DM_ERROR_SHELL_HANDLE = 10252,/*Ω≈±æ‘À–– ß∞‹*/
    DM_ERROR_MCU_IOCTL_ERROR = 10253,/*MCU ß∞‹*/
    DM_ERROR_SOCKET_IOCTL_ERROR = 10254,/*Õ¯¬ÁÕ®–≈ ß∞‹*/
    DM_ERROR_FW_HEADER = 10255,/*πÃº˛–£—È≤ªÕ®π˝£¨Õ∑≤ø“Ï≥£*/
    DM_ERROR_FW_PROJECT_NUMBER = 10256,/*πÃº˛–£—È≤ªÕ®π˝£¨œÓƒø∫≈“Ï≥£*/
    DM_ERROR_FW_MD5_VERIFY = 10257,/*πÃº˛–£—È≤ªÕ®π˝£¨MD5–£—È“Ï≥£*/
	DM_ERROR_POWER_CONDITIONS = 10258,/*…˝º∂–£—È≤ªÕ®π˝£¨µÁ¡øÃıº˛≤ª¬˙◊„*/
	DM_ERROR_FW_FILE_CTL = 10259,/*πÃº˛–£—È≤ªÕ®π˝£¨πÃº˛Œﬁ∑®≤Ÿ◊˜*/
	DM_ERROR_FW_LENGTH = 10260,/*πÃº˛–£—È≤ªÕ®π˝£¨πÃº˛≥§∂»“Ï≥£*/
    DM_ERROR_ADD_NETWORK = 10261,/*add network failed*/
    DM_ERROR_SET_HIDE_ATTR = 10262,/*can not support current file system*/
	DM_ERROR_DATABASE = 10212,/*数据库操作失败*/
	DM_ERROR_UNKNOW = 11000,/*未知错误*/

	DM_ERROR_DB_MKDIR = 12001,/*数据库创建文件夹或文件失败*/
	DM_ERROR_DB_RENAME = 12002,/*数据库重命名文件夹或文件失败*/
	DM_ERROR_DB_DELETE = 12003,/*数据库删除文件夹或文件失败*/
	DM_ERROR_DB_UPLOAD = 12004,
	DM_ERROR_DB_USER_TABLE = 12005,
	DM_ERROR_DB_DEV_TABLE = 12006,
	DM_ERROR_DB_HDISK_TABLE = 12007,
	DM_ERROR_DB_BACKUP_TABLE = 12008,
    DM_ERROR_DB_COPY = 12009,
}ERROR_CODE_T;	 
	 
#ifdef __cplusplus
	 }
#endif
	 
	 
#endif
	 


