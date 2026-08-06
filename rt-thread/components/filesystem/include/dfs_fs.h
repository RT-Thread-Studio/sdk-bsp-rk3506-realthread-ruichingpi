/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __DFS_FS_H__
#define __DFS_FS_H__

#include <dfs.h>
#include <sys/errno.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Pre-declaration */
struct dfs_filesystem;
struct dfs_file;

/* File system operations */
struct dfs_filesystem_ops
{
    char *name;
    uint32_t flags;      /* flags for file system operations */

    /* operations for file */
    const struct dfs_file_ops *fops;

    /* mount and unmount file system */
    int (*mount)    (struct dfs_filesystem *fs, unsigned long rwflag, const void *data);
    int (*unmount)  (struct dfs_filesystem *fs);

    /* make a file system */
    int (*mkfs)     (rt_device_t dev_id, const char *fs_name);
    int (*statfs)   (struct dfs_filesystem *fs, struct statfs *buf);

    int (*unlink)   (struct dfs_filesystem *fs, const char *pathname);
    int (*stat)     (struct dfs_filesystem *fs, const char *filename, struct stat *buf);
    int (*rename)   (struct dfs_filesystem *fs, const char *oldpath, const char *newpath);
};

/* Mounted file system */
struct dfs_filesystem
{
    rt_device_t dev_id;     /* Attached device */

    char *path;             /* File system mount point */
    const struct dfs_filesystem_ops *ops; /* Operations for file system type */

    void *data;             /* Specific file system data */
};

/* file system partition table */
struct dfs_partition
{
    uint8_t type;        /* file system type */
    off_t  offset;       /* partition start offset */
    size_t size;         /* partition size */
    rt_sem_t lock;
};

/* mount table */
struct dfs_mount_tbl
{
    const char   *device_name;
    const char   *path;
    const char   *filesystemtype;
    unsigned long rwflag;
    const void   *data;
};

typedef int (*__kpi_dfs_register)(const struct dfs_filesystem_ops *ops);
typedef struct dfs_filesystem *(*__kpi_dfs_filesystem_lookup)(const char *path);

typedef int (*__kpi_dfs_mount)(const char *device_name,
                               const char *path,
                               const char *filesystemtype,
                               unsigned long rwflag,
                               const void *data);
typedef int (*__kpi_dfs_unmount)(const char *specialfile);

typedef int (*__kpi_dfs_mkfs)(const char *fs_name, const char *device_name);

KPI_EXTERN(dfs_register);
KPI_EXTERN(dfs_filesystem_lookup);
KPI_EXTERN(dfs_mount);
KPI_EXTERN(dfs_unmount);
KPI_EXTERN(dfs_mkfs);

#ifdef __cplusplus
}
#endif

#endif /* __DFS_FS_H__ */
