#include "db/db_base.h"
#include "hidisk_errno.h"
#include <stdio.h>


#define DB_TIMEOUT_VAL 3


char g_database[128] = {0};
char g_backup_db[128] = {0};
char g_root_dir[64] = {0};
unsigned char g_root_level = 3;


static char g_db_write_task_sql_buf[SQL_CMD_WRITE_BUF_SIZE];


char *get_db_write_task_sql_buf(void)
{
	return g_db_write_task_sql_buf;
}


error_t sqlite3_exec_busy_wait(sqlite3 *db, const char *zSql, 
                             sqlite3_callback xCallback, void *pArg)
{
    int  res;     
    char *ErrMsg;
    uint8_t try_times = 0;

sqlite_retry:
    res = sqlite3_exec(db,zSql,xCallback,pArg,&ErrMsg);
    if(res != SQLITE_OK)
    {
       if(res == SQLITE_BUSY)
       {
            ++try_times;
            DMCLOG_D("sqlite busy(%dth) when execute %s", try_times, zSql);
            sqlite3_free(ErrMsg);

            if(try_times < DB_TIMEOUT_VAL)
            {
                sleep(1);
                goto sqlite_retry;
            }
            else
            {
                DMCLOG_D("sqlite busy over %d times", DB_TIMEOUT_VAL);
            }
       }
       else
       {
            DMCLOG_D("sqlite3 error:%s\nsql=%s",ErrMsg, zSql);
            sqlite3_free(ErrMsg);
       }
       
#ifdef PLATFORM_X86
        // add by wenhao at 2014-11-22 for debug sql error.
//        log_sync();
        assert(0);
#endif
       return res;
    }

    //DMCLOG_D("excute sql(%s) success", zSql);
    return SQLITE_OK;
}



error_t start_transaction(sqlite3 *database)
{
    return sqlite3_exec_busy_wait(database, "begin", NULL, NULL);
}

error_t commit_transaction(sqlite3 *database)
{
    return sqlite3_exec_busy_wait(database, "commit", NULL, NULL);
}


error_t rollback_transaction(sqlite3 *database)
{
    return sqlite3_exec_busy_wait(database, "rollback", NULL, NULL);
}


int wal_hook_callback(void *arg, sqlite3 *db, const char *name, int npage)
{
	error_t errcode = SQLITE_OK;

	if((errcode = sqlite3_wal_checkpoint(db, NULL)) != SQLITE_OK)
	{
		log_warning("sqlite wal checkpoint error\n");
	}

	return errcode;
}


int close_db_connection(sqlite3 *db)
{
	int ret;
	int n = 0;

	while((ret = sqlite3_close(db))==SQLITE_BUSY)
	{
		sleep(1);
		n++;
		if(n>=DB_TIMEOUT_VAL)
		{
			break;
		}
	}

	if(ret == SQLITE_OK)
	{
		log_trace("close db ok");
	}
	else if(ret == SQLITE_BUSY)
	{
		log_warning("db is busy, unable to close");
	}
	else
	{
		log_warning("close db error(0x%X)",ret);
	}

	return ret;
}



error_t create_db_connection(sqlite3 **database, uint8_t write_con)
{
	error_t ret;

	ret = sqlite3_open(DATABASE, database);
    if(ret)
    {
        log_warning("Can't open database: %s\n", sqlite3_errmsg(*database));
	    return ret;
    }
	else
    {
        //log_trace("Opened database successfully\n");
		return RET_SUCCESS;
    }

	if(write_con)
	{
        if((ret = (int)sqlite3_wal_hook(*database, wal_hook_callback, NULL)) != SQLITE_OK)
		{
		    log_warning("register wal hook error");
			return ret;
		}
	}

	return ret;
}
error_t create_db_disk_connection(sqlite3 **database, char *g_database,uint8_t write_con)
{
	error_t ret;
	if(g_database == NULL)
	{
		return -1;
	}
	ret = sqlite3_open(g_database, database);
    if(ret)
    {
        log_warning("Can't open database: %s\n", sqlite3_errmsg(*database));
	    return ret;
    }
	else
    {
        //log_trace("Opened database successfully\n");
		return RET_SUCCESS;
    }

	if(write_con)
	{
        if((ret = (int)sqlite3_wal_hook(*database, wal_hook_callback, NULL)) != SQLITE_OK)
		{
		    log_warning("register wal hook error");
			return ret;
		}
	}

	return ret;
}


char *sqlite3_str_escape(const char *str, char *dest, size_t size)
{
    if(str == NULL)
    {
        dest[0] = '\0';
        return NULL;
    }

#if 0   
    int i;
    int j = 0;
    int len = strlen(str);
    for(i = 0; i < len; i++)
    {
        switch(str[i])
        {
            case '\'':
            case '\"':
                dest[j] = dest[j+1] = str[i];
                j+=2;
                break;
        #if 0
            case '/':
            case '_':
        #endif
            case '[':
            case ']':
            case '%':
            case '&': 
            case '(':
            case ')':
                dest[j] = '/';
                j++;
        
            default :
                dest[j] = str[i];
                j++;
        }
    }
    dest[j] = '\0';
#else
    sqlite3_snprintf(size, dest, "%q", str);

#endif

//    log_debug("str(%s)-->str_escape(%s)", str, dest);
    return dest;
}

#if 0
static int _is_escape(char c)
{
    int flag = 0;
    
    switch(c)
    {
    #if 0
        case '/':
        case '_':
    #endif
        case '[':
        case ']':
        case '%':
        case '&': 
        case '(':
        case ')': 
            flag = 1;
            break;

        default:
            flag = 0;
    }

    return flag;
}

char *sqlite3_str_descape(const char *str, char *dest, size_t size)
{
    if(str == NULL)
    {
        dest[0] = '\0';
        return NULL;
    }

    int i;
    int j = 0;
    int len = strlen(str);
    for(i = 0; i < len; ++i)
    {
        switch(str[i])
        {
            case '\'':
            case '\"':
                if(str[i+1] == str[i])
                {
                    ++i;
                }
                dest[j] = str[i];
                ++j;
                break;

            case '/':
                if(_is_escape(str[i+1]))
                {
                    ++i;
                }
                dest[j] = str[i];
                ++j;
                break;
                
            default:
                dest[j] = str[i];
                ++j;
        }
    }

    dest[j] = '\0';

    
    return dest;
}
#endif

