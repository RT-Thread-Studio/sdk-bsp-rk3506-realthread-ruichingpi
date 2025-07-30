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
#include <string.h>
#include <stdlib.h>

#if !defined(SAL_USING_POSIX)
#error "Please enable SAL_USING_POSIX!"
#else
#include <sys/time.h>
#include <sys/select.h>
#endif
#include <sys/socket.h>
#include "netdb.h"

#define DEBUG_UDP_SERVER

#define DBG_TAG "UDP"
#ifdef DEBUG_UDP_SERVER
#define DBG_LVL DBG_LOG
#else
#define DBG_LVL DBG_INFO
#endif
#include <rtdbg.h>

#define BUFSZ   1024

static int started = 0;
static int is_running = 0;
static int port = 5000;

static void udpserv(void *paramemter)
{
    int sock;
    int bytes_read;
    char *recv_data;
    socklen_t addr_len;
    struct sockaddr_in server_addr, client_addr;

    struct timeval timeout;
    fd_set readset;

    recv_data = rt_malloc(BUFSZ);
    if (recv_data == RT_NULL)
    {
        LOG_E("No memory");
        return;
    }

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        LOG_E("Create socket error");
        goto __exit;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    if (bind(sock, (struct sockaddr *)&server_addr,
             sizeof(struct sockaddr)) == -1)
    {
        LOG_E("Unable to bind");
        goto __exit;
    }

    addr_len = sizeof(struct sockaddr);
    LOG_I("UDPServer Waiting for client on port %d...", port);

    started = 1;
    is_running = 1;

    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    while (is_running)
    {
        FD_ZERO(&readset);
        FD_SET(sock, &readset);

        /* Wait for read or write */
        if (select(sock + 1, &readset, RT_NULL, RT_NULL, &timeout) == 0)
            continue;

        bytes_read = recvfrom(sock, recv_data, BUFSZ - 1, 0,
            (struct sockaddr *)&client_addr, &addr_len);
        if (bytes_read < 0)
        {
            LOG_E("Received error, close the connect.");
            goto __exit;
        }
        else if (bytes_read == 0)
        {
            LOG_W("Received warning, recv function return 0.");
            continue;
        }
        else
        {
            recv_data[bytes_read] = '\0';

            LOG_D("Received data = %s", recv_data);

            if (strcmp(recv_data, "exit") == 0)
            {
                goto __exit;
            }
        }
    }

__exit:
    if (recv_data)
    {
        rt_free(recv_data);
        recv_data = RT_NULL;
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
    rt_kprintf("Usage: udpserver -p <port>\n");
    rt_kprintf("       udpserver --stop\n");
    rt_kprintf("       udpserver --help\n");
    rt_kprintf("\n");
    rt_kprintf("Miscellaneous:\n");
    rt_kprintf("  -p           Specify the host port number\n");
    rt_kprintf("  --stop       Stop udpserver program\n");
    rt_kprintf("  --help       Print help information\n");
}

static void udpserver_test(int argc, char** argv)
{
    rt_thread_t tid;

    if (argc == 1 || argc > 3)
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
        else if (rt_strcmp(argv[1], "-p") == 0)
        {
            if (argv[2] == RT_NULL)
            {
                goto __usage;
            }

            if (started)
            {
                LOG_I("The udpserver has started!");
                LOG_I("Please stop udpserver firstly, by: udpserver --stop");
                return;
            }

            port = atoi(argv[2]);
        }
        else
        {
            goto __usage;
        }
    }

    tid = rt_thread_create("udp_serv", udpserv, RT_NULL, 2048, 20, 20);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }

    return;

__usage:
    usage();
}

MSH_CMD_EXPORT_ALIAS(udpserver_test, udpserver,
    Start a udp server. Help: udpserver --help);
