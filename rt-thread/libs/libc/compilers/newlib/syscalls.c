/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <newlib_compiler.h>
#include <reent.h>
#include <rtthread.h>
#ifdef RT_USING_RTC
#include <rtdevice.h>
#endif /* RT_USING_RTC */
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/stat.h>
#ifdef RT_USING_POSIX_STDIO
#include <posix/stdio.h>
#endif /* RT_USING_POSIX_STDIO */
#include <posix/stdlib.h>
#ifdef RT_USING_MODULE
#include <dlmodule.h>
#endif /* RT_USING_MODULE */
#include <compiler_private.h>
#define DBG_TAG    "newlib.syscalls"
#define DBG_LVL    DBG_INFO
#include <rtdbg.h>

#ifdef RT_USING_HEAP /* Memory routine */
void *_malloc_r(struct _reent *ptr, size_t size)
{
    void* result;

    result = (void*)rt_malloc(size);
    if (result == RT_NULL)
    {
        ptr->_errno = ENOMEM;
    }

    return result;
}

void *_realloc_r(struct _reent *ptr, void *old, size_t newlen)
{
    void* result;

    result = (void*)rt_realloc(old, newlen);
    if (result == RT_NULL)
    {
        ptr->_errno = ENOMEM;
    }

    return result;
}

void *_calloc_r(struct _reent *ptr, size_t size, size_t len)
{
    void* result;

    result = (void*)rt_calloc(size, len);
    if (result == RT_NULL)
    {
        ptr->_errno = ENOMEM;
    }

    return result;
}

void _free_r(struct _reent *ptr, void *addr)
{
    rt_free(addr);
}

#else
void *_sbrk_r(struct _reent *ptr, ptrdiff_t incr)
{
    RT_ASSERT(0);
    return RT_NULL;
}
#endif /*RT_USING_HEAP*/

void __libc_init_array(void)
{
    /* we not use __libc init_aray to initialize C++ objects */
    /* __libc_init_array is ARM code, not Thumb; it will cause a hardfault. */
}

/* Reentrant versions of system calls.  */
#ifndef _REENT_ONLY
int *__errno(void)
{
    return _rt_errno();
}
#endif

int _getpid_r(struct _reent *ptr)
{
    return 0;
}

int _close_r(struct _reent *ptr, int fd)
{
#ifdef DFS_USING_POSIX
    return rt_close(fd);
#else
    ptr->_errno = ENOTSUP;
    return -1;
#endif /* DFS_USING_POSIX */
}

int _execve_r(struct _reent *ptr, const char * name, char *const *argv, char *const *env)
{
    ptr->_errno = ENOTSUP;
    return -1;
}

int _fcntl_r(struct _reent *ptr, int fd, int cmd, int arg)
{
    ptr->_errno = ENOTSUP;
    return -1;
}

int _fork_r(struct _reent *ptr)
{
    ptr->_errno = ENOTSUP;
    return -1;
}

int _fstat_r(struct _reent *ptr, int fd, struct stat *pstat)
{
    ptr->_errno = ENOTSUP;
    return -1;
}

int _isatty_r(struct _reent *ptr, int fd)
{
    if (fd >=0 && fd < 3)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int _kill_r(struct _reent *ptr, int pid, int sig)
{
    ptr->_errno = ENOTSUP;
    return -1;
}

int _link_r(struct _reent *ptr, const char *old, const char *new)
{
    ptr->_errno = ENOTSUP;
    return -1;
}

int _wait_r(struct _reent *ptr, int *status)
{
    ptr->_errno = ENOTSUP;
    return -1;
}

mode_t umask(mode_t mask)
{
    return 022;
}

int flock(int fd, int operation)
{
    return 0;
}

_off_t _lseek_r(struct _reent *ptr, int fd, _off_t pos, int whence)
{
#ifdef DFS_USING_POSIX
    _off_t rc;
    rc = rt_lseek(fd, pos, whence);
    return rc;
#else
    ptr->_errno = ENOTSUP;
    return -1;
#endif /* DFS_USING_POSIX */
}

int _mkdir_r(struct _reent *ptr, const char *name, int mode)
{
#ifdef DFS_USING_POSIX
    int rc;
    rc = rt_mkdir(name, mode);
    return rc;
#else
    ptr->_errno = ENOTSUP;
    return -1;
#endif /* DFS_USING_POSIX */
}

int _open_r(struct _reent *ptr, const char *file, int flags, int mode)
{
#ifdef DFS_USING_POSIX
    int rc;
    rc = rt_open(file, flags, mode);
    return rc;
#else
    ptr->_errno = ENOTSUP;
    return -1;
#endif /* DFS_USING_POSIX */
}

_ssize_t _read_r(struct _reent *ptr, int fd, void *buf, size_t nbytes)
{
#ifdef DFS_USING_POSIX
    _ssize_t rc;
    if (fd == STDIN_FILENO)
    {
#ifdef RT_USING_POSIX_STDIO
        // if (rt_posix_stdio_get_console() < 0)
        // {
        //     // LOG_W("Do not invoke standard input before initializing Compiler");
        //     return 0;
        // }
#else
        ptr->_errno = ENOTSUP;
        return -1;
#endif /* RT_USING_POSIX_STDIO */
    }
    else if (fd == STDOUT_FILENO || fd == STDERR_FILENO)
    {
        ptr->_errno = ENOTSUP;
        return -1;
    }

    rc = rt_read(fd, buf, nbytes);
    return rc;
#else
    ptr->_errno = ENOTSUP;
    return -1;
#endif /* DFS_USING_POSIX */
}

int _rename_r(struct _reent *ptr, const char *old, const char *new)
{
#ifdef DFS_USING_POSIX
    int rc;
    rc = rt_rename(old, new);
    return rc;
#else
    ptr->_errno = ENOTSUP;
    return -1;
#endif /* DFS_USING_POSIX */
}

int _stat_r(struct _reent *ptr, const char *file, struct stat *pstat)
{
#ifdef DFS_USING_POSIX
    int rc;
    rc = rt_stat(file, pstat);
    return rc;
#else
    ptr->_errno = ENOTSUP;
    return -1;
#endif /* DFS_USING_POSIX */
}

int _unlink_r(struct _reent *ptr, const char *file)
{
#ifdef DFS_USING_POSIX
    return rt_unlink(file);
#else
    ptr->_errno = ENOTSUP;
    return -1;
#endif /* DFS_USING_POSIX */
}

_ssize_t _write_r(struct _reent *ptr, int fd, const void *buf, size_t nbytes)
{
#ifdef DFS_USING_POSIX
    _ssize_t rc;
#endif /* DFS_USING_POSIX */

    if (fd == STDOUT_FILENO || fd == STDERR_FILENO)
    {
#if defined(RT_USING_CONSOLE) && defined(RT_USING_DEVICE)
        rt_device_t console;

        console = rt_console_get_device();
        if (console)
            return rt_device_write(console, -1, buf, nbytes);

        return nbytes;
#else
        ptr->_errno = ENOTSUP;
        return -1;
#endif /* defined(RT_USING_CONSOLE) && defined(RT_USING_DEVICE) */
    }
    else if (fd == STDIN_FILENO)
    {
        ptr->_errno = ENOTSUP;
        return -1;
    }

#ifdef DFS_USING_POSIX
    rc = rt_write(fd, buf, nbytes);
    return rc;
#else
    ptr->_errno = ENOTSUP;
    return -1;
#endif /* DFS_USING_POSIX */
}

/* for exit() and abort() */
__attribute__ ((noreturn)) void _exit (int status)
{
    __rt_libc_exit(status);
    while(1);
}

/*
These functions are implemented and replaced by the 'common/time.c' file
int _gettimeofday_r(struct _reent *ptr, struct timeval *__tp, void *__tzp);
_CLOCK_T_  _times_r(struct _reent *ptr, struct tms *ptms);
*/
int _gettimeofday(struct timeval *tv, void *tzvp)
{
    rt_tick_t tick;
    rt_device_t device;
    
    if (tv)
    {
        device = rt_device_find("rtc");
        if (device)
        {
            rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, tv);
            return 0;
        }

        tick = rt_tick_get();
        tv->tv_sec = tick / RT_TICK_PER_SECOND;
        tv->tv_usec = (tick % RT_TICK_PER_SECOND) * (1000000 / RT_TICK_PER_SECOND);

        return 0;
    }
    
    return -1;
}
