/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __SITRONIX_TS_H__
#define __SITRONIX_TS_H__

#include <rtthread.h>
#include <rtdevice.h>

/* IC register */
#define ST_MISC_INFO_SWU_FLAG          0x80
#define ST_MISC_INFO_PROX_FLAG         0x20
#define ST_MISC_INFO_COORD_CHKSUM_FLAG 0x10

enum
{
    FIRMWARE_VERSION = 0,
    STATUS_REG,
    DEVICE_CONTROL_REG,
    TIMEOUT_TO_IDLE_REG,
    X_RESOLUTION_HIGH = 0x05,
    X_RESOLUTION_LOW = 0x06,
    Y_RESOLUTION_HIGH = 0x07,
    Y_RESOLUTION_LOW = 0x08,
    MAX_NUM_TOUCHES = 0x09,
    SENSING_COUNTER_H = 0x0A,
    SENSING_COUNTER_L = 0x0B,
    DEVICE_CONTROL_REG2 = 0x09,
    FIRMWARE_REVISION_3 = 0x0C,
    FIRMWARE_REVISION_2,
    FIRMWARE_REVISION_1,
    FIRMWARE_REVISION_0,
    TOUCH_INFO = 0x10,
    GESTURES = 0x12,
    PROX_RAW_HEADER = 0x5A,
    MISC_INFO = 0xF0,
    MISC_CONTROL = 0xF1,
    SMART_WAKE_UP_REG = 0xF2,
    CHIP_ID = 0xF4,
    XY_CHS = 0xF5,
    CMDIO_CONTROL = 0xF8,
    PAGE_REG = 0xFF,
    CMDIO_PORT = 0x110,
    EX_DIFF_EN = 0x130,
    DATA_OUTPUT_BUFFER = 0x140,
};

struct sitronix_ts_host_interface
{
    int irq_gpio;
    int rst_gpio;
    void *if_data;

    int (*read)(uint16_t addr, uint8_t *data, uint16_t length, void *if_data);
    int (*write)(uint16_t addr, uint8_t *data, uint16_t length, void *if_data);
    int (*dread)(uint8_t *data, uint16_t length, void *if_data);
    int (*dwrite)(uint8_t *data, uint16_t length, void *if_data);
    int (*aread)(uint8_t *tx_buf,
        uint16_t tx_len,
        uint8_t *rx_buf,
        uint16_t rx_len,
        void *if_data);
    int (*awrite)(uint8_t *tx_buf, uint16_t tx_len, void *if_data);
};

struct sitronix_ts_device_info
{
    uint8_t chip_id;
    uint8_t fw_version;
    uint8_t fw_revision[4];
    uint8_t customer_info[4];
    uint8_t max_touches;
    uint16_t x_res;
    uint16_t y_res;
    uint8_t x_chs;
    uint8_t y_chs;
    uint8_t k_chs;
    uint8_t n_chs;
    uint8_t misc_info;
    uint8_t chip_ver;
    uint8_t max_swk_touch_num;
};

struct sitronix_ts_data
{
    struct rt_touch_device device;
    struct sitronix_ts_device_info ts_dev_info;
    const struct sitronix_ts_host_interface *host_if;

    /* coord buf */
    uint8_t coord_buf[400];

    /* swu */
    rt_bool_t swu_flag;
    rt_bool_t swu_status;

    /* proximity */
    rt_bool_t is_support_proximity;

    /* coord checksum */
    rt_bool_t is_support_coord_chksum;
    rt_bool_t proximity_enabled;

    /* ProxAlgRaw Buf */
    uint8_t prox_raw_buf[130];
};

static inline int sitronix_ts_reg_read(struct sitronix_ts_data *ts_data,
    uint16_t addr,
    uint8_t *data,
    uint16_t len)
{
    return ts_data->host_if->read(addr, data, len, ts_data->host_if->if_data);
}

static inline int sitronix_ts_reg_write(struct sitronix_ts_data *ts_data,
    uint16_t addr,
    uint8_t *data,
    uint16_t len)
{
    return ts_data->host_if->write(addr, data, len, ts_data->host_if->if_data);
}

#endif /* __SITRONIX_TS_H__ */
