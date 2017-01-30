#ifndef _MYLIBC_UNISTD_H
#define _MYLIBC_UNISTD_H 1

#include "internal/defines.h"
#include "internal/stddef.h"
#include "internal/sys/types.h"

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

_MYLIBC_API extern __mylibc_ssize_t write(int __fd, const void *__buf,
                                          __mylibc_size_t __n);

#endif
