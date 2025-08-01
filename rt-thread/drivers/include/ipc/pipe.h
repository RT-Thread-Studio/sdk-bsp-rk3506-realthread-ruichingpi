/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __PIPE_H__
#define __PIPE_H__

#include <kpi.h>
#include <rtconfig.h>
#include <rtdef.h>
#include "condvar.h"

/**
 * Pipe Device
 */

struct rt_pipe_device
{
    struct rt_device parent;
    rt_bool_t is_named;
#ifdef RT_USING_POSIX_DEVIO
    int pipeno; /* for unamed pipe */
#endif

    /* ring buffer in pipe device */
    struct rt_ringbuffer *fifo;
    rt_uint16_t bufsz;

    rt_wqueue_t reader_queue;
    rt_wqueue_t writer_queue;
    int writer;
    int reader;

    struct rt_condvar waitfor_parter;
    struct rt_mutex lock;
};
typedef struct rt_pipe_device rt_pipe_t;

typedef rt_pipe_t *(*__kpi_rt_pipe_create)(const char *name, int bufsz);
typedef rt_err_t (*__kpi_rt_pipe_open)(rt_device_t device, rt_uint16_t oflag);
typedef rt_ssize_t (*__kpi_rt_pipe_read)(
    rt_device_t device, rt_off_t pos, void *buffer, rt_size_t count);
typedef rt_ssize_t (*__kpi_rt_pipe_write)(
    rt_device_t device, rt_off_t pos, const void *buffer, rt_size_t count);
typedef rt_err_t (*__kpi_rt_pipe_control)(rt_device_t dev, int cmd, void *args);
typedef rt_err_t (*__kpi_rt_pipe_close)(rt_device_t device);
typedef int (*__kpi_rt_pipe_delete)(const char *name);

KPI_EXTERN(rt_pipe_create);
KPI_EXTERN(rt_pipe_open);
KPI_EXTERN(rt_pipe_read);
KPI_EXTERN(rt_pipe_write);
KPI_EXTERN(rt_pipe_control);
KPI_EXTERN(rt_pipe_close);
KPI_EXTERN(rt_pipe_delete);

#endif /* __PIPE_H__ */
