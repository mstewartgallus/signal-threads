#define _GNU_SOURCE

#include "_mylibc/pthread.h"
#include "_mylibc/pthread.h"
#include "_mylibc/start.h"

#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

extern char **environ;

int main(int argc, char **argv)
{
    _mylibc_pthread_initialize();

    if (argc < 2) {
        fprintf(stderr, "sorry no arg\n");
        return EXIT_FAILURE;
    }

    char const *self = argv[0U];
    char const *binary_name = argv[1U];

    dlerror();
    void *binary = dlopen(binary_name, RTLD_NOW | RTLD_GLOBAL | RTLD_NODELETE
                                       | RTLD_DEEPBIND);
    {
        char *err = dlerror();
        if (err != 0) {
		fprintf(stderr, "%s: dlopen: %s\n", self, err);
            return EXIT_FAILURE;
        }
    }

    dlerror();
    void *main_pointer = dlsym(binary, "main");
    {
        char *err = dlerror();
        if (err != 0) {
		fprintf(stderr, "%s: dlsym: %s\n", self, err);
            return EXIT_FAILURE;
        }
    }

    return ((int (*)(int, char **, char **))main_pointer)(argc - 1, argv + 1,
                                                          environ);
}
