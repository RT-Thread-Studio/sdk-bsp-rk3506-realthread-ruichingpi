/*
 * RT-Thread Secure
 *
 * Copyright (c) Shanghai Real-Thread Electronic Technology Co., Ltd.
 *
 * All rights reserved.
 */

#include <rtthread.h>

#ifndef DFS_SERVICE_H__
#define DFS_SERVICE_H__

#define APP_DFS_FDTABLE_GET  0x01

struct dfs_fdtable *dfs_fdtable_get(void);

#endif /* DFS_SERVICE_H__ */
