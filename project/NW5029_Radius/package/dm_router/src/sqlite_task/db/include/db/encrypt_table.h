#ifndef ENCRYPT_TABLE_H
#define ENCRYPT_TABLE_H

#include "base.h"
#include "stdbool.h"
#include "db/sqlite3.h"

#define ENCRYPT_TABLE_NAME "encrypt_table"
#define MAX_ENCRYPT_NAME_SIZE 512
#define MAX_ENCRYPT_PATH_SIZE 1024

typedef struct
{
    int id;
	char name[MAX_ENCRYPT_NAME_SIZE];
    char path[MAX_ENCRYPT_PATH_SIZE];
    long date;
    long long fileSize;
    int isDir;
}encrypt_info_t;

error_t get_max_encrypt_id(sqlite3 *database,int *max_id);
error_t encrypt_table_query(sqlite3 *database, encrypt_info_t *pInfo);
error_t encrypt_table_update(sqlite3 *database, encrypt_info_t *pInfo);
error_t encrypt_table_insert(sqlite3 *database, encrypt_info_t *pInfo);
error_t encrypt_table_delete(sqlite3 *database, encrypt_info_t *pInfo);




#endif
