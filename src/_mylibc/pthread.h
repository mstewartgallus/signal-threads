#ifndef __MYLIBC_PTHREAD_H
#define __MYLIBC_PTHREAD_H 1

#include <ucontext.h>

struct __ssg_pthread
{
    ucontext_t ucontext;
    struct __ssg_pthread *volatile previous;
    struct __ssg_pthread *volatile next;
    struct __ssg_pthread *volatile waiter;
    void *volatile retval;
    int saved_errno;
    ucontext_t cancel_point;
    unsigned short cancelled : 1;
    unsigned short finished : 1;
    unsigned short yield_locked : 1;
};

extern struct __ssg_pthread *volatile __mylibc_pthread_current_thread;

void _mylibc_pthread_initialize(void);

static void _mylibc_pthread_yield_lock(void)
{
    __mylibc_pthread_current_thread->yield_locked = 1;
}

static void _mylibc_pthread_yield_unlock(void)
{
    __mylibc_pthread_current_thread->yield_locked = 0;
}

static unsigned char _mylibc_pthread_yield_is_locked(void)
{
    return __mylibc_pthread_current_thread->yield_locked;
}

void _mylibc_pthread_yield_unsafe(void);

#endif
