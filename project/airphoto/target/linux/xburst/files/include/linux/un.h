#ifndef _LINUX_UN_H
#define _LINUX_UN_H
//#include <sys/socket.h>
#define UNIX_PATH_MAX	108
typedef unsigned short __kernel_sa_family_t;
typedef __kernel_sa_family_t sa_family_t;

struct sockaddr_un {
	sa_family_t sun_family;	/* AF_UNIX */
	char sun_path[UNIX_PATH_MAX];	/* pathname */
};

#endif /* _LINUX_UN_H */
