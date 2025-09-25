/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __NEWLIB_COMPILER_H__
#define __NEWLIB_COMPILER_H__

#include <kpi.h>
#include <sys/unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>

typedef int (*__kpi_rt_creat)(const char *path, mode_t mode);
typedef int (*__kpi_rt_open)(const char *file, int flags, ...);
typedef ssize_t (*__kpi_rt_read)(int fd, void *buf, size_t len);
typedef ssize_t (*__kpi_rt_write)(int fd, const void *buf, size_t len);
typedef int (*__kpi_rt_close)(int d);
typedef off_t (*__kpi_rt_lseek)(int fd, off_t offset, int whence);
typedef int (*__kpi_rt_fstat)(int, struct stat *);
typedef int (*__kpi_rt_unlink)(const char *pathname);
typedef int (*__kpi_rt_rename)(const char *old_file, const char *new_file);
typedef int (*__kpi_rt_mkdir)(const char *, mode_t);
typedef int (*__kpi_rt_stat)(const char *, struct stat *);
typedef int (*__kpi_rt_fcntl)(int fildes, int cmd, ...);

typedef int (*__kpi_rt_statfs)(const char *path, struct statfs *buf);
typedef int (*__kpi_rt_fstatfs)(int fd, struct statfs *buf);

KPI_EXTERN(rt_creat);
KPI_EXTERN(rt_open);
KPI_EXTERN(rt_read);
KPI_EXTERN(rt_write);
KPI_EXTERN(rt_close);
KPI_EXTERN(rt_lseek);
KPI_EXTERN(rt_fstat);
KPI_EXTERN(rt_unlink);
KPI_EXTERN(rt_rename);
KPI_EXTERN(rt_mkdir);
KPI_EXTERN(rt_stat);
KPI_EXTERN(rt_fcntl);

KPI_EXTERN(rt_statfs);
KPI_EXTERN(rt_fstatfs);

#endif /* __NEWLIB_COMPILER_H__ */
