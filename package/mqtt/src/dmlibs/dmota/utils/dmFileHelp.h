//
//  dmFileHelp.h
//  DMOTA
//
//  Created by 杨明 on 16/1/14.
//  Copyright © 2016年 杨明. All rights reserved.
//

#ifndef dmFileHelp_h
#define dmFileHelp_h

#include <stdio.h>

/**
 *  保存字符串到文件
 *
 *  @param filePath 文件绝对路径
 *  @param str      字符串
 *
 *  @return 0为成功，非0为失败
 */
extern int dmfh_stringToFile(const char *filePath,const char *strBuffer);

/**
 *  从文件中读取字符串
 *
 *  @param filePath  文件路径
 *  @param strBuffer 读取的字符串缓存区域
 *
 *  @return 返回读取的长度，大于0为正常，-1为异常
 */
extern size_t dmfh_fileToString(const char *filePath,char *strBuffer,size_t bufLength);

/**
 *  获取文件的后最名
 *
 *  @param file_name 目标文件名
 *  @param extension 扩展名buffer
 */
extern void dmfh_get_extension(const char *file_name,char *extension);
/**
 *  判断文件是否存在
 *
 *  @param filePath 文件路径
 *
 *  @return 0：不存在，非0，存在
 */
extern int dmfh_isFileExist(const char *filePath);
/**
 *  删除文件
 *
 *  @param filePath 删除指定文件
 *
 *  @return 0成功，非0失败
 */
extern int dmfh_deleteFile(const char *filePath);

#endif /* dmFileHelp_h */
