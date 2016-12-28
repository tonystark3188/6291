#ifndef __HIDISK_THREAD_H__
#define __HIDISK_THREAD_H__
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>

#define PTHREAD_T pthread_t
#define PTHREAD_ATTR_T pthread_attr_t
#define PTHREAD_MUTEX_T pthread_mutex_t
#define PTHREAD_COND_T pthread_cond_t

#define PTHREAD_MUTEX_INIT(MUTEX,ATTR) pthread_mutex_init(MUTEX,ATTR);
#define PTHREAD_COND_INIT(COND,ATTR) pthread_cond_init(COND,ATTR)
#define PTHREAD_ATTR_INIT(ATTR) pthread_attr_init(ATTR)
#define PTHREAD_ATTR_SETDETACHSTATE(ATTR,STATE) pthread_attr_setdetachstate(ATTR,STATE)
#define PTHREAD_CREATE(TIDP,ATTR,START_RTN,ARG) pthread_create(TIDP,ATTR,START_RTN,ARG)
#define PTHREAD_DETACH(TID) pthread_detach(TID)
#define PTHREAD_COND_SIGNAL(COND) pthread_cond_signal(COND)
#define PTHREAD_SELF() pthread_self()
#define PTHREAD_MUTEX_LOCK(MUTEX) pthread_mutex_lock(MUTEX)
#define PTHREAD_MUTEX_UNLOCK(MUTEX) pthread_mutex_unlock(MUTEX)

#define PTHREAD_COND_WAIT(COND,MUTEX) pthread_cond_wait(COND,MUTEX)
#define PTHREAD_EXIT(RETVAL) pthread_exit(RETVAL)
#define PTHREAD_COND_BROADCAST(COND) pthread_cond_broadcast(COND)
#define PTHREAD_JOIN(ID,RETVAL) pthread_join(ID,RETVAL)
#define PTHREAD_ATTR_DESTROY(ATTR) pthread_attr_destroy(ATTR)
#define PTHREAD_MUTEX_DESTORY(MUTEX) pthread_mutex_destroy(MUTEX)
#define PTHREAD_COND_DESTROY(COND) pthread_cond_destroy(COND)
#define PTHREAD_SETCANCELSTATE(STATE,OLDSTATE) pthread_setcancelstate(STATE,OLDSTATE)
#define PTHREAD_SETCANCELTYPE(TYPE,OLDTYPE) pthread_setcanceltype(TYPE,OLDTYPE)
#define PTHREAD_CLEANUP_PUSH(ROUTINE,ARG) pthread_cleanup_push(ROUTINE,ARG)
#define PTHREAD_CLEANUP_POP(EXCUTE) pthread_cleanup_pop(EXCUTE)

/*############################## Prototypes ##################################*/

#if __cplusplus
}
#endif

#endif /* __HIDISK_THREAD_H__ */

