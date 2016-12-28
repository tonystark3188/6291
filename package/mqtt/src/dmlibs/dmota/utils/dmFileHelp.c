//
//  dmFileHelp.c
//  DMOTA
//
//  Created by 杨明 on 16/1/14.
//  Copyright © 2016年 杨明. All rights reserved.
//

#include "dmFileHelp.h"
#include <string.h>


/**
 *  判断文件是否存在
 *
 *  @param filePath 文件路径
 *
 *  @return 0：不存在，非0，存在
 */
int dmfh_isFileExist(const char *filePath)
{
    
    FILE* pf = fopen(filePath, "r");
    if (pf) {
        
        fclose(pf);
        return 1;
    }
    return 0;
}
/**
 *  删除文件
 *
 *  @param filePath 删除指定文件
 *
 *  @return 0成功，非0失败
 */
int dmfh_deleteFile(const char *filePath)
{
    return remove(filePath);
}

/**
 *  保存字符串到文件
 *
 *  @param filePath 文件绝对路径
 *  @param str      字符串
 *
 *  @return 0为成功，非0为失败
 */
int dmfh_stringToFile(const char *filePath,const char *strBuffer)
{
    FILE *op = NULL;
    op=fopen(filePath,"w");
    if (NULL != op) {
        fprintf(op,"%s",strBuffer);
        fclose(op);
        return 0;
    }
    return -1;
}

/**
 *  从文件中读取字符串
 *
 *  @param filePath  文件路径
 *  @param strBuffer 读取的字符串缓存区域
 *
 *  @return 返回读取的长度，大于0为正常
 */
size_t dmfh_fileToString(const char *filePath,char *strBuffer,size_t bufLength)
{
    size_t len = 0;
    size_t buflen = bufLength;
    FILE * pFile;
    
    memset(strBuffer,0,buflen);
    
    pFile = fopen (filePath , "r");
    if (pFile == NULL)
        len = 0;
    else {
        len = fread(strBuffer, 1, buflen, pFile);
        
        fclose (pFile);
    }
    return len;
}

/**
 *  获取文件的后最名
 *
 *  @param file_name 目标文件名
 *  @param extension 扩展名buffer
 */
void dmfh_get_extension(const char *file_name,char *extension)
{
    int i=0;
    unsigned long length=strlen(file_name);
    if (length > 0) {
        i = (int)length - 1;
        while(i >=0)
        {
            if(file_name[i]=='.')
                break;
            i--;
        }
        if(i>= 0)
        {
            
            strcpy(extension,file_name+i+1);
            return ;
        }
        else
            strcpy(extension,"\0");
    }
    strcpy(extension,"\0");

}