/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <rtthread.h>

#ifndef __ECAT_SERVICE_H__
#define __ECAT_SERVICE_H__

#define ECAT_SERVICE_MASTER_INIT       (0x01)
#define ECAT_SERVICE_MASTER_DEINIT     (0x02)
#define ECAT_SERVICE_MASTER_START      (0x03)
#define ECAT_SERVICE_MASTER_STOP       (0x04)
#define ECAT_SERVICE_MASTER_STATE      (0x05)
#define ECAT_SERVICE_SLAVE_STATE       (0x06)
#define ECAT_SERVICE_SLAVE_INFO        (0x07)
#define ECAT_SERVICE_SDO_WRITE         (0x08)
#define ECAT_SERVICE_SDO_READ          (0x09)
#define ECAT_SERVICE_DC_CONFIG         (0x0A)
#define ECAT_SERVICE_DC_CONFIG_EX      (0x0B)
#define ECAT_SERVICE_CONFIG_HANDLER    (0x0C)
#define ECAT_SERVICE_PDO_BEGIN_HANDLER (0x0D)
#define ECAT_SERVICE_PDO_END_HANDLER   (0x0E)
#define ECAT_SERVICE_ERROR_HANDLER     (0x0F)
#define ECAT_SERVICE_SLAVE_COUNT       (0x10)

struct ecat_master_state_arg
{
    ec_master_t *master;
    ec_master_state_t *state;
};

struct ecat_slave_state_arg
{
    ec_master_t *master;
    ec_slave_state_t *state;
    uint16_t slave;
};

struct ecat_slave_info_arg
{
    ec_master_t *master;
    ec_slave_info_t *info;
    uint16_t slave;
};

struct ecat_pdo_arg
{
    ec_master_t *master;
    void *data;
    int size;
    int timeout;
    uint16_t slave;
    uint16_t index;
    uint8_t subindex;
    uint8_t complete_access;
};

struct ecat_dc_config_arg
{
    ec_master_t *master;
    uint32_t cycle_time;
    int32_t cycle_shift;
    uint16_t slave;
    uint8_t act;
};

struct ecat_dc_config_ex_arg
{
    ec_master_t *master;
    uint32_t cycle_time0;
    uint32_t cycle_time1;
    int32_t cycle_shift;
    uint16_t slave;
    uint8_t act;
};

struct ecat_config_handler_arg
{
    ec_master_t *master;
    void (*handler)(struct ec_master *master);
};

struct ecat_pdo_handler_arg
{
    ec_master_t *master;
    void (*handler)(struct ec_master *master,
        uint16_t slave,
        uint8_t *input,
        uint8_t *output);
};

struct ecat_error_handler_arg
{
    ec_master_t *master;
    void (*handler)(struct ec_master *master,
        uint32_t error_code,
        const unsigned char *error_str);
};

struct ecat_slavecount_arg
{
    ec_master_t *master;
    int count;
};

#endif /* __ECAT_SERVICE_H__ */