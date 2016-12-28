/*
 * =============================================================================
 *
 *       Filename:  my_event_base.h
 *
 *    Description:  event base module
 *
 *        Version:  1.0
 *        Created:  2014/8/27 11:21:39
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ye Wenhao (), wenhaoye@126.com
 *   Organization:  
 *
 * =============================================================================
 */
#ifndef _MY_EVENT_BASE_H_
#define _MY_EVENT_BASE_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "base.h"

#define MAX_EVENT_NUMS 16
#define EVENT_READ      (1 << 0)
#define EVENT_WRITE     (1 << 1)
#define EVENT_EXC       (1 << 2)

#define INVALIED_EVENT_BASE_FD NULL

typedef void * event_base_t;
typedef int (*event_handler)(void *self);

#define EVENT_OBJ_NAME_SIZE     32

typedef struct
{
    char name[EVENT_OBJ_NAME_SIZE];
	int fd;
    unsigned int flag;
	void *arg;    
    event_handler e_handler;
	event_handler release;
    
    unsigned int e_flag;
}EventObj;


#define EVENT_SERVER_QUIT_MSG_ID             0
#define EVENT_SERVER_DB_ERROR_MSG_ID         1
#define EVENT_SERVER_MAX_CTRL_MSG_ID         2

typedef struct
{
    uint16_t ctrl_msg_id;
    char ctrl_msg_name[16];
    event_handler ctrl_msg_handler;
    void *ctrl_msg_handler_arg;
    
    void *data;
    uint16_t data_len;
}EventServerSubHandle;

event_base_t my_event_base_init(uint16_t max_fd_nums);
int my_event_base_exit(void);
char * my_event_base_loop(event_base_t base_fd);
int my_event_base_add(event_base_t base_fd, EventObj *event);

int start_event_task(event_base_t base_fd);
int add_event_server_ctrl_sub_handler(EventServerSubHandle *handler);
int send_msg_to_event_server(char *msg, size_t len);


#ifdef __cplusplus
}
#endif

#endif

