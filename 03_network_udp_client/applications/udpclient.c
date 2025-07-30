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
#include <stdlib.h>
#include <sys/socket.h>
#include "netdb.h"

#define DEBUG_UDP_CLIENT

#define DBG_TAG "UDP"
#ifdef DEBUG_UDP_CLIENT
#define DBG_LVL DBG_LOG
#else
#define DBG_LVL DBG_INFO
#endif
#include <rtdbg.h>

static int started = 0;
static int is_running = 0;
static char url[256];
static int port = 8080;
static int count = 10;
const char send_data[] = "This is UDP Client from RT-Thread.\n";

static void udpclient(void *arg)
{
    int sock;
    struct hostent *host;
    struct sockaddr_in server_addr;

    host = (struct hostent *)gethostbyname(url);
    if (host == RT_NULL)
    {
        LOG_E("Get host by name failed!");
        return;
    }

    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        LOG_E("Create socket error");
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    started = 1;
    is_running = 1;

    while (count && is_running)
    {
        sendto(sock, send_data, rt_strlen(send_data), 0,
               (struct sockaddr *)&server_addr, sizeof(struct sockaddr));

        rt_thread_mdelay(1000);
        count --;
    }

    if (count == 0)
    {
        LOG_I("UDP client send data finished!");
    }

    if (sock >= 0)
    {
        closesocket(sock);
        sock = -1;
    }

    started = 0;
    is_running = 0;
}

static void usage(void)
{
    rt_kprintf("Usage: udpclient -h <host> -p <port> [--cnt] [count]\n");
    rt_kprintf("       udpclient --stop\n");
    rt_kprintf("       udpclient --help\n");
    rt_kprintf("\n");
    rt_kprintf("Miscellaneous:\n");
    rt_kprintf("  -h           Specify host address\n");
    rt_kprintf("  -p           Specify the host port number\n");
    rt_kprintf("  --cnt        Specify the send data count\n");
    rt_kprintf("  --stop       Stop udpclient program\n");
    rt_kprintf("  --help       Print help information\n");
}

static void udpclient_test(int argc, char** argv)
{
    rt_thread_t tid;

    if (argc == 1 || argc > 7)
    {
        LOG_I("Please check the command you entered!\n");
        goto __usage;
    }
    else
    {
        if (rt_strcmp(argv[1], "--help") == 0)
        {
            goto __usage;
        }
        else if (rt_strcmp(argv[1], "--stop") == 0)
        {
            is_running = 0;
            return;
        }
        else if (argv[1] != RT_NULL && rt_strcmp(argv[1], "-h") == 0)
        {
            if ( argv[2] == RT_NULL)
            {
                goto __usage;
            }

            if (started)
            {
                LOG_I("The udpclient has started!");
                LOG_I("Please stop udpclient firstly, by: udpclient --stop");
                return;
            }

            if (argc == 7 && rt_strcmp(argv[5], "--cnt") == 0)
            {
                count = atoi(argv[6]);
            }

            if (rt_strlen(argv[2]) > sizeof(url))
            {
                LOG_E("The input url is too long, max %d bytes!", sizeof(url));
                return;
            }

            rt_memset(url, 0x0, sizeof(url));
            rt_strncpy(url, argv[2], rt_strlen(argv[2]));

            if (argv[3] != RT_NULL && rt_strcmp(argv[3], "-p") == 0)
            {
                if (argv[4] == RT_NULL)
                {
                    goto __usage;
                }

                port = atoi(argv[4]);
            }

        }
        else
        {
            goto __usage;
        }
    }

    tid = rt_thread_create("udp_client", udpclient, RT_NULL, 2048,
        RT_THREAD_PRIORITY_MAX/3, 20);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }

    return;

__usage:
    usage();
}

MSH_CMD_EXPORT_ALIAS(udpclient_test, udpclient,
    Start a udp client. Help: udpclient --help);
