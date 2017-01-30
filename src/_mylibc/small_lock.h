#ifndef __MYLIBC_SMALL_LOCK_H
#define __MYLIBC_SMALL_LOCK_H 1

typedef int __mylibc_small_lock_t;

#define __MYLIBC_SMALL_LOCK_INIT 0

void _mylibc_small_lock_acquire(__mylibc_small_lock_t *small_lock);
int _mylibc_small_lock_try_acquire(__mylibc_small_lock_t *small_lock);
int _mylibc_small_lock_release(__mylibc_small_lock_t *small_lock);

#endif
