//
//  dmota_json.c
//  DMOTA
//
//  Created by 杨明 on 16/1/15.
//  Copyright © 2016年 杨明. All rights reserved.
//

#include "dmota_json.h"
#include "cJSON.h"
#include "dmFileHelp.h"
#include "my_debug.h"
#include "utils/dmStringUtils.h"

#include <stdlib.h>
#include <string.h>
#include "my_debug.h"

/**
 *  保存json对象到指定的文件中
 *
 *  @param filePath     文件路径
 *  @param pRootJsonObj json对象
 *
 *  @return 0为正常，非零异常
 */
int saveJsonObjToFile(const char *filePath,cJSON * pRootJsonObj)
{
    char * p = cJSON_Print(pRootJsonObj);
//    DMCLOG_D("保存的json数据为：%s.", p);
    int ret = dmfh_stringToFile(filePath, p);
    free(p);
    if(ret == 0)
    {
        //保存到文件成功
        ret = 0;
    }
    else{
        ret = -1;
    }
    return ret;
}
/**
    初始化缓存文件，先清除缓存文件，在返回基础根json对象
 
 - returns: 基础根json对象
 */
cJSON* initCacheFile(const char * filePath)
{
    //1.文件解析出错，如果文件存在则删除该文件
    if (dmfh_isFileExist(filePath)) {
        dmfh_deleteFile(filePath);
    }
    //2.生成新的json对象，并保存数据到文件中
    return makeOTAJsonObjInit();
}

cJSON* makeCJSONWithFWInfo(FWInfo * pFWInfo)
{
    cJSON * obj = cJSON_CreateObject();
    
    cJSON_AddStringToObject(obj, "name", pFWInfo->name);
    cJSON_AddStringToObject(obj, "version", pFWInfo->version);
    cJSON_AddNumberToObject(obj, "versionFlag", pFWInfo->versionFlag);
    
    return obj;
}

cJSON* makeCJSONWithFWFileInfo(FWFileInfo * pFWFileInfo)
{
    cJSON * obj = cJSON_CreateObject();
    
    cJSON_AddItemToObject(obj, "FWInfo", makeCJSONWithFWInfo(pFWFileInfo->pFWInfo));
    cJSON_AddStringToObject(obj, "FilePath", pFWFileInfo->filePath);
    cJSON_AddNumberToObject(obj, "FileSize", pFWFileInfo->fileSize);
    cJSON_AddStringToObject(obj, "Description_zh", pFWFileInfo->description_zh);
    cJSON_AddStringToObject(obj, "Description_en", pFWFileInfo->description_en);
    return obj;
}

cJSON* makeCJSONWithOTAInfo(OTAInfo *pOTAInfo)
{
    cJSON * obj = cJSON_CreateObject();
    cJSON * connectedArray = cJSON_CreateArray();
    cJSON * fwFileInfoArray = cJSON_CreateArray();
    int i = 0;
    while (pOTAInfo->connected[i] != NULL) {
        cJSON_AddItemToArray(connectedArray, makeCJSONWithFWInfo(pOTAInfo->connected[i]));
        i++;
    }
    
    i = 0;
    while(pOTAInfo->downloaded[i] != NULL) {
        cJSON_AddItemToArray(fwFileInfoArray, makeCJSONWithFWFileInfo(pOTAInfo->downloaded[i]));
        i++;
    }
    
    
    cJSON_AddItemToObject(obj, "Connected", connectedArray);
    cJSON_AddItemToObject(obj, "FWFileInfoArray", fwFileInfoArray);
    return obj;
    
}
/**
 *  创建一个初始化fw数据缓存文件的cJson对象
 *
 *  @return 返回json对象
 */
cJSON* makeOTAJsonObjInit()
{
    cJSON * obj = cJSON_CreateObject();
    cJSON * connectedArray = cJSON_CreateArray();
    cJSON * fwFileInfoArray = cJSON_CreateArray();
    
    cJSON_AddItemToObject(obj, "Connected", connectedArray);
    cJSON_AddItemToObject(obj, "FWFileInfoArray", fwFileInfoArray);
    return obj;
    
}



FWInfo * makeFWInfo(char *name,char *version,int versionFlag)
{
    if(name == NULL||version == NULL)
    {
        return NULL;
    }
    FWInfo * pFWInfo = (FWInfo * )malloc(sizeof(FWInfo));
    memset(pFWInfo, 0, sizeof(FWInfo));
    strcpy(pFWInfo->name,name);
    strcpy(pFWInfo->version,version);
    pFWInfo->versionFlag = versionFlag;
    return pFWInfo;
}

FWFileInfo * makeFWFileInfo(char *name,char *version,int versionFlag,char *filePath,long fileSize,char *description_zh,char *description_en)
{
    FWFileInfo * pFWFileInfo = (FWFileInfo * )malloc(sizeof(FWFileInfo));
    pFWFileInfo->pFWInfo = makeFWInfo(name, version,versionFlag);
    if (filePath != NULL) {
        pFWFileInfo->filePath = strdup(filePath);
    }
    if (description_zh != NULL) {
        pFWFileInfo->description_zh = strdup(description_zh);
    }
    if (description_en != NULL) {
        pFWFileInfo->description_en = strdup(description_en);
    }
    pFWFileInfo->fileSize = fileSize;
    return pFWFileInfo;
}

FWDownloadInfo* makeFWDownloadInfo(char *status,char *version,char *fwurl,double fwSize,char *updatecontent_zh,char *updatecontent_en)
{
    FWDownloadInfo * pFWDownloadInfo = (FWDownloadInfo * )malloc(sizeof(FWDownloadInfo));
    memset(pFWDownloadInfo, 0, sizeof(FWDownloadInfo));
    if (status != NULL) {
        pFWDownloadInfo->status = (char*)malloc(strlen(status)+1);
        strcpy(pFWDownloadInfo->status, status);
    }
    if (version != NULL) {
        pFWDownloadInfo->version = (char*)malloc(strlen(version)+1);
        strcpy(pFWDownloadInfo->version, version);
    }
    if (fwurl != NULL) {
        pFWDownloadInfo->fwurl = strdup(fwurl);
//        pFWDownloadInfo->fwurl = (char*)malloc(strlen(fwurl)+1);
//        strcpy(pFWDownloadInfo->fwurl, fwurl);
//        DMCLOG_D("pFWDownloadInfo->fwurl = %s",pFWDownloadInfo->fwurl);
    }
    
    pFWDownloadInfo->fwSize = fwSize;
    
    if (updatecontent_zh != NULL) {
        pFWDownloadInfo->updatecontent_zh = (char*)calloc(1,strlen(updatecontent_zh)+1);
        strcpy(pFWDownloadInfo->updatecontent_zh, updatecontent_zh);
    }
    
    if (updatecontent_en != NULL) {
        pFWDownloadInfo->updatecontent_en = (char*)calloc(1,strlen(updatecontent_en)+1);
        strcpy(pFWDownloadInfo->updatecontent_en, updatecontent_en);
    }
    
    return pFWDownloadInfo;
}

void freeFWFileInfo(FWFileInfo *pFWFileInfo)
{
    if (pFWFileInfo != NULL) {
        if (pFWFileInfo->pFWInfo != NULL) {
            free(pFWFileInfo->pFWInfo);
        }
        if (pFWFileInfo->filePath != NULL) {
            free(pFWFileInfo->filePath);
        }
        if (pFWFileInfo->description_zh != NULL) {
            free(pFWFileInfo->description_zh);
        }
        if (pFWFileInfo->description_en != NULL) {
            free(pFWFileInfo->description_en);
        }
        free(pFWFileInfo);
    }
}

void freeFWDownloadInfo(FWDownloadInfo *pFWDownloadInfo)
{
    if (pFWDownloadInfo != NULL) {
        if (pFWDownloadInfo->status != NULL) {
            free(pFWDownloadInfo->status);
        }
        if (pFWDownloadInfo->version != NULL) {
            free(pFWDownloadInfo->version);
        }
        if (pFWDownloadInfo->fwurl != NULL) {
            free(pFWDownloadInfo->fwurl);
        }
        if (pFWDownloadInfo->updatecontent_zh != NULL) {
            free(pFWDownloadInfo->updatecontent_zh);
        }
        if (pFWDownloadInfo->updatecontent_en != NULL) {
            free(pFWDownloadInfo->updatecontent_en);
        }
        free(pFWDownloadInfo);
    }
}

void freeOTAInfo(OTAInfo *pOTAInfo)
{
    if (NULL != pOTAInfo) {
        int i = 0;
        if(pOTAInfo->connected != NULL)
        {
            i = 0;
            while(pOTAInfo->connected[i] != NULL){
                free(pOTAInfo->connected[i]);
                pOTAInfo->connected[i] = NULL;
                i++;
            }
            free(pOTAInfo->connected);
        }
        
        if(pOTAInfo->downloaded != NULL)
        {
            i= 0;
            while(pOTAInfo->downloaded[i] != NULL)
            {
                free(pOTAInfo->downloaded[i]);
                pOTAInfo->downloaded[i] = NULL;
                i++;
            }
            free(pOTAInfo->downloaded);
        }
        
        free(pOTAInfo);
    }
}


FWInfo * makeFWInfoWithcJSON(cJSON * cjson)
{
    if(NULL != cjson)
    {
        cJSON* cJSON_name = cJSON_GetObjectItem(cjson,"name");
        char* name = cJSON_name->valuestring;
        cJSON* cJSON_version = cJSON_GetObjectItem(cjson,"version");
        char* version = cJSON_version->valuestring;
        cJSON* cJSON_versionFlag = cJSON_GetObjectItem(cjson,"versionFlag");
        int versionFlag = cJSON_versionFlag->valueint;
        return makeFWInfo(name, version,versionFlag);
    }
    return NULL;
}

FWFileInfo * makeFWFileInfoWithcJSON(cJSON * cjson)
{
    if(NULL != cjson)
    {
        cJSON* cJSON_FWInfo = cJSON_GetObjectItem(cjson,"FWInfo");
        FWInfo * pFWInfo = makeFWInfoWithcJSON(cJSON_FWInfo);
        cJSON* cJSON_FilePath = cJSON_GetObjectItem(cjson,"FilePath");
        char* filePath = cJSON_FilePath->valuestring;
        cJSON* cJSON_FileSize = cJSON_GetObjectItem(cjson,"FileSize");
        long fileSize = cJSON_FileSize->valuedouble;
        
        cJSON* cJSON_DescripitonZh = cJSON_GetObjectItem(cjson,"Description_zh");
        char* description_zh = cJSON_DescripitonZh->valuestring;
        
        cJSON* cJSON_DescripitonEn = cJSON_GetObjectItem(cjson,"Description_en");
        char* description_en = cJSON_DescripitonEn->valuestring;
    
        FWFileInfo * pFWFileInfo = makeFWFileInfo(pFWInfo->name, pFWInfo->version, pFWInfo->versionFlag, filePath,fileSize,description_zh,description_en);
        free(pFWInfo);
        return pFWFileInfo;
    }
    return NULL;
}

FWDownloadInfo* makeFWDownloadInfoWithcJSON(cJSON * cjson)
{
    if(NULL != cjson)
    {
        char *status = NULL,*version = NULL,*fwurl = NULL,*updatecontent_zh = NULL,*updatecontent_en = NULL;
        double fwSize = 0;
        cJSON* cJSON_status = cJSON_GetObjectItem(cjson,"status");
        if (NULL != cJSON_status) {
             status = cJSON_status->valuestring;
        }
        
        cJSON* cJSON_version = cJSON_GetObjectItem(cjson,"version");
        if (NULL != cJSON_version) {
            version = cJSON_version->valuestring;
        }
        
        cJSON* cJSON_fwurl = cJSON_GetObjectItem(cjson,"fwurl");
        if (NULL != cJSON_fwurl) {
            fwurl = cJSON_fwurl->valuestring;
            DMCLOG_D("fwurl = %s",fwurl);
        }
        
        cJSON* cJSON_fwSize = cJSON_GetObjectItem(cjson,"fwSize");
        if (NULL != cJSON_fwSize) {
            fwSize = cJSON_fwSize->valuedouble;
        }
        
        cJSON* cJSON_updatecontent_zh = cJSON_GetObjectItem(cjson,"updatecontent_zh");
        if (NULL != cJSON_updatecontent_zh) {
            updatecontent_zh = cJSON_updatecontent_zh->valuestring;
        }
        
        cJSON* cJSON_updatecontent_en = cJSON_GetObjectItem(cjson,"updatecontent_en");
        if (NULL != cJSON_updatecontent_en) {
            updatecontent_en = cJSON_updatecontent_en->valuestring;
        }
        
       
        FWDownloadInfo * pFWDownloadInfo = makeFWDownloadInfo(status, version, fwurl, fwSize, updatecontent_zh,updatecontent_en);
        return pFWDownloadInfo;
    }
    return NULL;
}


int saveFWFileInfoToJsonObj(cJSON * rootJsonObj,FWFileInfo *pFWFile)
{
    
   cJSON* pFWFileInfoArrayJsonObj = cJSON_GetObjectItem(rootJsonObj, "FWFileInfoArray");
    
    
    if (pFWFileInfoArrayJsonObj != NULL) {
        int size = cJSON_GetArraySize(pFWFileInfoArrayJsonObj);
        int i = 0;
        //找到目前下载的array中是否有当前需要插入的文件信息，如果有则要删除，如果没有则直接添加。
        for (i = 0; i < size; i++) {
            cJSON* itemFWFileInfoJsonObj = cJSON_GetArrayItem(pFWFileInfoArrayJsonObj, i);
            if (itemFWFileInfoJsonObj != NULL) {
                FWFileInfo* pFWFileInfo = makeFWFileInfoWithcJSON(itemFWFileInfoJsonObj);
                
                if (!strcmp(pFWFile->pFWInfo->name, pFWFileInfo->pFWInfo->name)) {
                    //需要移除该节点
                    //cJSON_Delete(itemFWFileInfoJsonObj);
                    cJSON_DeleteItemFromArray(pFWFileInfoArrayJsonObj, i);
                    freeFWFileInfo(pFWFileInfo);
                    break;
                }
                freeFWFileInfo(pFWFileInfo);
            }
        }
        //添加新节点
        cJSON* pFWFileJsonObj = makeCJSONWithFWFileInfo(pFWFile);
        cJSON_AddItemToArray(pFWFileInfoArrayJsonObj, pFWFileJsonObj);
        return 0;
    }
    return -1;
    
}

/**
 *  从文件变为json对象
 *
 *  @param filePath 目标文件
 *
 *  @return 返回json对象
 */
cJSON * makeRootJson(const char * filePath)
{
    //读取文件
    char buf[2000];
    size_t len = dmfh_fileToString(filePath, buf, sizeof(buf));
    if (!len){
        DMCLOG_E("读取数据失败");
        return NULL;
    }
    //获取到json对象
    cJSON * pJson = cJSON_Parse(buf);
    if(NULL == pJson)
    {
        // parse faild, return
        DMCLOG_E("json数据解析错误");
        return NULL;
    }
    return pJson;
}

/**
 *  获取最后一次连接过的fw信息
 *
 *  @param filePath 本地缓存信息文件路径
 *  @param pFWInfo  输出信息
 *
 *  @return 0为获取到最后一次连接的fw信息，非0则获取失败
 */
int getFWInfoLastTime(char *filePath,FWInfo* pFWInfo)
{
    
    if((NULL == pFWInfo) || (NULL == filePath))
    {
        return -5;
    }
    cJSON *pRootJson = makeRootJson(filePath);
    
    if (pRootJson != NULL) {
        
        //查询最后一次
        cJSON * pConnected = cJSON_GetObjectItem(pRootJson, "Connected");
        if(NULL == pConnected)
        {
            //get object named "Connected" faild
            DMCLOG_E("没有找到链接过的节点");
            cJSON_Delete(pRootJson);
            return -3;
        }
        
        int arrayLen = cJSON_GetArraySize(pConnected);
        if(arrayLen > 0)
        {
            //返回结果
            cJSON * cJSON_fwInfo = cJSON_GetArrayItem(pConnected, 0);
            if(NULL != cJSON_fwInfo)
            {
                
                FWInfo* ptFWInfo = makeFWInfoWithcJSON(cJSON_fwInfo);
                memset(pFWInfo->name, 0, sizeof(pFWInfo->name));
                strcpy(pFWInfo->name, ptFWInfo->name);
                memset(pFWInfo->version, 0, sizeof(pFWInfo->version));
                strcpy(pFWInfo->version, ptFWInfo->version);
                pFWInfo->versionFlag = ptFWInfo->versionFlag;
                
                free(ptFWInfo);
                cJSON_Delete(pRootJson);
                return 0;
            }
        }
        cJSON_Delete(pRootJson);
    }
    
    return -4;
}

/**
 *  保存固件信息到缓存文件中的第一位
 *
 *  @param filePath 缓存文件路径
 *  @param pFWInfo  固件信息
 *
 *  @return 返回0 正常，非0异常
 */
int saveFWInfoToCacheFile(char *filePath,FWInfo* pFWInfo)
{
    int ret = -1;
    
    if((NULL == pFWInfo) || (NULL == filePath))
    {
        return -5;
    }
    cJSON *pRootJson = makeRootJson(filePath);
    
    if(pRootJson == NULL)
    {
        //没有找到文件，需要重新创建新的结构文件
        pRootJson = initCacheFile(filePath);
    }

        //查询最后一次
        cJSON * pConnectedArrayJsonObj = cJSON_GetObjectItem(pRootJson, "Connected");
        if(NULL == pConnectedArrayJsonObj)
        {
            //get object named "Connected" faild
            DMCLOG_E("没有找到链接过的节点");
            cJSON_Delete(pRootJson);
            return -3;
        }
        
        int arrayLen = cJSON_GetArraySize(pConnectedArrayJsonObj);
        int i = 0;
        //找到目前下载的array中是否有当前需要插入的文件信息，如果有则要删除，如果没有则直接添加。
        for (i = 0; i < arrayLen; i++) {
            cJSON* itemConnectedJsonObj = cJSON_GetArrayItem(pConnectedArrayJsonObj, i);
            if (itemConnectedJsonObj != NULL) {
                FWInfo* pItemFWInfo = makeFWInfoWithcJSON(itemConnectedJsonObj);
                
                if (!strcmp(pItemFWInfo->name, pFWInfo->name)) {
                    //需要移除该节点
                    //cJSON_Delete(itemDownloadJsonObj);
                    cJSON_DeleteItemFromArray(pConnectedArrayJsonObj, i);
                    free(pItemFWInfo);
                    break;
                }
                free(pItemFWInfo);
            }
        }
        //添加到数组的第一个
        
        cJSON *fwInfoJsonObj = makeCJSONWithFWInfo(pFWInfo);
        
        cJSON_InsertItemInArray(pConnectedArrayJsonObj, 0, fwInfoJsonObj);
      
        
        //保存到文件中
        
        
        ret = saveJsonObjToFile(filePath,pRootJson);
        
        
        cJSON_Delete(pRootJson);
        return ret;
}

/**
 *  保存缓存文件信息到文件中，如果有重复则删除原有信息添加新的信息
 *
 *  @param cacheInfoFilePath 缓存文件路径
 *  @param pFWFileInfo       新的固件文件信息
 *
 *  @return 0为完成，非0为异常
 */
int saveFWFileInfoToCacheInfoFile(const char *cacheInfoFilePath ,FWFileInfo *pFWFileInfo)
{
    //   const char * filePath = cacheInfoFilePath;
    if((NULL == pFWFileInfo) || (NULL == cacheInfoFilePath))
    {
        return -5;
    }
    
    cJSON *pRootJson = makeRootJson(cacheInfoFilePath);
    
    if (pRootJson != NULL) {
        //查询最后一次
        cJSON * pFWFileInfoArray = cJSON_GetObjectItem(pRootJson, "FWFileInfoArray");
        if(NULL == pFWFileInfoArray)
        {
            //get object named "Connected" faild
            DMCLOG_D("没有找到下载过的信息过的节点");
            
            cJSON_Delete(pRootJson);
            //需要创建DownLoaded节点并把当前节点
            pRootJson = initCacheFile(cacheInfoFilePath);
        }
        
    }
    else{
        pRootJson = initCacheFile(cacheInfoFilePath);
        
    }
    //开始保存数据到json对象中
    int ret = saveFWFileInfoToJsonObj(pRootJson,pFWFileInfo);
    
    if(ret == 0)
    {
        //保存成功，写入到文件中
        ret = saveJsonObjToFile(cacheInfoFilePath,pRootJson);
    }
    
    cJSON_Delete(pRootJson);
    return ret;
    
}

/**
 *  从本地缓存中判断是否有新固件
 *
 *  @param cacheFilePath    缓存文件路径
 *  @param pFWInfo          固件信息
 *  @param resultFWFileInfo 返回查询结果，如果有则返回，如果没有则为NULL
 *
 *  @return 0:为没有找到  1：本地存在新固件
 */
int hasLatestFromLocalCache(const char* cacheFilePath,FWInfo *pFWInfo,FWFileInfo** resultFWFileInfo)
{
    int ret = 0;
    
    //1.获取本地缓存目录信息文件json对象
    cJSON *pRootJsonObj = makeRootJson(cacheFilePath);
    
    if (pRootJsonObj != NULL) {
        //2.文件不为空。读取缓存文件中的fwfileInfo数组信息
        cJSON * pFWFileInfoArrayJsonObj = cJSON_GetObjectItem(pRootJsonObj, "FWFileInfoArray");
        int i = 0;
        int arraySize = cJSON_GetArraySize(pFWFileInfoArrayJsonObj);
        for (i = 0; i<arraySize; i++) {
            cJSON* itemFWFileInfoJsonObj = cJSON_GetArrayItem(pFWFileInfoArrayJsonObj, i);
            if (itemFWFileInfoJsonObj != NULL) {
                FWFileInfo* pFWFileInfo = makeFWFileInfoWithcJSON(itemFWFileInfoJsonObj);
                
                if ((!strcmp(pFWInfo->name, pFWFileInfo->pFWInfo->name))) {
                    //比较2个的版本号，如果当前下载的版本比传入的版本号高则返回有最新版本切换回当前本地fw的文件信息
                    //todo

                    if(dm_version_cmp(pFWInfo->version,pFWFileInfo->pFWInfo->version) < 0)
                    {
                        //有新版本固件，返回对应的信息
                        if(resultFWFileInfo != NULL)
                        {
                            *resultFWFileInfo = pFWFileInfo;
                            ret = 1;
                            break;
                        }
                        
                    }

                    freeFWFileInfo(pFWFileInfo);
                    break;
                }
                freeFWFileInfo(pFWFileInfo);
            }
        }
        cJSON_Delete(pRootJsonObj);
    }
    
    return ret;
}
