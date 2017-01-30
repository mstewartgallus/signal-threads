#ifndef _MYLIBC_STDIO_H
#define _MYLIBC_STDIO_H 1

#include "internal/defines.h"

#define EOF (-1)

#define stdin __mylibc_stdin()
#define stdout __mylibc_stdout()
#define stderr __mylibc_stderr()

struct __mylibc_FILE;
typedef struct __mylibc_FILE FILE;

_MYLIBC_API extern struct __mylibc_FILE *__mylibc_stdin(void);
_MYLIBC_API extern struct __mylibc_FILE *__mylibc_stderr(void);
_MYLIBC_API extern struct __mylibc_FILE *__mylibc_stdout(void);

_MYLIBC_API extern void flockfile(struct __mylibc_FILE *__filehandle);
_MYLIBC_API extern int ftrylockfile(struct __mylibc_FILE *__filehandle);
_MYLIBC_API extern void funlockfile(struct __mylibc_FILE *__filehandle);

_MYLIBC_API extern int puts(char const *__s);

_MYLIBC_API extern void perror(char const *__s);

#endif
