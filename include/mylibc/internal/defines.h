#ifndef _MYLIBC_DEFINES_H
#define _MYLIBC_DEFINES_H 1

#if defined _WIN32 || defined __CYGWIN__
#define _MYLIBC_IMPORT __declspec(dllimport)
#define _MYLIBC_EXPORT __declspec(dllexport)
#elif __GNUC__ >= 4
#define _MYLIBC_IMPORT __attribute__((visibility("default")))
#define _MYLIBC_EXPORT __attribute__((visibility("default")))
#else
#define _MYLIBC_IMPORT
#define _MYLIBC_EXPORT
#endif

#ifdef _MYLIBC_DLL_EXPORTS
#define _MYLIBC_API _MYLIBC_EXPORT
#else
#define _MYLIBC_API _MYLIBC_IMPORT
#endif

#endif /* DEFINES */
