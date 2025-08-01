/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __service_H__
#define __service_H__

#include <rtthread.h>

/* service flag */
#define SERVICE_FLAG_RDONLY     0x02
#define SERVICE_FLAG_WRONLY     0x04
#define SERVICE_FLAG_RDWR       0x08
#define SERVICE_FLAG_STANDALONE 0x10
#define SERVICE_FLAG_MASK       0xFF

/* service open flag */
#define SERVICE_OFLAG_CLOSE     0x01
#define SERVICE_OFLAG_RDONLY    0x02
#define SERVICE_OFLAG_WRONLY    0x04
#define SERVICE_OFLAG_RDWR      0x08
#define SERVICE_OFLAG_OPEN      0x10
#define SERVICE_OFLAG_MASK      0xFF

/* service cmd */
#define SERVICE_CMD_GET_STATUS  0x01
#define SERVICE_CMD_SET_MODE    0x02
#define SERVICE_CMD_RESET       0x03

typedef enum
{
    NETWORK_service = 0x01,
} service_type_t;

struct service_core;

struct service_core_ops
{
    rt_err_t (*open)(struct service_core *service, rt_uint16_t oflag);
    rt_err_t (*close)(struct service_core *service);
    rt_ssize_t (*read)(struct service_core *service,
        rt_off_t pos,
        void *buffer,
        rt_size_t size);
    rt_ssize_t (*write)(struct service_core *service,
        rt_off_t pos,
        const void *buffer,
        rt_size_t size);
    rt_err_t (*control)(struct service_core *service, int cmd, void *args);
};

struct service_core
{
    struct rt_object parent;
    struct rt_mutex mutex;

    rt_uint16_t flag;
    rt_uint16_t type;
    rt_uint16_t open_flag;
    rt_uint8_t ref_count;
    rt_uint8_t service_id;

    rt_err_t (*rx_indicate)(struct service_core *service, rt_size_t size);
    rt_err_t (*tx_complete)(struct service_core *service, void *buffer);
    const struct service_core_ops *ops;

    void *user_data;
};

typedef struct service_core *(*__kpi_service_find)(const char *name);
typedef rt_err_t (*__kpi_service_register)(struct service_core *service,
    const char *name,
    rt_uint16_t flags,
    void *user_data);
typedef rt_err_t (*__kpi_service_unregister)(struct service_core *service);
typedef struct service_core *(*__kpi_service_create)(int type, int attach_size);
typedef void (*__kpi_service_destroy)(struct service_core *service);
typedef rt_err_t (*__kpi_service_open)(
    struct service_core *service, rt_uint16_t oflag);
typedef rt_err_t (*__kpi_service_close)(struct service_core *service);
typedef rt_ssize_t (*__kpi_service_read)(
    struct service_core *service, rt_off_t pos, void *buffer, rt_size_t size);
typedef rt_ssize_t (*__kpi_service_write)(struct service_core *service,
    rt_off_t pos,
    const void *buffer,
    rt_size_t size);
typedef rt_err_t (*__kpi_service_control)(
    struct service_core *service, int cmd, void *arg);
typedef rt_err_t (*__kpi_service_set_rx_indicate)(struct service_core *service,
    rt_err_t (*rx_ind)(struct service_core *service, rt_size_t size));
typedef rt_err_t (*__kpi_service_set_tx_complete)(struct service_core *service,
    rt_err_t (*tx_done)(struct service_core *service, void *buffer));
typedef rt_err_t (*__kpi_service_lock)(struct service_core *service);
typedef rt_err_t (*__kpi_service_unlock)(struct service_core *service);

KPI_EXTERN(service_find);
KPI_EXTERN(service_register);
KPI_EXTERN(service_unregister);
KPI_EXTERN(service_create);
KPI_EXTERN(service_destroy);
KPI_EXTERN(service_open);
KPI_EXTERN(service_close);
KPI_EXTERN(service_read);
KPI_EXTERN(service_write);
KPI_EXTERN(service_control);
KPI_EXTERN(service_set_rx_indicate);
KPI_EXTERN(service_set_tx_complete);
KPI_EXTERN(service_lock);
KPI_EXTERN(service_unlock);

#endif /* __service_H__ */
