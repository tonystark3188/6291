#ifndef __MYSIGNAL_H
#define __MYSIGNAL_H
#include <signal.h>

typedef void (*MYSIGNALHANDLE)(int sig);//typedef int (*mysignalhandle)(int sig);------int func(int sig)
void signal_setup(MYSIGNALHANDLE func);

extern char *signal_str[];
#endif
