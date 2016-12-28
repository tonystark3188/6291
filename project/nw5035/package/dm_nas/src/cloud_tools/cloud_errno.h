/*
 * =============================================================================
 *
 *       Filename:  cloud_errno.h
 *
 *    Description:  cloud errno define module
 *
 *        Version:  1.0
 *        Created:  2016/10/20
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver
 *   Organization:  longsys
 *
 * =============================================================================
 */
#ifndef _MY_CLOUD_ERRNO_H_
#define _MY_CLOUD_ERRNO_H_
	 
#ifdef __cplusplus
	 extern "C"{
#endif
	 
	
	 
#define SQLITE_FATAL_ERROR(x) (((x)==SQLITE_INTERNAL) || ((x)==SQLITE_NOMEM) || ((x)==SQLITE_INTERRUPT) \
								 || ((x)==SQLITE_IOERR) || ((x)==SQLITE_CORRUPT) || ((x)==SQLITE_FULL) \
								 || ((x) == SQLITE_BUSY) )
	 
	 
	 
#define RET_SUCCESS (0)
	 
#define AIRNAS_ERRNO (0xFFFFFFFF - 1)
#define EINVAL_ARG (0xFFFFFFFF - 2)
#define ENULL_POINT (0xFFFFFFFF - 3)
#define EMALLOC (0xFFFFFFFF - 4)
#define EMATCH (0xFFFFFFFF - 5)
	 //#define EPTHREAD (0xFFFFFFFF - 6)
#define EOUTBOUND (0xFFFFFFFF - 6)
#define ENOMEMBER (0xFFFFFFFF - 7)
#define EEXISTE (0xFFFFFFFF - 8)
#define ESTREAM_EOF (0xFFFFFFFF - 9)
#define EINVAL_OBJ (0xFFFFFFFF - 10)
#define ESHELL_CMD (0xFFFFFFFF - 11)
#define ESERVER_BUSY (0xFFFFFFFF - 12)
	 
#define EJSON (0xFFFFFFFF - 0x10)
#define EJSON_PARSE (EJSON - 1)
#define EJSON_GET (EJSON - 2)
#define EJSON_ADD (EJSON - 3)
#define EJSON_NEW (EJSON - 4)
#define EJSON_VAL (EJSON - 5)
	 
#define EFILE (0xFFFFFFFF - 0x20)
#define EOPEN (EFILE - 1)
#define EREAD (EFILE - 2)
#define EWRITE (EFILE - 3)
#define EIOCTL (EFILE - 4)
#define ESTATFS (EFILE - 5)
#define EWRITEV (EFILE - 6)
#define EREADV (EFILE - 7)
#define ESYMLINK (EFILE - 8)
#define EUNLINK (EFILE - 9)
#define EFILE_RM (EFILE - 10)
#define EFILE_MV (EFILE - 11)
#define EFILE_CP (EFILE - 12)
#define EFILE_STAT (EFILE - 13)
#define EFILE_CREAT (EFILE - 14)
#define EFCNTL   (EFILE - 15)
	 
	 
#define EDIR (0xFFFFFFFF - 0x30)
#define EMKDIR (EDIR - 1)
#define ERMDIR (EDIR - 2)
#define EDIR_EXIST (EDIR - 3)
#define EDIR_RENAME (EDIR - 4)
	 
#define ELOG (0xFFFFFFFF - 0x40)
#define ELOG_INIT (ELOG - 1)
	 
#define EMMPOOL (0xFFFFFFFF - 0x50)
#define EMMPOOL_INIT (EMMPOOL - 1)
#define EMMPOOL_ALLOC (EMMPOOL - 2)
	 
#define ETHREAD (0xFFFFFFFF - 0x60)
#define EMUTEX (ETHREAD - 1)
#define ESEM (ETHREAD - 2)
#define ESEM_QUEUE (ETHREAD - 3)
#define ESEM_POST (ETHREAD - 4)
	 
#define EDISK (0xFFFFFFFF - 0x70)
#define EDISK_NOT_EXIST (EDISK - 1)
#define EDISK_PARTION (EDISK - 2)
#define EDISK_FS (EDISK - 3)
#define EPARTION_NOT_EXIST (EDISK - 4)
#define EPARTION_NOT_MOUNT (EDISK - 5)
#define EPARTION_FS_NOT_EXT (EDISK - 6)
#define EPARTION_MOUNTING (EDISK - 7)
	 
	 
#define ESOCK (0xFFFFFFFF - 0x80)
#define ESOCK_RECV (ESOCK - 1)
#define ESOCK_SEND (ESOCK - 2)
#define ESOCK_SETOPT (ESOCK - 3)
#define ESOCK_BIND (ESOCK - 4)
#define ESOCK_LISTEN (ESOCK - 5)
#define ESOCK_ACCEPT (ESOCK - 6)
#define ESOCK_CONNECT (ESOCK - 7)
#define ESOCK_IOCTL (ESOCK - 8)
#define ESOCK_SETSOCKOPT (ESOCK - 9)
#define ESOCK_ARP (ESOCK - 10)
#define ESOCK_ADDR_TRANSF (ESOCK - 11)
#define ESOCK_GETHOST (ESOCK - 12)
#define ESOCK_GETADDR (ESOCK - 13)
	 
#define EDB                     (0xFFFFFFFF - 0x90)
#define EDB_ID_OVERFLOW         (EDB-1)
#define EDB_RECORD_NOT_EXIST    (EDB-2)
#define EDB_INVALID_OPERATION   (EDB-3)
#define EDB_BACKUP_INIT		    (EDB-4)
#define EDB_NOT_EXIST           (EDB-5)
#define EDB_NOT_MATCH           (EDB-6)
#define EDB_EMPTY               (EDB-7)
#define EDB_BACKUP_DB_NOT_EXIST (EDB-8)
#define EDB_BACKUP_DB_NOT_MATCH (EDB-9)
#define EDB_USER_INVALID        (EDB-10)
#define EDB_EXISTED             (EDB-11)
#define EDB_DEVICE_ID_OVERFLOW  (EDB-12)
#define EDB_INVALID_ID          (EDB-13)
#define EDB_EMPTY_LIST          (EDB-14)
#define EDB_INVALID_VERSION     (EDB-15)
#define EDB_RESTARTING          (EDB-16)
#define EDB_STOP_WORKING        (EDB-17)
#define EDB_RECORD_IS_EXIST     (EDB-18)
#define EDB_FILE_IS_GENENAL     (EDB-19)


	 
#define ECLIENT (0xFFFFFFFF - 0x110)
#define ECLIENT_FMT (ECLIENT - 1)
#define ECLIENT_CMD (ECLIENT - 2)
#define ECLIENT_PERMSION (ECLIENT - 3)
#define ECLIENT_PARAM (ECLIENT - 4)
	 
#define EMESSAGE (0xFFFFFFFF - 0x120)
#define EMESSAGE_NEW (EMESSAGE - 1)
#define EMESSAGE_SEND (EMESSAGE - 2)
#define EMESSAGE_RECV (EMESSAGE - 3)
#define EMESSAGE_FREE (EMESSAGE - 4)
	 
#define  EUCI  (0xFFFFFFFF - 0x130)
	 
#define EJPEG (0xFFFFFFFF - 0x140)
#define EJPEG_DEC (EJPEG - 1)
	 
#define ERT (0xFFFFFFFF - 0x150)
#define ERT_MSG_TYPE  (ERT - 1)
#define ERT_MSG_TRUNC (ERT - 2)
#define ERT_DATA_REMNANT (ERT - 3)
#define ERT_COND_NOT_MATCH (ERT - 4)
	 
#define EEVENT (0xFFFFFFFF - 0x160)
#define EUEVENT (EEVENT - 1)
#define EUEVENT_SUBSYS_NOT_DEFINE (EUEVENT - 1)
#define EINOTIFY (EEVENT - 3)
#define EINOTIFY_INIT (EINOTIFY - 1)
#define EINOTIFY_ADD  (EINOTIFY - 2)
#define EINOTIFY_DEL  (EINOTIFY - 3)
	 
#define EHDISK (0xFFFFFFFF - 0x180)
#define EHDISK_NOT_EXIST (EHDISK - 1)
	 
#define EHTTP (0xFFFFFFFF - 0x190)
#define EHTTP_REQUST_FAILED (EHTTP - 1)
#define EHTTP_HEAD_PARSE    (EHTTP - 2)
#define EHTTP_UNEXPECTED_STATUS (EHTTP - 3)
	 
#define EUPGRADE           (0xFFFFFFFF - 0x1A0)
#define EUPGRADE_STATUS    (EUPGRADE - 1)
#define EUPGRADE_BUSY      (EUPGRADE - 2)
	 
#define EPUB_SERV (0xFFFFFFFF - 0x1B0)
	 //#define EPUB_SER
#define EEPOLL (0xFFFFFFFF - 0x1C0)
#define EEPOLL_CREATE (EEPOLL - 1)
#define EEPOLL_CTL   (EEPOLL - 2)
#define EEPOLL_WAIT   (EEPOLL - 3)


typedef enum ERROR_CODE
{
	/**
	 * 成功
	 */
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
	ERROR_PASSWORD_OLD = 10222,/*old password error*/
	ERROR_SESSION_PROCESS = 10223,/*should login again*/
	ERROR_PASSWORD_IS_NOT_EXIST = 10224,/*the root password is not exist*/
	ERROR_FILE_SEARCH = 10225,/* 文件模糊搜索失败 */
	ERROR_PASSWORD_RESET = 10226,/*reset password error*/
	ERROR_FILE_LOCKED = 10315,/*当前文件被锁住*/
	
	DM_ERROR_UCI = 10251,/*UCI 错误*/
	DM_ERROR_SHELL_HANDLE = 10252,/*脚本运行失败*/
	DM_ERROR_MCU_IOCTL_ERROR = 10253,/*MCU失败*/
	DM_ERROR_SOCKET_IOCTL_ERROR = 10254,/*网络通信失败*/
	DM_ERROR_FW_HEADER = 10255,/*固件校验不通过，头部异常*/
	DM_ERROR_FW_PROJECT_NUMBER = 10256,/*固件校验不通过，项目号异常*/
	DM_ERROR_FW_MD5_VERIFY = 10257,/*固件校验不通过，MD5校验异常*/
	DM_ERROR_POWER_CONDITIONS = 10258,/*升级校验不通过，电量条件不满足*/
	DM_ERROR_FW_FILE_CTL = 10259,/*固件校验不通过，固件无法操作*/
	DM_ERROR_FW_LENGTH = 10260,/*固件校验不通过，固件长度异常*/
	DM_ERROR_ADD_NETWORK_NO_FIND = 10261,/*添加网络扫描失败*/
	DM_ERROR_SET_HIDE_ATTR = 10262,/*set attr error or can not support*/

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
      /**
       * 失败
       */
      FAIL,

      /**
       * 没有授权(没有登录)
       */
      NOT_AUTHORIZATION = 13000,
      /**
       * 拒绝访问(用户没有权限访问)
       */
      PERMISSION_DENIED = 13001,
      /**
       * 用户名不存在
       */
      USERNAME_NOT_FOUND= 13002,
      /**
       * 密码错误
       */
      PASSWORD_ERROR	= 13003
}ERROR_CODE_VFS;	 
	 
#ifdef __cplusplus
	 }
#endif
	 
	 
#endif
	 


