#ifndef DB_BASE_H
#define DB_BASE_H

#include "db/sqlite3.h"
#include "base.h"
#include "hidisk_errno.h"

extern char g_database[128];
extern char g_backup_db[128];

#define  DATABASE g_database
#define BACKUP_DB g_backup_db


#define  SQL_CMD_WRITE_BUF_SIZE    2048
#define  SQL_CMD_QUERY_BUF_SIZE    1024//512


error_t sqlite3_exec_busy_wait(sqlite3 *db, const char *zSql, 
                             sqlite3_callback xCallback, void *pArg);
error_t start_transaction(sqlite3 *database);
error_t commit_transaction(sqlite3 *database);
error_t rollback_transaction(sqlite3 *database);
error_t create_db_connection(sqlite3 **database, uint8_t write_task);
error_t create_db_disk_connection(sqlite3 **database, char *g_database,uint8_t write_con);
char *get_db_write_task_sql_buf(void);
int close_db_connection(sqlite3 *db);
char *sqlite3_str_escape(const char *str, char *dest, size_t size);
char *sqlite3_str_descape(const char *str, char *dest, size_t size);


#endif
