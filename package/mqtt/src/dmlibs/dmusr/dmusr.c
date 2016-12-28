//
//  dmusr.c
//  DMOTA
//
//  Created by Oliver on 16/1/22.
//  Copyright © 2016年 Oliver. All rights reserved.
//

#include "dmusr.h"
#include "my_debug.h"
#include <string.h>
#include <stdlib.h>

/**
 *  功能：用户注册接口
 *  权限：开放权限
 *
 *  @param usr_name:如果是第三方用户则直接填写userId即可,password:如果是第三方用户则填写空即可（即使填写也不保存）,type:第三方平台类型，0:本地用户,1:qq,2:微信
 *
 *  @return 0为成功，非0为异常。
 */
extern DMUSR_RESULT dm_usr_register(char *usr_name,char *password,int type)
{
    return DMUSR_SUCCES;
}

/**
 *  功能：用户登录
 *  权限：开放权限
 *
 *  @param usr_name:如果是第三方用户则直接填写userId即可,password:如果是第三方用户则填写空即可（即使填写也不保存）,type:第三方平台类型，0:本地用户,1:qq,2:微信
 
 *
 *  @return 0为成功，非0为异常。
 */
extern DMUSR_RESULT dm_usr_login(char *usr_name,char *password,int type)
{
    return DMUSR_SUCCES;
}

/**
 *  功能：用户退出登录
 *  权限：登陆过的用户
 *
 *  @param usr_name:如果是第三方用户则直接填写userId即可,password:如果是第三方用户则填写空即可（即使填写也不保存）,type:第三方平台类型，0:本地用户,1:qq,2:微信
 *
 *  @return 0为成功，非0为异常。
 */
extern DMUSR_RESULT dm_usr_logout()
{
    return DMUSR_SUCCES;
}

/**
 *  功能：用户绑定设备
 *  权限：登陆过的用户
 *
 *  @param device_id:设备ID，device_type:设备类型
 *
 *  @return 0为成功，非0为异常。
 */
extern DMUSR_RESULT dm_usr_bind(char *device_id,char *device_type)
{
    return DMUSR_SUCCES;
}


/**
 *  功能：用户与设备解绑
 *  权限：登陆过的用户
 *
 *  @param device_id:设备ID，device_type:设备类型
 *
 *  @return 0为成功，非0为异常。
 */
extern DMUSR_RESULT dm_usr_unbind(char *device_id,char *device_type)
{
    return DMUSR_SUCCES;
}


/**
 *  功能：获取绑定设备列表
 *  权限：登陆过的用户
 *
 *  @param device_id:设备ID，device_type:设备类型
 *
 *  @return 0为成功，非0为异常。
 */
extern DMUSR_RESULT dm_usr_get_bind_list(ServerQueryResult **serverQueryResult)
{
    return DMUSR_SUCCES;
}