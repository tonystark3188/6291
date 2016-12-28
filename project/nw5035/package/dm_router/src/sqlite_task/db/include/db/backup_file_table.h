#ifndef BACKUP_FILE_TABLE_H
#define BACKUP_FILE_TABLE_H

#include "db/device_table.h"
#include "db/file_table.h"
#include "base.h"
#include "hidisk_errno.h"

#define INVALID_BACKUP_FILE_ID 0xFFFFFFFF

#define MAX_LOG_SIZE 512
#define MAX_HASH_CODE_SIZE 128

typedef enum
{
	BACKUP_TYPE_ALL,
	BACKUP_TYPE_MESSAGE,
	BACKUP_TYPE_ADDRESS_LIST,
	BACKUP_TYPE_CALL_RECORD,
}backup_file_type_t;


typedef struct
{
	struct dl_list node;
	uint32_t id;
	char file_uuid[64];
	char name[BACKUP_PATH_SIZE];
	off_t size;
	int  file_type;
	unsigned backup_time;
	char device_uuid[MAX_USER_DEV_NAME_LEN];
	char path[2048];
	int file_status;// 1:备份完成,0:备份尚未完成
	int owner;
	file_uuid_list_t *flist;
}backup_file_info_t;
#define BACKUP_FILE_TABLE_UPDATE_STATE 0x01

typedef struct
{
	backup_file_info_t backup_info;
	uint8_t cmd;
}backup_file_update_t;

#if 0
void register_backup_file_table_ops(void);
error_t backup_file_table_init(char *g_database);
#endif
#endif
