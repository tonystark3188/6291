//
//  dmota.c
//  DMOTA
//
//  Created by 杨明 on 16/1/14.
//  Copyright © 2016年 杨明. All rights reserved.
//

#include "dmota.h"
#include "my_debug.h"
#include "cJSON.h"
#include "dmFileHelp.h"
#include "dmota_json.h"
#include "dmNetutil.h"
#include "dmota_net.h"
#include "dmota_constant.h"
#include "utils/dmStringUtils.h"
#include <stdlib.h>
#include <string.h>
#include "my_debug.h"

//
//
//char * makeJson()
//{
//    cJSON * pJsonRoot = NULL;
//    
//    pJsonRoot = cJSON_CreateObject();
//    if(NULL == pJsonRoot)
//    {
//        //error happend here
//        return NULL;
//    }
//    cJSON_AddStringToObject(pJsonRoot, "hello", "hello world");
//    cJSON_AddNumberToObject(pJsonRoot, "number", 10010);
//    cJSON_AddBoolToObject(pJsonRoot, "bool", 1);
//    cJSON * pSubJson = NULL;
//    pSubJson = cJSON_CreateObject();
//    if(NULL == pSubJson)
//    {
//        // create object faild, exit
//        cJSON_Delete(pJsonRoot);
//        return NULL;
//    }
//    cJSON_AddStringToObject(pSubJson, "subjsonobj", "a sub json string");
//    cJSON_AddItemToObject(pJsonRoot, "subobj", pSubJson);
//    
//    char * p = cJSON_Print(pJsonRoot);
//    // else use :
//    // char * p = cJSON_PrintUnformatted(pJsonRoot);
//    if(NULL == p)
//    {
//        //convert json list to string faild, exit
//        //because sub json pSubJson han been add to pJsonRoot, so just delete pJsonRoot, if you also delete pSubJson, it will coredump, and error is : double free
//        cJSON_Delete(pJsonRoot);
//        return NULL;
//    }
//    //free(p);
//    
//    cJSON_Delete(pJsonRoot);
//    
//    return p;
//}
//
//void parseJson(char * pMsg)
//{
//    if(NULL == pMsg)
//    {
//        return;
//    }
//    cJSON * pJson = cJSON_Parse(pMsg);
//    if(NULL == pJson)
//    {
//        // parse faild, return
//        return ;
//    }
//    
//    // get string from json
//    cJSON * pSub = cJSON_GetObjectItem(pJson, "hello");
//    if(NULL == pSub)
//    {
//        //get object named "hello" faild
//    }
//    printf("obj_1 : %s\n", pSub->valuestring);
//    
//    // get number from json
//    pSub = cJSON_GetObjectItem(pJson, "number");
//    if(NULL == pSub)
//    {
//        //get number from json faild
//    }
//    printf("obj_2 : %d\n", pSub->valueint);
//    
//    // get bool from json
//    pSub = cJSON_GetObjectItem(pJson, "bool");
//    if(NULL == pSub)
//    {
//        // get bool from json faild
//    }
//    printf("obj_3 : %d\n", pSub->valueint);
//    
//    // get sub object
//    pSub = cJSON_GetObjectItem(pJson, "subobj");
//    if(NULL == pSub)
//    {
//        // get sub object faild
//    }
//    cJSON * pSubSub = cJSON_GetObjectItem(pSub, "subjsonobj");
//    if(NULL == pSubSub)
//    {
//        // get object from subject object faild
//    }
//    printf("sub_obj_1 : %s\n", pSubSub->valuestring);
//    
//    cJSON_Delete(pJson);
//}
//
//
//void parseJson2(char * pMsg)
//{
//    if(NULL == pMsg)
//    {
//        return;
//    }
//    cJSON * pJson = cJSON_Parse(pMsg);
//    if(NULL == pJson)
//    {
//        // parse faild, return
//        return ;
//    }
//    
//    cJSON * pConnected = cJSON_GetObjectItem(pJson, "Connected");
//    if(NULL == pConnected)
//    {
//        //get object named "Connected" faild
//    }
//    
//    int arrayLen = cJSON_GetArraySize(pConnected);
//    
//    DMCLOG_D("arrayLen = %d.", arrayLen);
//    
//    
//    
//    cJSON_Delete(pJson);
//}
//
//
//
//#define EXAMPLESIZE 3
//
//
//
//
//
//char *makeJson2()
//{
//    cJSON * pJsonRoot = NULL;
//    
//    
//    OTAInfo *pOTAInfo = (OTAInfo *)malloc(sizeof(OTAInfo));
//    
//    FWInfo ** connected = (FWInfo **)malloc(sizeof(FWInfo *) *EXAMPLESIZE);
//    FWFileInfo **downloaded= (FWFileInfo **)malloc(sizeof(FWFileInfo *) *EXAMPLESIZE);;
//    memset(connected, 0, sizeof(FWInfo *) *EXAMPLESIZE);
//    memset(downloaded, 0, sizeof(FWFileInfo *) *EXAMPLESIZE);
//    
//    pOTAInfo->connected = connected;
//    pOTAInfo->downloaded = downloaded;
//    
//    pOTAInfo->connected[0] = makeFWInfo("A100(M03)", "1.0.0.1");
//    pOTAInfo->connected[1] = makeFWInfo("A62(M03)", "1.0.3.4");
//    
//    pOTAInfo->downloaded[0] = makeFWFileInfo("A100(M03)", "1.0.0.2","A100(M03).bin");
//    pOTAInfo->downloaded[1] = makeFWFileInfo("A62(M03)", "1.0.3.5","A62(M03).bin");
//
//    
//    pJsonRoot = makeCJSONWithOTAInfo(pOTAInfo);
//    
//    if(NULL == pJsonRoot)
//    {
//        //error happend here
//        freeOTAInfo(pOTAInfo);
//        
//        return NULL;
//    }
//
//    
//    char * p = cJSON_Print(pJsonRoot);
//    // else use :
//    // char * p = cJSON_PrintUnformatted(pJsonRoot);
//    if(NULL == p)
//    {
//        //convert json list to string faild, exit
//        //because sub json pSubJson han been add to pJsonRoot, so just delete pJsonRoot, if you also delete pSubJson, it will coredump, and error is : double free
//        cJSON_Delete(pJsonRoot);
//        freeOTAInfo(pOTAInfo);
//        return NULL;
//    }
//    //free(p);
//    
//    cJSON_Delete(pJsonRoot);
//    freeOTAInfo(pOTAInfo);
//    
//    return p;
//}


/************************************/


FWQueryResult* makeFWQueryResult(char *name,char *version,char *uri,long fileSize,char *description_zh,char *description_en)
{
    FWQueryResult *pFWQueryResult = (FWQueryResult *)malloc(sizeof(FWQueryResult));
    memset(pFWQueryResult, 0, sizeof(FWQueryResult));
    
    if(name != NULL)
    {
        pFWQueryResult->name = strdup(name);
    }
    if(name != NULL)
    {
        pFWQueryResult->version = strdup(version);
    }
    if(name != NULL)
    {
        pFWQueryResult->uri = strdup(uri);
    }

    pFWQueryResult->fileSize = fileSize;

    if(name != NULL)
    {
        pFWQueryResult->description_zh = strdup(description_zh);
    }
    
    if(name != NULL&&description_en != NULL)
    {
        pFWQueryResult->description_en = strdup(description_en);
    }
    
    return pFWQueryResult;
}

void freeFWQueryResult(FWQueryResult *pFWQueryResult)
{
    if(pFWQueryResult != NULL)
    {
        if (pFWQueryResult->name) {
            free(pFWQueryResult->name);
        }
        if (pFWQueryResult->version) {
            free(pFWQueryResult->version);
        }
        if (pFWQueryResult->uri) {
            free(pFWQueryResult->uri);
        }
        if (pFWQueryResult->description_zh) {
            free(pFWQueryResult->description_zh);
        }
        if (pFWQueryResult->description_en) {
            free(pFWQueryResult->description_en);
        }
        free(pFWQueryResult);
    }
}

/**
 *  设置ota的缓存目录
 *
 *  @param cacheDir 缓存目录绝对路径
 */
void dmota_setLocalCacheFileDir(char *cacheDir)
{
    setLocalCacheFileDir(cacheDir);
}
/**
 *  保存固件信息到缓存文件中的第一位
 *
 *  @param filePath 缓存文件路径
 *  @param pFWInfo  固件信息
 *
 *  @return 返回0 正常，非0异常
 */
int dmota_saveFWInfoToCacheFile(FWInfo* pFWInfo)
{
    return saveFWInfoToCacheFile(getLocalCacheInfoFilePath(), pFWInfo);
}

/**
 *  获取最后一次连接过的fw信息
 *
 *  @param filePath 本地缓存信息文件路径
 *  @param pFWInfo  输出信息
 *
 *  @return 0为获取到最后一次连接的fw信息，非0则获取失败
 */
int dmota_getFWInfoLastTime(FWInfo* pFWInfo)
{
    return getFWInfoLastTime(getLocalCacheInfoFilePath(), pFWInfo);
}

/**
 *  OTA与服务器比较是否是最新版本
 *
 *  @param pFWInfo 需要比较的fw信息,
 *  resultNewFWInfo 如果有最新版本返回当前最新版本信息
 *
 *  @return YES_DM存在最新版本，NO_DM没有获取到最新版本
 */
int hasLatestFromServer(FWInfo *pFWInfo,FWDownloadInfo** resultFWDownloadInfo)
{
    FWDownloadInfo* pFWDownloadInfo = NULL;
    int ret = net_hasLatestFromServer(pFWInfo,&pFWDownloadInfo);
    if (ret) {
        
        if (resultFWDownloadInfo) {
            *resultFWDownloadInfo = pFWDownloadInfo;
//            memset(resultNewFWInfo, 0, sizeof(FWInfo));
//            strcpy(resultNewFWInfo->name, pFWInfo->name);
//            strcpy(resultNewFWInfo->version, resultFWDownloadInfo->version);
            return ret;
        }
        
        if (pFWDownloadInfo) {
            freeFWDownloadInfo(pFWDownloadInfo);
        }
    }
    
    return ret;
}

/**
 *  从服务器下载最新固件
 *
 *  @param pFWInfo          当前需要下载的固件信息
 *  @param resultFWFileInfo 返回最新的固件的基本信息，包含固件名称，固件版本，已下载完成的本地路径
 *  @param progress_chanage 进度回调信息，包含总文件大小，当前进度。一般返回0，如果返回非零则中断下载任务。
 *
 *  @return 0为正常，非0异常
 */
int dmota_downloadFormServer(FWInfo *pFWInfo,FWFileInfo ** resultFWFileInfo,int (*progress_chanage)(long progress,long total))
{
    int ret = -1;
    //下载最新固件
    FWFileInfo * pFWFileInfo = NULL;
    ret = net_downloadFormServer(pFWInfo,&pFWFileInfo,progress_chanage);
    
    if ((!ret) && (pFWFileInfo != NULL)) {
        //DMCLOG_D("下载完成");
        //保存最新的下载数据
        
        ret = saveFWFileInfoToCacheInfoFile(getLocalCacheInfoFilePath(),pFWFileInfo);
        if (ret == 0) {
            //保存成功
            DMCLOG_D("the latest fw has stored in cache path\n");
            *resultFWFileInfo = pFWFileInfo;
            return 0;
        }
        else{
            DMCLOG_E("数据保存失败");
        }
        freeFWFileInfo(pFWFileInfo);
    }
    else
    {
        DMCLOG_E("下载失败");
    }
    return -1;
}


/**
 *  获取当前固件是否有最新版本固件
 *  1.先后去网络。
 *  2.如果没有，再查找本地。
 *
 *  @param pFWInfo fw基本信息
 *  @param resultNewFWInfo 返回最新的版本信息
 *
 *  @return 0为没有，非0为存在
 */
DMOTA_QUERY_RESULT dmota_hasLatest(FWInfo *pFWInfo,FWQueryResult **resultFWQueryResult)
{
    //判断是否连接互联网
    FWDownloadInfo *pFWDownloadInfo = NULL;
    int ret = hasLatestFromServer(pFWInfo,&pFWDownloadInfo);
    
    if (ret) {
        //网络已经找到最新版本
        //再到本地缓存区域查找是否有最新版本，如果有本地最新版本，切本地版本和网络版本一至则返回本地版本信息，如果本地版本低于网服务器版本则返回服务器版本信息。
        
        FWFileInfo* resultFWFileInfo = NULL;
        ret = hasLatestFromLocalCache(getLocalCacheInfoFilePath(),pFWInfo,&resultFWFileInfo);
        
        if (ret) {
            
            if(!dm_version_cmp(resultFWFileInfo->pFWInfo->version, pFWDownloadInfo->version))
            {
                //本地已经有服务器最新版本，则直接返回本地路径即可
                if (resultFWQueryResult) {
                    *resultFWQueryResult = makeFWQueryResult(resultFWFileInfo->pFWInfo->name, resultFWFileInfo->pFWInfo->version,resultFWFileInfo->filePath,resultFWFileInfo->fileSize,resultFWFileInfo->description_zh,resultFWFileInfo->description_en);
                }
                
                if (resultFWFileInfo) {
                    freeFWFileInfo(resultFWFileInfo);
                }
                
                if(pFWDownloadInfo)
                    freeFWDownloadInfo(pFWDownloadInfo);
                return DMOTA_QUERY_RESULT_LOCAL;
            }
            
        }
        
        //返回网络fw版本信息
        {
            if (NULL != resultFWQueryResult) {
                *resultFWQueryResult = makeFWQueryResult(pFWInfo->name, pFWDownloadInfo->version,pFWDownloadInfo->fwurl,pFWDownloadInfo->fwSize,pFWDownloadInfo->updatecontent_zh,pFWDownloadInfo->updatecontent_en);
                
            }
            
            if(pFWDownloadInfo)
            freeFWDownloadInfo(pFWDownloadInfo);
            //服务器有新版本固件
            return DMOTA_QUERY_RESULT_NET;
        }

    }
    else{
        DMCLOG_D("服务器没有获取到最新版本");
        FWFileInfo* resultFWFileInfo = NULL;
       ret = hasLatestFromLocalCache(getLocalCacheInfoFilePath(),pFWInfo,&resultFWFileInfo);
        if (ret) {
            if (resultFWQueryResult) {
                *resultFWQueryResult = makeFWQueryResult(resultFWFileInfo->pFWInfo->name, resultFWFileInfo->pFWInfo->version,resultFWFileInfo->filePath,resultFWFileInfo->fileSize,resultFWFileInfo->description_zh,resultFWFileInfo->description_en);
            }
            
            if (resultFWFileInfo) {
                freeFWFileInfo(resultFWFileInfo);
            }
            return DMOTA_QUERY_RESULT_LOCAL;
        }
        
    }
    return DMOTA_QUERY_RESULT_NONE;
    
}

int progress_chanage(long progress,long total)
{
//    DMCLOG_D("progress = %ld,total = %ld", progress, total);
    
    return 0;
}





void dmota_test()
{
//    char *filePath = "/Users/glenn/Desktop/json.txt";
    //printf("dmota_test\n");
    DMCLOG_I("dmota_test");
    int ret = -1;
    
//    char* pJsonStr = makeJson2();
//    
//    DMCLOG_D("pJsonStr = %s.", pJsonStr);
//    
//    int ret = dmfh_stringToFile(filePath, pJsonStr);
//    
//    if(ret)
//    {
//        DMCLOG_E("字符串保存失败");
//    }
//    else
//    {
//        DMCLOG_D("字符串已保存到文件：%s",filePath);
//    }
    
//    
//    char buf[2000];
//    
//    size_t len = dmfh_fileToString(filePath, buf, sizeof(buf));
//    
//    if (len) {
//        DMCLOG_D("读取到的数据为:%s", buf);
//        parseJson2(buf);
//    }
//    else{
//        DMCLOG_E("读取数据失败");
//    }
    
    
//    FWInfo* pFWInfo = makeFWInfo("A999(M.03)", "1.0.00.1");
//    ret = dmota_saveFWInfoToCacheFile(pFWInfo);
//    if(!ret)
//    {
//        DMCLOG_D("保存成功");
//        FWInfo fwInfo;
//        memset(&fwInfo, 0, sizeof(FWInfo));
//        
//        ret = dmota_getFWInfoLastTime(&fwInfo);
//        
//        if (!ret) {
//            DMCLOG_D("最后一次连接的fw信息为：name = %s.version = %s",fwInfo.name,fwInfo.version);
//            FWDownloadInfo* resultFWDownloadInfo = NULL;
//            ret = net_hasLatestFromServer(&fwInfo,&resultFWDownloadInfo);
//        
//            if (!ret) {
//                DMCLOG_D("有新版本固件");
//                if(resultFWDownloadInfo != NULL)
//                {
//                   DMCLOG_D("新版本固件信息：version = %s,fwurl = %s.filesize = %ld.content = %s.",resultFWDownloadInfo->version,resultFWDownloadInfo->fwurl,resultFWDownloadInfo->fwSize, resultFWDownloadInfo->updatecontent);
//                }
//                //下载最新固件
//                FWFileInfo * pFWFileInfo = NULL;
//        //        ret = dmota_net_downloadFormServer(pFWInfo,&pFWFileInfo,progress_chanage);
//                ret = dmota_downloadFormServer(&fwInfo,&pFWFileInfo,progress_chanage);
//            }
//            else{
//                DMCLOG_D("没有新版本固件");
//            }
//            
//        }
//        else{
//            DMCLOG_E("获取最后一次版本信息错误");
//        }
//        
//    }
//    else{
//        DMCLOG_E("保存失败");
//    }
//    free(pFWInfo);
    
    
    
    
    
//    pFWInfo = makeFWInfo("A999(M.03)", "1.0.00.1");
//    
//    
//    ret = dmota_net_isLatestFromServer(pFWInfo,NULL);
//    
//    if (!ret) {
//        DMCLOG_D("有新版本固件");
//        //下载最新固件
//        FWFileInfo * pFWFileInfo = NULL;
////        ret = dmota_net_downloadFormServer(pFWInfo,&pFWFileInfo,progress_chanage);
//        ret = dmota_downloadFormServer(pFWInfo,&pFWFileInfo,progress_chanage);
//    }
//    else{
//        DMCLOG_D("没有新版本固件");
//    }
//    
//    free(pFWInfo);
    
    
//    ret = dm_version_cmp("1.0.1","1.0.1.13");
//    DMCLOG_D("ret = %d.", ret);
    
    
    FWInfo* pFWInfo = makeFWInfo("A999(M.03)", "1.0.00.1",1);
    
    FWQueryResult *resultFWQueryResult = NULL;
    ret = dmota_hasLatest(pFWInfo,&resultFWQueryResult);
    if (ret) {
        DMCLOG_D("有新版本固件");
        //下载最新固件
        if(resultFWQueryResult)
        {
//            DMCLOG_D("当前信息 queryResult = %d,name:%s ,version = %s,filesize = %ld,uri = %s,description = %s.",ret,resultFWQueryResult->name,resultFWQueryResult->version,resultFWQueryResult->fileSize,resultFWQueryResult->uri,resultFWQueryResult->description_zh,resultFWQueryResult->description_en);
            
            freeFWQueryResult(resultFWQueryResult);
        }
        
    }
    else{
        DMCLOG_D("没有新版本固件");
    }
    
    free(pFWInfo);
}



