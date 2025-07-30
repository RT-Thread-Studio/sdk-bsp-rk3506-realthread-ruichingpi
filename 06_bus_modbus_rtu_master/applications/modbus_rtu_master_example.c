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
#include <modbus.h>

#define RS485_RTS_PIN 57

static int modbus_rtu_master_example(void)
{
    modbus_t *ctx;
    uint16_t tab_reg[10];
    int rc;
    int i;

    ctx = modbus_new_rtu("/dev/uart5", 115200, 'N', 8, 1);
    if (ctx == NULL)
    {
        rt_kprintf("Unable to create the libmodbus context\n");
        return -1;
    }

    modbus_set_slave(ctx, 1);
    modbus_set_response_timeout(ctx, 1, 0);
    modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
    modbus_rtu_set_rts(ctx, RS485_RTS_PIN, MODBUS_RTU_RTS_UP);

    if (modbus_connect(ctx) == -1)
    {
        rt_kprintf("Connection failed\n");
        modbus_free(ctx);
        return -1;
    }

    rc = modbus_read_registers(ctx, 0, 10, tab_reg);
    if (rc == -1)
    {
        rt_kprintf("Failed to read registers\n");
        modbus_close(ctx);
        modbus_free(ctx);
        return -1;
    }

    rt_kprintf("Read %d registers:\n", rc);
    for (i = 0; i < rc; i++) { rt_kprintf("Register %d: %d\n", i, tab_reg[i]); }

    int write_start_address = 0;
    uint16_t write_values[3] = { 100, 200, 300 };
    rc = modbus_write_registers(ctx, write_start_address, 3, write_values);
    if (rc == -1)
    {
        rt_kprintf("Failed to write multiple registers\n");
    }
    else
    {
        rt_kprintf(
            "Successfully wrote multiple registers "
            "starting at address %d\n",
            write_start_address);
    }

    rc = modbus_read_registers(ctx, 0, 10, tab_reg);
    if (rc == -1)
    {
        rt_kprintf("Failed to read registers after write\n");
    }
    else
    {
        rt_kprintf("Registers after write:\n");
        for (i = 0; i < rc; i++)
        {
            rt_kprintf("Register %d: %d\n", i, tab_reg[i]);
        }
    }

    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
MSH_CMD_EXPORT(modbus_rtu_master_example, modbus_rtu_master_example);
