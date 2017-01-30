#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>

static void *task_a(void *);
static void *task_b(void *);
static void *task_c(void *);

static void my_sleep(void);

int main(void)
{
    int errnum;
    pthread_t thread_a;
    pthread_t thread_b;
    pthread_t thread_c;

    puts("Hello world!");

    {
        pthread_t xx;
        if ((errnum = pthread_create(&xx, NULL, task_a, NULL)) != 0) {
            errno = errnum;
            perror("pthread_create");
            return EXIT_FAILURE;
        }
        thread_a = xx;
    }

    {
        pthread_t xx;
        if ((errnum = pthread_create(&xx, NULL, task_b, NULL)) != 0) {
            errno = errnum;
            perror("pthread_create");
            return EXIT_FAILURE;
        }
        thread_b = xx;
    }

    {
        pthread_t xx;
        if ((errnum = pthread_create(&xx, NULL, task_c, NULL)) != 0) {
            errno = errnum;
            perror("pthread_create");
            return EXIT_FAILURE;
        }
        thread_c = xx;
    }

    {
        struct timespec timespec;
        int errnum;

        timespec.tv_nsec = 40000000;
        timespec.tv_sec = 0;

        do {
            errnum = -1 == nanosleep(&timespec, &timespec) ? errno : 0;
        } while (EINTR == errnum);
    }

    pthread_cancel(thread_a);
    pthread_cancel(thread_b);
    pthread_cancel(thread_c);

    pthread_join(thread_a, NULL);
    pthread_join(thread_b, NULL);
    pthread_join(thread_c, NULL);

    return EXIT_SUCCESS;
}

static void *task_a(void *arg)
{
    for (;;) {
        puts("Fiber A!");
        my_sleep();
    }
}

static void *task_b(void *arg)
{
    for (;;) {
        puts("Fiber B!");
        my_sleep();
    }
}

static void *task_c(void *arg)
{
    for (;;) {
        puts("Fiber C!");
        my_sleep();
    }
}

static void my_sleep(void)
{
    struct timespec timespec;
    int errnum;

    timespec.tv_nsec = 20000000;
    timespec.tv_sec = 0;

    do {
        errnum = -1 == nanosleep(&timespec, &timespec) ? errno : 0;
    } while (EINTR == errnum);
}
