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
#include "ecat_master.h"
#include "ecat_service.h"
#include <service.h>
#include <stdint.h>
#include <stdlib.h>

static struct service_core *service = RT_NULL;

rt_err_t ecat_master_init(ec_master_t *master)
{
    if (!master)
    {
        return (-RT_EINVAL);
    }

    if (service != RT_NULL)
    {
        return service_control(service, ECAT_SERVICE_MASTER_INIT, master);
    }

    return (-RT_ERROR);
}

rt_err_t ecat_master_deinit(ec_master_t *master)
{
    if (!master)
    {
        return (-RT_EINVAL);
    }

    if (service != RT_NULL)
    {
        return service_control(service, ECAT_SERVICE_MASTER_DEINIT, master);
    }

    return (-RT_ERROR);
}

rt_err_t ecat_simple_start(ec_master_t *master)
{
    if (!master)
    {
        return (-RT_EINVAL);
    }

    if (service != RT_NULL)
    {
        return service_control(service, ECAT_SERVICE_MASTER_START, master);
    }

    return (-RT_ERROR);
}

rt_err_t ecat_simple_stop(ec_master_t *master)
{
    if (!master)
    {
        return (-RT_EINVAL);
    }

    if (service != RT_NULL)
    {
        return service_control(service, ECAT_SERVICE_MASTER_STOP, master);
    }

    return (-RT_ERROR);
}

rt_err_t ecat_master_state(ec_master_t *master, ec_master_state_t *state)
{
    if ((!master) || (!state))
    {
        return (-RT_EINVAL);
    }

    if (service != RT_NULL)
    {
        struct ecat_master_state_arg state_arg;
        state_arg.master = master;
        state_arg.state = state;
        return service_control(service, ECAT_SERVICE_MASTER_STATE, &state_arg);
    }

    return (-RT_ERROR);
}

rt_err_t ecat_slave_state(
    ec_master_t *master, uint16_t slave, ec_slave_state_t *state)
{
    if ((!master) || (!state))
    {
        return (-RT_EINVAL);
    }

    if (service != RT_NULL)
    {
        struct ecat_slave_state_arg state_arg;
        state_arg.master = master;
        state_arg.state = state;
        state_arg.slave = slave;
        return service_control(service, ECAT_SERVICE_SLAVE_STATE, &state_arg);
    }

    return (-RT_ERROR);
}

rt_err_t ecat_slave_info(
    ec_master_t *master, uint16_t slave, ec_slave_info_t *info)
{
    if ((!master) || (!info))
    {
        return (-RT_EINVAL);
    }

    if (service != RT_NULL)
    {
        struct ecat_slave_info_arg info_arg;
        info_arg.master = master;
        info_arg.info = info;
        info_arg.slave = slave;
        return service_control(service, ECAT_SERVICE_SLAVE_INFO, &info_arg);
    }

    return (-RT_ERROR);
}

int ecat_sdo_write(ec_master_t *master,
    uint16_t slave,
    uint16_t index,
    uint8_t subindex,
    uint8_t complete_access,
    void *data,
    int size,
    int timeout)
{
    if (!master)
    {
        return (-RT_EINVAL);
    }

    if (service != RT_NULL)
    {
        struct ecat_pdo_arg pdo_arg;
        pdo_arg.master = master;
        pdo_arg.slave = slave;
        pdo_arg.index = index;
        pdo_arg.subindex = subindex;
        pdo_arg.complete_access = complete_access;
        pdo_arg.data = data;
        pdo_arg.size = size;
        pdo_arg.timeout = timeout;
        return service_control(service, ECAT_SERVICE_SDO_WRITE, &pdo_arg);
    }

    return 0;
}

int ecat_sdo_read(ec_master_t *master,
    uint16_t slave,
    uint16_t index,
    uint8_t subindex,
    uint8_t complete_access,
    void *data,
    int size,
    int timeout)
{
    if (!master)
    {
        return (-RT_EINVAL);
    }

    if (service != RT_NULL)
    {
        struct ecat_pdo_arg pdo_arg;
        pdo_arg.master = master;
        pdo_arg.slave = slave;
        pdo_arg.index = index;
        pdo_arg.subindex = subindex;
        pdo_arg.complete_access = complete_access;
        pdo_arg.data = data;
        pdo_arg.size = size;
        pdo_arg.timeout = timeout;
        return service_control(service, ECAT_SERVICE_SDO_READ, &pdo_arg);
    }

    return 0;
}

void ecat_dc_config(ec_master_t *master,
    uint16_t slave,
    uint8_t act,
    uint32_t cycle_time,
    int32_t cycle_shift)
{
    if (!master)
    {
        return;
    }

    if (service != RT_NULL)
    {
        struct ecat_dc_config_arg dc_cfg;
        dc_cfg.master = master;
        dc_cfg.slave = slave;
        dc_cfg.act = act;
        dc_cfg.cycle_time = cycle_time;
        dc_cfg.cycle_shift = cycle_shift;
        service_control(service, ECAT_SERVICE_DC_CONFIG, &dc_cfg);
    }
}

void ecat_dc_config_ex(ec_master_t *master,
    uint16_t slave,
    uint8_t act,
    uint32_t cycle_time0,
    uint32_t cycle_time1,
    int32_t cycle_shift)
{
    if (!master)
    {
        return;
    }

    if (service != RT_NULL)
    {
        struct ecat_dc_config_ex_arg dc_cfg;
        dc_cfg.master = master;
        dc_cfg.slave = slave;
        dc_cfg.act = act;
        dc_cfg.cycle_time0 = cycle_time0;
        dc_cfg.cycle_time1 = cycle_time1;
        dc_cfg.cycle_shift = cycle_shift;
        service_control(service, ECAT_SERVICE_DC_CONFIG_EX, &dc_cfg);
    }
}

void ecat_set_config_handler(
    ec_master_t *master, void (*ecat_config_handler)(struct ec_master *master))
{
    if (!master)
    {
        return;
    }

    master->ecat_config_handler = ecat_config_handler;
}

void ecat_set_process_data_begin_handler(ec_master_t *master,
    void (*ecat_process_data_begin_handler)(struct ec_master *master,
        uint16_t slave,
        uint8_t *input,
        uint8_t *output))
{
    if (!master)
    {
        return;
    }

    master->ecat_process_data_begin_handler = ecat_process_data_begin_handler;
}

void ecat_set_process_data_end_handler(ec_master_t *master,
    void (*ecat_process_data_end_handler)(struct ec_master *master,
        uint16_t slave,
        uint8_t *input,
        uint8_t *output))
{
    if (!master)
    {
        return;
    }

    master->ecat_process_data_end_handler = ecat_process_data_end_handler;
}

void ecat_set_error_handler(ec_master_t *master,
    void (*error_handler)(struct ec_master *master,
        uint32_t error_code,
        const unsigned char *error_str))
{
    if (!master)
    {
        return;
    }

    master->error_handler = error_handler;
}

int ecat_slavecount(ec_master_t *master)
{
    if (!master)
    {
        return -1;
    }

    if (service != RT_NULL)
    {
        struct ecat_slavecount_arg slavecount_arg;
        slavecount_arg.master = master;
        slavecount_arg.count = 0;
        service_control(service, ECAT_SERVICE_SLAVE_COUNT, &slavecount_arg);
        return slavecount_arg.count;
    }

    return 0;
}

int ecat_service_init(void)
{
    service = service_find("ecat_service");
    if (!service)
    {
        rt_kprintf("not find ethercat service\n");
        return (-RT_ERROR);
    }

    return RT_EOK;
}
