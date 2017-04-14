//
//  dm_sort.c
//  
//
//  Created by oliver on 16/9/5.
//
//

#include "dm_sort.h"
#include <locale.h>

char sort_path[256];
static bool is_sort_file_exist(const char *path)
{
    if(access(path,F_OK) == 0)
    {
        DMCLOG_D("the %s is exist",path);
        return true;
    }
    DMCLOG_D("the %s is not exist",path);
    return false;
}

int get_sort_info_from_path(SortInfo *pSortInfo,char *path)
{
    if(pSortInfo == NULL)
    {
        DMCLOG_D("the para is null");
        return -1;
    }
    JObj* root_json = NULL;
    JObj* sort_type_json = NULL;
    JObj* sort_reverse_json = NULL;
    root_json = json_object_from_file(path);
    if(root_json == NULL)
    {
        return -1;
    }
    
    sort_type_json = JSON_GET_OBJECT(root_json,"sort_type");
    if(sort_type_json != NULL)
    {
        pSortInfo->sort_type = JSON_GET_OBJECT_VALUE(sort_type_json,int);
    }
    
    sort_reverse_json = JSON_GET_OBJECT(root_json,"sort_reverse");
    if(sort_reverse_json != NULL)
    {
        pSortInfo->sort_reverse = JSON_GET_OBJECT_VALUE(sort_reverse_json,int);
    }
    return 0;
}

int set_sort_info_to_path(char *path,SortInfo *pSortInfo)
{
    if(path == NULL||pSortInfo == NULL)
    {
        DMCLOG_D("the para is null");
        return -1;
    }
    FILE* fp;
    if( (fp = fopen(path, "w+")) == NULL)
    {
        DMCLOG_D("open error :%d",errno);
        return -1;
    }
    fclose(fp);
    
    JObj* root_json = json_object_new_object();
    JSON_ADD_OBJECT(root_json, "sort_type", JSON_NEW_OBJECT(pSortInfo->sort_type,int));
    JSON_ADD_OBJECT(root_json, "sort_reverse", JSON_NEW_OBJECT(pSortInfo->sort_reverse,int));
    json_object_to_file(path, root_json);
    json_object_put(root_json);
    return 0;
}

/**
 *  初始化文件排序服务
 *
 *  @param path 存储排序信息的绝对路径
 *
 *  @return 0为成功，－1为失败
 */
int init_sort_module(char *path)
{
    int ret = 0;
    sprintf(sort_path,"%s/sort.json",path);
    DMCLOG_D("sort_path = %s",sort_path);
    if(is_sort_file_exist(sort_path) == true)
    {
        if((ret = get_sort_info_from_path(&pSortInfo,sort_path)) < 0 )
        {
            return -1;
        }
    }else{
        pSortInfo.sort_type = SORT_MTIME;
        pSortInfo.sort_reverse = SORT_DES;
        set_sort_info_to_path(sort_path,&pSortInfo);
    }
    return 0;
}

int get_sort_type()
{
    return pSortInfo.sort_type;
}

int set_sort_type(int sort_type)
{
    pSortInfo.sort_type = sort_type;
    set_sort_info_to_path(sort_path,&pSortInfo);
    return 0;
}

int get_sort_reverse()
{
    return 1;//pSortInfo.sort_reverse;
}

int set_sort_reverse(int sort_reverse)
{
    pSortInfo.sort_reverse = sort_reverse;
    set_sort_info_to_path(sort_path,&pSortInfo);
    return 0;
}
/**
 *  依据规则判断需要排序的元素的大小
 *
 *  @param a:元素，b:元素
 *
 *  @return 大小的差距
 */
static int sortcmp(const void *a, const void *b)
{
    struct file_dnode *d1 = *(struct file_dnode **)a;
    struct file_dnode *d2 = *(struct file_dnode **)b;
    //unsigned sort_opts =SORT_MTIME;// get_sort_type();
    //  TODO: use pre-initialized function pointer
    // instead of branch forest
	return (d1->index - d2->index );
}

/**
 *  归并排序
 *
 *  @param dn:文件列表二级指针,size:文件列表长度
 *
 *  @return void
 */
void dnsort(struct file_dnode **dn, int size)
{
//    p_debug("local = %s",setlocale(LC_COLLATE,NULL));
    qsort(dn, size, sizeof(*dn), sortcmp);
}