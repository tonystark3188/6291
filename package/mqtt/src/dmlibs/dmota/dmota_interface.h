//
//  dmota_interface.h
//  DMOTA
//
//  Created by 杨明 on 16/1/22.
//  Copyright © 2016年 杨明. All rights reserved.
//

#ifndef dmota_interface_h
#define dmota_interface_h

#include <stdio.h>
#include "dmota.h"


/**
 *  获取当前固件是否有最新版本固件
 *
 *  @param resultNewFWInfo 返回最新的版本信息，
 *
 *  @return 0为没有，非0为存在，如果为local代表当前本地的缓存中已经存在最新版本，无需下载。如果返回的时net则代表最新版本在服务器上面。如果当前没有连接设备则只能调用download方法进行下载。如果连接上了设备则可以直接使用upgrade进行fw的升级。这个需要app上层自行判断调用下面那总方式。
 */
extern DMOTA_QUERY_RESULT ota_hasLatest(FWQueryResult **resultFWQueryResult);

/**
 *  从服务器下载最新固件
 *
 *  @param resultFWQueryResult 返回最新的固件的基本信息，包含固件名称，固件版本，已下载完成的本地路径
 *  @param progress_chanage 进度回调信息，包含总文件大小，当前进度。一般返回0，如果返回非零则中断下载任务。
 *
 *  @return 0为正常，非0异常
 */
extern int ota_downloadFormServer(FWQueryResult **resultFWQueryResult,int (*progress_chanage)(long progress,long total));


/**
 *  内部自动判断升级，如果服务器最新则直接从服务器下载更新，如果无法连接服务器，则直接从本地缓存中直接更新。并且上传固件到fw段进行升级。
 *
 *  @param resultFWQueryResult 返回最新的固件的基本信息，包含固件名称，固件版本，已下载完成的本地路径
 *  @param progress_chanage 进度回调信息，包含总文件大小，当前进度。一般返回0，如果返回非零则中断下载任务。
 *  升级过程分为3各步骤，及 下载，上传，升级等待，整个过程分为3段，各占1/3，返回的进度是总进度。
 *
 *  @return 0为正常，非0异常
 */
extern int ota_upgrade(int (*progress_chanage)(long progress,long total));

#endif /* dmota_interface_h */
