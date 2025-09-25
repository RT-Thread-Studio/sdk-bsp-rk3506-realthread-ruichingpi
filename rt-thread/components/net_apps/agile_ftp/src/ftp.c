// #include <dfs_posix.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "ftp_session.h"

#include <rtthread.h>
#include <rtdevice.h>
#ifdef COMP_USING_AGILE_FTP
#include <ftp.h>


#define DBG_TAG "ftp"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#ifndef FTP_DEFAULT_PORT
#define FTP_DEFAULT_PORT        21
#endif

#define FTP_THREAD_STACKSIZE (8 * 1024) /* FTP thread stack size */
#define FTP_PRIORITY         9          /* FTP thread priority */
#define FTP_SERVER_TIMESLICE 100

static rt_bool_t ftp_server_enbale = RT_FALSE;



static int ftp_port = FTP_DEFAULT_PORT;
static rt_uint8_t force_restart = 0;

int ftp_force_restart(void)
{
    force_restart = 1;
    return RT_EOK;
}

int ftp_get_port(void)
{
    return ftp_port;
}

int ftp_set_port(int port)
{
    if((port <= 0) || (port > 65535))
        return -RT_ERROR;
    ftp_port = port;
    return RT_EOK;
}

static void ftp_entry(void *parameter)
{
    int server_fd = -1;
    int enable = 1;
    int flags;
    struct sockaddr_in addr;
    socklen_t addrlen;

    // select使用
    fd_set readset, exceptset;
    // select超时时间
    struct timeval select_timeout;
    select_timeout.tv_sec = 1;
    select_timeout.tv_usec = 0;

    rt_thread_mdelay(5000);

_ftp_start:
    server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server_fd < 0)
        goto _ftp_restart;

    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&enable, sizeof(enable)) < 0)
        goto _ftp_restart;

    rt_memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(ftp_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        goto _ftp_restart;

    if(listen(server_fd, 1) < 0)
        goto _ftp_restart;

    flags = fcntl(server_fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(server_fd, F_SETFL, flags);

    LOG_I("service launched success.");
    while(1)
    {
        if(force_restart)
        {
            force_restart = 0;
            ftp_session_force_quit();
            break;
        }

        FD_ZERO(&readset);
        FD_ZERO(&exceptset);

        FD_SET(server_fd, &readset);
        FD_SET(server_fd, &exceptset);

        int rc = select(server_fd + 1, &readset, RT_NULL, &exceptset, &select_timeout);
        if(rc < 0)
            break;
        if(rc > 0)
        {
            if (FD_ISSET(server_fd, &exceptset))
                break;
            if (FD_ISSET(server_fd, &readset))
            {
                addrlen = sizeof(struct sockaddr_in);
                int client_fd = accept(server_fd, (struct sockaddr *)&addr, &addrlen);
                if(client_fd < 0)
                    break;
                if(ftp_session_create(client_fd, &addr, addrlen) != RT_EOK)
                    close(client_fd);
            }
        }
    }

_ftp_restart:
    LOG_W("service go wrong, now wait restarting...");
    if(server_fd >= 0)
    {
        close(server_fd);
        server_fd = -1;
    }

    rt_thread_mdelay(1000);
    goto _ftp_start;
}

int ftp_init(rt_uint32_t stack_size, rt_uint8_t priority, rt_uint32_t tick)
{
    rt_thread_t tid = rt_thread_create("ftp", ftp_entry, RT_NULL, stack_size, priority, tick);
    RT_ASSERT(tid != RT_NULL);
    rt_thread_startup(tid);

    return RT_EOK;
}

static int ftp_server(void)
{
    rt_err_t ret = RT_EOK;

    if (RT_TRUE == ftp_server_enbale)
    {
        rt_kprintf("ftp server started\n");
        return 0;
    }

    /* set ftp server configuration, */
    /* default username: loogg, password: loogg, port: 21 */
    ftp_set_session_username("admin");
    ftp_set_session_password("admin");

    ret = ftp_init(FTP_THREAD_STACKSIZE, FTP_PRIORITY, FTP_SERVER_TIMESLICE);
    if (RT_EOK != ret)
    {
        rt_kprintf("ftp server start fail\n");
    }
    else
    {
        rt_kprintf("ftp server start success\n");
        ftp_server_enbale = RT_TRUE;
    }

    return 0;
}
MSH_CMD_EXPORT(ftp_server, start ftp server);
#endif /* COMP_USING_AGILE_FTP */

