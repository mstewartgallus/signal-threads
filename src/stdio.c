#include "_mylibc/small_lock.h"
#include "_mylibc/string.h"

#include "mylibc/stdio.h"
#include "mylibc/unistd.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>

struct __mylibc_FILE
{
    __mylibc_small_lock_t lock;
    int fildes;
};

static struct __mylibc_FILE stdin_file
    = {__MYLIBC_SMALL_LOCK_INIT, STDIN_FILENO};
static struct __mylibc_FILE stdout_file
    = {__MYLIBC_SMALL_LOCK_INIT, STDOUT_FILENO};
static struct __mylibc_FILE stderr_file
    = {__MYLIBC_SMALL_LOCK_INIT, STDERR_FILENO};

static void inner_flockfile(struct __mylibc_FILE *__filehandle);
static int inner_ftrylockfile(struct __mylibc_FILE *__filehandle);
static void inner_funlockfile(struct __mylibc_FILE *__filehandle);

static int inner_fputs_unlocked(char const *s, struct __mylibc_FILE *stream);

struct __mylibc_FILE *__mylibc_stdin(void) { return &stdin_file; }

struct __mylibc_FILE *__mylibc_stderr(void) { return &stderr_file; }

struct __mylibc_FILE *__mylibc_stdout(void) { return &stdout_file; }

void flockfile(struct __mylibc_FILE *__filehandle)
{
    inner_flockfile(__filehandle);
}

int ftrylockfile(struct __mylibc_FILE *__filehandle)
{
    return inner_ftrylockfile(__filehandle);
}

void funlockfile(struct __mylibc_FILE *__filehandle)
{
    inner_funlockfile(__filehandle);
}

void perror(char const *__s)
{
    inner_flockfile(stderr);

    /* inner_fputs_unlocked(_mylibc_errno_str(errno), stderr); */
    inner_fputs_unlocked(": ", stderr);
    inner_fputs_unlocked(__s, stderr);
    inner_fputs_unlocked("\n", stderr);

    inner_funlockfile(stderr);
}

int puts(char const *__s)
{
    int res;

    inner_flockfile(stdout);

    if (EOF == (res = inner_fputs_unlocked(__s, stdout))) {
        goto unlock_file;
    }

    if (EOF == (res = inner_fputs_unlocked("\n", stdout))) {
        goto unlock_file;
    }

unlock_file:
    inner_funlockfile(stdout);
    return res;
}

static void inner_flockfile(struct __mylibc_FILE *__filehandle)
{
    _mylibc_small_lock_acquire(&__filehandle->lock);
}

static int inner_ftrylockfile(struct __mylibc_FILE *__filehandle)
{
    return EAGAIN == _mylibc_small_lock_try_acquire(&__filehandle->lock) ? -1
                                                                         : 0;
}

static void inner_funlockfile(struct __mylibc_FILE *__filehandle)
{
    _mylibc_small_lock_release(&__filehandle->lock);
}

static int poll_once(int fd, short events, short *reventsp, int timeout);

static int check_for_poll_error(int fd, short revents);

static int inner_fputs_unlocked(char const *s, struct __mylibc_FILE *stream)
{
    int errnum;

    int fildes = stream->fildes;

    size_t len = _mylibc_strlen(s);

    size_t bytes_wrote = 0u;
    while (bytes_wrote != len) {
        int xx = write(fildes, s + bytes_wrote, len - bytes_wrote);
        if (-1 == xx) {
            errnum = errno;
            assert(errnum != 0);
        } else {
            errnum = 0;
        }

        switch (errnum) {
        case 0:
            bytes_wrote += xx;
            break;

        case EINTR: {
            short revents;
            do {
                errnum = poll_once(fildes, POLLIN, &revents, -1);
            } while (EINTR == errnum);
            if (errnum != 0) {
                errno = errnum;
                return EOF;
            }

            if ((errnum = check_for_poll_error(fildes, revents)) != 0) {
                errno = errnum;
                return EOF;
            }
            break;
        }

        default:
            errno = errnum;
            return EOF;
        }
    }
    return len;
}

static int poll_once(int fd, short events, short *reventsp, int timeout)
{
    struct pollfd pollfd;
    int errnum;

    pollfd.fd = fd;
    pollfd.events = events;

    if (-1 == poll(&pollfd, 1u, timeout)) {
        errnum = errno;
        assert(errnum != 0);
        return errnum;
    }

    *reventsp = pollfd.revents;
    return 0;
}

static int check_for_poll_error(int fd, short revents)
{
    int errnum = 0;

    if ((revents & POLLNVAL) != 0) {
        errnum = EBADF;
    } else if ((revents & POLLHUP) != 0) {
        errnum = EPIPE;
    } else if ((revents & POLLERR) != 0) {
        int xx = errnum;
        socklen_t optlen = sizeof xx;
        if (-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &xx, &optlen)) {
            errnum = errno;
        } else {
            errnum = xx;
        }
    }

    return errnum;
}
