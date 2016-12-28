//
//  dmusr.h
//  DMUSR
//
//  Created by Oiver on 16/6/24.
//  Copyright © 2016年 Oliver. All rights reserved.
//

#ifndef dmusr_h
#define dmusr_h

#include <stdio.h>

/************************************************************************************
 *                          ENUM                                                    *
 ***********************************************************************************/

typedef enum {
    DMUSR_SUCCES = 0,//操作成功
    DMUSR_PERMISSION_DENIED = 10000,//没有权限
    DMUSR_UNBOUND = 10001,//没有绑定过设备
    DMUSR_PARA_ERROR = 10002,//参数错误
    DMUSR_NOT_LOGIN = 10003,//没有登录
    DMUSR_TIMEOUT = 10004,//任务超时
    DMUSR_SERVER_ERROR = 50000,//服务端错误
} DMUSR_RESULT;

/************************************************************************************
 *                          STRUCT                                                    *
 ***********************************************************************************/
typedef struct {
    int     error_code;//0为成功，非0为异常
    char    *error_msg_zh;/*当前文件的中文描述信息*/
    char    *error_msg_en;/*  当前文件的英文描述信息*/
    char    *device_id;//设备ID
    char    *device_type;//设备类型
} ServerQueryResult;

/**
 *  功能：用户注册接口
 *  权限：开放权限
 *
 *  @param usr_name:如果是第三方用户则直接填写userId即可,password:如果是第三方用户则填写空即可（即使填写也不保存）,type:第三方平台类型，0:本地用户,1:qq,2:微信
 *
 *  @return 0为成功，非0为异常。
 */
extern DMUSR_RESULT dm_usr_register(char *usr_name,char *password,int type);

/**
 *  功能：用户登录
 *  权限：开放权限
 *
 *  @param usr_name:如果是第三方用户则直接填写userId即可,password:如果是第三方用户则填写空即可（即使填写也不保存）,type:第三方平台类型，0:本地用户,1:qq,2:微信
 
 *
 *  @return 0为成功，非0为异常。
 */
extern DMUSR_RESULT dm_usr_login(char *usr_name,char *password,int type);

/**
 *  功能：用户退出登录
 *  权限：登陆过的用户
 *
 *  @param usr_name:如果是第三方用户则直接填写userId即可,password:如果是第三方用户则填写空即可（即使填写也不保存）,type:第三方平台类型，0:本地用户,1:qq,2:微信
 *
 *  @return 0为成功，非0为异常。
 */
extern DMUSR_RESULT dm_usr_logout();

/**
 *  功能：用户绑定设备
 *  权限：登陆过的用户
 *
 *  @param device_id:设备ID，device_type:设备类型
 *
 *  @return 0为成功，非0为异常。
 */
extern DMUSR_RESULT dm_usr_bind(char *device_id,char *device_type);


/**
 *  功能：用户与设备解绑
 *  权限：登陆过的用户
 *
 *  @param device_id:设备ID，device_type:设备类型
 *
 *  @return 0为成功，非0为异常。
 */
extern DMUSR_RESULT dm_usr_unbind(char *device_id,char *device_type);


/**
 *  功能：获取绑定设备列表
 *  权限：登陆过的用户
 *
 *  @param device_id:设备ID，device_type:设备类型
 *
 *  @return 0为成功，非0为异常。
 */
extern DMUSR_RESULT dm_usr_get_bind_list(ServerQueryResult **serverQueryResult);

#endif /* dmusr_h */
