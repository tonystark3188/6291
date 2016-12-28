//
//  dmota_json.h
//  DMOTA
//
//  Created by 杨明 on 16/1/15.
//  Copyright © 2016年 杨明. All rights reserved.
//

#ifndef dmota_json_h
#define dmota_json_h

#include <stdio.h>
#include "cJSON.h"


/**
 *  固件信息
 */
typedef struct{
    char name[32];
    char version[32];
    int versionFlag;//1:test flag,0:format flag
} FWInfo;

/**
 *  固件文件信息
 */
typedef struct{
    FWInfo *pFWInfo;
    /**
     *  相对路径及文件名
     */
    char *filePath;
    /**
     *  文件大小
     */
    long fileSize;
    /**
     *  下载文件的中文描述信息
     */
    char *description_zh;
    /**
     *  下载文件的英文描述信息
     */
    char *description_en;
} FWFileInfo;

/**
 *  固件下载信息
 */
typedef struct{
    /**
     *  返回值状态
     */
    char *status;
    /**
     *  版本信息
     */
    char *version;
    /**
     *  下载的url地址
     */
    char *fwurl;
    /**
     *  文件大小
     */
    long fwSize;
    /**
     *  下载文件的中文描述信息
     */
    char *updatecontent_zh;
    /**
     *  下载文件的英文描述信息
     */
    char *updatecontent_en;
    /*
     *   1:测试版本
     *   0:正式版本
     */
    int versionFlag;
} FWDownloadInfo;

typedef struct{
    /**
     *  链接过的设备信息
     */
    FWInfo ** connected;
    /**
     *  下载过的固件信息
     */
    FWFileInfo ** downloaded;
} OTAInfo;


/**
 *  创建一个初始化fw数据缓存文件的cJson对象
 *
 *  @return <#return value description#>
 */
extern cJSON* makeOTAJsonObjInit();

extern cJSON* makeCJSONWithFWInfo(FWInfo * pFWInfo);
extern cJSON* makeCJSONWithFWFileInfo(FWFileInfo * pFWFileInfo);
extern cJSON* makeCJSONWithOTAInfo(OTAInfo *pOTAInfo);

extern FWInfo * makeFWInfo(char *name,char *version,int verisonFlag);
extern FWFileInfo * makeFWFileInfo(char *name,char *version,int verisonFlag,char *filePath,long fileSize,char *description_zh,char *description_en);

extern FWInfo * makeFWInfoWithcJSON(cJSON * cjson);
extern FWDownloadInfo* makeFWDownloadInfoWithcJSON(cJSON * cjson);

extern void freeFWFileInfo(FWFileInfo *pFWFileInfo);
extern void freeFWDownloadInfo(FWDownloadInfo *pFWDownloadInfo);
extern void freeOTAInfo(OTAInfo *pOTAInfo);


/**
 *  获取最后一次连接过的fw信息
 *
 *  @param filePath 本地缓存信息文件路径
 *  @param pFWInfo  输出信息
 *
 *  @return 0为获取到最后一次连接的fw信息，非0则获取失败
 */
extern int getFWInfoLastTime(char *filePath,FWInfo* pFWInfo);
/**
 *  保存缓存文件信息到文件中，如果有重复则删除原有信息添加新的信息
 *
 *  @param cacheInfoFilePath 缓存文件路径
 *  @param pFWFileInfo       新的固件文件信息
 *
 *  @return 0为完成，非0为异常
 */
extern int saveFWFileInfoToCacheInfoFile(const char *cacheInfoFilePath ,FWFileInfo *pFWFileInfo);
/**
 *  保存固件信息到缓存文件中的第一位
 *
 *  @param filePath 缓存文件路径
 *  @param pFWInfo  固件信息
 *
 *  @return 返回0 正常，非0异常
 */
extern int saveFWInfoToCacheFile(char *filePath,FWInfo* pFWInfo);
/**
 *  从本地缓存中判断是否有新固件
 *
 *  @param cacheFilePath    缓存文件路径
 *  @param pFWInfo          固件信息
 *  @param resultFWFileInfo 返回查询结果，如果有则返回，如果没有则为NULL
 *
 *  @return 0为没有，非0为有
 */
extern int hasLatestFromLocalCache(const char* cacheFilePath,FWInfo *pFWInfo,FWFileInfo** resultFWFileInfo);


#endif /* dmota_json_h */
