/*
 * Copyright (c) 2004-2005 Sergey Lyubka <valenok@gmail.com>
 * All rights reserved
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Sergey Lyubka wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */

#include "defs.h"
#include "base.h"

void 
set_close_on_exec(int fd)
{
	(void) fcntl(fd, F_SETFD, FD_CLOEXEC);
}

int
my_stat(const char *path, struct stat *stp)
{
	return (stat(path, stp));
}

int
my_open(const char *path, int flags, int mode)
{
	return (open(path, flags, mode));
}





int
set_non_blocking_mode(int fd)
{
	int	ret = -1;
	int	flags;

	if ((flags = fcntl(fd, F_GETFL, 0)) == -1) {
		DMCLOG_D("nonblock: fcntl(F_GETFL): %d", ERRNO);
	} else if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) != 0) {
		DMCLOG_D("nonblock: fcntl(F_SETFL): %d", ERRNO);
	} else {
		ret = 0;	/* Success */
	}

	return (ret);
}
