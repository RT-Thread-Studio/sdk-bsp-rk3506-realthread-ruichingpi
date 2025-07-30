/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __KERRNO_H__
#define __KERRNO_H__

#include <rttypes.h>
#include <kpi.h>

#if defined(RT_USING_LIBC) && !defined(RT_USING_NANO)
/* POSIX error code compatible */
#define RT_EOK                          0               /**< There is no error */
#define RT_ERROR                        255             /**< A generic/unknown error happens */
#define RT_ETIMEOUT                     ETIMEDOUT       /**< Timed out */
#define RT_EFULL                        ENOSPC          /**< The resource is full */
#define RT_EEMPTY                       ENODATA         /**< The resource is empty */
#define RT_ENOMEM                       ENOMEM          /**< No memory */
#define RT_ENOSYS                       ENOSYS          /**< Function not implemented */
#define RT_EBUSY                        EBUSY           /**< Busy */
#define RT_EIO                          EIO             /**< IO error */
#define RT_EINTR                        EINTR           /**< Interrupted system call */
#define RT_EINVAL                       EINVAL          /**< Invalid argument */
#define RT_ENOENT                       ENOENT          /**< No entry */
#define RT_ENOSPC                       ENOSPC          /**< No space left */
#define RT_EPERM                        EPERM           /**< Operation not permitted */
#define RT_EFAULT                       EFAULT          /**< Bad address */
#define RT_ENOBUFS                      ENOBUFS         /**< No buffer space is available */
#define RT_ESCHEDISR                    253             /**< scheduler failure in isr context */
#define RT_ESCHEDLOCKED                 252             /**< scheduler failure in critical region */
#define RT_ETRAP                        254             /**< Trap event */
#else
#define RT_EOK                          0               /**< There is no error */
#define RT_ERROR                        1               /**< A generic/unknown error happens */
#define RT_ETIMEOUT                     2               /**< Timed out */
#define RT_EFULL                        3               /**< The resource is full */
#define RT_EEMPTY                       4               /**< The resource is empty */
#define RT_ENOMEM                       5               /**< No memory */
#define RT_ENOSYS                       6               /**< Function not implemented */
#define RT_EBUSY                        7               /**< Busy */
#define RT_EIO                          8               /**< IO error */
#define RT_EINTR                        9               /**< Interrupted system call */
#define RT_EINVAL                       10              /**< Invalid argument */
#define RT_ENOENT                       11              /**< No entry */
#define RT_ENOSPC                       12              /**< No space left */
#define RT_EPERM                        13              /**< Operation not permitted */
#define RT_ETRAP                        14              /**< Trap event */
#define RT_EFAULT                       15              /**< Bad address */
#define RT_ENOBUFS                      16              /**< No buffer space is available */
#define RT_ESCHEDISR                    17              /**< scheduler failure in isr context */
#define RT_ESCHEDLOCKED                 18              /**< scheduler failure in critical region */
#endif /* defined(RT_USING_LIBC) && !defined(RT_USING_NANO) */

typedef rt_err_t (*__kpi_rt_get_errno)(void);
typedef void (*__kpi_rt_set_errno)(rt_err_t no);
typedef int *(*__kpi__rt_errno)(void);
typedef const char *(*__kpi_rt_strerror)(rt_err_t error);

#if !defined(RT_USING_NEWLIBC) && !defined(_WIN32)
#ifndef errno
#define errno       *_rt_errno()
#endif /* errno */
#endif /* !defined(RT_USING_NEWLIBC) && !defined(_WIN32) */

KPI_EXTERN(rt_get_errno);
KPI_EXTERN(rt_set_errno);
KPI_EXTERN(_rt_errno);
KPI_EXTERN(rt_strerror);

#endif /* __KERRNO_H__ */
