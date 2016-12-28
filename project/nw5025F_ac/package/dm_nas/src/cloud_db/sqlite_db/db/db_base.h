#ifndef DB_BASE_H
#define DB_BASE_H

#include "sqlite3.h"
#include "base.h"

extern char g_database[128];
extern char g_backup_db[128];

#define	DATABASE g_database
#define BACKUP_DB g_backup_db

#define  SQL_CMD_WRITE_BUF_SIZE    2048
#define  SQL_CMD_QUERY_BUF_SIZE    1024//512


#define MAX_FILE_UUID_LEN	64
#define	MAX_FILE_NAME_LEN 	512
#define	MAX_FILE_PATH_LEN 	1024
#define	MAX_REAL_NAME_LEN 	16
#define	MAX_REAL_PATH_LEN 	32

#define PUBLIC_PATH 		"public"
#define PUBLIC_BUCKET_NAME 	"public_table"
#define PUBLIC_USER_NAME 	"public"
#define PUBLIC_PASSWORD  	"13141314"







typedef int error_t;

error_t sqlite3_exec_busy_wait(sqlite3 *db, const char *zSql, 
                             sqlite3_callback xCallback, void *pArg);

error_t start_transaction(sqlite3 *database);

error_t commit_transaction(sqlite3 *database);

error_t rollback_transaction(sqlite3 *database);

error_t create_db_connection(sqlite3 **database, uint8_t write_task);

error_t close_db_connection(sqlite3 *db);

char *sqlite3_str_escape(const char *str, char *dest, size_t size);

char *sqlite3_str_descape(const char *str, char *dest, size_t size);


#endif
