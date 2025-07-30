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
#include <string.h>

#if !defined(SAL_USING_POSIX)
#error "Please enable SAL_USING_POSIX!"
#else
#include <sys/select.h>
#include <sys/time.h>
#endif
#include "netdb.h"
#include <sys/socket.h>

#define DEBUG_TCP_SERVER

#define DBG_TAG "TCP"
#ifdef DEBUG_TCP_SERVER
#define DBG_LVL DBG_LOG
#else
#define DBG_LVL DBG_INFO
#endif
#include <rtdbg.h>

#define BUFSZ (1024)

static int started = 0;
static int is_running = 0;
static int port = 5000;

static const char send_data[] = "This is TCP Server from RT-Thread.";

static void tcpserv(void *arg)
{
    int ret;
    char *recv_data;
    int sock, bytes_received;
    int connected = -1;
    struct sockaddr_in server_addr, client_addr;

    struct timeval timeout;
    fd_set readset, readset_c;
    socklen_t sin_size = sizeof(struct sockaddr_in);

    recv_data = rt_malloc(BUFSZ);
    if (recv_data == RT_NULL)
    {
        LOG_E("No memory");
        return;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        LOG_E("Create socket error");
        goto __exit;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(server_addr.sin_zero), 0x0, sizeof(server_addr.sin_zero));

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) ==
        -1)
    {
        LOG_E("Unable to bind");
        goto __exit;
    }

    if (listen(sock, 10) == -1)
    {
        LOG_E("Listen error");
        goto __exit;
    }

    LOG_I("\nTCPServer Waiting for client on port %d...\n", port);

    started = 1;
    is_running = 1;

    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    while (is_running)
    {
        FD_ZERO(&readset);
        FD_SET(sock, &readset);

        LOG_I("Waiting for a new connection...");

        /* Wait for read or write */
        if (select(sock + 1, &readset, RT_NULL, RT_NULL, &timeout) == 0)
        {
            continue;
        }

        connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
        if (connected < 0)
        {
            LOG_E("accept connection failed! errno = %d", errno);
            continue;
        }

        LOG_I("I got a connection from (%s , %d)\n",
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        while (is_running)
        {
            FD_ZERO(&readset_c);
            FD_SET(connected, &readset_c);

            /* Wait for read or write */
            if (select(connected + 1, &readset_c, RT_NULL, RT_NULL, &timeout) ==
                0)
            {
                continue;
            }

            bytes_received = recv(connected, recv_data, BUFSZ, 0);
            if (bytes_received < 0)
            {
                LOG_E("Received error, close the connect.");
                closesocket(connected);
                connected = -1;
                break;
            }
            else if (bytes_received == 0)
            {
                LOG_W("Received warning, recv function return 0.");
                continue;
            }
            else
            {
                recv_data[bytes_received] = '\0';
                if (strcmp(recv_data, "q") == 0 || strcmp(recv_data, "Q") == 0)
                {
                    LOG_I("Got a 'q' or 'Q', close the connect.");
                    closesocket(connected);
                    connected = -1;
                    break;
                }
                else if (strcmp(recv_data, "exit") == 0)
                {
                    closesocket(connected);
                    connected = -1;
                    goto __exit;
                }
                else
                {
                    LOG_D("Received data = %s", recv_data);
                }
            }

            ret = send(connected, send_data, rt_strlen(send_data), 0);
            if (ret < 0)
            {
                LOG_E("send error, close the connect.");
                closesocket(connected);
                connected = -1;
                break;
            }
            else if (ret == 0)
            {
                LOG_W("Send warning, send function return 0.");
            }
        }
    }

__exit:
    if (recv_data)
    {
        rt_free(recv_data);
        recv_data = RT_NULL;
    }

    if (connected >= 0)
    {
        closesocket(connected);
        connected = -1;
    }

    if (sock >= 0)
    {
        closesocket(sock);
        sock = -1;
    }

    started = 0;
    is_running = 0;
    return;
}

static void usage(void)
{
    rt_kprintf("Usage: tcpserver -p <port>\n");
    rt_kprintf("       tcpserver --stop\n");
    rt_kprintf("       tcpserver --help\n");
    rt_kprintf("\n");
    rt_kprintf("Miscellaneous:\n");
    rt_kprintf("  -p           Specify the host port number\n");
    rt_kprintf("  --stop       Stop tcpserver program\n");
    rt_kprintf("  --help       Print help information\n");
}

static void tcpserver_test(int argc, char **argv)
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
                LOG_I("The tcpserver has started!");
                LOG_I("Please stop tcpserver firstly, by: tcpserver --stop");
                return;
            }

            port = atoi(argv[2]);
        }
        else
        {
            goto __usage;
        }
    }

    tid = rt_thread_create(
        "tcp_serv", tcpserv, RT_NULL, 2048, RT_THREAD_PRIORITY_MAX / 3, 20);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    return;

__usage:
    usage();
}

MSH_CMD_EXPORT_ALIAS(
    tcpserver_test, tcpserver, Start a tcp server.Help : tcpserver-- help);
