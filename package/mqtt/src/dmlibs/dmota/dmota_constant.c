//
//  dmota_constant.c
//  DMOTA
//
//  Created by 杨明 on 16/1/21.
//  Copyright © 2016年 杨明. All rights reserved.
//

#include "dmota_constant.h"
#include <string.h>
#include <stdlib.h>

char *LocalCacheFileDir = LOCAL_CACHE_FILE_DIR;
char LocalCacheInfoFilePath[256];

/**
 *  设置本地缓存目录
 *  例子：/sdcard/appdir/cache
 *  @param cacheDir 缓存目录路径
 *
 *  @return wu
 */
void setLocalCacheFileDir(char *cacheDir)
{
    if (strcmp(LocalCacheFileDir, LOCAL_CACHE_FILE_DIR)) {
        //需要释放内存
        free(LocalCacheFileDir);
    }
    LocalCacheFileDir = strdup(cacheDir);
}
/**
 *  获取缓存目录路径
 *
 *  @return 返回路径的引用
 */
char *getLocalCacheFileDir()
{
    return LocalCacheFileDir;
}


char *getLocalCacheInfoFilePath()
{
    memset(LocalCacheInfoFilePath,0 ,sizeof(LocalCacheInfoFilePath));
    strcpy(LocalCacheInfoFilePath, getLocalCacheFileDir());
    strcat(LocalCacheInfoFilePath, "/");
    strcat(LocalCacheInfoFilePath, LOCAL_CACHE_INFO_FILENAME);
    return LocalCacheInfoFilePath;
}