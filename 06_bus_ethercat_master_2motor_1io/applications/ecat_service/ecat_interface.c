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
#include <rtdevice.h>
#include "stdlib.h"
#include <stdio.h>
#include <string.h>

#ifdef COMP_USING_ETHERCAT
#include "cJSON.h"
#include "ecat_interface.h"
#include "ecat_master.h"

#define LOG_TAG "[ec intf]"
#define LOG_LVL LOG_LVL_DBG
#include <rtdbg.h>

uint32_t ecat_master_discover_json(ec_master_t *master,
    const char *ip,
    uint16_t port,
    const char *mac,
    char *buff,
    uint32_t len)
{
    uint32_t ret = 0;
    if (master == RT_NULL)
        return ret;

    if ((buff == RT_NULL) || (len == 0))
        return ret;

    cJSON *root_obj = cJSON_CreateObject();
    if (root_obj == RT_NULL)
    {
        return ret;
    }
    cJSON_AddStringToObject(root_obj, "cmd", "discover");

    cJSON *data_obj = cJSON_AddObjectToObject(root_obj, "data");
    if (data_obj == RT_NULL)
    {
        cJSON_Delete(root_obj);
        return ret;
    }

    cJSON_AddStringToObject(data_obj, "name", master->name);
    cJSON_AddStringToObject(data_obj, "type", master->name);
    cJSON_AddStringToObject(data_obj, "ip", ip);
    cJSON_AddNumberToObject(data_obj, "port", port);
    cJSON_AddStringToObject(data_obj, "mac", mac);
    ret = cJSON_PrintPreallocated(root_obj, buff, len, 0);
    if (ret)
    {
        ret = rt_strnlen(buff, len) + 1;
    }
    cJSON_Delete(root_obj);
    return ret;
}

uint32_t ecat_master_info_json(ec_master_t *master,
    const char *ip,
    uint16_t port,
    char *buff,
    uint32_t len)
{
    uint32_t ret = 0;
    if (master == RT_NULL)
        return ret;

    if ((buff == RT_NULL) || (len == 0))
        return ret;

    cJSON *root_obj = cJSON_CreateObject();
    if (root_obj == RT_NULL)
    {
        return ret;
    }
    cJSON_AddStringToObject(root_obj, "cmd", "master_info");

    cJSON *data_obj = cJSON_AddObjectToObject(root_obj, "data");
    if (data_obj == RT_NULL)
    {
        cJSON_Delete(root_obj);
        return ret;
    }

    cJSON_AddStringToObject(data_obj, "name", master->name);
    cJSON_AddStringToObject(data_obj, "type", master->name);
    cJSON_AddStringToObject(data_obj, "ip", ip);
    cJSON_AddNumberToObject(data_obj, "port", port);
    cJSON_AddNumberToObject(data_obj, "state", master->state.al_states);
    cJSON_AddNumberToObject(data_obj, "loop_cycle", master->main_cycletime_us);
    cJSON_AddNumberToObject(data_obj, "mb_cycle", master->sub_cycletime_us);
    cJSON_AddNumberToObject(data_obj, "pdo_timeout", master->pdo_timeout);
    cJSON_AddNumberToObject(data_obj, "sdo_tx_timeout", master->sdo_tx_timeout);
    cJSON_AddNumberToObject(data_obj, "sdo_rx_timeout", master->sdo_rx_timeout);
    cJSON_AddNumberToObject(
        data_obj, "recovery_timeout", master->recovery_timeout_ms);
    cJSON_AddNumberToObject(data_obj, "info_cycle", master->info_cycle);
    cJSON_AddBoolToObject(data_obj, "pdi_check", master->pdi_check);
    cJSON_AddBoolToObject(data_obj, "wdt_enable", master->wdt_enable);
    cJSON_AddNumberToObject(data_obj, "wdt_timeout", master->wdt_timeout);

    ret = cJSON_PrintPreallocated(root_obj, buff, len, 0);
    if (ret)
    {
        ret = rt_strnlen(buff, len) + 1;
    }
    cJSON_Delete(root_obj);
    return ret;
}

uint32_t ecat_master_state_json(ec_master_t *master, char *buff, uint32_t len)
{
    uint32_t ret = 0;

    if (master == RT_NULL)
        return ret;

    if ((buff == RT_NULL) || (len == 0))
        return ret;

    cJSON *root_obj = cJSON_CreateObject();
    if (root_obj == RT_NULL)
    {
        return ret;
    }
    cJSON_AddStringToObject(root_obj, "cmd", "master_state");

    cJSON *data_obj = cJSON_AddObjectToObject(root_obj, "data");
    if (data_obj == RT_NULL)
    {
        cJSON_Delete(root_obj);
        return ret;
    }
    cJSON_AddNumberToObject(data_obj, "state", master->state.al_states);
    cJSON_AddNumberToObject(data_obj, "request_state", master->state.al_states);

    ret = cJSON_PrintPreallocated(root_obj, buff, len, 0);
    if (ret)
    {
        ret = rt_strnlen(buff, len) + 1;
    }
    cJSON_Delete(root_obj);
    return ret;
}

uint32_t ecat_scan_slave_json(ec_master_t *master, char *buff, uint32_t len)
{
    uint32_t ret = 0;
    cJSON *slaveinfo_arr = RT_NULL;

    if (master == RT_NULL)
        return ret;

    if ((buff == RT_NULL) || (len == 0))
        return ret;

    cJSON *root_obj = cJSON_CreateObject();
    if (root_obj == RT_NULL)
    {
        return ret;
    }
    cJSON_AddStringToObject(root_obj, "cmd", "scan_slave");

    slaveinfo_arr = cJSON_AddArrayToObject(root_obj, "data");
    if (slaveinfo_arr == RT_NULL)
    {
        cJSON_Delete(root_obj);
        return ret;
    }
    int slavecount = ecat_slavecount(master);
    for (int slave = 1; slave <= slavecount; slave++)
    {
        cJSON *item = cJSON_CreateObject();
        if (item == RT_NULL)
        {
            break;
        }
        ec_slave_info_t info;
        ecat_slave_info(master, slave, &info);
        cJSON_AddNumberToObject(item, "index", info.index);
        cJSON_AddStringToObject(item, "name", info.name);
        cJSON_AddNumberToObject(item, "parent", info.parent);
        cJSON_AddNumberToObject(item, "state", info.state);
        cJSON_AddNumberToObject(item, "vendor_id", info.vendor_id);
        cJSON_AddNumberToObject(item, "product_id", info.product_code);
        cJSON_AddNumberToObject(item, "revision", info.revision_number);
        cJSON_AddNumberToObject(item, "configadr", info.revision_number);
        cJSON_AddNumberToObject(item, "inputbits", info.Ibits);
        cJSON_AddNumberToObject(item, "outputbits", info.Obits);
        cJSON_AddNumberToObject(item, "mbx_proto", info.mbx_proto);
        cJSON_AddNumberToObject(item, "interface_type", info.Itype);
        cJSON_AddNumberToObject(item, "device_type", info.Dtype);
        cJSON_AddNumberToObject(item, "pdelay", info.pdelay);
        cJSON_AddNumberToObject(item, "dc_cycle", info.DCcycle);
        cJSON_AddNumberToObject(item, "dc_shift", info.DCshift);
        cJSON_AddNumberToObject(item, "dc_active", info.DCactive);
        cJSON_AddNumberToObject(item, "dc_next", info.DCnext);
        cJSON_AddNumberToObject(item, "dc_previous", info.DCprevious);
        cJSON_AddItemToArray(slaveinfo_arr, item);
    }

    ret = cJSON_PrintPreallocated(root_obj, buff, len, 0);
    if (ret)
    {
        ret = rt_strnlen(buff, len) + 1;
    }
    cJSON_Delete(root_obj);
    return ret;
}

uint32_t ecat_slave_info_json(ec_master_t *master, char *buff, uint32_t len)
{
    uint32_t ret = 0;
    cJSON *slaveinfo_arr = RT_NULL;

    if (master == RT_NULL)
        return ret;

    if ((buff == RT_NULL) || (len == 0))
        return ret;

    cJSON *root_obj = cJSON_CreateObject();
    if (root_obj == RT_NULL)
    {
        return ret;
    }
    cJSON_AddStringToObject(root_obj, "cmd", "slave_info");

    slaveinfo_arr = cJSON_AddArrayToObject(root_obj, "data");
    if (slaveinfo_arr == RT_NULL)
    {
        cJSON_Delete(root_obj);
        return ret;
    }
    int slavecount = ecat_slavecount(master);
    for (int slave = 1; slave <= slavecount; slave++)
    {
        cJSON *item = cJSON_CreateObject();
        if (item == RT_NULL)
        {
            break;
        }
        ec_slave_info_t info;
        ecat_slave_info(master, slave, &info);
        cJSON_AddNumberToObject(item, "index", info.index);
        cJSON_AddStringToObject(item, "name", info.name);
        cJSON_AddNumberToObject(item, "parent", info.parent);
        cJSON_AddNumberToObject(item, "state", info.state);
        cJSON_AddNumberToObject(item, "vendor_id", info.vendor_id);
        cJSON_AddNumberToObject(item, "product_id", info.product_code);
        cJSON_AddNumberToObject(item, "revision", info.revision_number);
        cJSON_AddNumberToObject(item, "configadr", info.revision_number);
        cJSON_AddNumberToObject(item, "inputbits", info.Ibits);
        cJSON_AddNumberToObject(item, "outputbits", info.Obits);
        cJSON_AddNumberToObject(item, "mbx_proto", info.mbx_proto);
        cJSON_AddNumberToObject(item, "interface_type", info.Itype);
        cJSON_AddNumberToObject(item, "device_type", info.Dtype);
        cJSON_AddNumberToObject(item, "pdelay", info.pdelay);
        cJSON_AddNumberToObject(item, "dc_cycle", info.DCcycle);
        cJSON_AddNumberToObject(item, "dc_shift", info.DCshift);
        cJSON_AddNumberToObject(item, "dc_active", info.DCactive);
        cJSON_AddNumberToObject(item, "dc_next", info.DCnext);
        cJSON_AddNumberToObject(item, "dc_previous", info.DCprevious);
        cJSON_AddItemToArray(slaveinfo_arr, item);
    }

    ret = cJSON_PrintPreallocated(root_obj, buff, len, 0);
    if (ret)
    {
        ret = rt_strnlen(buff, len) + 1;
    }
    cJSON_Delete(root_obj);
    return ret;
}

uint32_t ecat_slave_state_json(
    ec_master_t *master, uint16_t index, char *buff, uint32_t len)
{
    uint32_t ret = 0;

    if (master == RT_NULL)
        return ret;

    if ((buff == RT_NULL) || (len == 0))
        return ret;

    cJSON *root_obj = cJSON_CreateObject();
    if (root_obj == RT_NULL)
    {
        return ret;
    }
    cJSON_AddStringToObject(root_obj, "cmd", "slave_state");

    cJSON *data_obj = cJSON_AddObjectToObject(root_obj, "data");
    if (data_obj == RT_NULL)
    {
        cJSON_Delete(root_obj);
        return ret;
    }
    ec_slave_info_t info;
    ecat_slave_info(master, index, &info);
    cJSON_AddNumberToObject(data_obj, "index", info.index);
    cJSON_AddNumberToObject(data_obj, "state", info.al_statuscode);
    cJSON_AddNumberToObject(data_obj, "request_state", info.al_statuscode);

    ret = cJSON_PrintPreallocated(root_obj, buff, len, 0);
    if (ret)
    {
        ret = rt_strnlen(buff, len) + 1;
    }
    cJSON_Delete(root_obj);
    return ret;
}

uint32_t ecat_dc_config_json(ec_master_t *master, char *buff, uint32_t len)
{
    uint32_t ret = 0;

    if (master == RT_NULL)
        return ret;

    if ((buff == RT_NULL) || (len == 0))
        return ret;

    cJSON *root_obj = cJSON_CreateObject();
    if (root_obj == RT_NULL)
    {
        return ret;
    }
    cJSON_AddStringToObject(root_obj, "cmd", "dc_config");

    cJSON *data_obj = cJSON_AddObjectToObject(root_obj, "data");
    if (data_obj == RT_NULL)
    {
        cJSON_Delete(root_obj);
        return ret;
    }
    cJSON_AddNumberToObject(data_obj, "dc_source", master->dc_index);
    cJSON_AddNumberToObject(data_obj, "dc_active", master->dc_active);
    cJSON_AddNumberToObject(data_obj, "dc_type", master->dc_type);
    cJSON_AddNumberToObject(data_obj, "cycle_time0", master->dc_cycltime0);
    cJSON_AddNumberToObject(data_obj, "cycle_time1", master->dc_cycltime1);
    cJSON_AddNumberToObject(data_obj, "shift_time", master->dc_cyclshift);

    ret = cJSON_PrintPreallocated(root_obj, buff, len, 0);
    if (ret)
    {
        ret = rt_strnlen(buff, len) + 1;
    }
    cJSON_Delete(root_obj);
    return ret;
}

uint32_t ecat_config_result_json(
    const char *cmd, int16_t error_code, char *buff, uint32_t len)
{
    uint32_t ret = 0;

    if ((buff == RT_NULL) || (len == 0))
        return ret;

    cJSON *root_obj = cJSON_CreateObject();
    if (root_obj == RT_NULL)
    {
        return ret;
    }
    cJSON_AddStringToObject(root_obj, "cmd", cmd);

    cJSON *data_obj = cJSON_AddObjectToObject(root_obj, "data");
    if (data_obj == RT_NULL)
    {
        cJSON_Delete(root_obj);
        return ret;
    }
    if (error_code == 0)
    {
        cJSON_AddStringToObject(data_obj, "result", "success");
        cJSON_AddNumberToObject(data_obj, "error_code", 0);
    }
    else
    {
        cJSON_AddStringToObject(data_obj, "result", "fail");
        cJSON_AddNumberToObject(data_obj, "error_code", error_code);
    }

    ret = cJSON_PrintPreallocated(root_obj, buff, len, 0);
    if (ret)
    {
        ret = rt_strnlen(buff, len) + 1;
    }
    cJSON_Delete(root_obj);
    return ret;
}

#endif
