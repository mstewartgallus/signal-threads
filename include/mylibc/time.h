#ifndef _MYLIBC_TIME_H
#define _MYLIBC_TIME_H 1

#include "sys/time.h"

_MYLIBC_API extern int nanosleep(struct timespec const *req,
                                 struct timespec *rem);

#endif
