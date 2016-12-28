/*
 * =============================================================================
 *
 *       Filename:  scan_task.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2015/9/24 16:45:38
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), wenhaoye@126.com
 *   Organization:  
 *
 * =============================================================================
 */
#include "base.h"
#include "task/task_base.h"
#include <ctype.h>
#include "disk_manage.h"

//static TaskObj g_scan_task;


void scan_disk_to_db(disk_info_t *disk_info,Message *p_msg)
{
    //DMCLOG_D("disk_path = %s,file_table_name = %s", disk_info->path,disk_info->file_table_name);
	char *pfi = &p_msg->msg_data.db_obj.data.scan_data.disk_path;
	strcpy(pfi,disk_info->path);
}

void *scan_task_func(void *self)
{
	ENTER_FUNC();
    Message *p_msg = NULL;
    SemObj *p_semobj = NULL;
	disk_info_t *disk_info = (disk_info_t *)self;
	//DMCLOG_D("disk_info->file_table_name = %s",disk_info->file_table_name);
    p_msg = new_message();
    if(p_msg == NULL)
    {
        DMCLOG_D("new_message failed!");
        return NULL;
    }
    p_msg->msg_header.ret_flag = MSG_SYN_RETURN;
    p_semobj = get_semobj();
    if(p_semobj == NULL)
    {
        DMCLOG_D("get_semobj failed!");
        return NULL;
    }
    p_msg->msg_header.m_syn = p_semobj;
	memset(&p_msg->msg_data, 0, sizeof(p_msg->msg_data));
	p_msg->msg_header.m_type = MSG_DB_SCANNING_DISK;
	scan_disk_to_db(disk_info,p_msg); 
    send_message_sync(p_msg);

    DMCLOG_D("msg response: msg_data(%d), m_type(%d)", p_msg->msg_data,p_msg->msg_header.m_type);
    free_message(&p_msg);
    free_semobj(&p_semobj);
	safe_free(self);
	notify_disk_scan_status(1);
	EXIT_FUNC();
    return NULL;
}
void *_scan_task_func(void *self)
{
	ENTER_FUNC();
	int enRet = 0;
    Message *p_msg = NULL;
	struct disk_node *disk_info = (struct disk_node *)self;
    p_msg = new_message();
    if(p_msg == NULL)
    {
        DMCLOG_D("new_message failed!");
        return NULL;
    }
	memset(&p_msg->msg_data, 0, sizeof(p_msg->msg_data));
	p_msg->msg_header.m_type = MSG_DB_SCANNING_DISK;
	strcpy(&p_msg->msg_data.db_obj.data.scan_data.disk_path,disk_info->path);
	enRet = disk_info->g_db_write_task.msg_cb(&disk_info->g_db_write_task,p_msg);

	//ret = (*task->msg_cb)(task, p_msg);	
	if(enRet < 0)
	{
		DMCLOG_D("handle msg failed(0x%x)", enRet);
	}
    free_message(&p_msg);
	safe_free(self);
	notify_disk_scan_status(1);
	EXIT_FUNC();
    return NULL;
}



int create_scan_task(void *disk_info)
{
	int enRet;
	BaseTask g_scan_task;
	MEMSET(g_scan_task);
	strcpy(g_scan_task.task_name, "scan_task");
	g_scan_task.task_func = _scan_task_func;
	g_scan_task.task_arg = (struct disk_node *)malloc(sizeof(struct disk_node));
	memcpy(g_scan_task.task_arg,disk_info,sizeof(struct disk_node));
	enRet = create_base_task(&g_scan_task);
	if(enRet < 0)
	{
		 DMCLOG_D("create_scan_task failed!");
		 return enRet;
	}
	return 0;
}



