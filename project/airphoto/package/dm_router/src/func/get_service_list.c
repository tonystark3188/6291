/*
 * =============================================================================
 *
 *       Filename:  get_service_list.c
 *
 *    Description:  get router service infomation
 *
 *        Version:  1.0
 *        Created:  2015/8/12 17:21
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver
 *   Organization:  
 *
 * =============================================================================
 */

#include "get_service_list.h"
#include "defs.h"

int get_service_list(service_list_t *service_list)
{
	int i = 0;
	for(i = 0;i < SERVICE_CONT;i++)
	{
		if( i == 0)
		{
			strcpy(service_list->service_info[i].name,"init_service");
			service_list->service_info[i].port = INIT_PORT;
		}else if( i == 1)
		{
			strcpy(service_list->service_info[i].name,"router_service");
			service_list->service_info[i].port = ROUTER_PORT;
		}else if(i == 2){
			strcpy(service_list->service_info[i].name,"file_service");
			service_list->service_info[i].port = FILE_PORT;
		}else if(i == 3){
			strcpy(service_list->service_info[i].name,"backup_service");
			service_list->service_info[i].port = FILE_PORT;
		}else if(i == 4){
			strcpy(service_list->service_info[i].name,"category_service");
			service_list->service_info[i].port = FILE_PORT;
		}
		service_list->count++;
	}
	return 0;
}
