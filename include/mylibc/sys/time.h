#ifndef _MYLIBC_SYS_TIME_H
#define _MYLIBC_SYS_TIME_H 1

#include "types.h"

struct timespec
{
    time_t tv_sec;
    suseconds_t tv_nsec;
};

#endif
