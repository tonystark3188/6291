//
//  dmStringUtils.c
//  DMOTA
//
//  Created by 杨明 on 16/1/21.
//  Copyright © 2016年 杨明. All rights reserved.
//

#include "dmStringUtils.h"
#include "string.h"
#include "stdlib.h"

int convertVersionToArray(const char *versionString,int* intArray)
{
    
    //
    char *delims= ".";
    
    char * result = NULL;
    char *tempString = strdup(versionString);
    char *p = tempString;
    int *pInt = intArray;
    
    result = strtok(tempString,delims);
    
    while(result != NULL)
    {
//        printf("%s\n",result);
        *pInt = atoi(result);
        pInt++;
        
        result=strtok(NULL,delims);
        
    }
    
    free(p);
    return 0;
}

/**
 *  版本号比较
 *  1.0.0.1
 *  @param dest1 目标1
 *  @param dest2 目标2
 *
 *  @return 0：相等  小于0：dest1<dest2  大于0：dest1 > dest2
 */
int dm_version_cmp(const char *dest1,const char *dest2)
{
    int destArray1[6] = {-1,-1,-1,-1,-1,-1};
    int destArray2[6] = {-1,-1,-1,-1,-1,-1};
    
    convertVersionToArray(dest1,destArray1);
    convertVersionToArray(dest2,destArray2);
    
    int i = 0;
    int result = 0;
    while (destArray1[i] != -1) {
        result = destArray1[i] - destArray2[i];
        if (result != 0) {
            return result;
        }
        i++;
    }

    return 0;
}