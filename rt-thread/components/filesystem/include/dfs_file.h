/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __DFS_FILE_H__
#define __DFS_FILE_H__

#include <dfs.h>
#include <dfs_fs.h>

struct rt_pollreq;
struct dfs_file;

struct dfs_file_ops
{
    int (*open)(struct dfs_file *fd);
    int (*close)(struct dfs_file *fd);
    int (*ioctl)(struct dfs_file *fd, int cmd, void *args);
    ssize_t (*read)(struct dfs_file *fd, void *buf, size_t count);
    ssize_t (*write)(struct dfs_file *fd, const void *buf, size_t count);
    int (*flush)(struct dfs_file *fd);
    off_t (*lseek)(struct dfs_file *fd, off_t offset);
    int (*getdents)(struct dfs_file *fd, struct dirent *dirp, uint32_t count);

    int (*poll)(struct dfs_file *fd, struct rt_pollreq *req);
};

/* file descriptor */
#define DFS_FD_MAGIC 0xfdfd

struct dfs_vnode
{
    uint16_t type; /* Type (regular or socket) */

    char *path;     /* Name (below mount point) */
    char *fullpath; /* Full path is hash key */
    int ref_count;  /* Descriptor reference count */
    rt_list_t list; /* The node of vnode hash table */

    struct dfs_filesystem *fs;
    const struct dfs_file_ops *fops;
    uint32_t flags; /* self flags, is dir etc.. */

    size_t size; /* Size in bytes */
    void *data;  /* Specific file system data */
};

struct dfs_file
{
    uint16_t magic;          /* file descriptor magic number */
    uint32_t flags;          /* Descriptor flags */
    int ref_count;           /* Descriptor reference count */
    off_t pos;               /* Current file position */
    struct dfs_vnode *vnode; /* file node struct */
    void *data;              /* Specific fd data */
};
#define DFS_FILE_POS(dfs_file) ((dfs_file)->pos)

#endif /* __DFS_FILE_H__ */
