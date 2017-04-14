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
#include "router_inotify.h"

void *_scan_task_func(void *self)
{
	ENTER_FUNC();
	int enRet = 0;
    Message *p_msg = NULL;
	struct disk_node *disk_info = (struct disk_node *)self;
	if(disk_info == NULL)
	{
		DMCLOG_D("_scan_task_func failed!");
        return NULL;
	}
    p_msg = new_message();
    if(p_msg == NULL)
    {
        DMCLOG_D("new_message failed!");
        return NULL;
    }
	memset(&p_msg->msg_data, 0, sizeof(p_msg->msg_data));
	p_msg->msg_header.m_type = MSG_DB_SCANNING_DISK;
	strcpy(p_msg->msg_data.db_obj.data.scan_data.disk_path,disk_info->path);
    p_msg->msg_data.db_obj.data.scan_data.mutex = &disk_info->mutex;
	enRet = disk_info->g_db_write_task.msg_cb(&disk_info->g_db_write_task,p_msg);
	if(enRet < 0)
	{
		DMCLOG_D("handle msg failed(0x%x)", enRet);
	}
    free_message(&p_msg);
	//notify_disk_scan_status(1);/*??????????????????????????*/
	EXIT_FUNC();
    return NULL;
}

int create_scan_task(void *disk_info)
{
    _scan_task_func(disk_info);
	uint16_t sign = get_database_sign();
	sign++;
	set_database_sign(sign);
	dm_cycle_change_inotify(data_base_changed);
	return 0;
}



