#ifndef __MYLIBC_FUTEX_H
#define __MYLIBC_FUTEX_H 1

#include "_mylibc/syscall.h"

#include "mylibc/errno.h"

#include <asm/unistd.h>
#include <linux/futex.h>
#include <linux/time.h>

#include <stddef.h>
#include <stdint.h>

static int _mylibc_futex_wait(int *uaddr, int val,
                              struct timespec const *timeout)
{
    return _mylibc_syscall_4(__NR_futex, NULL, (intptr_t)uaddr, FUTEX_WAIT, val,
                             (intptr_t)timeout);
}

static int _mylibc_futex_wake(unsigned *wokeupp, int *uaddr, int val)
{
    intptr_t xx;
    int errnum;

    if ((errnum = _mylibc_syscall_3(__NR_futex, &xx, (intptr_t)uaddr,
                                    FUTEX_WAKE, val)) != 0) {
        return errnum;
    }

    if (wokeupp != NULL) {
        *wokeupp = xx;
    }
    return 0;
}

#endif
