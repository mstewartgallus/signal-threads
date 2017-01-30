#ifndef __MYLIBC_SYSCALL_H
#define __MYLIBC_SYSCALL_H 1

#include "mylibc/internal/errno.h"
#include "mylibc/internal/stddef.h"

#define __mylibc_syscall_LOAD_REGS_0()
#define __mylibc_syscall_LOAD_REGS_1(arg1)                                     \
    register __mylibc_intptr_t r_1 __asm__("rdi") = arg1;

#define __mylibc_syscall_LOAD_REGS_2(arg1, arg2)                               \
    __mylibc_syscall_LOAD_REGS_1(arg1);                                        \
    register __mylibc_intptr_t r_2 __asm__("rsi") = arg2;

#define __mylibc_syscall_LOAD_REGS_3(arg1, arg2, arg3)                         \
    __mylibc_syscall_LOAD_REGS_2(arg1, arg2);                                  \
    register __mylibc_intptr_t r_3 __asm__("rdx") = arg3;

#define __mylibc_syscall_LOAD_REGS_4(arg1, arg2, arg3, arg4)                   \
    __mylibc_syscall_LOAD_REGS_3(arg1, arg2, arg3);                            \
    register __mylibc_intptr_t r_4 __asm__("r10") = arg4;

#define __mylibc_syscall_LOAD_REGS_5(arg1, arg2, arg3, arg4, arg5)             \
    __mylibc_syscall_LOAD_REGS_4(arg1, arg2, arg3, arg4);                      \
    register __mylibc_intptr_t r_5 __asm__("r8") = arg5;

#define __mylibc_syscall_LOAD_REGS_6(arg1, arg2, arg3, arg4, arg5, arg6)       \
    __mylibc_syscall_LOAD_REGS_5(arg1, arg2, arg3, arg4, arg5);                \
    register __mylibc_intptr_t r_6 __asm__("r9") = arg6;

#define __mylibc_syscall_INPUT_REGS_0
#define __mylibc_syscall_INPUT_REGS_1 , "r"(r_1)
#define __mylibc_syscall_INPUT_REGS_2 __mylibc_syscall_INPUT_REGS_1, "r"(r_2)
#define __mylibc_syscall_INPUT_REGS_3 __mylibc_syscall_INPUT_REGS_2, "r"(r_3)
#define __mylibc_syscall_INPUT_REGS_4 __mylibc_syscall_INPUT_REGS_3, "r"(r_4)
#define __mylibc_syscall_INPUT_REGS_5 __mylibc_syscall_INPUT_REGS_4, "r"(r_5)
#define __mylibc_syscall_INPUT_REGS_6 __mylibc_syscall_INPUT_REGS_5, "r"(r_6)

#define _MYLIBC_IMPLEMENT_SYSCALL(syscall_number, resultp, nr, ...)            \
    do {                                                                       \
        __mylibc_syscall_LOAD_REGS_##nr(__VA_ARGS__);                          \
                                                                               \
        register __mylibc_intptr_t r_result_and_number __asm__("rax")          \
            = syscall_number;                                                  \
        __asm__ volatile(                                                      \
            "syscall\n"                                                        \
            : "=r"(r_result_and_number)                                        \
            : "r"(r_result_and_number)__mylibc_syscall_INPUT_REGS_##nr         \
            : "memory", "cc", "r11", "rcx");                                   \
        if (-4095 <= r_result_and_number && r_result_and_number < 0)           \
            return -r_result_and_number;                                       \
                                                                               \
        if (resultp != 0)                                                      \
            *resultp = r_result_and_number;                                    \
        return 0;                                                              \
    } while (0)

static int _mylibc_syscall_0(__mylibc_intptr_t syscall_nr,
                             __mylibc_intptr_t *resultp)
{
    _MYLIBC_IMPLEMENT_SYSCALL(syscall_nr, resultp, 0);
}

static int _mylibc_syscall_1(__mylibc_intptr_t syscall_nr,
                             __mylibc_intptr_t *resultp, __mylibc_intptr_t arg1)
{
    _MYLIBC_IMPLEMENT_SYSCALL(syscall_nr, resultp, 1, arg1);
}

static int _mylibc_syscall_2(__mylibc_intptr_t syscall_nr,
                             __mylibc_intptr_t *resultp, __mylibc_intptr_t arg1,
                             __mylibc_intptr_t arg2)
{
    _MYLIBC_IMPLEMENT_SYSCALL(syscall_nr, resultp, 2, arg1, arg2);
}

static int _mylibc_syscall_3(__mylibc_intptr_t syscall_nr,
                             __mylibc_intptr_t *resultp, __mylibc_intptr_t arg1,
                             __mylibc_intptr_t arg2, __mylibc_intptr_t arg3)
{
    _MYLIBC_IMPLEMENT_SYSCALL(syscall_nr, resultp, 3, arg1, arg2, arg3);
}

static int _mylibc_syscall_4(__mylibc_intptr_t syscall_nr,
                             __mylibc_intptr_t *resultp, __mylibc_intptr_t arg1,
                             __mylibc_intptr_t arg2, __mylibc_intptr_t arg3,
                             __mylibc_intptr_t arg4)
{
    _MYLIBC_IMPLEMENT_SYSCALL(syscall_nr, resultp, 4, arg1, arg2, arg3, arg4);
}

static int _mylibc_syscall_5(__mylibc_intptr_t syscall_nr,
                             __mylibc_intptr_t *resultp, __mylibc_intptr_t arg1,
                             __mylibc_intptr_t arg2, __mylibc_intptr_t arg3,
                             __mylibc_intptr_t arg4, __mylibc_intptr_t arg5)
{
    _MYLIBC_IMPLEMENT_SYSCALL(syscall_nr, resultp, 5, arg1, arg2, arg3, arg4,
                              arg5);
}

static int _mylibc_syscall_6(__mylibc_intptr_t syscall_nr,
                             __mylibc_intptr_t *resultp, __mylibc_intptr_t arg1,
                             __mylibc_intptr_t arg2, __mylibc_intptr_t arg3,
                             __mylibc_intptr_t arg4, __mylibc_intptr_t arg5,
                             __mylibc_intptr_t arg6)
{
    _MYLIBC_IMPLEMENT_SYSCALL(syscall_nr, resultp, 6, arg1, arg2, arg3, arg4,
                              arg5, arg6);
}

#endif
