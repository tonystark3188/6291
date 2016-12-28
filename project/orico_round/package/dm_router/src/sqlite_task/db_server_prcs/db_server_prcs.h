#ifndef __DB_SERVER_PRCS_H__
#define __DB_SERVER_PRCS_H__

#if __cplusplus
extern "C" {
#endif

#include "task/scan_task.h"
#include "config.h"
#include "base.h"
#include "task/task_base.h"

/*############################## Global Variable #############################*/

#ifdef DB_TASK
/*############################## Functions ###################################*/
typedef struct
{
    SystemConfigInfo sys_conf_info;
    SystemStatus sys_status;   
}SystemInfo;
SystemInfo g_sys_info;
void db_server_prcs_task(void);
#endif

#if __cplusplus
}
#endif

#endif /* __PROCESS_DEV_INFO_H__ */

