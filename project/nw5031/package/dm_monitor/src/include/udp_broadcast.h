/************************************************************************
#
#  Copyright (c) 2014-2016  I-MOVE(SHENTHEN) Co., Ltd.
#  All Rights Reserved
#
#  author: Oliver
#  create date: 2015-4-9
# 
# Unless you and longsys execute a separate written software license 
# agreement governing use of this software, this software is licensed 
# to you under the terms of the GNU General Public License version 2 
# (the "GPL"), with the following added to such license:
# 
#    As a special exception, the copyright holders of this software give 
#    you permission to link this software with independent modules, and 
#    to copy and distribute the resulting executable under terms of your 
#    choice, provided that you also meet, for each linked independent 
#    module, the terms and conditions of the license of that module. 
#    An independent module is a module which is not derived from this
#    software.  The special exception does not apply to any modifications 
#    of the software.  
# 
# Not withstanding the above, under no circumstances may you combine 
# this software in any way with any other longsys software provided 
# under a license other than the GPL, without longsys's express prior 
# written consent. 
#
# Revision Table
#
# Version     | Name             |Date           |Description
# ------------|------------------|---------------|-------------------
#  0.1.0.1    |Oliver       |2015-4-9     |Trial Version
#
*************************************************************************/

#ifndef __UDP_BROADCAST_H__
#define __UDP_BROADCAST_H__

#if __cplusplus
extern "C" {
#endif

#define BROADCAST_PORT	 27213
#define BROADCAST_CMD  	 0x0400
#define BROADCAST_SEQ    0
#define BROADCAST_VER    0
#define BROADCAST_DEVICE 0
#define BROADCAST_APPID  0
#define BROADCAST_CODE   0
#define SEND_BUF_SIZE    1024
#define REV_BUF_SIZE     1024
#define IP_LEN           32

int upd_broadcast(void);


#if __cplusplus
}
#endif

#endif /* __MSG_CLIENT_H__ */

