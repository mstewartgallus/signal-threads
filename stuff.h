/*
 * Many-to-one preemptive userspace threads.
 *
 * BUGS
 *
 * The use of SIGALRM and ITIMER_REAL is reserved by the
 * implementation.
 *
 * There's still a bunch of thread local state that is improperly
 * global which breaks things.
 *
 * - thread local variables (the .tdata section should be swapped in
 *   and out depending on the thread.)
 *
 * - the thread-specific data area
 * - pending signals
 * - the sigaltstack
 * - the TID
 *
 * Strictly speaking, the only functions callable without locking are
 * asynch-signal safe ones but most thread safe functions should be
 * callable (although that depends on the implementation).
 *
 * Also, nonblocking direct system calls such as mmap should probably
 * be async-signal-safe but they are aren't guaranteed to be. Blocking
 * direct system calls are less likely to be async-signal-safe because
 * weird stuff for interruptibility is done by the implementation.
 *
 * Below is a list of functions that are probably safe to call.
 *
 * If POSIX.1-2004 is supported (2004 guarantees a few extras but they
 * were removed in 2008 so they probably shouldn't be trusted):
 *
 * _Exit()
 * _exit()
 * abort()
 * accept()
 * access()
 * aio_error()
 * aio_return()
 * aio_suspend()
 * alarm()
 * bind()
 * cfgetispeed()
 * cfgetospeed()
 * cfsetispeed()
 * cfsetospeed()
 * chdir()
 * chmod()
 * chown()
 * clock_gettime()
 * close()
 * connect()
 * creat()
 * dup()
 * dup2()
 * execle()
 * execve()
 * fchmod()
 * fchown()
 * fcntl()
 * fdatasync()
 * fork()
 * fstat()
 * fsync()
 * ftruncate()
 * getegid()
 * geteuid()
 * getgid()
 * getgroups()
 * getpeername()
 * getpgrp()
 * getpid()
 * getppid()
 * getsockname()
 * getsockopt()
 * getuid()
 * kill()
 * link()
 * listen()
 * lseek()
 * lstat()
 * mkdir()
 * mkfifo()
 * open()
 * pause()
 * pipe()
 * poll()
 * posix_trace_event()
 * pselect()
 * raise()
 * read()
 * readlink()
 * recv()
 * recvfrom()
 * recvmsg()
 * rename()
 * rmdir()
 * select()
 * sem_post()
 * send()
 * sendmsg()
 * sendto()
 * setgid()
 * setpgid()
 * setsid()
 * setsockopt()
 * setuid()
 * shutdown()
 * sigaction()
 * sigaddset()
 * sigdelset()
 * sigemptyset()
 * sigfillset()
 * sigismember()
 * signal()
 * sigpause()
 * sigpending()
 * sigprocmask()
 * sigqueue()
 * sigset()
 * sigsuspend()
 * sleep()
 * sockatmark()
 * socket()
 * socketpair()
 * stat()
 * symlink()
 * tcdrain()
 * tcflow()
 * tcflush()
 * tcgetattr()
 * tcgetpgrp()
 * tcsendbreak()
 * tcsetattr()
 * tcsetpgrp()
 * time()
 * timer_getoverrun()
 * timer_gettime()
 * timer_settime()
 * times()
 * umask()
 * uname()
 * unlink()
 * utime()
 * wait()
 * waitpid()
 * write()
 *
 * If POSIX.1-2008 is supported:
 *
 * execl()
 * execv()
 * faccessat()
 * fchmodat()
 * fchownat()
 * fexecve()
 * fstatat()
 * futimens()
 * linkat()
 * mkdirat()
 * mkfifoat()
 * mknod()
 * mknodat()
 * openat()
 * readlinkat()
 * renameat()
 * symlinkat()
 * unlinkat()
 * utimensat()
 * utimes()
 */
