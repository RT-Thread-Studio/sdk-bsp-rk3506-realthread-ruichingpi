/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-08-21     zhangjun     copy from minilibc
 * 2020-09-07     Meco Man     combine gcc armcc iccarm
 * 2021-02-05     Meco Man     add timegm()
 * 2021-02-07     Meco Man     fixed gettimeofday()
 * 2021-02-08     Meco Man     add settimeofday() stime()
 * 2021-02-10     Meco Man     add ctime_r() and re-implement ctime()
 * 2021-02-11     Meco Man     fix bug #3183 - align days[] and months[] to 4 bytes
 * 2021-02-12     Meco Man     add errno
 * 2012-12-08     Bernard      <clock_time.c> fix the issue of _timevalue.tv_usec initialization,
 *                             which found by Rob <rdent@iinet.net.au>
 * 2021-02-12     Meco Man     move all of the functions located in <clock_time.c> to this file
 * 2021-03-15     Meco Man     fixed a bug of leaking memory in asctime()
 * 2021-05-01     Meco Man     support fixed timezone
 * 2021-07-21     Meco Man     implement that change/set timezone APIs
 * 2023-07-03     xqyjlj       refactor posix time and timer
 * 2023-07-16     Shell        update signal generation routine for lwp
 *                             adapt to new api and do the signal handling in thread context
 * 2023-08-12     Meco Man     re-implement RT-Thread lightweight timezone API
 * 2023-09-15     xqyjlj       perf rt_hw_interrupt_disable/enable
 * 2023-10-23     Shell        add lock for _g_timerid
 * 2023-11-16     Shell        Fixup of nanosleep if previous call was interrupted
 */

#include "sys/time.h"
#include <rthw.h>
#include <rtthread.h>
#ifdef RT_USING_RTC
#include <rtdevice.h>
#endif /* RT_USING_RTC */
#include <sys/errno.h>
#include <unistd.h>
#include "sys/rt_time.h"

#define DBG_TAG    "time"
#define DBG_LVL    DBG_INFO
#include <rtdbg.h>

#define _WARNING_NO_RTC "Cannot find a RTC device!"

rt_align(RT_ALIGN_SIZE) static const char days[] = "Sun Mon Tue Wed Thu Fri Sat ";
rt_align(RT_ALIGN_SIZE) static const char months[] = "Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec ";

static void num2str(char *c, int i)
{
    c[0] = i / 10 + '0';
    c[1] = i % 10 + '0';
}

#ifdef RT_USING_RTC
static rt_err_t _control_rtc(int cmd, void *arg)
{
    static rt_device_t device = RT_NULL;
    rt_err_t rst = -RT_ERROR;

    if (device == RT_NULL)
    {
        device = rt_device_find("rtc");
    }

    /* read timestamp from RTC device */
    if (device != RT_NULL)
    {
        if (rt_device_open(device, 0) == RT_EOK)
        {
            rst = rt_device_control(device, cmd, arg);
            rt_device_close(device);
        }
    }
    else
    {
        LOG_W(_WARNING_NO_RTC);
        return -RT_ENOSYS;
    }
    return rst;
}
#endif /* RT_USING_RTC */

/* lightweight timezone and daylight saving time */
#ifdef RT_LIBC_USING_LIGHT_TZ_DST
#ifndef RT_LIBC_TZ_DEFAULT_HOUR
#define RT_LIBC_TZ_DEFAULT_HOUR   (8U)
#endif /* RT_LIBC_TZ_DEFAULT_HOUR */

#ifndef RT_LIBC_TZ_DEFAULT_MIN
#define RT_LIBC_TZ_DEFAULT_MIN    (0U)
#endif /* RT_LIBC_TZ_DEFAULT_MIN */

#ifndef RT_LIBC_TZ_DEFAULT_SEC
#define RT_LIBC_TZ_DEFAULT_SEC    (0U)
#endif /* RT_LIBC_TZ_DEFAULT_SEC */

static volatile int32_t _current_tz_offset_sec = \
    RT_LIBC_TZ_DEFAULT_HOUR * 3600U + RT_LIBC_TZ_DEFAULT_MIN * 60U + RT_LIBC_TZ_DEFAULT_SEC;

/* return current timezone offset in seconds */
void rt_tz_set(int32_t offset_sec)
{
    _current_tz_offset_sec = offset_sec;
}

int32_t rt_tz_get(void)
{
    return _current_tz_offset_sec;
}

int8_t rt_tz_is_dst(void)
{
    return 0U; /* TODO */
}
#endif /* RT_LIBC_USING_LIGHT_TZ_DST */

struct tm* rt_localtime_r(const time_t* t, struct tm* r)
{
    time_t local_tz;
#if defined(RT_LIBC_USING_LIGHT_TZ_DST)
    local_tz = *t + rt_tz_get();
#else
    local_tz = *t + 0U;
#endif /* RT_LIBC_USING_LIGHT_TZ_DST */
    return gmtime_r(&local_tz, r);
}

struct tm* rt_localtime(const time_t* t)
{
    static struct tm tmp;
    return rt_localtime_r(t, &tmp);
}

char* rt_asctime_r(const struct tm *t, char *buf)
{
    if(t == RT_NULL || buf == RT_NULL)
    {
        rt_set_errno(EFAULT);
        return RT_NULL;
    }

    rt_memset(buf, RT_NULL, 26);

    /* Checking input validity */
    if ((int)rt_strlen(days) <= (t->tm_wday << 2) || (int)rt_strlen(months) <= (t->tm_mon << 2))
    {
        LOG_W("asctime_r: the input parameters exceeded the limit, please check it.");
        *(int*) buf = *(int*) days;
        *(int*) (buf + 4) = *(int*) months;
        num2str(buf + 8, t->tm_mday);
        if (buf[8] == '0')
            buf[8] = ' ';
        buf[10] = ' ';
        num2str(buf + 11, t->tm_hour);
        buf[13] = ':';
        num2str(buf + 14, t->tm_min);
        buf[16] = ':';
        num2str(buf + 17, t->tm_sec);
        buf[19] = ' ';
        num2str(buf + 20, 2000 / 100);
        num2str(buf + 22, 2000 % 100);
        buf[24] = '\n';
        buf[25] = '\0';
        return buf;
    }

    /* "Wed Jun 30 21:49:08 1993\n" */
    *(int*) buf = *(int*) (days + (t->tm_wday << 2));
    *(int*) (buf + 4) = *(int*) (months + (t->tm_mon << 2));
    num2str(buf + 8, t->tm_mday);
    if (buf[8] == '0')
        buf[8] = ' ';
    buf[10] = ' ';
    num2str(buf + 11, t->tm_hour);
    buf[13] = ':';
    num2str(buf + 14, t->tm_min);
    buf[16] = ':';
    num2str(buf + 17, t->tm_sec);
    buf[19] = ' ';
    num2str(buf + 20, (t->tm_year + 1900) / 100);
    num2str(buf + 22, (t->tm_year + 1900) % 100);
    buf[24] = '\n';
    buf[25] = '\0';
    return buf;
}

char *rt_asctime(const struct tm *timeptr)
{
    static char buf[26];
    return rt_asctime_r(timeptr, buf);
}

char *rt_ctime_r(const time_t * tim_p, char * result)
{
    struct tm tm;
    return rt_asctime_r(rt_localtime_r(tim_p, &tm), result);
}

char *rt_ctime(const time_t *tim_p)
{
    return rt_asctime(rt_localtime(tim_p));
}

/**
 * Returns the current time.
 *
 * @param time_t * t the timestamp pointer, if not used, keep NULL.
 *
 * @return The value ((time_t)-1) is returned if the calendar time is not available.
 *         If timer is not a NULL pointer, the return value is also stored in timer.
 *
 */
rt_weak time_t rt_time(time_t *t)
{
#ifdef RT_USING_RTC
    time_t _t;

    if (_control_rtc(RT_DEVICE_CTRL_RTC_GET_TIME, &_t) != RT_EOK)
    {
        rt_set_errno(EFAULT);
        return (time_t)-1;
    }

    if (t)
        *t = _t;

    return _t;
#else
    rt_set_errno(EFAULT);
    return (time_t)-1;
#endif
}
