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
#ifdef PKG_USING_AGILE_FTP
#include <ftp.h>

#define FTP_THREAD_STACKSIZE (8 * 1024) /* FTP thread stack size */
#define FTP_PRIORITY         9          /* FTP thread priority */
#define FTP_SERVER_TIMESLICE 100

static rt_bool_t ftp_server_enbale = RT_FALSE;

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
#endif /* PKG_USING_AGILE_FTP */
