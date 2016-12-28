//
//  dmdownload.h
//  DMDOWNLOAD
//
//  Created by Oiver on 16/6/27.
//  Copyright © 2016年 Oliver. All rights reserved.
//

#ifndef dmdownload_h
#define dmdownload_h

#include <stdio.h>
#include "list.h"
#include "my_json.h"

//#define BT_DOWNLOAD_PATH "/SD-disk-1/.exsystem/Downloads"
//#define BT_DOWNLOAD_PATH "SD-disk-1"
#define BT_ROOT_PATH "/tmp"

#define LOCAL_PATH "http://127.0.0.1"

/************************************************************************************
 *                          ENUM                                                    *
 ***********************************************************************************/

typedef enum {
    DMDOWNLOAD_SUCCES = 0,//操作成功
    DMDOWNLOAD_PARA_ERROR = 10000,//参数错误
    DMDOWNLOAD_TIMEOUT = 10001,//任务超时
    DMDOWNLOAD_ADD_TASK_ERROR = 10002,//添加任务失败
    DMDOWNLOAD_QUERY_TASK_ERROR = 10003,//获取任务列表失败
    DMDOWNLOAD_QUERY_DIR_ERROR = 10004,//获取任务目录失败
    DMDOWNLOAD_QUERY_GLOBAL_ERROR = 10005,//获取全局状况失败
    DMDOWNLOAD_START_TASK_ERROR = 10006,//开始任务失败
    DMDOWNLOAD_PAUSE_TASK_ERROR = 10007,//暂停任务失败   
    DMDOWNLOAD_REMOVE_TASK_ERROR = 10008,//删除任务失败
    DMDOWNLOAD_BT_FILE_ERROR = 10009,//download bt fiel error
    DMDOWNLOAD_CTRL_TASK_ERROR = 10010,//control task error
} DMDOWNLOAD_RESULT;

/************************************************************************************
 *                          STRUCT                                                    *
 ***********************************************************************************/

typedef struct{
    struct      dl_list node;
    char        task_id[64];//任务标识符
    char        *url;//地址资源
    char		*uri;
    char		type;//0:��ͨ�ļ�1:BT ���ӣ�2:��ʱ�ļ�
    int         progress;//进度，大于等于0，小于等于100
    int         ctrl;//控制标识位
    int         download_speed;//下载速度KB
    int         status;//0:下载中，1:暂停中，2:等待中，3:已完成，4 下载失败
    char        *dir;//任务下载目录
    char        *download_path;//任务下载路径
    char        *file_name;//下载的文件名
    long long   completed_length;//任务下载已完成的长度
    long long   length;//任务下载总长度
    long long   upload_length;//任务上传的总长度
    int         upload_speed;//上传速度KB
}MDownloadTask;

typedef struct{
    struct dl_list node;
    char   *dir_path;//任务下载目录
    char   *dir_name;//目录名称
    unsigned file_count;//目录下的文件个数
}DownloadDir;

typedef struct{
    struct dl_list node;
    int  download_speed;//整体下载速度KB
    int  upload_speed;//整体上传速度KB
    int  numActive;//当前正在下载的任务个数
    int  numStopped;//当前已停止的任务个数
    int  numWaiting;//当前已暂停的任务个数
}DownloadGlobalStatus;

/**
 *  功能：添加下载任务
 *  权限：开放权限
 *
 *  @param url:输入，资源地址；task_id:输出，任务唯一标识符
 *
 *  @return 0为成功，非0为异常。
 */
typedef DMDOWNLOAD_RESULT (*ARI2AC_ADD_TASK_CB)(char *url,char *task_id);

/**
 *  功能：控制下载任务
 *  权限：开放权限
 *
 *  @param ctrl 输入 0 :下载任务，1:删除任务，2:暂停任务
 *         task_id 输入 任务唯一标识符
 *
 *  @return 0为成功，非0为异常。
 */
typedef DMDOWNLOAD_RESULT (*ARI2AC_CTR_TASK_CB)(int ctrl,char *task_id);

/**
 *  功能：控制所有下载任务
 *  权限：开放权限
 *
 *  @param ctrl 输入 0 :下载任务，1:删除任务，2:暂停任务
 *         task_id 输入 任务唯一标识符
 *
 *  @return 0为成功，非0为异常。
 */
typedef DMDOWNLOAD_RESULT (*ARI2AC_CTR_ALL_TASK_CB)(int ctrl);

/**
 *  func:set upload speed or download speed follow by para
 *
 *  @param dGlobalStatus->download_speed >= 0:need to set download speed,dGlobalStatus->download_speed < 0:no need to set download speed
 *  @return 0:succ,-1:failed
 */
typedef DMDOWNLOAD_RESULT (*ARI2AC_CTR_SPEED_CB)(DownloadGlobalStatus *dGlobalStatus);


/**
 *  功能：获取下载任务列表
 *  权限：开放权限
 *
 *  @param status 0 :下载中的任务，1:暂停中的任务，2:等待中的任务，3:已完成的任务，4下载失败的任务
 *          head 链表首指针
 *  @return 0为成功，非0为异常。
 */
typedef DMDOWNLOAD_RESULT (*ARI2AC_QUERY_TASK_BY_STATUS_CB)(int status,struct dl_list *head);


/**
 *  功能：获取下载任务列表
 *  权限：开放权限
 *
 *  @param status 0 :下载中的任务，1:暂停中的任务，2:等待中的任务，3:已完成的任务，4下载失败的任务
 *          head 链表首指针
 *  @return 0为成功，非0为异常。
 */
typedef DMDOWNLOAD_RESULT (*ARI2AC_QUERY_TASK_BY_GID_CB)(char *gid,struct dl_list *head);

/**
 *  功能：获取下载目录列表
 *  权限：开放权限
 *
 *  @param head 链表首指针
 *
 *  @return 0为成功，非0为异常。
 */
typedef DMDOWNLOAD_RESULT (*ARI2AC_QUERY_DIR_CB)(struct dl_list *head);
/**
 *  功能：获取下载模块全局状况
 *  权限：开放权限
 *
 *  @param dGlobalStatus 输出
 *
 *  @return 0为成功，非0为异常。
 */
typedef DMDOWNLOAD_RESULT (*ARI2AC_QUERY_GLOBAL_CB)(DownloadGlobalStatus *dGlobalStatus);



typedef struct{
    struct dl_list                  d_head;   //list head for result
    struct dl_list                  t_head;
    int                             get_flag;//获取目录｜获取任务列表｜获取全局状态
    int                             set_flag;//设置目录|控制任务列表｜控制全局状态
    int                             ctrl;//0 :下载任务，1:暂停，2:等待，3:删除
    int                             status;
    DownloadGlobalStatus            dGlobalStatus;//下载模块的全局状况
    ARI2AC_ADD_TASK_CB              add_task_cb;//添加下载任务
    ARI2AC_CTR_TASK_CB              ctr_task_cb;//控制下载任务
    ARI2AC_CTR_ALL_TASK_CB          ctr_all_task_cb;//控制下载任务
    ARI2AC_CTR_SPEED_CB          	ctr_speed_cb;//控制下载任务
    ARI2AC_QUERY_TASK_BY_STATUS_CB  query_task_by_status_cb;//获取下载任务列表
    ARI2AC_QUERY_TASK_BY_GID_CB     query_task_by_gid_cb;//获取单个任务详情
    ARI2AC_QUERY_DIR_CB             query_dir_cb;//获取下载目录信息
    ARI2AC_QUERY_GLOBAL_CB          query_global_cb;//获取整体下载状况
}DownloadTaskInfo;

/**
 *  功能：添加下载任务
 *  权限：开放权限
 *
 *  @param url:输入，资源地址；task_id:输出，任务唯一标识符
 *
 *  @return 0为成功，非0为异常。
 */
DMDOWNLOAD_RESULT download_add_task(void *self);

/**
 *  功能：控制下载任务
 *  权限：开放权限
 *
 *  @param dTaskInfo->ctrl 0 :下载任务，1:删除任务，2:暂停任务
 *         dTaskInfo->total_enable 0:控制链表中的任务，1:控制所有任务,任务列表置为空
 *         dTaskInfo->dTaskList 需要控制的任务链表
 *
 *  @return 0为成功，非0为异常。
 */
DMDOWNLOAD_RESULT download_ctr_task(void *self);

/**
 *  功能：控制全部下载任务
 *  权限：开放权限
 *
 *  @param dTaskInfo->ctrl 0 :下载任务，1:暂停，2:等待，3:删除
 *
 *  @return 0为成功，非0为异常。
 */
DMDOWNLOAD_RESULT download_ctr_all_task(void *self);
/**
 *  功能：获取下载任务列表
 *  权限：开放权限
 *
 *  @param status 0 :下载中的任务，1:暂停中的任务，2:等待中的任务，3:已完成的任务，4下载失败的任务
 *
 *  @return 0为成功，非0为异常。
 */
DMDOWNLOAD_RESULT download_query_task(void *self);
/**
 *  功能：获取下载目录列表
 *  权限：开放权限
 *
 *  @param
 *
 *  @return DownloadTaskInfo->dDirList:目录列表，非空为成功，NULL为异常。
 */
DMDOWNLOAD_RESULT download_query_dir(void *self);


/**
 *  功能：获取下载模块全局状况
 *  权限：开放权限
 *
 *  @param
 *
 *  @return 0为成功，非0为异常。
 */
DMDOWNLOAD_RESULT download_query_global(void *self);

/**
 * func:ctrl download or upload speed
 *
 *  @param dTaskInfo->upload_speed,dTaskInfo->download_speed
 *
 *  @return 0:succ ,-1:failed
 */
DMDOWNLOAD_RESULT download_ctr_speed(void *self);

#endif /* dmdownload_h */
