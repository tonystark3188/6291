/*
 * =============================================================================
 *
 *       Filename:  inotify_base.h
 *
 *    Description:  inotify module
 *
 *        Version:  1.0
 *        Created:  2015/1/5 13:11:18
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ye Wenhao (), wenhaoye@126.com
 *   Organization:  
 *
 * =============================================================================
 */
#ifndef _INOTIFY_BASE_H_
#define _INOTIFY_BASE_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <sys/inotify.h>
#include "event/my_event_base.h"


typedef int (*fd_watch_handler)(void *arg, struct inotify_event *event);

int init_inotify_watcher(void);
int add_inotify_watcher_event(event_base_t base_fd);
int add_file_watch(const char *path, uint32_t mask, fd_watch_handler handler, void *arg);
int del_file_watch(int wd);



#ifdef __cplusplus
}
#endif

#endif
