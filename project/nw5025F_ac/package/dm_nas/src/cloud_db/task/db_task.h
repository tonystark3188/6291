#ifndef DB_TASK_H_
#define DB_TASK_H_

#include "cloud_errno.h"
#include "db_base.h"
#include "task_base.h"

TaskObj g_db_query_task;
TaskObj g_db_write_task;

error_t get_db_errcode(void);
void recover_db_task(void);

int close_db_all_connection(void);
int create_db_all_connection(void);
int restart_db(void);

int brocast_db_error(void);


#endif
