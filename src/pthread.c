#define _GNU_SOURCE

#include "mylibc/internal/pthread.h"
#include "_mylibc/pthread.h"

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <ucontext.h>
#include <unistd.h>

struct __ssg_pthread *volatile __mylibc_pthread_current_thread;

static struct __ssg_pthread main_thread;

static size_t default_stack_size;
static size_t page_size;
static char thread_switch_signal_stack[SIGSTKSZ];

static void initialize_context(struct __ssg_pthread *self);

static void thread_start_shim(struct __ssg_pthread *self,
                              void *(*start_routine)(void *), void *argument);

static void handle_fiber_switch_signal(int sig, siginfo_t *info,
                                       void *ucontext);

void _mylibc_pthread_initialize(void)
{
    {
        stack_t stack = {0};

        stack.ss_sp = thread_switch_signal_stack;
        stack.ss_size = sizeof thread_switch_signal_stack;

        sigaltstack(&stack, NULL);
    }

    {
        rlim_t rlim_cur;
        {
            struct rlimit rlim;

            getrlimit(RLIMIT_STACK, &rlim);
            rlim_cur = rlim.rlim_cur;
        }

        default_stack_size = RLIM_INFINITY == rlim_cur ? SIGSTKSZ : rlim_cur;
    }

    page_size = sysconf(_SC_PAGESIZE);

    main_thread.previous = &main_thread;
    main_thread.next = &main_thread;
    main_thread.waiter = NULL;
    main_thread.cancelled = 0U;
    main_thread.finished = 0U;
    main_thread.yield_locked = 0U;

    __mylibc_pthread_current_thread = &main_thread;

    {
        struct sigaction action = {0};

        action.sa_sigaction = handle_fiber_switch_signal;
        action.sa_flags = SA_RESTART | SA_ONSTACK | SA_SIGINFO;

        sigfillset(&action.sa_mask);

        sigaction(SIGALRM, &action, NULL);
    }

    {
        sigset_t sigset;
        sigemptyset(&sigset);

        sigaddset(&sigset, SIGALRM);

        sigprocmask(SIG_UNBLOCK, &sigset, NULL);
    }

    getcontext(&main_thread.cancel_point);
    if (main_thread.cancelled) {
        /* TODO: Wait for threads to exit */
        _Exit(EXIT_SUCCESS);
    }

    {
        struct itimerval val;
        long usec = 20000;

        val.it_interval.tv_usec = usec;
        val.it_interval.tv_sec = 0;

        val.it_value.tv_usec = usec;
        val.it_value.tv_sec = 0;

        setitimer(ITIMER_REAL, &val, NULL);
    }
}

int pthread_create(struct __ssg_pthread **threadp,
                   struct __ssg_pthread_attr const *attr,
                   void *(*start_routine)(void *), void *arg)
{
    void *stack;
    void *stack_bottom;
    struct __ssg_pthread *new_thread;
    size_t stack_size;
    int protect_status;
    int errnum;

    assert(NULL == attr);

    errnum = 0;

    stack_size = default_stack_size;
    {
        _mylibc_pthread_yield_lock();
        stack = mmap(NULL, stack_size + page_size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK | MAP_GROWSDOWN,
                     -1, 0u);
        _mylibc_pthread_yield_unlock();
    }
    errnum = MAP_FAILED == stack ? errno : 0;

    if (errnum != 0)
        return errnum;

    /* On x86 stacks grow down */

    /* Allocate guard page */
    {
        _mylibc_pthread_yield_lock();
        protect_status = mprotect((char *)stack, page_size, PROT_NONE);
        _mylibc_pthread_yield_unlock();
    }

    errnum = -1 == protect_status ? errno : 0;
    if (errnum != 0) {
        goto unmap_stack;
    }

    stack_bottom = stack + page_size;

    {
        _mylibc_pthread_yield_lock();
        new_thread = (struct __ssg_pthread *)malloc(sizeof *new_thread);
        _mylibc_pthread_yield_unlock();
    }

    errnum = NULL == new_thread ? errno : 0;
    if (errnum != 0) {
        goto unmap_stack;
    }

    new_thread->waiter = NULL;
    new_thread->cancelled = 0U;
    new_thread->finished = 0U;
    new_thread->yield_locked = 1U;
    new_thread->saved_errno = 0;

    {
        struct ucontext *ucontext = &new_thread->ucontext;

        getcontext(ucontext);
        ucontext->uc_stack.ss_sp = stack_bottom;
        ucontext->uc_stack.ss_size = stack_size;
        makecontext(ucontext, (void (*)(void))thread_start_shim, 4, new_thread,
                    start_routine, arg);
    }

    {
        struct __ssg_pthread *self;
        struct __ssg_pthread *next;

        _mylibc_pthread_yield_lock();

        self = __mylibc_pthread_current_thread;
        next = self->next;

        new_thread->previous = self;
        new_thread->next = next;

        self->next = new_thread;
        next->previous = new_thread;

        /* Yield because we may have blocked it */
        _mylibc_pthread_yield_unsafe();

        _mylibc_pthread_yield_unlock();
    }

    if (errnum != 0) {
        _mylibc_pthread_yield_lock();
        free(new_thread);
        _mylibc_pthread_yield_unlock();
    }

unmap_stack:
    if (errnum != 0) {
        _mylibc_pthread_yield_lock();
        munmap(stack, stack_size + page_size);
        _mylibc_pthread_yield_unlock();
    }

    if (0 == errnum) {
        *threadp = new_thread;
    }

    return errnum;
}

int pthread_join(struct __ssg_pthread *thread, void **retval)
{
    struct __ssg_pthread *self;
    struct __ssg_pthread *previous;
    struct __ssg_pthread *next;
    int errnum;

    assert(thread != NULL);

    errnum = 0;

    _mylibc_pthread_yield_lock();

    self = __mylibc_pthread_current_thread;

    assert(self != NULL);

    if (thread == self || self->waiter == thread) {
        errnum = EDEADLK;
        goto unlock_yields;
    }

    if (thread->waiter != NULL) {
        errnum = EINVAL;
        goto unlock_yields;
    }

    if (thread->finished) {
        goto get_return_value;
    }

    previous = self->previous;
    next = self->next;

    assert(previous != NULL);
    assert(next != NULL);

    assert(next != self);
    assert(previous != self);

    next->previous = previous;
    previous->next = next;

    self->previous = NULL;
    self->next = NULL;

    thread->waiter = self;

    self->saved_errno = errno;
    initialize_context(next);
    swapcontext(&self->ucontext, &next->ucontext);

get_return_value:
    if (retval != NULL)
        *retval = thread->retval;

unlock_yields:
    /* Yield because we may have blocked it */
    _mylibc_pthread_yield_unsafe();

    _mylibc_pthread_yield_unlock();

    assert(thread != NULL);

    if (0 == errnum) {
        _mylibc_pthread_yield_lock();
        munmap(thread->ucontext.uc_stack.ss_sp - page_size,
               thread->ucontext.uc_stack.ss_size + page_size);
        free(thread);
        _mylibc_pthread_yield_unlock();
    }

    return errnum;
}

int pthread_cancel(struct __ssg_pthread *thread)
{
    struct __ssg_pthread *self;

    self = __mylibc_pthread_current_thread;

    _mylibc_pthread_yield_lock();

    thread->cancelled = 1U;

    self->saved_errno = errno;
    initialize_context(thread);
    swapcontext(&self->ucontext, &thread->cancel_point);

    _mylibc_pthread_yield_unlock();

    return 0;
}

int sched_yield(void)
{
    /* Prevent the timer from yielding for us */
    _mylibc_pthread_yield_lock();

    _mylibc_pthread_yield_unsafe();

    _mylibc_pthread_yield_unlock();

    return 0;
}

void _mylibc_pthread_yield_unsafe(void)
{
    struct __ssg_pthread *self;
    struct __ssg_pthread *new_thread;

    self = __mylibc_pthread_current_thread;
    new_thread = self->next;

    assert(self != NULL);
    assert(new_thread != NULL);

    if (self != new_thread) {
        self->saved_errno = errno;
        initialize_context(new_thread);

        swapcontext(&self->ucontext, &new_thread->ucontext);
    }
}

static void thread_start_shim(struct __ssg_pthread *self,
                              void *(*start_routine)(void *), void *argument)
{
    void *retval;

    assert(self != NULL);

    getcontext(&self->cancel_point);
    if (self->cancelled) {
        retval = NULL;
    } else {
        _mylibc_pthread_yield_unlock();

        retval = start_routine(argument);
    }

    _mylibc_pthread_yield_lock();

    {
        ucontext_t *ucontext;
        struct __ssg_pthread *previous;
        struct __ssg_pthread *next;
        struct __ssg_pthread *waiter;
        struct __ssg_pthread *new_current_thread;

        ucontext = &self->ucontext;

        waiter = self->waiter;
        previous = self->previous;
        next = self->next;

        assert(previous != NULL);
        assert(next != NULL);

        self->finished = 1U;
        self->retval = retval;
        self->next = NULL;
        self->previous = NULL;

        {
            stack_t stack;
            ucontext_t empty_ucontext = {0};

            stack = ucontext->uc_stack;

            *ucontext = empty_ucontext;

            ucontext->uc_stack = stack;
        }

        if (NULL == waiter) {
            assert(next != self);
            assert(previous != self);

            previous->next = next;
            next->previous = previous;

            new_current_thread = next;
        } else {
            if (next == self) {
                assert(previous == self);

                previous = waiter;
                next = waiter;
            }

            waiter->previous = previous;
            waiter->next = next;

            previous->next = waiter;
            next->previous = waiter;

            new_current_thread = waiter;
        }

        initialize_context(new_current_thread);
        setcontext(&new_current_thread->ucontext);
    }
}

static void handle_fiber_switch_signal(int sig, siginfo_t *info,
                                       void *untyped_ucontextp)
{
    struct __ssg_pthread *self;
    struct __ssg_pthread *new_thread;
    ucontext_t *ucontextp;

    ucontextp = (ucontext_t *)untyped_ucontextp;

    if (_mylibc_pthread_yield_is_locked()) {
        return;
    }

    self = __mylibc_pthread_current_thread;
    new_thread = self->next;

    if (self != new_thread) {
        self->ucontext = *ucontextp;
#ifdef __linux__
        self->ucontext.uc_mcontext.fpregs = &self->ucontext.__fpregs_mem;
#endif

        self->saved_errno = errno;

        initialize_context(new_thread);

#ifdef __linux__
        setcontext(&new_thread->ucontext);
#else
        *ucontextp = new_thread->ucontext;
#endif
    }
}

static void initialize_context(struct __ssg_pthread *self)
{
    errno = self->saved_errno;
    __mylibc_pthread_current_thread = self;
}

static void diff(struct timespec const *later, struct timespec const *older,
                 struct timespec *result);

int nanosleep(struct timespec const *req, struct timespec *rem)
{
    struct timespec old_time;
    struct timespec current_time;
    struct timespec time_difference;
    int errnum;

    errnum = 0;

    clock_gettime(CLOCK_REALTIME, &old_time);

    errnum = -1 == pselect(0, NULL, NULL, NULL, req, NULL) ? errno : 0;

    clock_gettime(CLOCK_REALTIME, &current_time);

    diff(&current_time, &old_time, &time_difference);

    diff(req, &time_difference, rem);

    if (errnum != 0) {
        errno = errnum;
        return -1;
    }

    return 0;
}

static void diff(struct timespec const *later, struct timespec const *older,
                 struct timespec *result)
{
    time_t seconds_delta;
    suseconds_t nanoseconds_delta;

    seconds_delta = later->tv_sec - older->tv_sec;
    nanoseconds_delta = later->tv_nsec - older->tv_nsec;

    if (nanoseconds_delta < 0) {
        --seconds_delta;
        nanoseconds_delta += 1000000000;
    }

    result->tv_sec = seconds_delta;
    result->tv_nsec = nanoseconds_delta;
}
