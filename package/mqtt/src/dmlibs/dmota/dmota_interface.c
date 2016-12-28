//
//  dmota_interface.c
//  DMOTA
//
//  Created by 杨明 on 16/1/22.
//  Copyright © 2016年 杨明. All rights reserved.
//

#include "dmota_interface.h"
#include "my_debug.h"
#include <string.h>
#include <stdlib.h>

/**
 *  获取当前固件是否有最新版本固件
 *
 *  @param resultNewFWInfo 返回最新的版本信息，
 *
 *  @return 0为没有，非0为存在，如果为local代表当前本地的缓存中已经存在最新版本，无需下载。如果返回的时net则代表最新版本在服务器上面。如果当前没有连接设备则只能调用download方法进行下载。如果连接上了设备则可以直接使用upgrade进行fw的升级。这个需要app上层自行判断调用下面那总方式。
 */
DMOTA_QUERY_RESULT ota_hasLatest(FWQueryResult **resultFWQueryResult)
{
    int res = 0;
    FWInfo pFWInfo;
    memset(&pFWInfo,0,sizeof(FWInfo));
    res = dmota_getFWInfoLastTime(&pFWInfo);
    if(res != 0)
    {
        printf("dm get FW latest info error,res = %d\n",res);
        return res;
    }
    res = dmota_hasLatest(&pFWInfo,resultFWQueryResult);
    if(res == 0)
    {
        printf("dm latest FW is not exist in local dir,res = %d\n",res);
    }
    return res;
}

/**
 *  从服务器下载最新固件
 *
 *  @param resultFWQueryResult 返回最新的固件的基本信息，包含固件名称，固件版本，已下载完成的本地路径
 *  @param progress_chanage 进度回调信息，包含总文件大小，当前进度。一般返回0，如果返回非零则中断下载任务。
 *
 *  @return 0为正常，非0异常
 */
int ota_downloadFormServer(FWQueryResult **resultFWQueryResult,int (*progress_chanage)(long progress,long total))
{
    int res = 0;
    FWInfo pFWInfo;
    memset(&pFWInfo,0,sizeof(FWInfo));
    res = dmota_getFWInfoLastTime(&pFWInfo);
    if(res != 0)
    {
        printf("dm get FW latest info error,res = %d\n",res);
        return res;
    }
//    strcpy(pFWInfo.name,"A999(M.03)");
//    strcpy(pFWInfo.version,"1.0.00.0");
    FWFileInfo *resultFWFileInfo;
    res = dmota_downloadFormServer(&pFWInfo,&resultFWFileInfo,progress_chanage);
    if(res != 0)
    {
        printf("dm download fw from server error,res = %d\n",res);
        return res;
    }
    if(resultFWFileInfo == NULL)
    {
        res = -1;
        return res;
    }
    *resultFWQueryResult = (FWQueryResult *)malloc(sizeof(FWQueryResult));
    memset(*resultFWQueryResult,0,sizeof(FWQueryResult));
    if(*resultFWQueryResult == NULL)
    {
        res = -1;
        return res;
    }
    if(resultFWFileInfo->pFWInfo != NULL)
    {
        (*resultFWQueryResult)->name = (char *)malloc(strlen(resultFWFileInfo->pFWInfo->name) + 1);
        strcpy((*resultFWQueryResult)->name,resultFWFileInfo->pFWInfo->name);
    }
    if(resultFWFileInfo->filePath != NULL)
    {
        (*resultFWQueryResult)->uri = (char *)malloc(strlen(resultFWFileInfo->filePath) + 1);
        strcpy((*resultFWQueryResult)->uri,resultFWFileInfo->filePath);
    }
    freeFWFileInfo(resultFWFileInfo);
    return res;
}