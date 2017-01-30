#ifndef _MYLIBC_INTERNAL_PTHREAD_H
#define _MYLIBC_INTERNAL_PTHREAD_H 1

#include "defines.h"

struct __ssg_pthread;

struct __ssg_pthread_attr
{
    int dummy;
};

_MYLIBC_API extern int pthread_create(struct __ssg_pthread **pthread,
                                      struct __ssg_pthread_attr const *attr,
                                      void *(*start_routine)(void *),
                                      void *arg);

_MYLIBC_API extern int pthread_join(struct __ssg_pthread *thread,
                                    void **retval);

_MYLIBC_API extern int pthread_cancel(struct __ssg_pthread *thread);

_MYLIBC_API extern int sched_yield(void);

#endif
