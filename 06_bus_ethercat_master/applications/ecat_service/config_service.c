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
#include "cJSON.h"
#include "ecat_interface.h"
#include "ecat_master.h"
#include "netdb.h"
#include <sys/socket.h> /* 使用BSD socket，需要包含socket.h头文件 */

#define DEBUG_ECAT_CONFIG_SERVICE

#define DBG_TAG "ecat srv"
#ifdef DEBUG_ECAT_CONFIG_SERVICE
#define DBG_LVL DBG_LOG
#else
#define DBG_LVL DBG_INFO /* DBG_ERROR */
#endif
#include <rtdbg.h>

#define BUFSZ 4096

static int udp_port = 4000;
static int tcp_port = 2000;

static ec_master_t *master = RT_NULL;

static uint32_t config_data_parse(char *in_data, char *out_data, uint32_t len)
{
    cJSON *root = cJSON_Parse(in_data);
    if (!root)
    {
        LOG_E("Error: %s\n", cJSON_GetErrorPtr());
        return 0;
    }
    cJSON *cmd = cJSON_GetObjectItem(root, "cmd");
    cJSON *data = cJSON_GetObjectItem(root, "data");
    if (cmd && cJSON_IsString(cmd))
    {
        if (strcmp(cmd->valuestring, "discover") == 0)
        {
            return ecat_master_discover_json(master, RT_LWIP_IPADDR, tcp_port,
                "AA:BB:CC:11:22:33", out_data, len);
        }
        else if (strcmp(cmd->valuestring, "master_info") == 0)
        {
            if (data && cJSON_IsObject(data))
            {
                cJSON *obj = cJSON_GetObjectItem(data, "loop_cycle");
                if (obj && cJSON_IsNumber(obj))
                    master->main_cycletime_us = obj->valueint;
                obj = cJSON_GetObjectItem(data, "mb_cycle");
                if (obj && cJSON_IsNumber(obj))
                    master->sub_cycletime_us = obj->valueint;
                obj = cJSON_GetObjectItem(data, "pdo_timeout");
                if (obj && cJSON_IsNumber(obj))
                    master->pdo_timeout = obj->valueint;
                obj = cJSON_GetObjectItem(data, "sdo_tx_timeout");
                if (obj && cJSON_IsNumber(obj))
                    master->sdo_tx_timeout = obj->valueint;
                obj = cJSON_GetObjectItem(data, "sdo_rx_timeout");
                if (obj && cJSON_IsNumber(obj))
                    master->sdo_rx_timeout = obj->valueint;
                obj = cJSON_GetObjectItem(data, "info_cycle");
                if (obj && cJSON_IsNumber(obj))
                    master->info_cycle = obj->valueint;
                obj = cJSON_GetObjectItem(data, "recovery_timeout");
                if (obj && cJSON_IsNumber(obj))
                    master->recovery_timeout_ms = obj->valueint;
                obj = cJSON_GetObjectItem(data, "wdt_timeout");
                if (obj && cJSON_IsNumber(obj))
                    master->wdt_timeout = obj->valueint;
                obj = cJSON_GetObjectItem(data, "wdt_enable");
                if (obj && cJSON_IsBool(obj))
                    master->wdt_enable = obj->valueint;
                obj = cJSON_GetObjectItem(data, "pdi_check");
                if (obj && cJSON_IsBool(obj))
                    master->pdi_check = obj->valueint;
            }
            return ecat_master_info_json(
                master, RT_LWIP_IPADDR, tcp_port, out_data, len);
        }
        else if (strcmp(cmd->valuestring, "master_state") == 0)
        {
            return ecat_master_state_json(master, out_data, len);
        }
        else if (strcmp(cmd->valuestring, "scan_slave") == 0)
        {
            return ecat_scan_slave_json(master, out_data, len);
        }
        else if (strcmp(cmd->valuestring, "slave_info") == 0)
        {
            return ecat_slave_info_json(master, out_data, len);
        }
        else if (strcmp(cmd->valuestring, "slave_state") == 0)
        {
            uint16_t index = 1;
            if (data && cJSON_IsObject(data))
            {
                cJSON *index_obj = cJSON_GetObjectItem(data, "index");
                if (index_obj && cJSON_IsNumber(index_obj))
                    index = index_obj->valueint;
            }
            return ecat_slave_state_json(master, index, out_data, len);
        }
        else if (strcmp(cmd->valuestring, "start") == 0)
        {
            ecat_simple_start(master);
            return ecat_config_result_json(cmd->valuestring, 0, out_data, len);
        }
        else if (strcmp(cmd->valuestring, "stop") == 0)
        {
            ecat_simple_stop(master);
            return ecat_config_result_json(cmd->valuestring, 0, out_data, len);
        }
        else if (strcmp(cmd->valuestring, "dc_config") == 0)
        {
            if (data && cJSON_IsObject(data))
            {
                cJSON *obj = cJSON_GetObjectItem(data, "dc_source");
                if (obj && cJSON_IsNumber(obj))
                    master->dc_index = obj->valueint;
                obj = cJSON_GetObjectItem(data, "dc_active");
                if (obj && cJSON_IsNumber(obj))
                    master->dc_active = obj->valueint;
                obj = cJSON_GetObjectItem(data, "dc_type");
                if (obj && cJSON_IsNumber(obj))
                    master->dc_type = obj->valueint;
                obj = cJSON_GetObjectItem(data, "cycle_time0");
                if (obj && cJSON_IsNumber(obj))
                    master->dc_cycltime0 = obj->valueint;
                obj = cJSON_GetObjectItem(data, "cycle_time1");
                if (obj && cJSON_IsNumber(obj))
                    master->dc_cycltime1 = obj->valueint;
                obj = cJSON_GetObjectItem(data, "shift_time");
                if (obj && cJSON_IsNumber(obj))
                    master->dc_cyclshift = obj->valueint;
            }
            return ecat_dc_config_json(master, out_data, len);
        }
    }
    else
    {
        LOG_E("parse cmd error!");
    }
    return 0;
}
static void udpserv_thread_entry(void *paramemter)
{
    int sock;
    int bytes_read;
    char *recv_data;
    char *send_data;
    socklen_t addr_len;
    struct sockaddr_in server_addr, client_addr;

    struct timeval timeout;
    fd_set readset;

    /* 分配接收用的数据缓冲 */
    recv_data = rt_malloc(BUFSZ);
    if (recv_data == RT_NULL)
    {
        LOG_E("No memory");
        return;
    }
    send_data = rt_malloc(BUFSZ);
    if (send_data == RT_NULL)
    {
        LOG_E("No memory");
        rt_free(recv_data);
        return;
    }
    /* 创建一个socket，类型是SOCK_DGRAM，UDP类型 */
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        LOG_E("Create socket error");
        goto __exit;
    }

    /* 初始化服务端地址 */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(udp_port);
    server_addr.sin_addr.s_addr = inet_addr(RT_LWIP_IPADDR);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    /* 绑定socket到服务端地址 */
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) ==
        -1)
    {
        LOG_E("Unable to bind");
        goto __exit;
    }

    addr_len = sizeof(struct sockaddr);
    LOG_I("UDPServer Waiting for client on port %d...", udp_port);

    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    while (1)
    {
        FD_ZERO(&readset);
        FD_SET(sock, &readset);

        /* Wait for read or write */
        if (select(sock + 1, &readset, RT_NULL, RT_NULL, &timeout) == 0)
            continue;

        /* 从sock中收取最大BUFSZ - 1字节数据 */
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
            recv_data[bytes_read] = '\0'; /* 把末端清零 */
            uint32_t ret = config_data_parse(recv_data, send_data, BUFSZ);
            if (ret > 0)
            {
                sendto(sock, send_data, ret, 0, (struct sockaddr *)&client_addr,
                    addr_len);
            }
        }
    }

__exit:
    if (recv_data)
    {
        rt_free(recv_data);
        recv_data = RT_NULL;
    }
    if (send_data)
    {
        rt_free(send_data);
        send_data = RT_NULL;
    }
    if (sock >= 0)
    {
        closesocket(sock);
        sock = -1;
    }
}

static void tcpserv_thread_entry(void *arg)
{
    char *recv_data; /* 用于接收的指针，后面会做一次动态分配以请求可用内存 */
    char *send_data;
    int sock = -1, connected = -1, bytes_received;
    struct sockaddr_in server_addr, client_addr;

    struct timeval timeout;
    fd_set readset, readset_c;
    socklen_t sin_size = sizeof(struct sockaddr_in);

    recv_data = rt_malloc(BUFSZ); /* 分配接收用的数据缓冲 */
    if (recv_data == RT_NULL)
    {
        LOG_E("No memory");
        return;
    }

    send_data = rt_malloc(BUFSZ); /* 分配接收用的数据缓冲 */
    if (send_data == RT_NULL)
    {
        LOG_E("No memory");
        rt_free(recv_data);
        return;
    }
    /* 一个socket在使用前，需要预先创建出来，指定SOCK_STREAM为TCP的socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        LOG_E("Create socket error");
        goto __exit;
    }

    /* 初始化服务端地址 */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(tcp_port); /* 服务端工作的端口 */
    server_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(server_addr.sin_zero), 0x0, sizeof(server_addr.sin_zero));

    /* 绑定socket到服务端地址 */
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) ==
        -1)
    {
        LOG_E("Unable to bind");
        goto __exit;
    }

    /* 在socket上进行监听 */
    if (listen(sock, 10) == -1)
    {
        LOG_E("Listen error");
        goto __exit;
    }

    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    while (1)
    {
        FD_ZERO(&readset);
        FD_SET(sock, &readset);

        /* Wait for read or write */
        if (select(sock + 1, &readset, RT_NULL, RT_NULL, &timeout) == 0)
            continue;

        /* 接受一个客户端连接socket的请求，这个函数调用是阻塞式的 */
        connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
        /* 返回的是连接成功的socket */
        if (connected < 0)
        {
            LOG_E("accept connection failed! errno = %d", errno);
            continue;
        }

        /* 接受返回的client_addr指向了客户端的地址信息 */
        LOG_I("I got a connection from (%s , %d)\n",
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        /* 客户端连接的处理 */
        while (1)
        {
            FD_ZERO(&readset_c);
            FD_SET(connected, &readset_c);

            /* Wait for read or write */
            if (select(connected + 1, &readset_c, RT_NULL, RT_NULL, &timeout) ==
                0)
                continue;

            /* 从connected socket中接收数据，接收buffer是1024大小，但并不一定能够收到1024大小的数据 */
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
                /* 打印recv函数返回值为0的警告信息 */
                LOG_W("Received warning, recv function return 0.");
                continue;
            }
            else
            {
                /* 有接收到数据，把末端清零 */
                recv_data[bytes_received] = '\0';
                uint32_t ret = config_data_parse(recv_data, send_data, BUFSZ);
                if (ret > 0)
                {
                    /* 发送数据到connected socket */
                    ret = send(connected, send_data, ret, 0);
                    if (ret < 0)
                    {
                        LOG_E("send error, close the connect.");
                        closesocket(connected);
                        connected = -1;
                        break;
                    }
                    else if (ret == 0)
                    {
                        /* 打印send函数返回值为0的警告信息 */
                        LOG_W("Send warning, send function return 0.");
                    }
                }
            }
        }
    }

__exit:
    if (recv_data)
    {
        rt_free(recv_data);
        recv_data = RT_NULL;
    }
    if (send_data)
    {
        rt_free(send_data);
        send_data = RT_NULL;
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

    return;
}

void ecat_config_service_init(ec_master_t *m)
{
    rt_thread_t tid;
    if (m == RT_NULL)
        return;
    master = m;
    tid = rt_thread_create(
        "ecat_udp", udpserv_thread_entry, RT_NULL, 2048, 20, 20);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    tid = rt_thread_create(
        "ecat_tcp", tcpserv_thread_entry, RT_NULL, 2048, 20, 20);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
}
