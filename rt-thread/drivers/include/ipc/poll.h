/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __POLL_H__
#define __POLL_H__

#include <rtconfig.h>
#include <rtdef.h>

struct rt_pollreq;
typedef void (*poll_queue_proc)(rt_wqueue_t *, struct rt_pollreq *);

typedef struct rt_pollreq
{
    poll_queue_proc _proc;
    short _key;
} rt_pollreq_t;

rt_inline void rt_poll_add(rt_wqueue_t *wq, rt_pollreq_t *req)
{
    if (req && req->_proc && wq)
    {
        req->_proc(wq, req);
    }
}

#endif /* __POLL_H__ */
