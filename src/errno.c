#include "mylibc/errno.h"

#include <errno.h>

errno_t volatile *__mylibc_errno(void) { return &errno; }
