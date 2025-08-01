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

static int modbus_rtu_pt100(void)
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

    rc = modbus_read_registers(ctx, 0, 2, tab_reg);
    if (rc == -1)
    {
        rt_kprintf("Failed to read registers after write\n");
    }
    else
    {
        for (i = 0; i < rc; i++)
        {
            if (tab_reg[i] != 0xEC78)
            {
                rt_kprintf(
                    "temp %d: %d.%d°C\n", i, tab_reg[i] / 10, tab_reg[i] % 10);
            }
            else
            {
                rt_kprintf("temp %d: Not connected\n", i);
            }
        }
    }

    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
MSH_CMD_EXPORT(modbus_rtu_pt100, modbus_rtu_pt100);
