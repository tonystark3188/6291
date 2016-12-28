/*
 * =============================================================================
 *
 *       Filename:  process_json.h
 *
 *    Description:  json process operation
 *
 *        Version:  1.0
 *        Created:  2015/7/2 11:00
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver (), 
 *   Organization:  
 *
 * =============================================================================
 */

#ifndef _PROCESS_JSON_H_
#define _PROCESS_JSON_H_

#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "base.h"



#ifdef __cplusplus
extern "C"{
#endif

int api_response(ClientTheadInfo *p_client_info);

int _parse_client_header_json(ClientTheadInfo *p_client_info);




#ifdef __cplusplus
}
#endif

#endif

