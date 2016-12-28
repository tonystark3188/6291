/*
 * =============================================================================
 *
 *       Filename: device_table.h
 *
 *    Description: device table related data structure definition.
 *
 *        Version:  1.0
 *        Created:  2014/09/17 
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Tao Sheng (), vanbreaker90@gmail.com
 *   Organization:  
 *
 * =============================================================================
 */


#ifndef DEVICE_TABLE_H
#define DEVICE_TABLE_H

#ifdef __cplusplus
extern "C"{
#endif


#include "base.h"
#include "hidisk_errno.h"
//#include "network/net_util.h"

//#define DEVICE_NAME_SIZE 80



typedef struct
{
    struct dl_list node;
	uint32_t id;
	char mac_addr[18];
	char device_name[MAX_USER_DEV_NAME_LEN];
	char session[32];
	char device_uuid[64];
	char disk_uuid[16];
	uint32_t owner;
}device_info_t;

error_t device_table_init(void);
void register_device_table_ops(void);


#ifdef __cplusplus
}
#endif

#endif
