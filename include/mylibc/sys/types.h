#ifndef _MYLIBC_SYS_TYPES_H
#define _MYLIBC_SYS_TYPES_H 1

#include "../internal/sys/types.h"

typedef __mylibc_ssize_t ssize_t;
typedef __mylibc_socklen_t socklen_t;
typedef __mylibc_pid_t pid_t;

typedef long int time_t;
typedef long int suseconds_t;

#endif
