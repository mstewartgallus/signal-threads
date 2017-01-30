SOURCES=\
  src/main.c\
  src/errno.c\
  src/small_lock.c\
  src/stdio.c\
  src/pthread.c

OBJECTS=\
  src/main.o\
  src/errno.o\
  src/small_lock.o\
  src/stdio.o\
  src/pthread.o

HEADERS=\
  include/mylibc/errno.h\
  include/mylibc/pthread.h\
  include/mylibc/signal.h\
  include/mylibc/stdio.h\
  include/mylibc/stdlib.h\
  include/mylibc/string.h\
  include/mylibc/unistd.h\
  \
  include/mylibc/sys/mman.h\
  include/mylibc/sys/resource.h\
  include/mylibc/sys/syscall.h\
  include/mylibc/sys/time.h\
  \
  include/mylibc/internal/errno.h\
  include/mylibc/internal/pthread.h\
  include/mylibc/internal/stddef.h\
  include/mylibc/internal/sys/types.h\
  \
  src/_mylibc/futex.h\
  src/_mylibc/pthread.h\
  src/_mylibc/small_lock.h\
  src/_mylibc/string.h\
  src/_mylibc/syscall.h

CFLAGS=-Wl,--as-needed -Bsymbolic -Wl,--no-allow-shlib-undefined -ggdb3 -std=c89 -Wall -Wextra -Wc++-compat -Wno-missing-field-initializers -Wno-missing-braces -Wno-unused-parameter -O3
CC=gcc

# TOOD: Add something like  -Wl,--dynamic-linker='./src/libmylibc.so'
USER_CFLAGS=-nostdinc -nostdlib -nodefaultlibs -nostartfiles -Iinclude/mylibc -fPIE -pie
USER_LDFLAGS=-Wl,-z,now -Wl,-z,relro -shared -Wl,-z,nodefaultlib -Wl,-export-dynamic -Lsrc -fPIE -pie -lmylibc

all: test/main

src/%.o: src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -fvisibility=hidden -D_MYLIBC_DLL_EXPORTS=1 -fPIC -fPIE -fPIC -Wno-unused-function -Iinclude -Isrc -c -o $@ $<

src/libmylibc.so: Makefile $(OBJECTS)
	$(CC) $(CFLAGS) -Wl,-soname=libmylibc.so -Wl,-export-dynamic -fPIC -fPIE -pie -fPIC -o $@ $(OBJECTS) -lrt -ldl

test/main.o: Makefile test/main.c
	$(CC) $(CFLAGS) -c -o $@ test/main.c $(USER_CFLAGS)

test/main: Makefile test/main.o src/libmylibc.so $(HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ test/main.o $(USER_LDFLAGS)

.PHONY: indent
indent:
	find . -type f -name '*.[ch]' -print0 | xargs -0 -- clang-format-3.5 -i

clean:
	- rm -- $(OBJECTS) src/libmylibc.so test/main.o test/main
