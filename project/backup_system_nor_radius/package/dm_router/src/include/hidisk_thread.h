/************************************************************************
#
#  Copyright (c) 2015-2016 longsys(SHENTHEN) Co., Ltd.
#  All Rights Reserved
#
#  author: Oliver
#  create date: 2015-3-19
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
# Not withstanding the above, under no circumstance
s may you combine 
# this software in any way with any other longsys software provided 
# under a license other than the GPL, without longsys's express prior 
# written consent. 
#
# Revision Table
#
# Version     | Name             |Date           |Description
# ------------|------------------|---------------|-------------------
#  0.1.0.1    |Oliver       |2015-3-19    |Trial Version
#
*************************************************************************/

#ifndef __HIDISK_THREAD_H__
#define __HIDISK_THREAD_H__

#if __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <resolv.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>

#define ECOS_SYSTEM
#ifndef ECOS_SYSTEM
#define PTHREAD_T pthread_t
#define PTHREAD_ATTR_T pthread_attr_t
#define PTHREAD_MUTEX_T pthread_mutex_t
#define PTHREAD_COND_T pthread_cond_t

#define PTHREAD_MUTEX_INIT(MUTEX,ATTR) pthread_mutex_init(MUTEX,ATTR);
#define PTHREAD_COND_INIT(COND,ATTR) pthread_cond_init(COND,ATTR)
#define PTHREAD_ATTR_INIT(ATTR) pthread_attr_init(ATTR)
#define PTHREAD_ATTR_SETDETACHSTATE(ATTR,STATE) pthread_attr_setdetachstate(ATTR,STATE)
#define PTHREAD_CREATE(TIDP,ATTR,START_RTN,ARG) pthread_create(TIDP,ATTR,START_RTN,ARG)
#define PTHREAD_DETACH(TID) pthread_detach(TID)
#define PTHREAD_COND_SIGNAL(COND) pthread_cond_signal(COND)
#define PTHREAD_SELF() pthread_self()
#define PTHREAD_MUTEX_LOCK(MUTEX) pthread_mutex_lock(MUTEX)
#define PTHREAD_MUTEX_UNLOCK(MUTEX) pthread_mutex_unlock(MUTEX)

#define PTHREAD_COND_WAIT(COND,MUTEX) pthread_cond_wait(COND,MUTEX)
#define PTHREAD_EXIT(RETVAL) pthread_exit(RETVAL)
#define PTHREAD_COND_BROADCAST(COND) pthread_cond_broadcast(COND)
#define PTHREAD_JOIN(ID,RETVAL) pthread_join(ID,RETVAL)
#define PTHREAD_ATTR_DESTROY(ATTR) pthread_attr_destroy(ATTR)
#define PTHREAD_MUTEX_DESTORY(MUTEX) pthread_mutex_destroy(MUTEX)
#define PTHREAD_COND_DESTROY(COND) pthread_cond_destroy(COND)
#define PTHREAD_SETCANCELSTATE(STATE,OLDSTATE) pthread_setcancelstate(STATE,OLDSTATE)
#define PTHREAD_SETCANCELTYPE(TYPE,OLDTYPE) pthread_setcanceltype(TYPE,OLDTYPE)
#define PTHREAD_CLEANUP_PUSH(ROUTINE,ARG) pthread_cleanup_push(ROUTINE,ARG)
#define PTHREAD_CLEANUP_POP(EXCUTE) pthread_cleanup_pop(EXCUTE)
#else
#define PTHREAD_T pthread_t
#define PTHREAD_ATTR_T pthread_attr_t
#define PTHREAD_MUTEX_T pthread_mutex_t
#define PTHREAD_COND_T pthread_cond_t

#define PTHREAD_MUTEX_INIT(MUTEX,ATTR) pthread_mutex_init(MUTEX,ATTR);
#define PTHREAD_COND_INIT(COND,ATTR) pthread_cond_init(COND,ATTR)
#define PTHREAD_ATTR_INIT(ATTR) pthread_attr_init(ATTR)
#define PTHREAD_ATTR_SETDETACHSTATE(ATTR,STATE) pthread_attr_setdetachstate(ATTR,STATE)
#define PTHREAD_CREATE(TIDP,ATTR,START_RTN,ARG) pthread_create(TIDP,ATTR,START_RTN,ARG)
#define PTHREAD_DETACH(TID) pthread_detach(TID)
#define PTHREAD_COND_SIGNAL(COND) pthread_cond_signal(COND)
#define PTHREAD_SELF() pthread_self()
#define PTHREAD_MUTEX_LOCK(MUTEX) pthread_mutex_lock(MUTEX)
#define PTHREAD_MUTEX_UNLOCK(MUTEX) pthread_mutex_unlock(MUTEX)
#define PTHREAD_COND_WAIT(COND,MUTEX) pthread_cond_wait(COND,MUTEX)
#define PTHREAD_EXIT(RETVAL) pthread_exit(RETVAL)
#define PTHREAD_COND_BROADCAST(COND) pthread_cond_broadcast(COND)
#define PTHREAD_JOIN(ID,RETVAL) pthread_join(ID,RETVAL)
#define PTHREAD_ATTR_DESTROY(ATTR) pthread_attr_destroy(ATTR)
#define PTHREAD_MUTEX_DESTORY(MUTEX) pthread_mutex_destroy(MUTEX)
#define PTHREAD_COND_DESTROY(COND) pthread_cond_destroy(COND)
#define PTHREAD_SETCANCELSTATE(STATE,OLDSTATE) pthread_setcancelstate(STATE,OLDSTATE)
#define PTHREAD_SETCANCELTYPE(TYPE,OLDTYPE) pthread_setcanceltype(TYPE,OLDTYPE)
#define PTHREAD_CLEANUP_PUSH(ROUTINE,ARG) pthread_cleanup_push(ROUTINE,ARG)
#define PTHREAD_CLEANUP_POP(EXCUTE) pthread_cleanup_pop(EXCUTE)

#endif

/*############################## Prototypes ##################################*/

#if __cplusplus
}
#endif

#endif /* __HIDISK_THREAD_H__ */



