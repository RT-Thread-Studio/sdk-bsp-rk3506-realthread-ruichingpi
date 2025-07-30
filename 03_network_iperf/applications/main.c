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

int main(void)
{
    rt_kprintf("Hello, RT-Thread app\n");

    rt_kprintf("-------------------------iperf cmd-------------------------\n");

    rt_kprintf("----------------------test TCP send BW---------------------\n");
    rt_kprintf("Run the following command on the PC:\n");
    rt_kprintf("iperf -s -i 1 -p 5001 \n");
    rt_kprintf("Run the following command on the development board:\n");

    rt_kprintf("iperf -c <IP> -p 5001 \n");

    rt_kprintf("----------------------test TCP recv BW---------------------\n");
    rt_kprintf("Run the following command on the PC:\n");
    rt_kprintf("iperf -c <IP> -P 1 -i 1 -p 5001 -w 20K -l 2M -f m -t 30 \n");

    rt_kprintf("Run the following command on the development board:\n");
    rt_kprintf("iperf -s -p 5001 \n");

    rt_kprintf("----------------------test UDP send BW---------------------\n");
    rt_kprintf("Run the following command on the PC:\n");
    rt_kprintf("iperf -s -u -i 1 -p 5001 \n");

    rt_kprintf("Run the following command on the development board:\n");
    rt_kprintf("iperf  -u -c <IP> -p 5001 \n");

    rt_kprintf("----------------------test UDP recv BW---------------------\n");
    rt_kprintf("Run the following command on the PC:\n");
    rt_kprintf("iperf -c <IP> -u -P 1 -i 1 -p 5001 -f m -b 100M -t 30 -T 1\n");

    rt_kprintf("Run the following command on the development board:\n");
    rt_kprintf("iperf - u - s - p 5001 \n ");

    return 0;
}
