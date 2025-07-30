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

#define DEBUG_TCP_CLIENT

#define DBG_TAG "TCP"
#ifdef DEBUG_TCP_CLIENT
#define DBG_LVL DBG_LOG
#else
#define DBG_LVL DBG_INFO
#endif
#include <rtdbg.h>

#define BUFSZ 1024

static int started = 0;
static int is_running = 0;
static char url[256];
static int port = 8080;

/* 发送用到的数据 */
static const char send_data[] = "This is TCP Client from RT-Thread.";

static void tcpclient(void *arg)
{
    int ret;
    char *recv_data;
    int bytes_received;
    int sock = -1;
    struct hostent *host = RT_NULL;
    struct sockaddr_in server_addr;

    struct timeval timeout;
    fd_set readset;

    host = gethostbyname(url);
    if (host == RT_NULL)
    {
        LOG_E("Get host by name failed!");
        return;
    }

    recv_data = rt_malloc(BUFSZ);
    if (recv_data == RT_NULL)
    {
        LOG_E("No memory");
        return;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        LOG_E("Create socket error");
        goto __exit;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    if (connect(sock, (struct sockaddr *)&server_addr,
            sizeof(struct sockaddr)) == -1)
    {
        LOG_E("Connect fail!");
        goto __exit;
    }

    started = 1;
    is_running = 1;

    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    while (is_running)
    {
        FD_ZERO(&readset);
        FD_SET(sock, &readset);

        /* Wait for read */
        if (select(sock + 1, &readset, RT_NULL, RT_NULL, &timeout) == 0)
            continue;

        bytes_received = recv(sock, recv_data, BUFSZ - 1, 0);
        if (bytes_received < 0)
        {
            LOG_E("Received error, close the socket.");
            goto __exit;
        }
        else if (bytes_received == 0)
        {
            LOG_W("Received warning, recv function return 0.");
            continue;
        }
        else
        {
            recv_data[bytes_received] = '\0';

            if ((rt_strcmp(recv_data, "q") == 0) ||
                (rt_strcmp(recv_data, "Q") == 0))
            {
                LOG_I("Got a 'q' or 'Q', close the socket.");
                goto __exit;
            }
            else
            {
                LOG_D("Received data = %s", recv_data);
            }
        }

        ret = send(sock, send_data, rt_strlen(send_data), 0);
        if (ret < 0)
        {
            LOG_I("send error, close the socket.");
            goto __exit;
        }
        else if (ret == 0)
        {
            LOG_W("Send warning, send function return 0.");
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

    return;
}

static void usage(void)
{
    rt_kprintf("Usage: tcpclient -h <host> -p <port>\n");
    rt_kprintf("       tcpclient --stop\n");
    rt_kprintf("       tcpclient --help\n");
    rt_kprintf("\n");
    rt_kprintf("Miscellaneous:\n");
    rt_kprintf("  -h           Specify host address\n");
    rt_kprintf("  -p           Specify the host port number\n");
    rt_kprintf("  --stop       Stop tcpclient program\n");
    rt_kprintf("  --help       Print help information\n");
}

static void tcpclient_test(int argc, char **argv)
{
    rt_thread_t tid;


    if ((argc == 1) || (argc > 5))
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
            if (argv[2] == RT_NULL)
            {
                goto __usage;
            }

            if (started)
            {
                LOG_I("The tcpclient has started!");
                LOG_I("Please stop tcpclient firstly, by: tcpclient --stop");
                return;
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

    tid = rt_thread_create(
        "tcp_client", tcpclient, RT_NULL, 2048, RT_THREAD_PRIORITY_MAX / 3, 20);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    return;

__usage:
    usage();
}

MSH_CMD_EXPORT_ALIAS(
    tcpclient_test, tcpclient, Start a tcp client.Help : tcpclient-- help);
