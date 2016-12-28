
#include "db_backup.h"



void remove_db_file(const char *db_path)
{
    char s[256] = {0};

    rm(db_path);
    S_SNPRINTF(s, sizeof(s),"%s-wal", db_path);
    rm(s);
    S_SNPRINTF(s, sizeof(s), "%s-shm", db_path);
    rm(s);
}


error_t db_backup_retry(sqlite3 *database)
{
	error_t errcode = RET_SUCCESS;
	sqlite3 *backup_db;
	sqlite3_backup *pbackup;
	
	remove_db_file(BACKUP_DB);

	errcode= sqlite3_open_v2(BACKUP_DB, &backup_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	if(errcode != SQLITE_OK)
	{
		DMCLOG_E("open %s error when backup\n", BACKUP_DB);
		return errcode;
	}

	pbackup = sqlite3_backup_init(backup_db,"main",database,"main");
    if(pbackup == NULL)
    {
        DMCLOG_E("sqlite3_backup_init failed");
		sqlite3_close(backup_db);
		return EDB_BACKUP_INIT;
	}

	errcode = sqlite3_backup_step(pbackup,-1);
	if(errcode != SQLITE_DONE)
	{
        DMCLOG_E("sqlite3_backup_step failed(0x%X)",errcode);
		sqlite3_close(backup_db);
		return EDB;
	}

	sqlite3_backup_finish(pbackup);

	sqlite3_close(backup_db);
	
    //return errcode;
    return RET_SUCCESS;
}


error_t db_backup(sqlite3 *database)
{
	error_t errcode = RET_SUCCESS;
	sqlite3 *backup_db;
	sqlite3_backup *pbackup;
	errcode= sqlite3_open_v2(BACKUP_DB, &backup_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	if(errcode != SQLITE_OK)
	{
		DMCLOG_E("open %s error when backup\n", BACKUP_DB);
		return db_backup_retry(database);
	}

	pbackup = sqlite3_backup_init(backup_db,"main",database,"main");
    if(pbackup == NULL)
    {
        DMCLOG_E("sqlite3_backup_init failed");
		sqlite3_close(backup_db);
		return db_backup_retry(database);
	}

	errcode = sqlite3_backup_step(pbackup,-1);
	if(errcode != SQLITE_DONE)
	{
        DMCLOG_E("sqlite3_backup_step failed(0x%X)", errcode);
		sqlite3_close(backup_db);
		return db_backup_retry(database);
	}

	sqlite3_backup_finish(pbackup);

	sqlite3_close(backup_db);
	
    //return errcode;
    return RET_SUCCESS;
}

