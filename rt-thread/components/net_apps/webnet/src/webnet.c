/*
 * File      : webnet.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 * This software is dual-licensed: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. For the terms of this
 * license, see <http://www.gnu.org/licenses/>.
 *
 * You are free to use this software under the terms of the GNU General
 * Public License, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * Alternatively for commercial application, you can contact us
 * by email <business@rt-thread.com> for commercial license.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-08-02     Bernard      the first version
 * 2012-07-03     Bernard      Add webnet port and webroot setting interface.
 */

#include <stdint.h>
#include <string.h>
#include <sys/errno.h>

#include <webnet.h>
#include <wn_module.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef SAL_USING_POSIX
#include <sys/select.h>
#else
#include <lwip/select.h>
#endif

#define RT_MAX(a, b) ((a > b) ? a : b)

#if defined(RT_USING_LWIP) && (RT_LWIP_TCPTHREAD_STACKSIZE < 1408)
#error The lwIP tcpip thread stack size(RT_LWIP_TCPTHREAD_STACKSIZE) must more than 1408
#endif

#define DBG_ENABLE
#define DBG_COLOR
#define DBG_SECTION_NAME    "wn"
#ifdef WEBNET_USING_LOG
#define DBG_LEVEL           DBG_LOG
#else
#define DBG_LEVEL           DBG_INFO
#endif /* WEBNET_USING_LOG */
#include <rtdbg.h>

static rt_list_t port_info_list;
static rt_mutex_t port_mutex;
static rt_bool_t list_inited = RT_FALSE;

void webnet_set_port(struct webnet_session *session, int port)
{
    session->port->port = port;
}

int webnet_get_port(struct webnet_session *session)
{
    return session->port->port;
}

void webnet_set_root(struct webnet_session *session, const char* webroot_path)
{
    rt_strncpy(session->port->root_path, webroot_path, WEBNET_PATH_MAX);
    session->port->root_path[WEBNET_PATH_MAX - 1] = '\0';
}

const char* webnet_get_root(struct webnet_session *session)
{
    return session->port->root_path;
}


static void webnet_port_list_init(void) {
    if (!list_inited) {
        rt_list_init(&port_info_list);
        port_mutex = rt_mutex_create("webnet_mutex", RT_IPC_FLAG_FIFO);
        list_inited = RT_TRUE;
    }
}

static rt_bool_t webnet_check_root_path(const char* root_path)
{
    if (root_path == RT_NULL || strlen(root_path) == 0) {
        LOG_E("Root path is null or empty");
        return RT_FALSE;
    }

    struct stat path_stat;
    if (stat(root_path, &path_stat) < 0) {
        LOG_E("stat path '%s' failed (errno=%d)", root_path, errno);

        if (errno == ENOENT) LOG_E("→ Reason: Path does not exist");
        if (errno == EACCES) LOG_E("→ Reason: No permission to access");
        if (errno == ENOTDIR) LOG_E("→ Reason: Not a directory");
        return RT_FALSE;
    }

    if (!(path_stat.st_mode & S_IFDIR)) {
        LOG_E("Path '%s' is not a directory (it's a file)", root_path);
        return RT_FALSE;
    }

    if (access(root_path, R_OK) < 0) {
        LOG_E("No read permission for path '%s' (errno=%d)", root_path, errno);
        return RT_FALSE;
    }

    LOG_D("Root path '%s' is valid (directory with read permission)", root_path);
    return RT_TRUE;
}

const char* webnet_get_root_by_port(rt_uint16_t port) {
    struct webnet_service *node;

    if (rt_mutex_take(port_mutex, RT_WAITING_FOREVER) != RT_EOK) {
        return RT_NULL;
    }

    rt_list_for_each_entry(node, &port_info_list, list) {
        if (node->port == port) {
            rt_mutex_release(port_mutex);
            return node->root_path;
        }
    }

    rt_mutex_release(port_mutex);
    return RT_NULL;
}

static rt_bool_t webnet_port_is_running(rt_uint16_t port) {
    struct webnet_service *node;
    rt_bool_t result = RT_FALSE;

    if (rt_mutex_take(port_mutex, RT_WAITING_FOREVER) != RT_EOK) {
        return RT_FALSE;
    }

    rt_list_for_each_entry(node, &port_info_list, list) {
        if (node->port == port) {
            result = RT_TRUE;
            break;
        }
    }

    rt_mutex_release(port_mutex);
    return result;
}

static void webnet_thread_entry(void *parameter)
{
    int listenfd = -1;
    fd_set readset, tempfds;
    fd_set writeset, tempwrtfds;
    int sock_fd, maxfdp1;
    struct sockaddr_in saddr;
    struct timeval rcv_to = {0, 50000};
    struct webnet_service *param = (struct webnet_service *)parameter;

    listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenfd < 0)
    {
        LOG_E("Port %d: Create socket failed (errno=%d)", param->port, errno);
        goto __exit;
    }

    rt_memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(param->port);

    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    setsockopt(listenfd, SOL_SOCKET, SO_RCVTIMEO, &rcv_to, sizeof(rcv_to));

    if (bind(listenfd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
    {
        LOG_E("Port %d: Bind failed (errno=%d, maybe in use)", param->port, errno);
        goto __exit;
    }

    if (listen(listenfd, WEBNET_CONN_MAX) == -1)
    {
        LOG_E("Port %d: Listen failed (errno=%d)", param->port, errno);
        goto __exit;
    }

    webnet_module_handle_event(RT_NULL, WEBNET_EVENT_INIT);

    for (;;)
    {
        FD_ZERO(&readset);
        FD_ZERO(&writeset);
        FD_SET(listenfd, &readset);

        maxfdp1 = webnet_sessions_set_fds(&readset, &writeset, param->port);
        maxfdp1 = RT_MAX(maxfdp1, listenfd + 1);

        tempfds = readset;
        tempwrtfds = writeset;
        sock_fd = select(maxfdp1, &tempfds, &tempwrtfds, RT_NULL, &rcv_to);

        if (sock_fd < 0)
        {
            LOG_E("Port %d: Select failed (errno=%d)", param->port, errno);
            continue;
        }

        if (sock_fd == 0)
        {
            continue;
        }

        if (FD_ISSET(listenfd, &tempfds)) {
            struct webnet_session* session = webnet_session_create(listenfd);
            if (session != RT_NULL)
            {
                session->port = param;
                FD_SET(session->socket, &readset);
                LOG_D("Port %d: New connection (root: %s)", param->port, param->root_path);
            }
        }

        webnet_sessions_handle_fds(&tempfds, &tempwrtfds, param->port);
    }

__exit:
    rt_free(param);
    if (listenfd >= 0) closesocket(listenfd);
    LOG_W("Port %d: Thread exited", param->port);
}

int webnet_start(rt_uint16_t port, const char* root_path)
{
    char thread_name[RT_NAME_MAX];

    if (port < 1 || port > 65535)
    {
        LOG_E("Invalid port %d (must be 1-65535)", port);
        return -1;
    }

    if (!webnet_check_root_path(root_path))
    {
        LOG_E("Invalid root path '%s' for port %d", root_path, port);
        return -3;
    }

    webnet_port_list_init();

    if (webnet_port_is_running(port))
    {
        LOG_E("Port %d is already running", port);
        return -2;
    }

    rt_snprintf(thread_name, RT_NAME_MAX, "webnet_%d", port);

    struct webnet_service *node = rt_malloc(sizeof(struct webnet_service));

    rt_thread_t tid = rt_thread_create(
        thread_name,
        webnet_thread_entry,
        node,
        WEBNET_THREAD_STACKSIZE,
        WEBNET_PRIORITY,
        5
    );

    if (tid == RT_NULL)
    {
        LOG_E("Create thread failed for port %d", port);
        return -4;
    }

    if (node != RT_NULL)
    {
        node->port = port;
        rt_strncpy(node->root_path, root_path, sizeof(node->root_path)-1);
        node->tid = tid;
        rt_list_init(&node->list);

        rt_mutex_take(port_mutex, RT_WAITING_FOREVER);
        rt_list_insert_after(&port_info_list, &node->list);
        rt_mutex_release(port_mutex);
    }

    rt_thread_startup(tid);

    LOG_I("Port %d: Started with root '%s'", port, root_path);
    return 0;
}

int webnet_init(void)
{
    return webnet_start(WEBNET_PORT, WEBNET_ROOT);
}