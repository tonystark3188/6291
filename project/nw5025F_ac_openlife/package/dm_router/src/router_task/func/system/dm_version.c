#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FW_VERSION
#define FW_VERSION		"1.0.1.150414_beta"
#endif

int dm_get_proc_version(char *version)
{
	if(version!=NULL)
	{
		strcpy(version,FW_VERSION);
		return 0;
	}
	return -1;
}

