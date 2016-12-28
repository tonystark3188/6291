/*
 * =============================================================================
 *
 *       Filename:  category_task.h
 *
 *    Description: file category service.
 *
 *        Version:  1.0
 *        Created:  2015/9/30 15:08:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver ()
 *   Organization:  
 *
 * =============================================================================
 */
#ifndef _CATEGORY_TASK_H_
#define _CATEGORY_TASK_H_
#include "base.h"
#include "task/task_base.h"
#include <ctype.h>
#include "scan_task.h"
#include "db/db_base.h"
#include "dm_encrypt.h"
#include "file_opr.h"

#ifdef __cplusplus
extern "C"{
#endif
error_t dm_create_db_file(char *name,all_disk_t* mAll_disk_t);
error_t create_table_file(sqlite3 *database);
error_t create_table_driv(sqlite3 *database,disk_info_t *disk_info);





#ifdef __cplusplus
}
#endif


#endif

