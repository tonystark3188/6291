//
//  dmota_net.h
//  DMOTA
//
//  Created by 杨明 on 16/1/15.
//  Copyright © 2016年 杨明. All rights reserved.
//

#ifndef dmota_net_h
#define dmota_net_h

#include <stdio.h>
#include "dmota_json.h"

extern int net_hasLatestFromServer(FWInfo *pFWInfo,FWDownloadInfo** resultFWDownloadInfo);
extern int net_downloadFormServer(FWInfo *pFWInfo,FWFileInfo ** pFWFileInfo,int (*progress_chanage)(long progress,long total));
#endif /* dmota_net_h */
