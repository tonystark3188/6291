//
//  dmota_constant.h
//  DMOTA
//
//  Created by 杨明 on 16/1/21.
//  Copyright © 2016年 杨明. All rights reserved.
//

#ifndef dmota_constant_h
#define dmota_constant_h

#include <stdio.h>

/**
 *  服务器域名
 */
#define SERVER_DNS "x.dmsys.com"
/**
 *  ota请求的uri
 */
#define SERVER_GET_URI "/GetXml"
/**
 *  本地的缓存目录，在程序初始化的时候也可以动态修改
 */
#define LOCAL_CACHE_FILE_DIR "/Users/glenn/Desktop"
/**
 *  缓存数据信息的描述文件
 */
#define LOCAL_CACHE_INFO_FILENAME "json.txt"

/**
 *  设置本地缓存目录
 * 例子：/sdcard/appdir/cache
 *  @param cacheDir 缓存目录路径
 *
 *  @return wu
 */
extern void setLocalCacheFileDir(char *cacheDir);
/**
 *  获取缓存目录路径
 *
 *  @return 返回路径的引用
 */
extern char *getLocalCacheFileDir();
/**
 *  返回当前缓存目录信息描述文件的路径
 *
 *  @return 返回路径
 */
extern char *getLocalCacheInfoFilePath();

#endif /* dmota_constant_h */
