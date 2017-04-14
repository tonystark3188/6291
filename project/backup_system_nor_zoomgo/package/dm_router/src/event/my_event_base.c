/*
 * =============================================================================
 *
 *       Filename:  my_event_base.c
 *
 *    Description:  event base module
 *
 *        Version:  1.0
 *        Created:  2014/8/27 11:21:14
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ye Wenhao (), wenhaoye@126.com
 *   Organization:  
 *
 * =============================================================================
 */
#include "event/my_event_base.h"
#include "task/task_base.h"
#include "file_opr.h"
#include <sys/select.h>
#include <sys/types.h>
#include "msg.h"

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif


typedef struct
{
	uint16_t max_fd_nums;
	uint16_t valid_fd_nums;
	uint8_t  loop_flag;
    EventObj *ready_events[MAX_EVENT_NUMS];
    uint16_t ready_events_cnt;
	EventObj event_objs[0];
}EventBaseObj;


#define EVENT_SERVER_SUN_PATH "/tmp/NasEvnSrv" // /var/run/NasEvnSrv
//#define EVENT_SERVER_SUN_PATH_MAX_LEN   80
#define EVENT_SERVER_MAX_BUF_LEN        64

typedef struct _EventServerControlInfo
{
    EventBaseObj *event_base;
    int fd;
    struct sockaddr_un s_addr;
    char buf[EVENT_SERVER_MAX_BUF_LEN];
    EventServerSubHandle sub_handlers[EVENT_SERVER_MAX_CTRL_MSG_ID];
}EventServerControlInfo;

static EventServerControlInfo g_event_server_ctrl_info;

static int _init_event_server_ctrl_info(void)
{
    memset(&g_event_server_ctrl_info, 0, sizeof(EventServerControlInfo));

    unlink(EVENT_SERVER_SUN_PATH);

    g_event_server_ctrl_info.fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(g_event_server_ctrl_info.fd < 0)
    {
        log_error("socket failed");
        return ESOCK;
    }

    g_event_server_ctrl_info.s_addr.sun_family = AF_UNIX;
    S_STRNCPY(g_event_server_ctrl_info.s_addr.sun_path, EVENT_SERVER_SUN_PATH, \
                UNIX_PATH_MAX);

    if(bind(g_event_server_ctrl_info.fd, (struct sockaddr *)&g_event_server_ctrl_info.s_addr, 
            sizeof(struct sockaddr)) < 0)
    {
        log_error("bind failed");
        safe_close(g_event_server_ctrl_info.fd);
        return ESOCK_BIND;
    }

#if 0
    if(listen(g_event_server_ctrl_info.fd, 3) < 0)
    {
        log_error("listen failed");
        safe_close(g_event_server_ctrl_info.fd);
        return ESOCK_LISTEN;
    }
#endif
    
    return RET_SUCCESS;
}

static int _event_server_ctrl_event_handler(void *self)
{
    EventObj *e = (EventObj *)(self);
    EventServerControlInfo *e_serv_info = (EventServerControlInfo *)(e->arg);

    log_trace("Enter:");
    if(!(e->e_flag & EVENT_READ))
    {
        log_error("e_flag has no EVENT_READ");
        return 1;
    }

    struct sockaddr_un cli_addr;
    socklen_t cli_addr_len;
    int ret;

    memset(e_serv_info->buf, 0, EVENT_SERVER_MAX_BUF_LEN);
    ret = recvfrom(e_serv_info->fd, e_serv_info->buf, EVENT_SERVER_MAX_BUF_LEN, 0,
                  (struct sockaddr *)&cli_addr, &cli_addr_len);
    if(ret < 0)
    {
        if(errno != EINTR && errno != EAGAIN)
        {
            log_error("recvfrom error(%d):%s", errno, strerror(errno));
            return ESOCK_RECV;
        }

        return RET_SUCCESS;
    }
    if(ret == 0)
    {
        log_warning("recvfrom 0 bytes");
        return 1;
    }
    else
    {
        log_debug("recv %d bytes:(%s)", ret, e_serv_info->buf);
    }

    // exit event task
    log_notice("Exit Event Task");

    uint16_t ctrl_msg_id = *((uint16_t *)e_serv_info->buf);
    if(ctrl_msg_id >= EVENT_SERVER_MAX_CTRL_MSG_ID)
    {
        log_error("ctrl_msg_id(%d) >= EVENT_SERVER_MAX_CTRL_MSG_ID(%d)", 
                    ctrl_msg_id, EVENT_SERVER_MAX_CTRL_MSG_ID);
        ASSERT(0);
        return RET_SUCCESS;
    }
    
    EventServerSubHandle *sub_handler = &(e_serv_info->sub_handlers[ctrl_msg_id]);
    if(sub_handler->ctrl_msg_handler != NULL)
    {
        sub_handler->data = e_serv_info->buf + sizeof(sub_handler->ctrl_msg_id);
        sub_handler->data_len = ret - sizeof(sub_handler->ctrl_msg_id);
        ret = (*sub_handler->ctrl_msg_handler)(sub_handler);

        sub_handler->data = NULL;
        sub_handler->data_len = 0;
    }

    log_trace("Exit");
    return ret;
}

static int _event_server_ctrl_event_release(void *self)
{
    EventObj *e = (EventObj *)(self);
    EventServerControlInfo *e_serv_info = (EventServerControlInfo *)(e->arg);

    log_trace("Enter");
    safe_close(e->fd);
    e_serv_info->fd = -1;
    log_trace("Exit");
    
    return RET_SUCCESS;
}

static int _add_event_server_ctrl_event(event_base_t base_fd)
{
    EventObj e;

    memset(&e, 0, sizeof(EventObj));
    e.fd = g_event_server_ctrl_info.fd;
    e.flag |= EVENT_READ;
    e.e_handler = _event_server_ctrl_event_handler;
    e.release = _event_server_ctrl_event_release;
    e.arg = &g_event_server_ctrl_info;
    S_STRNCPY(e.name, "EVENT-CTRL", sizeof(e.name));
    
    return my_event_base_add(base_fd, &e);
}

int send_msg_to_event_server(char *msg, size_t len)
{
    struct sockaddr_un s_addr;
    int fd;

    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(fd < 0)
    {
        log_error("socket failed");
        return ESOCK;
    }

    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sun_family = AF_UNIX;
    S_STRNCPY(s_addr.sun_path, EVENT_SERVER_SUN_PATH, UNIX_PATH_MAX);

    if(sendto(fd, msg, len, 0, (struct sockaddr *)&s_addr, sizeof(s_addr)) <= 0)
    {
        log_error("sendto failed");
        safe_close(fd);
        return ESOCK_SEND;
    }

    safe_close(fd);
    return RET_SUCCESS;
}

int add_event_server_ctrl_sub_handler(EventServerSubHandle *handler)
{
    if(handler == NULL)
    {
        log_warning("NULL point");
        return ENULL_POINT;
    }

    uint16_t ctrl_msg_id = handler->ctrl_msg_id;
    if(ctrl_msg_id >= EVENT_SERVER_MAX_CTRL_MSG_ID)
    {
        log_error("ctrl_msg_id(%d) >= EVENT_SERVER_MAX_CTRL_MSG_ID(%d)",
                    ctrl_msg_id, EVENT_SERVER_MAX_CTRL_MSG_ID);
        return EOUTBOUND;
    }

    if(g_event_server_ctrl_info.sub_handlers[ctrl_msg_id].ctrl_msg_handler == NULL)
    {
        memcpy(&g_event_server_ctrl_info.sub_handlers[ctrl_msg_id],
                handler, sizeof(EventServerSubHandle));
    }
    else
    {
        log_error("already use!");
        ASSERT(0);
        return EEVENT;
    }

    return RET_SUCCESS;
}

static int _handle_event_server_ctrl_quit_msg(void *self)
{
    EventServerSubHandle *handler = (EventServerSubHandle *)(self);
    EventServerControlInfo *event_server = (EventServerControlInfo *)(handler->ctrl_msg_handler_arg);

    log_warning("Quit Event Server");

    if(event_server->event_base != NULL)
    {
        event_server->event_base->loop_flag = 0;
    }

    return RET_SUCCESS;
}

static int _add_event_server_strl_quit_handler(void)
{
    EventServerSubHandle handler;
    memset(&handler, 0, sizeof(handler));

    handler.ctrl_msg_id = EVENT_SERVER_QUIT_MSG_ID;
    S_STRNCPY(handler.ctrl_msg_name, "Quit", sizeof(handler.ctrl_msg_name));
    handler.ctrl_msg_handler = _handle_event_server_ctrl_quit_msg;
    handler.ctrl_msg_handler_arg = &g_event_server_ctrl_info;
    
    return add_event_server_ctrl_sub_handler(&handler);
}

int my_event_base_exit(void)
{
    uint16_t msg = EVENT_SERVER_QUIT_MSG_ID;
    return send_msg_to_event_server((char *)(&msg), sizeof(msg));
}


/*
 * Desc: init my base event module
 *
 * max_fd_nums: input, the fd nums we want to monitor
 * Return: failed on -1, else other number. 
 */
event_base_t my_event_base_init(uint16_t max_fd_nums)
{
    int ret;
    if((ret = _init_event_server_ctrl_info()) != RET_SUCCESS)
    {
        log_error("_init_event_server_ctrl_info failed, ret(0x%x)", ret);
        return NULL;
    }

    if((ret = _add_event_server_strl_quit_handler()) != RET_SUCCESS)
    {
        log_error("_add_event_server_strl_quit_handler failed, ret(0x%x)", ret);
        return NULL;
    }
    
	if(max_fd_nums > MAX_EVENT_NUMS)
		max_fd_nums = MAX_EVENT_NUMS;

	EventBaseObj *event_base;
	int malloc_size = (sizeof(EventBaseObj) + sizeof(EventObj) * max_fd_nums);

	event_base = (EventBaseObj *)malloc(malloc_size);
	if(event_base == NULL)
	{
		log_warning("malloc failed!");
		return NULL;
	}
	
	memset(event_base, 0, malloc_size);
	event_base->max_fd_nums = max_fd_nums;
	event_base->loop_flag = 1;

	int i;
	for(i = 0; i < max_fd_nums; ++i)
	{
		event_base->event_objs[i].fd = -1;		
	}

    if((ret = _add_event_server_ctrl_event(event_base)) != RET_SUCCESS)
    {
        log_error("_add_event_server_ctrl_event failed, ret(0x%x)", ret);
        safe_free(event_base);
        return NULL;
    }

    g_event_server_ctrl_info.event_base = event_base;
	return (event_base_t)(event_base);
}

#if 0
/*
 * Desc: exit my base event module
 *
 * base_fd: input, the return value of my_event_base_init()
 * Return: success on 0.
 */
int my_event_base_exit(event_base_t base_fd)
{
	EventBaseObj *event_base = (EventBaseObj *)(base_fd);
	
	if(event_base == NULL)
	{
		log_warning("NULL pointer!");
		return EINVAL_ARG;
	}

	event_base->loop_flag = 0;
    while(event_base->valid_fd_nums)
    {
        log_trace("waiting my_event_base_loop exit...");
	    sleep(10);
    }

    safe_free(event_base);
	return 0;
}
#endif

/*
 * Desc: find valid event object space in arrays.
 *
 * event_base: input, point to EventBaseObj.
 * Return: success return on vaild index of arrays, else on -1.
 */
static int _find_valid_event_obj_space(EventBaseObj *event_base)
{
	int i;

	if(event_base->valid_fd_nums > event_base->max_fd_nums)
	{
		log_warning("no vaild event obj space!");
		return EOUTBOUND;
	}

	for(i = 0; i < event_base->max_fd_nums; ++i)
	{
		if(event_base->event_objs[i].fd < 0)
			break;
	}	
	
	if(i >= event_base->max_fd_nums)
		i = -1;

	return i;
}

/*
 * Desc: start my event base loop.
 *
 * base_fd: input,  point to EventBaseObj.
 */
char *my_event_base_loop(event_base_t base_fd)
{
	EventBaseObj *event_base = (EventBaseObj *)(base_fd);
	if(event_base == NULL)
	{
		log_warning("NULL pointer!");
        ASSERT(0);
		return NULL;
	}	

    log_trace("Enter");
	fd_set read_fds;
	fd_set write_fds;
	fd_set ex_fds;
	uint16_t i;
	int max_fd;
    EventObj *tmp_event;
    int ret;

    log_notice("start to select");
	while(event_base->loop_flag)
	{
        log_debug("begin");
		max_fd = 0;
		FD_ZERO(&read_fds);
		FD_ZERO(&write_fds);
		FD_ZERO(&ex_fds);
        event_base->ready_events_cnt = 0;
        memset(&event_base->ready_events[0], 0, MAX_EVENT_NUMS * sizeof(EventObj *));
        
		for(i = 0; i < event_base->max_fd_nums; ++i)
		{
			if(event_base->event_objs[i].fd < 0)
				continue;
			if(event_base->event_objs[i].flag & EVENT_READ)
				FD_SET(event_base->event_objs[i].fd, &read_fds);
			if(event_base->event_objs[i].flag & EVENT_WRITE)
				FD_SET(event_base->event_objs[i].fd, &write_fds);
			if(event_base->event_objs[i].flag & EVENT_EXC)
				FD_SET(event_base->event_objs[i].fd, &ex_fds);	
			max_fd = MAX(max_fd, event_base->event_objs[i].fd);	
		}

		if(select(max_fd+1, &read_fds, &write_fds, &ex_fds, NULL) < 0)
		{
			log_error("select failed!");
			break;
		}

		for(i = 0; i < event_base->max_fd_nums; ++i)
		{
			if(event_base->event_objs[i].fd < 0)
				continue;

            event_base->event_objs[i].e_flag = 0;
			if(FD_ISSET(event_base->event_objs[i].fd, &read_fds))
                event_base->event_objs[i].e_flag |= EVENT_READ;
			if(FD_ISSET(event_base->event_objs[i].fd, &write_fds))
                event_base->event_objs[i].e_flag |= EVENT_WRITE;
			if(FD_ISSET(event_base->event_objs[i].fd, &ex_fds))
                event_base->event_objs[i].e_flag |= EVENT_EXC;

            if(event_base->event_objs[i].e_flag > 0)
            {
                event_base->ready_events[event_base->ready_events_cnt] = &(event_base->event_objs[i]);
                event_base->ready_events_cnt++;   
            }
		}

        log_notice("ready_events_cnt(%d)", event_base->ready_events_cnt);
        for(i = 0; i < event_base->ready_events_cnt; ++i)
        {
            tmp_event = event_base->ready_events[i];
            if(tmp_event->e_handler)
            {
                log_notice("handle %s", tmp_event->name);
                ret = (*tmp_event->e_handler)(tmp_event);
                if(ret < 0)
                {
                    if(tmp_event->release)
                        (*tmp_event->release)(tmp_event);
                    event_base->valid_fd_nums -= 1;
                }
            }
        }

        log_debug("end");
	}

	for(i = 0; i < event_base->max_fd_nums; ++i)
	{
		if(event_base->event_objs[i].fd < 0)
			continue;
		
		//safe_close(event_base->event_objs[i].fd);
		if(event_base->event_objs[i].release)
			(*event_base->event_objs[i].release)(&event_base->event_objs[i]);
	}	

    event_base->valid_fd_nums = 0;
    safe_free(event_base);
    log_trace("Exit");
	return NULL;
}

/*
 * Desc: add new EventObj.
 *
 * base_fd: input, 
 * event: input
 * Return: success on 0.
 */
int my_event_base_add(event_base_t base_fd, EventObj *event)
{
	EventBaseObj *event_base = (EventBaseObj *)(base_fd);
    int i = -1;
    
	if(event_base == NULL)
	{
		log_error("NULL pointer!");
		return EINVAL_ARG;
	}

    i = _find_valid_event_obj_space(event_base);
	if(i < 0)
	{
		log_warning("_find_valid_event_obj_space failed");
		return EOUTBOUND;
	}	

	if(set_fd_nonblock(event->fd) < 0)
	{
		log_warning("set_fd_nonblock failed!");
		return EFCNTL;
	}

	memcpy(&event_base->event_objs[i], event, sizeof(EventObj));
	event_base->valid_fd_nums += 1;

	return RET_SUCCESS;
}

static BaseTask g_event_task;
#define EVENT_TASK_NAME "event_task"

static void * _event_task_cb(void *self)
{
    return my_event_base_loop(self);
}

int start_event_task(event_base_t base_fd)
{
    int ret;
    MEMSET(g_event_task);
    
    log_trace("Enter");

    if(base_fd == INVALIED_EVENT_BASE_FD)
    {
        log_error("invalid base_fd");
        return EINVAL_ARG;
    }
    
    g_event_task.task_arg = base_fd;
    g_event_task.task_func = _event_task_cb;
    S_STRNCPY(g_event_task.task_name, EVENT_TASK_NAME, sizeof(g_event_task.task_name));

    if((ret = create_base_task(&g_event_task)) != RET_SUCCESS)
    {
        log_error("create_base_task failed");
    }

    log_trace("Exit");
    return ret;
}



#if 0
int my_event_base_del(int base_fd)
{

}

int my_event_base_test(void)
{
	int eb_fd = my_event_base_init(10);
	if(eb_fd == -1)
	{
		printf("my_event_base_init() failed!\n");
		return -1;
	}

	EventObj event;
	memset(&event, 0, sizeof(EventObj));
//	event.fd = STDIN;


	my_event_base_loop(eb_fd);

	return 0;
}

#endif

