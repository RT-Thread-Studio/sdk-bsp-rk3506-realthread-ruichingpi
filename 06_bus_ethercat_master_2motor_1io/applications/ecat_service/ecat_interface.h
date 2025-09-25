/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __ETHERCAT_INTERFACE_H
#define __ETHERCAT_INTERFACE_H

#include <rtthread.h>
#include <ecat_master.h>

uint32_t ecat_master_discover_json(ec_master_t *master,
    const char *ip,
    uint16_t port,
    const char *mac,
    char *buff,
    uint32_t len);

uint32_t ecat_master_info_json(ec_master_t *master,
    const char *ip,
    uint16_t port,
    char *buff,
    uint32_t len);

uint32_t ecat_master_state_json(ec_master_t *master, char *buff, uint32_t len);

uint32_t ecat_scan_slave_json(ec_master_t *master, char *buff, uint32_t len);

uint32_t ecat_slave_info_json(ec_master_t *master, char *buff, uint32_t len);

uint32_t ecat_slave_state_json(
    ec_master_t *master, uint16_t index, char *buff, uint32_t len);

uint32_t ecat_dc_config_json(ec_master_t *master, char *buff, uint32_t len);

uint32_t ecat_config_result_json(
    const char *cmd, int16_t error_code, char *buff, uint32_t len);

#endif /* __ETHERCAT_INTERFACE_H */
