#ifndef __DB_SERVER_PRCS_H__
#define __DB_SERVER_PRCS_H__

#if __cplusplus
extern "C" {
#endif
#include "config.h"
#include "base.h"
#include "task_base.h"
#include "db_update.h"

/*############################## Global Variable #############################*/
/*############################## Functions ###################################*/
typedef struct
{
    SystemConfigInfo sys_conf_info;
    SystemStatus sys_status;   
}SystemInfo;
SystemInfo g_sys_info;
void db_prcs_task(void);

#if __cplusplus
}
#endif

#endif /* __PROCESS_DEV_INFO_H__ */

