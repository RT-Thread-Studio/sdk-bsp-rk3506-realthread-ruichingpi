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

static void virtual_eth_example(int argc, char *argv[])
{
    rt_kprintf("Virtual Network Card Data Transceiver Connectivity Test\n");

    rt_kprintf("Please use the following commands to test network connectivity:\n\n");

    rt_kprintf("Run on Linux terminal to get IP address:\n");
    rt_kprintf("   ifconfig\n\n");

    rt_kprintf("Run on RT-Thread terminal to ping Linux:\n");
    rt_kprintf("   ping <host address> [netdev name]\n\n");

    rt_kprintf("Example:\n");
    rt_kprintf("   ping 192.168.100.100 veth\n");
}
MSH_CMD_EXPORT(virtual_eth_example, virtual ethernet example);
