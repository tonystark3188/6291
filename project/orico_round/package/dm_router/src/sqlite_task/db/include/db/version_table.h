#ifndef VERSION_TABLE_H
#define VERSION_TABLE_H

#include "base.h"
#include "stdbool.h"
#include "db/sqlite3.h"


#define MAX_VERSION_SIZE 16

typedef struct
{
	char version[MAX_VERSION_SIZE];
	uint8_t state;
}version_info_t;

typedef struct
{
	char    version[MAX_VERSION_SIZE]; //version number string
	uint8_t state;    //state   
	bool update_version; //go to update version?
	bool update_state;   //go to update state?
}update_version_t;



error_t version_table_query(sqlite3 *database, version_info_t *pvi);
error_t version_table_update(sqlite3 *database, update_version_t *puv);
error_t version_table_insert(sqlite3 *database, version_info_t *pvi);




#endif
