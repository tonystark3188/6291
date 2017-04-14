/*
 * =============================================================================
 *
 *       Filename:  inotify_base.c
 *
 *    Description:  inotify module
 *
 *        Version:  1.0
 *        Created:  2015/1/5 13:11:00
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ye Wenhao (), wenhaoye@126.com
 *   Organization:  
 *
 * =============================================================================
 */
#include <sys/ioctl.h>
#include "base.h"
#include "event/inotify_base.h"

#define MAX_FILE_WATCH_OBJ_NUMS 4
#define MAX_INOTIFY_WATCHER_BUF_SIZE 4096


typedef struct _FdWatchObj
{
	int wd;
	fd_watch_handler func;
	void *arg;
}FdWatchObj;

typedef struct _InotifyWatcher
{
	int fd;
	char buf[MAX_INOTIFY_WATCHER_BUF_SIZE];
	FdWatchObj objs[MAX_FILE_WATCH_OBJ_NUMS];
}InotifyWatcher;

static InotifyWatcher g_inotify_watcher;

int init_inotify_watcher(void)
{
	g_inotify_watcher.fd = inotify_init();
	if(g_inotify_watcher.fd < 0)
	{
		log_error("inotify_init failed(%s)", strerror(errno));
		return EINOTIFY_INIT;
	}

	uint16_t i;
	for(i = 0; i < MAX_FILE_WATCH_OBJ_NUMS; ++i)
	{
		g_inotify_watcher.objs[i].arg = NULL;
		g_inotify_watcher.objs[i].func = NULL;
		g_inotify_watcher.objs[i].wd = -1;
	}

	return RET_SUCCESS;
}

static int _get_inotify_event_size(int fd, uint32_t *size)
{
	*size = 0;
	int ret = ioctl(fd, FIONREAD, size);
	if(ret < 0)
	{
		log_warning("ioctl failed(%s)", strerror(errno));
		return EIOCTL;
	}

	return RET_SUCCESS;
}

static int _handle_inotify_event(InotifyWatcher *watcher, struct inotify_event *event)
{
	uint16_t i;
	int ret;
	FdWatchObj *obj = NULL;

	for(i = 0; i < MAX_FILE_WATCH_OBJ_NUMS; ++i)
	{
		if(watcher->objs[i].wd == event->wd)
		{
			obj = &watcher->objs[i];
			break;
		}
	}

	if(obj == NULL)
	{
		log_debug("can not find watch obj");
		return RET_SUCCESS;
	}

	ret = (*obj->func)(obj->arg, event);
	return ret;
}

static int _inotify_watcher_event_handler(void *self)
{
	EventObj *e = (EventObj *)(self);
	InotifyWatcher *watcher = (InotifyWatcher *)(e->arg);

	log_trace("Enter");
	if(!(e->e_flag & EVENT_READ))
	{
		log_warning("e_flag has no EVENT_READ");
		return 1;
	}

	// 1. read inotify event
	int ret = read(watcher->fd, watcher->buf, MAX_INOTIFY_WATCHER_BUF_SIZE);
	if(ret < 0)
	{
		if(errno != EINTR && errno != EAGAIN)
		{
			log_error("read failed(%s)", strerror(errno));
			return EREAD;
		}

		return 1;
	}
	else if(ret == 0)
	{
		log_warning("read 0 byte!");
		return 1;
	}
	else
	{
		if(ret >= MAX_INOTIFY_WATCHER_BUF_SIZE)
			ret = MAX_INOTIFY_WATCHER_BUF_SIZE-1;

		watcher->buf[ret] = '\0';
		log_debug("read %d bytes", ret);
	}

	// 2. handle inotify event
	int i = 0;
	struct inotify_event *event;
	while(i < ret)
	{
		event = (struct inotify_event *)(&watcher->buf[i]);
		_handle_inotify_event(watcher, event);
		i += sizeof(struct inotify_event) + event->len;
	}


	log_trace("Exit");
	return RET_SUCCESS;
}

static int _inotify_watcher_event_release(void *self)
{
	EventObj *e = (EventObj *)(self);
	InotifyWatcher *watcher = (InotifyWatcher *)(e->arg);

	log_trace("Enter");
	uint16_t i;
	for(i = 0; i < MAX_FILE_WATCH_OBJ_NUMS; ++i)
	{
		if(watcher->objs[i].wd >= 0)
		{
			inotify_rm_watch(watcher->fd, watcher->objs[i].wd);
			(*watcher->objs[i].func)(watcher->objs[i].arg, NULL);
			watcher->objs[i].wd = -1;
		}
	}

	safe_close(e->fd);
	watcher->fd = -1;
	log_trace("Exit");

	return RET_SUCCESS;
}

int add_inotify_watcher_event(event_base_t base_fd)
{
	EventObj event;

	memset(&event, 0, sizeof(EventObj));
	event.fd = g_inotify_watcher.fd;
	event.flag |= EVENT_READ;
	event.e_handler = _inotify_watcher_event_handler;
	event.release = _inotify_watcher_event_release;
	event.arg = &g_inotify_watcher;
	S_STRNCPY(event.name, "INOTIFY-WATCH", sizeof(event.name));

	return my_event_base_add(base_fd, &event);
}

int add_file_watch(const char *path, uint32_t mask, fd_watch_handler handler, void *arg)
{
	uint16_t i;

	if(path == NULL || handler == NULL)
	{
		log_warning("NULL point");
		return ENULL_POINT;
	}

	for(i = 0; i < MAX_FILE_WATCH_OBJ_NUMS; ++i)
	{
		if(g_inotify_watcher.objs[i].wd < 0)
			break;
	}

	if(i >= MAX_FILE_WATCH_OBJ_NUMS)
	{
		log_error("no valid objs");
		return EOUTBOUND;
	}

	int wd = inotify_add_watch(g_inotify_watcher.fd, path, mask);
	if(wd < 0)
	{
		log_error("inotify_add_watch failed(%s)", strerror(errno));
		return EINOTIFY_ADD;
	}

	g_inotify_watcher.objs[i].func = handler;
	g_inotify_watcher.objs[i].arg = arg;
	g_inotify_watcher.objs[i].wd = wd;

	return RET_SUCCESS;
}

int del_file_watch(int wd)
{
	if(wd < 0)
	{
		log_warning("invalid arg");
		return EINVAL_ARG;
	}

	if(inotify_rm_watch(g_inotify_watcher.fd, wd) < 0)
	{
		log_warning("inotify_rm_watch failed(%s)", strerror(errno));
		return EINOTIFY_DEL;
	}

	uint16_t i;
	for(i = 0; i < MAX_FILE_WATCH_OBJ_NUMS; ++i)
	{
		if(g_inotify_watcher.objs[i].wd == wd)
		{
			(*g_inotify_watcher.objs[i].func)(g_inotify_watcher.objs[i].arg, NULL);
			memset(&g_inotify_watcher.objs[i], 0, sizeof(FdWatchObj));
			g_inotify_watcher.objs[i].wd = -1;
		}
	}

	return RET_SUCCESS;
}

#if 0
int del_file_watch_by_path(const char *path)
{
	if(path == NULL)
	{
		log_warning("NULL point");
		return ENULL_POINT;
	}

	uint16_t i;
	for(i = 0; i < MAX_FILE_WATCH_OBJ_NUMS; ++i)
	{
		if(strcmp(path, ))
	}

	return RET_SUCCESS;
}
#endif


