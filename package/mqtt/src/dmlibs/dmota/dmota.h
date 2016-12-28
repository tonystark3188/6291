//
//  dmota.h
//  DMOTA
//
//  Created by 杨明 on 16/1/14.
//  Copyright © 2016年 杨明. All rights reserved.
//

#ifndef dmota_h
#define dmota_h

#include <stdio.h>
#include "dmota_json.h"



typedef enum {
    /**
     *  没有最新版本
     */
    DMOTA_QUERY_RESULT_NONE = 0,
    /**
     *  有最新版本，文件在本地
     */
    DMOTA_QUERY_RESULT_LOCAL,
    /**
     *  有最新版本，文件在服务器
     */
    DMOTA_QUERY_RESULT_NET
} DMOTA_QUERY_RESULT;

typedef struct {
    /**
     *  版本的基本信息，包含项目号和版本号
     */
    char *name;
    /**
     *  版本信息
     */
    char *version;
    /**
     *  当前文件资源的绝对地址
     */
    char *uri;
    /**
     *  文件大小
     */
    long fileSize;
    /**
     *  当前文件的中文描述信息
     */
    char *description_zh;
    /**
     *  当前文件的英文描述信息
     */
    char *description_en;
} FWQueryResult;

extern void dmota_test();

/**
 *  设置ota的缓存目录
 *
 *  @param cacheDir 缓存目录绝对路径
 */
extern void dmota_setLocalCacheFileDir(char *cacheDir);
/**
 *  保存固件信息到缓存文件中的第一位
 *
 *  @param filePath 缓存文件路径
 *  @param pFWInfo  固件信息
 *
 *  @return 返回0 正常，非0异常
 */
extern int dmota_saveFWInfoToCacheFile(FWInfo* pFWInfo);
/**
 *  获取最后一次连接过的fw信息
 *
 *  @param filePath 本地缓存信息文件路径
 *  @param pFWInfo  输出信息
 *
 *  @return 0为获取到最后一次连接的fw信息，非0则获取失败
 */
extern int dmota_getFWInfoLastTime(FWInfo* pFWInfo);
/**
 *  OTA与服务器比较是否是最新版本（推荐在只连接到互联网的情况下调用）
 *
 *  @param pFWInfo 需要比较的fw信息,
 *  resultFWDownloadInfo 如果有最新版本返回当前最新版本信息
 *
 *  @return YES_DM存在最新版本，NO_DM没有获取到最新版本
 */
//extern int hasLatestFromServer(FWInfo *pFWInfo,FWInfo* resultNewFWInfo);
/**
 *  获取当前固件是否有最新版本固件 (推荐在设备已经连接的情况下调用)
 *  1.先后去网络。
 *  2.如果没有，再查找本地。
 *
 *  @param pFWInfo fw基本信息
 *  @param resultNewFWInfo 返回最新的版本信息
 *
 *  @return 0为没有，非0为存在
 */
extern DMOTA_QUERY_RESULT dmota_hasLatest(FWInfo *pFWInfo,FWQueryResult **resultFWQueryResult);

/**
 *  从服务器下载最新固件
 *
 *  @param pFWInfo          当前需要下载的固件信息
 *  @param resultFWFileInfo 返回最新的固件的基本信息，包含固件名称，固件版本，已下载完成的本地路径
 *  @param progress_chanage 进度回调信息，包含总文件大小，当前进度。一般返回0，如果返回非零则中断下载任务。
 *
 *  @return 0为正常，非0异常
 */
extern int dmota_downloadFormServer(FWInfo *pFWInfo,FWFileInfo ** resultFWFileInfo,int (*progress_chanage)(long progress,long total));



extern void freeFWQueryResult(FWQueryResult *pFWQueryResult);


#endif /* dmota_h */
