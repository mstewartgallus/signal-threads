#ifndef _MYLIBC_SIGNAL_H
#define _MYLIBC_SIGNAL_H 1

#include "internal/defines.h"

#include <stddef.h>

#include <asm-generic/signal.h>

#undef SIGRTMIN

struct __mylibc_sigset
{
    unsigned long __val[1024u / (8u * sizeof(unsigned long))];
};

typedef union __mylibc_siginfo
{
    struct
    {
        int si_signo;
        int si_errno;
        int si_code;
    } __fields;

    int __padding[128u / sizeof(int)];
} siginfo_t;

#define SIGRTMIN __mylibc_sigrtmin()

_MYLIBC_API extern int __mylibc_sigrtmin(void);

#endif
