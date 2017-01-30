#include "_mylibc/futex.h"
#include "_mylibc/small_lock.h"

#include <errno.h>
#include <stdbool.h>

/* Simple mutex from Futexes Are Tricky by Ulrich Drepper */

enum { UNLOCKED = 0, LOCKED_WITH_NO_WAITERS, LOCKED_WITH_WAITERS };

void _mylibc_small_lock_acquire(__mylibc_small_lock_t *small_lock)
{
    int errnum;

    int last_state;
    {
        int expect_and_last = UNLOCKED;
        __atomic_compare_exchange_n(small_lock, &expect_and_last,
                                    LOCKED_WITH_NO_WAITERS, false,
                                    __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        if (UNLOCKED == expect_and_last) {
            goto observe_writes;
        }
        last_state = expect_and_last;
    }

    for (;;) {
        if (LOCKED_WITH_WAITERS == last_state) {
            do {
                errnum
                    = _mylibc_futex_wait(small_lock, LOCKED_WITH_WAITERS, NULL);
            } while (EINTR == errnum);
        }

        {
            int expect_and_last = LOCKED_WITH_NO_WAITERS;
            __atomic_compare_exchange_n(small_lock, &expect_and_last,
                                        LOCKED_WITH_WAITERS, false,
                                        __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
            if (expect_and_last != UNLOCKED) {
                do {
                    errnum = _mylibc_futex_wait(small_lock, LOCKED_WITH_WAITERS,
                                                NULL);
                } while (EINTR == errnum);
            }
        }

        {
            int expect_and_last = UNLOCKED;
            __atomic_compare_exchange_n(small_lock, &expect_and_last,
                                        LOCKED_WITH_WAITERS, false,
                                        __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
            if (UNLOCKED == expect_and_last) {
                break;
            }
            last_state = expect_and_last;
        }
    }

observe_writes:
    __atomic_thread_fence(__ATOMIC_ACQUIRE);
}

int _mylibc_small_lock_try_acquire(__mylibc_small_lock_t *small_lock)
{
    int expect_and_last = UNLOCKED;
    __atomic_compare_exchange_n(small_lock, &expect_and_last,
                                LOCKED_WITH_NO_WAITERS, false, __ATOMIC_SEQ_CST,
                                __ATOMIC_SEQ_CST);
    if (expect_and_last != UNLOCKED) {
        return EAGAIN;
    }

    __atomic_thread_fence(__ATOMIC_ACQUIRE);

    return 0;
}

int _mylibc_small_lock_release(__mylibc_small_lock_t *small_lock)
{
    __atomic_thread_fence(__ATOMIC_RELEASE);

    /* The lock must either be LOCKED_WITH_NO_WAITERS or
     * LOCKED_WITH_WAITERS.
     */

    /* This part is slightly modified from Ulrich Drepper's paper to
     * allow another thread to free an acquired lock.
     */
    for (;;) {
        int last_lock_state;
        {
            int expect_and_last = LOCKED_WITH_NO_WAITERS;
            __atomic_compare_exchange_n(small_lock, &expect_and_last, UNLOCKED,
                                        false, __ATOMIC_SEQ_CST,
                                        __ATOMIC_SEQ_CST);
            last_lock_state = expect_and_last;
        }
        switch (last_lock_state) {
        case UNLOCKED:
            return EPERM;

        case LOCKED_WITH_NO_WAITERS:
            goto unlocked_lock_no_wake;

        case LOCKED_WITH_WAITERS: {
            int expect_and_last = LOCKED_WITH_WAITERS;
            __atomic_compare_exchange_n(small_lock, &expect_and_last, UNLOCKED,
                                        false, __ATOMIC_SEQ_CST,
                                        __ATOMIC_SEQ_CST);
            switch (expect_and_last) {
            case UNLOCKED:
                return EPERM;

            case LOCKED_WITH_NO_WAITERS:
                continue;

            case LOCKED_WITH_WAITERS:
                goto unlocked_lock_wake_waiter;
            }
        }
        }
    }

unlocked_lock_wake_waiter:
    _mylibc_futex_wake(NULL, small_lock, 1);

unlocked_lock_no_wake:
    ;
    return 0;
}
