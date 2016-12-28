//
//  dmota_net.c
//  DMOTA
//
//  Created by 杨明 on 16/1/15.
//  Copyright © 2016年 杨明. All rights reserved.
//

#include "dmota_net.h"
#include "my_debug.h"
#include "dmota.h"
#include "utils/dmFileHelp.h"
#include "utils/dmNetutil.h"
#include "dmota_constant.h"

#include <stdlib.h>
#include <string.h>



/**
 *  获取本地缓存文件全路径
 *
 *  @param pFWInfo         fw基本信息
 *  @param pFWDownloadInfo 下载文件基本信息
 *  @param buf             全路径buffer
 *
 *  @return 0为成功
 */
int getLocalCacheFilePath(FWInfo *pFWInfo,FWDownloadInfo* pFWDownloadInfo,char *buf)
{
    char *latestVersion = pFWDownloadInfo->version;
    char *fwName = pFWInfo->name;
    
    char newFileName[256]={0};
    char extensionName[64]={0};
    dmfh_get_extension(pFWDownloadInfo->fwurl,extensionName);
    strcpy(newFileName, getLocalCacheFileDir());
    strcat(newFileName, "/");
    strcat(newFileName, fwName);
    strcat(newFileName, "_");
    strcat(newFileName, latestVersion);
    strcat(newFileName, ".");
    strcat(newFileName, extensionName);
    strcpy(buf, newFileName);
    return 0;
}

static int net_hasLatestFromServerWithFWInfoAndFWDownloadInfo(FWInfo *pFWInfo,FWDownloadInfo** resultFWDownloadInfo)
{
    int result = 0;
    char url[512]={0};
    
    sprintf(url, "http://%s%s?customCode=%s&versionCode=%s&mac=&language=zh|en&time=120&versionflag=%d",SERVER_DNS,SERVER_GET_URI,pFWInfo->name,pFWInfo->version,pFWInfo->versionFlag);//add english request url zh|en
    DMCLOG_D("url = %s",url);
    char* contentBuf = (char *)malloc(1024*2);
    int contentLen = 0;
    memset(contentBuf, 0, 1024*2);
    int ret = netGet(url, NULL, 1024*2, contentBuf, &contentLen);
    
    if (ret == 0) {
        DMCLOG_D("获取服务器数据成功");
        DMCLOG_D("contentBuf = %s",contentBuf);
        cJSON * pJson = cJSON_Parse(contentBuf);
        if(NULL != pJson)
        {
            FWDownloadInfo* pFWDownloadInfo = makeFWDownloadInfoWithcJSON(pJson);
            
            
            if ((pFWDownloadInfo != NULL) && (pFWDownloadInfo->status != NULL)) {
                if (!strcmp(pFWDownloadInfo->status, "success_fwUpgrade")) {
                    
                    
                    result = 1;
                }
                else if (!strcmp(pFWDownloadInfo->status, "error_version")) {
                    result = 0;
                }
                else if (!strcmp(pFWDownloadInfo->status, "error_paraformat")) {
                    result = 0;
                }
            }
            if (resultFWDownloadInfo == NULL) {
                freeFWDownloadInfo(pFWDownloadInfo);
            }
            else{
                *resultFWDownloadInfo = pFWDownloadInfo;
            }
        }
    }else{
    	DMCLOG_E("获取服务器数据失败");
    }
    //    testGhttp();
    
    free(contentBuf);
    return result;
}

int net_hasLatestFromServer(FWInfo *pFWInfo,FWDownloadInfo** resultFWDownloadInfo)
{
    return net_hasLatestFromServerWithFWInfoAndFWDownloadInfo(pFWInfo,resultFWDownloadInfo);
}


int net_downloadFormServer(FWInfo *pFWInfo,FWFileInfo ** pFWFileInfo,int (*progress_chanage)(long progress,long total))
{
    int ret = 0;
    FWDownloadInfo *pFWDownloadInfo=NULL;
    //先获取最新版本信息
    ret = net_hasLatestFromServerWithFWInfoAndFWDownloadInfo(pFWInfo,&pFWDownloadInfo);
    if ((ret)&&(pFWDownloadInfo != NULL)) {
        DMCLOG_D("the latest fw address:%s.", pFWDownloadInfo->fwurl);
        char localCacheFilePath[256] = {0};
        getLocalCacheFilePath(pFWInfo, pFWDownloadInfo, localCacheFilePath);
        DMCLOG_D("local cache path:%s.", localCacheFilePath);
        //开始下载固件到本地
//        ret = download(pFWDownloadInfo->fwurl, localCacheFilePath);
        ret =  download_http_file(pFWDownloadInfo->fwurl , localCacheFilePath , progress_chanage);
 
        if (!ret) {
            pFWDownloadInfo->versionFlag = pFWInfo->versionFlag;
            *pFWFileInfo = makeFWFileInfo(pFWInfo->name,pFWDownloadInfo->version,pFWDownloadInfo->versionFlag, localCacheFilePath,pFWDownloadInfo->fwSize,pFWDownloadInfo->updatecontent_zh,pFWDownloadInfo->updatecontent_en);
            
        }
        
        if (pFWDownloadInfo != NULL) {
            freeFWDownloadInfo(pFWDownloadInfo);
        }
        
    }
    
    return ret;
}
