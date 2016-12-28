//
//  dmStringUtils.h
//  DMOTA
//
//  Created by 杨明 on 16/1/21.
//  Copyright © 2016年 杨明. All rights reserved.
//

#ifndef dmStringUtils_h
#define dmStringUtils_h

#include <stdio.h>
/**
 *  版本号比较
 *  1.0.0.1
 *  @param dest1 目标1
 *  @param dest2 目标2
 *
 *  @return 0：相等  小于0：dest1<dest2  大于0：dest1 > dest2
 */
extern int dm_version_cmp(const char *dest1,const char *dest2);
#endif /* dmStringUtils_h */
