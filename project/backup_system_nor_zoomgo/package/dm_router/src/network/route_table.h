#ifndef _ROUTE_TABLE_H_
#define _ROUTE_TABLE_H_

#include "base.h"
//#include "network/net_util.h"



typedef enum
{
	DST_ADDR_STR=0,
	DST_ADDR_INT
}rt_cond_type_t;

typedef struct
{
	rt_cond_type_t type;
    char inf_name[8];
	union 
	{
		struct sockaddr_in sockaddr;
		char str_addr[INET_ADDRSTRLEN];
	}dest;
    
}rt_match_cond_t;

int get_route_gateway(rt_match_cond_t *c, void *buf, size_t sz);
int get_route_gateway_ext(rt_match_cond_t *c, void *buf, size_t sz);



#endif
