#ifndef __MYLIBC_STRING_H
#define __MYLIBC_STRING_H 1

#include <stddef.h>

static size_t _mylibc_strlen(char const *str)
{
    size_t ii = 0u;

    for (;;) {
        char k = str[ii];
        if ('\0' == k) {
            break;
        }
        ++ii;
    }

    return ii;
}

static void *_mylibc_memset(void *s, int c, size_t n)
{
    char *x = (char *)s;
    size_t ii;

    for (ii = 0u; ii < n; ++ii) {
        x[ii] = c;
    }
    return s;
}

#endif
