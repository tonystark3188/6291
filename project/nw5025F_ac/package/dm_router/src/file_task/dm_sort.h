//
//  dm_sort.h
//  
//
//  Created by apple on 16/9/5.
//
//

#ifndef ____dm_sort__
#define ____dm_sort__

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include "my_debug.h"
#include "my_json.h"
#include "router_task.h"
//#include "router_task.h"


#define ENABLE_LOCALE_SUPPORT   1
typedef unsigned long long      uoff_t;


typedef enum
{
    SORT_DES            = 0,//降序
    SORT_ASC,               //升序
}SortReverse;


typedef struct sort_info{
    int sort_type;//0:time,1:大小，2:名称
    int sort_reverse;//0:降序，1:升序
}SortInfo;

SortInfo pSortInfo;

/**
 *  初始化文件排序服务
 *
 *  @param path 存储排序信息的绝对路径
 *
 *  @return 0为成功，－1为失败
 */
int init_sort_module(char *path);
/**
 *  获取文件排序的规则
 *
 *  @param null
 *
 *  @return 0:时间，1:文件大小，2:文件名称
 */
int get_sort_type();
/**
 *  设置文件排序的规则
 *
 *  @param null
 *
 *  @return 0:时间，1:文件大小，2:文件名称
 */
int set_sort_type(int sort_type);

/**
 *  获取文件排序的升降
 *
 *  @param null
 *
 *  @return 0:降序，1:升序
 */
int get_sort_reverse();
/**
 *  设置文件排序的升降
 *
 *  @param null
 *
 *  @return 0:降序，1:升序
 */
int set_sort_reverse(int sort_reverse);

/**
 *  归并排序
 *
 *  @param dn:文件列表二级指针,size:文件列表长度
 *
 *  @return void
 */
void dnsort(struct file_dnode **dn, int size);
#endif /* defined(____dm_sort__) */
