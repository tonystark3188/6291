#ifndef _DB_MM_POOL_H_
#define _DB_MM_POOL_H_

#include "db/file_table.h"
#include "base.h"

file_info_t *new_db_fd(void);
file_info_t *new_db_fd_no_wait(void);
int free_db_fd(file_info_t **pp_db_fd);



#endif
