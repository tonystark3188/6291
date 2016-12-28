#ifndef DB_BACKUP_H
#define DB_BACKUP_H

#include "base.h"
#include "db/sqlite3.h"
#include "hidisk_errno.h"

error_t db_backup(sqlite3 *database);
void remove_db_file(const char *db_path);


#endif
